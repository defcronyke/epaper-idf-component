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
#include "esp_crt_bundle.h"
#include "protocol_examples_common.h"
#include "epaper-idf-httpd.h"
#include "epaper-idf-httpsd.h"
#include "epaper-idf-http.h"

static const char *TAG = "epaper-idf-http";

esp_event_loop_handle_t epaper_idf_http_event_loop_handle;

static struct epaper_idf_http_task_action_value_t http_task_action_value;
static struct epaper_idf_http_task_action_t http_task_action;

// #ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_HTTPSD_H_INCLUDED__

static bool epaper_idf_https_is_init;

// #endif

// mbedtls_net_context server_fd;
// mbedtls_ssl_context ssl;
// mbedtls_ssl_config conf;
// mbedtls_x509_crt cacert;
// mbedtls_pk_context pkey;
// mbedtls_entropy_context entropy;
// mbedtls_ctr_drbg_context ctr_drbg;

ESP_EVENT_DEFINE_BASE(EPAPER_IDF_HTTP_EVENT);

#define WEB_URL CONFIG_EPAPER_IDF_IMAGES_INDEX_JSON_URL
#define WEB_SERVER CONFIG_EPAPER_IDF_IMAGES_INDEX_JSON_URL_HOST
#define WEB_PORT "443"

// static const char *REQUEST = "GET " WEB_URL " HTTP/1.1\r\n"
static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
                             "Host: " WEB_SERVER "\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";

// static const unsigned char cacert_pem_start[] asm("_binary_ca_cert_conf_pem_start");
// static const unsigned char cacert_pem_end[] asm("_binary_ca_cert_conf_pem_end");

// static const unsigned char prvtkey_pem_start[] asm("_binary_ca_key_conf_pem_start");
// static const unsigned char prvtkey_pem_end[] asm("_binary_ca_key_conf_pem_end");

static char buf[1024];
// static char buf[512];

static mbedtls_net_context server_fd;
static mbedtls_ssl_context ssl;
static mbedtls_ssl_config conf;
static mbedtls_x509_crt cacert;
// static mbedtls_pk_context pkey;
static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;

void epaper_idf_http_get(struct epaper_idf_http_task_action_value_t action_value)
{
  esp_err_t err = ESP_OK;

  int ret, flags, len;

  if (!epaper_idf_https_is_init)
  {

    // TODO: Add this to Kconfig menu.
    conf.read_timeout = 10000;

    mbedtls_ssl_init(&ssl);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    ESP_LOGI(TAG, "Seeding the random number generator");

    mbedtls_ssl_config_init(&conf);

    mbedtls_entropy_init(&entropy);
    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     NULL, 0)) != 0)
    {
      ESP_LOGE(TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
      abort();
    }

    ESP_LOGI(TAG, "Attaching the certificate bundle...");

    ret = esp_crt_bundle_attach(&conf);

    if (ret < 0)
    {
      ESP_LOGE(TAG, "esp_crt_bundle_attach returned -0x%x\n\n", -ret);
      abort();
    }

    ESP_LOGI(TAG, "Setting hostname for TLS session...");

    /* Hostname set here should match CN in server certificate */
    if ((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
    {
      ESP_LOGE(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
      abort();
    }

    ESP_LOGI(TAG, "Setting up the SSL/TLS structure...");

    if ((ret = mbedtls_ssl_config_defaults(&conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
      ESP_LOGE(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
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

    // mbedtls_ssl_session_reset(&ssl);

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
      ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
      goto exit;
    }

    epaper_idf_https_is_init = true;
  }

  /** The HTTP GET request. */
  while (1)
  {
    mbedtls_net_init(&server_fd);

    ESP_LOGI(TAG, "Connecting to %s:%s...", WEB_SERVER, WEB_PORT);

    if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER,
                                   WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
    {
      ESP_LOGE(TAG, "mbedtls_net_connect returned -%x", -ret);
      goto exit;
    }

    ESP_LOGI(TAG, "Connected.");

    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    ESP_LOGI(TAG, "Performing the SSL/TLS handshake...");

    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
    {
      if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
      {
        ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
        goto exit;
      }
    }

    ESP_LOGI(TAG, "Verifying peer X.509 certificate...");

    if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
    {
      /* In real life, we probably want to close connection if ret != 0 */
      ESP_LOGW(TAG, "Failed to verify peer certificate!");
      bzero(buf, sizeof(buf));
      mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
      ESP_LOGW(TAG, "verification info: %s", buf);
    }
    else
    {
      ESP_LOGI(TAG, "Certificate verified.");
    }

    ESP_LOGI(TAG, "Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&ssl));

    ESP_LOGI(TAG, "Writing HTTP request...");

    size_t written_bytes = 0;
    do
    {
      ret = mbedtls_ssl_write(&ssl,
                              (const unsigned char *)REQUEST + written_bytes,
                              strlen(REQUEST) - written_bytes);
      if (ret >= 0)
      {
        ESP_LOGI(TAG, "%d bytes written", ret);
        written_bytes += ret;
      }
      else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ)
      {
        ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
        goto exit;
      }
    } while (written_bytes < strlen(REQUEST));

    ESP_LOGI(TAG, "Reading HTTP response...");

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
        ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
        break;
      }

      if (ret == 0)
      {
        ESP_LOGI(TAG, "connection closed");
        break;
      }

      len = ret;
      ESP_LOGD(TAG, "%d bytes read", len);
      /* Print response directly to stdout as it is read */
      for (int i = 0; i < len; i++)
      {
        putchar(buf[i]);
      }
    } while (1);

    mbedtls_ssl_close_notify(&ssl);

    // break;

  exit:
    mbedtls_ssl_session_reset(&ssl);
    mbedtls_net_free(&server_fd);

    if (ret != 0)
    {
      mbedtls_strerror(ret, buf, 100);
      ESP_LOGE(TAG, "Last error was: -0x%x - %s", -ret, buf);
    }

    putchar('\n'); // JSON output doesn't have a newline at end

    ESP_LOGI(TAG, "completed https request");

    break;
  }

  // Send an event which says "this task is finished".
  err = esp_event_post_to(epaper_idf_http_event_loop_handle, EPAPER_IDF_HTTP_EVENT, EPAPER_IDF_HTTP_EVENT_FINISH, EPAPER_IDF_HTTP_TASK_ACTION_VALUE_CAST_VOID_P(action_value), sizeof(action_value), portMAX_DELAY);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Sending event failed");
  }
}

// void epaper_idf_http_get(struct epaper_idf_http_task_action_value_t action_value)
// {
//   ESP_LOGI(TAG, "Starting HTTP(S) GET request...");

//   esp_err_t err = ESP_OK;

//   int ret, flags, len;

//   // unsigned int cacert_pem_bytes = cacert_pem_end - cacert_pem_start;
//   // unsigned int prvtkey_pem_bytes = prvtkey_pem_end - prvtkey_pem_start;

//   // #ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_HTTPSD_H_INCLUDED__
//   if (!epaper_idf_https_is_init)
//   {
//     ESP_LOGI(TAG, "HTTPS initializing...");

// #ifdef CONFIG_MBEDTLS_DEBUG
//     mbedtls_esp_enable_debug_log(&conf, 5);
// #endif

//     // TODO: Add this to Kconfig menu.
//     // conf.read_timeout = 10000;

//     mbedtls_net_init(&server_fd);

//     mbedtls_ssl_init(&ssl);
//     mbedtls_ssl_config_init(&conf);

//     mbedtls_x509_crt_init(&cacert);
//     // mbedtls_pk_init(&pkey);
//     mbedtls_entropy_init(&entropy);
//     mbedtls_ctr_drbg_init(&ctr_drbg);

//     // /** NOTE: 1. Load the certificates and private RSA key */
//     // ESP_LOGI(TAG, "Loading the server cert. and key...");

//     // /** TODO: This demonstration program uses embedded test certificates.
//     //  * Instead, you may want to use mbedtls_x509_crt_parse_file() to read the
//     //  * server and CA certificates, as well as mbedtls_pk_parse_keyfile().
//     //  */
//     // ESP_LOGI(TAG, "SSL server context set own certification......");
//     // ESP_LOGI(TAG, "Parsing test srv_crt......");
//     // ret = mbedtls_x509_crt_parse(&cacert, (const unsigned char *)cacert_pem_start,
//     //                              cacert_pem_bytes);
//     // if (ret != 0)
//     // {
//     //   ESP_LOGI(TAG, " failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", ret);
//     //   goto exit;
//     // }
//     // ESP_LOGI(TAG, "OK");

//     // ESP_LOGI(TAG, "SSL server context set private key......");
//     // ret = mbedtls_pk_parse_key(&pkey, (const unsigned char *)prvtkey_pem_start,
//     //                            prvtkey_pem_bytes, NULL, 0);
//     // if (ret != 0)
//     // {
//     //   ESP_LOGI(TAG, " failed\n  !  mbedtls_pk_parse_key returned %d\n\n", ret);
//     //   goto exit;
//     // }
//     // ESP_LOGI(TAG, "OK");

//     ESP_LOGI(TAG, "Seeding the random number generator");

//     if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
//                                      NULL, 0)) != 0)
//     {
//       ESP_LOGE(TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
//       abort();
//     }
//     ESP_LOGI(TAG, "OK");

//     // ESP_LOGI(TAG, "Setting hostname for TLS session...");

//     // /** NOTE: Hostname set here should match CN in server certificate */
//     // if ((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
//     // {
//     //   ESP_LOGE(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
//     //   abort();
//     // }

//     // ESP_LOGI(TAG, "Setting up the SSL/TLS structure...");

//     // if ((ret = mbedtls_ssl_config_defaults(&conf,
//     //                                        MBEDTLS_SSL_IS_CLIENT,
//     //                                        MBEDTLS_SSL_TRANSPORT_STREAM,
//     //                                        MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
//     // {
//     //   ESP_LOGE(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
//     //   goto exit;
//     // }

//     ESP_LOGI(TAG, "Attaching the certificate bundle...");

//     // #ifdef CONFIG_MBEDTLS_DEBUG
//     //   mbedtls_esp_enable_debug_log(&conf, 5);
//     // #endif

//     ret = esp_crt_bundle_attach(&conf);

//     if (ret < 0)
//     {
//       ESP_LOGE(TAG, "esp_crt_bundle_attach returned -0x%x\n\n", -ret);
//       abort();
//     }

//     /** NOTE: MBEDTLS_SSL_VERIFY_OPTIONAL is bad for security, in this example
//       it will print	a warning if CA verification fails but it will continue to
//       connect. You should consider using MBEDTLS_SSL_VERIFY_REQUIRED in your
//       own code. */
//     // mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);

//     mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);

//     mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

//     // #ifdef CONFIG_MBEDTLS_DEBUG
//     // mbedtls_esp_enable_debug_log(&conf, 5);
//     // mbedtls_esp_enable_debug_log(&conf, CONFIG_MBEDTLS_DEBUG_LEVEL);
//     // #endif

//     // #if defined(MBEDTLS_SSL_CACHE_C)
//     //     mbedtls_ssl_conf_session_cache(&conf, &cache,
//     //                                    mbedtls_ssl_cache_get,
//     //                                    mbedtls_ssl_cache_set);
//     // #endif

//     // mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
//     // mbedtls_ssl_conf_ca_chain(&conf, cacert.next, NULL);
//     // if ((ret = mbedtls_ssl_conf_own_cert(&conf, &cacert, &pkey)) != 0)
//     // {
//     //   ESP_LOGI(TAG, " failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n\n", ret);
//     //   goto exit;
//     // }

//     // if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
//     // {
//     //   ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
//     //   goto exit;
//     // }

//     // /** TODO: Call this in the case where HTTPS server is disabled
//     //   (it currently calls this instead) */
//     // if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
//     // {
//     //   ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
//     //   goto exit;
//     // }

//     mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

//     ESP_LOGI(TAG, "Setting hostname for TLS session...");

//     /** NOTE: Hostname set here should match CN in server certificate */
//     if ((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
//     {
//       ESP_LOGE(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
//       abort();
//     }

//     epaper_idf_https_is_init = true;

//     ESP_LOGI(TAG, "HTTPS initialized.");
//   }

//   ESP_LOGI(TAG, "Setting up the SSL/TLS structure...");

//   if ((ret = mbedtls_ssl_config_defaults(&conf,
//                                          MBEDTLS_SSL_IS_CLIENT,
//                                          MBEDTLS_SSL_TRANSPORT_STREAM,
//                                          MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
//   {
//     ESP_LOGE(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
//     goto exit;
//   }

//   // #endif

//   // while (1)
//   // {

//   // mbedtls_net_init(&server_fd);

//   // mbedtls_net_init(&server_fd);

//   ESP_LOGI(TAG, "Connecting to %s:%s...", WEB_SERVER, WEB_PORT);

//   if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER,
//                                  WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
//   {
//     ESP_LOGE(TAG, "mbedtls_net_connect returned -%x", -ret);
//     goto exit;
//   }

//   ESP_LOGI(TAG, "Connected.");

//   // mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

//   ESP_LOGI(TAG, "Performing the SSL/TLS handshake...");

//   while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
//   {
//     if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
//     {
//       ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
//       goto exit;
//     }
//   }

//   ESP_LOGI(TAG, "Verifying peer X.509 certificate...");

//   if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
//   {
//     /* In real life, we probably want to close connection if ret != 0 */
//     ESP_LOGW(TAG, "Failed to verify peer certificate!");
//     bzero(buf, sizeof(buf));
//     mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
//     ESP_LOGW(TAG, "verification info: %s", buf);

//     // // TODO: does this work here?
//     // goto exit;
//   }
//   else
//   {
//     ESP_LOGI(TAG, "Certificate verified.");
//   }

//   ESP_LOGI(TAG, "Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&ssl));

//   ESP_LOGI(TAG, "Writing HTTP request...");

//   size_t written_bytes = 0;
//   do
//   {
//     ret = mbedtls_ssl_write(
//         &ssl,
//         (const unsigned char *)REQUEST + written_bytes,
//         strlen(REQUEST) - written_bytes);
//     if (ret >= 0)
//     {
//       ESP_LOGI(TAG, "%d bytes written", ret);
//       written_bytes += ret;
//     }
//     else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ)
//     {
//       ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
//       goto exit;
//     }
//   } while (written_bytes < strlen(REQUEST));

//   ESP_LOGI(TAG, "Reading HTTP response...");

//   do
//   {
//     len = sizeof(buf) - 1;
//     bzero(buf, sizeof(buf));
//     ret = mbedtls_ssl_read(&ssl, (unsigned char *)buf, len);

//     if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
//       continue;

//     if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
//     {
//       ret = 0;
//       break;
//     }

//     if (ret < 0)
//     {
//       ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
//       break;
//     }

//     if (ret == 0)
//     {
//       ESP_LOGI(TAG, "connection closed");
//       break;
//     }

//     len = ret;
//     ESP_LOGD(TAG, "%d bytes read", len);
//     /* Print response directly to stdout as it is read */
//     for (int i = 0; i < len; i++)
//     {
//       putchar(buf[i]);
//     }
//   } while (1);

//   mbedtls_ssl_close_notify(&ssl);

// exit:
//   // ESP_LOGI(TAG, "Exiting...");

//   // esp_crt_bundle_detach(&conf);
//   // mbedtls_ssl_session_reset(&ssl);
//   // mbedtls_net_free(&server_fd);

//   if (ret != 0)
//   {
//     mbedtls_strerror(ret, buf, 100);
//     ESP_LOGE(TAG, "Last error was: -0x%x - %s", -ret, buf);
//   }

//   putchar('\n'); // JSON output doesn't have a newline at end

//   ESP_LOGI(TAG, "Completed https request.");

//   //   break;
//   // }

//   // Send an event which says "this task is finished".
//   err = esp_event_post_to(epaper_idf_http_event_loop_handle, EPAPER_IDF_HTTP_EVENT, EPAPER_IDF_HTTP_EVENT_FINISH, EPAPER_IDF_HTTP_TASK_ACTION_VALUE_CAST_VOID_P(action_value), sizeof(action_value), portMAX_DELAY);
//   if (err != ESP_OK)
//   {
//     ESP_LOGE(TAG, "Sending event failed");
//   }
// }

void epaper_idf_http_task(void *pvParameter)
{
  epaper_idf_https_is_init = false;

  while (1)
  {
    if (pvParameter != NULL)
    {
      http_task_action = EPAPER_IDF_HTTP_TASK_ACTION_COPY(pvParameter);
    }

    if (http_task_action.value != NULL)
    {
      http_task_action_value = EPAPER_IDF_HTTP_TASK_ACTION_VALUE_COPY(http_task_action.value);
    }

    // BaseType_t stack_res = uxTaskGetStackHighWaterMark(NULL);
    // ESP_LOGW(TAG, "!!! [ http task ] before: epaper_idf_http_get() -> STACK SIZE !!!: %d", stack_res);

    epaper_idf_http_get(http_task_action_value);

    BaseType_t stack_res = uxTaskGetStackHighWaterMark(NULL);
    ESP_LOGW(TAG, "!!! [ http task ] after: epaper_idf_http_get() -> STACK SIZE !!!: %d", stack_res);

    vTaskDelete(NULL);
  }
}
