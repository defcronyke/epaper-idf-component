#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_HTTPD_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_HTTPD_H_INCLUDED__
/**	epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.

		Contains some modified example code from here:
		https://github.com/espressif/esp-idf/blob/release/v4.2/examples/protocols/openssl_server/main/openssl_server_example.h

		Original Example Code Header:
		This example code is in the Public Domain (or CC0 licensed, at your option.)

		Unless required by applicable law or agreed to in writing, this
		software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
		CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_event_base.h"

// #ifndef EPAPER_IDF_COMPONENT_INIT_FS_DEFINED
#define EPAPER_IDF_COMPONENT_INIT_FS_DEFINED

// #define OPENSSL_EXAMPLE_TASK_STACK_WORDS 10240
// #define OPENSSL_EXAMPLE_TASK_PRIORITY    8

// #define OPENSSL_EXAMPLE_RECV_BUF_LEN       1024

// #define OPENSSL_EXAMPLE_LOCAL_TCP_PORT     443

extern SemaphoreHandle_t xMutexHTTPD;

/** An event base type for "epaper-idf-httpd". */
ESP_EVENT_DECLARE_BASE(EPAPER_IDF_HTTPD_EVENT);
enum	/**< The events. */
{
	EPAPER_IDF_HTTPD_EVENT_FINISH,	/**< An event for "this task is finished". */
};

/** The event loop handle. */
extern esp_event_loop_handle_t epaper_idf_httpd_event_loop_handle;

// /** Task action IDs of actions this task can perform. */
// enum epaper_idf_httpd_task_action_id {

// };

// /** Task action values. */
// struct epaper_idf_httpd_task_action_value_t {

// };

// typedef void* epaper_idf_httpd_task_action_value_void_p;

// #define _EPAPER_IDF_HTTPD_TASK_ACTION_VALUE_CAST_VOID_P(x) (epaper_idf_httpd_task_action_value_void_p)&x
// #define EPAPER_IDF_HTTPD_TASK_ACTION_VALUE_CAST_VOID_P(x) _EPAPER_IDF_HTTPD_TASK_ACTION_VALUE_CAST_VOID_P(x)

// #define _EPAPER_IDF_HTTPD_TASK_ACTION_VALUE_COPY(x) *(struct epaper_idf_httpd_task_action_value_t*)x
// #define EPAPER_IDF_HTTPD_TASK_ACTION_VALUE_COPY(x) _EPAPER_IDF_HTTPD_TASK_ACTION_VALUE_COPY(x)

// /** Task actions. */
// struct epaper_idf_httpd_task_action_t {
// 	enum epaper_idf_httpd_task_action_id id;
// 	epaper_idf_httpd_task_action_value_void_p value;
// };

// typedef void* epaper_idf_httpd_task_action_void_p;

// #define _EPAPER_IDF_HTTPD_TASK_ACTION_CAST_VOID_P(x) (epaper_idf_httpd_task_action_void_p)&x
// #define EPAPER_IDF_HTTPD_TASK_ACTION_CAST_VOID_P(x) _EPAPER_IDF_HTTPD_TASK_ACTION_CAST_VOID_P(x)

// #define _EPAPER_IDF_HTTPD_TASK_ACTION_COPY(x) *(struct epaper_idf_httpd_task_action_t*)x
// #define EPAPER_IDF_HTTPD_TASK_ACTION_COPY(x) _EPAPER_IDF_HTTPD_TASK_ACTION_COPY(x)

#ifdef __cplusplus
extern "C"
{
#endif

esp_err_t init_fs(void);

// esp_err_t start_httpd(const char *base_path);

/** The esp-idf task function. */
void epaper_idf_httpd_task(void *pvParameter);

#ifdef __cplusplus
}
#endif

#endif
