#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_EXAMPLE_NONE_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_EXAMPLE_NONE_H_INCLUDED__
/**	epaper-idf-component - example - http-slideshow

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "epaper-idf-device.h"
/** Include the header file for the e-paper display device
  that you selected in the Kconfig menu settings. It defines 
  a "EpaperIDFDevice" class which can be used to interact 
  with your e-paper display device. Choose your e-paper device
  from the Kconfig menu, and make sure to set the correct pin
  mappings while you're at it. */
#include EPAPER_IDF_DEVICE_HEADER
#include "epaper-idf-task.h"

void none(void)
{
#error "ERROR: No main task selected. You can choose which main task to run in the project's Kconfig menu. Run the following command and select a project main task in the 'Project config' menu: idf.py menuconfig"
}

#define main_fn none

#endif
