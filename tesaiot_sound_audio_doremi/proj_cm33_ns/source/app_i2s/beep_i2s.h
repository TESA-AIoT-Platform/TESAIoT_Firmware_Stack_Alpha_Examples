/******************************************************************************
 * File Name : beep_i2s.h
 *
 * Description : Header for overriding I2S TX ISR to feed beep samples
 ********************************************************************************
 */
#ifndef __BEEP_I2S_H__
#define __BEEP_I2S_H__

#include "app_i2s.h"
#include "beep_generator.h"
#include "cy_pdl.h"


#ifdef __cplusplus
extern "C" {
#endif

/* Install the beep ISR for I2S TX to override default handler */
void beep_i2s_install_isr(void);

#ifdef __cplusplus
}
#endif

#endif /* __BEEP_I2S_H__ */
