#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_DEVICE_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_DEVICE_H_INCLUDED__
/*  epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/
#include "esp_system.h"
#include "epaper-idf-util.h"

#ifndef EPAPER_IDF_DEVICE_HEADER
#ifdef CONFIG_EPAPER_IDF_DEVICE_MODEL_GDEW075T8_OPT
#define EPAPER_IDF_DEVICE_HEADER CONFIG_EPAPER_IDF_DEVICE_MODEL_GDEW075T8

#elif CONFIG_EPAPER_IDF_DEVICE_MODEL_USER_OPT
#define EPAPER_IDF_DEVICE_HEADER CONFIG_EPAPER_IDF_DEVICE_MODEL_USER

/** NOTE: Add some #elif cases here for other e-paper 
  devices if you want. */

// #elif CONFIG_EPAPER_IDF_DEVICE_MODELNUM_OPT
// #define EPAPER_IDF_DEVICE_HEADER CONFIG_EPAPER_IDF_DEVICE_MODELNUM

#else
#define EPAPER_IDF_DEVICE_HEADER CONFIG_EPAPER_IDF_DEVICE_MODEL_NONE
#error "ERROR: No device model selected from the Kconfig menu."
#endif
#endif

#ifndef EPAPER_IDF_DEVICE_MODEL
#ifdef CONFIG_EPAPER_IDF_DEVICE_MODEL_GDEW075T8_OPT
#define EPAPER_IDF_DEVICE_MODEL Gdew075T8
#elif CONFIG_EPAPER_IDF_DEVICE_MODEL_NONE_OPT
#define EPAPER_IDF_DEVICE_MODEL none
#endif
#endif

#ifndef EpaperIDFDevice
#define EpaperIDFDevice EPAPER_IDF_DEVICE_MODEL
#endif

/** Include the header file for the e-paper display device
  that you selected in the Kconfig menu settings. It defines 
  a "EpaperIDFDevice" class which can be used to interact 
  with your e-paper display device. Choose your e-paper device
  from the Kconfig menu, and make sure to set the correct pin
  mappings while you're at it. */
#include EPAPER_IDF_DEVICE_HEADER

#endif
