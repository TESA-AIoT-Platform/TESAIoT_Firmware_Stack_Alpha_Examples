#include "app_events.h"
#include "cm33_cli.h"
#include "cm33_ipc_pipe.h"
#include "cm33_system.h"
#include "cy_pdl.h"
#include "cy_time.h"
#include "cyabs_rtos.h"
#include "cyabs_rtos_impl.h"
#include "cybsp.h"
#include "date_time.h"
#include "error_handler.h"
#include "examples.h"

#include "ipc_communication.h"
#include "ipc_log.h"
#include "ipc_log_transport.h"
#include "retarget_io_init.h"
#include "sensor_hub_fusion.h"
#include "udp_server_app.h"
#include "user_buttons.h"
#include "wifi_manager.h"
#include <FreeRTOS.h>
#include <stdio.h>
#include <task.h>

#define CM55_ENABLE_DELAY_MS (2000U)

static void cm55_starter_task(void *pv)
{
  (void)pv;
  vTaskDelay(pdMS_TO_TICKS(CM55_ENABLE_DELAY_MS));
  cm33_system_enable_cm55();
  vTaskDelete(NULL);
}

int main()
{
  uint32_t reset_reason = Cy_SysLib_GetResetReason();

  if (!cm33_system_init())
  {
    handle_error("CM33 system init failed");
  }

  date_time_init(cm33_system_get_rtc());

  if (!ipc_log_transport_init())
  {
    handle_error("IPC log transport init failed");
  }

  (void)printf("[CM33] reset reason: 0x%08lX\n", (unsigned long)reset_reason);

  printf("********************************************************\n"
         "CM33: Wi-Fi Manager, UDP Server & IPC PIPE Data Exchange\n"
         "********************************************************\n");

  if (!udp_server_app_init())
  {
    handle_error("UDP server init failed");
  }

  if (!wifi_manager_init())
  {
    handle_error("WiFi manager init failed");
  }

  if (!wifi_manager_start())
  {
    handle_error("WiFi manager start failed");
  }

  if (!user_buttons_init())
  {
    handle_error("User buttons init failed");
  }

  if (!cm33_ipc_pipe_start())
  {
    handle_error("CM33 IPC pipe start failed");
  }

  {
    cy_rslt_t r = create_sensor_hub_fusion_task();
    if (CY_RSLT_SUCCESS != r)
    {
      (void)printf("[CM33] BSXLITE fusion task create failed 0x%08lX (check heap)\n",
                   (unsigned long)r);
      handle_error("BSXLITE: fusion task create failed");
    }
    (void)printf("[CM33] BSXLITE fusion task created\n");
    fflush(stdout);
  }

  if (!cm33_cli_init())
  {
    handle_error("CM33 CLI init failed");
  }

  if (!cm33_cli_start())
  {
    handle_error("CM33 CLI start failed");
  }

  if (pdPASS != xTaskCreate(cm55_starter_task, "CM55Start", 256U, NULL,
                            tskIDLE_PRIORITY, NULL))
  {
    handle_error("CM55 starter task create failed");
  }

  (void)Cy_IPC_Sema_Set(IPC0_SEMA_CH_NUM, IPC_SEMA_INDEX_DEBUG_UART);

  vTaskStartScheduler();
  handle_error("vTaskStartScheduler returned - check FreeRTOS heap/config");
}
