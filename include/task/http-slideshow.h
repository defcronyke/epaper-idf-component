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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
// #include <time.h>
// #include <sys/time.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "freertos/queue.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "soc/rtc.h"
#if CONFIG_EXAMPLE_CONNECT_WIFI
#include "esp_wifi.h"
#endif
#include "protocol_examples_common.h"
#include "epaper-idf-device.h"
/** Include the header file for the e-paper display device
  that you selected in the Kconfig menu settings. It defines 
  a "EpaperIDFDevice" class which can be used to interact 
  with your e-paper display device. Choose your e-paper device
  from the Kconfig menu, and make sure to set the correct pin
  mappings while you're at it. */
#include EPAPER_IDF_DEVICE_HEADER
#include "epaper-idf-task.h"
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

void http_slideshow(void);

#define main_fn http_slideshow

#endif
