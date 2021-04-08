#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_SPI_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_SPI_H_INCLUDED__
/*  epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "epaper-idf-gfx.h"

#define EPAPER_IDF_IO_SPI_FREQ_DEFAULT 4

#ifndef EPAPER_IDF_IO_SPI_FREQ
#define EPAPER_IDF_IO_SPI_FREQ EPAPER_IDF_IO_SPI_FREQ_DEFAULT
#endif

class EpaperIDFSPI : public virtual EpaperIDFIO
{
public:
	spi_device_handle_t spi;

	void _is_derived();
	void init(uint8_t freq = EPAPER_IDF_IO_SPI_FREQ, uint32_t verbose = 0) override;

private:
	uint32_t verbose = EPAPER_IDF_LOG_VERB;
};

#endif
