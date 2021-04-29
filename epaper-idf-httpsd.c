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

#include "esp_netif.h"
#include "esp_event.h"
#include "esp_event_base.h"
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
#define mbedtls_free free
#define mbedtls_time time
#define mbedtls_time_t time_t
#define mbedtls_calloc calloc
#define mbedtls_fprintf fprintf
#define mbedtls_printf printf
#define mbedtls_exit exit
#define MBEDTLS_EXIT_SUCCESS EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE EXIT_FAILURE
#endif

#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl_cache.h"
#include "esp_crt_bundle.h"
#include "protocol_examples_common.h"
#include "cJSON.h"
#include "epaper-idf-httpsd.h"

#if !defined(MBEDTLS_ENTROPY_C) ||                                \
    !defined(MBEDTLS_SSL_TLS_C) || !defined(MBEDTLS_SSL_SRV_C) || \
    !defined(MBEDTLS_NET_C) || !defined(MBEDTLS_CTR_DRBG_C)
int main(void)
{
  mbedtls_printf("MBEDTLS_ENTROPY_C and/or "
                 "MBEDTLS_SSL_TLS_C and/or MBEDTLS_SSL_SRV_C and/or "
                 "MBEDTLS_NET_C and/or MBEDTLS_CTR_DRBG_C and/or not defined.\n");
  mbedtls_exit(0);
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

/** Size of memory to be allocated for the heap, when using the library's memory
  management and MBEDTLS_MEMORY_BUFFER_ALLOC_C is enabled. */
#define MEMORY_HEAP_SIZE 120000

#define DFL_SERVER_ADDR NULL
#define DFL_SERVER_PORT "4433"
#define DFL_RESPONSE_SIZE -1
#define DFL_DEBUG_LEVEL 0
#define DFL_NBIO 0
#define DFL_EVENT 0
#define DFL_READ_TIMEOUT 0
#define DFL_CA_FILE ""
#define DFL_CA_PATH ""
#define DFL_CRT_FILE ""
#define DFL_KEY_FILE ""
#define DFL_CRT_FILE2 ""
#define DFL_KEY_FILE2 ""
#define DFL_ASYNC_OPERATIONS "-"
#define DFL_ASYNC_PRIVATE_DELAY1 (-1)
#define DFL_ASYNC_PRIVATE_DELAY2 (-1)
#define DFL_ASYNC_PRIVATE_ERROR (0)
#define DFL_PSK ""
#define DFL_PSK_IDENTITY "Client_identity"
#define DFL_ECJPAKE_PW NULL
#define DFL_PSK_LIST NULL
#define DFL_FORCE_CIPHER 0
#define DFL_VERSION_SUITES NULL
#define DFL_RENEGOTIATION MBEDTLS_SSL_RENEGOTIATION_DISABLED
#define DFL_ALLOW_LEGACY -2
#define DFL_RENEGOTIATE 0
#define DFL_RENEGO_DELAY -2
#define DFL_RENEGO_PERIOD ((uint64_t)-1)
#define DFL_EXCHANGES 1
#define DFL_MIN_VERSION -1
#define DFL_MAX_VERSION -1
#define DFL_ARC4 -1
#define DFL_SHA1 -1
#define DFL_AUTH_MODE -1
#define DFL_CERT_REQ_CA_LIST MBEDTLS_SSL_CERT_REQ_CA_LIST_ENABLED
#define DFL_MFL_CODE MBEDTLS_SSL_MAX_FRAG_LEN_NONE
#define DFL_TRUNC_HMAC -1
#define DFL_TICKETS MBEDTLS_SSL_SESSION_TICKETS_ENABLED
#define DFL_TICKET_TIMEOUT 86400
#define DFL_CACHE_MAX -1
#define DFL_CACHE_TIMEOUT -1
#define DFL_SNI NULL
#define DFL_ALPN_STRING NULL
#define DFL_CURVES NULL
#define DFL_DHM_FILE NULL
#define DFL_TRANSPORT MBEDTLS_SSL_TRANSPORT_STREAM
#define DFL_COOKIES 1
#define DFL_ANTI_REPLAY -1
#define DFL_HS_TO_MIN 0
#define DFL_HS_TO_MAX 0
#define DFL_DTLS_MTU -1
#define DFL_BADMAC_LIMIT -1
#define DFL_DGRAM_PACKING 1
#define DFL_EXTENDED_MS -1
#define DFL_ETM -1

#endif

#define WEB_SERVER CONFIG_LWIP_LOCAL_HOSTNAME
#define DFL_EVENT 0

static const char *HTTPSD_TAG = "epaper-idf-httpsd";

esp_event_loop_handle_t epaper_idf_httpsd_event_loop_handle;

ESP_EVENT_DEFINE_BASE(EPAPER_IDF_HTTPSD_EVENT);

#if defined(MBEDTLS_SSL_CACHE_C)
static mbedtls_ssl_cache_context cache;
#endif

#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_HTTPD_H_INCLUDED__
bool fs_initialized = false;
#endif

static bool epaper_idf_httpsd_is_init;

#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_HTTPD_H_INCLUDED__
#if CONFIG_EXAMPLE_WEB_DEPLOY_SEMIHOST
esp_err_t init_fs(void)
{
  if (fs_initialized)
  {
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
#endif

#if CONFIG_EXAMPLE_WEB_DEPLOY_SD
esp_err_t init_fs(void)
{
  if (fs_initialized)
  {
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
#endif

#if CONFIG_EXAMPLE_WEB_DEPLOY_SF
esp_err_t init_fs(void)
{
  if (fs_initialized)
  {
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
#endif /** #endif CONFIG_EXAMPLE_WEB_DEPLOY_SEMIHOST */
#endif /** #endif __EPAPER_IDF_COMPONENT_EPAPER_IDF_HTTPD_H_INCLUDED__ */

static unsigned char buf[MBEDTLS_EXAMPLE_RECV_BUF_LEN];

static const unsigned char cacert_pem_start[] asm("_binary_ca_cert_conf_pem_start");
static const unsigned char cacert_pem_end[] asm("_binary_ca_cert_conf_pem_end");

static const unsigned char prvtkey_pem_start[] asm("_binary_ca_key_conf_pem_start");
static const unsigned char prvtkey_pem_end[] asm("_binary_ca_key_conf_pem_end");

#if defined(MBEDTLS_TIMING_C)
static mbedtls_timing_delay_context timer;
#endif

static mbedtls_net_context listen_fd, client_fd;
static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;
static mbedtls_ssl_context ssl;
static mbedtls_ssl_config conf;
static mbedtls_x509_crt srvcert;
static mbedtls_pk_context pkey;

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
  return (ret == MBEDTLS_ERR_SSL_WANT_READ ||
          ret == MBEDTLS_ERR_SSL_WANT_WRITE ||
          ret == MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS);
}

/*
 * Wait for an event from the underlying transport or the timer
 * (Used in event-driven IO mode).
 */
#if !defined(MBEDTLS_TIMING_C)
int idle(mbedtls_net_context *fd,
         int idle_reason)
#else
int idle(mbedtls_net_context *fd,
         mbedtls_timing_delay_context *timer,
         int idle_reason)
#endif
{
  int ret;
  int poll_type = 0;

  if (idle_reason == MBEDTLS_ERR_SSL_WANT_WRITE)
    poll_type = MBEDTLS_NET_POLL_WRITE;
  else if (idle_reason == MBEDTLS_ERR_SSL_WANT_READ)
    poll_type = MBEDTLS_NET_POLL_READ;
#if !defined(MBEDTLS_TIMING_C)
  else
    return (0);
#endif

  while (1)
  {
    /* Check if timer has expired */
#if defined(MBEDTLS_TIMING_C)
    if (timer != NULL &&
        mbedtls_timing_get_delay(timer) == 2)
    {
      break;
    }
#endif /* MBEDTLS_TIMING_C */

    /* Check if underlying transport became available */
    if (poll_type != 0)
    {
      ret = mbedtls_net_poll(fd, poll_type, 0);
      if (ret < 0)
        return (ret);
      if (ret == poll_type)
        break;
    }
  }

  return (0);
}

/** EXAMPLE: EPAPER_IDF_HTTPSD_HTTPS_RESPONSE */
/** #define EPAPER_IDF_HTTPSD_HTTPS_RESPONSE \
  "HTTP/1.1 200 OK\r\n" \
  "Content-Type: text/html\r\n\r\n" \
  "%s\0"
*/

/** NOTE: The complete HTTPS response string. */
#define EPAPER_IDF_HTTPSD_HTTPS_RESPONSE \
  "%s %s\r\n"                            \
  "Content-Type: %s\r\n"                 \
  "Content-Length: %d\r\n\r\n"           \
  "%s\r\n"

/** TODO: Add these in Kconfig menu. */
static char *resp_proto = "HTTP/1.1";
static char *resp_status = "200 OK";
static char *resp_header_content_type = "text/html";

static void start_httpsd(void *pvParameter)
{
  esp_err_t res = ESP_OK;

#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_HTTPD_H_INCLUDED__
  ESP_ERROR_CHECK(init_fs());
#endif

  // const unsigned char cacert_pem_start[] asm("_binary_ca_cert_conf_pem_start");
  // const unsigned char cacert_pem_end[] asm("_binary_ca_cert_conf_pem_end");
  // const unsigned int cacert_pem_bytes = cacert_pem_end - cacert_pem_start;

  // const unsigned char prvtkey_pem_start[] asm("_binary_ca_key_conf_pem_start");
  // const unsigned char prvtkey_pem_end[] asm("_binary_ca_key_conf_pem_end");
  // const unsigned int prvtkey_pem_bytes = prvtkey_pem_end - prvtkey_pem_start;

  int ret = 0, len, written, frags;

  unsigned int cacert_pem_bytes = cacert_pem_end - cacert_pem_start;
  unsigned int prvtkey_pem_bytes = prvtkey_pem_end - prvtkey_pem_start;

  /** NOTE: The HTTPS system only gets initialized once. */
  if (!epaper_idf_httpsd_is_init)
  {

    mbedtls_net_init(&listen_fd);
    mbedtls_net_init(&client_fd);
    ESP_LOGI(HTTPSD_TAG, "SSL server context create ......");
    mbedtls_ssl_init(&ssl);
    ESP_LOGI(HTTPSD_TAG, "OK");
    mbedtls_ssl_config_init(&conf);

#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_init(&cache);
#endif

    mbedtls_x509_crt_init(&srvcert);
    mbedtls_pk_init(&pkey);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    /** NOTE: 1. Load the certificates and private RSA key */
    ESP_LOGI(HTTPSD_TAG, "Loading the server cert. and key...");

    /** TODO: This demonstration program uses embedded test certificates.
     * Instead, you may want to use mbedtls_x509_crt_parse_file() to read the
     * server and CA certificates, as well as mbedtls_pk_parse_keyfile().
     */
    ESP_LOGI(HTTPSD_TAG, "SSL server context set own certification......");
    ESP_LOGI(HTTPSD_TAG, "Parsing test srv_crt......");
    ret = mbedtls_x509_crt_parse(&srvcert, (const unsigned char *)cacert_pem_start,
                                 cacert_pem_bytes);
    if (ret != 0)
    {
      ESP_LOGI(HTTPSD_TAG, " failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", ret);
      goto exit;
    }
    ESP_LOGI(HTTPSD_TAG, "OK");

    ESP_LOGI(HTTPSD_TAG, "SSL server context set private key......");
    ret = mbedtls_pk_parse_key(&pkey, (const unsigned char *)prvtkey_pem_start,
                               prvtkey_pem_bytes, NULL, 0);
    if (ret != 0)
    {
      ESP_LOGI(HTTPSD_TAG, " failed\n  !  mbedtls_pk_parse_key returned %d\n\n", ret);
      goto exit;
    }
    ESP_LOGI(HTTPSD_TAG, "OK");

    /*
    * 2. Setup the listening TCP socket
    */
    ESP_LOGI(HTTPSD_TAG, "SSL server socket bind at localhost:443 ......");
    if ((ret = mbedtls_net_bind(&listen_fd, NULL, "443", MBEDTLS_NET_PROTO_TCP)) != 0)
    {
      ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_net_bind returned %d\n\n", ret);
      goto exit;
    }
    ESP_LOGI(HTTPSD_TAG, "OK");

    /*
    * 3. Seed the RNG
    */
    ESP_LOGI(HTTPSD_TAG, "  . Seeding the random number generator...");
    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     (const unsigned char *)HTTPSD_TAG,
                                     strlen(HTTPSD_TAG))) != 0)
    {
      ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
      goto exit;
    }
    ESP_LOGI(HTTPSD_TAG, "OK");

    /*
      * 4. Setup stuff
      */
    ESP_LOGI(HTTPSD_TAG, "  . Setting up the SSL data....");

#ifdef CONFIG_MBEDTLS_DEBUG
    mbedtls_esp_enable_debug_log(&conf, 5);
#endif

    if ((ret = mbedtls_ssl_config_defaults(&conf,
                                           MBEDTLS_SSL_IS_SERVER,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
      ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret);
      goto exit;
    }

    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_conf_session_cache(&conf, &cache,
                                   mbedtls_ssl_cache_get,
                                   mbedtls_ssl_cache_set);
#endif

    mbedtls_ssl_conf_ca_chain(&conf, srvcert.next, NULL);
    if ((ret = mbedtls_ssl_conf_own_cert(&conf, &srvcert, &pkey)) != 0)
    {
      ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n\n", ret);
      goto exit;
    }

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
      ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
      goto exit;
    }

#if defined(MBEDTLS_TIMING_C)
    mbedtls_ssl_set_timer_cb(&ssl, &timer, mbedtls_timing_set_delay,
                             mbedtls_timing_get_delay);
#endif

    ESP_LOGI(HTTPSD_TAG, "OK");

    // Send an event which says "this task is initialized".
    res = esp_event_post_to(epaper_idf_httpsd_event_loop_handle, EPAPER_IDF_HTTPSD_EVENT, EPAPER_IDF_HTTPSD_EVENT_HTTPS_INITIALIZED, NULL, 0, portMAX_DELAY);
    if (res != ESP_OK)
    {
      ESP_LOGE(HTTPSD_TAG, "Sending event failed");
    }

    epaper_idf_httpsd_is_init = true;

  reset:
#ifdef MBEDTLS_ERROR_C
    if (ret != 0)
    {
      char error_buf[200];
      mbedtls_strerror(ret, error_buf, 200);
      ESP_LOGI(HTTPSD_TAG, "Last error was: %d - %s\n\n", ret, error_buf);
    }
#endif

    mbedtls_net_free(&client_fd);

    mbedtls_ssl_session_reset(&ssl);
  } /** END: if (!epaper_idf_httpsd_is_init) */

  /*
	 * 3. Wait until a client connects
	 */
  ESP_LOGI(HTTPSD_TAG, "  . Waiting for a remote connection ...");
  if ((ret = mbedtls_net_accept(&listen_fd, &client_fd,
                                NULL, 0, NULL)) != 0)
  {
    ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_net_accept returned %d\n\n", ret);
    goto exit;
  }
  mbedtls_ssl_set_bio(&ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, NULL);
  ESP_LOGI(HTTPSD_TAG, "OK");

  /*
	 * 5. Handshake
	 */
  ESP_LOGI(HTTPSD_TAG, "  . Performing the SSL/TLS handshake...");
  while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
  {
    if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_ssl_handshake returned %d\n\n", ret);
      goto reset;
    }
  }
  ESP_LOGI(HTTPSD_TAG, "OK");

  /*
	 * 6. Read the HTTP Request
	 */
  ESP_LOGI(HTTPSD_TAG, "  < Read from client:");
  do
  {
    len = sizeof(buf) - 1;
    memset(buf, 0, sizeof(buf));
    ret = mbedtls_ssl_read(&ssl, buf, len);

    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
      continue;

    if (ret <= 0)
    {
      switch (ret)
      {
      case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
        ESP_LOGI(HTTPSD_TAG, " connection was closed gracefully\n");
        break;

      case MBEDTLS_ERR_NET_CONN_RESET:
        ESP_LOGI(HTTPSD_TAG, " connection was reset by peer\n");
        break;

      default:
        ESP_LOGI(HTTPSD_TAG, " mbedtls_ssl_read returned -0x%x\n", -ret);
        break;
      }

      break;
    }

    len = ret;
    ESP_LOGI(HTTPSD_TAG, " %d bytes read\n\n%s", len, (char *)buf);

    if (ret > 0)
      break;
  } while (1);

  static const char *resp_body = "<a href=\"https://" CONFIG_LWIP_LOCAL_HOSTNAME "\">https://" CONFIG_LWIP_LOCAL_HOSTNAME "</a>";

  /** NOTE: 7. Write the 200 Response
      Example handler: "GET /[*]" */
  ESP_LOGI(HTTPSD_TAG, "  > Write to client:");

  size_t resp_body_size = (strlen(resp_body) + 2) * sizeof(char);

  len = snprintf(
      (char *)buf,
      strlen(EPAPER_IDF_HTTPSD_HTTPS_RESPONSE) * sizeof(char) +
          strlen(resp_proto) * sizeof(char) +
          strlen(resp_status) * sizeof(char) +
          strlen(resp_header_content_type) * sizeof(char) +
          resp_body_size,
      EPAPER_IDF_HTTPSD_HTTPS_RESPONSE,
      resp_proto,
      resp_status,
      resp_header_content_type,
      resp_body_size,
      resp_body);

  written = 0;
  frags = 0;

  while (written < len)
  {
    while ((ret = mbedtls_ssl_write(&ssl, buf + written, len - written)) <= 0)
    {
      if (ret == MBEDTLS_ERR_NET_CONN_RESET)
      {
        mbedtls_printf(" failed\n  ! peer closed the connection\n\n");
        goto reset;
      }

      if (!mbedtls_status_is_ssl_in_progress(ret))
      {
        mbedtls_printf(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
        goto reset;
      }

      /** TODO: For event-driven IO, wait for socket to become available */
      if (DFL_EVENT == 1 /* level triggered IO */)
      {
#if defined(MBEDTLS_TIMING_C)
        idle(&client_fd, &timer, ret);
#else
        idle(&client_fd, ret);
#endif
      }

      if (ret == MBEDTLS_ERR_NET_CONN_RESET)
      {
        ESP_LOGI(HTTPSD_TAG, " failed\n  ! peer closed the connection\n\n");
        goto reset;
      }

      if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
      {
        ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
        goto exit;
      }
    }

    written += ret;
    frags++;
  }

  len = ret;
  ESP_LOGI(HTTPSD_TAG, " %d bytes written\n\n%s\n", len, (char *)buf);

  ESP_LOGI(HTTPSD_TAG, "Closing the connection...");

  while ((ret = mbedtls_ssl_close_notify(&ssl)) < 0)
  {
    if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
        ret != MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      ESP_LOGI(HTTPSD_TAG, " failed\n  ! mbedtls_ssl_close_notify returned %d\n\n", ret);
      goto reset;
    }
  }
  ESP_LOGI(HTTPSD_TAG, "OK");

  ret = 0;
  goto reset;

exit:

#ifdef MBEDTLS_ERROR_C
  if (ret != 0)
  {
    char error_buf[100];
    mbedtls_strerror(ret, error_buf, 100);
    ESP_LOGI(HTTPSD_TAG, "Last error was: %d - %s\n\n", ret, error_buf);
  }
#endif

  mbedtls_net_free(&client_fd);
  mbedtls_net_free(&listen_fd);

  mbedtls_x509_crt_free(&srvcert);
  mbedtls_pk_free(&pkey);
  mbedtls_ssl_free(&ssl);
  mbedtls_ssl_config_free(&conf);
#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_cache_free(&cache);
#endif
  mbedtls_ctr_drbg_free(&ctr_drbg);
  mbedtls_entropy_free(&entropy);

  ESP_LOGI(HTTPSD_TAG, "Closing Task");
  vTaskDelete(NULL);
}

void epaper_idf_httpsd_task(void *pvParameter)
{
  epaper_idf_httpsd_is_init = false;

  // epaper_idf_httpsd_is_init = false;

  // while (1)
  // {

  start_httpsd(NULL);

  // while (1)
  // {
  //   vTaskDelay(1000 / portTICK_PERIOD_MS);
  // }

  vTaskDelete(NULL);

  // }
}
