/**	epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.

		Contains some modified example code from here:
		https://github.com/cornelis-61/esp32_Captdns/blob/master/main/captdns.c

		Original Example Code Header:
	* ----------------------------------------------------------------------------
	* "THE BEER-WARE LICENSE" (Revision 42):
	* Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
	* this notice you can do whatever you want with this stuff. If we meet some day, 
	* and you think this stuff is worth it, you can buy me a beer in return. 
	*
	* modified for ESP32 by Cornelis 
	* 
	* ----------------------------------------------------------------------------
*/
#include <sys/time.h>
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_system.h"
#include "lwip/sockets.h"
#include "lwip/err.h"
#include "esp_netif.h"
// #include "tcpip_adapter.h"
#include "epaper-idf-dns.h"

static const char *DNS_TAG = "epaper-idf-dns";

// static const char* dns_not_found_ip = "126.233.53.78";

static int sock_fd;

//Function to put unaligned 16-bit network values
static void  setn16(void *pp, int16_t n) {
	char *p=pp;
	*p++=(n>>8);
	*p++=(n&0xff);
}

//Function to put unaligned 32-bit network values
static void  setn32(void *pp, int32_t n) {
	char *p=pp;
	*p++=(n>>24)&0xff;
	*p++=(n>>16)&0xff;
	*p++=(n>>8)&0xff;
	*p++=(n&0xff);
}

static uint16_t  my_ntohs(uint16_t *in) {
	char *p=(char*)in;
	return ((p[0]<<8)&0xff00)|(p[1]&0xff);
}

//Parses a label into a C-string containing a dotted 
//Returns pointer to start of next fields in packet
static char*  label_to_str(char *packet, char *labelPtr, int packetSz, char *res, int resMaxLen) {
	int i, j, k;
	char *endPtr=NULL;
	i=0;
	do {
		if ((*labelPtr&0xC0)==0) {
			j=*labelPtr++; //skip past length
			//Add separator period if there already is data in res
			if (i<resMaxLen && i!=0) res[i++]='.';
			//Copy label to res
			for (k=0; k<j; k++) {
				if ((labelPtr-packet)>packetSz) return NULL;
				if (i<resMaxLen) res[i++]=*labelPtr++;
			}
		} else if ((*labelPtr&0xC0)==0xC0) {
			//Compressed label pointer
			endPtr=labelPtr+2;
			int offset=my_ntohs(((uint16_t *)labelPtr))&0x3FFF;
			//Check if offset points to somewhere outside of the packet
			if (offset>packetSz) return NULL;
			labelPtr=&packet[offset];
		}
		//check for out-of-bound-ness
		if ((labelPtr-packet)>packetSz) return NULL;
	} while (*labelPtr!=0);
	res[i]=0; //zero-terminate
	if (endPtr==NULL) endPtr=labelPtr+1;
	return endPtr;
}

//Converts a dotted hostname to the weird label form dns uses.
static char* str_to_label(char *str, char *label, int maxLen) {
	char *len=label; //ptr to len byte
	char *p=label+1; //ptr to next label byte to be written
	while (1) {
		if (*str=='.' || *str==0) {
			*len=((p-len)-1);	//write len of label bit
			len=p;				//pos of len for next part
			p++;				//data ptr is one past len
			if (*str==0) break;	//done
			str++;
		} else {
			*p++=*str++;	//copy byte
            //if ((p-label)>maxLen) return NULL;	//check out of bounds
		}
	}
	*len=0;
	return p; //ptr to first free byte in resp
}

//Receive a DNS packet and maybe send a response back
static void  dns_recv(struct sockaddr_in *premote_addr, char *pusrdata, unsigned short length) {

	char buff[EPAPER_IDF_DNS_LEN];
	char reply[EPAPER_IDF_DNS_LEN];
	int i;
	char *rend=&reply[length];
	char *p=pusrdata;
	DnsHeader *hdr=(DnsHeader*)p;
	DnsHeader *rhdr=(DnsHeader*)&reply[0];
	p+=sizeof(DnsHeader);
    //	ESP_LOGI(DNS_TAG, "DNS packet: id 0x%X flags 0x%X rcode 0x%X qcnt %d ancnt %d nscount %d arcount %d len %d\n", 
    //		my_ntohs(&hdr->id), hdr->flags, hdr->rcode, my_ntohs(&hdr->qdcount), my_ntohs(&hdr->ancount), my_ntohs(&hdr->nscount), my_ntohs(&hdr->arcount), length);
	//Some sanity checks:
	if (length>EPAPER_IDF_DNS_LEN) return; 								//Packet is longer than DNS implementation allows
	if (length<sizeof(DnsHeader)) return; 						//Packet is too short
	if (hdr->ancount || hdr->nscount || hdr->arcount) return;	//this is a reply, don't know what to do with it
	if (hdr->flags&EPAPER_IDF_DNS_FLAG_TC) return;								//truncated, can't use this
	//Reply is basically the request plus the needed data
	memcpy(reply, pusrdata, length);
	rhdr->flags|=EPAPER_IDF_DNS_FLAG_QR;

	for (i=0; i<my_ntohs(&hdr->qdcount); i++) 
	{
		//Grab the labels in the q string
		p=label_to_str(pusrdata, p, length, buff, sizeof(buff));
		if (p==NULL) return;
		DnsQuestionFooter *qf=(DnsQuestionFooter*)p;
		p+=sizeof(DnsQuestionFooter);
		
		ESP_LOGI(DNS_TAG, "DNS: EPAPER_IDF_DNS_Q (type 0x%X cl 0x%X) for %s\n", my_ntohs(&qf->type), my_ntohs(&qf->cl), buff);
		

		if (my_ntohs(&qf->type)==EPAPER_IDF_DNS_QTYPE_A) {
			//They want to know the IPv4 address of something.
			//Build the response.
			
			rend=str_to_label(buff, rend, sizeof(reply)-(rend-reply)); //Add the label
			if (rend==NULL) return;
			DnsResourceFooter *rf=(DnsResourceFooter *)rend;
			rend+=sizeof(DnsResourceFooter);
			setn16(&rf->type, EPAPER_IDF_DNS_QTYPE_A);
			setn16(&rf->cl, EPAPER_IDF_DNS_QCLASS_IN);
			setn32(&rf->ttl, 0);
			setn16(&rf->rdlength, 4); //IPv4 addr is 4 bytes;
			//Grab the current IP of the softap interface
            
			esp_netif_ip_info_t info;
			// tcpip_adapter_ip_info_t info;
			//tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &info);
			esp_netif_get_ip_info(esp_netif_next(NULL), &info);
			// tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &info);
			//struct ip_info info;
			//wifi_get_ip_info(SOFTAP_IF, &info);
			*rend++=ip4_addr1(&info.ip);
			*rend++=ip4_addr2(&info.ip);
			*rend++=ip4_addr3(&info.ip);
			*rend++=ip4_addr4(&info.ip);
			setn16(&rhdr->ancount, my_ntohs(&rhdr->ancount)+1);
			//ESP_LOGI(DNS_TAG, "IP Address:  %s\n", ip4addr_ntoa(&info.ip));
            //ESP_LOGI(DNS_TAG, "Added A rec to resp. Resp len is %d\n", (rend-reply));

		} else if (my_ntohs(&qf->type)==EPAPER_IDF_DNS_QTYPE_NS) {
			//Give ns server. Basically can be whatever we want because it'll get resolved to our IP later anyway.
			rend=str_to_label(buff, rend, sizeof(reply)-(rend-reply)); //Add the label
			DnsResourceFooter *rf=(DnsResourceFooter *)rend;
			rend+=sizeof(DnsResourceFooter);
			setn16(&rf->type, EPAPER_IDF_DNS_QTYPE_NS);
			setn16(&rf->cl, EPAPER_IDF_DNS_QCLASS_IN);
			setn16(&rf->ttl, 0);
			setn16(&rf->rdlength, 4);
			*rend++=2;
			*rend++='n';
			*rend++='s';
			*rend++=0;
			setn16(&rhdr->ancount, my_ntohs(&rhdr->ancount)+1);
            //ESP_LOGI(DNS_TAG, "Added NS rec to resp. Resp len is %d\n", (rend-reply));
		} else if (my_ntohs(&qf->type)==EPAPER_IDF_DNS_QTYPE_URI) {
			//Give uri to us
			rend=str_to_label(buff, rend, sizeof(reply)-(rend-reply)); //Add the label
			DnsResourceFooter *rf=(DnsResourceFooter *)rend;
			rend+=sizeof(DnsResourceFooter);
			DnsUriHdr *uh=(DnsUriHdr *)rend;
			rend+=sizeof(DnsUriHdr);
			setn16(&rf->type, EPAPER_IDF_DNS_QTYPE_URI);
			setn16(&rf->cl, EPAPER_IDF_DNS_QCLASS_URI);
			setn16(&rf->ttl, 0);
			setn16(&rf->rdlength, 4+16);
			setn16(&uh->prio, 10);
			setn16(&uh->weight, 1);
			memcpy(rend, "http://esp.nonet", 16);
			rend+=16;
			setn16(&rhdr->ancount, my_ntohs(&rhdr->ancount)+1);
            //ESP_LOGI(DNS_TAG, "Added NS rec to resp. Resp len is %d\n", (rend-reply));
		}
	}	
	//Send the response
	//ESP_LOGI(DNS_TAG, "Send response");
	sendto(sock_fd,(uint8_t*)reply, rend-reply, 0, (struct sockaddr *)premote_addr, sizeof(struct sockaddr_in));

}

static void dns_task(void *pvParameters) {
	struct sockaddr_in server_addr;
	uint32_t ret;
	struct sockaddr_in from;
	socklen_t fromlen;
	//struct tcpip_adapter_ip_info_t ipconfig;
	char udp_msg[EPAPER_IDF_DNS_LEN];
	
	//memset(&ipconfig, 0, sizeof(ipconfig));
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;	   
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(53);
	server_addr.sin_len = sizeof(server_addr);
	
	do {
		sock_fd=socket(AF_INET, SOCK_DGRAM, 0);
		if (sock_fd==-1) {
			ESP_LOGI(DNS_TAG, "dns_task failed to create sock!");
			vTaskDelay(1000/portTICK_RATE_MS);
		}
	} while (sock_fd==-1);
	
	do {
		ret=bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
		if (ret!=0) {
			ESP_LOGI(DNS_TAG, "dns_task failed to bind sock!");
			vTaskDelay(1000/portTICK_RATE_MS);
		}
	} while (ret!=0);

    ESP_LOGI(DNS_TAG, "DNS initialized.");
	
	while(1) {
		memset(&from, 0, sizeof(from));
		fromlen=sizeof(struct sockaddr_in);
		ret=recvfrom(sock_fd, (uint8_t *)udp_msg, EPAPER_IDF_DNS_LEN, 0,(struct sockaddr *)&from,(socklen_t *)&fromlen);
		if (ret>0) dns_recv(&from,udp_msg,ret);
	}
	
	close(sock_fd);
	vTaskDelete(NULL);
}

void epaper_idf_dns_init(void)
{
  xTaskCreate(dns_task, (const char *)"dns_task", 10000, NULL, 3, NULL);
}
