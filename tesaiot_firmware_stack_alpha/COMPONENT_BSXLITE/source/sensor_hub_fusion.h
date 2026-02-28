/******************************************************************************
 * File Name:   sensor_hub_fusion.h
 *
 * Description: This file is the public interface of sensor_hub_fusion.c This
 *              file contains the fusion FreeRTOS task configuration parameters.
 *
 * Related Document: See README.md
 *
 *
 *******************************************************************************
 * (c) 2025, Infineon Technologies AG, or an affiliate of Infineon
 * Technologies AG. All rights reserved.
 * This software, associated documentation and materials ("Software") is
 * owned by Infineon Technologies AG or one of its affiliates ("Infineon")
 * and is protected by and subject to worldwide patent protection, worldwide
 * copyright laws, and international treaty provisions. Therefore, you may use
 * this Software only as provided in the license agreement accompanying the
 * software package from which you obtained this Software. If no license
 * agreement applies, then any use, reproduction, modification, translation, or
 * compilation of this Software is prohibited without the express written
 * permission of Infineon.
 *
 * Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
 * IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
 * THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
 * SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
 * Infineon reserves the right to make changes to the Software without notice.
 * You are responsible for properly designing, programming, and testing the
 * functionality and safety of your intended application of the Software, as
 * well as complying with any legal requirements related to its use. Infineon
 * does not guarantee that the Software will be free from intrusion, data theft
 * or loss, or other breaches ("Security Breaches"), and Infineon shall have
 * no liability arising out of any Security Breaches. Unless otherwise
 * explicitly approved by Infineon, the Software may not be used in any
 * application where a failure of the Product or any consequences of the use
 * thereof can reasonably be expected to result in personal injury.
 *******************************************************************************/

#ifndef _SENSOR_HUB_FUSION_H_
#define _SENSOR_HUB_FUSION_H_

#if defined(__cplusplus)
extern "C"
{
#endif

#include "FreeRTOS.h"
#include "cy_result.h"
#include "task.h"
#include <stdbool.h>
#include <stdint.h>

/******************************************************************************
 * Macros
 *****************************************************************************/
#define TASK_SENSOR_HUB_FUSION_PRIORITY (configMAX_PRIORITIES - 1)
#define TASK_SENSOR_HUB_FUSION_STACK_SIZE (8192U)
#define TASK_SENSOR_HUB_FUSION_RATE_MS (10U)

  typedef enum
  {
    SENSOR_HUB_OUTPUT_MODE_QUATERNION = 1,
    SENSOR_HUB_OUTPUT_MODE_EULER = 2,
    SENSOR_HUB_OUTPUT_MODE_DATA = 3
  } sensor_hub_output_mode_t;

  typedef struct
  {
    bool task_running;
    bool imu_ready;
    bool gt911_ready;
    bool fusion_enabled;
    bool stream_enabled;
    bool touch_stream_enabled;
    uint16_t stream_rate_hz;
    uint16_t sample_rate_hz;
    sensor_hub_output_mode_t output_mode;
    uint32_t loop_count;
    uint32_t imu_read_ok;
    uint32_t imu_read_fail;
    uint32_t touch_read_ok;
    uint32_t touch_read_fail;
    uint32_t touch_send_ok;
    uint32_t touch_send_fail;
    uint8_t calib_acc;
    uint8_t calib_gyr;
    bool calib_supported;
    int16_t touch_x;
    int16_t touch_y;
    uint8_t touch_pressed;
    bool swap_yz;
  } sensor_hub_fusion_status_t;

  typedef struct
  {
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
    float qw;
    float qx;
    float qy;
    float qz;
  } sensor_hub_sample_t;

  /*******************************************************************************
   * Function prototypes
   *******************************************************************************/
  cy_rslt_t create_sensor_hub_fusion_task(void);
  bool sensor_hub_fusion_get_status(sensor_hub_fusion_status_t *out_status);
  bool sensor_hub_fusion_get_sample(sensor_hub_sample_t *out_sample);
  void sensor_hub_fusion_set_stream(bool enable);
  void sensor_hub_fusion_set_swap_yz(bool enable);
  void sensor_hub_fusion_set_sample_rate(uint16_t rate_hz);
  void sensor_hub_fusion_set_touch_stream(bool enable);
  void sensor_hub_fusion_set_fusion_enabled(bool enable);
  void sensor_hub_fusion_set_output_mode(sensor_hub_output_mode_t mode);
  bool sensor_hub_fusion_calib_status(uint8_t *acc, uint8_t *gyr, uint8_t *mag);
  bool sensor_hub_fusion_calib_reset(void);

#if defined(__cplusplus)
}
#endif

#endif /* _SENSOR_HUB_FUSION_H_ */

/* [] END OF FILE */
