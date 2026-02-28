/*******************************************************************************
 * File Name        : wifi_connect.h
 *
 * Description      : Function prototypes and definitions for the Wi-Fi
 *                    connection manager (connect, disconnect, reconnect).
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure), CYW55513 Wi-Fi
 *
 *******************************************************************************/

#ifndef WIFI_CONNECT_H_
#define WIFI_CONNECT_H_

#include "cy_wcm.h"
#include "cybsp.h"

/*******************************************************************************
 * Types
 *******************************************************************************/

typedef struct wifi_connect wifi_connect_t;

typedef void (*wifi_connect_on_connected_t)(wifi_connect_t *wifi,
                                            const cy_wcm_ip_address_t *ip_addr,
                                            void *user_ctx);
typedef void (*wifi_connect_on_disconnected_t)(wifi_connect_t *wifi, void *user_ctx);

typedef struct
{
  wifi_connect_on_connected_t on_connected;     /**< Called when connected and IP obtained */
  wifi_connect_on_disconnected_t on_disconnected; /**< Called when disconnected */
  void *user_ctx;                               /**< Passed to callbacks */
} wifi_connect_callbacks_t;

typedef struct
{
  uint32_t reconnect_interval_ms; /**< Delay between reconnect attempts (ms) */
} wifi_connect_config_t;

typedef struct
{
  char ssid[32 + 1];              /**< AP SSID, null-terminated */
  char password[64 + 1];          /**< AP password, null-terminated */
  cy_wcm_security_t security;     /**< WPA2 etc. */
  cy_wcm_ip_setting_t ip_setting; /**< DHCP or static */
} wifi_connect_params_t;

/*******************************************************************************
 * Public API
 *******************************************************************************/

/** Allocates handle, initializes SDIO and WCM, creates connection task. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t wifi_connect_init(wifi_connect_t **wifi,
                            const wifi_connect_config_t *config,
                            const wifi_connect_callbacks_t *callbacks);

/** Starts connection to AP. Copies params and notifies task. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t wifi_connect_start(wifi_connect_t *wifi, const wifi_connect_params_t *params);

/** Disconnects from AP and sets state to IDLE. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t wifi_connect_stop(wifi_connect_t *wifi);

/** Deletes task, deregisters WCM callback, deinits WCM and frees handle. Returns CY_RSLT_SUCCESS on success. */
cy_rslt_t wifi_connect_deinit(wifi_connect_t *wifi);

/** Returns true if currently connected to AP. */
bool wifi_connect_is_connected(wifi_connect_t *wifi);

#endif /* WIFI_CONNECT_H_ */
