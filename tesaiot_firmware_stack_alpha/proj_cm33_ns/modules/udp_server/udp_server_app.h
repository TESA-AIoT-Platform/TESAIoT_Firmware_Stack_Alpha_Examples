/*******************************************************************************
 * File Name        : udp_server_app.h
 *
 * Description      : API for the UDP server application (init, start, stop,
 *                    process, send, LED toggle, status).
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure)
 *
 *******************************************************************************/

#ifndef UDP_SERVER_APP_H_
#define UDP_SERVER_APP_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Initializes UDP server (port 57345). Call before start/process/send. Returns true on success.
 */
bool udp_server_app_init(void);

/** Starts server socket; call after Wi-Fi connected. Idempotent. */
void udp_server_app_start(void);

/** Stops server socket and clears peers. */
void udp_server_app_stop(void);

/** Processes RX queue; invoke from main loop. */
void udp_server_app_process(void);

/** Sends data to last peer; no-op if no peer. */
void udp_server_app_send(const uint8_t *data, size_t length);

/** Sends LED toggle cmd ('0'/'1') to all tracked peers. */
void udp_server_app_send_led_toggle(void);

typedef struct
{
  uint16_t port;        /* Bind port. */
  uint16_t peer_count;  /* Number of tracked peers. */
  uint32_t local_ip_v4; /* Local IPv4 (host byte order). */
} udp_server_app_status_t;

/**
 * Fills out with port, peer count, and local IPv4. Returns false if out NULL or not initialized.
 */
bool udp_server_app_get_status(udp_server_app_status_t *out);

#endif
