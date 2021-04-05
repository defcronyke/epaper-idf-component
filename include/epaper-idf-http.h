#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_HTTP_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_HTTP_H_INCLUDED__
/*  epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/
#include "esp_event_base.h"

// An event base type for "epaper-idf-http".
ESP_EVENT_DECLARE_BASE(EPAPER_IDF_HTTP_EVENT);
enum	// The events.
{
	EPAPER_IDF_HTTP_EVENT_FINISH,	// An event for "this task is finished".
};

// The event loop handle.
extern esp_event_loop_handle_t epaper_idf_http_event_loop_handle;

#ifdef __cplusplus
extern "C"
{
#endif

	// The esp-idf task function.
	void epaper_idf_http_task(void *pvParameter);

#ifdef __cplusplus
}
#endif

#endif
