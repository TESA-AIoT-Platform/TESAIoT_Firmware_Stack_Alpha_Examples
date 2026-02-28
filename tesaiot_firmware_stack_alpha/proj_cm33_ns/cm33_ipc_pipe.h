#ifndef CM33_IPC_PIPE_H
#define CM33_IPC_PIPE_H

#include "ipc_communication.h"
#include "user_buttons.h"
#include <stdbool.h>
#include <stdint.h>

bool cm33_ipc_pipe_start(void);

bool cm33_ipc_send_gyro_data(const gyro_data_t *data, uint32_t sequence);
bool cm33_ipc_send_button_event(const button_event_t *event);
bool cm33_ipc_send_touch(int16_t x, int16_t y, uint8_t pressed);
bool cm33_ipc_send_wifi_scan_results(const wifi_info_t *results, uint32_t count);
bool cm33_ipc_send_wifi_scan_complete(const ipc_wifi_scan_complete_t *scan_complete);
bool cm33_ipc_send_wifi_status(const ipc_wifi_status_t *status);

bool cm33_ipc_send_ping(void);
bool cm33_ipc_send_cli_message(const char *text);

uint32_t cm33_ipc_get_recv_pending(void);
uint32_t cm33_ipc_get_recv_total(void);
uint32_t cm33_ipc_get_send_queue_used(void);
uint32_t cm33_ipc_get_send_queue_capacity(void);

#endif /* CM33_IPC_PIPE_H */
