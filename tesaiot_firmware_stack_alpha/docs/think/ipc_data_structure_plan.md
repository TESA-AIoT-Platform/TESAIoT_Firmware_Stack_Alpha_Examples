# IPC Structured Data Implementation Plan (Legacy Reference)

## Overview
This document is a historical planning note. The current implementation has already evolved and should be validated against `shared/include/ipc_communication.h`, CM33/CM55 IPC modules, and module user manuals.

## Proposed Changes

### 1. Shared Header Update (`ipc_communication.h`)

**Goal**: Support structured data types via a single payload buffer; wifi and gyro data are sent in separate messages based on request.

**Details**:
- Increase `IPC_DATA_MAX_LEN` to accommodate larger structures (e.g., 128 bytes).
- Define data structures used when sending structured payloads:
    - `wifi_info_t`: For Wi-Fi status. Fields map to: **SSID** | **RSSI** | **Channel** | **MAC Address** | **Security**.
    - `gyro_data_t`: For accelerometer/gyroscope data.
- Keep a single `char data[IPC_DATA_MAX_LEN]` in `ipc_msg_t`. The buffer is interpreted as legacy string, `wifi_info_t`, or `gyro_data_t` according to `cmd`; only one payload type is sent per message.

**Code Snippet**:
```c
#define IPC_DATA_MAX_LEN (128UL)

typedef struct {
  char ssid[64];
  int32_t rssi;
  uint8_t channel;
  uint8_t mac[6];
  char security[32];
} wifi_info_t;

typedef struct {
  float ax;
  float ay;
  float az;
} gyro_data_t;

typedef struct {
  uint8_t client_id;
  uint16_t intr_mask;
  uint8_t cmd;
  uint32_t value;
  char data[IPC_DATA_MAX_LEN];
} ipc_msg_t;
```

### 2. CM33 Project Updates (`proj_cm33_ns`)

No change to existing `msg.data` usage; payload remains `char data[IPC_DATA_MAX_LEN]`.

### 3. CM55 Project Updates (`proj_cm55`)

No change to existing receiver logic; payload remains `msg->data`.

---

## New Feature Usage (Future Implementation)

Wifi and gyro are sent in separate messages based on request. The payload buffer is cast or copied to the appropriate struct.

**CM33 Side (Sender)** – wifi in one message:
```c
cm33_msg_data.cmd = IPC_CMD_WIFI_STATUS;
wifi_info_t *w = (wifi_info_t *)cm33_msg_data.data;
strncpy(w->ssid, "MyNetwork", sizeof(w->ssid) - 1);
w->rssi = -60;
w->channel = 6;
strncpy(w->security, "WPA2", sizeof(w->security) - 1);
Cy_IPC_Pipe_SendMessage(..., (void *)&cm33_msg_data, ...);
```

**CM33 Side (Sender)** – gyro in a separate message:
```c
cm33_msg_data.cmd = IPC_CMD_GYRO;
gyro_data_t *g = (gyro_data_t *)cm33_msg_data.data;
g->ax = 0.1f; g->ay = 0.2f; g->az = 0.3f;
Cy_IPC_Pipe_SendMessage(..., (void *)&cm33_msg_data, ...);
```

**CM55 Side (Receiver)**:
```c
if (msg->cmd == IPC_CMD_WIFI_STATUS) {
    wifi_info_t *w = (wifi_info_t *)msg->data;
    process_wifi(w->ssid, w->rssi, w->channel, w->mac, w->security);
} else if (msg->cmd == IPC_CMD_GYRO) {
    gyro_data_t *g = (gyro_data_t *)msg->data;
    process_gyro(g->ax, g->ay, g->az);
}
```

---

## Summary of Updates

- **`shared/include/ipc_communication.h`**
  - `IPC_DATA_MAX_LEN` increased from 64 to 128.
  - Added `wifi_info_t` (SSID, RSSI, Channel, MAC address, Security) and `gyro_data_t` (ax, ay, az).
  - Added command codes `IPC_CMD_WIFI_STATUS` (0x91) and `IPC_CMD_GYRO` (0x92).
  - `ipc_msg_t` unchanged: single `char data[IPC_DATA_MAX_LEN]`; payload type is determined by `cmd`.
- **CM33 (`proj_cm33_ns`)** and **CM55 (`proj_cm55`)**
  - No code changes; existing use of `msg.data` and `IPC_DATA_MAX_LEN` remains valid.

## What to Do Next

### 1. Handle structured payloads in CM55

In `cm55_ipc_pipe.c`, extend `cm55_msg_callback` to branch on `cmd` and interpret `data` as `wifi_info_t` or `gyro_data_t` when `cmd` is `IPC_CMD_WIFI_STATUS` or `IPC_CMD_GYRO`. Keep existing handling for legacy string and `IPC_CMD_LOG` messages.

### 2. Implement Wi‑Fi status on request

- **CM55**: Send a request to CM33 when Wi‑Fi status is needed (e.g., using `IPC_CMD_WIFI_SCAN_LIST`).
- **CM33**: On that request, fill `wifi_info_t` in the shared message (e.g. from WiFi driver: SSID, RSSI, MAC), set `cmd = IPC_CMD_WIFI_STATUS`, and send the message back to CM55.

### 3. Implement gyro data path

- **CM33**: When gyro data is available (periodic or on request), fill `gyro_data_t` in the message buffer, set `cmd = IPC_CMD_GYRO`, and send to CM55.
- **CM55**: In `cm55_msg_callback`, when `cmd == IPC_CMD_GYRO`, cast `msg->data` to `gyro_data_t *` and pass to display or application logic.

### 4. Optional follow‑ups

- Add `process_wifi` / `process_gyro` (or equivalent) on CM55 and hook them from the callback.
- Ensure `wifi_info_t` and `gyro_data_t` layout and alignment are safe for IPC (e.g. packed if needed for ABI).
- Add a simple request/response protocol (e.g. request cmd + value) if CM55 must explicitly request wifi or gyro updates.
