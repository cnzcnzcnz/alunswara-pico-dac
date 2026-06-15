/*
 * Standalone OLED test firmware for Raspberry Pi Pico
 * Tests I2C bus scan + SSD1306 display
 * 
 * LED behavior:
 *   - Fast blink (100ms) = I2C device found, trying to init OLED
 *   - Slow blink (1000ms) = No I2C device found (wiring issue)
 *   - Solid ON = OLED init success, displaying test pattern
 *
 * To use: temporarily replace main.c in CMakeLists.txt with test_oled.c
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#define SSD1306_I2C       i2c0
#define SSD1306_SDA_PIN   0
#define SSD1306_SCL_PIN   1
#define LED_PIN           25

static int i2c_scan(void) {
    int found = -1;
    for (uint8_t addr = 0x08; addr < 0x78; addr++) {
        uint8_t data = 0;
        int ret = i2c_read_timeout_us(SSD1306_I2C, addr, &data, 1, false, 5000);
        if (ret >= 0) {
            found = addr;
            break;
        }
    }
    return found;
}

static void ssd1306_cmd(uint8_t addr, uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    i2c_write_timeout_us(SSD1306_I2C, addr, buf, 2, false, 10000);
}

static void ssd1306_init_display(uint8_t addr) {
    static const uint8_t cmds[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x1F,
        0xD3, 0x00, 0x40, 0x8D, 0x14,
        0x20, 0x00, 0xA1, 0xC8, 0xDA,
        0x02, 0x81, 0xCF, 0xD9, 0xF1,
        0xDB, 0x40, 0xA4, 0xA6, 0xAF
    };
    for (size_t i = 0; i < sizeof(cmds); i++) {
        ssd1306_cmd(addr, cmds[i]);
    }
}

static void ssd1306_fill_test_pattern(uint8_t addr) {
    /* Set address range */
    ssd1306_cmd(addr, 0x21); ssd1306_cmd(addr, 0); ssd1306_cmd(addr, 127);
    ssd1306_cmd(addr, 0x22); ssd1306_cmd(addr, 0); ssd1306_cmd(addr, 3);

    /* Send checkerboard pattern */
    uint8_t buf[129];
    buf[0] = 0x40;  /* data mode */
    for (int page = 0; page < 4; page++) {
        for (int x = 0; x < 128; x++) {
            buf[1 + x] = (page % 2 == 0) ? 0xAA : 0x55;
        }
        i2c_write_timeout_us(SSD1306_I2C, addr, buf, 129, false, 50000);
    }
}

int main(void) {
    stdio_init_all();

    /* LED init */
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    /* I2C init */
    i2c_init(SSD1306_I2C, 400000);
    gpio_set_function(SSD1306_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SSD1306_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SSD1306_SDA_PIN);
    gpio_pull_up(SSD1306_SCL_PIN);

    sleep_ms(100);  /* Wait for OLED power-up */

    /* Scan I2C bus */
    int addr = i2c_scan();

    if (addr < 0) {
        /* No device found — slow blink forever */
        while (1) {
            gpio_put(LED_PIN, 1); sleep_ms(1000);
            gpio_put(LED_PIN, 0); sleep_ms(1000);
        }
    }

    /* Device found — fast blink 5 times to indicate address */
    for (int i = 0; i < 5; i++) {
        gpio_put(LED_PIN, 1); sleep_ms(100);
        gpio_put(LED_PIN, 0); sleep_ms(100);
    }
    sleep_ms(500);

    /* Blink the address value (hex low nibble) */
    uint8_t low_nibble = addr & 0x0F;
    for (uint8_t i = 0; i < low_nibble; i++) {
        gpio_put(LED_PIN, 1); sleep_ms(300);
        gpio_put(LED_PIN, 0); sleep_ms(300);
    }
    sleep_ms(1000);

    /* Try to init SSD1306 */
    ssd1306_init_display((uint8_t)addr);
    sleep_ms(50);

    /* Fill with test pattern */
    ssd1306_fill_test_pattern((uint8_t)addr);

    /* Solid LED = success */
    gpio_put(LED_PIN, 1);

    while (1) {
        sleep_ms(1000);
    }
}
