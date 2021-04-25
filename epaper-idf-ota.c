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
#include "esp_vfs_semihost.h"
#include "esp_spiffs.h"
#include "sdmmc_cmd.h"
#if CONFIG_EXAMPLE_WEB_DEPLOY_SD
#include "driver/sdmmc_host.h"
#endif
#include "esp_vfs_fat.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "nvs.h"
#include "nvs_flash.h"
#if CONFIG_PROJECT_CONNECT_WIFI
#include "esp_wifi.h"
#endif
#include "esp_event.h"
#include "esp_event_base.h"
#include "errno.h"
#include "epaper-idf-ota.h"
#include "epaper-idf-httpd.h"

#define OTA_URL_SIZE 256
#define BUFFSIZE 1024
#define HASH_LEN 32

static const char *TAG = "ota";

// NOTE: This has to match the value in the partition table file: partitions.csv
static const uint spiffs_partition_www_size = 0xbd000;

static char ota_write_data[BUFFSIZE + 1] = { 0 };
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

esp_event_loop_handle_t epaper_idf_ota_event_loop_handle;

ESP_EVENT_DEFINE_BASE(EPAPER_IDF_OTA_EVENT);

static bool diagnostic(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type    = GPIO_PIN_INTR_DISABLE;
    io_conf.mode         = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en   = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "Diagnostics (5 sec)...");
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    bool diagnostic_is_ok = gpio_get_level(CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);

    gpio_reset_pin(CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);
    return diagnostic_is_ok;
}

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

static void print_sha256(const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    ESP_LOGI(TAG, "%s: %s", label, hash_print);
}

static void http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

static void ota_partition2() {
    esp_err_t err;
    
    const esp_partition_t *update_partition = NULL;

    ESP_LOGI(TAG, "Starting OTA www");

    const esp_partition_t *running = esp_ota_get_running_partition();

    ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)",
             running->type, running->subtype, running->address);

    esp_http_client_config_t config = {
        .url = CONFIG_PROJECT_FIRMWARE_UPGRADE_URL2,
        .cert_pem = (char *)server_cert_pem_start,
        .timeout_ms = CONFIG_PROJECT_OTA_RECV_TIMEOUT,
    };

#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL_FROM_STDIN
    char url_buf[OTA_URL_SIZE];
    if (strcmp(config.url, "FROM_STDIN") == 0) {
        example_configure_stdin_stdout();
        fgets(url_buf, OTA_URL_SIZE, stdin);
        int len = strlen(url_buf);
        url_buf[len - 1] = '\0';
        config.url = url_buf;
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong firmware upgrade image url");
        // esp_restart();
        return;
        // abort();
    }
#endif

#ifdef CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true;
#endif

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialise HTTP connection");
        // esp_restart();
        return;
        // task_fatal_error();
    }
    err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        // esp_restart();
        return;
        // task_fatal_error();
    }
    esp_http_client_fetch_headers(client);

    update_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, "www");

    ESP_LOGI(TAG, "Erasing SPIFFS partition");
    err = esp_partition_erase_range(update_partition, 0, spiffs_partition_www_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to erase SPIFFS partition: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        // esp_restart();
        return;
        // task_fatal_error();
    }

    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);
    assert(update_partition != NULL);

    int binary_file_length = 0;
    /*deal with all receive packet*/
    bool image_header_was_checked = false;

    unsigned int i = 0;

    while (1) {
        int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
        if (data_read < 0) {
            ESP_LOGE(TAG, "Error: SSL data read error");
            http_cleanup(client);
            // esp_restart();
            return;
            // task_fatal_error();
        } else if (data_read > 0) {
            if (image_header_was_checked == false) {
                if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {

                } else {
                    ESP_LOGE(TAG, "received package is not fit len");
                    http_cleanup(client);
                    // esp_restart();
                    return;
                    // task_fatal_error();
                }
            }

            err = esp_partition_write(update_partition, data_read*i, (const void *)ota_write_data, data_read);
            if (err != ESP_OK) {
                http_cleanup(client);
                // esp_restart();
                return;
                // task_fatal_error();
            }
            binary_file_length += data_read;
            ESP_LOGD(TAG, "Written image length %d", binary_file_length);

        } else if (data_read == 0) {
           /*
            * As esp_http_client_read never returns negative error code, we rely on
            * `errno` to check for underlying transport connectivity closure if any
            */
            if (errno == ECONNRESET || errno == ENOTCONN) {
                ESP_LOGE(TAG, "Connection closed, errno = %d", errno);
                break;
            }
            if (esp_http_client_is_complete_data_received(client) == true) {
                ESP_LOGI(TAG, "Connection closed");
                break;
            }
        }

        // Increment write offset counter.
        i++;
    }
    ESP_LOGI(TAG, "Total Write binary data length: %d", binary_file_length);
    if (esp_http_client_is_complete_data_received(client) != true) {
        ESP_LOGE(TAG, "Error in receiving complete file");
        http_cleanup(client);
        // esp_restart();
        return;
        // task_fatal_error();
    }

    ESP_LOGI(TAG, "Prepare to restart system!");
    // esp_restart();
    return;
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
				// Deploy spiffs partition
				ota_partition2();

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

			ESP_ERROR_CHECK(init_fs());

			uint8_t sha_256[HASH_LEN] = { 0 };
			esp_partition_t partition;

			// get sha256 digest for the partition table
			partition.address   = ESP_PARTITION_TABLE_OFFSET;
			partition.size      = ESP_PARTITION_TABLE_MAX_LEN;
			partition.type      = ESP_PARTITION_TYPE_DATA;
			esp_partition_get_sha256(&partition, sha_256);
			print_sha256(sha_256, "SHA-256 for the partition table: ");

			// get sha256 digest for bootloader
			partition.address   = ESP_BOOTLOADER_OFFSET;
			partition.size      = ESP_PARTITION_TABLE_OFFSET;
			partition.type      = ESP_PARTITION_TYPE_APP;
			esp_partition_get_sha256(&partition, sha_256);
			print_sha256(sha_256, "SHA-256 for bootloader: ");

			// get sha256 digest for running partition
			esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256);
			print_sha256(sha_256, "SHA-256 for current firmware: ");

			const esp_partition_t *running2 = esp_ota_get_running_partition();
			esp_ota_img_states_t ota_state;
			if (esp_ota_get_state_partition(running2, &ota_state) == ESP_OK) {
					if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
							// run diagnostic function ...
							bool diagnostic_is_ok = diagnostic();
							if (diagnostic_is_ok) {
									ESP_LOGI(TAG, "Diagnostics completed successfully! Continuing execution ...");
									esp_ota_mark_app_valid_cancel_rollback();
							} else {
									ESP_LOGE(TAG, "Diagnostics failed! Start rollback to the previous version ...");
									esp_ota_mark_app_invalid_rollback_and_reboot();
							}
					}
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
