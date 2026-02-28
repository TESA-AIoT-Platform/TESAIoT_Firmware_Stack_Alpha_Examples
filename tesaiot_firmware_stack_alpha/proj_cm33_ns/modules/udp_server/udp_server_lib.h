/*******************************************************************************
 * File Name        : udp_server_lib.h
 *
 * Description      : Function prototypes and definitions for the UDP server
 *                    library (non-blocking, callback-driven, multi-peer).
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure)
 *
 *******************************************************************************/

#ifndef UDP_SERVER_LIB_H_
#define UDP_SERVER_LIB_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "cy_result.h"
#include "cy_secure_sockets.h"

/*******************************************************************************
 * Macros
 *******************************************************************************/

#ifndef UDP_SERVER_MAX_PEERS
#define UDP_SERVER_MAX_PEERS (4U)  /* Max tracked peers (LRU eviction). */
#endif

#ifndef UDP_SERVER_MAX_PAYLOAD_SIZE
#define UDP_SERVER_MAX_PAYLOAD_SIZE (256U)  /* Max RX/TX payload bytes. */
#endif

#ifndef UDP_SERVER_RX_QUEUE_LENGTH
#define UDP_SERVER_RX_QUEUE_LENGTH (8U)  /* RX queue depth. */
#endif

/*******************************************************************************
 * Types
 *******************************************************************************/

typedef struct udp_server udp_server_t;

typedef void (*udp_server_on_data_t)(udp_server_t *server, const uint8_t *data,
                                    size_t length,
                                    const cy_socket_sockaddr_t *peer,
                                    void *user_ctx);

typedef void (*udp_server_on_peer_t)(udp_server_t *server, uint16_t peer_index,
                                    const cy_socket_sockaddr_t *peer,
                                    void *user_ctx);

typedef void (*udp_server_on_error_t)(udp_server_t *server, cy_rslt_t result, void *user_ctx);

typedef struct
{
  udp_server_on_data_t on_data;       /* Invoked when RX packet received. */
  udp_server_on_peer_t on_peer_added; /* Invoked when new peer added. */
  udp_server_on_peer_t on_peer_evicted; /* Invoked when LRU peer evicted. */
  udp_server_on_error_t on_error;     /* Invoked on recv or queue error. */
  void *user_ctx;                     /* Passed to all callbacks. */
} udp_server_callbacks_t;

typedef struct
{
  uint16_t port;            /* Bind port (host byte order). */
  uint32_t bind_ip_v4;      /* Bind IPv4 (little-endian, 0 = any). */
  uint32_t recv_timeout_ms; /* Socket recv timeout (0 = no timeout). */
  uint16_t max_peers;       /* Max peers to track. */
  uint16_t max_payload_size; /* Max RX payload size. */
  uint16_t rx_queue_length; /* RX queue length. */
} udp_server_config_t;

typedef struct
{
  bool in_use;                    /* Slot in use. */
  cy_socket_sockaddr_t addr;      /* Peer address. */
  TickType_t last_seen_ticks;     /* LRU tracking. */
} udp_server_peer_t;

struct udp_server
{
  cy_socket_t socket_handle;      /* Secure socket handle. */
  cy_socket_sockaddr_t bind_addr; /* Bound address. */
  QueueHandle_t rx_queue;         /* RX packet queue. */
  udp_server_callbacks_t callbacks;
  udp_server_config_t config;
  udp_server_peer_t peers[UDP_SERVER_MAX_PEERS];
  uint16_t peer_count;            /* Active peer count. */
  int16_t last_peer_index;        /* Last RX peer index (-1 if none). */
};

/*******************************************************************************
 * Public API
 *******************************************************************************/

/** Initializes server. Validates config. Call before any other API. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t udp_server_lib_init(udp_server_t *server,
                          const udp_server_config_t *config,
                          const udp_server_callbacks_t *callbacks);

/** Creates and binds socket. Call cy_socket_init() first. Idempotent if already started. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t udp_server_socket_start(udp_server_t *server);

/** Closes socket and clears peers. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t udp_server_stop(udp_server_t *server);

/** Sends to last active peer. Fails if no peer yet. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t udp_server_send(udp_server_t *server, const uint8_t *data, size_t length);

/** Sends to specified peer. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t udp_server_send_to(udp_server_t *server, const uint8_t *data, size_t length,
                             const cy_socket_sockaddr_t *peer);

/** Processes RX queue; invokes on_data for each packet. Returns number of packets processed. */
uint32_t udp_server_process(udp_server_t *server, uint32_t max_packets);

/** Returns count of tracked peers. */
uint16_t udp_server_get_peer_count(const udp_server_t *server);

/** Gets peer address by index. Returns true if peer exists and was copied. */
bool udp_server_get_peer(const udp_server_t *server, uint16_t peer_index, cy_socket_sockaddr_t *out_peer);

/** Gets bound port. Returns true on success. */
bool udp_server_get_local_port(const udp_server_t *server, uint16_t *out_port);

/** Gets bind IPv4 (little-endian). Returns true if IPv4 and success. */
bool udp_server_get_bind_ip_v4(const udp_server_t *server, uint32_t *out_ip_v4);

/** Gets last active peer address. Returns true if peer exists and was copied. */
bool udp_server_get_last_peer(const udp_server_t *server, cy_socket_sockaddr_t *out_peer);

#endif /* UDP_SERVER_LIB_H_ */
