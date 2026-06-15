#include "ssd1306.h"
#include "font.h"
#include <string.h>
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#define SSD1306_I2C       i2c0
#define SSD1306_SDA_PIN   0
#define SSD1306_SCL_PIN   1

/* Send a single command byte to the SSD1306 via I2C (with timeout) */
static int ssd1306_send_command(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};  /* 0x00 = command control byte */
    int ret = i2c_write_timeout_us(SSD1306_I2C, SSD1306_I2C_ADDR, buf, 2, false, 10000);
    return ret;
}

bool ssd1306_init(ssd1306_t *oled) {
    /* Initialize I2C1 at 400kHz */
    i2c_init(SSD1306_I2C, 400000);

    /* Configure GPIO4 (SDA) and GPIO5 (SCL) for I2C with pull-ups */
    gpio_set_function(SSD1306_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SSD1306_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SSD1306_SDA_PIN);
    gpio_pull_up(SSD1306_SCL_PIN);

    /* SSD1306 initialization command sequence for 128x32 */
    static const uint8_t init_cmds[] = {
        0xAE,       /* Display OFF */
        0xD5, 0x80, /* Set display clock divide ratio */
        0xA8, 0x1F, /* Set multiplex ratio (32-1 = 0x1F) */
        0xD3, 0x00, /* Set display offset = 0 */
        0x40,       /* Set start line = 0 */
        0x8D, 0x14, /* Charge pump enable */
        0x20, 0x00, /* Horizontal addressing mode */
        0xA1,       /* Segment remap (column 127 mapped to SEG0) */
        0xC8,       /* COM output scan direction (remapped) */
        0xDA, 0x02, /* COM pins hardware config for 128x32 */
        0x81, 0x8F, /* Set contrast */
        0xD9, 0xF1, /* Set pre-charge period */
        0xDB, 0x40, /* Set VCOMH deselect level */
        0xA4,       /* Display from RAM content */
        0xA6,       /* Normal display (not inverted) */
        0xAF        /* Display ON */
    };

    /* Send init sequence, check for NACK (display not present) */
    for (size_t i = 0; i < sizeof(init_cmds); i++) {
        int ret = ssd1306_send_command(init_cmds[i]);
        if (ret == PICO_ERROR_GENERIC) {
            /* I2C NACK — display not detected */
            return false;
        }
    }

    /* Clear framebuffer */
    memset(oled->buffer, 0, SSD1306_BUF_SIZE);
    oled->modified = false;

    return true;
}

void ssd1306_clear(ssd1306_t *oled) {
    memset(oled->buffer, 0, SSD1306_BUF_SIZE);
    oled->modified = true;
}

void ssd1306_set_pixel(ssd1306_t *oled, int16_t x, int16_t y, bool on) {
    /* Bounds check — clip out-of-range coordinates */
    if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT) {
        return;
    }

    if (on) {
        oled->buffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
    } else {
        oled->buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
    oled->modified = true;
}

void ssd1306_draw_char(ssd1306_t *oled, int16_t x, int16_t y, char c,
                       const uint8_t *font, uint8_t font_width, uint8_t font_height) {
    /* Only render printable ASCII characters */
    if (c < 32 || c > 126) {
        return;
    }

    /* Skip entirely if character is fully off-screen */
    if (x + font_width <= 0 || x >= SSD1306_WIDTH ||
        y + font_height <= 0 || y >= SSD1306_HEIGHT) {
        return;
    }

    uint8_t char_index = (uint8_t)(c - 32);

    if (font_height == FONT_5X8_HEIGHT && font_width == FONT_5X8_WIDTH) {
        /*
         * 5x8 font: each character is 5 bytes (one per column).
         * Each byte's bits represent rows (bit 0 = top row).
         */
        const uint8_t *glyph = &font[char_index * font_width];

        for (uint8_t col = 0; col < font_width; col++) {
            int16_t px = x + col;
            if (px < 0 || px >= SSD1306_WIDTH) continue;

            uint8_t col_data = glyph[col];
            for (uint8_t row = 0; row < font_height; row++) {
                int16_t py = y + row;
                if (py < 0 || py >= SSD1306_HEIGHT) continue;

                if (col_data & (1 << row)) {
                    oled->buffer[px + (py / 8) * SSD1306_WIDTH] |= (1 << (py % 8));
                }
            }
        }
    } else if (font_height == FONT_8X16_HEIGHT && font_width == FONT_8X16_WIDTH) {
        /*
         * 8x16 font: each character is 16 bytes (one per row).
         * Each byte's bits represent columns (bit 7 = leftmost column).
         */
        const uint8_t *glyph = &font[char_index * font_height];

        for (uint8_t row = 0; row < font_height; row++) {
            int16_t py = y + row;
            if (py < 0 || py >= SSD1306_HEIGHT) continue;

            uint8_t row_data = glyph[row];
            for (uint8_t col = 0; col < font_width; col++) {
                int16_t px = x + col;
                if (px < 0 || px >= SSD1306_WIDTH) continue;

                if (row_data & (0x80 >> col)) {
                    oled->buffer[px + (py / 8) * SSD1306_WIDTH] |= (1 << (py % 8));
                }
            }
        }
    }
}

void ssd1306_draw_string(ssd1306_t *oled, int16_t x, int16_t y, const char *str,
                         const uint8_t *font, uint8_t font_width, uint8_t font_height) {
    int16_t cursor_x = x;

    while (*str) {
        /* Stop early if cursor is past the right edge */
        if (cursor_x >= SSD1306_WIDTH) {
            break;
        }

        ssd1306_draw_char(oled, cursor_x, y, *str, font, font_width, font_height);
        cursor_x += font_width + 1;  /* 1-pixel spacing between characters */
        str++;
    }

    oled->modified = true;
}

void ssd1306_display(ssd1306_t *oled) {
    /* Set column address range: 0 to 127 */
    ssd1306_send_command(0x21);  /* Set column address */
    ssd1306_send_command(0);     /* Start column */
    ssd1306_send_command(127);   /* End column */

    /* Set page address range: 0 to 3 (4 pages for 32-pixel height) */
    ssd1306_send_command(0x22);  /* Set page address */
    ssd1306_send_command(0);     /* Start page */
    ssd1306_send_command(3);     /* End page */

    /* Send framebuffer data with 0x40 control byte prefix (data mode) */
    uint8_t buf[SSD1306_BUF_SIZE + 1];
    buf[0] = 0x40;  /* Data control byte */
    memcpy(&buf[1], oled->buffer, SSD1306_BUF_SIZE);
    i2c_write_timeout_us(SSD1306_I2C, SSD1306_I2C_ADDR, buf, SSD1306_BUF_SIZE + 1, false, 50000);

    oled->modified = false;
}

void ssd1306_set_contrast(uint8_t contrast) {
    ssd1306_send_command(0x81);      /* Set contrast command */
    ssd1306_send_command(contrast);  /* Contrast value 0-255 */
}
