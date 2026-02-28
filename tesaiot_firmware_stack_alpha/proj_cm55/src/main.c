#include "retarget_io_init.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "FreeRTOS.h"
#include "cy_time.h"
#include "cyabs_rtos.h"
#include "cyabs_rtos_impl.h"
#include "task.h"

#include "display_controller.h"
#include "rtos_stats.h"
#include "tesa/utils/tesa_datetime.h"
#include "tesa_datetime.h"
#include "test_lvgl.h"
#include "ui/tabview/tabview.h"

#include "tesa/event_bus/examples.h"

#include "cm55_fatal_error.h"
#include "cm55_ipc_app.h"
#include "cm55_system.h"
#include "ipc_communication.h"

#define SSID "TERNION"
#define PASSWORD "111122134"

#define STARTUP_WIFI_DELAY_MS (3000U)
#define STARTUP_WIFI_TASK_STACK (256U)

static void display_tick_cb(const system_tick_hook_params_t *params)
{
  (void)params;
  display_controller_tick();
}

static void startup_wifi_task(void *arg)
{
  (void)arg;
  vTaskDelay(pdMS_TO_TICKS(STARTUP_WIFI_DELAY_MS));
  cm55_trigger_connect(SSID, PASSWORD, 0U);
  cm55_trigger_status_request();
  vTaskDelete(NULL);
}

static bool tesaiot_wifi_init(void)
{
  return (xTaskCreate(startup_wifi_task, "StartupWiFi", STARTUP_WIFI_TASK_STACK, NULL,
                      tskIDLE_PRIORITY + 1U, NULL) == pdPASS);
}

int main(void)
{
  if (!cm55_system_init())
  {
    cm55_handle_fatal_error(NULL);
  }

  cm55_system_register_tick_callback(display_tick_cb, NULL);

  /* Setup IPC communication for CM55 */
  if (!cm55_ipc_app_init())
  {
    cm55_handle_fatal_error(NULL);
  }

  /* Delay for 100ms */
  Cy_SysLib_Delay(100);

  // printf("Initialize the display\r\n\n");

  // /* Initialize the display */
  // if (pdPASS != display_controller_init())
  // {
  //   cm55_handle_fatal_error(NULL);
  // }

  // printf("********************************************************\n"
  //        "CM55: LVGL Display, WiFi & IPC PIPE Data Exchange\n"
  //        "********************************************************\n");

  if (!tesaiot_wifi_init())
  {
    cm55_handle_fatal_error(NULL);
  }

  /* Start the scheduler */
  vTaskStartScheduler();
  // cm55_handle_fatal_error(NULL);
}

/* [] END OF FILE */
