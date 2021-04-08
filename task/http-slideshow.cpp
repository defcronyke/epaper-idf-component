/*  epaper-idf-component - example - http-slideshow

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/
#include "task/http-slideshow.h"

static void http_slideshow_deep_sleep(int32_t sleep_duration_us);

const char *TAG = "http-slideshow";

const char *epaper_idf_wifi_task_name = "epaper_idf_wifi_task";
const uint32_t epaper_idf_wifi_task_stack_depth = 4096;
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
}

static void epaper_idf_wifi_stopped_event_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
	ESP_LOGI(TAG, "event received: EPAPER_IDF_WIFI_EVENT_STOPPED");

	http_slideshow_deep_sleep(CONFIG_EPAPER_IDF_DEEP_SLEEP_SECONDS * 1000000);
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

// Enter deep sleep.
static void http_slideshow_stop_wifi(int32_t delay_secs)
{
	ESP_LOGI(TAG, "stopping the wifi interface");

	// Disable wifi for deep sleep.
	static const epaper_idf_wifi_task_action_t wifi_task_action_stop = EPAPER_IDF_WIFI_TASK_ACTION_STOP;

	xTaskCreate(&epaper_idf_wifi_task, epaper_idf_wifi_task_name, epaper_idf_wifi_task_stack_depth * 8, (void*)&wifi_task_action_stop, epaper_idf_wifi_task_priority, NULL);
	ESP_LOGI(TAG, "Task started: %s", epaper_idf_wifi_task_name);
}

static void http_slideshow_deep_sleep(int32_t sleep_duration_us) {
	ESP_LOGI(TAG, "preparing for deep sleep");

	ESP_ERROR_CHECK(esp_event_handler_unregister_with(epaper_idf_wifi_event_loop_handle, EPAPER_IDF_WIFI_EVENT, EPAPER_IDF_WIFI_EVENT_FINISH, epaper_idf_wifi_finish_event_handler));

	ESP_ERROR_CHECK(esp_event_handler_unregister_with(epaper_idf_ota_event_loop_handle, EPAPER_IDF_OTA_EVENT, EPAPER_IDF_OTA_EVENT_FINISH, epaper_idf_ota_finish_event_handler));
	ESP_ERROR_CHECK(esp_event_loop_delete(epaper_idf_ota_event_loop_handle));

	ESP_ERROR_CHECK(esp_event_handler_unregister_with(epaper_idf_http_event_loop_handle, EPAPER_IDF_HTTP_EVENT, EPAPER_IDF_HTTP_EVENT_FINISH, epaper_idf_http_finish_event_handler));
	ESP_ERROR_CHECK(esp_event_loop_delete(epaper_idf_http_event_loop_handle));

	ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, sta_got_ip_event_handler));
	ESP_ERROR_CHECK(esp_event_loop_delete_default());

	ESP_LOGI(TAG, "Enabling deep sleep timer wakeup after approx: %f secs", (float)sleep_duration_us / 1000000.f);
	ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(sleep_duration_us));

	// Isolate GPIO12 pin from external circuits. This is needed for modules
	// which have an external pull-up resistor on GPIO12 (such as ESP32-WROVER)
	// to minimize current consumption.
	ESP_ERROR_CHECK(rtc_gpio_isolate(GPIO_NUM_12));

	// Hibernate for lowest power consumption.
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF));
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF));
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF));
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF));

	ESP_LOGI(TAG, "entering deep sleep...\n");

	esp_deep_sleep_start();
}

extern "C" void http_slideshow_task(void *pvParameter)
{
	ESP_LOGI(TAG, "task %s is running", http_slideshow_task_name);

	while (1)
	{
		struct timeval now;
		gettimeofday(&now, NULL);
			
		ESP_LOGI(TAG, "%s loop", http_slideshow_task_name);

		// Use the appropriate epaper device.
		EpaperIDFSPI io;
		EpaperIDFDevice dev(io);

		// Number of seconds to delay (either with deep sleep or the vTaskDelay function).
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

		if (no_deep_sleep)
		{
			ESP_LOGI(TAG, "waiting for %d secs\n", delay_secs);
			vTaskDelay((delay_secs * 1000) / portTICK_PERIOD_MS);
		} else {
			// Enter deep sleep.
			http_slideshow_stop_wifi(delay_secs);

			vTaskDelete(NULL);
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
	ESP_ERROR_CHECK(esp_event_handler_instance_register_with(epaper_idf_wifi_event_loop_handle, EPAPER_IDF_WIFI_EVENT, EPAPER_IDF_WIFI_EVENT_STOPPED, epaper_idf_wifi_stopped_event_handler, epaper_idf_wifi_event_loop_handle, NULL));

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

	// Initialize wifi and connect.
	static const epaper_idf_wifi_task_action_t wifi_task_action_connect = EPAPER_IDF_WIFI_TASK_ACTION_CONNECT;

	xTaskCreate(&epaper_idf_wifi_task, epaper_idf_wifi_task_name, epaper_idf_wifi_task_stack_depth * 8, (void*)&wifi_task_action_connect, epaper_idf_wifi_task_priority, NULL);
	ESP_LOGI(TAG, "Task started: %s", epaper_idf_wifi_task_name);
}
