/*******************************************************************************
 * File Name        : wifi_scanner.h
 *
 * Description      : Function prototypes and definitions for the Wi-Fi scanner.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 *
 *******************************************************************************/

#ifndef WIFI_SCANNER_H
#define WIFI_SCANNER_H

#include "FreeRTOS.h"
#include "cy_wcm.h"
#include "task.h"
#include "wifi_scanner_types.h"
#include <stdbool.h>

/*******************************************************************************
 * Global Variables
 *******************************************************************************/
extern TaskHandle_t wifi_scanner_task_handle;

typedef void (*wifi_scanner_callback_t)(void *user_data, const wifi_info_t *results, uint32_t count);

typedef struct
{
  uint32_t task_priority;
  uint32_t task_stack_size;
  wifi_filter_mode_t filter_mode;
  wifi_scanner_callback_t callback;
  void *user_data;
} wifi_scanner_config_t;

/*******************************************************************************
 * Public API
 *******************************************************************************/
/** Copies config and optionally starts the scanner task. Returns true on success. */
bool wifi_scanner_setup(wifi_scanner_config_t *config, bool auto_start);

/** Creates the Wi-Fi scan task. Returns true if task was created or already running. */
bool wifi_scanner_start(void);

/** Deletes the Wi-Fi scan task. Returns true if task was deleted. */
bool wifi_scanner_stop(void);

/** Triggers a scan with optional filter; NULL keeps current filter. Returns true if scan was triggered. */
bool wifi_scanner_scan(wifi_filter_config_t *filter_config);

/** Returns true if the scanner task exists. */
bool wifi_scanner_is_running(void);

/** Returns true if a scan is in progress. */
bool wifi_scanner_is_scanning(void);

/** Sets SSID filter and triggers a scan. Returns true if scan was triggered. */
bool wifi_scanner_scan_ssid(const char *ssid);

/** Sets BSSID filter and triggers a scan. Returns true if scan was triggered. */
bool wifi_scanner_scan_bssid(const char *bssid);

/** Sets security filter and triggers a scan. Returns true if scan was triggered. */
bool wifi_scanner_scan_security(cy_wcm_security_t security);

/** Sets channel filter and triggers a scan. Returns true if scan was triggered. */
bool wifi_scanner_scan_channel(uint8_t channel);

/** Sets minimum RSSI filter and triggers a scan. Returns true if scan was triggered. */
bool wifi_scanner_scan_rssi(int32_t rssi);

/** Clears filter and triggers a scan (all APs). Returns true if scan was triggered. */
bool wifi_scanner_scan_all(void);

/** Registers a callback for scan-complete events. Multiple callbacks may be registered. Returns false if callback is NULL or list is full. */
bool wifi_scanner_on_scan_complete(wifi_scanner_callback_t callback, void *user_data);

#endif /* WIFI_SCANNER_H */
