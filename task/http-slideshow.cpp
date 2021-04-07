/*  epaper-idf-component - example - http-slideshow

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/
#include "task/http-slideshow.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
// #include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_system.h"
// #include "nvs_flash.h"
#include "esp_event.h"
#include "esp_sleep.h"
#if CONFIG_EXAMPLE_CONNECT_WIFI
#include "esp_wifi.h"
#endif
#include "driver/rtc_io.h"
#include "soc/rtc.h"
#include "protocol_examples_common.h"
#include "epaper-idf-device.h"
#include "epaper-idf-gfx.h"
#include "epaper-idf-spi.h"
// #include "epaper-idf-task.h"
#include "epaper-idf-wifi.h"
#include "epaper-idf-ota.h"
#include "epaper-idf-http.h"

#ifndef EPAPER_IDF_DEEP_SLEEP_SECONDS_POS_MIN
#define EPAPER_IDF_DEEP_SLEEP_SECONDS_POS_MIN 15
#endif

#ifndef EPAPER_IDF_DEEP_SLEEP_SECONDS_NEG_MAX
#define EPAPER_IDF_DEEP_SLEEP_SECONDS_NEG_MAX -15
#endif

extern "C" void http_slideshow_task(void *pvParameter);

const char *TAG = "http-slideshow";

const char *epaper_idf_wifi_task_name = "epaper_idf_wifi_task";
const uint32_t epaper_idf_wifi_task_stack_depth = 2048;
UBaseType_t epaper_idf_wifi_task_priority = 5;

const char *epaper_idf_ota_task_name = "epaper_idf_ota_task";
const uint32_t epaper_idf_ota_task_stack_depth = 2048;
UBaseType_t epaper_idf_ota_task_priority = 5;

const char *epaper_idf_http_task_name = "epaper_idf_http_task";
const uint32_t epaper_idf_http_task_stack_depth = 2048;
UBaseType_t epaper_idf_http_task_priority = 5;

const char *http_slideshow_task_name = "http_slideshow_task";
const uint32_t http_slideshow_task_stack_depth = 4096;
UBaseType_t http_slideshow_task_priority = 5;

static void epaper_idf_wifi_finish_event_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
	ESP_LOGI(TAG, "event received: EPAPER_IDF_WIFI_EVENT_FINISH");

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
}

static void sta_got_ip_event_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
	ESP_LOGI(TAG, "event received: IP_EVENT_STA_GOT_IP");

	xTaskCreate(&epaper_idf_ota_task, epaper_idf_ota_task_name, epaper_idf_ota_task_stack_depth * 8, NULL, epaper_idf_ota_task_priority, NULL);
	ESP_LOGI(TAG, "Task started: %s", epaper_idf_ota_task_name);
}

static void epaper_idf_ota_finish_event_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
	ESP_LOGI(TAG, "event received: EPAPER_IDF_OTA_EVENT_FINISH");

	xTaskCreate(&epaper_idf_http_task, epaper_idf_http_task_name, epaper_idf_http_task_stack_depth * 8, NULL, epaper_idf_http_task_priority, NULL);
	ESP_LOGI(TAG, "Task started: %s", epaper_idf_http_task_name);
}

static void epaper_idf_http_finish_event_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
	ESP_LOGI(TAG, "event received: EPAPER_IDF_HTTP_EVENT_FINISH");

	xTaskCreate(&http_slideshow_task, http_slideshow_task_name, http_slideshow_task_stack_depth * 8, NULL, http_slideshow_task_priority, NULL);
	ESP_LOGI(TAG, "Task started: %s", http_slideshow_task_name);
}

static void http_slideshow_task_main(void)
{
	ESP_LOGI(TAG, "task main");

	// Use the appropriate epaper device.
	EpaperIDFSPI io;
	EpaperIDFDevice dev(io);
}

// Enter deep sleep.
static void http_slideshow_deep_sleep(int32_t delay_secs)
{
	ESP_LOGI(TAG, "preparing for deep sleep");

	// Disable wifi for deep sleep.
	ESP_ERROR_CHECK(example_disconnect());
	esp_wifi_stop();

	esp_event_handler_unregister_with(epaper_idf_wifi_event_loop_handle, EPAPER_IDF_WIFI_EVENT, EPAPER_IDF_WIFI_EVENT_FINISH, epaper_idf_wifi_finish_event_handler);
	esp_event_loop_delete(epaper_idf_wifi_event_loop_handle);

	esp_event_handler_unregister_with(epaper_idf_ota_event_loop_handle, EPAPER_IDF_OTA_EVENT, EPAPER_IDF_OTA_EVENT_FINISH, epaper_idf_ota_finish_event_handler);
	esp_event_loop_delete(epaper_idf_ota_event_loop_handle);

	esp_event_handler_unregister_with(epaper_idf_http_event_loop_handle, EPAPER_IDF_HTTP_EVENT, EPAPER_IDF_HTTP_EVENT_FINISH, epaper_idf_http_finish_event_handler);
	esp_event_loop_delete(epaper_idf_http_event_loop_handle);

	esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, sta_got_ip_event_handler);
	esp_event_loop_delete_default();

	ESP_LOGI(TAG, "Enabling deep sleep timer wakeup after approx: %d secs", delay_secs);
	esp_sleep_enable_timer_wakeup(delay_secs * 1000000);

	// Isolate GPIO12 pin from external circuits. This is needed for modules
	// which have an external pull-up resistor on GPIO12 (such as ESP32-WROVER)
	// to minimize current consumption.
	rtc_gpio_isolate(GPIO_NUM_12);

	// Hibernate for lowest power consumption.
	esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
	esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
	esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
	esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
	esp_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_OFF);

	ESP_LOGI(TAG, "entering deep sleep...\n");

	esp_deep_sleep_start();
}

extern "C" void http_slideshow_task(void *pvParameter)
{
	while (1)
	{
		struct timeval now;
		gettimeofday(&now, NULL);

		int32_t delay_secs = (int32_t)CONFIG_EPAPER_IDF_DEEP_SLEEP_SECONDS;
		bool no_deep_sleep = false;
		if (delay_secs < 0)
		{
			no_deep_sleep = true;
			delay_secs = (int32_t)epaper_idf_clamp((float)CONFIG_EPAPER_IDF_DEEP_SLEEP_SECONDS, (float)INT32_MIN, (float)EPAPER_IDF_DEEP_SLEEP_SECONDS_NEG_MAX) * -1;
		}
		else
		{
			delay_secs = (int32_t)epaper_idf_clamp((float)CONFIG_EPAPER_IDF_DEEP_SLEEP_SECONDS, (float)EPAPER_IDF_DEEP_SLEEP_SECONDS_POS_MIN, (float)INT32_MAX);
		}

		while (1)
		{
			ESP_LOGI(TAG, "%s loop", http_slideshow_task_name);

			if (no_deep_sleep)
			{
				http_slideshow_task_main();

				ESP_LOGI(TAG, "waiting for %d secs\n", delay_secs);

				vTaskDelay((delay_secs * 1000) / portTICK_PERIOD_MS);
			}
			else
			{
				http_slideshow_task_main();

				// Enter deep sleep.
				http_slideshow_deep_sleep(delay_secs);

				vTaskDelete(NULL);
			}
		}
	}
}

void http_slideshow(void)
{
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, sta_got_ip_event_handler, NULL, NULL));

	esp_event_loop_args_t epaper_idf_wifi_event_loop_args = {
			.queue_size = 5,
			.task_name = "epaper_idf_wifi_event_loop_task", // task will be created
			.task_priority = uxTaskPriorityGet(NULL),
			.task_stack_size = epaper_idf_wifi_task_stack_depth,
			.task_core_id = tskNO_AFFINITY};

	ESP_ERROR_CHECK(esp_event_loop_create(&epaper_idf_wifi_event_loop_args, &epaper_idf_wifi_event_loop_handle));
	ESP_ERROR_CHECK(esp_event_handler_instance_register_with(epaper_idf_wifi_event_loop_handle, EPAPER_IDF_WIFI_EVENT, EPAPER_IDF_WIFI_EVENT_FINISH, epaper_idf_wifi_finish_event_handler, epaper_idf_wifi_event_loop_handle, NULL));

	esp_event_loop_args_t epaper_idf_ota_event_loop_args = {
			.queue_size = 5,
			.task_name = "epaper_idf_ota_event_loop_task", // task will be created
			.task_priority = uxTaskPriorityGet(NULL),
			.task_stack_size = epaper_idf_ota_task_stack_depth,
			.task_core_id = tskNO_AFFINITY};

	ESP_ERROR_CHECK(esp_event_loop_create(&epaper_idf_ota_event_loop_args, &epaper_idf_ota_event_loop_handle));
	ESP_ERROR_CHECK(esp_event_handler_instance_register_with(epaper_idf_ota_event_loop_handle, EPAPER_IDF_OTA_EVENT, EPAPER_IDF_OTA_EVENT_FINISH, epaper_idf_ota_finish_event_handler, epaper_idf_ota_event_loop_handle, NULL));

	esp_event_loop_args_t epaper_idf_http_event_loop_args = {
			.queue_size = 5,
			.task_name = "epaper_idf_http_event_loop_task", // task will be created
			.task_priority = uxTaskPriorityGet(NULL),
			.task_stack_size = epaper_idf_http_task_stack_depth,
			.task_core_id = tskNO_AFFINITY};

	ESP_ERROR_CHECK(esp_event_loop_create(&epaper_idf_http_event_loop_args, &epaper_idf_http_event_loop_handle));
	ESP_ERROR_CHECK(esp_event_handler_instance_register_with(epaper_idf_http_event_loop_handle, EPAPER_IDF_HTTP_EVENT, EPAPER_IDF_HTTP_EVENT_FINISH, epaper_idf_http_finish_event_handler, epaper_idf_http_event_loop_handle, NULL));

	// Initialize wifi.
	xTaskCreate(&epaper_idf_wifi_task, epaper_idf_wifi_task_name, epaper_idf_wifi_task_stack_depth * 8, NULL, epaper_idf_wifi_task_priority, NULL);
	ESP_LOGI(TAG, "Task started: %s", epaper_idf_wifi_task_name);
}
