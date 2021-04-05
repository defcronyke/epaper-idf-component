#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_EXAMPLE_HTTP_SLIDESHOW_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_EXAMPLE_HTTP_SLIDESHOW_H_INCLUDED__
/*  epaper-idf-component - example - http-slideshow

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/
#include "epaper-idf-task.h"

// enum {
//     WIFI_EVENT_ID_START,
//     WIFI_EVENT_ID_CONNECT,
//     WIFI_EVENT_ID_DISCONNECT,
// };

// ESP_EVENT_DECLARE_BASE(WIFI_EVENT);

void http_slideshow(void);

#define main_fn http_slideshow

#endif