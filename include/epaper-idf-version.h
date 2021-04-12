#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_VERSION_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_VERSION_H_INCLUDED__
/**	epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/
#include "epaper-idf-util.h"

#ifndef EPAPER_IDF_VERSION_MAJOR
/** The project's major version number. Changing this breaks 
	backwards-compatibility by changing the EpaperIDF class name. 
	You can override this in the file: ../CMakeLists.txt */
#define EPAPER_IDF_VERSION_MAJOR 0
#endif

#ifndef EPAPER_IDF_VERSION_MINOR
/** The project's minor version number. Changing this breaks 
	backwards-compatibility by changing the EpaperIDF class name. 
	You can override this in the file: ../CMakeLists.txt */
#define EPAPER_IDF_VERSION_MINOR 1
#endif

#define EPAPER_IDF_VERSION_PRE v
#define EPAPER_IDF_VERSION_SEP .

#define EPAPER_IDF_VERSION_CLASS_PRE EpaperIDF_
#define EPAPER_IDF_VERSION_CLASS_SEP _

#define _EPAPER_IDF_VER(pre, ver) #pre "" #ver
#define EPAPER_IDF_VER(pre, ver) _EPAPER_IDF_VER(pre, ver)

#define _EPAPER_IDF_VER_CL(pre, sep, ver, maj, min) pre##ver##maj##sep##min
#define EPAPER_IDF_VER_CL(pre, sep, ver, maj, min) _EPAPER_IDF_VER_CL(pre, sep, ver, maj, min)

#define EPAPER_IDF_VERSION_SHORT EPAPER_IDF_NAME2(EPAPER_IDF_VERSION_MAJOR, EPAPER_IDF_VERSION_MINOR, EPAPER_IDF_VERSION_SEP)


#if CONFIG_APP_PROJECT_VER_FROM_CONFIG != 1

#ifndef EPAPER_IDF_VERSION_MICRO
#define EPAPER_IDF_VERSION_MICRO EPAPER_IDF_QUOTE(0)
#endif

#define EPAPER_IDF_VERSION EPAPER_IDF_VER(EPAPER_IDF_VERSION_PRE, EPAPER_IDF_NAME2(EPAPER_IDF_VERSION_SHORT, EPAPER_IDF_VERSION_MICRO, EPAPER_IDF_VERSION_SEP))

#else

#ifndef EPAPER_IDF_VERSION_MICRO
#define EPAPER_IDF_VERSION_MICRO CONFIG_APP_PROJECT_VER
#endif

#ifndef EPAPER_IDF_VERSION
#define EPAPER_IDF_VERSION EPAPER_IDF_VER(EPAPER_IDF_VERSION_PRE, EPAPER_IDF_NAME2(EPAPER_IDF_VERSION_SHORT, EPAPER_IDF_VERSION_MICRO, EPAPER_IDF_VERSION_SEP))
#endif

#endif


#ifndef EPAPER_IDF_VERSION_CLASS
#define EPAPER_IDF_VERSION_CLASS EPAPER_IDF_VER_CL(EPAPER_IDF_VERSION_CLASS_PRE, EPAPER_IDF_VERSION_CLASS_SEP, EPAPER_IDF_VERSION_PRE, EPAPER_IDF_VERSION_MAJOR, EPAPER_IDF_VERSION_MINOR)
#endif

#ifndef EpaperIDF
#define EpaperIDF EPAPER_IDF_VERSION_CLASS
#endif

#endif
