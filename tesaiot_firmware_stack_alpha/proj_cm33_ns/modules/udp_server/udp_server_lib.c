/*******************************************************************************
 * File Name        : udp_server_lib.c
 *
 * Description      : Implementation of the UDP server library (non-blocking,
 *                    callback-driven, multi-peer).
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure)
 *
 *******************************************************************************/

#include "udp_server_lib.h"

#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "cy_secure_sockets.h"

/*******************************************************************************
 * Macros
 *******************************************************************************/

#define UDP_SERVER_INVALID_PEER_INDEX (-1)

/*******************************************************************************
 * Types
 *******************************************************************************/

typedef struct
{
  uint16_t length;
  cy_socket_sockaddr_t peer;
  uint8_t data[UDP_SERVER_MAX_PAYLOAD_SIZE];
} udp_server_rx_packet_t;

/*******************************************************************************
 * Private Functions
 *******************************************************************************/

/** Returns peer index if found, else UDP_SERVER_INVALID_PEER_INDEX. */
static int16_t udp_server_find_peer(const udp_server_t *server, const cy_socket_sockaddr_t *peer)
{
  uint16_t index = 0;

  for (index = 0; index < server->config.max_peers; ++index)
  {
    if (!server->peers[index].in_use)
    {
      continue;
    }

    if ((server->peers[index].addr.port == peer->port) &&
        (server->peers[index].addr.ip_address.version == peer->ip_address.version) &&
        (server->peers[index].addr.ip_address.ip.v4 == peer->ip_address.ip.v4))
    {
      return (int16_t)index;
    }
  }

  return UDP_SERVER_INVALID_PEER_INDEX;
}

/** Returns first free peer slot index, or max_peers if full. */
static uint16_t udp_server_find_free_peer(const udp_server_t *server)
{
  uint16_t index = 0;

  for (index = 0; index < server->config.max_peers; ++index)
  {
    if (!server->peers[index].in_use)
    {
      return index;
    }
  }

  return server->config.max_peers;
}

/** Returns index of oldest (LRU) peer. */
static uint16_t udp_server_find_oldest_peer(const udp_server_t *server)
{
  uint16_t index = 0;
  uint16_t oldest_index = 0;
  TickType_t oldest_ticks = 0;
  bool oldest_set = false;

  for (index = 0; index < server->config.max_peers; ++index)
  {
    if (!server->peers[index].in_use)
    {
      continue;
    }

    if (!oldest_set || (server->peers[index].last_seen_ticks < oldest_ticks))
    {
      oldest_set = true;
      oldest_ticks = server->peers[index].last_seen_ticks;
      oldest_index = index;
    }
  }

  return oldest_index;
}

/** Updates or adds peer; evicts LRU if full. Invokes on_peer_added/on_peer_evicted. */
static void udp_server_update_peer(udp_server_t *server, const cy_socket_sockaddr_t *peer)
{
  int16_t existing_index = udp_server_find_peer(server, peer);
  TickType_t now_ticks = xTaskGetTickCount();

  if (existing_index >= 0)
  {
    server->peers[existing_index].last_seen_ticks = now_ticks;
    server->last_peer_index = existing_index;
    return;
  }

  uint16_t slot = udp_server_find_free_peer(server);
  bool evicted = false;
  uint16_t evicted_index = 0;
  cy_socket_sockaddr_t evicted_peer;

  if (slot >= server->config.max_peers)
  {
    slot = udp_server_find_oldest_peer(server);
    evicted = true;
    evicted_index = slot;
    evicted_peer = server->peers[slot].addr;
  }

  server->peers[slot].in_use = true;
  server->peers[slot].addr = *peer;
  server->peers[slot].last_seen_ticks = now_ticks;
  server->last_peer_index = (int16_t)slot;

  if (!evicted)
  {
    server->peer_count++;
  }

  if (evicted && (server->callbacks.on_peer_evicted != NULL))
  {
    server->callbacks.on_peer_evicted(server, evicted_index, &evicted_peer, server->callbacks.user_ctx);
  }

  if (server->callbacks.on_peer_added != NULL)
  {
    server->callbacks.on_peer_added(server, slot, peer, server->callbacks.user_ctx);
  }
}

/** Socket receive callback; enqueues packet for task processing. */
static cy_rslt_t udp_server_recv_callback(cy_socket_t socket_handle, void *arg)
{
  udp_server_t *server = (udp_server_t *)arg;
  udp_server_rx_packet_t packet;
  cy_rslt_t result;
  uint32_t bytes_received = 0;

  if ((server == NULL) || (server->rx_queue == NULL))
  {
    return CY_RSLT_TYPE_ERROR;
  }

  memset(&packet, 0, sizeof(packet));
  result = cy_socket_recvfrom(socket_handle, packet.data, server->config.max_payload_size,
                              CY_SOCKET_FLAGS_NONE, &packet.peer, NULL, &bytes_received);

  if (result != CY_RSLT_SUCCESS)
  {
    if (server->callbacks.on_error != NULL)
    {
      server->callbacks.on_error(server, result, server->callbacks.user_ctx);
    }

    return result;
  }

  packet.length = (uint16_t)bytes_received;
  if (packet.length > 0)
  {
    if (xQueueSend(server->rx_queue, &packet, 0) != pdPASS)
    {
      if (server->callbacks.on_error != NULL)
      {
        server->callbacks.on_error(server, CY_RSLT_TYPE_ERROR, server->callbacks.user_ctx);
      }
    }
  }

  return result;
}

/*******************************************************************************
 * Public API
 *******************************************************************************/

/** Initializes server. Validates config. Call before any other API. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t udp_server_lib_init(udp_server_t *server,
                          const udp_server_config_t *config,
                          const udp_server_callbacks_t *callbacks)
{
  if ((server == NULL) || (config == NULL) || (callbacks == NULL))
  {
    return CY_RSLT_TYPE_ERROR;
  }

  if (config->max_peers == 0U || config->max_peers > UDP_SERVER_MAX_PEERS)
  {
    return CY_RSLT_TYPE_ERROR;
  }

  if (config->max_payload_size == 0U || config->max_payload_size > UDP_SERVER_MAX_PAYLOAD_SIZE)
  {
    return CY_RSLT_TYPE_ERROR;
  }

  if (config->rx_queue_length == 0U || config->rx_queue_length > UDP_SERVER_RX_QUEUE_LENGTH)
  {
    return CY_RSLT_TYPE_ERROR;
  }

  memset(server, 0, sizeof(*server));
  server->config = *config;
  server->callbacks = *callbacks;
  server->socket_handle = CY_SOCKET_INVALID_HANDLE;
  server->last_peer_index = UDP_SERVER_INVALID_PEER_INDEX;

  server->bind_addr.port = server->config.port;
  server->bind_addr.ip_address.version = CY_SOCKET_IP_VER_V4;
  server->bind_addr.ip_address.ip.v4 = server->config.bind_ip_v4;

  server->rx_queue = xQueueCreate(server->config.rx_queue_length, sizeof(udp_server_rx_packet_t));
  if (server->rx_queue == NULL)
  {
    return CY_RSLT_TYPE_ERROR;
  }

  return CY_RSLT_SUCCESS;
}

/** Creates and binds socket. Call cy_socket_init() first. Idempotent if already started. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t udp_server_socket_start(udp_server_t *server)
{
  cy_rslt_t result;
  cy_socket_opt_callback_t recv_option;
  uint32_t timeout_ms = 0;

  if (server == NULL)
  {
    return CY_RSLT_TYPE_ERROR;
  }

  if (server->socket_handle != CY_SOCKET_INVALID_HANDLE)
  {
    return CY_RSLT_SUCCESS;
  }

  result = cy_socket_create(CY_SOCKET_DOMAIN_AF_INET, CY_SOCKET_TYPE_DGRAM, CY_SOCKET_IPPROTO_UDP,
                            &server->socket_handle);
  if (result != CY_RSLT_SUCCESS)
  {
    return result;
  }

  recv_option.callback = udp_server_recv_callback;
  recv_option.arg = server;
  result = cy_socket_setsockopt(server->socket_handle, CY_SOCKET_SOL_SOCKET, CY_SOCKET_SO_RECEIVE_CALLBACK,
                                &recv_option, sizeof(recv_option));
  if (result != CY_RSLT_SUCCESS)
  {
    cy_socket_delete(server->socket_handle);
    server->socket_handle = CY_SOCKET_INVALID_HANDLE;
    return result;
  }

  if (server->config.recv_timeout_ms > 0U)
  {
    timeout_ms = server->config.recv_timeout_ms;
    result = cy_socket_setsockopt(server->socket_handle, CY_SOCKET_SOL_SOCKET, CY_SOCKET_SO_RCVTIMEO,
                                  &timeout_ms, sizeof(timeout_ms));
    if (result != CY_RSLT_SUCCESS)
    {
      cy_socket_delete(server->socket_handle);
      server->socket_handle = CY_SOCKET_INVALID_HANDLE;
      return result;
    }
  }

  result = cy_socket_bind(server->socket_handle, &server->bind_addr, sizeof(server->bind_addr));
  if (result != CY_RSLT_SUCCESS)
  {
    cy_socket_delete(server->socket_handle);
    server->socket_handle = CY_SOCKET_INVALID_HANDLE;
    return result;
  }

  return CY_RSLT_SUCCESS;
}

/** Closes socket and clears peers. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t udp_server_stop(udp_server_t *server)
{
  if (server == NULL)
  {
    return CY_RSLT_TYPE_ERROR;
  }

  if (server->socket_handle != CY_SOCKET_INVALID_HANDLE)
  {
    cy_socket_delete(server->socket_handle);
    server->socket_handle = CY_SOCKET_INVALID_HANDLE;
  }

  server->peer_count = 0;
  server->last_peer_index = UDP_SERVER_INVALID_PEER_INDEX;
  memset(server->peers, 0, sizeof(server->peers));

  return CY_RSLT_SUCCESS;
}

/** Sends to last active peer. Fails if no peer yet. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t udp_server_send(udp_server_t *server, const uint8_t *data, size_t length)
{
  uint32_t bytes_sent = 0;

  if ((server == NULL) || (data == NULL) || (length == 0U))
  {
    return CY_RSLT_TYPE_ERROR;
  }

  if (server->last_peer_index < 0)
  {
    return CY_RSLT_TYPE_ERROR;
  }

  return cy_socket_sendto(server->socket_handle, data, (uint32_t)length, CY_SOCKET_FLAGS_NONE,
                          &server->peers[server->last_peer_index].addr, sizeof(cy_socket_sockaddr_t), &bytes_sent);
}

/** Sends to specified peer. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t udp_server_send_to(udp_server_t *server, const uint8_t *data, size_t length,
                             const cy_socket_sockaddr_t *peer)
{
  uint32_t bytes_sent = 0;

  if ((server == NULL) || (data == NULL) || (length == 0U) || (peer == NULL))
  {
    return CY_RSLT_TYPE_ERROR;
  }

  return cy_socket_sendto(server->socket_handle, data, (uint32_t)length, CY_SOCKET_FLAGS_NONE, peer,
                          sizeof(cy_socket_sockaddr_t), &bytes_sent);
}

/** Processes RX queue; invokes on_data for each packet. Returns number of packets processed. */
uint32_t udp_server_process(udp_server_t *server, uint32_t max_packets)
{
  udp_server_rx_packet_t packet;
  uint32_t processed = 0;

  if (server == NULL)
  {
    return 0;
  }

  while (processed < max_packets)
  {
    if (xQueueReceive(server->rx_queue, &packet, 0) != pdPASS)
    {
      break;
    }

    udp_server_update_peer(server, &packet.peer);
    if (server->callbacks.on_data != NULL)
    {
      server->callbacks.on_data(server, packet.data, packet.length, &packet.peer, server->callbacks.user_ctx);
    }

    processed++;
  }

  return processed;
}

/** Returns count of tracked peers. */
uint16_t udp_server_get_peer_count(const udp_server_t *server)
{
  if (server == NULL)
  {
    return 0;
  }

  return server->peer_count;
}

/** Gets peer address by index. Returns true if peer exists and was copied. */
bool udp_server_get_peer(const udp_server_t *server, uint16_t peer_index, cy_socket_sockaddr_t *out_peer)
{
  if ((server == NULL) || (out_peer == NULL) || (peer_index >= server->config.max_peers))
  {
    return false;
  }

  if (!server->peers[peer_index].in_use)
  {
    return false;
  }

  *out_peer = server->peers[peer_index].addr;
  return true;
}

/** Writes bound port to out_port. Returns true if server and out_port are valid. */
bool udp_server_get_local_port(const udp_server_t *server, uint16_t *out_port)
{
  if ((server == NULL) || (out_port == NULL))
  {
    return false;
  }

  *out_port = server->bind_addr.port;
  return true;
}

/** Writes bind IPv4 address to out_ip_v4. Returns true only for IPv4 bind. */
bool udp_server_get_bind_ip_v4(const udp_server_t *server, uint32_t *out_ip_v4)
{
  if ((server == NULL) || (out_ip_v4 == NULL))
  {
    return false;
  }

  if (CY_SOCKET_IP_VER_V4 != server->bind_addr.ip_address.version)
  {
    return false;
  }

  *out_ip_v4 = server->bind_addr.ip_address.ip.v4;
  return true;
}

/** Copies last active peer address to out_peer. Returns true if there is a valid last peer. */
bool udp_server_get_last_peer(const udp_server_t *server, cy_socket_sockaddr_t *out_peer)
{
  int16_t last_index;

  if ((server == NULL) || (out_peer == NULL))
  {
    return false;
  }

  last_index = server->last_peer_index;
  if ((last_index < 0) || ((uint16_t)last_index >= server->config.max_peers))
  {
    return false;
  }

  if (!server->peers[(uint16_t)last_index].in_use)
  {
    return false;
  }

  *out_peer = server->peers[(uint16_t)last_index].addr;
  return true;
}

/* [] END OF FILE */
