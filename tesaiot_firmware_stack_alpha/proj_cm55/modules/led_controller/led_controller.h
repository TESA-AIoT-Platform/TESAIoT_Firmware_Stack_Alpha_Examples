/**
 * LED controller API: types and declarations for controlling the three user LEDs
 * (red, green, blue) on the CM55 core.
 */
#ifndef LED_CONTROLLER_H_
#define LED_CONTROLLER_H_

#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum
{
  LED_RED = 0,    /* User LED 1 (red), P16[7]. */
  LED_GREEN,      /* User LED 2 (green), P16[6]. */
  LED_BLUE,       /* User LED 3 (blue), P16[5]. */
  LED_COUNT       /* Number of user LEDs; use for bounds checking. */
} led_id_t;

/**
 * Optional init; GPIO is configured by cybsp_init(). Returns true for compatibility.
 */
bool led_controller_init(void);

/**
 * Set one LED on or off. Invalid id is ignored (no-op).
 */
void led_controller_set(led_id_t id, bool on);

/**
 * Toggle one LED. Invalid id is ignored (no-op).
 */
void led_controller_toggle(led_id_t id);

/**
 * Read current state of one LED. Returns true if on, false if off or id invalid.
 */
bool led_controller_get(led_id_t id);

/**
 * Set all three user LEDs on or off.
 */
void led_controller_set_all(bool on);

/**
 * Turn all user LEDs off.
 */
void led_controller_off_all(void);

#if defined(__cplusplus)
}
#endif

#endif /* LED_CONTROLLER_H_ */
