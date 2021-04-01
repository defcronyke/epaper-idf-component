#ifndef __EPAPER_IDF_COMPONENT_EPAPER_IDF_IO_H_INCLUDED__
#define __EPAPER_IDF_COMPONENT_EPAPER_IDF_IO_H_INCLUDED__
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

class EpaperIDFIO {
  public:
    virtual void _is_derived() = 0;
    virtual void init(uint8_t freq, uint32_t verbose);

  private:
    uint32_t verbose;
};

#endif
