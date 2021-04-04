/*  epaper-idf-component - example - http-slideshow

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "driver/rtc_io.h"
#include "soc/rtc.h"
#include "protocol_examples_common.h"
#include "epaper-idf-task.h"
#include "epaper-idf-device.h"
#include "task/epaper-idf-task-http-slideshow.h"
#include "epaper-idf-gfx.h"
#include "epaper-idf-spi.h"
#include "epaper-idf-ota.h"

#define EPAPER_IDF_DEEP_SLEEP_SECONDS_POS_MIN 15
#define EPAPER_IDF_DEEP_SLEEP_SECONDS_NEG_MAX -15

const char *TAG = "http-slideshow";
const char *task_name = "http_slideshow_task";
QueueHandle_t epaper_idf_taskqueue = NULL;

void http_slideshow_task_main(void) {
  ESP_LOGI(TAG, "task main");
}

void http_slideshow_task(void *pvParameter)
{
  // Wait for task queue to be initialized first.
  if (epaper_idf_taskqueue == NULL)
  {
    ESP_LOGI(TAG, "Task queue is not ready.");
    return;
  }

  struct timeval now;
  gettimeofday(&now, NULL);

  int32_t delay_secs = (int32_t)CONFIG_EPAPER_IDF_DEEP_SLEEP_SECONDS;
  bool no_deep_sleep = false;
  if (delay_secs < 0) {
    no_deep_sleep = true;
    delay_secs = (int32_t)epaper_idf_clamp((float)CONFIG_EPAPER_IDF_DEEP_SLEEP_SECONDS, (float)INT32_MIN, (float)EPAPER_IDF_DEEP_SLEEP_SECONDS_NEG_MAX) * -1;
  } else {
    delay_secs = (int32_t)epaper_idf_clamp((float)CONFIG_EPAPER_IDF_DEEP_SLEEP_SECONDS, (float)EPAPER_IDF_DEEP_SLEEP_SECONDS_POS_MIN, (float)INT32_MAX);
  }

  vTaskDelay(1000 / portTICK_PERIOD_MS);

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

    if (no_deep_sleep) {
      http_slideshow_task_main();

      ESP_LOGI(TAG, "waiting for %d secs\n", delay_secs);

      vTaskDelay((delay_secs * 1000) / portTICK_PERIOD_MS);

    } else {
      http_slideshow_task_main();

      ESP_LOGI(TAG, "deep sleeping for approx: %d secs", delay_secs);

      // Disable wifi for deep sleep.
      esp_wifi_stop();

      ESP_LOGI(TAG, "Enabling deep sleep timer wakeup after approx: %d secs", delay_secs);
      esp_sleep_enable_timer_wakeup(delay_secs * 1000000);

      // Isolate GPIO12 pin from external circuits. This is needed for modules
      // which have an external pull-up resistor on GPIO12 (such as ESP32-WROVER)
      // to minimize current consumption.
      rtc_gpio_isolate(GPIO_NUM_12);
      rtc_gpio_isolate((gpio_num_t)CONFIG_EPAPER_IDF_SPI_MOSI);
      rtc_gpio_isolate((gpio_num_t)CONFIG_EPAPER_IDF_SPI_CLK);
      rtc_gpio_isolate((gpio_num_t)CONFIG_EPAPER_IDF_SPI_CS);
      rtc_gpio_isolate((gpio_num_t)CONFIG_EPAPER_IDF_DC);
      rtc_gpio_isolate((gpio_num_t)CONFIG_EPAPER_IDF_RST);
      rtc_gpio_isolate((gpio_num_t)CONFIG_EPAPER_IDF_BUSY);

      // Hibernate for lowest power consumption.
      esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
      esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
      esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
      esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);

      ESP_LOGI(TAG, "entering deep sleep...\n");

      esp_deep_sleep_start();
    }

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

  xTaskCreate(&epaper_idf_ota_task, epaper_idf_ota_task_name, 1024 * 8, NULL, 5, NULL);
  ESP_LOGI(TAG, "Task started: %s", epaper_idf_ota_task_name);
}
