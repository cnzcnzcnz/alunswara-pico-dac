#ifndef SSD1306_H_
#define SSD1306_H_

#include <stdint.h>
#include <stdbool.h>

#define SSD1306_WIDTH       128
#define SSD1306_HEIGHT      32
#define SSD1306_BUF_SIZE    (SSD1306_WIDTH * SSD1306_HEIGHT / 8)  // 512 bytes
#define SSD1306_I2C_ADDR    0x3C

typedef struct {
    uint8_t buffer[SSD1306_BUF_SIZE];
    bool    modified;
} ssd1306_t;

// Initialize SSD1306 display via I2C. Returns true if display detected.
bool ssd1306_init(ssd1306_t *oled);

// Clear the framebuffer (all pixels off)
void ssd1306_clear(ssd1306_t *oled);

// Set or clear a single pixel at (x, y). Clips to display bounds.
void ssd1306_set_pixel(ssd1306_t *oled, int16_t x, int16_t y, bool on);

// Draw a single character at position (x, y) using specified font
void ssd1306_draw_char(ssd1306_t *oled, int16_t x, int16_t y, char c,
                       const uint8_t *font, uint8_t font_width, uint8_t font_height);

// Draw a null-terminated string starting at (x, y) using specified font
void ssd1306_draw_string(ssd1306_t *oled, int16_t x, int16_t y, const char *str,
                         const uint8_t *font, uint8_t font_width, uint8_t font_height);

// Transfer framebuffer to the SSD1306 display via I2C
void ssd1306_display(ssd1306_t *oled);

// Set display contrast (0-255). Used for fade transitions.
void ssd1306_set_contrast(uint8_t contrast);

#endif
