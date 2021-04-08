#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_UTIL_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_UTIL_H_INCLUDED__
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
// #include "freertos/FreeRTOS.h"

#define EPAPER_IDF_LOG_VERB_DEFAULT 0
#ifndef EPAPER_IDF_LOG_VERB
#define EPAPER_IDF_LOG_VERB EPAPER_IDF_LOG_VERB_DEFAULT
#endif

#define EPAPER_IDF_QUOTE_HIDDEN(x) #x
#define EPAPER_IDF_QUOTE(x) EPAPER_IDF_QUOTE_HIDDEN(x)

#define _EPAPER_IDF_SYMBOL(x) x
#define EPAPER_IDF_SYMBOL(x) _EPAPER_IDF_SYMBOL(x)

#define EPAPER_IDF_NAME2_HIDDEN(a, b, sep) #a "" #sep "" #b
#define EPAPER_IDF_NAME2(a, b, sep) EPAPER_IDF_NAME2_HIDDEN(a, b, sep)

#define EPAPER_IDF_NAME3_HIDDEN(a, b, c, sep) #a "" #sep "" #b "" #sep "" #c
#define EPAPER_IDF_NAME3(a, b, c, sep) EPAPER_IDF_NAME3_HIDDEN(a, b, c, sep)

// Depends on the above pre-processor directives.
#include "epaper-idf-version.h"

// Clamp a number into a fixed range.
inline float epaper_idf_clamp(float x, float min, float max)
{
	if (x < min)
	{
		return min;
	}
	else if (x >= max)
	{
		return max;
	}
	else
	{
		return x;
	}
}

// Map a number into a different range.
inline float epaper_idf_map(float x, float in_min, float in_max, float out_min, float out_max)
{
	return out_min + ((out_max - out_min) / (in_max - in_min)) * (x - in_min);
}

#endif
