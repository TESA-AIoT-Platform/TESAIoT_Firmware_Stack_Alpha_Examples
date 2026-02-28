# Scan Filter Operation

This document describes how the Wi-Fi scan filter works in the CM33 non-secure scan task.

## Overview

The scan task runs on-demand Wi-Fi scans (triggered by CM55 via IPC or by the user button). On each run it may apply **one** filter so that only networks matching that filter are reported. The filter type is selected by the user button: each button press advances to the next filter mode. Filtering is performed by the Cypress Wi-Fi Connection Manager (WCM) when `cy_wcm_start_scan()` is called with a filter; the application does not post-process results by filter type.

## Filter Modes

The active filter is determined by `scan_filter_mode_select` (see `wifi_scan_task.h`):

| Mode               | Description |
|--------------------|-------------|
| `SCAN_FILTER_NONE` | No filter; all visible networks are returned. |
| `SCAN_FILTER_SSID` | Only networks whose SSID matches the configured string. |
| `SCAN_FILTER_MAC`  | Only the AP with the configured MAC (BSSID). |
| `SCAN_FILTER_BAND` | Only networks in the configured band (2.4 GHz, 5 GHz, 6 GHz, or any). |
| `SCAN_FILTER_RSSI` | Only networks with signal strength above the configured minimum RSSI. |

Only one of these is in effect for a given scan. There is no combination of multiple filters in a single scan.

## How the Mode Is Selected

The active filter can be changed in two ways:

1.  **User Button (CM33)**: On each CM33 task loop iteration, if the physical user button was pressed, `scan_filter_mode_select` increments.
2.  **IPC Request (CM55)**: The CM55 can send an `IPC_CMD_WIFI_SET_FILTER` message to directly set the filter mode on the CM33.

The order of cycles is: **NONE → SSID → MAC → BAND → RSSI → NONE → …**

## Configuration Values

Filter parameters are defined in `proj_cm33_ns/wifi_scan_task_config.h`:

| Define                 | Used when           | Meaning |
|------------------------|---------------------|--------|
| `SCAN_FOR_SSID_VALUE`  | `SCAN_FILTER_SSID`  | Exact SSID string to match (e.g. `"SSID"` or your network name). |
| `SCAN_FOR_MAC_ADDRESS` | `SCAN_FILTER_MAC`   | Six-byte AP MAC (BSSID) to match. |
| `SCAN_FOR_BAND_VALUE`  | `SCAN_FILTER_BAND`  | `CY_WCM_WIFI_BAND_*` (e.g. `CY_WCM_WIFI_BAND_ANY`, `CY_WCM_WIFI_BAND_2_4GHZ`). |
| `SCAN_FOR_RSSI_VALUE`  | `SCAN_FILTER_RSSI`  | Minimum RSSI (e.g. `CY_WCM_SCAN_RSSI_FAIR`). |

Change these to match the SSID, MAC, band, or RSSI threshold you want to use when that filter mode is active.

### SSID filter behavior

`SCAN_FOR_SSID_VALUE` is **not** a special keyword. It is the literal network name (SSID) used for an **exact match** when `SCAN_FILTER_SSID` is active. The WCM returns only access points whose SSID equals this string (case-sensitive).

The number of results depends only on how many visible APs have that exact SSID:

- **0** if no AP in range has that name.
- **1** if one AP (e.g. your router) has that name.
- **Up to** `WIFI_SCAN_RESULT_MAX` (20) if multiple APs use that SSID (e.g. same name on multiple bands or repeaters).

Examples: with `"SSID"` you may see many results because that name is common; with `"TERNION"` or `"SSID1234"` you typically see one or zero depending on whether such a network is in range. To list all visible networks regardless of name, use filter mode **NONE** (no filter).

## Scan Flow (High Level)

1. **Task loop** (`wifi_scan_task` in `wifi_scan_task.c`):
   - Waits for notification (`xTaskNotifyWait`) from either CM55 IPC (scan request) or from `scan_callback` (scan complete).
   - If a button press was detected: advance `scan_filter_mode_select` (with wrap to NONE).
   - According to `scan_filter_mode_select`, fill a `cy_wcm_scan_filter_t` with the appropriate mode and the corresponding config value (SSID, MAC, band, or RSSI).
   - If mode is `SCAN_FILTER_NONE`: call `cy_wcm_start_scan(scan_callback, NULL, NULL)` (no filter).
   - Otherwise: call `cy_wcm_start_scan(scan_callback, NULL, &scan_filter)` so WCM applies the filter.
   - Wait for the scan-complete notification from `scan_callback`.
   - Loop back and wait for the next notification.

2. **Results** are delivered in `scan_callback`. Each result is converted to `wifi_info_t` and stored in `s_working_list` (up to `WIFI_SCAN_RESULT_MAX` entries). If `REMOVE_NON_ASCII_SSID` is defined, APs whose SSID contains non-printable ASCII are skipped in this callback.

3. When the scan status is **complete**, the working list is sorted by RSSI (strongest first), then copied into the published list. The scan callback notifies the scan task via `xTaskNotify` and invokes `wifi_scan_manager_on_scan_complete()`.

4. **Result Reporting**: The Manager sends a **Summary Message** (`IPC_CMD_WIFI_SCAN_SUMMARY`) containing the filter details, followed by the individual **Wi-Fi Results** (`IPC_CMD_WIFI_STATUS`) with a 5ms delay between packets.

## Optional: Non-ASCII SSID Filtering

If `REMOVE_NON_ASCII_SSID` is defined in `wifi_scan_task_config.h`, the scan callback additionally skips any AP whose SSID contains bytes outside the printable ASCII range (0x20–0x7E). This is applied **after** WCM returns results and does not change which filter mode (SSID/MAC/band/RSSI) is used for the scan itself.

## Summary

- **One filter per scan**: NONE, SSID, MAC, BAND, or RSSI.
- **User button or IPC** sets the filter mode.
- **Detailed summaries** are sent back to the CM55 to describe the scan context.
- **Automatic throttling** ensures stable transmission of results to the CM55.
- **Filtering** is done by the WCM in `cy_wcm_start_scan()`; the application only configures the filter and collects the (already filtered) results in `scan_callback`.
