#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_LOG_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_LOG_H_INCLUDED__
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
#include "epaper-idf-util.h"

#define EPAPER_IDF_LOG_VERB_DEFAULT 0
#ifndef EPAPER_IDF_LOG_VERB

/** TODO: Set this in the Kconfig menu. */
#define EPAPER_IDF_LOG_VERB EPAPER_IDF_LOG_VERB_DEFAULT

#endif

#endif
