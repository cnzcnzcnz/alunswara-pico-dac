#ifndef DISPLAY_H_
#define DISPLAY_H_

typedef enum {
    DISPLAY_PHASE_SPLASH,
    DISPLAY_PHASE_TRANSITION,
    DISPLAY_PHASE_INFO
} display_phase_t;

// Initialize display system (call once at startup, before main loop)
void display_init(void);

// Called periodically from Core 0 main loop. Manages phases and rendering.
void display_update(void);

// Get current display phase (for debug/testing)
display_phase_t display_get_phase(void);

#endif /* DISPLAY_H_ */
