#ifndef FONT_H_
#define FONT_H_

#include <stdint.h>

/* Font size constants */
#define FONT_5X8_WIDTH   5
#define FONT_5X8_HEIGHT  8
#define FONT_8X16_WIDTH  8
#define FONT_8X16_HEIGHT 16

/* Font bitmap data arrays (defined in font.c) */
extern const uint8_t font_5x8[];
extern const uint8_t font_8x16[];

#endif
