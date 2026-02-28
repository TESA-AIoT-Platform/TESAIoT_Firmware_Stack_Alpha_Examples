#include "wifi_radio.h"

#include "cy_sd_host.h"
#include "cy_sysint.h"
#include "cy_wcm.h"
#include "cy_wcm_error.h"
#include "cybsp.h"
#include "cycfg_peripherals.h"
#include "mtb_hal_gpio.h"
#include "mtb_hal_sdio.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdbool.h>

#define APP_SDIO_INTERRUPT_PRIORITY   (7U)
#define APP_HOST_WAKE_INTERRUPT_PRIORITY (2U)
#define APP_SDIO_FREQUENCY_HZ         (25000000U)
#define SDHC_SDIO_64BYTES_BLOCK       (64U)

#if (CY_CFG_PWR_SYS_IDLE_MODE == CY_CFG_PWR_MODE_DEEPSLEEP)
#include "cy_syspm.h"
#endif

static mtb_hal_sdio_t sdio_instance;
static cy_stc_sd_host_context_t sdhc_host_context;
static cy_wcm_config_t wcm_config;

static volatile bool s_sdio_inited = false;
static volatile bool s_wcm_inited = false;
static SemaphoreHandle_t s_init_mutex = NULL;

#if (CY_CFG_PWR_SYS_IDLE_MODE == CY_CFG_PWR_MODE_DEEPSLEEP)
static cy_stc_syspm_callback_params_t sdhc_ds_params = {
    .context = &sdhc_host_context,
    .base = CYBSP_WIFI_SDIO_HW,
};
static cy_stc_syspm_callback_t sdhc_deep_sleep_cb = {
    .callback = Cy_SD_Host_DeepSleepCallback,
    .skipMode = 0U,
    .type = CY_SYSPM_DEEPSLEEP,
    .callbackParams = &sdhc_ds_params,
    .prevItm = NULL,
    .nextItm = NULL,
    .order = 1U,
};
#endif

static void sdio_interrupt_handler(void)
{
  mtb_hal_sdio_process_interrupt(&sdio_instance);
}

static void host_wake_interrupt_handler(void)
{
  mtb_hal_gpio_process_interrupt(&wcm_config.wifi_host_wake_pin);
}

static cy_rslt_t app_sdio_init(void)
{
  cy_rslt_t result;
  mtb_hal_sdio_cfg_t sdio_hal_cfg;

  cy_stc_sysint_t sdio_intr_cfg = {.intrSrc = CYBSP_WIFI_SDIO_IRQ,
                                   .intrPriority = APP_SDIO_INTERRUPT_PRIORITY};
  cy_stc_sysint_t host_wake_intr_cfg = {.intrSrc = CYBSP_WIFI_HOST_WAKE_IRQ,
                                        .intrPriority = APP_HOST_WAKE_INTERRUPT_PRIORITY};

  cy_en_sysint_status_t int_status = Cy_SysInt_Init(&sdio_intr_cfg, sdio_interrupt_handler);
  if (CY_SYSINT_SUCCESS != int_status)
  {
    return CY_RSLT_TYPE_ERROR;
  }

  NVIC_EnableIRQ(CYBSP_WIFI_SDIO_IRQ);

  result = mtb_hal_sdio_setup(&sdio_instance, &CYBSP_WIFI_SDIO_sdio_hal_config, NULL, &sdhc_host_context);
  if (CY_RSLT_SUCCESS != result)
  {
    return result;
  }

  Cy_SD_Host_Enable(CYBSP_WIFI_SDIO_HW);
  Cy_SD_Host_Init(CYBSP_WIFI_SDIO_HW, CYBSP_WIFI_SDIO_sdio_hal_config.host_config, &sdhc_host_context);
  Cy_SD_Host_SetHostBusWidth(CYBSP_WIFI_SDIO_HW, CY_SD_HOST_BUS_WIDTH_4_BIT);

  sdio_hal_cfg.frequencyhal_hz = APP_SDIO_FREQUENCY_HZ;
  sdio_hal_cfg.block_size = SDHC_SDIO_64BYTES_BLOCK;
  mtb_hal_sdio_configure(&sdio_instance, &sdio_hal_cfg);

#if (CY_CFG_PWR_SYS_IDLE_MODE == CY_CFG_PWR_MODE_DEEPSLEEP)
  Cy_SysPm_RegisterCallback(&sdhc_deep_sleep_cb);
#endif

  mtb_hal_gpio_setup(&wcm_config.wifi_wl_pin, CYBSP_WIFI_WL_REG_ON_PORT_NUM, CYBSP_WIFI_WL_REG_ON_PIN);
  mtb_hal_gpio_setup(&wcm_config.wifi_host_wake_pin, CYBSP_WIFI_HOST_WAKE_PORT_NUM, CYBSP_WIFI_HOST_WAKE_PIN);

  int_status = Cy_SysInt_Init(&host_wake_intr_cfg, host_wake_interrupt_handler);
  if (CY_SYSINT_SUCCESS != int_status)
  {
    return CY_RSLT_TYPE_ERROR;
  }

  NVIC_EnableIRQ(CYBSP_WIFI_HOST_WAKE_IRQ);

  return CY_RSLT_SUCCESS;
}

cy_rslt_t wifi_radio_init(void)
{
  cy_rslt_t result = CY_RSLT_SUCCESS;

  if (NULL == s_init_mutex)
  {
    s_init_mutex = xSemaphoreCreateMutex();
    if (NULL == s_init_mutex)
    {
      return CY_RSLT_TYPE_ERROR;
    }
  }

  if (pdTRUE != xSemaphoreTake(s_init_mutex, portMAX_DELAY))
  {
    return CY_RSLT_TYPE_ERROR;
  }

  if (!s_sdio_inited)
  {
    result = app_sdio_init();
    if (CY_RSLT_SUCCESS != result)
    {
      (void)xSemaphoreGive(s_init_mutex);
      return result;
    }
    s_sdio_inited = true;
  }

  if (!s_wcm_inited)
  {
    wcm_config.interface = CY_WCM_INTERFACE_TYPE_STA;
    wcm_config.wifi_interface_instance = &sdio_instance;
    result = cy_wcm_init(&wcm_config);
    if (CY_RSLT_SUCCESS != result)
    {
      (void)xSemaphoreGive(s_init_mutex);
      return result;
    }
    s_wcm_inited = true;
  }

  (void)xSemaphoreGive(s_init_mutex);
  return result;
}

cy_wcm_config_t *wifi_radio_get_wcm_config(void)
{
  return &wcm_config;
}
