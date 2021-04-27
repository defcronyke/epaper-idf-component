/**	epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.

		Contains some modified example code from here:
		https://github.com/espressif/esp-idf/blob/release/v4.2/examples/protocols/openssl_server/main/openssl_server_example_main.c
		https://github.com/espressif/esp-idf/blob/release/v4.2/examples/protocols/http_server/restful_server/main/rest_server.c

		Original Example Code Header:
		This example code is in the Public Domain (or CC0 licensed, at your option.)

		Unless required by applicable law or agreed to in writing, this
		software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
		CONDITIONS OF ANY KIND, either express or implied.
*/
// #include <stdio.h>
// #include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>

#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs_semihost.h"
#include "esp_spiffs.h"
#include "sdmmc_cmd.h"
#if CONFIG_EXAMPLE_WEB_DEPLOY_SD
#include "driver/sdmmc_host.h"
#endif
#include "esp_vfs_fat.h"
#include "esp_vfs.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "nvs.h"
#include "nvs_flash.h"
#if CONFIG_PROJECT_CONNECT_WIFI
#include "esp_wifi.h"
#endif
// #include "openssl/ssl.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_free       free
#define mbedtls_time       time
#define mbedtls_time_t     time_t
#define mbedtls_calloc    calloc
#define mbedtls_fprintf    fprintf
#define mbedtls_printf     printf
#define mbedtls_exit            exit
#define MBEDTLS_EXIT_SUCCESS    EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE    EXIT_FAILURE
#endif

// #include "mbedtls/net_sockets.h"
#include "mbedtls/esp_debug.h"
// #include "mbedtls/ssl.h"
// #include "mbedtls/entropy.h"
// #include "mbedtls/ctr_drbg.h"
// #include "mbedtls/certs.h"
// #include "mbedtls/x509.h"
// #include "mbedtls/error.h"
// #include "mbedtls/debug.h"
// #include "mbedtls/timing.h"
#include "mbedtls/ssl_cache.h"
#include "esp_crt_bundle.h"
#include "protocol_examples_common.h"
#include "esp_http_server.h"
#include "esp_https_server.h"
#include "cJSON.h"
#include "epaper-idf-httpsd.h"



#if !defined(MBEDTLS_ENTROPY_C) || \
    !defined(MBEDTLS_SSL_TLS_C) || !defined(MBEDTLS_SSL_SRV_C) || \
    !defined(MBEDTLS_NET_C) || !defined(MBEDTLS_CTR_DRBG_C)
int main( void )
{
    mbedtls_printf("MBEDTLS_ENTROPY_C and/or "
           "MBEDTLS_SSL_TLS_C and/or MBEDTLS_SSL_SRV_C and/or "
           "MBEDTLS_NET_C and/or MBEDTLS_CTR_DRBG_C and/or not defined.\n");
    mbedtls_exit( 0 );
}
#else

#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"
#include "mbedtls/timing.h"
    
#include <stdio.h>                                                       
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#if !defined(_MSC_VER)
#include <inttypes.h>
#endif

#if !defined(_WIN32)
#include <signal.h>
#endif

#if defined(MBEDTLS_SSL_CACHE_C)
#include "mbedtls/ssl_cache.h"
#endif

#if defined(MBEDTLS_SSL_TICKET_C)
#include "mbedtls/ssl_ticket.h"
#endif

#if defined(MBEDTLS_SSL_COOKIE_C)
#include "mbedtls/ssl_cookie.h"
#endif

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
#include "mbedtls/memory_buffer_alloc.h"
#endif

#if defined(MBEDTLS_SSL_SERVER_NAME_INDICATION) && defined(MBEDTLS_FS_IO)
#define SNI_OPTION
#endif

#if defined(_WIN32)
#include <windows.h>
#endif



/* Size of memory to be allocated for the heap, when using the library's memory
 * management and MBEDTLS_MEMORY_BUFFER_ALLOC_C is enabled. */
#define MEMORY_HEAP_SIZE        120000

#define DFL_SERVER_ADDR         NULL
#define DFL_SERVER_PORT         "4433"
#define DFL_RESPONSE_SIZE       -1
#define DFL_DEBUG_LEVEL         0
#define DFL_NBIO                0
#define DFL_EVENT               0
#define DFL_READ_TIMEOUT        0
#define DFL_CA_FILE             ""
#define DFL_CA_PATH             ""
#define DFL_CRT_FILE            ""
#define DFL_KEY_FILE            ""
#define DFL_CRT_FILE2           ""
#define DFL_KEY_FILE2           ""
#define DFL_ASYNC_OPERATIONS    "-"
#define DFL_ASYNC_PRIVATE_DELAY1 ( -1 )
#define DFL_ASYNC_PRIVATE_DELAY2 ( -1 )
#define DFL_ASYNC_PRIVATE_ERROR  ( 0 )
#define DFL_PSK                 ""
#define DFL_PSK_IDENTITY        "Client_identity"
#define DFL_ECJPAKE_PW          NULL
#define DFL_PSK_LIST            NULL
#define DFL_FORCE_CIPHER        0
#define DFL_VERSION_SUITES      NULL
#define DFL_RENEGOTIATION       MBEDTLS_SSL_RENEGOTIATION_DISABLED
#define DFL_ALLOW_LEGACY        -2
#define DFL_RENEGOTIATE         0
#define DFL_RENEGO_DELAY        -2
#define DFL_RENEGO_PERIOD       ( (uint64_t)-1 )
#define DFL_EXCHANGES           1
#define DFL_MIN_VERSION         -1
#define DFL_MAX_VERSION         -1
#define DFL_ARC4                -1
#define DFL_SHA1                -1
#define DFL_AUTH_MODE           -1
#define DFL_CERT_REQ_CA_LIST    MBEDTLS_SSL_CERT_REQ_CA_LIST_ENABLED
#define DFL_MFL_CODE            MBEDTLS_SSL_MAX_FRAG_LEN_NONE
#define DFL_TRUNC_HMAC          -1
#define DFL_TICKETS             MBEDTLS_SSL_SESSION_TICKETS_ENABLED
#define DFL_TICKET_TIMEOUT      86400
#define DFL_CACHE_MAX           -1
#define DFL_CACHE_TIMEOUT       -1
#define DFL_SNI                 NULL
#define DFL_ALPN_STRING         NULL
#define DFL_CURVES              NULL
#define DFL_DHM_FILE            NULL
#define DFL_TRANSPORT           MBEDTLS_SSL_TRANSPORT_STREAM
#define DFL_COOKIES             1
#define DFL_ANTI_REPLAY         -1
#define DFL_HS_TO_MIN           0
#define DFL_HS_TO_MAX           0
#define DFL_DTLS_MTU            -1
#define DFL_BADMAC_LIMIT        -1
#define DFL_DGRAM_PACKING        1
#define DFL_EXTENDED_MS         -1
#define DFL_ETM                 -1

#endif




#define WEB_SERVER CONFIG_LWIP_LOCAL_HOSTNAME
#define DFL_EVENT 0



#define HTTP_RESPONSE \
"%s %s\r\n" \
"Content-Type: %s\r\n\r\n" \
"%s\r\n"


/* // #define HTTP_RESPONSE \
// "HTTP/1.1 200 OK\r\n" \
// "Content-Type: text/html\r\n\r\n" \
// "%s\r\n" */


static const char* HTTPSD_TAG = "epaper-idf-httpsd";

esp_event_loop_handle_t epaper_idf_httpsd_event_loop_handle;

ESP_EVENT_DEFINE_BASE(EPAPER_IDF_HTTPSD_EVENT);

static unsigned char buf[MBEDTLS_EXAMPLE_RECV_BUF_LEN];

static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;
static mbedtls_ssl_context ssl;
static mbedtls_ssl_config conf;
static mbedtls_x509_crt srvcert;
static mbedtls_pk_context pkey;

#if defined(MBEDTLS_SSL_CACHE_C)
static mbedtls_ssl_cache_context cache;
#endif



// static char buf[1024];


#ifndef EPAPER_IDF_COMPONENT_INIT_FS_DEFINED
// // TODO: move these to the task: http-slideshow.cpp
static bool fs_initialized = false;
#endif


static bool epaper_idf_http_is_init = false;
// static mbedtls_net_context server_fd;
// static mbedtls_entropy_context entropy;
// static mbedtls_ctr_drbg_context ctr_drbg;
// static mbedtls_ssl_context ssl;
// static mbedtls_x509_crt cacert;
// static mbedtls_ssl_config conf;

// static char recv_buf[OPENSSL_EXAMPLE_RECV_BUF_LEN];

static const char* base_path = CONFIG_EXAMPLE_WEB_MOUNT_POINT;

/* #define OPENSSL_EXAMPLE_SERVER_ACK "HTTP/1.1 200 OK\r\n" \
                                "Content-Type: text/html\r\n" \
                                "Content-Length: 106\r\n\r\n" \
                                "<html>\r\n" \
                                "<head>\r\n" \
                                "<title>OpenSSL example</title></head><body>\r\n" \
                                "OpenSSL server example!\r\n" \
                                "</body>\r\n" \
                                "</html>\r\n" \
                                "\r\n" */


#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (MBEDTLS_EXAMPLE_TASK_STACK_WORDS)
// #define SCRATCH_BUFSIZE (5240)
// #define SCRATCH_BUFSIZE (10240)
// #define SCRATCH_BUFSIZE (40240)


#define REST_CHECK(a, str, goto_tag, ...)	\
do                                      \
{                                       \
	if (!(a))                             \
	{                                     \
		ESP_LOGE(HTTPSD_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__);	\
		goto goto_tag;	\
	}	\
} while (0)


typedef struct rest_server_context
{
  // SSL *ssl;
	char base_path[ESP_VFS_PATH_MAX + 1];
	char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

#if CONFIG_EXAMPLE_WEB_DEPLOY_SEMIHOST
#ifndef EPAPER_IDF_COMPONENT_INIT_FS_DEFINED
#define EPAPER_IDF_COMPONENT_INIT_FS_DEFINED
esp_err_t init_fs(void)
{
	if (fs_initialized) {
		ESP_LOGI(HTTPSD_TAG, "fs is already initialized: no-op");
		return ESP_OK;
	}

	esp_err_t res = esp_vfs_semihost_register(CONFIG_EXAMPLE_WEB_MOUNT_POINT, CONFIG_EXAMPLE_HOST_PATH_TO_MOUNT);
	if (res != ESP_OK)
	{
		ESP_LOGE(HTTPSD_TAG, "Failed to register semihost driver (%s)!", esp_err_to_name(res));
		return ESP_FAIL;
	}

	fs_initialized = true;

	return ESP_OK;
}
#endif  /** #endif EPAPER_IDF_COMPONENT_INIT_FS_DEFINED */
#endif

#if CONFIG_EXAMPLE_WEB_DEPLOY_SD
#ifndef EPAPER_IDF_COMPONENT_INIT_FS_DEFINED
#define EPAPER_IDF_COMPONENT_INIT_FS_DEFINED
esp_err_t init_fs(void)
{
	if (fs_initialized) {
		ESP_LOGI(HTTPSD_TAG, "fs is already initialized: no-op");
		return ESP_OK;
	}

	sdmmc_host_t host = SDMMC_HOST_DEFAULT();
	sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

	gpio_set_pull_mode(15, GPIO_PULLUP_ONLY); // CMD
	gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);  // D0
	gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);  // D1
	gpio_set_pull_mode(12, GPIO_PULLUP_ONLY); // D2
	gpio_set_pull_mode(13, GPIO_PULLUP_ONLY); // D3

	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
		.format_if_mount_failed = true,
    .max_files = 10,
		// .max_files = 4,
		.allocation_unit_size = 16 * 1024};

	sdmmc_card_t *card;
	esp_err_t res = esp_vfs_fat_sdmmc_mount(CONFIG_EXAMPLE_WEB_MOUNT_POINT, &host, &slot_config, &mount_config, &card);
	if (res != ESP_OK)
	{
		if (res == ESP_FAIL)
		{
			ESP_LOGE(HTTPSD_TAG, "Failed to mount filesystem.");
		}
		else
		{
			ESP_LOGE(HTTPSD_TAG, "Failed to initialize the card (%s)", esp_err_to_name(res));
		}
		return ESP_FAIL;
	}
	/* print card info if mount successfully */
	sdmmc_card_print_info(stdout, card);

	fs_initialized = true;

	return ESP_OK;
}
#endif  /** #endif EPAPER_IDF_COMPONENT_INIT_FS_DEFINED */
#endif

#if CONFIG_EXAMPLE_WEB_DEPLOY_SF
#ifndef EPAPER_IDF_COMPONENT_INIT_FS_DEFINED
#define EPAPER_IDF_COMPONENT_INIT_FS_DEFINED
esp_err_t init_fs(void)
{
	if (fs_initialized) {
		ESP_LOGI(HTTPSD_TAG, "fs is already initialized: no-op");
		return ESP_OK;
	}

	esp_vfs_spiffs_conf_t conf = {
		.base_path = CONFIG_EXAMPLE_WEB_MOUNT_POINT,
		.partition_label = NULL,
		.max_files = 10,
		// .max_files = 5,
		.format_if_mount_failed = false};
	esp_err_t res = esp_vfs_spiffs_register(&conf);

	if (res != ESP_OK)
	{
		if (res == ESP_FAIL)
		{
			ESP_LOGE(HTTPSD_TAG, "Failed to mount or format filesystem");
		}
		else if (res == ESP_ERR_NOT_FOUND)
		{
			ESP_LOGE(HTTPSD_TAG, "Failed to find SPIFFS partition");
		}
		else
		{
			ESP_LOGE(HTTPSD_TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(res));
		}
		return ESP_FAIL;
	}

	size_t total = 0, used = 0;
	res = esp_spiffs_info(NULL, &total, &used);
	if (res != ESP_OK)
	{
		ESP_LOGE(HTTPSD_TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(res));
	}
	else
	{
		ESP_LOGI(HTTPSD_TAG, "Partition size: total: %d, used: %d", total, used);
	}

	fs_initialized = true;

	return ESP_OK;
}
#endif  /** #endif EPAPER_IDF_COMPONENT_INIT_FS_DEFINED */
#endif  /** #endif CONFIG_EXAMPLE_WEB_DEPLOY_SEMIHOST */



/** Return true if \p ret is a status code indicating that there is an
 * operation in progress on an SSL connection, and false if it indicates
 * success or a fatal error.
 *
 * The possible operations in progress are:
 *
 * - A read, when the SSL input buffer does not contain a full message.
 * - A write, when the SSL output buffer contains some data that has not
 *   been sent over the network yet.
 * - An asynchronous callback that has not completed yet. */
static int mbedtls_status_is_ssl_in_progress(int ret)
{
    return(ret == MBEDTLS_ERR_SSL_WANT_READ ||
            ret == MBEDTLS_ERR_SSL_WANT_WRITE ||
            ret == MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS);
}




/*
 * Wait for an event from the underlying transport or the timer
 * (Used in event-driven IO mode).
 */
#if !defined(MBEDTLS_TIMING_C)
int idle( mbedtls_net_context *fd,
          int idle_reason )
#else
int idle( mbedtls_net_context *fd,
          mbedtls_timing_delay_context *timer,
          int idle_reason )
#endif
{
    int ret;
    int poll_type = 0;

    if( idle_reason == MBEDTLS_ERR_SSL_WANT_WRITE )
        poll_type = MBEDTLS_NET_POLL_WRITE;
    else if( idle_reason == MBEDTLS_ERR_SSL_WANT_READ )
        poll_type = MBEDTLS_NET_POLL_READ;
#if !defined(MBEDTLS_TIMING_C)
    else
        return( 0 );
#endif

    while( 1 )
    {
        /* Check if timer has expired */
#if defined(MBEDTLS_TIMING_C)                                                                                        
        if( timer != NULL &&
            mbedtls_timing_get_delay( timer ) == 2 )
        {
            break;
        }
#endif /* MBEDTLS_TIMING_C */

        /* Check if underlying transport became available */
        if( poll_type != 0 )
        {
            ret = mbedtls_net_poll( fd, poll_type, 0 );
            if( ret < 0 )
                return( ret );
            if( ret == poll_type )
                break;
        }
    }

    return( 0 );
}




/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
	const char *type = "text/plain";
	if (CHECK_FILE_EXTENSION(filepath, ".html"))
	{
		type = "text/html";
	}
	else if (CHECK_FILE_EXTENSION(filepath, ".js"))
	{
		type = "application/javascript";
	}
	else if (CHECK_FILE_EXTENSION(filepath, ".css"))
	{
		type = "text/css";
	}
	else if (CHECK_FILE_EXTENSION(filepath, ".png"))
	{
		type = "image/png";
	}
	else if (CHECK_FILE_EXTENSION(filepath, ".ico"))
	{
		type = "image/x-icon";
	}
	else if (CHECK_FILE_EXTENSION(filepath, ".svg"))
	{
		type = "text/xml";
	}
	return httpd_resp_set_type(req, type);
}

/* Send HTTP response with the contents of the requested file */
// static esp_err_t rest_common_get_handler(httpd_req_t *req)
// static esp_err_t rest_common_get_handler(httpd_req_t *req)
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{
  // char recv_buf[OPENSSL_EXAMPLE_RECV_BUF_LEN];

  // memset(recv_buf, 0, OPENSSL_EXAMPLE_RECV_BUF_LEN);
  // res = SSL_read(ssl, recv_buf, OPENSSL_EXAMPLE_RECV_BUF_LEN - 1);
  // if (res <= 0) {
  //     break;
  // }
  // ESP_LOGI(HTTPSD_TAG, "SSL read: %s", recv_buf);
  // if (strstr(recv_buf, "GET ") &&
  //     strstr(recv_buf, " HTTP/1.1")) {
  //     ESP_LOGI(HTTPSD_TAG, "SSL get matched message");
  //     ESP_LOGI(HTTPSD_TAG, "SSL write message");
  //     res = SSL_write(ssl, send_data, send_bytes);
  //     if (res > 0) {
  //         ESP_LOGI(HTTPSD_TAG, "OK");
  //     } else {
  //         ESP_LOGI(HTTPSD_TAG, "error");
  //     }
  //     break;
  // }



  ESP_LOGI(HTTPSD_TAG, "rest_common_get_handler called");



	size_t req_hdr_host_len = httpd_req_get_hdr_value_len(req, "host");

	char req_hdr_host_val[req_hdr_host_len];

  char * req_hdr_host_val_ptr = (char*)calloc(req_hdr_host_len, sizeof(char));

  

  // esp_err_t res = httpd_req_get_hdr_value_str(req, "host", &req_hdr_host_val, sizeof(req_hdr_host_val));
	esp_err_t res = httpd_req_get_hdr_value_str(req, "host", req_hdr_host_val_ptr, sizeof(char) * req_hdr_host_len);
	if (res != ESP_OK) {
		ESP_LOGE(HTTPSD_TAG, "failed getting HOST header value: %d", res);

		switch (res)
		{
		case ESP_ERR_NOT_FOUND:
			ESP_LOGE(HTTPSD_TAG, "failed getting HOST header value: ESP_ERR_NOT_FOUND: Key not found: %d", res);
			break;

		case ESP_ERR_INVALID_ARG:
			ESP_LOGE(HTTPSD_TAG, "failed getting HOST header value: ESP_ERR_INVALID_ARG: Null arguments: %d", res);
			break;

		case ESP_ERR_HTTPD_INVALID_REQ:
			ESP_LOGE(HTTPSD_TAG, "failed getting HOST header value: ESP_ERR_HTTPD_INVALID_REQ: Invalid HTTP request pointer: %d", res);
			break;

		case ESP_ERR_HTTPD_RESULT_TRUNC:
			ESP_LOGE(HTTPSD_TAG, "failed getting HOST header value: ESP_ERR_HTTPD_RESULT_TRUNC: Value string truncated: %d", res);
			break;
		
		default:
			break;
		}
	}

	ESP_LOGI(HTTPSD_TAG, "Got HOST header value: %s", req_hdr_host_val);


	const char redir_trigger_host[] = "connectivitycheck.gstatic.com";

	if (strncmp(req_hdr_host_val, redir_trigger_host, strlen(redir_trigger_host)) == 0) {
		const char resp[] = "302 Found";
		
		ESP_LOGI(HTTPSD_TAG, "Detected redirect trigger HOST: %s", redir_trigger_host);
		
		httpd_resp_set_status(req, resp);

    

		// TODO: There is a value "CONFIG_LWIP_LOCAL_HOSTNAME" in the Kconfig 
		// menu which isn't being used here but it should be.
		httpd_resp_set_hdr(req, "Location", "https://epaper");

		httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

	} else {
		// const char resp[] = HTTPD_200;

		ESP_LOGI(HTTPSD_TAG, "No redirect needed for HOST: %s", req_hdr_host_val);
		
		// httpd_resp_set_status(req, HTTPD_200);
		// httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
		
		// httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

		





		rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;

		char filepath[FILE_PATH_MAX];

		// Find the start of any query parameters and remove them.
		uint endpos = strlen(req->uri);

		char *s = NULL;
		s = strstr(req->uri, "?");

		if (s != NULL) {
			endpos = s - req->uri;
		}

		char req_uri[endpos + 1];
		strlcpy(req_uri, req->uri, sizeof(req_uri));

		strlcpy(filepath, rest_context->base_path, sizeof(filepath));
		if (req_uri[strlen(req_uri) - 1] == '/')
		{
			strlcat(filepath, "/index.html", sizeof(filepath));
		}
		else
		{
			strlcat(filepath, req_uri, sizeof(filepath));
		}

		int fd = open(filepath, O_RDONLY, 0);
		if (fd == -1)
		{
			ESP_LOGE(HTTPSD_TAG, "Failed to open file : %s", filepath);

			memset((void *)filepath, 0, sizeof(filepath));

			strlcpy(filepath, rest_context->base_path, sizeof(filepath));

			strlcat(filepath, "/index.html", sizeof(filepath));

			fd = open(filepath, O_RDONLY, 0);
			if (fd == -1)
			{
				ESP_LOGE(HTTPSD_TAG, "Failed to open file : %s", filepath);

				/* Respond with 500 Internal Server Error */
				httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");

				free(req_hdr_host_val_ptr);

        return ESP_FAIL;
			}
		}

		set_content_type_from_file(req, filepath);

		char *chunk = rest_context->scratch;
		ssize_t read_bytes;
		do
		{
			/* Read file in chunks into the scratch buffer */
			read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
			if (read_bytes == -1)
			{
				ESP_LOGE(HTTPSD_TAG, "Failed to read file : %s", filepath);
			}
			else if (read_bytes > 0)
			{
				/* Send the buffer contents as HTTP response chunk */
				if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK)
				{
					close(fd);
					ESP_LOGE(HTTPSD_TAG, "File sending failed!");
					/* Abort sending file */
					httpd_resp_sendstr_chunk(req, NULL);
					/* Respond with 500 Internal Server Error */
					httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
					
          free(req_hdr_host_val_ptr);
          
          return ESP_FAIL;
				}
			}
		} while (read_bytes > 0);
		/* Close file after sending complete */
		close(fd);
		ESP_LOGI(HTTPSD_TAG, "File sending complete");
		/* Respond with an empty chunk to signal HTTP response completion */
		httpd_resp_send_chunk(req, NULL, 0);
	}


	// rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;

	// char filepath[FILE_PATH_MAX];

	// // Find the start of any query parameters and remove them.
	// uint endpos = strlen(req->uri);

	// char *s = NULL;
	// s = strstr(req->uri, "?");

	// if (s != NULL) {
	// 	endpos = s - req->uri;
	// }

	// char req_uri[endpos + 1];
	// strlcpy(req_uri, req->uri, sizeof(req_uri));

	// strlcpy(filepath, rest_context->base_path, sizeof(filepath));
	// if (req_uri[strlen(req_uri) - 1] == '/')
	// {
	// 	strlcat(filepath, "/index.html", sizeof(filepath));
	// }
	// else
	// {
	// 	strlcat(filepath, req_uri, sizeof(filepath));
	// }

	// int fd = open(filepath, O_RDONLY, 0);
	// if (fd == -1)
	// {
	// 	ESP_LOGE(HTTPSD_TAG, "Failed to open file : %s", filepath);

	// 	memset((void *)filepath, 0, sizeof(filepath));

	// 	strlcpy(filepath, rest_context->base_path, sizeof(filepath));

	// 	strlcat(filepath, "/index.html", sizeof(filepath));

	// 	fd = open(filepath, O_RDONLY, 0);
	// 	if (fd == -1)
	// 	{
	// 		ESP_LOGE(HTTPSD_TAG, "Failed to open file : %s", filepath);

	// 		/* Respond with 500 Internal Server Error */
	// 		httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
	// 		return ESP_FAIL;
	// 	}
	// }

	// set_content_type_from_file(req, filepath);

	// char *chunk = rest_context->scratch;
	// ssize_t read_bytes;
	// do
	// {
	// 	/* Read file in chunks into the scratch buffer */
	// 	read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
	// 	if (read_bytes == -1)
	// 	{
	// 		ESP_LOGE(HTTPSD_TAG, "Failed to read file : %s", filepath);
	// 	}
	// 	else if (read_bytes > 0)
	// 	{
	// 		/* Send the buffer contents as HTTP response chunk */
	// 		if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK)
	// 		{
	// 			close(fd);
	// 			ESP_LOGE(HTTPSD_TAG, "File sending failed!");
	// 			/* Abort sending file */
	// 			httpd_resp_sendstr_chunk(req, NULL);
	// 			/* Respond with 500 Internal Server Error */
	// 			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
	// 			return ESP_FAIL;
	// 		}
	// 	}
	// } while (read_bytes > 0);
	// /* Close file after sending complete */
	// close(fd);
	// ESP_LOGI(HTTPSD_TAG, "File sending complete");
	// /* Respond with an empty chunk to signal HTTP response completion */
	// httpd_resp_send_chunk(req, NULL, 0);

  free(req_hdr_host_val_ptr);

	return ESP_OK;
}

// /* System info GET handler */
// static esp_err_t system_info_get_handler(httpd_req_t *req)
// {
// 	httpd_resp_set_type(req, "application/json");
// 	cJSON *root = cJSON_CreateObject();
// 	esp_chip_info_t chip_info;
// 	esp_chip_info(&chip_info);
// 	cJSON_AddStringToObject(root, "version", IDF_VER);
// 	cJSON_AddNumberToObject(root, "cores", chip_info.cores);
// 	const char *sys_info = cJSON_Print(root);

// 	// httpd_resp_send(req, sys_info, -1);
// 	httpd_resp_sendstr(req, sys_info);

// 	free((void *)sys_info);
// 	cJSON_Delete(root);
// 	return ESP_OK;
// }

static esp_err_t util_restart_post_handler(httpd_req_t *req)
{
	httpd_resp_set_type(req, "application/json");

	cJSON *root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "msg", "success");
	cJSON_AddNumberToObject(root, "statusCode", 200);

	const char *sys_info = cJSON_Print(root);
	
	// httpd_resp_send(req, sys_info, -1);
	httpd_resp_sendstr(req, sys_info);

	free((void *)sys_info);
	cJSON_Delete(root);

	esp_restart();

	return ESP_OK;
}

static void start_httpsd(void* pvParameter)
{
  esp_err_t res = ESP_OK;


  // TODO: we probably should do this here also
	ESP_ERROR_CHECK(init_fs());




  // static httpd_handle_t server = NULL;
	
    // Configure the HTTPS server.
    // httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();

    extern const unsigned char cacert_pem_start[] asm("_binary_ca_cert_conf_pem_start");
    extern const unsigned char cacert_pem_end[]   asm("_binary_ca_cert_conf_pem_end");
    const unsigned int cacert_pem_bytes = cacert_pem_end - cacert_pem_start;

    extern const unsigned char prvtkey_pem_start[] asm("_binary_ca_key_conf_pem_start");
    extern const unsigned char prvtkey_pem_end[]   asm("_binary_ca_key_conf_pem_end");
    const unsigned int prvtkey_pem_bytes = prvtkey_pem_end - prvtkey_pem_start;


    // // config.httpd.stack_size = SCRATCH_BUFSIZE;

    // // config.httpd.max_open_sockets = 4;
    // // config.httpd.max_uri_handlers = 16;
    // // config.httpd.max_resp_headers = 16;

    // // config.httpd.recv_wait_timeout = 60;
    // // config.httpd.send_wait_timeout = 60;

    // config.httpd.uri_match_fn = httpd_uri_match_wildcard;
    
    // // config.httpd.lru_purge_enable = true;

    // // TODO: Choose this in Kconfig menu.
    // config.transport_mode = HTTPD_SSL_TRANSPORT_SECURE;
    // // config.transport_mode = HTTPD_SSL_TRANSPORT_INSECURE;

    // // TODO: Choose this in Kconfig menu.
    // config.port_secure = 443;

    // // TODO: Choose this in Kconfig menu.
    // config.port_insecure = 80;

    // config.cacert_pem = cacert_pem_start;
    // config.cacert_len = cacert_pem_end - cacert_pem_start;

    // config.prvtkey_pem = prvtkey_pem_start;
    // config.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;

    // // // TODO: Support optional client verify certificate.
    // // config.client_verify_cert_pem = client_verify_cert_pem_start;
    // // config.client_verify_cert_len = client_verify_cert_pem_bytes;



#if defined(MBEDTLS_TIMING_C)
    mbedtls_timing_delay_context timer;
#endif


    mbedtls_net_context listen_fd, client_fd;
    int ret = 0, len, written, frags, exchanges_left;


  if (!epaper_idf_http_is_init) {

	

    // extern const unsigned char cacert_pem_start[] asm("_binary_cacert_pem_start");
    // extern const unsigned char cacert_pem_end[]   asm("_binary_cacert_pem_end");
    // const unsigned int cacert_pem_bytes = cacert_pem_end - cacert_pem_start;

    // extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
    // extern const unsigned char prvtkey_pem_end[]   asm("_binary_prvtkey_pem_end");
    // const unsigned int prvtkey_pem_bytes = prvtkey_pem_end - prvtkey_pem_start;

    mbedtls_net_init( &listen_fd );
    mbedtls_net_init( &client_fd );
    ESP_LOGI(HTTPSD_TAG, "SSL server context create ......");
    mbedtls_ssl_init( &ssl );
    ESP_LOGI(HTTPSD_TAG, "OK");
    mbedtls_ssl_config_init( &conf );
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_init( &cache );
#endif
    mbedtls_x509_crt_init( &srvcert );
    mbedtls_pk_init( &pkey );
    mbedtls_entropy_init( &entropy );
    mbedtls_ctr_drbg_init( &ctr_drbg );

    /*
	 * 1. Load the certificates and private RSA key
	 */
	mbedtls_printf( "\n  . Loading the server cert. and key..." );

	/** TODO: This demonstration program uses embedded test certificates.
	 * Instead, you may want to use mbedtls_x509_crt_parse_file() to read the
	 * server and CA certificates, as well as mbedtls_pk_parse_keyfile().
	 */
	ESP_LOGI(HTTPSD_TAG, "SSL server context set own certification......");
	ESP_LOGI(HTTPSD_TAG, "Parsing test srv_crt......");
	ret = mbedtls_x509_crt_parse( &srvcert, (const unsigned char *) cacert_pem_start,
						cacert_pem_bytes );
	if( ret != 0 )
	{
		ESP_LOGI(HTTPSD_TAG, " failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", ret );
		goto exit;
	}
    ESP_LOGI(HTTPSD_TAG, "OK");

	ESP_LOGI(HTTPSD_TAG, "SSL server context set private key......");
    ret =  mbedtls_pk_parse_key( &pkey, (const unsigned char *) prvtkey_pem_start,
    						prvtkey_pem_bytes, NULL, 0 );
	if( ret != 0 )
	{
		ESP_LOGI(HTTPSD_TAG, " failed\n  !  mbedtls_pk_parse_key returned %d\n\n", ret );
		goto exit;
	}
    ESP_LOGI(HTTPSD_TAG, "OK");

    /*
	 * 2. Setup the listening TCP socket
	 */
	ESP_LOGI(HTTPSD_TAG, "SSL server socket bind at localhost:443 ......");
	if( ( ret = mbedtls_net_bind( &listen_fd, NULL, "443", MBEDTLS_NET_PROTO_TCP ) ) != 0 )
	{
		ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_net_bind returned %d\n\n", ret );
		goto exit;
	}
	ESP_LOGI(HTTPSD_TAG, "OK");

	/*
	 * 3. Seed the RNG
	 */
	ESP_LOGI(HTTPSD_TAG, "  . Seeding the random number generator..." );
	if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
							   (const unsigned char *) HTTPSD_TAG,
							   strlen( HTTPSD_TAG ) ) ) != 0 )
	{
		ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
		goto exit;
	}
	ESP_LOGI(HTTPSD_TAG, "OK");

    /*
     * 4. Setup stuff
     */
	ESP_LOGI(HTTPSD_TAG, "  . Setting up the SSL data...." );
#ifdef CONFIG_MBEDTLS_DEBUG
    mbedtls_esp_enable_debug_log(&conf, 5);
#endif
    if( ( ret = mbedtls_ssl_config_defaults( &conf,
                    MBEDTLS_SSL_IS_SERVER,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
    	ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );

#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_conf_session_cache( &conf, &cache,
                                   mbedtls_ssl_cache_get,
                                   mbedtls_ssl_cache_set );
#endif

    mbedtls_ssl_conf_ca_chain( &conf, srvcert.next, NULL );
    if( ( ret = mbedtls_ssl_conf_own_cert( &conf, &srvcert, &pkey ) ) != 0 )
    {
    	ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n\n", ret );
        goto exit;
    }

    if( ( ret = mbedtls_ssl_setup( &ssl, &conf ) ) != 0 )
    {
    	ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret );
        goto exit;
    }


#if defined(MBEDTLS_TIMING_C)
    mbedtls_ssl_set_timer_cb( &ssl, &timer, mbedtls_timing_set_delay,
                                            mbedtls_timing_get_delay );
#endif


	ESP_LOGI(HTTPSD_TAG, "OK");

reset:
#ifdef MBEDTLS_ERROR_C
	if( ret != 0 )
	{
		char error_buf[200];
		mbedtls_strerror( ret, error_buf, 200 );
		ESP_LOGI(HTTPSD_TAG, "Last error was: %d - %s\n\n", ret, error_buf );
	}
#endif

	mbedtls_net_free( &client_fd );

	mbedtls_ssl_session_reset( &ssl );

	









// 		// TODO: Add this to Kconfig menu.
// 		conf.read_timeout = 10000;

// 		mbedtls_ssl_init(&ssl);
//     mbedtls_x509_crt_init(&cacert);;
// 		mbedtls_ctr_drbg_init(&ctr_drbg);
// 		ESP_LOGI(HTTPSD_TAG, "Seeding the random number generator");

// 		mbedtls_ssl_config_init(&conf);

// 		mbedtls_entropy_init(&entropy);
// 		if ((res = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
// 																		NULL, 0)) != 0)
// 		{
// 			ESP_LOGE(HTTPSD_TAG, "mbedtls_ctr_drbg_seed returned %d", res);
// 			abort();
// 		}

// 		ESP_LOGI(HTTPSD_TAG, "Attaching the certificate bundle...");

// 		res = esp_crt_bundle_attach(&conf);

// 		if (res < 0)
// 		{
// 			ESP_LOGE(HTTPSD_TAG, "esp_crt_bundle_attach returned -0x%x\n\n", -res);
// 			abort();
// 		}

// 		ESP_LOGI(HTTPSD_TAG, "Setting hostname for TLS session...");

// 		/* Hostname set here should match CN in server certificate */
// 		if ((res = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
// 		{
// 			ESP_LOGE(HTTPSD_TAG, "mbedtls_ssl_set_hostname returned -0x%x", -res);
// 			abort();
// 		}

// 		ESP_LOGI(HTTPSD_TAG, "Setting up the SSL/TLS structure...");

    

// 		if ((res = mbedtls_ssl_config_defaults(&conf,
//                                           MBEDTLS_SSL_IS_SERVER,
// 																					MBEDTLS_SSL_TRANSPORT_STREAM,
// 																					MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
// 		{
// 			ESP_LOGE(HTTPSD_TAG, "mbedtls_ssl_config_defaults returned %d", res);
// 			goto exit;
// 		}

// 		/* MBEDTLS_SSL_VERIFY_OPTIONAL is bad for security, in this example it will print
// 			a warning if CA verification fails but it will continue to connect.
// 			You should consider using MBEDTLS_SSL_VERIFY_REQUIRED in your own code.
// 		*/
// 		mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
// 		mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
// 		mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
// #ifdef CONFIG_MBEDTLS_DEBUG
// 		mbedtls_esp_enable_debug_log(&conf, CONFIG_MBEDTLS_DEBUG_LEVEL);
// #endif

// 		if ((res = mbedtls_ssl_setup(&ssl, &conf)) != 0)
// 		{
// 			ESP_LOGE(HTTPSD_TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -res);
// 			goto exit;
// 		}

		epaper_idf_http_is_init = true;
	}



  /*
	 * 3. Wait until a client connects
	 */
	ESP_LOGI(HTTPSD_TAG, "  . Waiting for a remote connection ..." );
	if( ( ret = mbedtls_net_accept( &listen_fd, &client_fd,
									NULL, 0, NULL ) ) != 0 )
	{
		ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_net_accept returned %d\n\n", ret );
		goto exit;
	}
	mbedtls_ssl_set_bio( &ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, NULL );
	ESP_LOGI(HTTPSD_TAG, "OK");

	/*
	 * 5. Handshake
	 */
	ESP_LOGI(HTTPSD_TAG, "  . Performing the SSL/TLS handshake..." );
	while( ( ret = mbedtls_ssl_handshake( &ssl ) ) != 0 )
	{
		if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
		{
			ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_ssl_handshake returned %d\n\n", ret );
			goto reset;
		}
	}
	ESP_LOGI(HTTPSD_TAG, "OK");

	/*
	 * 6. Read the HTTP Request
	 */
	ESP_LOGI(HTTPSD_TAG, "  < Read from client:" );
	do
	{
		len = sizeof( buf ) - 1;
		memset( buf, 0, sizeof( buf ) );
		ret = mbedtls_ssl_read( &ssl, buf, len );

		if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
			continue;

		if( ret <= 0 )
		{
			switch( ret )
			{
				case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
					ESP_LOGI(HTTPSD_TAG, " connection was closed gracefully\n" );
					break;

				case MBEDTLS_ERR_NET_CONN_RESET:
					ESP_LOGI(HTTPSD_TAG, " connection was reset by peer\n" );
					break;

				default:
					ESP_LOGI(HTTPSD_TAG, " mbedtls_ssl_read returned -0x%x\n", -ret );
					break;
			}

			break;
		}

		len = ret;
		ESP_LOGI(HTTPSD_TAG, " %d bytes read\n\n%s", len, (char *) buf );

		if( ret > 0 )
			break;
	}
	while( 1 );

  // TODO: Add these in Kconfig menu.
  static char* resp_proto = "HTTP/1.1";
  static char* resp_status = "200 OK";
  static char* resp_header_content_type = "text/html";

  static const char* resp_body = "<a href=\"https://"CONFIG_LWIP_LOCAL_HOSTNAME"\">https://"CONFIG_LWIP_LOCAL_HOSTNAME"</a>\0";


  /** NOTE: Example handler: "GET /[*]"
	 * 7. Write the 200 Response
	 */
	ESP_LOGI(HTTPSD_TAG, "  > Write to client:");

	len = snprintf(
    (char*)buf,
    strlen(HTTP_RESPONSE) * sizeof(char) +
    strlen(resp_proto) * sizeof(char) +
    strlen(resp_status) * sizeof(char) +
    strlen(resp_header_content_type) * sizeof(char) +
    strlen(resp_body) * sizeof(char),
    HTTP_RESPONSE,
    resp_proto,
    resp_status,
    resp_header_content_type,
    resp_body
  );



  written = 0;
  frags = 0;
	while(written < len)
	{
    while((ret = mbedtls_ssl_write(&ssl, buf + written, len - written)) <= 0)
    {
      if( ret == MBEDTLS_ERR_NET_CONN_RESET )
      {
          mbedtls_printf( " failed\n  ! peer closed the connection\n\n" );
          goto reset;
      }

      if( ! mbedtls_status_is_ssl_in_progress( ret ) )
      {
          mbedtls_printf( " failed\n  ! mbedtls_ssl_write returned %d\n\n", ret );
          goto reset;
      }

      /* For event-driven IO, wait for socket to become available */
      // if( opt.event == 1 /* level triggered IO */ )
      if( DFL_EVENT == 1 /* level triggered IO */ )
      {
#if defined(MBEDTLS_TIMING_C)
          idle( &client_fd, &timer, ret );
#else
          idle( &client_fd, ret );
#endif
      }

      // if(ret == MBEDTLS_ERR_NET_CONN_RESET)
      // {
      //   ESP_LOGI(HTTPSD_TAG, " failed\n  ! peer closed the connection\n\n" );
      //   goto reset;
      // }

      // if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
      // {
      //   ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_ssl_write returned %d\n\n", ret );
      //   goto exit;
      // }
    }

    written += ret;
    frags++;
	}

	len = ret;
	ESP_LOGI(HTTPSD_TAG, " %d bytes written\n\n%s\n", len, (char *) buf );

	ESP_LOGI(HTTPSD_TAG, "Closing the connection..." );

	while( ( ret = mbedtls_ssl_close_notify( &ssl ) ) < 0 )
	{
		if( ret != MBEDTLS_ERR_SSL_WANT_READ &&
			ret != MBEDTLS_ERR_SSL_WANT_WRITE )
		{
			ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_ssl_close_notify returned %d\n\n", ret );
			goto reset;
		}
	}
	ESP_LOGI(HTTPSD_TAG, "OK");

	ret = 0;
	goto reset;

exit:

#ifdef MBEDTLS_ERROR_C
	if( ret != 0 )
	{
		char error_buf[100];
		mbedtls_strerror( ret, error_buf, 100 );
		ESP_LOGI(HTTPSD_TAG,"Last error was: %d - %s\n\n", ret, error_buf );
	}
#endif

	mbedtls_net_free( &client_fd );
	mbedtls_net_free( &listen_fd );

	mbedtls_x509_crt_free( &srvcert );
	mbedtls_pk_free( &pkey );
	mbedtls_ssl_free( &ssl );
	mbedtls_ssl_config_free( &conf );
#if defined(MBEDTLS_SSL_CACHE_C)
	mbedtls_ssl_cache_free( &cache );
#endif
	mbedtls_ctr_drbg_free( &ctr_drbg );
	mbedtls_entropy_free( &entropy );

    ESP_LOGI(HTTPSD_TAG, "Closing Task");
	vTaskDelete(NULL);
	return ;














  // // HTTPS server
  // ESP_LOGI(HTTPSD_TAG, "Starting HTTPS Server...");

  // REST_CHECK(base_path, "wrong base path", err);
	
  // rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
	
  // REST_CHECK(rest_context, "No memory for rest context", err);
	
  // strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    
  //   // // TODO: Support optional client verify certificate.
  //   // extern const uint8_t client_verify_cert_pem_start[] asm("_binary_client_verify_cert_conf_pem_start");
  //   // extern const uint8_t client_verify_cert_pem_end[]   asm("_binary_client_verify_cert_conf_pem_end");
  //   // const size_t client_verify_cert_pem_bytes = client_verify_cert_pem_end - client_verify_cert_pem_start;


    


  //   REST_CHECK(httpd_ssl_start(&server, &config) == ESP_OK, "Start HTTPS server failed", err_start);
  //   ESP_LOGI(HTTPSD_TAG, "Started HTTPS Server.");


  //   ESP_LOGI(HTTPSD_TAG, "Registering HTTPS server URI handlers...");



  //   // /* URI handler for getting system info */
  //   // // static const httpd_uri_t system_info_get_uri = {
  //   // httpd_uri_t system_info_get_uri = {
  //   //   .uri = "/api/util/info",
  //   //   .method = HTTP_GET,
  //   //   .handler = system_info_get_handler,
  //   //   .user_ctx = rest_context,
  //   // };

  //   /* URI handler for restarting the device */
  //   httpd_uri_t util_restart_post_uri = {
  //         .uri = "/api/util/restart",
  //         .method = HTTP_POST,
  //         .handler = util_restart_post_handler,
  //         .user_ctx = rest_context,
  //   };

  //   /* URI handler for getting root server path (/index.html) */
  //   httpd_uri_t root_get_uri = {
  //         .uri = "/",
  //         .method = HTTP_GET,
  //         .handler = rest_common_get_handler,
  //         .user_ctx = rest_context,
  //   };

  //   /* URI handler for getting web server files */
  //   httpd_uri_t common_get_uri = {
  //         .uri = "/*",
  //         .method = HTTP_GET,
  //         .handler = rest_common_get_handler,
  //         .user_ctx = rest_context,
  //   };



    
    // // system_info_get_uri.user_ctx = rest_context;
    // res = httpd_register_uri_handler(server, &system_info_get_uri);
    // if (res != ESP_OK) {
    //   ESP_LOGE(HTTPSD_TAG, "Failed registering HTTPS server URI handler: %s: %d", "/api/util/info", res);
    //   goto err_start;
    // }

    
    
    // res = httpd_register_uri_handler(server, &util_restart_post_uri);	
    // if (res != ESP_OK) {
    //   ESP_LOGE(HTTPSD_TAG, "Failed registering HTTPS server URI handler: %s: %d", "POST /api/util/restart", res);
    //   goto err_start;
    // }

    
    // res = httpd_register_uri_handler(server, &root_get_uri);
    // if (res != ESP_OK) {
    //   ESP_LOGE(HTTPSD_TAG, "Failed registering HTTPS server URI handler: %s: %d", "GET /", res);
    //   goto err_start;
    // }
    
    
    // res = httpd_register_uri_handler(server, &common_get_uri);
    // if (res != ESP_OK) {
    //   ESP_LOGE(HTTPSD_TAG, "Failed registering HTTPS server URI handler: %s: %d", "GET /*", res);
    //   goto err_start;
    // }

    // ESP_LOGI(HTTPSD_TAG, "Registered HTTPS server URI handlers.");




//     ESP_LOGI(HTTPSD_TAG, "SSL server context create ......");
//     /* For security reasons, it is best if you can use
//        TLSv1_2_server_method() here instead of TLS_server_method().
//        However some old browsers may not support TLS v1.2.
//     */
//     ctx = SSL_CTX_new(TLS_server_method());
//     if (!ctx) {
//         ESP_LOGI(HTTPSD_TAG, "failed");
//         goto failed1;
//     }
//     ESP_LOGI(HTTPSD_TAG, "OK");

//     ESP_LOGI(HTTPSD_TAG, "SSL server context set own certification......");
//     res = SSL_CTX_use_certificate_ASN1(ctx, cacert_pem_bytes, cacert_pem_start);
//     if (!res) {
//         ESP_LOGI(HTTPSD_TAG, "failed");
//         goto failed2;
//     }
//     ESP_LOGI(HTTPSD_TAG, "OK");

//     ESP_LOGI(HTTPSD_TAG, "SSL server context set private key......");
//     res = SSL_CTX_use_PrivateKey_ASN1(0, ctx, prvtkey_pem_start, prvtkey_pem_bytes);
//     if (!res) {
//         ESP_LOGI(HTTPSD_TAG, "failed");
//         goto failed2;
//     }
//     ESP_LOGI(HTTPSD_TAG, "OK");

//     ESP_LOGI(HTTPSD_TAG, "SSL server create socket ......");
//     sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     if (sockfd < 0) {
//         ESP_LOGI(HTTPSD_TAG, "failed");
//         goto failed2;
//     }
//     ESP_LOGI(HTTPSD_TAG, "OK");

//     ESP_LOGI(HTTPSD_TAG, "SSL server socket bind ......");
//     memset(&sock_addr, 0, sizeof(sock_addr));
//     sock_addr.sin_family = AF_INET;
//     sock_addr.sin_addr.s_addr = 0;
//     sock_addr.sin_port = htons(OPENSSL_EXAMPLE_LOCAL_TCP_PORT);
//     res = bind(sockfd, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
//     if (res) {
//         ESP_LOGI(HTTPSD_TAG, "failed");
//         goto failed3;
//     }
//     ESP_LOGI(HTTPSD_TAG, "OK");

//     ESP_LOGI(HTTPSD_TAG, "SSL server socket listen ......");
//     res = listen(sockfd, 32);
//     if (res) {
//         ESP_LOGI(HTTPSD_TAG, "failed");
//         goto failed3;
//     }
//     ESP_LOGI(HTTPSD_TAG, "OK");

// reconnect:
//     ESP_LOGI(HTTPSD_TAG, "SSL server create ......");
//     ssl = SSL_new(ctx);
//     if (!ssl) {
//         ESP_LOGI(HTTPSD_TAG, "failed");
//         goto failed3;
//     }
//     ESP_LOGI(HTTPSD_TAG, "OK");

//     ESP_LOGI(HTTPSD_TAG, "SSL server socket accept client ......");
//     new_sockfd = accept(sockfd, (struct sockaddr *)&sock_addr, &addr_len);
//     if (new_sockfd < 0) {
//         ESP_LOGI(HTTPSD_TAG, "failed" );
//         goto failed4;
//     }
//     ESP_LOGI(HTTPSD_TAG, "OK");

//     SSL_set_fd(ssl, new_sockfd);

//     ESP_LOGI(HTTPSD_TAG, "SSL server accept client ......");
//     res = SSL_accept(ssl);
//     if (!res) {
//         ESP_LOGI(HTTPSD_TAG, "failed");
//         goto failed5;
//     }
//     ESP_LOGI(HTTPSD_TAG, "OK");

//     ESP_LOGI(HTTPSD_TAG, "SSL server read message ......");
//     do {
//       // Handle the HTTPS client request.
//       rest_common_get_handler();


//         // memset(recv_buf, 0, OPENSSL_EXAMPLE_RECV_BUF_LEN);
//         // res = SSL_read(ssl, recv_buf, OPENSSL_EXAMPLE_RECV_BUF_LEN - 1);
//         // if (res <= 0) {
//         //     break;
//         // }
//         // ESP_LOGI(HTTPSD_TAG, "SSL read: %s", recv_buf);
//         // if (strstr(recv_buf, "GET ") &&
//         //     strstr(recv_buf, " HTTP/1.1")) {
//         //     ESP_LOGI(HTTPSD_TAG, "SSL get matched message");
//         //     ESP_LOGI(HTTPSD_TAG, "SSL write message");
//         //     res = SSL_write(ssl, send_data, send_bytes);
//         //     if (res > 0) {
//         //         ESP_LOGI(HTTPSD_TAG, "OK");
//         //     } else {
//         //         ESP_LOGI(HTTPSD_TAG, "error");
//         //     }
//         //     break;
//         // }
//     } while (1);
    
//     SSL_shutdown(ssl);
// failed5:
//     close(new_sockfd);
//     new_sockfd = -1;
// failed4:
//     SSL_free(ssl);
//     ssl = NULL;
//     goto reconnect;
// failed3:
//     close(sockfd);
//     sockfd = -1;
// failed2:
//     SSL_CTX_free(ctx);
//     ctx = NULL;
// failed1:
//     vTaskDelete(NULL);
//     return ;











// 	// HTTP
// 	ESP_LOGI(HTTPSD_TAG, "Starting HTTP Server...");

// 	// const char base_path[] = CONFIG_EXAMPLE_WEB_MOUNT_POINT;
// 	// const char base_path[] = "/www";

// 	REST_CHECK(base_path, "wrong base path", err);
// 	rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
// 	REST_CHECK(rest_context, "No memory for rest context", err);
// 	strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

// 	httpd_handle_t server = NULL;
	
// 	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
// 	config.uri_match_fn = httpd_uri_match_wildcard;
// 	config.lru_purge_enable = true;

// 	REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start HTTP server failed", err_start);
// 	ESP_LOGI(HTTPSD_TAG, "Started HTTP Server.");

// 	ESP_LOGI(HTTPSD_TAG, "Registering HTTP server URI handlers...");

// 	/* URI handler for getting system info */
// 	httpd_uri_t system_info_get_uri = {
// 		.uri = "/api/v1/system/info",
// 		.method = HTTP_GET,
// 		.handler = system_info_get_handler,
// 		.user_ctx = rest_context};
// 	httpd_register_uri_handler(server, &system_info_get_uri);

// 	/* URI handler for restarting the device */
// 	httpd_uri_t util_restart_post_uri = {
// 		.uri = "/api/util/restart",
// 		.method = HTTP_POST,
// 		.handler = util_restart_post_handler,
// 		.user_ctx = rest_context};
// 	httpd_register_uri_handler(server, &util_restart_post_uri);	

// 	/* URI handler for getting web server files */
// 	httpd_uri_t common_get_uri = {
// 		.uri = "/*",
// 		.method = HTTP_GET,
// 		.handler = rest_common_get_handler,
// 		.user_ctx = rest_context};
// 	httpd_register_uri_handler(server, &common_get_uri);

// 	ESP_LOGI(HTTPSD_TAG, "Registered HTTP server URI handlers.");


// 	// ESP_LOGI(HTTPSD_TAG, "Registering HTTP URI handlers.");
	
// 	// httpd_register_uri_handler(server, &system_info_get_uri);
// 	// httpd_register_uri_handler(server, &util_restart_post_uri);
// 	// httpd_register_uri_handler(server, &common_get_uri);


// 	// HTTPS
// 	// ESP_LOGI(HTTPSD_TAG, "Starting HTTPS Server...");

// 	// httpd_handle_t server_s = NULL;
// 	// httpd_ssl_config_t config_s = HTTPD_SSL_CONFIG_DEFAULT();
// 	// config_s.httpd.uri_match_fn = httpd_uri_match_wildcard;
// 	// // config_s.httpd.max_open_sockets = CONFIG_LWIP_MAX_SOCKETS - 3;
// 	// // config_s.httpd.max_open_sockets = 1;
// 	// // config_s.transport_mode = HTTPD_SSL_TRANSPORT_INSECURE;

// 	// extern const unsigned char ca_cert_conf_pem_start[] asm("_binary_ca_cert_conf_pem_start");
// 	// extern const unsigned char ca_cert_conf_pem_end[]   asm("_binary_ca_cert_conf_pem_end");
// 	// config_s.cacert_pem = ca_cert_conf_pem_start;
// 	// config_s.cacert_len = ca_cert_conf_pem_end - ca_cert_conf_pem_start;

// 	// // config_s.client_verify_cert_pem = config_s.cacert_pem;
// 	// // config_s.client_verify_cert_len = config_s.cacert_len;
	
// 	// // extern const unsigned char cacert_pem_start[] asm("_binary_cacert_pem_start");
// 	// // extern const unsigned char cacert_pem_end[]   asm("_binary_cacert_pem_end");
// 	// // config_s.cacert_pem = cacert_pem_start;
// 	// // config_s.cacert_len = cacert_pem_end - cacert_pem_start;

// 	// extern const unsigned char ca_key_conf_pem_start[] asm("_binary_ca_key_conf_pem_start");
// 	// extern const unsigned char ca_key_conf_pem_end[]   asm("_binary_ca_key_conf_pem_end");
// 	// config_s.prvtkey_pem = ca_key_conf_pem_start;
// 	// config_s.prvtkey_len = ca_key_conf_pem_end - ca_key_conf_pem_start;

// 	// // extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
// 	// // extern const unsigned char prvtkey_pem_end[]   asm("_binary_prvtkey_pem_end");
// 	// // config_s.prvtkey_pem = prvtkey_pem_start;
// 	// // config_s.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;

// 	// // REST_CHECK(httpd_ssl_start(&server_s, &config_s) == ESP_OK, "Start HTTPS server failed", err_start);

// 	// // esp_err_t res = httpd_ssl_start(&server_s, &config_s);
// 	// // if (res != ESP_OK) {
// 	// // 		ESP_LOGE(HTTPSD_TAG, "Error starting HTTPS server: %d", res);
// 	// // 		return ESP_OK;
// 	// // }

// // 	ESP_LOGI(HTTPSD_TAG, "Registering HTTPS URI handlers.");

// // 	/* URI handler for getting system info */
// // 	httpd_uri_t system_info_get_uri = {
// // 		.uri = "/api/v1/system/info",
// // 		.method = HTTP_GET,
// // 		.handler = system_info_get_handler,
// // 		.user_ctx = rest_context};
// // 	// httpd_register_uri_handler(server_s, &system_info_get_uri);

// // 	/* URI handler for restarting the device */
// // 	httpd_uri_t util_restart_post_uri = {
// // 		.uri = "/api/util/restart",
// // 		.method = HTTP_POST,
// // 		.handler = util_restart_post_handler,
// // 		.user_ctx = rest_context};
// // 	// httpd_register_uri_handler(server_s, &util_restart_post_uri);	

// // 	/* URI handler for getting web server files */
// // 	httpd_uri_t common_get_uri = {
// // 		.uri = "/*",
// // 		.method = HTTP_GET,
// // 		.handler = rest_common_get_handler,
// // 		.user_ctx = rest_context};
// // 	// httpd_register_uri_handler(server_s, &common_get_uri);

// // 	// REST_CHECK(httpd_ssl_start(&server_s, &config_s) == ESP_OK, "Start HTTPS server failed", err_start);

// // 	ESP_LOGI(HTTPSD_TAG, "Started HTTPS Server.");

// 	// return ESP_OK;
// 	return;
// err_start:
// 	free(rest_context);
// err:
// 	return;
// // 	return ESP_FAIL;



	// return;

// exit:

//   mbedtls_ssl_session_reset(&ssl);
//   mbedtls_net_free(&server_fd);

//   if (res != 0)
//   {
//     mbedtls_strerror(res, buf, 100);
//     ESP_LOGE(HTTPSD_TAG, "Last error was: -0x%x - %s", -res, buf);
//   }

//   putchar('\n'); // JSON output doesn't have a newline at end

//   ESP_LOGI(HTTPSD_TAG, "completed https request");

//   return;

// err_start:

// 	free(rest_context);

// err:

// 	return;
}

void epaper_idf_httpsd_task(void* pvParameter) {
	while (1)
	{
		/** Inner scope to ensure all destructors are called before vTaskDelete. */
		{
			// if (pvParameter != NULL) {
			// 	httpd_task_action = EPAPER_IDF_HTTPSD_TASK_ACTION_COPY(pvParameter);
			// }

			// if (httpd_task_action.value != NULL) {
			// 	httpd_task_action_value = EPAPER_IDF_HTTPSD_TASK_ACTION_VALUE_COPY(httpd_task_action.value);
			// }

			// start_httpd(CONFIG_EXAMPLE_WEB_MOUNT_POINT);

			start_httpsd(NULL);

			while (1) {
				vTaskDelay(1000 / portTICK_PERIOD_MS);
			}

			// uint32_t val = 0;

			// xTaskNotifyWait(0, ULONG_MAX, &val, portMAX_DELAY);
		}

		vTaskDelete(NULL);
	}
}
