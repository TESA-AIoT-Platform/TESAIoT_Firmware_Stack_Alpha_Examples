#ifndef APP_EVENTS_H
#define APP_EVENTS_H

#include "user_buttons_types.h"

/**
 * Standard application-level event IDs used with the Event Bus.
 */
typedef enum
{
  APP_EVENT_BTN_PRESSED = BUTTON_EVENT_PRESSED,
  APP_EVENT_BTN_RELEASED = BUTTON_EVENT_RELEASED,
  APP_EVENT_BTN_CHANGED = BUTTON_EVENT_CHANGED,

  APP_EVENT_WIFI_SCAN_COMPLETE,
  APP_EVENT_MAX
} app_event_t;

#endif /* APP_EVENTS_H */
