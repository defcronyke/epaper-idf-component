/*	epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.

		Contains some modified example code from here:
		https://github.com/espressif/esp-idf/blob/release/v4.2/examples/system/ota/advanced_https_ota/main/advanced_https_ota_example.c

		Original Example Code Header:
		This code is in the Public Domain (or CC0 licensed, at your option.)

		Unless required by applicable law or agreed to in writing, this
		software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
		CONDITIONS OF ANY KIND, either express or implied.
*/
#include "epaper-idf-wifi.h"
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#if CONFIG_EXAMPLE_CONNECT_WIFI
#include "esp_wifi.h"
#endif

esp_event_loop_handle_t epaper_idf_wifi_event_loop_handle;

ESP_EVENT_DEFINE_BASE(EPAPER_IDF_WIFI_EVENT);

static void epaper_idf_wifi_init(void) {
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
}

void epaper_idf_wifi_task(void* pvParameter) {
	while (1) {
		epaper_idf_wifi_init();

		// Send an event which says "this task is finished".
		ESP_ERROR_CHECK(esp_event_post_to(epaper_idf_wifi_event_loop_handle, EPAPER_IDF_WIFI_EVENT, EPAPER_IDF_WIFI_EVENT_FINISH, NULL, 0, portMAX_DELAY));

		vTaskDelete(NULL);
	}
}
