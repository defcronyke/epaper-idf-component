#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_DEVICE_NONE_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_DEVICE_NONE_H_INCLUDED__
/*  epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/

#define EpaperIDFDevice EpaperIDFDeviceNone

#error "ERROR: ----- ¡INITIAL SETUP NOT COMPLETE! -----  No e-paper display device selected. You need to choose a device to use from the Kconfig 'Project display config' menu first. Run this command and make sure you set everything up in all the 'Project ...' menus correctly before you try to build this again: idf.py menuconfig"

#endif
