/* epaper-idf - ota.c

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is in the Public Domain (or CC0 licensed, at your option.)

		Unless required by applicable law or agreed to in writing, this
		software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
		CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "nvs.h"

#if CONFIG_PROJECT_CONNECT_WIFI
#include "esp_wifi.h"
#endif

extern QueueHandle_t epaper_idf_taskqueue;

static const char *TAG = "ota";
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

#define OTA_URL_SIZE 256

static esp_err_t validate_image_header(esp_app_desc_t *new_app_info)
{
  if (new_app_info == NULL)
  {
    return ESP_ERR_INVALID_ARG;
  }

  const esp_partition_t *running = esp_ota_get_running_partition();
  esp_app_desc_t running_app_info;
  if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK)
  {
    ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
  }

#ifndef CONFIG_PROJECT_SKIP_VERSION_CHECK
  if (memcmp(new_app_info->version, running_app_info.version, sizeof(new_app_info->version)) == 0)
  {
    ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
    return ESP_FAIL;
  }
#endif

  return ESP_OK;
}

void ota_task(void *pvParameter)
{
  if (epaper_idf_taskqueue == 0)
  {
    printf("Task queue is not ready.\n");
    return;
  }

  ESP_LOGI(TAG, "Starting OTA");

  esp_err_t ota_finish_err = ESP_OK;
  esp_http_client_config_t config = {
      .url = CONFIG_PROJECT_FIRMWARE_UPGRADE_URL,
      .cert_pem = (char *)server_cert_pem_start,
      .timeout_ms = CONFIG_PROJECT_OTA_RECV_TIMEOUT,
      .keep_alive_enable = true,
  };

#ifdef CONFIG_PROJECT_FIRMWARE_UPGRADE_URL_FROM_STDIN
  char url_buf[OTA_URL_SIZE];
  if (strcmp(config.url, "FROM_STDIN") == 0)
  {
    example_configure_stdin_stdout();
    fgets(url_buf, OTA_URL_SIZE, stdin);
    int len = strlen(url_buf);
    url_buf[len - 1] = '\0';
    config.url = url_buf;
  }
  else
  {
    ESP_LOGE(TAG, "Configuration mismatch: wrong firmware upgrade image url");
    abort();
  }
#endif

#ifdef CONFIG_PROJECT_SKIP_COMMON_NAME_CHECK
  config.skip_cert_common_name_check = true;
#endif

  esp_https_ota_config_t ota_config = {
      .http_config = &config,
  };

  esp_https_ota_handle_t https_ota_handle = NULL;
  esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "ESP HTTPS OTA Begin failed");
    // vTaskDelete(NULL);
    goto ota_end;
  }

  esp_app_desc_t app_desc;
  err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "esp_https_ota_read_img_desc failed");
    goto ota_end;
  }
  err = validate_image_header(&app_desc);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "image header verification failed");
    goto ota_end;
  }

  while (1)
  {
    err = esp_https_ota_perform(https_ota_handle);
    if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS)
    {
      break;
      // while (1)
      // {
      // printf("OTA update not needed. Moving on...");
      // printf("Sending task queue value: %lu \n", counter);
      // xQueueSend(epaper_idf_taskqueue, (void *)NULL, (TickType_t)0);
      // xQueueSend(epaper_idf_taskqueue, (void *)&counter, (TickType_t)0);
      // counter++;
      // }
    }

    // esp_https_ota_perform returns after every read operation which gives user the ability to
    // monitor the status of OTA upgrade by calling esp_https_ota_get_image_len_read, which gives length of image
    // data read so far.
    ESP_LOGD(TAG, "Image bytes read: %d", esp_https_ota_get_image_len_read(https_ota_handle));
  }

  if (esp_https_ota_is_complete_data_received(https_ota_handle) != true)
  {
    // the OTA image was not completely received and user can customise the response to this situation.
    ESP_LOGE(TAG, "Complete data was not received.");
  }

ota_end:
  ota_finish_err = esp_https_ota_finish(https_ota_handle);
  if ((err == ESP_OK) && (ota_finish_err == ESP_OK))
  {
    ESP_LOGI(TAG, "ESP_HTTPS_OTA upgrade successful. Rebooting ...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_restart();
  }
  else
  {
    if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED)
    {
      ESP_LOGE(TAG, "Image validation failed, image is corrupted");
    }
    ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed %d", ota_finish_err);

    unsigned long start = 1;

    xQueueSend(epaper_idf_taskqueue, (void *)&start, (TickType_t)0);

    while (1)
    {
      vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
    // vTaskDelete(NULL);
  }
}
