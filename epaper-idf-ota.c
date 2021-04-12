/**	epaper-idf-component

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
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "nvs.h"
#if CONFIG_PROJECT_CONNECT_WIFI
#include "esp_wifi.h"
#endif
#include "esp_event.h"
#include "esp_event_base.h"
#include "epaper-idf-ota.h"

#define OTA_URL_SIZE 256

static const char *TAG = "ota";
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

esp_event_loop_handle_t epaper_idf_ota_event_loop_handle;

ESP_EVENT_DEFINE_BASE(EPAPER_IDF_OTA_EVENT);

static esp_err_t validate_image_header(esp_app_desc_t *new_app_info, esp_app_desc_t *running_app_info)
{
	if (new_app_info == NULL || running_app_info == NULL)
	{
		return ESP_ERR_INVALID_ARG;
	}

#ifndef CONFIG_PROJECT_SKIP_VERSION_CHECK
	if (memcmp(new_app_info->version, running_app_info->version, sizeof(new_app_info->version)) == 0)
	{
		ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
		return ESP_FAIL;
	}
#endif

	return ESP_OK;
}

void epaper_idf_ota_task(void *pvParameter)
{
	while (1) {
		while (1)
		{
			ESP_LOGI(TAG, "Starting OTA Task");

			const esp_partition_t *running = esp_ota_get_running_partition();
			esp_app_desc_t running_app_info;
			if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK)
			{
				ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
			}

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
				vTaskDelay(1000 / portTICK_PERIOD_MS);
				continue;
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
				goto ota_end;
			}

			esp_app_desc_t app_desc;
			err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
			if (err != ESP_OK)
			{
				ESP_LOGE(TAG, "esp_https_ota_read_img_desc failed");
				goto ota_end;
			}
			err = validate_image_header(&app_desc, &running_app_info);
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
			}

			// Send an event which says "this task is finished".
			err = esp_event_post_to(epaper_idf_ota_event_loop_handle, EPAPER_IDF_OTA_EVENT, EPAPER_IDF_OTA_EVENT_FINISH, NULL, 0, portMAX_DELAY);
			if (err != ESP_OK) {
				ESP_LOGE(TAG, "Sending event failed");
			}

			break;
		}

		vTaskDelete(NULL);
	}
}
