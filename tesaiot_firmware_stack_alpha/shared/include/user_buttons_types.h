/*******************************************************************************
 * File Name        : user_buttons_types.h
 *
 * Description      : Shared types for user button IPC payloads (CM33/CM55).
 *                    Minimal dependencies (stdbool, stdint) for use on both cores.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 *
 *******************************************************************************/

#ifndef USER_BUTTONS_TYPES_H
#define USER_BUTTONS_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
  BUTTON_ID_0 = 0,
  BUTTON_ID_1,
  BUTTON_ID_MAX
} button_id_t;

typedef enum
{
  BUTTON_EVENT_RELEASED = 0,
  BUTTON_EVENT_PRESSED,
  BUTTON_EVENT_CHANGED,
  BUTTON_EVENT_MAX
} button_event_id_t;

typedef enum
{
  BUTTON_STATE_RELEASED = 0,
  BUTTON_STATE_PRESSED,
  BUTTON_STATE_MAX
} button_state_t;

typedef struct
{
  uint32_t button_id;
  uint32_t press_count;
  bool is_pressed;
} button_event_t;

#endif /* USER_BUTTONS_TYPES_H */
