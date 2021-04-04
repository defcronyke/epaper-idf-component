/*  epaper-idf-component - example - http-slideshow

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/
#include <stdio.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "protocol_examples_common.h"
#include "epaper-idf-util.h"
#include "epaper-idf-gfx.h"
#include "epaper-idf-spi.h"
#include "epaper-idf-ota.h"

const char *TAG = "http-slideshow";
const char *task_name = "http_slideshow_task";
QueueHandle_t epaper_idf_taskqueue = NULL;

void http_slideshow_task(void *pvParameter)
{
  // Wait for task queue to be initialized first.
  if (epaper_idf_taskqueue == NULL)
  {
    ESP_LOGI(TAG, "Task queue is not ready.\n");
    return;
  }

  // Use the appropriate epaper device.
  EpaperIDFSPI io;
  EpaperIDFDevice dev(io);

  unsigned long start = 0;

  while (1)
  {
    // Wait for OTA task to finish first.
    while (start != 1)
    {
      xQueueReceive(epaper_idf_taskqueue, &start, (TickType_t)(1000 / portTICK_PERIOD_MS));
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG, "%s loop", task_name);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void http_slideshow(void)
{
  // Initialize NVS.
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    // 1.OTA app partition table has a smaller NVS partition size than the non-OTA
    // partition table. This size mismatch may cause NVS initialization to fail.
    // 2.NVS partition contains data in new format and cannot be recognized by this version of code.
    // If this happens, we erase NVS partition and initialize NVS again.
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
		 * Read "Establishing Wi-Fi or Ethernet Connection" section in
		 * examples/protocols/README.md for more information about this function.
		*/
  ESP_ERROR_CHECK(example_connect());

#if CONFIG_PROJECT_CONNECT_WIFI
  /* Ensure to disable any WiFi power save mode, this allows best throughput
		 * and hence timings for overall OTA operation.
		 */
  esp_wifi_set_ps(WIFI_PS_NONE);
#endif // CONFIG_PROJECT_CONNECT_WIFI

  ESP_LOGI(TAG, "Creating task queue...");

  epaper_idf_taskqueue = xQueueCreate(20, sizeof(unsigned long));
  if (epaper_idf_taskqueue == NULL)
  {
    ESP_LOGE(TAG, "Task queue creation failed.");
    return;
  }

  ESP_LOGI(TAG, "Task queue created.");

  // TODO: Do we need to wait for 1 second here?
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  xTaskCreate(&http_slideshow_task, task_name, 4096 * 8, NULL, 5, NULL);
  ESP_LOGI(TAG, "Task started: %s", task_name);

  xTaskCreate(&ota_task, ota_task_name, 1024 * 8, NULL, 5, NULL);
  ESP_LOGI(TAG, "Task started: %s", ota_task_name);
}
