/******************************************************************************
 * File Name : beep_i2s.c
 *
 * Description : Override I2S TX ISR to send beep generator samples
 ********************************************************************************
 */
#include "beep_i2s.h"

/* Local zero buffer to prefill if beep inactive */
// static uint16_t zeros_buf[HW_FIFO_HALF_SIZE / 2] = {0};

/* Custom ISR feeding I2S TX from beep generator */
static void i2s_tx_interrupt_handler_beep(void) {
  uint32_t intr_status = Cy_AudioTDM_GetTxInterruptStatusMasked(TDM_STRUCT0_TX);

  if (intr_status & CY_TDM_INTR_TX_FIFO_TRIGGER) {
    for (int i = 0; i < HW_FIFO_HALF_SIZE / 2; i++) {
      int16_t sample = 0;
      if (beep_generator_is_active()) {
        sample = beep_generator_get_next_sample();
      } else {
        sample = 0;
      }
      /* Stereo: write same sample to Left and Right */
      Cy_AudioTDM_WriteTxData(TDM_STRUCT0_TX, (uint32_t)sample);
      Cy_AudioTDM_WriteTxData(TDM_STRUCT0_TX, (uint32_t)sample);
    }
    /* If beep finished, flag playback ended to stop I2S in main loop */
    if (beep_generator_is_complete()) {
      /* Signal end-of-playback */
      audio_playback_ended = true;
    }
  } else if (intr_status & CY_TDM_INTR_TX_FIFO_UNDERFLOW) {
    CY_ASSERT(0);
  }

  /* Clear all Tx I2S Interrupt */
  Cy_AudioTDM_ClearTxInterrupt(TDM_STRUCT0_TX, CY_TDM_INTR_TX_MASK);
}

void beep_i2s_install_isr(void) {
  /* Configure system interrupt to point to our beep ISR */
  static const cy_stc_sysint_t tx_isr_cfg = {
      .intrSrc = (IRQn_Type)tdm_0_interrupts_tx_0_IRQn,
      .intrPriority = I2S_ISR_PRIORITY,
  };
  Cy_SysInt_Init(&tx_isr_cfg, i2s_tx_interrupt_handler_beep);
  /* Enable IRQ using compile-time constant directly to satisfy Clang */
  NVIC_EnableIRQ((IRQn_Type)tdm_0_interrupts_tx_0_IRQn);
}
