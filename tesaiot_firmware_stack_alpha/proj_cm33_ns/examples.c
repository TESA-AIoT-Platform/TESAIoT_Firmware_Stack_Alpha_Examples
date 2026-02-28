#include "FreeRTOS.h"
#include "user_buttons.h"
#include <stdio.h>

static void on_btn_change(user_buttons_t switch_handle, const button_event_t *evt)
{
  (void)switch_handle;

  //!! DO NOT USE printf in the interrupt context
  const char *action = evt->is_pressed ? "PRESSED" : "RELEASED";
  printf("[Example] Button %lu %s (Count: %lu)\n", (unsigned long)evt->button_id, action,
         (unsigned long)evt->press_count);
}

void examples_init(void)
{
  user_buttons_init();
  user_button_on_changed(BUTTON_ID_0, on_btn_change);
  user_button_on_changed(BUTTON_ID_1, on_btn_change);
}





