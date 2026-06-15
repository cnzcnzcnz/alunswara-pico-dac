#ifndef AUDIO_STATE_H_
#define AUDIO_STATE_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    STREAM_STATE_DISCONNECTED,
    STREAM_STATE_IDLE,
    STREAM_STATE_ACTIVE
} stream_state_t;

typedef struct {
    uint32_t sample_rate;
    uint8_t  bit_depth;
    stream_state_t stream_state;
    bool     is_hires;
} audio_info_t;

/* Called from USB callbacks (ISR context) */
void audio_state_set_sample_rate(uint32_t rate);
void audio_state_set_resolution(uint8_t bits);
void audio_state_set_streaming(bool active);
void audio_state_set_connected(bool connected);

/* Called from display update loop (Core 0 main loop) */
audio_info_t audio_state_get_info(void);
bool audio_state_has_changed(void);

#endif /* AUDIO_STATE_H_ */
