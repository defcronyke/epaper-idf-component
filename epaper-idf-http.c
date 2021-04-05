/*  epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "epaper-idf-task.h"

// #if CONFIG_PROJECT_CONNECT_WIFI
// #include "esp_wifi.h"
// #endif

static const char *TAG = "epaper-idf-http";

#define WEB_SERVER CONFIG_EPAPER_IDF_IMAGES_INDEX_JSON_URL_HOST
#define WEB_PORT "443"
#define WEB_URL CONFIG_EPAPER_IDF_IMAGES_INDEX_JSON_URL

static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
  "Host: "WEB_SERVER"\r\n"
  "User-Agent: esp-idf/1.0 esp32\r\n"
  "\r\n";

void epaper_idf_http_get_https()
{
  char buf[512];
  int ret, len;

  esp_tls_cfg_t cfg = {
    .crt_bundle_attach = esp_crt_bundle_attach,
  };

  struct esp_tls *tls = esp_tls_conn_http_new(WEB_URL, &cfg);

  if (tls != NULL)
  {
    ESP_LOGI(TAG, "Connection established...");
  }
  else
  {
    ESP_LOGE(TAG, "Connection failed...");
    goto exit;
  }

  size_t written_bytes = 0;
  do
  {
    printf("!!!!  WEB_URL: %s  !!!!\n", WEB_URL);
    printf("!!!!  REQUEST: %s  !!!!\n", REQUEST);
    // printf("!!!!  ret: %d  !!!!\n", ret);

    ret = esp_tls_conn_write(tls,
      REQUEST + written_bytes,
      strlen(REQUEST) - written_bytes);
    if (ret >= 0)
    {
      ESP_LOGI(TAG, "%d bytes written", ret);
      written_bytes += ret;
    }
    else if (ret != ESP_TLS_ERR_SSL_WANT_READ && ret != ESP_TLS_ERR_SSL_WANT_WRITE)
    {
      ESP_LOGE(TAG, "esp_tls_conn_write  returned 0x%x", ret);
      goto exit;
    } 
    // else {
    //   printf("!!!!  WEB_URL: %s  !!!!\n", WEB_URL);
    //   printf("!!!!  REQUEST: %s  !!!!\n", REQUEST);
    //   printf("!!!!  ret: %d  !!!!\n", ret);
    // }
  } while (written_bytes < strlen(REQUEST));

  ESP_LOGI(TAG, "Reading HTTP response...");

  do
  {
    len = sizeof(buf) - 1;
    bzero(buf, sizeof(buf));
    ret = esp_tls_conn_read(tls, (char *)buf, len);

    if (ret == ESP_TLS_ERR_SSL_WANT_WRITE || ret == ESP_TLS_ERR_SSL_WANT_READ)
      continue;

    if (ret < 0)
    {
      ESP_LOGE(TAG, "esp_tls_conn_read  returned -0x%x", -ret);
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

exit:
  esp_tls_conn_delete(tls);
  putchar('\n'); // JSON output doesn't have a newline at end

  static int request_count;
  ESP_LOGI(TAG, "Completed %d requests", ++request_count);

  for (int countdown = 10; countdown >= 0; countdown--)
  {
    ESP_LOGI(TAG, "%d...", countdown);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  ESP_LOGI(TAG, "Starting again!");
}

void epaper_idf_http_task(void *pvParameter) {
  if (epaper_idf_taskqueue_http == 0)
  {
    printf("Task queue http is not ready.\n");
    return;
  }

  if (epaper_idf_taskqueue_ota == 0)
  {
    printf("Task queue http is not ready.\n");
    return;
  }

  // Wait for other task to finish first.
  unsigned long start = 0;
  while (start != 1)
  {
    xQueueReceive(epaper_idf_taskqueue_ota, &start, (TickType_t)(1000 / portTICK_PERIOD_MS));
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  epaper_idf_http_get_https();

  xQueueSend(epaper_idf_taskqueue_http, (void *)&start, (TickType_t)0);

  while(1) {
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}
