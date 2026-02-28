# CM33 CLI Commands — List for UI Design

This document lists all CLI commands so each can be mapped to a UI action or screen. Instead of typing in a terminal, the UI application provides interfaces for every command.

> **Warning**
> `imu fusion ...` features depend on BSXlite on CM33 and are supported only with the `GCC_ARM` toolchain.
> Fusion functionality is not supported on other CM33 toolchains.

## All commands (26)

| Command   | Description                                      | Subcommands / Parameters                                                                                             |
| --------- | ------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------- |
| help      | List commands and short help                     | (none)                                                                                                                |
| version   | Firmware/CLI version string                       | (none)                                                                                                                |
| clear     | Clear the terminal screen                         | (none)                                                                                                                |
| echo      | Echo arguments to output                          | args... (optional text)                                                                                               |
| uptime    | Print time since boot (seconds)                   | (none)                                                                                                                |
| heap      | Print FreeRTOS heap free (bytes)                  | (none)                                                                                                                |
| time      | Date/time                                        | **Subcommands**: now, full, date, clock, set, sync, ntp. **set** args: hour, min, sec, day, month, year (6 numbers).  |
| date      | Print current date (YYYY-MM-DD)                   | (none)                                                                                                                |
| sysinfo   | System snapshot (uptime, heap, time, tasks)       | (none)                                                                                                                |
| log       | Log path info                                    | **Subcommand**: status                                                                                                |
| tasks     | List FreeRTOS tasks                              | (none)                                                                                                                |
| buttons   | Button states                                    | **Subcommand**: status                                                                                                |
| led       | User LED control                                 | **Subcommands**: on, off, toggle                                                                                      |
| mac       | Print WiFi MAC address                            | (none)                                                                                                                |
| ip        | Print STA IPv4 address                            | (none)                                                                                                                |
| gateway   | Print default gateway IPv4                        | (none)                                                                                                                |
| netmask   | Print STA netmask IPv4                            | (none)                                                                                                                |
| ping      | Ping IPv4 host                                    | **Parameters**: a.b.c.d required, [timeout_ms] optional (default 2000)                                                |
| stacks    | Task stack high-water marks (bytes free)          | (none)                                                                                                                |
| imu       | IMU/fusion controls and diagnostics               | **Subcommands**: status, data, stream, sample, fusion, calib, swap, help. stream: status, on, off; sample: status, rate &lt;hz&gt;; fusion: status, mode quat&#124;euler&#124;data, on, off; calib: status, reset; swap: status, on, off. |
| touch     | Touch and touch-IPC diagnostics                   | **Subcommands**: status, stream, ipc status. stream: status, on, off; ipc: status. No-arg prints usage. |
| wifi      | WiFi operations                                   | **Subcommands**: scan, connect, disconnect, status, list, info. **connect** args: ssid required, [pass] optional.     |
| udp       | UDP server / send                                 | **Subcommands**: start, stop, send, status. **send** arg: msg (message text).                                         |
| ipc       | IPC to CM55                                       | **Subcommands**: ping, send, status, recv. **send** arg: msg.                                                          |
| reset     | Software reset (like reset button)                | (none)                                                                                                                |
| reboot    | Reboot (same as reset)                            | (none)                                                                                                                |

## UI hints

- **Info only (no arguments, single action)**  
  help, version, clear, uptime, heap, date, sysinfo, log, tasks, mac, ip, gateway, netmask, stacks — one button or menu item that runs the command and shows output.

- **Subcommand picker**  
  time (now / full / date / clock / set / sync / ntp), buttons (status), led (on / off / toggle), imu (status / data / stream / sample / fusion / calib / swap / help), touch (status / stream / ipc status), wifi (scan / connect / disconnect / status / list / info), udp (start / stop / send / status), ipc (ping / send / status / recv). UI: choose subcommand first, then show any parameter inputs.

- **Text or numeric inputs**  
  - **echo**: optional text field.  
  - **time set**: six number inputs (hour, min, sec, day, month, year).  
  - **ping**: IPv4 address (required), timeout_ms (optional, default 2000).  
  - **wifi connect**: SSID (required), password (optional).  
  - **udp send**, **ipc send**: message text (required).
  - **imu stream on**: no input; **imu stream status** has no input.
  - **imu sample rate**: required rate_hz numeric input (runtime IMU sampling/read cadence); **imu sample status** has no input.
  - **imu fusion mode**: enum picker (quat / euler / data).
  - **imu swap**: status (no input), on, off.
  - **touch stream**: toggle (on / off).

- **Destructive (confirm before run)**  
  reset, reboot — show a confirmation dialog before executing.
