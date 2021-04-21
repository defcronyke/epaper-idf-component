#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_HTTPD_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_HTTPD_H_INCLUDED__
/**	epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_system.h"
// #include "esp_event_base.h"

extern SemaphoreHandle_t xMutexHTTPD;

// struct httpd_arg_t
// {

// } httpd_arg;

#ifdef __cplusplus
extern "C"
{
#endif

esp_err_t start_httpd(const char *base_path);

// /** The esp-idf task function. */
// void epaper_idf_http_task(void *pvParameter);

// void epaper_idf_http_get(struct epaper_idf_http_task_action_value_t action_value);

#ifdef __cplusplus
}
#endif

#endif
