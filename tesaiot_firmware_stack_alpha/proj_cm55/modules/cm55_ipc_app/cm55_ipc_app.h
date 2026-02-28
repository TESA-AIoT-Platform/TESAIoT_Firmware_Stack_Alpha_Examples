/*******************************************************************************
 * File Name        : cm55_ipc_app.h
 *
 * Description      : Public API for CM55 IPC application layer. Wi-Fi scan
 *                    triggers, button state, and scan results.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 *
 *******************************************************************************/

#ifndef CM55_IPC_APP_H
#define CM55_IPC_APP_H

#include "ipc_communication.h"
#include "wifi_scanner_types.h"

#include <stdbool.h>
#include <stdint.h>

/** IPC event types: delivered from CM33 to CM55 and dispatched by app receiver task to cm55_ipc_event_cb_t. */
typedef enum
{
  CM55_IPC_EVENT_LOG,
  CM55_IPC_EVENT_GYRO,
  CM55_IPC_EVENT_WIFI_STATUS,
  CM55_IPC_EVENT_WIFI_COMPLETE,
  CM55_IPC_EVENT_BUTTON,
} cm55_ipc_event_t;

/** Payload for log messages (pointer to null-terminated text). */
typedef struct
{
  const char *text; /* Null-terminated log line; may be NULL. */
} cm55_ipc_payload_log_t;

/** Payload for gyro updates (shared gyro_data_t and sequence number). */
typedef struct
{
  const gyro_data_t *data; /* Pointer to gyro sample; may be NULL. */
  uint32_t sequence;       /* Monotonic sequence number for ordering. */
} cm55_ipc_payload_gyro_t;

/** Payload when Wi-Fi scan completes (list and count, up to max list size). */
typedef struct
{
  const wifi_info_t *list; /* Array of scan results; valid only when count > 0. */
  uint32_t count;          /* Number of entries in list. */
} cm55_ipc_payload_wifi_complete_t;

typedef struct
{
  const ipc_wifi_status_t *status;
} cm55_ipc_payload_wifi_status_t;

/** Payload for button events (id, press count, current pressed state). */
typedef struct
{
  uint32_t button_id;   /* Button identifier (e.g. BUTTON_ID_0). */
  uint32_t press_count; /* Total press count since boot. */
  bool is_pressed;      /* true if button is currently held down. */
} cm55_ipc_payload_button_t;

/** Union of all event payloads; use with cm55_ipc_event_t to interpret which member is valid. */
typedef union
{
  cm55_ipc_payload_log_t log;                     /* Valid when event is CM55_IPC_EVENT_LOG. */
  cm55_ipc_payload_gyro_t gyro;                   /* Valid when event is CM55_IPC_EVENT_GYRO. */
  cm55_ipc_payload_wifi_status_t wifi_status;     /* Valid when event is CM55_IPC_EVENT_WIFI_STATUS. */
  cm55_ipc_payload_wifi_complete_t wifi_complete; /* Valid when event is CM55_IPC_EVENT_WIFI_COMPLETE. */
  cm55_ipc_payload_button_t button;               /* Valid when event is CM55_IPC_EVENT_BUTTON. */
} cm55_ipc_event_payload_t;

/** Callback invoked for each typed event by the app receiver task; user_data is optional. */
typedef void (*cm55_ipc_event_cb_t)(cm55_ipc_event_t event, const cm55_ipc_event_payload_t *payload, void *user_data);

/**
 * One-time init: starts pipe (if not already), creates receiver task and work queue. Call before
 * trigger/get API. Returns false on failure.
 */
bool cm55_ipc_app_init(void);

/**
 * Request full Wi-Fi scan on CM33 (no SSID filter). Sends request via pipe; results arrive as
 * CM55_IPC_EVENT_WIFI_COMPLETE.
 */
void cm55_trigger_scan_all(void);

/**
 * Request Wi-Fi scan filtered by SSID on CM33. ssid may be NULL. Results arrive as
 * CM55_IPC_EVENT_WIFI_COMPLETE.
 */
void cm55_trigger_scan_ssid(const char *ssid);
void cm55_trigger_connect(const char *ssid, const char *password, uint32_t security);
void cm55_trigger_disconnect(void);
void cm55_trigger_status_request(void);

/**
 * Read current button state; press_count and is_pressed may be NULL. Returns false if button_id
 * invalid.
 */
bool cm55_get_button_state(uint32_t button_id, uint32_t *press_count, bool *is_pressed);

/**
 * Copy up to max_count scan results into out_list and set out_count. Clears ready flag. Returns false if scan not
 * ready or args invalid.
 */
bool cm55_get_wifi_list(wifi_info_t *out_list, uint32_t max_count, uint32_t *out_count);
bool cm55_get_wifi_status(ipc_wifi_status_t *out_status);
bool cm55_get_wifi_debug_text(char *out_text, uint32_t out_size);
uint32_t cm55_get_wifi_debug_sequence(void);

#endif
