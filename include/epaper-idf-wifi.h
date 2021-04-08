#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_WIFI_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_WIFI_H_INCLUDED__
/*  epaper-idf-component

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

// An event base type for "epaper-idf-wifi".
ESP_EVENT_DECLARE_BASE(EPAPER_IDF_WIFI_EVENT);
enum	// The events.
{
	EPAPER_IDF_WIFI_EVENT_FINISH,	// An event for "this task is finished".
};

// The event loop handle.
extern esp_event_loop_handle_t epaper_idf_wifi_event_loop_handle;

// #define epaper_idf_wifi_task_action_t uint32_t

/** Actions this task can perform. */
enum epaper_idf_wifi_task_action_t {
	EPAPER_IDF_WIFI_TASK_ACTION_CONNECT = 1U,	/**< Initialize (if necessary), and connect to a wifi network. */
	EPAPER_IDF_WIFI_TASK_ACTION_DISCONNECT,		/**< Disconnect wifi interface. */
	EPAPER_IDF_WIFI_TASK_ACTION_STOP,					/**< Disconnect and stop wifi interface. */
};

#ifdef __cplusplus
extern "C"
{
#endif

void epaper_idf_wifi_task(void* pvParameter);

#ifdef __cplusplus
}
#endif

#endif
