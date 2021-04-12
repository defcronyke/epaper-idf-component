#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_TASK_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_TASK_H_INCLUDED__
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
#include "esp_system.h"

#ifndef EPAPER_IDF_MAIN_TASK_HEADER
#ifdef CONFIG_EPAPER_IDF_MAIN_TASK_HTTP_SLIDESHOW_OPT
#define EPAPER_IDF_MAIN_TASK_HEADER CONFIG_EPAPER_IDF_MAIN_TASK_HTTP_SLIDESHOW

#elif CONFIG_EPAPER_IDF_MAIN_TASK_USER_OPT
#define EPAPER_IDF_MAIN_TASK_HEADER CONFIG_EPAPER_IDF_MAIN_TASK_USER

/** NOTE: Add some #elif cases here for your own custom tasks if you want. */

/** #elif CONFIG_EPAPER_IDF_MAIN_TASK_TASKNAME_OPT */
/** #define EPAPER_IDF_MAIN_TASK_HEADER CONIG_EPAPER_IDF_MAIN_TASK_TASKNAME */

#else
#define EPAPER_IDF_MAIN_TASK_HEADER CONFIG_EPAPER_IDF_MAIN_TASK_NONE
#error "ERROR: No main task selected from the Kconfig menu."
#endif
#endif

/** Include the header file for the main task that 
  you selected in the Kconfig menu settings. It 
  defines a "void main_fn()" function which can be called
  from your program's "void app_main()" function to 
  start your main task (either an example task, or your
  own task if you added it above and chose it from the
  Kconfig menu). */
#include EPAPER_IDF_MAIN_TASK_HEADER

#endif
