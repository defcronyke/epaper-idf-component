#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_H_INCLUDED__
/*  epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/
#include "epaper-idf-util.h"
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "Adafruit_GFX.h"

#ifndef EPAPER_IDF_DEVICE_CLASS
#define EPAPER_IDF_DEVICE_CLASS EPAPER_IDF_QUOTE(EPAPER_IDF_DEVICE_MODEL)
#endif

class EpaperIDFGFX : public virtual Adafruit_GFX
{
	public:
		EpaperIDFGFX(int16_t w, int16_t h) 
		:	Adafruit_GFX(w, h),
			tag("epaper-idf")
		{
			ESP_LOGI(tag, "EpaperIDF %s constructed: EpaperIDF -> %s", EPAPER_IDF_VERSION, EPAPER_IDF_QUOTE(EPAPER_IDF_VERSION_CLASS));
		};

		virtual ~EpaperIDFGFX()
		{
			ESP_LOGI(tag, "EpaperIDF %s deconstructed: EpaperIDF -> %s", EPAPER_IDF_VERSION, EPAPER_IDF_QUOTE(EPAPER_IDF_VERSION_CLASS));
		};

		virtual void drawPixel(int16_t x, int16_t y, uint16_t colour) = 0;
    virtual void init(uint32_t verbose = 0) = 0;
    virtual void update() = 0;

	private:
		const char *tag;
};
#endif
