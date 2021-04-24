/**	epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.

		Contains some modified example code from here:
		https://github.com/espressif/esp-idf/blob/release/v4.2/examples/protocols/https_mbedtls/main/https_mbedtls_example_main.c

		Original Example Code Header:
		Original Copyright (C) 2006-2016, ARM Limited, All Rights Reserved, Apache 2.0 License.
		Additions Copyright (C) Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD, Apache 2.0 License.

		Licensed under the Apache License, Version 2.0 (the "License");
		you may not use this file except in compliance with the License.
		You may obtain a copy of the License at

			http://www.apache.org/licenses/LICENSE-2.0

		Unless required by applicable law or agreed to in writing, software
		distributed under the License is distributed on an "AS IS" BASIS,
		WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
		See the License for the specific language governing permissions and
		limitations under the License.
*/
#include "epaper-idf-http.h"
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#if CONFIG_EXAMPLE_CONNECT_WIFI
#include "esp_wifi.h"
#endif
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "esp_crt_bundle.h"
#include "protocol_examples_common.h"

static const char *HTTP_TAG = "epaper-idf-http";

esp_event_loop_handle_t epaper_idf_http_event_loop_handle;

static struct epaper_idf_http_task_action_value_t http_task_action_value;
static struct epaper_idf_http_task_action_t http_task_action;

static bool epaper_idf_http_is_init = false;

static mbedtls_net_context server_fd;
static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;
static mbedtls_ssl_context ssl;
static mbedtls_x509_crt cacert;
static mbedtls_ssl_config conf;

static char buf[512];

ESP_EVENT_DEFINE_BASE(EPAPER_IDF_HTTP_EVENT);

#define WEB_URL CONFIG_EPAPER_IDF_IMAGES_INDEX_JSON_URL
#define WEB_SERVER CONFIG_EPAPER_IDF_IMAGES_INDEX_JSON_URL_HOST
#define WEB_PORT "443"

static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
														 "Host: " WEB_SERVER "\r\n"
														 "User-Agent: esp-idf/1.0 esp32\r\n"
														 "\r\n";

void epaper_idf_http_get(struct epaper_idf_http_task_action_value_t action_value) {
	esp_err_t err = ESP_OK;

	int ret, flags, len;

	if (!epaper_idf_http_is_init) {

		// TODO: Add this to Kconfig menu.
		conf.read_timeout = 10000;

		mbedtls_ssl_init(&ssl);
		mbedtls_x509_crt_init(&cacert);
		mbedtls_ctr_drbg_init(&ctr_drbg);
		ESP_LOGI(HTTP_TAG, "Seeding the random number generator");

		mbedtls_ssl_config_init(&conf);

		mbedtls_entropy_init(&entropy);
		if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
																		NULL, 0)) != 0)
		{
			ESP_LOGE(HTTP_TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
			abort();
		}

		ESP_LOGI(HTTP_TAG, "Attaching the certificate bundle...");

		ret = esp_crt_bundle_attach(&conf);

		if (ret < 0)
		{
			ESP_LOGE(HTTP_TAG, "esp_crt_bundle_attach returned -0x%x\n\n", -ret);
			abort();
		}

		ESP_LOGI(HTTP_TAG, "Setting hostname for TLS session...");

		/* Hostname set here should match CN in server certificate */
		if ((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
		{
			ESP_LOGE(HTTP_TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
			abort();
		}

		ESP_LOGI(HTTP_TAG, "Setting up the SSL/TLS structure...");

		if ((ret = mbedtls_ssl_config_defaults(&conf,
																					MBEDTLS_SSL_IS_CLIENT,
																					MBEDTLS_SSL_TRANSPORT_STREAM,
																					MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
		{
			ESP_LOGE(HTTP_TAG, "mbedtls_ssl_config_defaults returned %d", ret);
			goto exit;
		}

		/* MBEDTLS_SSL_VERIFY_OPTIONAL is bad for security, in this example it will print
			a warning if CA verification fails but it will continue to connect.
			You should consider using MBEDTLS_SSL_VERIFY_REQUIRED in your own code.
		*/
		mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
		mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
		mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
	#ifdef CONFIG_MBEDTLS_DEBUG
		mbedtls_esp_enable_debug_log(&conf, CONFIG_MBEDTLS_DEBUG_LEVEL);
	#endif

		if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
		{
			ESP_LOGE(HTTP_TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
			goto exit;
		}

		epaper_idf_http_is_init = true;
	}

	while (1)
	{
		mbedtls_net_init(&server_fd);

		ESP_LOGI(HTTP_TAG, "Connecting to %s:%s...", WEB_SERVER, WEB_PORT);

		if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER,
																	WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
		{
			ESP_LOGE(HTTP_TAG, "mbedtls_net_connect returned -%x", -ret);
			goto exit;
		}

		ESP_LOGI(HTTP_TAG, "Connected.");

		mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

		ESP_LOGI(HTTP_TAG, "Performing the SSL/TLS handshake...");

		while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
		{
			if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
			{
				ESP_LOGE(HTTP_TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
				goto exit;
			}
		}

		ESP_LOGI(HTTP_TAG, "Verifying peer X.509 certificate...");

		if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
		{
			/* In real life, we probably want to close connection if ret != 0 */
			ESP_LOGW(HTTP_TAG, "Failed to verify peer certificate!");
			bzero(buf, sizeof(buf));
			mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
			ESP_LOGW(HTTP_TAG, "verification info: %s", buf);
		}
		else
		{
			ESP_LOGI(HTTP_TAG, "Certificate verified.");
		}

		ESP_LOGI(HTTP_TAG, "Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&ssl));

		ESP_LOGI(HTTP_TAG, "Writing HTTP request...");

		size_t written_bytes = 0;
		do
		{
			ret = mbedtls_ssl_write(&ssl,
															(const unsigned char *)REQUEST + written_bytes,
															strlen(REQUEST) - written_bytes);
			if (ret >= 0)
			{
				ESP_LOGI(HTTP_TAG, "%d bytes written", ret);
				written_bytes += ret;
			}
			else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ)
			{
				ESP_LOGE(HTTP_TAG, "mbedtls_ssl_write returned -0x%x", -ret);
				goto exit;
			}
		} while (written_bytes < strlen(REQUEST));

		ESP_LOGI(HTTP_TAG, "Reading HTTP response...");

		do
		{
			len = sizeof(buf) - 1;
			bzero(buf, sizeof(buf));
			ret = mbedtls_ssl_read(&ssl, (unsigned char *)buf, len);

			if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
				continue;

			if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
			{
				ret = 0;
				break;
			}

			if (ret < 0)
			{
				ESP_LOGE(HTTP_TAG, "mbedtls_ssl_read returned -0x%x", -ret);
				break;
			}

			if (ret == 0)
			{
				ESP_LOGI(HTTP_TAG, "connection closed");
				break;
			}

			len = ret;
			ESP_LOGD(HTTP_TAG, "%d bytes read", len);
			/* Print response directly to stdout as it is read */
			for (int i = 0; i < len; i++)
			{
				putchar(buf[i]);
			}
		} while (1);

		mbedtls_ssl_close_notify(&ssl);

	exit:
		mbedtls_ssl_session_reset(&ssl);
		mbedtls_net_free(&server_fd);

		if (ret != 0)
		{
			mbedtls_strerror(ret, buf, 100);
			ESP_LOGE(HTTP_TAG, "Last error was: -0x%x - %s", -ret, buf);
		}

		putchar('\n'); // JSON output doesn't have a newline at end

		ESP_LOGI(HTTP_TAG, "completed https request");

		break;
	}

	// Send an event which says "this task is finished".
	err = esp_event_post_to(epaper_idf_http_event_loop_handle, EPAPER_IDF_HTTP_EVENT, EPAPER_IDF_HTTP_EVENT_FINISH, EPAPER_IDF_HTTP_TASK_ACTION_VALUE_CAST_VOID_P(action_value), sizeof(action_value), portMAX_DELAY);
	if (err != ESP_OK) {
		ESP_LOGE(HTTP_TAG, "Sending event failed");
	}
}

void epaper_idf_http_task(void *pvParameter)
{
	while (1)
	{
		/** Inner scope to ensure all destructors are called before vTaskDelete. */
		{
			if (pvParameter != NULL) {
				http_task_action = EPAPER_IDF_HTTP_TASK_ACTION_COPY(pvParameter);
			}

			if (http_task_action.value != NULL) {
				http_task_action_value = EPAPER_IDF_HTTP_TASK_ACTION_VALUE_COPY(http_task_action.value);
			}

			epaper_idf_http_get(http_task_action_value);
		}

		vTaskDelete(NULL);
	}
}
