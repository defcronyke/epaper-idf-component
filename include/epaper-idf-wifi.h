#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_WIFI_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_WIFI_H_INCLUDED__
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
#include "freertos/event_groups.h"
#include "esp_event_base.h"

/** An event base type for "epaper-idf-wifi". */
ESP_EVENT_DECLARE_BASE(EPAPER_IDF_WIFI_EVENT);
enum	/**< The events. */
{
	EPAPER_IDF_WIFI_EVENT_FINISH,		/**< An event for "this task is finished". */
	EPAPER_IDF_WIFI_EVENT_STOPPED,	/**< Do something after the wifi interface has stopped. */
};

/** The event loop handle. */
extern esp_event_loop_handle_t epaper_idf_wifi_event_loop_handle;

/** Task action IDs of actions this task can perform. */
enum epaper_idf_wifi_task_action_id {
	EPAPER_IDF_WIFI_TASK_ACTION_CONNECT = 1U,	/**< Initialize (if necessary), and connect to a wifi network. */
	EPAPER_IDF_WIFI_TASK_ACTION_DISCONNECT,		/**< Disconnect the wifi interface. */
	EPAPER_IDF_WIFI_TASK_ACTION_STOP,					/**< Disconnect and stop the wifi interface. */
};

/** Task action values. */
struct epaper_idf_wifi_task_action_value_t {
	bool no_deep_sleep;					/**< If true, don't deep sleep after the wifi request. */
	uint64_t deep_sleep_usecs;	/**< Deep sleep for this number of microseconds (seconds * 1000000). */
};

typedef void* epaper_idf_wifi_task_action_value_void_p;

#define _EPAPER_IDF_WIFI_TASK_ACTION_VALUE_CAST_VOID_P(x) (epaper_idf_wifi_task_action_value_void_p)&x
#define EPAPER_IDF_WIFI_TASK_ACTION_VALUE_CAST_VOID_P(x) _EPAPER_IDF_WIFI_TASK_ACTION_VALUE_CAST_VOID_P(x)

#define _EPAPER_IDF_WIFI_TASK_ACTION_VALUE_COPY(x) *(struct epaper_idf_wifi_task_action_value_t*)x
#define EPAPER_IDF_WIFI_TASK_ACTION_VALUE_COPY(x) _EPAPER_IDF_WIFI_TASK_ACTION_VALUE_COPY(x)

/** Task actions. */
struct epaper_idf_wifi_task_action_t {
	enum epaper_idf_wifi_task_action_id id;
	epaper_idf_wifi_task_action_value_void_p value;
};

typedef void* epaper_idf_wifi_task_action_void_p;

#define _EPAPER_IDF_WIFI_TASK_ACTION_CAST_VOID_P(x) (epaper_idf_wifi_task_action_void_p)&x
#define EPAPER_IDF_WIFI_TASK_ACTION_CAST_VOID_P(x) _EPAPER_IDF_WIFI_TASK_ACTION_CAST_VOID_P(x)

#define _EPAPER_IDF_WIFI_TASK_ACTION_COPY(x) *(struct epaper_idf_wifi_task_action_t*)x
#define EPAPER_IDF_WIFI_TASK_ACTION_COPY(x) _EPAPER_IDF_WIFI_TASK_ACTION_COPY(x)

#ifdef __cplusplus
extern "C"
{
#endif

/** The esp-idf task function. */
void epaper_idf_wifi_task(void* pvParameter);

#ifdef __cplusplus
}
#endif

#endif
