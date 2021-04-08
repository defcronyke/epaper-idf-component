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
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "epaper-idf-util.h"
#include "epaper-idf-io.h"
// #include "epaper-idf-gfx.h"
// #include "epaper-idf-spi.h"

#ifndef EPAPER_IDF_DEVICE_HEADER
#ifdef CONFIG_EPAPER_IDF_DEVICE_MODEL_GDEW075T8_OPT

/** Header file for the device. */
#define EPAPER_IDF_DEVICE_HEADER CONFIG_EPAPER_IDF_DEVICE_MODEL_GDEW075T8

/** Class name for the device. */
#define EPAPER_IDF_DEVICE_MODEL	Gdew075T8

/** Type of device. */
#include "epaper-idf-spi.h"

#elif CONFIG_EPAPER_IDF_DEVICE_MODEL_USER_OPT

/**	Header file for the device.
 * 
 * 	NOTE:	You need both of these #include lines in 
 * 				each of your epaper-idf-component device 
 * 				files, and in your epaper-idf-component
 * 				task header.
 * 
  					#include "epaper-idf-device.h"
  					#include EPAPER_IDF_DEVICE_HEADER
 */
#define EPAPER_IDF_DEVICE_HEADER CONFIG_EPAPER_IDF_DEVICE_MODEL_USER

/** Class name for the device. */
#define EPAPER_IDF_DEVICE_MODEL none

/** Type of device. */
#include "epaper-idf-spi.h"

/** NOTE: Add some #elif cases here for other e-paper 
 * 				devices if you want. */

// #elif CONFIG_EPAPER_IDF_DEVICE_MODELNUM_OPT

/** Header file for the device. */
// #define EPAPER_IDF_DEVICE_HEADER CONFIG_EPAPER_IDF_DEVICE_MODEL_GDEW075T8

/** Class name for the device. */
// #define EPAPER_IDF_DEVICE_MODEL Gdew075T8

/** Type of device. */
// #include "epaper-idf-spi.h"

#else
#define EPAPER_IDF_DEVICE_HEADER CONFIG_EPAPER_IDF_DEVICE_MODEL_NONE

#define EPAPER_IDF_DEVICE_MODEL none
#error "ERROR: No device model selected from the Kconfig menu."

#endif
#endif

// #ifndef EPAPER_IDF_DEVICE_MODEL
// #ifdef CONFIG_EPAPER_IDF_DEVICE_MODEL_GDEW075T8_OPT

// #define EPAPER_IDF_DEVICE_MODEL Gdew075T8
// #include "epaper-idf-spi.h"

// #elif CONFIG_EPAPER_IDF_DEVICE_MODEL_NONE_OPT

// #define EPAPER_IDF_DEVICE_MODEL none

// /** NOTE: Add some #elif cases here for other e-paper 
//   devices if you want. */

// #endif
// #endif

#ifndef EpaperIDFDevice
/** Class name alias for the device. */
#define EpaperIDFDevice EPAPER_IDF_SYMBOL(EPAPER_IDF_DEVICE_MODEL)
#endif

#endif
