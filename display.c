#include "display.h"
#include "ssd1306.h"
#include "audio_state.h"
#include "font.h"
#include "splash_frames.h"
#include <stdio.h>
#include <string.h>
#include "bsp/board_api.h"

/* ---------- Display string buffer type ---------- */
typedef struct {
    char line1[17];  /* Max 16 chars at 8x16 font (128px / 8 = 16) */
    char line2[17];
    char line3[26];  /* Max 25 chars at 5x8 font (128px / (5+1) ≈ 21) */
} display_strings_t;

/* ---------- Static state ---------- */
static ssd1306_t g_oled;
static bool g_display_available = false;
static display_phase_t g_phase = DISPLAY_PHASE_SPLASH;
static uint32_t g_phase_start_ms = 0;
static uint32_t g_last_update_ms = 0;
static uint8_t g_splash_frame = 0;

/* ---------- Forward declarations ---------- */
static void display_render_current_info(void);
static display_strings_t display_format_info(const audio_info_t *info);

/* ---------- Public API ---------- */

void display_init(void) {
    if (!ssd1306_init(&g_oled)) {
        g_display_available = false;
        return;
    }
    g_display_available = true;

    /* Render first frame of splash animation */
    memcpy(g_oled.buffer, SPLASH_FRAMES[0], SPLASH_FRAME_BYTES);
    if (!ssd1306_display(&g_oled)) {
        g_display_available = false;
        return;
    }

    g_splash_frame = 0;
    g_phase = DISPLAY_PHASE_SPLASH;
    g_phase_start_ms = board_millis();
    g_last_update_ms = g_phase_start_ms;
}

void display_update(void) {
    if (!g_display_available) return;

    uint32_t now = board_millis();

    switch (g_phase) {
    case DISPLAY_PHASE_SPLASH: {
        /* Animate splash at SPLASH_FPS */
        uint32_t frame_interval_ms = 1000 / SPLASH_FPS;
        if (now - g_last_update_ms >= frame_interval_ms) {
            g_last_update_ms = now;
            g_splash_frame++;
            if (g_splash_frame >= SPLASH_FRAME_COUNT) {
                /* Animation complete — transition to info */
                g_phase = DISPLAY_PHASE_TRANSITION;
                g_phase_start_ms = now;
            } else {
                /* Show next frame */
                memcpy(g_oled.buffer, SPLASH_FRAMES[g_splash_frame], SPLASH_FRAME_BYTES);
                if (!ssd1306_display(&g_oled)) {
                    g_display_available = false;  // Mark display as unavailable
                }
            }
        }
        break;
    }

    case DISPLAY_PHASE_TRANSITION: {
        uint32_t elapsed = now - g_phase_start_ms;
        if (elapsed >= 300) {
            ssd1306_set_contrast(0xFF);  /* Restore full contrast */
            g_phase = DISPLAY_PHASE_INFO;
            g_last_update_ms = 0;  /* Force immediate update */
            display_render_current_info();
        } else {
            uint8_t contrast = 255 - (uint8_t)(elapsed * 255 / 300);
            ssd1306_set_contrast(contrast);
        }
        break;
    }

    case DISPLAY_PHASE_INFO:
        /* Rate limit to ~15fps (66ms minimum interval) */
        if (now - g_last_update_ms < 66) return;
        g_last_update_ms = now;

        if (audio_state_has_changed()) {
            display_render_current_info();
        }
        break;
    }
}

display_phase_t display_get_phase(void) {
    return g_phase;
}

/* ---------- Internal helpers ---------- */

static display_strings_t display_format_info(const audio_info_t *info) {
    display_strings_t strings;
    memset(&strings, 0, sizeof(strings));

    switch (info->stream_state) {
    case STREAM_STATE_DISCONNECTED:
        snprintf(strings.line1, sizeof(strings.line1), "No Signal");
        /* line2 and line3 remain empty */
        break;

    case STREAM_STATE_IDLE:
        snprintf(strings.line1, sizeof(strings.line1), "Ready");
        snprintf(strings.line3, sizeof(strings.line3), "Connected");
        break;

    case STREAM_STATE_ACTIVE: {
        /* Line 1: Hi-Res indicator */
        if (info->is_hires) {
            snprintf(strings.line1, sizeof(strings.line1), "Hi-Res");
        } else {
            snprintf(strings.line1, sizeof(strings.line1), "Standard");
        }

        /* Line 2: "{rate}kHz / {depth}-bit" */
        uint32_t rate_integer = info->sample_rate / 1000;
        uint32_t rate_decimal = (info->sample_rate % 1000) / 100;
        snprintf(strings.line2, sizeof(strings.line2), "%lu.%lukHz / %u-bit",
                 (unsigned long)rate_integer, (unsigned long)rate_decimal,
                 info->bit_depth);

        /* Line 3: mode */
        snprintf(strings.line3, sizeof(strings.line3), "USB PCM");
        break;
    }
    }

    return strings;
}

static void display_render_current_info(void) {
    audio_info_t info = audio_state_get_info();
    display_strings_t strings = display_format_info(&info);

    ssd1306_clear(&g_oled);

    /* Layout for 128x32 display (per design doc):
     * Line 1: y=0,  font 8x16
     * Line 2: y=12, font 8x16
     * Line 3: y=24, font 5x8
     */
    ssd1306_draw_string(&g_oled, 0, 0, strings.line1,
                        font_8x16, FONT_8X16_WIDTH, FONT_8X16_HEIGHT);
    ssd1306_draw_string(&g_oled, 0, 12, strings.line2,
                        font_8x16, FONT_8X16_WIDTH, FONT_8X16_HEIGHT);
    ssd1306_draw_string(&g_oled, 0, 24, strings.line3,
                        font_5x8, FONT_5X8_WIDTH, FONT_5X8_HEIGHT);

    if (!ssd1306_display(&g_oled)) {
        g_display_available = false;  // Mark display as unavailable if error
    }
}
