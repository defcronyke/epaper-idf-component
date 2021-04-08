/*  epaper-idf-component

		Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

		This code is released under the license terms contained in the
		file named LICENSE, which is found in the top-level folder in
		this project. You must agree to follow those license terms,
		otherwise you aren't allowed to copy, distribute, or use any 
		part of this project in any way.
*/
#include "epaper-idf-spi.h"

void EpaperIDFSPI::_is_derived() {}

void EpaperIDFSPI::init(uint8_t frequency, uint32_t verbose)
{
	this->verbose = verbose;
	if (this->verbose > 0)
	{
		printf("EpaperIDFSPI initialized verbose message.\n");
	}

	printf("EpaperIDFSPI initialized.\n");
}
