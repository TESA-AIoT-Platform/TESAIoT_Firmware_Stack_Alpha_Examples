# WiFi Scan Architecture - CM33 and CM55 Collaboration

This document explains how the CM33 and CM55 cores work together to perform on-demand Wi-Fi scanning with advanced filtering capabilities.

## System Architecture

```mermaid
graph TB
    subgraph CM55["CM55 Core (UI/UX & Display)"]
        CM55_UI[Application/Tasks]
        CM55_Queue[IPC Send Queue]
        CM55_Sender[IPC Sender Task]
        CM55_IPC[IPC Pipe EP2]
        CM55_Receiver[IPC Receiver Task]
        CM55_Print[Result Printer]
    end
    
    subgraph CM33["CM33 Core (Connectivity)"]
        CM33_IPC[IPC Pipe EP1]
        CM33_Task[IPC Task]
        WifiManager[WiFi Scan Manager]
        WifiScanTask[WiFi Scan Task]
        WiFiHW[WiFi Hardware<br/>SDIO/WCM]
    end
    
    CM55_UI -->|Trigger API| CM55_Queue
    CM55_Queue --> CM55_Sender
    CM55_Sender -->|ipc_msg_t| CM55_IPC
    CM55_IPC -->|IPC_CMD_WIFI_RSSI_REQ<br/>IPC_CMD_WIFI_FILTER_SCAN_REQ| CM33_IPC
    CM33_IPC -->|Parse Packet| CM33_Task
    CM33_Task -->|Trigger Request| WifiManager
    WifiManager -->|Trigger Dynamic Scan| WifiScanTask
    WifiScanTask -->|cy_wcm_start_scan| WiFiHW
    WiFiHW -->|scan_callback| WifiScanTask
    WifiScanTask -->|Notify Complete| WifiManager
    WifiManager -->|1. Summary<br/>2. Results| CM33_IPC
    CM33_IPC -->|IPC_CMD_WIFI_SCAN_SUMMARY<br/>IPC_CMD_WIFI_STATUS| CM55_IPC
    CM55_IPC -->|Task Processing| CM55_Receiver
    CM55_Receiver -->|Trigger Print| CM55_Print
```

## Operation Modes

| Mode | Purpose | Request Command | Response Format | Filtering Logic |
| :--- | :--- | :--- | :--- | :--- |
| **Global Scan** | Retrieve full list of APs | `0x93` | `wifi_scan_summary_t` + multiple `wifi_info_t` | Software (all APs) |
| **RSSI Monitor** | Periodic signal update | `0x94` (`wifi_scan_request_t`) | `wifi_status_t` | Managed Search |
| **Filtered Scan** | Details for specific target | `0x96` (`wifi_scan_request_t`) | `wifi_filter_result_t` | **Hardware Filter** |

## Data Flow & Synchronization

To allow the high-performance CM55 core to handle display formatting without starving the CM33's connectivity tasks, the following synchronization is used:

1.  **Summary First**: CM33 sends `IPC_CMD_WIFI_SCAN_SUMMARY` containing result count and active filter parameters.
2.  **Throttled Results**: CM33 iterates through results, sending each as an `IPC_CMD_WIFI_STATUS` message with a **5ms delay** between packets.
3.  **Accumulation**: CM55 listener task copies results into a local buffer.
4.  **Ready Trigger**: Once the final segment (index == count-1) is received, the CM55 sets `s_wifi_list_ready = true`.
5.  **Autonomous Printing**: The CM55's `cm55_ipc_receiver_task` sees the flag and invokes `print_wifi_list()`, providing human-readable console output.

## Sequence Diagram - Global Scan

```mermaid
sequenceDiagram
    participant CM55_App as CM55 Application
    participant CM55_IPC as CM55 IPC Pipe
    participant CM33_IPC as CM33 IPC Pipe
    participant Manager as WiFi Scan Manager
    participant WifiScanTask as WiFi Scan Task
    participant WiFiHW as WiFi Hardware
    
    CM55_App->>CM55_IPC: Send Scan Request (Global)
    CM55_IPC->>CM33_IPC: IPC_CMD_WIFI_SCAN_LIST
    CM33_IPC->>Manager: wifi_scan_manager_request_scan()
    Manager->>WifiScanTask: wifi_scan_task_trigger_scan()
    
    WifiScanTask->>WiFiHW: cy_wcm_start_scan(filter: NONE)
    
    WiFiHW-->>WifiScanTask: scan_callback(Multiple Results)
    WifiScanTask->>Manager: wifi_scan_manager_on_scan_complete()
    
    Manager->>CM33_IPC: 1. Send Summary (Filter Mode, Params)
    Manager->>CM33_IPC: 2. Send Results (Throttled with 5ms delay)
    CM33_IPC->>CM55_IPC: IPC_CMD_WIFI_SCAN_SUMMARY + IPC_CMD_WIFI_STATUS
    Note right of CM55_IPC: Accumulates segments...
    
    CM55_IPC-->>CM55_App: s_wifi_list_ready = true
    Note over CM55_App: s_wifi_list_valid marked true (Persistent)
    Note over CM55_App: CM55 updates local list/state for UI and debug output
```

## Module Responsibilities

### CM33 WiFi Scan Task
- **Initialization**: Sets up SDIO and WCM.
- **Filter Management**: Handles manual filters (button) and dynamic filters (IPC).
- **Sorting**: Ranks results by RSSI before publishing.

### CM33 WiFi Scan Manager
- **State Tracking**: Tracks if a filtered scan is "pending".
- **Result Search**: After a scan, it searches for target SSIDs to satisfy RSSI/Filter requests.
- **IPC Marshalling**: Packages results into `wifi_status_t`, `wifi_filter_result_t`, and throttles `wifi_info_t` segment bursts.

### CM55 Application (IPC Pipe Module)
- **Request Management**: Uses `s_ipc_send_queue` and `cm55_ipc_sender_task` to throttle and manage outgoing requests from UI/Tasks.
- **Result Accumulation**: Reassembles the Wi-Fi list from individual segments.
- **Persistence**: Maintains `s_wifi_list_valid` so UI components can access data even after console printing is complete.
- **Display Logic**: Provides Wi-Fi data/state to CM55 UI components and debug views.

---
*Last updated: 2026-02-08*
