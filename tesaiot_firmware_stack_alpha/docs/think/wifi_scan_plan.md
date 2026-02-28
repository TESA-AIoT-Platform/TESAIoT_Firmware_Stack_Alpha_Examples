# WiFi Scan Results Storage and CM55 Request Plan

## Overview
Extend the CM33 scan flow so that when a scan operation completes, WiFi results are stored in an array (sorted by RSSI) and can be requested later by the CM55 core via IPC.

## Goals
1. When the scan operation is completed, save WiFi information in an array so the CM55 core can request it later.
2. Sort the saved array by RSSI (strongest signal first).

## Current Behavior
- `scan_task` runs periodically and calls `cy_wcm_start_scan(scan_callback, ...)`.
- `scan_callback` is invoked per AP with `CY_WCM_SCAN_INCOMPLETE` (result in `cy_wcm_scan_result_t`) and once with `CY_WCM_SCAN_COMPLETE`.
- Results are only printed; no persistent storage. CM55 has no access to scan data.

## Proposed Changes

### 1. CM33: Store scan results in an array

**Location**: `proj_cm33_ns` (e.g. `scan_task.c` and optionally a small shared module for the array access).

**Data**:
- Use the existing `wifi_info_t` from `shared/include/ipc_communication.h` (ssid, rssi, channel, mac, security) for each stored AP.
- Define a maximum number of APs to store (e.g. `WIFI_SCAN_RESULT_MAX` = 20 or 32).
- Maintain a **published** array of `wifi_info_t` and a count, readable by the IPC handler when CM55 requests scan results.

**Flow**:
1. **During scan**: On each `scan_callback` with `CY_WCM_SCAN_INCOMPLETE` and valid `result_ptr`, append the AP to a **working** array (convert `cy_wcm_scan_result_t` to `wifi_info_t`), up to the max size. Keep optional `print_scan_result` behavior if desired.
2. **On scan complete**: When `scan_callback` is called with `CY_WCM_SCAN_COMPLETE`:
   - Sort the working array by RSSI (descending: higher RSSI first).
   - Copy the sorted entries (and count) into the **published** array/count used for CM55 requests.
   - Then notify the scan task with `xTaskNotify` as today.

**Conversion**: Map `cy_wcm_scan_result_t` → `wifi_info_t`: SSID, `signal_strength` → rssi, channel, BSSID → mac, and security enum → string (reuse existing security string table in `scan_task.c`).

**Thread safety**: Access to the published array and count from the IPC path must be safe (e.g. single writer in scan context, reader in IPC; consider a critical section or volatile count + copy-out on CM55 request if needed).

### 2. Sort by RSSI

- Sort the working list by **RSSI descending** (strongest first) before copying to the published array.
- Implementation: simple sort (e.g. insertion or qsort) on the array of `wifi_info_t` (or on indices) by the `rssi` field. Prefer a bounded in-place sort to avoid dynamic allocation.

### 3. CM55 request path

**Protocol** (align with `docs/ipc_data_structure_plan.md`):
- CM55 sends a request to CM33 using **`IPC_CMD_WIFI_SCAN_LIST`** when it needs the WiFi scan list.
- CM33 IPC handler: on `IPC_CMD_WIFI_SCAN_LIST`, reply with the stored scan results.

**Response format** (choose one and document):
- **Option A**: One IPC message per AP: for each entry in the published array (up to count), send a message with `cmd = IPC_CMD_WIFI_STATUS` and `data` cast as `wifi_info_t`. CM55 receives a sequence of WiFi info messages.
- **Option B**: Single “scan list” message: define a structure (e.g. count + fixed-size array of `wifi_info_t`) that fits in or spans the IPC payload, and send one (or a few) messages with a new cmd. CM55 parses once for the full list.

**Recommendation**: Start with Option A (reuse `IPC_CMD_WIFI_STATUS` and `wifi_info_t` per message) for minimal protocol change; introduce Option B later if a single-message response is preferred.

### 4. Summary of implementation steps

| Step | Action |
|------|--------|
| 1 | Add `WIFI_SCAN_RESULT_MAX` and working + published arrays of `wifi_info_t` (and count) in CM33. |
| 2 | In `scan_callback`, on each `CY_WCM_SCAN_INCOMPLETE`, append converted result to working array (cap at max). |
| 3 | On `CY_WCM_SCAN_COMPLETE`, sort working array by RSSI descending, then copy to published array and set count; then notify task. |
| 4 | Expose a getter (or direct access) for published array/count for the CM33 IPC handler. |
| 5 | In CM33 IPC handler, on `IPC_CMD_WIFI_SCAN_LIST` request from CM55, send back one message per stored AP using `IPC_CMD_WIFI_STATUS` and `wifi_info_t` in `data`. |
| 6 | On CM55, send `IPC_CMD_WIFI_SCAN_LIST` when the scan list is needed; handle incoming `IPC_CMD_WIFI_STATUS` messages and aggregate into a local list for the UI or application. |

## Files to touch

- **proj_cm33_ns**: `scan_task.c` (accumulate, sort, publish), `scan_task.h` (optional: declare getter / max size). If IPC lives in another file (e.g. `cm33_ipc_pipe.c`), that file must call the getter and send the reply.
- **shared/include/ipc_communication.h**: Defines `IPC_CMD_WIFI_SCAN_LIST` (0x93) for the request; add a new payload type here only if adopting Option B.
- **proj_cm55**: Send `IPC_CMD_WIFI_SCAN_LIST` when the scan list is needed; IPC callback handles `IPC_CMD_WIFI_STATUS` and presents the list to the application/UI.

## Dependencies

- Existing `wifi_info_t`, `IPC_CMD_WIFI_STATUS`, and `IPC_CMD_WIFI_SCAN_LIST` in `ipc_communication.h`.
- Existing scan flow and `scan_callback` in `scan_task.c`.
- IPC request/response behavior described in `ipc_data_structure_plan.md`.

---

## Summary of changes (implemented)

- **scan_task.h**: Added `ipc_communication.h`, `WIFI_SCAN_RESULT_MAX` (20), and `scan_task_get_published_results()`.
- **scan_task.c**: Working and published arrays of `wifi_info_t` with count; `scan_security_to_string()`, `scan_result_to_wifi_info()`, and `sort_working_by_rssi_desc()` (insertion sort by RSSI desc). Callback appends to working list on `CY_WCM_SCAN_INCOMPLETE` (capped), and on `CY_WCM_SCAN_COMPLETE` sorts, copies to published under critical section, then notifies. Scan task resets working count before each scan. Getter copies published list/count under critical section.
- **cm33_ipc_pipe.c**: On `IPC_CMD_WIFI_SCAN_LIST`, calls getter, then sends one `IPC_CMD_WIFI_STATUS` message per AP with `value = (count << 16) | index` and `data` as `wifi_info_t`; clears `msg_cmd` after handling.
- **cm55_ipc_pipe.c**: Callback decodes `IPC_CMD_WIFI_STATUS` into `s_wifi_list[]` (count/index from `value`); sets `s_wifi_list_ready` when list complete. IPC task prints “WiFi list N APs” when ready. Send task alternates `IPC_CMD_START` and `IPC_CMD_WIFI_SCAN_LIST` every 1.5 s (scan list every 3 s).

## What to do next

- **Expose list to UI**: Have the CM55 application or LVGL UI read `s_wifi_list` and `s_wifi_list_count` when `s_wifi_list_ready` (or provide an API that returns a snapshot) and display the AP list (e.g. list or dropdown).
- **Request on demand**: Replace or supplement the periodic `IPC_CMD_WIFI_SCAN_LIST` send in `cm55_ipc_send_task` with a request triggered by user action (e.g. “Refresh” or opening a WiFi settings screen).
- **Optional Option B**: If a single-message response is preferred, define a scan-list payload (e.g. count + array) in `ipc_communication.h`, extend CM33 to send one (or few) messages with that payload, and update CM55 to parse it.
- **Validation**: Run on hardware; confirm CM33 scan fills the list, CM55 receives the burst of `IPC_CMD_WIFI_STATUS`, and the assembled list matches (count and order by RSSI).

