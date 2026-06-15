#include "audio_state.h"

static volatile audio_info_t g_audio_state = {
    .sample_rate = 44100,
    .bit_depth = 16,
    .stream_state = STREAM_STATE_DISCONNECTED,
    .is_hires = false
};

static volatile bool g_state_changed = false;

void audio_state_set_sample_rate(uint32_t rate) {
    g_audio_state.sample_rate = rate;
    g_audio_state.is_hires = (rate >= 88200) || (g_audio_state.bit_depth == 24);
    g_state_changed = true;
}

void audio_state_set_resolution(uint8_t bits) {
    g_audio_state.bit_depth = bits;
    g_audio_state.is_hires = (g_audio_state.sample_rate >= 88200) || (bits == 24);
    g_state_changed = true;
}

void audio_state_set_streaming(bool active) {
    g_audio_state.stream_state = active ? STREAM_STATE_ACTIVE : STREAM_STATE_IDLE;
    g_state_changed = true;
}

void audio_state_set_connected(bool connected) {
    g_audio_state.stream_state = connected ? STREAM_STATE_IDLE : STREAM_STATE_DISCONNECTED;
    g_state_changed = true;
}

audio_info_t audio_state_get_info(void) {
    audio_info_t info;
    info.sample_rate = g_audio_state.sample_rate;
    info.bit_depth = g_audio_state.bit_depth;
    info.stream_state = g_audio_state.stream_state;
    info.is_hires = g_audio_state.is_hires;
    return info;
}

bool audio_state_has_changed(void) {
    if (g_state_changed) {
        g_state_changed = false;
        return true;
    }
    return false;
}
