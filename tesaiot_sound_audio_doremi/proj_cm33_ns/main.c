

#include "app_i2s.h"
#include "retarget_io_init.h"

#include "source/app_i2s/beep_generator.h"
#include "source/app_i2s/beep_i2s.h"

/* Button interrupt priority */
#define USER_BTN_1_ISR_PRIORITY (5u)

/* The timeout value in microsecond used to wait for core to be booted */
#define CM55_BOOT_WAIT_TIME_USEC (10U)

/* App boot address for CM55 project */
#define CM55_APP_BOOT_ADDR                                                     \
  (CYMEM_CM33_0_m55_nvm_START + CYBSP_MCUBOOT_HEADER_SIZE)

/* Main */
int main(void) {
  cy_rslt_t result;

  /* Initialize the device and board peripherals */
  result = cybsp_init();
  /* Board init failed. Stop program execution */
  handle_app_error(result);

  __enable_irq();

  /* I2S initialization */
  app_i2s_init();

  beep_i2s_install_isr();

  /* TLV codec initiailization */
  app_tlv_codec_init();

  /* Enable CM55. */
  /* CY_CM55_APP_BOOT_ADDR must be updated if CM55 memory layout is changed.*/
  Cy_SysEnableCM55(MXCM55, CM55_APP_BOOT_ADDR, CM55_BOOT_WAIT_TIME_USEC);

  /* */
  app_i2s_enable();
  app_i2s_activate();

  /* Set key transposition (default: KEY_C, change to transpose) */
  beep_set_key(KEY_B); /* Try: KEY_G, KEY_D, KEY_F, etc. */

  /* Auto-start playback with Song 4 (Jingle Bells) */
  beep_set_song(4);
  beep_scale_start();
  Cy_GPIO_Write(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN, CYBSP_LED_STATE_ON);

  /* Run song service loop */
  beep_song_service_loop();
}
/* [] END OF FILE */
