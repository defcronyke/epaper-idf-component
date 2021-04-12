#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_HTTP_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_HTTP_H_INCLUDED__
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
#include "esp_event_base.h"

/** An event base type for "epaper-idf-http". */
ESP_EVENT_DECLARE_BASE(EPAPER_IDF_HTTP_EVENT);
enum	/**< The events. */
{
	EPAPER_IDF_HTTP_EVENT_FINISH,	/**< An event for "this task is finished". */
};

/** The event loop handle. */
extern esp_event_loop_handle_t epaper_idf_http_event_loop_handle;

/** Task action IDs of actions this task can perform. */
enum epaper_idf_http_task_action_id {
	EPAPER_IDF_HTTP_TASK_ACTION_GET_INDEX_JSON = 1U,	/**< Get the public/http-slideshow/index.json file. */
};

/** Task action values. */
struct epaper_idf_http_task_action_value_t {
	bool no_deep_sleep;		/**< If true, don't deep sleep after the http request. */
};

typedef void* epaper_idf_http_task_action_value_void_p;

#define _EPAPER_IDF_HTTP_TASK_ACTION_VALUE_CAST_VOID_P(x) (epaper_idf_http_task_action_value_void_p)&x
#define EPAPER_IDF_HTTP_TASK_ACTION_VALUE_CAST_VOID_P(x) _EPAPER_IDF_HTTP_TASK_ACTION_VALUE_CAST_VOID_P(x)

#define _EPAPER_IDF_HTTP_TASK_ACTION_VALUE_COPY(x) *(struct epaper_idf_http_task_action_value_t*)x
#define EPAPER_IDF_HTTP_TASK_ACTION_VALUE_COPY(x) _EPAPER_IDF_HTTP_TASK_ACTION_VALUE_COPY(x)

/** Task actions. */
struct epaper_idf_http_task_action_t {
	enum epaper_idf_http_task_action_id id;
	epaper_idf_http_task_action_value_void_p value;
};

typedef void* epaper_idf_http_task_action_void_p;

#define _EPAPER_IDF_HTTP_TASK_ACTION_CAST_VOID_P(x) (epaper_idf_http_task_action_void_p)&x
#define EPAPER_IDF_HTTP_TASK_ACTION_CAST_VOID_P(x) _EPAPER_IDF_HTTP_TASK_ACTION_CAST_VOID_P(x)

#define _EPAPER_IDF_HTTP_TASK_ACTION_COPY(x) *(struct epaper_idf_http_task_action_t*)x
#define EPAPER_IDF_HTTP_TASK_ACTION_COPY(x) _EPAPER_IDF_HTTP_TASK_ACTION_COPY(x)

#ifdef __cplusplus
extern "C"
{
#endif

/** The esp-idf task function. */
void epaper_idf_http_task(void *pvParameter);

void epaper_idf_http_get(struct epaper_idf_http_task_action_value_t action_value);

#ifdef __cplusplus
}
#endif

#endif
