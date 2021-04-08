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
#include "protocol_examples_common.h"

esp_event_loop_handle_t epaper_idf_wifi_event_loop_handle;

ESP_EVENT_DEFINE_BASE(EPAPER_IDF_WIFI_EVENT);

const char *epaper_idf_wifi_tag = "epaper-idf-wifi";

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

static void epaper_idf_wifi_connect(void) {
	/*	This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
			Read "Establishing Wi-Fi or Ethernet Connection" section in
			$IDF_PATH/examples/protocols/README.md for more information about this function.
	*/
	ESP_ERROR_CHECK(example_connect());

#if CONFIG_PROJECT_CONNECT_WIFI
	/* Ensure to disable any WiFi power save mode, this allows best throughput
		 and hence timings for overall OTA operation.
	*/
	esp_wifi_set_ps(WIFI_PS_NONE);
#endif // CONFIG_PROJECT_CONNECT_WIFI
}

static void epaper_idf_wifi_disconnect(void) {
	ESP_ERROR_CHECK(example_disconnect());
}

// Attempt to connect to wifi each time this task runs.
void epaper_idf_wifi_task(void* pvParameter) {	
	bool wifi_is_init = false;

	enum epaper_idf_wifi_task_action_t action = *(enum epaper_idf_wifi_task_action_t*)pvParameter;

	while (1) {
		switch (action) {
			/** Initialize (if necessary), and connect to a wifi network. */
			case EPAPER_IDF_WIFI_TASK_ACTION_CONNECT: {
				ESP_LOGI(epaper_idf_wifi_tag, "running epaper idf wifi task action: EPAPER_IDF_WIFI_TASK_ACTION_CONNECT");

				// Init only once.
				if (!wifi_is_init) {
					ESP_LOGI(epaper_idf_wifi_tag, "wifi initializing...");
					epaper_idf_wifi_init();
					wifi_is_init = true;
				}

				ESP_LOGI(epaper_idf_wifi_tag, "attempting to connect to wifi network...");

				// Connect to wifi.
				epaper_idf_wifi_connect();

				break;
			}

			/** Disconnect wifi interface. */
			case EPAPER_IDF_WIFI_TASK_ACTION_DISCONNECT: {
				ESP_LOGI(epaper_idf_wifi_tag, "running epaper idf wifi task action: EPAPER_IDF_WIFI_TASK_ACTION_DISCONNECT");

				// Disconnect wifi.
				epaper_idf_wifi_disconnect();

				break;
			}

			/** Disconnect and stop wifi interface. */
			case EPAPER_IDF_WIFI_TASK_ACTION_STOP: {
				ESP_LOGI(epaper_idf_wifi_tag, "running epaper idf wifi task action: EPAPER_IDF_WIFI_TASK_ACTION_DISCONNECT");

				// Disconnect wifi.
				epaper_idf_wifi_disconnect();

				// Stop wifi interface.
				esp_wifi_stop();

				break;
			}

			default: {
				break;
			}
		}

		// Send an event which says "this task is finished".
		ESP_ERROR_CHECK(esp_event_post_to(epaper_idf_wifi_event_loop_handle, EPAPER_IDF_WIFI_EVENT, EPAPER_IDF_WIFI_EVENT_FINISH, NULL, 0, portMAX_DELAY));

		vTaskDelete(NULL);
	}
}
