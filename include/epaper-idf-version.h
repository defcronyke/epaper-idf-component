#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_VERSION_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_VERSION_H_INCLUDED__
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
// #include "epaper-idf-util.h"

#define EPAPER_IDF_VERSION_MAJOR 0
#define EPAPER_IDF_VERSION_MINOR 1
#define EPAPER_IDF_VERSION_MICRO 0

#define EPAPER_IDF_VERSION_PRE v
#define EPAPER_IDF_VERSION_SEP .
#define EPAPER_IDF_VERSION_CLASS_PRE EpaperIDF
#define EPAPER_IDF_VERSION_CLASS_SEP _

#define EPAPER_IDF_VER_HIDDEN(pre, ver) #pre "" #ver
#define EPAPER_IDF_VER(pre, ver) EPAPER_IDF_VER_HIDDEN(pre, ver)

#define EPAPER_IDF_VER_CL_HIDDEN(pre, sep, ver, a, b, c) pre##sep##ver##a##sep##b##sep##c
#define EPAPER_IDF_VER_CL(pre, sep, ver, a, b, c) EPAPER_IDF_VER_CL_HIDDEN(pre, sep, ver, a, b, c)

#define EPAPER_IDF_VERSION_SHORT EPAPER_IDF_NAME2(EPAPER_IDF_VERSION_MAJOR, EPAPER_IDF_VERSION_MINOR, EPAPER_IDF_VERSION_SEP)
#define EPAPER_IDF_VERSION EPAPER_IDF_VER(EPAPER_IDF_VERSION_PRE, EPAPER_IDF_NAME2(EPAPER_IDF_VERSION_SHORT, EPAPER_IDF_VERSION_MICRO, EPAPER_IDF_VERSION_SEP))
#define EPAPER_IDF_VERSION_CLASS EPAPER_IDF_VER_CL(EPAPER_IDF_VERSION_CLASS_PRE, EPAPER_IDF_VERSION_CLASS_SEP, EPAPER_IDF_VERSION_PRE, EPAPER_IDF_VERSION_MAJOR, EPAPER_IDF_VERSION_MINOR, EPAPER_IDF_VERSION_MICRO)

#endif
