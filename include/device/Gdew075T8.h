#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_DEVICE_GDEW075T8_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_DEVICE_GDEW075T8_H_INCLUDED__
/*  epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/
#include <stdio.h>
#include <stdlib.h>
#include "epaper-idf-device.h"
#include "epaper-idf-gfx.h"
#include "epaper-idf-spi.h"

#define EPAPER_IDF_GDEW075T8_WIDTH 640
#define EPAPER_IDF_GDEW075T8_HEIGHT 384
#define EPAPER_IDF_GDEW075T8_NUM_BITS 8
#define EPAPER_IDF_GDEW075T8_NUM_COLOURS 1
#define EPAPER_IDF_GDEW075T8_PAR_DELAY_MS 200

#define EPAPER_IDF_GDEW075T8_BUFFER_SIZE ((uint32_t(EPAPER_IDF_GDEW075T8_WIDTH) * uint32_t(EPAPER_IDF_GDEW075T8_HEIGHT) * uint32_t(EPAPER_IDF_GDEW075T8_NUM_COLOURS)) / EPAPER_IDF_GDEW075T8_NUM_BITS)

class EpaperIDFDevice : public virtual EpaperIDFGFX
{
public:
	EpaperIDFDevice(EpaperIDFSPI &io);
	virtual ~EpaperIDFDevice();

	void _is_derived();
	void init(uint32_t verbose = 0);
	void drawPixel(int16_t x, int16_t y, uint16_t colour);
	void update();

private:
	EpaperIDFSPI &io;
	const char *tag;
	uint32_t verbose;
	uint32_t num_colours;
	uint8_t buffer[EPAPER_IDF_GDEW075T8_BUFFER_SIZE];
};

#endif
