/**	epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/
#include "epaper-idf-device.h"
/** Include the header file for the e-paper display device
  that you selected in the Kconfig menu settings. It defines 
  a "EpaperIDFDevice" class which can be used to interact 
  with your e-paper display device. Choose your e-paper device
  from the Kconfig menu, and make sure to set the correct pin
  mappings while you're at it. */
#include EPAPER_IDF_DEVICE_HEADER

EpaperIDFDevice::EpaperIDFDevice(EpaperIDFSPI &io)
		: Adafruit_GFX(EPAPER_IDF_GDEW075T8_WIDTH, EPAPER_IDF_GDEW075T8_HEIGHT),
			EpaperIDF(EPAPER_IDF_GDEW075T8_WIDTH, EPAPER_IDF_GDEW075T8_HEIGHT),
			io(io),
			tag("epaper-idf-device"),
			verbose(0),
			num_colours(EPAPER_IDF_GDEW075T8_NUM_COLOURS)
{
	ESP_LOGI(this->tag, EPAPER_IDF_QUOTE(EPAPER_IDF_DEVICE_MODEL) " constructed");
}

EpaperIDFDevice::~EpaperIDFDevice()
{
	ESP_LOGI(this->tag, EPAPER_IDF_QUOTE(EPAPER_IDF_DEVICE_MODEL) " deconstructed");
}

void EpaperIDFDevice::_is_derived() {}

void EpaperIDFDevice::init(uint32_t verbose)
{
	this->verbose = verbose;

	ESP_LOGI(tag, EPAPER_IDF_QUOTE(EPAPER_IDF_DEVICE_MODEL) " initializing...");

	this->io.init(EPAPER_IDF_IO_SPI_FREQ, this->verbose);

	// TODO: Maybe need to tweak watchdog timer settings here.

	if (this->verbose > 0)
	{
		ESP_LOGI(tag, EPAPER_IDF_QUOTE(EPAPER_IDF_DEVICE_MODEL) " initialized verbose message");
	}

	ESP_LOGI(tag, EPAPER_IDF_QUOTE(EPAPER_IDF_DEVICE_MODEL) " initialized");
}

void EpaperIDFDevice::drawPixel(int16_t x, int16_t y, uint16_t colour)
{
}

void EpaperIDFDevice::update()
{
}
