/*******************************************************************************
 * File Name        : cm33_cli.c
 *
 * Description      : UART command-line interface on the debug UART. Single task,
 *                    line edit, history, table-driven command dispatch. Uses
 *                    wifi_manager, udp_server_app, IPC pipe, date_time, WCM.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 * Version          : 1.0
 * Target           : PSoC Edge E84, CM33 (non-secure)
 *
 *******************************************************************************/

#include "cm33_cli.h"
#include "cm33_ipc_pipe.h"
#include "date_time.h"
#include "ipc_communication.h"
#include "retarget_io_init.h"
#include "udp_server_app.h"
#include "user_buttons.h"
#include "user_buttons_types.h"
#include "wifi_manager.h"
#include "wifi_scanner_types.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_wcm.h"
#include "cy_result.h"
#ifdef COMPONENT_BSXLITE
#include "sensor_hub_fusion.h"
#endif
#include <FreeRTOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <task.h>

#if (configHEAP_ALLOCATION_SCHEME != HEAP_ALLOCATION_TYPE3)
#include <portable.h>
#endif

#define CM33_CLI_MAX_ARGC  16   /* Maximum number of tokens per line (command + arguments). */
#define CM33_CLI_PROMPT    "cli> "  /* Prompt string printed before each line. */

typedef enum
{
  CM33_CLI_ESC_NONE,     /* Not in an escape sequence. */
  CM33_CLI_ESC_ESC,      /* Saw ESC; waiting for '['. */
  CM33_CLI_ESC_BRACKET   /* Saw ESC [; waiting for A/B (Up/Down). */
} cm33_cli_esc_state_t;

typedef void (*cm33_cli_cmd_fn)(int argc, char *argv[]);

typedef struct
{
  const char   *cmd;   /* Command name (first token). */
  const char   *help;  /* Short help string shown by help. */
  cm33_cli_cmd_fn fn;  /* Handler called with argc and argv. */
} cm33_cli_cmd_t;

static void cm33_cli_cmd_help(int argc, char *argv[]);
static void cm33_cli_cmd_version(int argc, char *argv[]);
static void cm33_cli_cmd_wifi(int argc, char *argv[]);
static void cm33_cli_cmd_udp(int argc, char *argv[]);
static void cm33_cli_cmd_reset(int argc, char *argv[]);
static void cm33_cli_cmd_ipc(int argc, char *argv[]);
static void cm33_cli_cmd_clear(int argc, char *argv[]);
static void cm33_cli_cmd_uptime(int argc, char *argv[]);
static void cm33_cli_cmd_heap(int argc, char *argv[]);
static void cm33_cli_cmd_time(int argc, char *argv[]);
static void cm33_cli_cmd_echo(int argc, char *argv[]);
static void cm33_cli_cmd_log(int argc, char *argv[]);
static void cm33_cli_cmd_tasks(int argc, char *argv[]);
static void cm33_cli_cmd_buttons(int argc, char *argv[]);
static void cm33_cli_cmd_sysinfo(int argc, char *argv[]);
static void cm33_cli_cmd_led(int argc, char *argv[]);
static void cm33_cli_cmd_date(int argc, char *argv[]);
static void cm33_cli_cmd_reboot(int argc, char *argv[]);
static void cm33_cli_cmd_mac(int argc, char *argv[]);
static void cm33_cli_cmd_ip(int argc, char *argv[]);
static void cm33_cli_cmd_gateway(int argc, char *argv[]);
static void cm33_cli_cmd_netmask(int argc, char *argv[]);
static void cm33_cli_cmd_ping(int argc, char *argv[]);
static void cm33_cli_cmd_stacks(int argc, char *argv[]);
static void cm33_cli_cmd_imu(int argc, char *argv[]);
static void cm33_cli_cmd_touch(int argc, char *argv[]);

static const cm33_cli_cmd_t s_cmds[] =
{
  { "help",    "List commands and short help",           cm33_cli_cmd_help },
  { "version", "Firmware/CLI version string",            cm33_cli_cmd_version },
  { "clear",   "Clear the terminal screen",              cm33_cli_cmd_clear },
  { "echo",    "Echo arguments to output",               cm33_cli_cmd_echo },
  { "uptime",  "Print time since boot (seconds)",       cm33_cli_cmd_uptime },
  { "heap",    "Print FreeRTOS heap free (bytes)",      cm33_cli_cmd_heap },
  { "time",    "time [now|date|clock|set|sync|ntp]",     cm33_cli_cmd_time },
  { "date",    "Print current date (YYYY-MM-DD)",        cm33_cli_cmd_date },
  { "sysinfo", "System snapshot (uptime, heap, time, tasks)", cm33_cli_cmd_sysinfo },
  { "log",     "log status",                             cm33_cli_cmd_log },
  { "tasks",   "List FreeRTOS tasks",                    cm33_cli_cmd_tasks },
  { "buttons", "buttons status",                         cm33_cli_cmd_buttons },
  { "led",     "led on|off|toggle",                      cm33_cli_cmd_led },
  { "mac",     "Print WiFi MAC address",                 cm33_cli_cmd_mac },
  { "ip",      "Print STA IPv4 address",                  cm33_cli_cmd_ip },
  { "gateway", "Print default gateway IPv4",              cm33_cli_cmd_gateway },
  { "netmask", "Print STA netmask IPv4",                  cm33_cli_cmd_netmask },
  { "ping",    "ping <a.b.c.d> [timeout_ms]",             cm33_cli_cmd_ping },
  { "stacks",  "Task stack high-water marks (bytes free)", cm33_cli_cmd_stacks },
  { "imu",     "imu status|data|stream|sample|fusion|calib|swap",      cm33_cli_cmd_imu },
  { "touch",   "touch status|stream|ipc status",           cm33_cli_cmd_touch },
  { "wifi",    "wifi scan|connect|disconnect|status|list|info", cm33_cli_cmd_wifi },
  { "udp",     "udp start|stop|send <msg>|status",       cm33_cli_cmd_udp },
  { "ipc",     "ipc ping|send|status|recv",              cm33_cli_cmd_ipc },
  { "reset",   "Software reset (like reset button)",     cm33_cli_cmd_reset },
  { "reboot",  "Reboot (same as reset)",                 cm33_cli_cmd_reboot },
};
static const size_t s_cmds_count = sizeof(s_cmds) / sizeof(s_cmds[0]);

static void cm33_cli_cmd_help(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  printf("Commands:\n");
  for (size_t i = 0U; i < s_cmds_count; i++)
  {
    printf("  %s  %s\n", s_cmds[i].cmd, s_cmds[i].help);
  }
}

static void cm33_cli_cmd_version(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  (void)printf("CM33 CLI 1.0 " __DATE__ " " __TIME__ "\n");
}

static void cm33_cli_cmd_clear(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  printf("\033[2J\033[H");
}

static void cm33_cli_cmd_echo(int argc, char *argv[])
{
  for (int i = 1; i < argc; i++)
  {
    if (i > 1)
    {
      printf(" ");
    }
    printf("%s", argv[i]);
  }
  if (argc > 1)
  {
    printf("\n");
  }
}

static void cm33_cli_cmd_uptime(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  {
    TickType_t ticks = xTaskGetTickCount();
    unsigned long sec = (unsigned long)(ticks / (TickType_t)configTICK_RATE_HZ);
    unsigned long tenths = (unsigned long)((ticks % (TickType_t)configTICK_RATE_HZ) * 10UL
                                           / (unsigned long)configTICK_RATE_HZ);
    printf("uptime %lu.%lu s (%lu ticks)\n", sec, tenths, (unsigned long)ticks);
  }
}

static void cm33_cli_cmd_heap(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
#if (configHEAP_ALLOCATION_SCHEME != HEAP_ALLOCATION_TYPE3)
  {
    size_t free_bytes = xPortGetFreeHeapSize();
    printf("heap free %u bytes\n", (unsigned)free_bytes);
#if (configHEAP_ALLOCATION_SCHEME == HEAP_ALLOCATION_TYPE4) || (configHEAP_ALLOCATION_SCHEME == HEAP_ALLOCATION_TYPE5)
    {
      size_t min_free = xPortGetMinimumEverFreeHeapSize();
      printf("heap min ever free %u bytes\n", (unsigned)min_free);
    }
#endif
  }
#else
  (void)printf("heap stats not available (heap_3)\n");
#endif
}

static void cm33_cli_cmd_reset(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  printf("Resetting...\r\n");
  __DSB();
  NVIC_SystemReset();
}

static void cm33_cli_cmd_ipc(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("Usage: ipc ping|send <msg>|status|recv\n");
    return;
  }
  if (strcmp(argv[1], "recv") == 0)
  {
    printf("IPC recv: pending %lu, total %lu\n",
           (unsigned long)cm33_ipc_get_recv_pending(),
           (unsigned long)cm33_ipc_get_recv_total());
    return;
  }
  if (strcmp(argv[1], "ping") == 0)
  {
    if (cm33_ipc_send_ping())
    {
      printf("Ping sent to CM55.\n");
    }
    else
    {
      printf("Ping send failed (queue full?).\n");
    }
    return;
  }
  if (strcmp(argv[1], "send") == 0)
  {
    if (argc < 3)
    {
      printf("Usage: ipc send <msg>\n");
      return;
    }
    if (cm33_ipc_send_cli_message(argv[2]))
    {
      printf("Sent to CM55.\n");
    }
    else
    {
      printf("Send failed (queue full?).\n");
    }
    return;
  }
  if (strcmp(argv[1], "status") == 0)
  {
    printf("IPC pipe: CM33 -> CM55 (running).\n");
    return;
  }
  printf("Unknown ipc subcommand '%s'. Use: ping|send|status|recv\n", argv[1]);
}

static void cm33_cli_cmd_time(int argc, char *argv[])
{
  if (argc < 2)
  {
    date_time_print_current_datetime(DATE_TIME_FORMAT_FULL);
    return;
  }
  if ((strcmp(argv[1], "now") == 0) || (strcmp(argv[1], "full") == 0))
  {
    date_time_print_current_datetime(DATE_TIME_FORMAT_FULL);
    return;
  }
  if (strcmp(argv[1], "date") == 0)
  {
    date_time_print_current_datetime(DATE_TIME_FORMAT_DATE);
    return;
  }
  if (strcmp(argv[1], "clock") == 0)
  {
    date_time_print_current_datetime(DATE_TIME_FORMAT_TIME);
    return;
  }
  if (strcmp(argv[1], "sync") == 0)
  {
    date_time_set_rtc_to_compile_time();
    return;
  }
  if (strcmp(argv[1], "ntp") == 0)
  {
    (void)printf("time ntp: NTP sync not available (requires NTP client). Use 'time sync' for compile-time.\n");
    return;
  }
  if (strcmp(argv[1], "set") == 0)
  {
    if (argc < 8)
    {
      printf("Usage: time set <hour> <min> <sec> <day> <month> <year>\n");
      return;
    }
    {
      int hour = atoi(argv[2]);
      int min = atoi(argv[3]);
      int sec = atoi(argv[4]);
      int day = atoi(argv[5]);
      int month = atoi(argv[6]);
      int year = atoi(argv[7]);
      date_time_set_rtc_time(hour, min, sec, day, month, year);
    }
    return;
  }
  (void)printf("Unknown time subcommand '%s'. Use: now|date|clock|set|sync|ntp\n", argv[1]);
}

static void cm33_cli_cmd_log(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  printf("Log: printf -> IPC to CM55 (ipc_log_transport).\n");
}

#define CM33_CLI_TASKS_MAX (24U)  /* Max tasks to list (uxTaskGetSystemState array size). */
static void cm33_cli_cmd_tasks(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
#if (configUSE_TRACE_FACILITY == 1)
  {
    TaskStatus_t status[CM33_CLI_TASKS_MAX];
    UBaseType_t cnt = uxTaskGetNumberOfTasks();
    if (cnt > CM33_CLI_TASKS_MAX)
    {
      cnt = CM33_CLI_TASKS_MAX;
    }
    if (0U != uxTaskGetSystemState(status, cnt, NULL))
    {
      printf("%-*s Pri State  HWM\n", (int)configMAX_TASK_NAME_LEN, "Task");
      for (UBaseType_t i = 0U; i < cnt; i++)
      {
        const char *state_str = "?";
        switch (status[i].eCurrentState)
        {
          case eRunning:   state_str = "R"; break;
          case eReady:    state_str = "rdy"; break;
          case eBlocked:  state_str = "blk"; break;
          case eSuspended: state_str = "sus"; break;
          case eDeleted:  state_str = "del"; break;
          default: break;
        }
        printf("%-*s %3lu %-4s %5u\n",
               (int)configMAX_TASK_NAME_LEN, status[i].pcTaskName,
               (unsigned long)status[i].uxCurrentPriority,
               state_str,
               (unsigned)status[i].usStackHighWaterMark);
      }
    }
    else
    {
      printf("tasks: uxTaskGetSystemState failed\n");
    }
  }
#else
  (void)printf("tasks: configUSE_TRACE_FACILITY not enabled\n");
#endif
}

static void cm33_cli_cmd_buttons(int argc, char *argv[])
{
  if ((argc >= 2) && (strcmp(argv[1], "status") == 0))
  {
    for (uint32_t i = 0U; i < (uint32_t)BUTTON_ID_MAX; i++)
    {
      button_id_t id = (button_id_t)i;
      uint32_t presses = 0U;
      bool is_pressed = false;
      if (user_buttons_get_state(id, &presses, &is_pressed))
      {
        printf("Button %u: %s, press_count %lu\n",
               (unsigned)id,
               is_pressed ? "pressed" : "released",
               (unsigned long)presses);
      }
      else
      {
        printf("Button %u: (no handle)\n", (unsigned)id);
      }
    }
    return;
  }
  printf("Usage: buttons status\n");
}

static void cm33_cli_cmd_sysinfo(int argc, char *argv[])
{
  char buf[32];
  (void)argc;
  (void)argv;
  {
    TickType_t ticks = xTaskGetTickCount();
    unsigned long sec = (unsigned long)(ticks / (TickType_t)configTICK_RATE_HZ);
    printf("uptime %lu s\n", sec);
  }
#if (configHEAP_ALLOCATION_SCHEME != HEAP_ALLOCATION_TYPE3)
  printf("heap free %u bytes\n", (unsigned)xPortGetFreeHeapSize());
#else
  printf("heap: (heap_3)\n");
#endif
  if (NULL != date_time_get_current_datetime(DATE_TIME_FORMAT_FULL, buf, sizeof(buf)))
  {
    printf("time %s\n", buf);
  }
  printf("tasks %lu\n", (unsigned long)uxTaskGetNumberOfTasks());
}

static void cm33_cli_cmd_led(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("Usage: led on|off|toggle\n");
    return;
  }
  if (strcmp(argv[1], "on") == 0)
  {
    Cy_GPIO_Write(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN, 0U);
    printf("LED on\n");
    return;
  }
  if (strcmp(argv[1], "off") == 0)
  {
    Cy_GPIO_Write(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN, 1U);
    printf("LED off\n");
    return;
  }
  if (strcmp(argv[1], "toggle") == 0)
  {
    Cy_GPIO_Inv(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);
    printf("LED toggled\n");
    return;
  }
  printf("Unknown led subcommand '%s'. Use: on|off|toggle\n", argv[1]);
}

static void cm33_cli_cmd_date(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  date_time_print_current_datetime(DATE_TIME_FORMAT_DATE);
}

static void cm33_cli_cmd_reboot(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  printf("Rebooting...\r\n");
  __DSB();
  NVIC_SystemReset();
}

static void cm33_cli_cmd_mac(int argc, char *argv[])
{
  cy_wcm_mac_t mac;
  cy_rslt_t result;

  (void)argc;
  (void)argv;
  (void)memset(&mac[0], 0, CY_WCM_MAC_ADDR_LEN);
  result = cy_wcm_get_mac_addr(CY_WCM_INTERFACE_TYPE_STA, &mac);
  if (CY_RSLT_SUCCESS == result)
  {
    (void)printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                 (unsigned)mac[0], (unsigned)mac[1], (unsigned)mac[2],
                 (unsigned)mac[3], (unsigned)mac[4], (unsigned)mac[5]);
  }
  else
  {
    (void)printf("MAC: get failed (0x%08lX)\n", (unsigned long)result);
  }
}

static void cm33_cli_cmd_ip(int argc, char *argv[])
{
  cy_wcm_ip_address_t ip_addr;
  cy_rslt_t result;

  (void)argc;
  (void)argv;
  (void)memset(&ip_addr, 0, sizeof(ip_addr));
  result = cy_wcm_get_ip_addr(CY_WCM_INTERFACE_TYPE_STA, &ip_addr);
  if (CY_RSLT_SUCCESS == result)
  {
    uint32_t v4 = ip_addr.ip.v4;
    (void)printf("IP: %u.%u.%u.%u\n",
                 (unsigned)(v4 & 0xFFU),
                 (unsigned)((v4 >> 8) & 0xFFU),
                 (unsigned)((v4 >> 16) & 0xFFU),
                 (unsigned)((v4 >> 24) & 0xFFU));
  }
  else
  {
    (void)printf("IP: get failed (0x%08lX)\n", (unsigned long)result);
  }
}

/* Prints IPv4 in dotted decimal (v4 in host byte order). */
static void cm33_cli_print_ipv4(uint32_t v4)
{
  (void)printf("%u.%u.%u.%u",
               (unsigned)(v4 & 0xFFU),
               (unsigned)((v4 >> 8) & 0xFFU),
               (unsigned)((v4 >> 16) & 0xFFU),
               (unsigned)((v4 >> 24) & 0xFFU));
}

static void cm33_cli_cmd_gateway(int argc, char *argv[])
{
  cy_wcm_ip_address_t addr;
  cy_rslt_t result;

  (void)argc;
  (void)argv;
  (void)memset(&addr, 0, sizeof(addr));
  result = cy_wcm_get_gateway_ip_address(CY_WCM_INTERFACE_TYPE_STA, &addr);
  if (CY_RSLT_SUCCESS == result)
  {
    (void)printf("gateway: ");
    cm33_cli_print_ipv4(addr.ip.v4);
    (void)printf("\n");
  }
  else
  {
    (void)printf("gateway: get failed (0x%08lX)\n", (unsigned long)result);
  }
}

static void cm33_cli_cmd_netmask(int argc, char *argv[])
{
  cy_wcm_ip_address_t addr;
  cy_rslt_t result;

  (void)argc;
  (void)argv;
  (void)memset(&addr, 0, sizeof(addr));
  result = cy_wcm_get_ip_netmask(CY_WCM_INTERFACE_TYPE_STA, &addr);
  if (CY_RSLT_SUCCESS == result)
  {
    (void)printf("netmask: ");
    cm33_cli_print_ipv4(addr.ip.v4);
    (void)printf("\n");
  }
  else
  {
    (void)printf("netmask: get failed (0x%08lX)\n", (unsigned long)result);
  }
}

#define CM33_CLI_PING_TIMEOUT_MS_DEFAULT 2000U  /* Default ping timeout in ms when not specified. */

/**
 * Parses dotted-decimal IPv4 string into host-order uint32_t. Returns false if format invalid
 * or any octet > 255; on success writes to *out_v4 and returns true.
 */
static bool cm33_cli_parse_ipv4(const char *str, uint32_t *out_v4)
{
  unsigned int a = 0U, b = 0U, c = 0U, d = 0U;
  int n = 0;
  if (4 != sscanf(str, "%u.%u.%u.%u%n", &a, &b, &c, &d, &n) || str[n] != '\0')
  {
    return false;
  }
  if (a > 255U || b > 255U || c > 255U || d > 255U)
  {
    return false;
  }
  *out_v4 = (uint32_t)a | ((uint32_t)b << 8) | ((uint32_t)c << 16) | ((uint32_t)d << 24);
  return true;
}

static void cm33_cli_cmd_ping(int argc, char *argv[])
{
  uint32_t ip_v4;
  uint32_t timeout_ms = CM33_CLI_PING_TIMEOUT_MS_DEFAULT;
  cy_wcm_ip_address_t ip_addr;
  uint32_t elapsed_ms = 0U;
  cy_rslt_t result;

  if (argc < 2)
  {
    (void)printf("Usage: ping <a.b.c.d> [timeout_ms]\n");
    return;
  }
  if (!cm33_cli_parse_ipv4(argv[1], &ip_v4))
  {
    (void)printf("ping: invalid IPv4 '%s'\n", argv[1]);
    return;
  }
  if (argc >= 3)
  {
    timeout_ms = (uint32_t)atoi(argv[2]);
    if (timeout_ms == 0U)
    {
      timeout_ms = CM33_CLI_PING_TIMEOUT_MS_DEFAULT;
    }
  }
  (void)memset(&ip_addr, 0, sizeof(ip_addr));
  ip_addr.ip.v4 = ip_v4;
  result = cy_wcm_ping(CY_WCM_INTERFACE_TYPE_STA, &ip_addr, timeout_ms, &elapsed_ms);
  if (CY_RSLT_SUCCESS == result)
  {
    (void)printf("ping ok, %lu ms\n", (unsigned long)elapsed_ms);
  }
  else
  {
    (void)printf("ping failed (0x%08lX)\n", (unsigned long)result);
  }
}

static void cm33_cli_cmd_stacks(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
#if (configUSE_TRACE_FACILITY == 1)
  {
    TaskStatus_t status[CM33_CLI_TASKS_MAX];
    UBaseType_t cnt = uxTaskGetNumberOfTasks();
    if (cnt > CM33_CLI_TASKS_MAX)
    {
      cnt = CM33_CLI_TASKS_MAX;
    }
    if (0U == uxTaskGetSystemState(status, cnt, NULL))
    {
      (void)printf("stacks: get state failed\n");
      return;
    }
    (void)printf("Task                    Stack high-water (bytes free)\n");
    for (UBaseType_t i = 0U; i < cnt; i++)
    {
      unsigned free_bytes = (unsigned)status[i].usStackHighWaterMark * (unsigned)sizeof(StackType_t);
      (void)printf("%-24s %u\n", status[i].pcTaskName, free_bytes);
    }
  }
#else
  (void)printf("stacks: configUSE_TRACE_FACILITY not enabled\n");
#endif
}

static bool cm33_cli_parse_u16_arg(const char *text, uint16_t *out_value)
{
  char *end_ptr = NULL;
  unsigned long value = 0UL;

  if ((NULL == text) || (NULL == out_value))
  {
    return false;
  }

  value = strtoul(text, &end_ptr, 10);
  if ((end_ptr == text) || ('\0' != *end_ptr) || (value > 65535UL))
  {
    return false;
  }

  *out_value = (uint16_t)value;
  return true;
}

static void cm33_cli_cmd_imu(int argc, char *argv[])
{
#ifdef COMPONENT_BSXLITE
  sensor_hub_fusion_status_t status;
  sensor_hub_sample_t sample;
  if ((argc < 2) || (0 == strcmp(argv[1], "help")))
  {
    (void)printf("[CM33.IMU] Usage: imu status|data|stream status|on|off|sample status|rate <hz>|fusion status|mode quat|euler|data|on|off|calib status|reset|swap status|on|off\n");
    return;
  }
  if (0 == strcmp(argv[1], "status"))
  {
    if (sensor_hub_fusion_get_status(&status))
    {
      (void)printf("[CM33.IMU.Status] task=%u imu_ready=%u gt911_ready=%u fusion=%u stream=%u rate_hz=%u sample_hz=%u mode=%s swap_yz=%u loops=%lu\n",
                   (unsigned int)status.task_running,
                   (unsigned int)status.imu_ready,
                   (unsigned int)status.gt911_ready,
                   (unsigned int)status.fusion_enabled,
                   (unsigned int)status.stream_enabled,
                   (unsigned int)status.stream_rate_hz,
                   (unsigned int)status.sample_rate_hz,
                   (SENSOR_HUB_OUTPUT_MODE_EULER == status.output_mode) ? "euler" :
                   (SENSOR_HUB_OUTPUT_MODE_DATA == status.output_mode) ? "data" : "quat",
                   (unsigned int)status.swap_yz,
                   (unsigned long)status.loop_count);
      (void)printf("[CM33.IMU.Status] imu_read ok=%lu fail=%lu\n",
                   (unsigned long)status.imu_read_ok,
                   (unsigned long)status.imu_read_fail);
    }
    else
    {
      (void)printf("[CM33.IMU.Status] unavailable\n");
    }
    return;
  }
  if (0 == strcmp(argv[1], "data"))
  {
    if (sensor_hub_fusion_get_sample(&sample))
    {
      (void)printf("[CM33.IMU.Data] acc=%.4f,%.4f,%.4f gyro=%.4f,%.4f,%.4f quat=%.6f,%.6f,%.6f,%.6f\n",
                   (double)sample.ax, (double)sample.ay, (double)sample.az,
                   (double)sample.gx, (double)sample.gy, (double)sample.gz,
                   (double)sample.qw, (double)sample.qx, (double)sample.qy, (double)sample.qz);
    }
    else
    {
      (void)printf("[CM33.IMU.Data] unavailable\n");
    }
    return;
  }
  if (0 == strcmp(argv[1], "stream"))
  {
    if (argc < 3)
    {
      (void)printf("[CM33.IMU.Stream] Usage: imu stream status|on|off\n");
      return;
    }
    if (0 == strcmp(argv[2], "status"))
    {
      if (3 != argc)
      {
        (void)printf("[CM33.IMU.Stream] Usage: imu stream status\n");
        return;
      }
      if (sensor_hub_fusion_get_status(&status))
      {
        (void)printf("[CM33.IMU.Stream] %s (%u Hz, same as sample rate)\n",
                     status.stream_enabled ? "on" : "off",
                     (unsigned int)status.stream_rate_hz);
      }
      else
      {
        (void)printf("[CM33.IMU.Stream] status unavailable\n");
      }
      return;
    }
    if (0 == strcmp(argv[2], "on"))
    {
      if (3 != argc)
      {
        (void)printf("[CM33.IMU.Stream] Usage: imu stream on\n");
        return;
      }
      sensor_hub_fusion_set_stream(true);
      if (sensor_hub_fusion_get_status(&status))
      {
        (void)printf("[CM33.IMU.Stream] on (%u Hz)\n", (unsigned int)status.sample_rate_hz);
      }
      else
      {
        (void)printf("[CM33.IMU.Stream] on\n");
      }
      return;
    }
    if (0 == strcmp(argv[2], "off"))
    {
      if (3 != argc)
      {
        (void)printf("[CM33.IMU.Stream] Usage: imu stream off\n");
        return;
      }
      sensor_hub_fusion_set_stream(false);
      (void)printf("[CM33.IMU.Stream] off\n");
      return;
    }
    (void)printf("[CM33.IMU.Stream] Usage: imu stream status|on|off\n");
    return;
  }
  if (0 == strcmp(argv[1], "swap"))
  {
    if (argc < 3)
    {
      (void)printf("[CM33.IMU.Swap] Usage: imu swap status|on|off\n");
      return;
    }
    if (0 == strcmp(argv[2], "status"))
    {
      if (3 != argc)
      {
        (void)printf("[CM33.IMU.Swap] Usage: imu swap status\n");
        return;
      }
      if (sensor_hub_fusion_get_status(&status))
      {
        (void)printf("[CM33.IMU.Swap] yz=%s\n", status.swap_yz ? "on" : "off");
      }
      else
      {
        (void)printf("[CM33.IMU.Swap] status unavailable\n");
      }
      return;
    }
    if (0 == strcmp(argv[2], "on"))
    {
      if (3 != argc)
      {
        (void)printf("[CM33.IMU.Swap] Usage: imu swap on\n");
        return;
      }
      sensor_hub_fusion_set_swap_yz(true);
      (void)printf("[CM33.IMU.Swap] on\n");
      return;
    }
    if (0 == strcmp(argv[2], "off"))
    {
      if (3 != argc)
      {
        (void)printf("[CM33.IMU.Swap] Usage: imu swap off\n");
        return;
      }
      sensor_hub_fusion_set_swap_yz(false);
      (void)printf("[CM33.IMU.Swap] off\n");
      return;
    }
    (void)printf("[CM33.IMU.Swap] Usage: imu swap status|on|off\n");
    return;
  }
  if (0 == strcmp(argv[1], "fusion"))
  {
    if (argc < 3)
    {
      (void)printf("[CM33.IMU.Fusion] Usage: imu fusion status|mode quat|euler|data|on|off\n");
      return;
    }
    if (0 == strcmp(argv[2], "status"))
    {
      if (3 != argc)
      {
        (void)printf("[CM33.IMU.Fusion] Usage: imu fusion status\n");
        return;
      }
      if (sensor_hub_fusion_get_status(&status))
      {
        (void)printf("[CM33.IMU.Fusion] fusion=%s mode=%s\n",
                     status.fusion_enabled ? "on" : "off",
                     (SENSOR_HUB_OUTPUT_MODE_EULER == status.output_mode) ? "euler" :
                     (SENSOR_HUB_OUTPUT_MODE_DATA == status.output_mode) ? "data" : "quat");
      }
      else
      {
        (void)printf("[CM33.IMU.Fusion] status unavailable\n");
      }
      return;
    }
    if (0 == strcmp(argv[2], "mode"))
    {
      if (4 != argc)
      {
        (void)printf("[CM33.IMU.Fusion] Usage: imu fusion mode quat|euler|data\n");
        return;
      }
      if (0 == strcmp(argv[3], "quat"))
      {
        sensor_hub_fusion_set_output_mode(SENSOR_HUB_OUTPUT_MODE_QUATERNION);
        (void)printf("[CM33.IMU.Fusion] mode quat\n");
        return;
      }
      if (0 == strcmp(argv[3], "euler"))
      {
        sensor_hub_fusion_set_output_mode(SENSOR_HUB_OUTPUT_MODE_EULER);
        (void)printf("[CM33.IMU.Fusion] mode euler\n");
        return;
      }
      if (0 == strcmp(argv[3], "data"))
      {
        sensor_hub_fusion_set_output_mode(SENSOR_HUB_OUTPUT_MODE_DATA);
        (void)printf("[CM33.IMU.Fusion] mode data\n");
        return;
      }
      (void)printf("[CM33.IMU.Fusion] Usage: imu fusion mode quat|euler|data\n");
      return;
    }
    if (0 == strcmp(argv[2], "on"))
    {
      if (3 != argc)
      {
        (void)printf("[CM33.IMU.Fusion] Usage: imu fusion on\n");
        return;
      }
      sensor_hub_fusion_set_fusion_enabled(true);
      (void)printf("[CM33.IMU.Fusion] on\n");
      return;
    }
    if (0 == strcmp(argv[2], "off"))
    {
      if (3 != argc)
      {
        (void)printf("[CM33.IMU.Fusion] Usage: imu fusion off\n");
        return;
      }
      sensor_hub_fusion_set_fusion_enabled(false);
      (void)printf("[CM33.IMU.Fusion] off\n");
      return;
    }
    (void)printf("[CM33.IMU.Fusion] Usage: imu fusion status|mode quat|euler|data|on|off\n");
    return;
  }
  if (0 == strcmp(argv[1], "sample"))
  {
    if (0 == strcmp(argv[2], "status"))
    {
      if (3 != argc)
      {
        (void)printf("[CM33.IMU.Sample] Usage: imu sample status\n");
        return;
      }
      if (sensor_hub_fusion_get_status(&status))
      {
        (void)printf("[CM33.IMU.Sample] rate=%u Hz\n", (unsigned int)status.sample_rate_hz);
      }
      else
      {
        (void)printf("[CM33.IMU.Sample] status unavailable\n");
      }
      return;
    }
    if ((4 != argc) || (0 != strcmp(argv[2], "rate")))
    {
      (void)printf("[CM33.IMU.Sample] Usage: imu sample status|rate <hz>\n");
      return;
    }
    {
      uint16_t rate_hz = 0U;
      if (false == cm33_cli_parse_u16_arg(argv[3], &rate_hz))
      {
        (void)printf("[CM33.IMU.Sample] Usage: imu sample rate <hz>\n");
        return;
      }
      sensor_hub_fusion_set_sample_rate(rate_hz);
      (void)printf("[CM33.IMU.Sample] rate set to %u Hz\n", (unsigned int)rate_hz);
      return;
    }
  }
  if (0 == strcmp(argv[1], "calib"))
  {
    if (argc < 3)
    {
      (void)printf("[CM33.IMU.Calib] Usage: imu calib status|reset\n");
      return;
    }
    if (0 == strcmp(argv[2], "status"))
    {
      if (3 != argc)
      {
        (void)printf("[CM33.IMU.Calib] Usage: imu calib status\n");
        return;
      }
      uint8_t acc = 0U;
      uint8_t gyr = 0U;
      uint8_t mag = 0U;
      bool ok = sensor_hub_fusion_calib_status(&acc, &gyr, &mag);
      bool supported = false;
      if (sensor_hub_fusion_get_status(&status))
      {
        supported = status.calib_supported;
      }
      (void)printf("[CM33.IMU.Calib] status: ready=%u supported=%u acc=%u gyr=%u mag=unsupported\n",
                   (unsigned int)ok, (unsigned int)supported, (unsigned int)acc, (unsigned int)gyr);
      return;
    }
    if (0 == strcmp(argv[2], "reset"))
    {
      if (3 != argc)
      {
        (void)printf("[CM33.IMU.Calib] Usage: imu calib reset\n");
        return;
      }
      bool ok = sensor_hub_fusion_calib_reset();
      (void)printf("[CM33.IMU.Calib] reset: %s\n", ok ? "ok" : "not implemented");
      return;
    }
    (void)printf("[CM33.IMU.Calib] Usage: imu calib status|reset\n");
    return;
  }
  (void)printf("[CM33.IMU] Usage: imu status|data|stream status|on|off|sample status|rate <hz>|fusion status|mode quat|euler|data|on|off|calib status|reset|swap status|on|off\n");
#else
  (void)argc;
  (void)argv;
  (void)printf("[CM33.IMU] unavailable (COMPONENT_BSXLITE not enabled)\n");
#endif
}

static void cm33_cli_cmd_touch(int argc, char *argv[])
{
#ifdef COMPONENT_BSXLITE
  sensor_hub_fusion_status_t status;
  if ((argc < 2) || (0 == strcmp(argv[1], "help")))
  {
    (void)printf("[CM33.Touch] Usage: touch status|stream status|on|off|ipc status\n");
    return;
  }
  if (0 == strcmp(argv[1], "status"))
  {
    if (sensor_hub_fusion_get_status(&status))
    {
      (void)printf("[CM33.Touch.Status] ready=%u x=%d y=%d pressed=%u stream=%u\n",
                   (unsigned int)status.gt911_ready,
                   (int)status.touch_x,
                   (int)status.touch_y,
                   (unsigned int)status.touch_pressed,
                   (unsigned int)status.touch_stream_enabled);
      (void)printf("[CM33.Touch.Status] read_ok=%lu read_fail=%lu send_ok=%lu send_fail=%lu\n",
                   (unsigned long)status.touch_read_ok,
                   (unsigned long)status.touch_read_fail,
                   (unsigned long)status.touch_send_ok,
                   (unsigned long)status.touch_send_fail);
    }
    else
    {
      (void)printf("[CM33.Touch.Status] unavailable\n");
    }
    return;
  }
  if (0 == strcmp(argv[1], "stream"))
  {
    if (argc < 3)
    {
      (void)printf("[CM33.Touch.Stream] Usage: touch stream status|on|off\n");
      return;
    }
    if (0 == strcmp(argv[2], "status"))
    {
      if (3 != argc)
      {
        (void)printf("[CM33.Touch.Stream] Usage: touch stream status\n");
        return;
      }
      if (sensor_hub_fusion_get_status(&status))
      {
        (void)printf("[CM33.Touch.Stream] %s\n", status.touch_stream_enabled ? "on" : "off");
      }
      else
      {
        (void)printf("[CM33.Touch.Stream] status unavailable\n");
      }
      return;
    }
    if (0 == strcmp(argv[2], "on"))
    {
      if (3 != argc)
      {
        (void)printf("[CM33.Touch.Stream] Usage: touch stream on\n");
        return;
      }
      sensor_hub_fusion_set_touch_stream(true);
      (void)printf("[CM33.Touch.Stream] on\n");
      return;
    }
    if (0 == strcmp(argv[2], "off"))
    {
      if (3 != argc)
      {
        (void)printf("[CM33.Touch.Stream] Usage: touch stream off\n");
        return;
      }
      sensor_hub_fusion_set_touch_stream(false);
      (void)printf("[CM33.Touch.Stream] off\n");
      return;
    }
    (void)printf("[CM33.Touch.Stream] Usage: touch stream status|on|off\n");
    return;
  }
  if (0 == strcmp(argv[1], "ipc"))
  {
    if ((3 != argc) || (0 != strcmp(argv[2], "status")))
    {
      (void)printf("[CM33.Touch.IPC] Usage: touch ipc status\n");
      return;
    }
    if (sensor_hub_fusion_get_status(&status))
    {
      (void)printf("[CM33.Touch.IPC] send_ok=%lu send_fail=%lu\n",
                   (unsigned long)status.touch_send_ok,
                   (unsigned long)status.touch_send_fail);
    }
    (void)printf("[CM33.Touch.IPC] queue used=%lu/%lu\n",
                 (unsigned long)cm33_ipc_get_send_queue_used(),
                 (unsigned long)cm33_ipc_get_send_queue_capacity());
    return;
  }
  (void)printf("[CM33.Touch] Usage: touch status|stream status|on|off|ipc status\n");
#else
  (void)argc;
  (void)argv;
  (void)printf("[CM33.Touch] unavailable (COMPONENT_BSXLITE not enabled)\n");
#endif
}

static void cm33_cli_cmd_wifi(int argc, char *argv[])
{
  if (argc < 2)
  {
    (void)printf("Usage: wifi scan|connect <ssid> [pass]|disconnect|status|list|info\n");
    return;
  }
  if (strcmp(argv[1], "info") == 0)
  {
    cy_wcm_associated_ap_info_t ap_info;
    (void)memset(&ap_info, 0, sizeof(ap_info));
    if (CY_RSLT_SUCCESS == cy_wcm_get_associated_ap_info(&ap_info))
    {
      (void)printf("SSID: %.32s\n", ap_info.SSID);
      (void)printf("BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n",
                   (unsigned)ap_info.BSSID[0], (unsigned)ap_info.BSSID[1],
                   (unsigned)ap_info.BSSID[2], (unsigned)ap_info.BSSID[3],
                   (unsigned)ap_info.BSSID[4], (unsigned)ap_info.BSSID[5]);
      (void)printf("RSSI: %d dBm  channel: %u  ch_width: %u  security: 0x%X\n",
                   (int)ap_info.signal_strength, (unsigned)ap_info.channel,
                   (unsigned)ap_info.channel_width, (unsigned)ap_info.security);
    }
    else
    {
      (void)printf("wifi info: not connected or get failed\n");
    }
    return;
  }
  if (strcmp(argv[1], "list") == 0)
  {
    wifi_info_t list[32];
    uint32_t count = 0U;
    if (wifi_manager_get_last_scan(list, 32U, &count) && (count > 0U))
    {
      printf("Last scan (%lu APs):\n", (unsigned long)count);
      for (uint32_t i = 0U; i < count; i++)
      {
        printf("  %d  %s  RSSI %d  ch %u\n",
               (int)i, list[i].ssid, (int)list[i].rssi, (unsigned)list[i].channel);
      }
    }
    else
    {
      printf("No scan results. Run 'wifi scan' first.\n");
    }
    return;
  }
  if (strcmp(argv[1], "scan") == 0)
  {
    ipc_wifi_scan_request_t req = { 0 };
    req.use_filter = false;
    if (wifi_manager_request_scan(&req))
    {
      printf("Scan requested.\n");
    }
    else
    {
      printf("Scan request failed (e.g. connected).\n");
    }
    return;
  }
  if (strcmp(argv[1], "connect") == 0)
  {
    if (argc < 3)
    {
      printf("Usage: wifi connect <ssid> [pass]\n");
      return;
    }
    ipc_wifi_connect_request_t req = { 0 };
    (void)strncpy(req.ssid, argv[2], (size_t)(sizeof(req.ssid) - 1U));
    req.ssid[sizeof(req.ssid) - 1U] = '\0';
    if (argc >= 4)
    {
      (void)strncpy(req.password, argv[3], (size_t)(sizeof(req.password) - 1U));
      req.password[sizeof(req.password) - 1U] = '\0';
    }
    if (wifi_manager_request_connect(&req))
    {
      printf("Connect requested.\n");
    }
    else
    {
      printf("Connect request failed.\n");
    }
    return;
  }
  if (strcmp(argv[1], "disconnect") == 0)
  {
    if (wifi_manager_request_disconnect())
    {
      printf("Disconnect requested.\n");
    }
    else
    {
      printf("Disconnect request failed.\n");
    }
    return;
  }
  if (strcmp(argv[1], "status") == 0)
  {
    if (wifi_manager_request_status())
    {
      printf("Status requested.\n");
    }
    else
    {
      printf("Status request failed.\n");
    }
    return;
  }
  (void)printf("Unknown wifi subcommand '%s'. Use: scan|connect|disconnect|status|list|info\n", argv[1]);
}

static void cm33_cli_cmd_udp(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("Usage: udp start|stop|send <msg>|status\n");
    return;
  }
  if (strcmp(argv[1], "start") == 0)
  {
    udp_server_app_start();
    printf("UDP server start requested.\n");
    return;
  }
  if (strcmp(argv[1], "stop") == 0)
  {
    udp_server_app_stop();
    printf("UDP server stopped.\n");
    return;
  }
  if (strcmp(argv[1], "send") == 0)
  {
    if (argc < 3)
    {
      printf("Usage: udp send <msg>\n");
      return;
    }
    {
      udp_server_app_status_t st = { 0 };
      if (!udp_server_app_get_status(&st) || st.peer_count == 0U)
      {
        printf("No peers. Run the UDP client (e.g. python udp_client.py --hostname <board-IP>) so it sends at least one packet; then try again.\n");
        return;
      }
      {
        const char *msg = argv[2];
        udp_server_app_send((const uint8_t *)msg, strlen(msg));
        printf("Sent.\n");
      }
    }
    return;
  }
  if (strcmp(argv[1], "status") == 0)
  {
    udp_server_app_status_t st = { 0 };
    if (udp_server_app_get_status(&st))
    {
      printf("port %u, peers %u, local IP %u.%u.%u.%u\n",
             (unsigned int)st.port,
             (unsigned int)st.peer_count,
             (unsigned int)(st.local_ip_v4 & 0xFFU),
             (unsigned int)((st.local_ip_v4 >> 8) & 0xFFU),
             (unsigned int)((st.local_ip_v4 >> 16) & 0xFFU),
             (unsigned int)((st.local_ip_v4 >> 24) & 0xFFU));
    }
    else
    {
      printf("UDP server not initialized.\n");
    }
    return;
  }
  printf("Unknown udp subcommand '%s'. Use: start|stop|send|status\n", argv[1]);
}

static void cm33_cli_print_unknown(const char *cmd)
{
  printf("Unknown command '%s'. Type 'help'.\n", cmd);
}

/**
 * Splits line into tokens by replacing spaces/tabs with NUL and filling argv with pointers.
 * Returns number of tokens. Modifies line in place; argv[] entries point into line.
 */
static unsigned int cm33_cli_tokenize(char *line, char *argv[], unsigned int max_argc)
{
  unsigned int argc = 0U;
  char *p = line;
  while (argc < max_argc)
  {
    /* Skip leading spaces and null them. */
    while (*p == ' ' || *p == '\t')
    {
      *p = '\0';
      p++;
    }
    if (*p == '\0')
    {
      break;
    }
    argv[argc] = p;
    argc++;
    /* Advance to next space or end of line. */
    while (*p != '\0' && *p != ' ' && *p != '\t')
    {
      p++;
    }
  }
  return argc;
}

/* Looks up argv[0] in the command table and runs the handler; otherwise prints unknown. */
static void cm33_cli_dispatch(int argc, char *argv[])
{
  if (argc == 0U)
  {
    return;
  }
  for (size_t i = 0U; i < s_cmds_count; i++)
  {
    if (strcmp(argv[0], s_cmds[i].cmd) == 0)
    {
      s_cmds[i].fn(argc, argv);
      return;
    }
  }
  cm33_cli_print_unknown(argv[0]);
}

/* Redraws the current line (e.g. after history change) and erases any trailing characters. */
static void cm33_cli_redraw_line(const char *line_buf, unsigned int len, unsigned int old_len)
{
  printf("\r" CM33_CLI_PROMPT "%s", line_buf);
  while (old_len > len)
  {
    printf(" \b");
    old_len--;
  }
}

static char s_history_buf[CM33_CLI_HISTORY_COUNT][CM33_CLI_LINE_MAX];  /* Circular history. */
static unsigned int s_history_count = 0U;   /* Number of entries currently stored. */
static unsigned int s_history_write = 0U;   /* Next slot to write (oldest is overwritten). */

/* Appends a completed line to history; overwrites oldest when full. */
static void cm33_cli_history_push(const char *line)
{
  (void)strncpy(s_history_buf[s_history_write], line, CM33_CLI_LINE_MAX - 1U);
  s_history_buf[s_history_write][CM33_CLI_LINE_MAX - 1U] = '\0';
  s_history_write = (s_history_write + 1U) % CM33_CLI_HISTORY_COUNT;
  if (s_history_count < CM33_CLI_HISTORY_COUNT)
  {
    s_history_count++;
  }
}

/* Copies history entry at index (0 = newest) into out; sets *out_len. No-op if index invalid. */
static void cm33_cli_history_get(int index, char *out, unsigned int out_size, unsigned int *out_len)
{
  if (index < 0 || (unsigned int)index >= s_history_count || out == NULL || out_size == 0U)
  {
    if (out_len != NULL)
    {
      *out_len = 0U;
    }
    return;
  }
  {
    unsigned int read_pos = (s_history_write + CM33_CLI_HISTORY_COUNT - s_history_count + (unsigned int)index) % CM33_CLI_HISTORY_COUNT;
    size_t n = strlen(s_history_buf[read_pos]);
    if (n >= out_size)
    {
      n = out_size - 1U;
    }
    (void)memcpy(out, s_history_buf[read_pos], n);
    out[n] = '\0';
    if (out_len != NULL)
    {
      *out_len = (unsigned int)n;
    }
  }
}

/* Returns the number of lines currently in history (0 to CM33_CLI_HISTORY_COUNT). */
static unsigned int cm33_cli_history_count(void)
{
  return s_history_count;
}

/**
 * CLI task: reads chars from debug UART, handles ESC sequences (Up/Down for history), backspace,
 * Enter to dispatch. On Enter tokenizes line_buf, pushes to history, dispatches, prints prompt.
 */
static void cm33_cli_task(void *pvParameters)
{
  (void)pvParameters;
  static char line_buf[CM33_CLI_LINE_MAX];      /* Current line being edited. */
  static char current_edit[CM33_CLI_LINE_MAX];  /* Snapshot when Up first pressed (restore on Down). */
  static char *argv_buf[CM33_CLI_MAX_ARGC];
  static unsigned int len = 0U;
  static int history_show = -1;                 /* -1 = not in history browse; else index into history. */
  static cm33_cli_esc_state_t esc_state = CM33_CLI_ESC_NONE;

  printf("\n" CM33_CLI_PROMPT);
  for (;;)
  {
    int c = retarget_io_getchar();
    if (c < 0)
    {
      continue;
    }
    /* Handle ESC [ A (Up) / ESC [ B (Down) for history. */
    if (esc_state != CM33_CLI_ESC_NONE)
    {
      if (esc_state == CM33_CLI_ESC_ESC)
      {
        esc_state = (c == '[') ? CM33_CLI_ESC_BRACKET : CM33_CLI_ESC_NONE;
        continue;
      }
      if (esc_state == CM33_CLI_ESC_BRACKET)
      {
        esc_state = CM33_CLI_ESC_NONE;
        if (c == 'A')
        {
          unsigned int n = cm33_cli_history_count();
          if (n > 0U)
          {
            unsigned int old_len = len;
            if (history_show < 0)
            {
              (void)memcpy(current_edit, line_buf, (size_t)len);
              current_edit[len] = '\0';
              history_show = (int)n - 1;
            }
            else if (history_show > 0)
            {
              history_show--;
            }
            cm33_cli_history_get(history_show, line_buf, CM33_CLI_LINE_MAX, &len);
            cm33_cli_redraw_line(line_buf, len, old_len);
          }
        }
        else if (c == 'B')
        {
          unsigned int n = cm33_cli_history_count();
          if (history_show >= 0)
          {
            unsigned int old_len = len;
            history_show++;
            if ((unsigned int)history_show >= n)
            {
              history_show = -1;
              (void)memcpy(line_buf, current_edit, sizeof(current_edit));
              len = (unsigned int)strlen(line_buf);
            }
            else
            {
              cm33_cli_history_get(history_show, line_buf, CM33_CLI_LINE_MAX, &len);
            }
            cm33_cli_redraw_line(line_buf, len, old_len);
          }
        }
        continue;
      }
    }
    if (c == 0x1B)
    {
      esc_state = CM33_CLI_ESC_ESC;
      continue;
    }
    /* Enter: tokenize, push history, dispatch, reset line and prompt. */
    if (c == '\r' || c == '\n')
    {
      printf("\r\n");
      esc_state = CM33_CLI_ESC_NONE;
      line_buf[len] = '\0';
      if (len > 0U)
      {
        cm33_cli_history_push(line_buf);
        unsigned int argc = cm33_cli_tokenize(line_buf, argv_buf, CM33_CLI_MAX_ARGC);
        cm33_cli_dispatch(argc, argv_buf);
      }
      history_show = -1;
      len = 0U;
      printf(CM33_CLI_PROMPT);
      continue;
    }
    /* Backspace / DEL: remove last character. */
    if (c == '\b' || c == 0x7F)
    {
      if (len > 0U)
      {
        len--;
        printf("\b \b");
      }
      continue;
    }
    if (len >= (CM33_CLI_LINE_MAX - 1U))
    {
      continue;
    }
    /* Printable: append to line and echo. */
    if (c >= 0x20 && c <= 0x7E)
    {
      line_buf[len++] = (char)c;
      printf("%c", c);
    }
  }
}

static TaskHandle_t s_cli_task_handle;  /* Non-NULL after start until stop. */

bool cm33_cli_init(void)
{
  return true;
}

bool cm33_cli_start(void)
{
  /* Create the CLI task at idle + offset priority. */
  BaseType_t ok = xTaskCreate(cm33_cli_task,
                              "cli",
                              CM33_CLI_STACK_SIZE,
                              NULL,
                              tskIDLE_PRIORITY + CM33_CLI_TASK_PRIORITY_OFFSET,
                              &s_cli_task_handle);
  return (ok == pdPASS);
}

void cm33_cli_stop(void)
{
  /* Delete the CLI task and clear the handle so start can be called again. */
  if (s_cli_task_handle != NULL)
  {
    vTaskDelete(s_cli_task_handle);
    s_cli_task_handle = NULL;
  }
}
