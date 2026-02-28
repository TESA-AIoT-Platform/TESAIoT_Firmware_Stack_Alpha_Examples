# Changelog

#### **[2026-02-23]**

- **Refactoring**
  - **UART ownership**: Finalized CM33 as sole debug UART owner. CM55 no longer initializes or directly accesses UART.
  - **CM55 stdout path**: CM55 `_write()` now routes stdout/stderr over IPC using `IPC_CMD_PRINT`; CM33 prints forwarded text on UART.
  - **IPC log transport**: Updated CM33 log transport worker to print locally (`fputs`/`fflush`) and removed `printf` macro redirection from `ipc_log.h`.

- **Documentation**
  - Updated architecture and module docs to match current print/log behavior (CM33 direct UART, CM55 print via IPC, legacy `IPC_CMD_LOG` references corrected).

#### **[2026-02-10]**

- **Refactoring**
  - **IPC pipe (CM55)**: Replaced polling and boolean flag with FreeRTOS work queue for event-driven processing. Introduced `ipc_event_type_t` and `ipc_work_item_t`; callback pushes work items via `xQueueSendFromISR`, receiver task processes LOG, GYRO, WIFI_COMPLETE, and BUTTON events in a single switch.
  - **Error handler (CM55)**: Added `proj_cm55/modules/error_handler/` with CM55-specific `handle_error()`, decoupling from proj_cm33_ns.
  - **cm55_ipc_pipe_start**: Replaced repeated cleanup blocks with goto-based cleanup labels; added proper resource teardown (queues, tasks) before `handle_error` on failure paths.

- **Documentation**
  - **cm55_ipc_pipe**: Added file header and function comments following user_buttons style.

#### **[2026-02-09]**

- **WiFi scanner**: Added `wifi_scanner_on_scan_complete(callback, user_data)` to allow registration of up to 8 callbacks for scan completion events. `scanner_callback()` invokes all registered callbacks with `(user_data, results, count)`; config callback is still invoked if set. CM33 IPC pipe registers in `cm33_ipc_pipe_start()`; main no longer passes callback in scanner config.

- **User buttons**: Replaced event bus with callback-only API. Callbacks now receive `(user_buttons_t switch_handle, const button_event_t *evt)`. Removed `user_buttons_get_bus()` and internal event bus; bridge task invokes registered callbacks directly. CM33 IPC pipe registers via `user_button_on_changed()` for both buttons. Simplified `user_buttons_configure()` to `(handle, port, pin, stack_size, priority)` (no event IDs).

#### **[2026-02-08]**

- **Features**
  - **WiFi RSSI Monitoring**: Implemented asynchronous RSSI requests from CM55 to CM33. CM33 triggers a hardware-filtered Wi-Fi scan to retrieve real-time signal strength for requested SSIDs.
  - **Targeted SSID Filtered Scan**: Added rich data retrieval (MAC, Channel, Security) for specific access points using dedicated IPC commands (0x96/0x97).
  - **Dynamic Hardware Filtering**: Optimized the CM33 `wifi_scan_task` to apply hardware radio filters directly for dynamic scan requests, improving discovery speed.

- **Refactoring**
  - **Unified IPC Sender (CM55)**: Consolidated three periodic request tasks into a single queue-driven `cm55_ipc_sender_task`. This allows UI components to trigger Wi-Fi scans and RSSI updates on-demand via a simple API.
  - **IPC Command Queue**: Introduced `s_ipc_send_queue` with 100ms throttling to handle burst requests from multiple UI widgets without overwhelming the IPC hardware buffer.

- **Improvements**
  - **UI Data Persistence**: Implemented `s_wifi_list_valid` flag and `cm55_get_rssi_status` on CM55 to ensure UI widgets maintain persistent data and fast signal updates, even after console formatting is complete.
  - **Scan Summaries**: Enhanced `print_wifi_list` to always display the summary line (filter mode and parameters) before results, supporting verification of targeted scans.

- **Bug Fixes**
  - **Stale Wi-Fi Data**: Fixed a race condition where the UI would show `--` after the console finished printing.
  - **Empty Scan Handling**: Corrected logic to ensure CM55 receives an explicit completion signal for scans with 0 results, clearing stale UI data correctly.


#### **[2026-02-07]**

- **Features**
  - **WiFi RSSI Monitoring**: Implemented asynchronous RSSI requests from CM55 to CM33. CM33 now triggers a physical Wi-Fi scan to retrieve real-time signal strength for requested SSIDs.
  - **Targeted SSID Filtered Scan**: Added rich data retrieval (MAC, Channel, Security) for specific access points using new IPC commands (0x96/0x97).
  - **Dynamic Hardware Filtering**: Optimized the CM33 `wifi_scan_task` to handle dynamic SSID requests by applying hardware-level filters directly to the radio scan, significantly improving targeted discovery speed.

- **Refactoring**
  - **Structured IPC Protocol**: Introduced a unified `wifi_scan_request_t` structure (enum mode + union params) for clearer and more extensible communication between cores.
  - **IPC Safety & Alignment**: Refactored `ipc_msg_t` for 32-bit alignment and introduced dedicated RX/TX buffers (`cmxx_rx_msg`) to eliminate race conditions and data corruption.

- **Documentation**
  - **WIFI_SCAN_FILTER.md**: Created detailed documentation for the new targeted scan feature.
  - **GET_RSSI_BY_SSID.md**: Updated to focus exclusively on the RSSI signal monitoring protocol.
  - **WIFI_SCAN_ARCHITECTURE.md**: Rewritten to include dynamic filtering logic, operation mode comparisons, and the new structured request format.

- **Bug Fixes**
  - **Build errors**: Resolved variable scope issues in CM33 scan task and struct member mismatches in CM55 UI callback.
  - **Logging**: Fixed a misleading log in the Wi-Fi task that displayed "Scanning without filter" during active dynamic scans.
  - **RSSI Console Flooding**: Refactored `wifi_scan_manager.c` to use hardware filtering for RSSI requests, preventing the console from being flooded with 20 items during periodic monitoring.

- **Tools/Setup**
  - **Automation**: Created `SKILL.md` and `.agent/workflows/` (build, program, clean, getlibs) to provide a standardized, cross-shell build environment for AI agents. 
  - **BUILD_SETUP.md**: Reorganized with clear shell-specific (Git Bash vs PowerShell) instructions and permanent vs. per-session environment setup.
  - **FIX_CLANG_IDE_ERRORS.md**: Added `docs/FIX_CLANG_IDE_ERRORS.md` with clangd error fixes (LVGL include path, suppress undeclared_var_use, -Wimplicit-int, func_returning_array_function, typecheck_cond_expect_scalar, or ignore all with `'*'`). Updated `.clangd` with LVGL path and diagnostic suppressions; `ipc_communication.h` and `cm33_ipc_pipe.h` include `stdint.h` before PDL headers.

#### **[2026-02-06]**

- **Bug fixes**
  - **Gyro data (CM55)**: Fixed CM55 not printing gyro data from CM33. Root cause: `last_msg_cmd` was overwritten by counter messages before the gyro print block ran. Now check `s_gyro_data_ready` independently and store `s_gyro_sequence` in the gyro callback; reduced CM33 `ipc_task` counter traffic from 500ms to 3000ms.
  
- **Refactoring**
  - **WiFi scan (CM33)**: Renamed `scan_task.*` and `scan_task_config.h` to `wifi_scan_task.*` and `wifi_scan_task_config.h`; renamed symbols (e.g. `scan_task_handle` → `wifi_scan_task_handle`, `SCAN_TASK_STACK_SIZE` → `WIFI_SCAN_TASK_STACK_SIZE`) for consistency. Updated docs (wifi_scan_architecture.md, README.md, SKILL.md).

#### **[2026-02-05]**

- **Documentation**
  - **TOOLCHAIN_CONFIGRATION.md**: Updated with more detailed instructions for configuring the toolchain; added sections for Windows users and clarified the purpose of the Makefile and environment variable options; documented Git Bash modification and `.vscode/tasks.json` modification.

#### **[2026-02-04]**

- **Features**
  - **Scan task**: Added optional filter to exclude APs with non-printable ASCII SSIDs (controlled by `REMOVE_NON_ASCII_SSID` in `scan_task_config.h`).

- **Refactoring**
  - **user_button**: Extracted user button handling to `user_button.c` / `user_button.h`.
  - **scan_task_config.h**: Consolidated all scan control flags and configuration; organized into sections and added comments for each item.
  - **scan_task.c**: Grouped static variables (scan result arrays, SDIO/WCM) with section comments; scan results sorted by RSSI (strongest first); moved REMOVE_NON_ASCII_SSID logic and constants to config.

- **Build**
  - **common.mk**: Changed toolchain from GCC_ARM to LLVM_ARM.
  - **proj_cm33_ns**: Added `-Wno-macro-redefined` to CFLAGS to suppress `MEM_SIZE` redefinition warning (lwIP `opt.h` vs GFXSS `viv_dc_options.h` in `cy_network_mw_core.c`).

- **Documentation**
  - **README.md**: Updated to reflect the latest changes; added instructions for using Git Bash with ModusToolbox.

#### **[2026-01-31]**

- **Bug fixes**
  - **IPC pipe**: Corrected CM55 `main.c` return check for `cm55_ipc_pipe_start()` (bool); success was previously treated as failure and disabled interrupts, causing `CY_IPC_PIPE_ERROR_SEND_BUSY` (0x008a0207) on CM33.

- **Features**
  - **Log transport (CM33 → CM55)**: Added `log_queue` and `log_ipc_transport` so CM33 logs are sent to CM55 over the IPC pipe with `IPC_CMD_LOG`; CM55 prints them on its UART. Banner, IPC counter, and errors now use `log_queue_printf()` and appear on the CM55 console.
