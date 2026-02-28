/******************************************************************************
 * File Name:   sensor_hub_data_fusion.c
 *
 * Description: This file contains the task that initializes and configures the
 *              bmi270 Motion Sensor and displays the sensor orientation.
 *
 * Related Document: See README.md
 *
 *
 *******************************************************************************
 * (c) 2023-2025, Infineon Technologies AG, or an affiliate of Infineon
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
 ******************************************************************************/

/*******************************************************************************
 * Header Files
 *******************************************************************************/
#include "sensor_hub_fusion.h"
#include "cybsp.h"
#include "mtb_hal.h"
#include "mtb_bmi270.h"
#include "retarget_io_init.h"
#include "semphr.h"
#include <math.h>

#include "bsxlite_interface.h"
#include "cm33_ipc_pipe.h"
#include "cycfg_pins.h"
#include "error_handler.h"
#include "ipc_log.h"
#if defined(MTB_CTP_GT911)
#include "mtb_ctp_gt911.h"
#endif

/*******************************************************************************
 * Macros
 *******************************************************************************/
#define GRAVITY_EARTH (9.80665f)
#define DEG_TO_RAD (0.01745f)
#define GYR_RANGE_DPS (2000.0f)
#define ACC_RANGE_2G (2.0f)
#define BSXLITE_INVALID_INSTANCE (0U)
#define USE_TOUCH (0)
#define I2C_SCAN_TIMEOUT_US (50U)

/* Custom app module ID to avoid collisions */
#define APP_RSLT_MODULE_ID (1U)

/* App error for task creation failure */
#define APP_RSLT_FUSION_TASK_CREATE_FAILED CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, \
                                                          APP_RSLT_MODULE_ID, \
                                                          2U)

/*******************************************************************************
 * Global Variables
 *******************************************************************************/
static mtb_hal_i2c_t CYBSP_I2C_CONTROLLER_hal_obj;
static cy_stc_scb_i2c_context_t CYBSP_I2C_CONTROLLER_context;
static mtb_hal_i2c_t CYBSP_I2C_CAM_CONTROLLER_hal_obj;
static cy_stc_scb_i2c_context_t CYBSP_I2C_CAM_CONTROLLER_context;

static cy_en_scb_i2c_status_t initStatus;
static volatile sensor_hub_fusion_status_t s_fusion_status = {0};
static volatile sensor_hub_sample_t s_fusion_sample = {0};
static volatile bool s_stream_enabled = true;
static volatile bool s_touch_stream_enabled = false;
static volatile bool s_fusion_enabled = true;
static volatile uint16_t s_sample_rate_hz = 1U;
static volatile sensor_hub_output_mode_t s_output_mode = SENSOR_HUB_OUTPUT_MODE_QUATERNION;
static volatile uint8_t s_calib_acc = 0U;
static volatile uint8_t s_calib_gyr = 0U;
static volatile bool s_calib_supported = true;
static volatile bool s_calib_reset_requested = false;
static volatile bool s_swap_yz = false;
static volatile bool s_bsxlite_ready = false;
static volatile bool s_i2c_ready = false;
static bsxlite_instance_t s_bsxlite_instance = BSXLITE_INVALID_INSTANCE;

/*******************************************************************************
 * Function Name: lsb_to_mps2
 *******************************************************************************
 * Summary:
 * This function converts the raw sensor value to meter/sec^2.
 *
 * Parameters:
 *  val       raw value
 *  g_range   The accelerometer range is expressed in multiples of g.
 *  bit_width data width in bits
 *
 * Return:
 *  float
 *
 *******************************************************************************/
static float lsb_to_mps2(int16_t val, int8_t g_range, uint8_t bit_width)
{
  float half_scale = (float)(1u << (bit_width - 1u));

  return ((GRAVITY_EARTH)*val * g_range) / half_scale;
}

/*******************************************************************************
 * Function Name: lsb_to_rps
 ********************************************************************************
 * Summary:
 * This function converts the raw gyroscope sensor value to rad/sec.
 *
 * Parameters:
 *  val       raw value
 *  dps       The gyroscope sensor range in degrees per second
 *  bit_width data width in bits
 *
 * Return:
 *  float
 *
 *******************************************************************************/
static float lsb_to_rps(int16_t val, float dps, uint8_t bit_width)
{
  float half_scale = (float)(1u << (bit_width - 1u));

  return ((DEG_TO_RAD) * ((dps) / (half_scale)) * (val));
}

static void i2c_scan_bus(CySCB_Type *base, cy_stc_scb_i2c_context_t *context, const char *name)
{
  uint32_t found = 0U;
  printf("[BSXLITE] I2C scan %s start\n", name);
  for (uint32_t addr = 0x08U; addr <= 0x77U; addr++)
  {
    cy_en_scb_i2c_status_t st = Cy_SCB_I2C_MasterSendStart(base,
                                                            addr,
                                                            CY_SCB_I2C_WRITE_XFER,
                                                            I2C_SCAN_TIMEOUT_US,
                                                            context);
    if (CY_SCB_I2C_SUCCESS == st)
    {
      (void)Cy_SCB_I2C_MasterSendStop(base, I2C_SCAN_TIMEOUT_US, context);
      printf("[BSXLITE] I2C %s found 0x%02lX\n", name, (unsigned long)addr);
      found++;
    }
  }
  printf("[BSXLITE] I2C scan %s done found=%lu\n", name, (unsigned long)found);
}

/*******************************************************************************
 * Function Name: sensor_hub_fusion_task
 *******************************************************************************
 * Summary:
 * This function initializes the I2C and BMI270 sensor, reads the sensor values,
 * and uses BSXLite library function to fuse 6 -axis sensor data and print the
 * Quaternion vectors and/or Euler angles over serial terminal.
 *
 * Parameters:
 *  void *
 *
 * Return:
 *  void
 *
 *******************************************************************************/
static void sensor_hub_fusion_task(void *pvParameters)
{
  TickType_t xLastWakeTime = 0;
  TickType_t xCurrWakeTime = 0;
  TickType_t xElapsedTime = 0;
  uint16_t sample_divider = 0U;

  /* BMI270 driver handle*/
  mtb_bmi270_t bmi270;
  /* BMI270 sensor data */
  mtb_bmi270_data_t bmi270_data;

  /* BSX Library related variables*/
  bsxlite_out_t bsxlite_fusion_out;
  bsxlite_return_t result;
  cy_rslt_t i2c_imu_result;
  vector_3d_t acc_in;
  vector_3d_t gyr_in;
#if defined(MTB_CTP_GT911) && USE_TOUCH
  cy_rslt_t gt911_result = CY_RSLT_SUCCESS;
  int16_t touch_x_last = 0;
  int16_t touch_y_last = 0;
  uint8_t touch_pressed_last = 0U;
  uint8_t touch_first_publish = 1U;
#endif

  (void)pvParameters;

  (void)printf("[CM33.IMU] fusion task entered\n");
  fflush(stdout);
  (void)printf("[CM33.IMU] fusion task started (priority %u)\n",
               (unsigned)TASK_SENSOR_HUB_FUSION_PRIORITY);
  fflush(stdout);
  s_fusion_status.task_running = true;
  memset(&acc_in, 0, sizeof(acc_in));
  memset(&gyr_in, 0, sizeof(gyr_in));
  memset(&bsxlite_fusion_out, 0, sizeof(bsxlite_fusion_out));
  s_fusion_status.stream_enabled = s_stream_enabled;
  s_fusion_status.touch_stream_enabled = s_touch_stream_enabled;
  s_fusion_status.fusion_enabled = s_fusion_enabled;
  s_fusion_status.output_mode = s_output_mode;
  s_fusion_status.stream_rate_hz = s_sample_rate_hz;
  s_fusion_status.sample_rate_hz = s_sample_rate_hz;
  s_fusion_status.calib_acc = s_calib_acc;
  s_fusion_status.calib_gyr = s_calib_gyr;
  s_fusion_status.calib_supported = s_calib_supported;
  s_fusion_status.swap_yz = s_swap_yz;

  initStatus = Cy_SCB_I2C_Init(CYBSP_I2C_CONTROLLER_HW,
                               &CYBSP_I2C_CONTROLLER_config,
                               &CYBSP_I2C_CONTROLLER_context);

  if (CY_SCB_I2C_SUCCESS != initStatus)
  {
    printf("[BSXLITE] I2C init failed (0x%08lX), IMU/touch disabled\n",
           (unsigned long)initStatus);
    s_i2c_ready = false;
  }
  else
  {
    Cy_SCB_I2C_Enable(CYBSP_I2C_CONTROLLER_HW);
    Cy_SysLib_Delay(15U);
    i2c_imu_result = mtb_hal_i2c_setup(&CYBSP_I2C_CONTROLLER_hal_obj,
                                       &CYBSP_I2C_CONTROLLER_hal_config,
                                       &CYBSP_I2C_CONTROLLER_context,
                                       NULL);
    if (CY_RSLT_SUCCESS != i2c_imu_result)
    {
      printf("[BSXLITE] HAL I2C setup failed (0x%08lX), IMU/touch disabled\n",
             (unsigned long)i2c_imu_result);
      s_i2c_ready = false;
    }
    else
    {
      s_i2c_ready = true;
      i2c_scan_bus(CYBSP_I2C_CONTROLLER_HW, &CYBSP_I2C_CONTROLLER_context, "SCB0");
    }
  }

  if (false == s_i2c_ready)
  {
    s_fusion_status.imu_ready = false;
    s_fusion_status.gt911_ready = false;
  }
  else
  {
  #define BMI270_INIT_RETRIES (3U)
  #define BMI270_DELAY_MS (20U)
  static const uint8_t bmi270_addresses[] = { MTB_BMI270_ADDRESS_DEFAULT, 0x69U };
  for (uint32_t addr_idx = 0U; addr_idx < (sizeof(bmi270_addresses) / sizeof(bmi270_addresses[0])); addr_idx++)
  {
    uint8_t addr = bmi270_addresses[addr_idx];
    for (uint32_t attempt = 0U; attempt < BMI270_INIT_RETRIES; attempt++)
    {
      if (attempt > 0U)
      {
        Cy_SysLib_Delay(BMI270_DELAY_MS);
      }
      i2c_imu_result = mtb_bmi270_init_i2c(&bmi270,
                                           &CYBSP_I2C_CONTROLLER_hal_obj,
                                           addr);
      if (CY_RSLT_SUCCESS == i2c_imu_result)
      {
        break;
      }
    }
    if (CY_RSLT_SUCCESS == i2c_imu_result)
    {
      break;
    }
    if (addr_idx == 0U)
    {
      Cy_SysLib_Delay(BMI270_DELAY_MS);
    }
  }
  if (CY_RSLT_SUCCESS != i2c_imu_result)
  {
    cy_rslt_t i2c_cam_result;
    cy_en_scb_i2c_status_t i2c_cam_init_status;

    printf("[BSXLITE] BMI270 init failed on SCB0 (0x%08lX), trying SCB5\n",
           (unsigned long)i2c_imu_result);

    i2c_cam_init_status = Cy_SCB_I2C_Init(CYBSP_I2C_CAM_CONTROLLER_HW,
                                          &CYBSP_I2C_CAM_CONTROLLER_config,
                                          &CYBSP_I2C_CAM_CONTROLLER_context);
    if (CY_SCB_I2C_SUCCESS == i2c_cam_init_status)
    {
      Cy_SCB_I2C_Enable(CYBSP_I2C_CAM_CONTROLLER_HW);
      Cy_SysLib_Delay(15U);
      i2c_cam_result = mtb_hal_i2c_setup(&CYBSP_I2C_CAM_CONTROLLER_hal_obj,
                                         &CYBSP_I2C_CAM_CONTROLLER_hal_config,
                                         &CYBSP_I2C_CAM_CONTROLLER_context,
                                         NULL);
      if (CY_RSLT_SUCCESS == i2c_cam_result)
      {
        i2c_scan_bus(CYBSP_I2C_CAM_CONTROLLER_HW, &CYBSP_I2C_CAM_CONTROLLER_context, "SCB5");
        for (uint32_t addr_idx = 0U; addr_idx < (sizeof(bmi270_addresses) / sizeof(bmi270_addresses[0])); addr_idx++)
        {
          uint8_t addr = bmi270_addresses[addr_idx];
          for (uint32_t attempt = 0U; attempt < BMI270_INIT_RETRIES; attempt++)
          {
            if (attempt > 0U)
            {
              Cy_SysLib_Delay(BMI270_DELAY_MS);
            }
            i2c_imu_result = mtb_bmi270_init_i2c(&bmi270,
                                                 &CYBSP_I2C_CAM_CONTROLLER_hal_obj,
                                                 addr);
            if (CY_RSLT_SUCCESS == i2c_imu_result)
            {
              break;
            }
          }
          if (CY_RSLT_SUCCESS == i2c_imu_result)
          {
            break;
          }
          if (addr_idx == 0U)
          {
            Cy_SysLib_Delay(BMI270_DELAY_MS);
          }
        }
      }
      else
      {
        i2c_imu_result = i2c_cam_result;
      }
    }
    else
    {
      i2c_imu_result = (cy_rslt_t)i2c_cam_init_status;
    }

    if (CY_RSLT_SUCCESS != i2c_imu_result)
    {
      printf("[BSXLITE] BMI270 init failed (0x%08lX), IMU disabled\n",
             (unsigned long)i2c_imu_result);
      s_fusion_status.imu_ready = false;
      s_i2c_ready = false;
    }
    else
    {
      printf("[BSXLITE] BMI270 initialized on SCB5\n");
      i2c_imu_result = mtb_bmi270_config_default(&bmi270);
      if (CY_RSLT_SUCCESS != i2c_imu_result)
      {
        printf("[BSXLITE] BMI270 config failed (0x%08lX), IMU disabled\n",
               (unsigned long)i2c_imu_result);
        s_fusion_status.imu_ready = false;
      }
      else
      {
        s_fusion_status.imu_ready = true;
        s_i2c_ready = true;
        result = bsxlite_init(&s_bsxlite_instance);
        CY_ASSERT(BSXLITE_OK == result);
        result = bsxlite_set_to_default(&s_bsxlite_instance);
        CY_ASSERT(BSXLITE_OK == result);
        s_bsxlite_ready = true;
      }
    }
  }
  else
  {
    i2c_imu_result = mtb_bmi270_config_default(&bmi270);
    if (CY_RSLT_SUCCESS != i2c_imu_result)
    {
      printf("[BSXLITE] BMI270 config failed (0x%08lX), IMU disabled\n",
             (unsigned long)i2c_imu_result);
      s_fusion_status.imu_ready = false;
    }
    else
    {
      s_fusion_status.imu_ready = true;
      result = bsxlite_init(&s_bsxlite_instance);
      CY_ASSERT(BSXLITE_OK == result);
      result = bsxlite_set_to_default(&s_bsxlite_instance);
      CY_ASSERT(BSXLITE_OK == result);
      s_bsxlite_ready = true;
    }
  }

  // #if defined(MTB_CTP_GT911)
  //   gt911_result = mtb_gt911_init(CYBSP_I2C_CONTROLLER_HW, &CYBSP_I2C_CONTROLLER_context);
  //   if (CY_RSLT_SUCCESS != gt911_result)
  //   {
  //     handle_error("BSXLITE: GT911 init failed");
  //   }
  //   s_fusion_status.gt911_ready = true;
  // #endif

#if defined(MTB_CTP_GT911) && USE_TOUCH
  if (s_fusion_status.imu_ready)
  {
    gt911_result = mtb_gt911_init(CYBSP_I2C_CONTROLLER_HW, &CYBSP_I2C_CONTROLLER_context);
    if (CY_RSLT_SUCCESS != gt911_result)
    {
      printf("[BSXLITE] GT911 init failed (0x%08lX), touch disabled\n",
             (unsigned long)gt911_result);
      s_fusion_status.gt911_ready = false;
    }
    else
    {
      s_fusion_status.gt911_ready = true;
    }
  }
  else
  {
    s_fusion_status.gt911_ready = false;
  }
#endif
#if !USE_TOUCH
  s_fusion_status.gt911_ready = false;
#endif
  }

  if (s_fusion_status.imu_ready)
  {
    printf("[CM33.IMU] IMU OK (BMI270 + BSXLite)\n");
  }
  else
  {
    printf("[CM33.IMU] IMU disabled (I2C or BMI270 init failed)\n");
  }

  printf("[CM33.IMU] ************************************************************\n");
  printf("[CM33.IMU] PSOC Edge MCU: Sensor hub IMU\n");
  printf("[CM33.IMU] ************************************************************\r\n");
  printf("[CM33.IMU] Code example configured in data fusion mode\r\n\n");
  printf("[CM33.IMU] init summary: i2c_ready=%u imu_ready=%u\n",
         (unsigned int)s_i2c_ready,
         (unsigned int)s_fusion_status.imu_ready);

  /* Initialize the xCurrWakeTime and xLastWakeTime with the current ticks.*/
  xCurrWakeTime = xTaskGetTickCount();
  xLastWakeTime = xCurrWakeTime;

  for (;;)
  {
    bool do_calib_reset = false;

    if (true == s_calib_reset_requested)
    {
      taskENTER_CRITICAL();
      if (true == s_calib_reset_requested)
      {
        s_calib_reset_requested = false;
        do_calib_reset = true;
      }
      taskEXIT_CRITICAL();
    }

    if (true == do_calib_reset && true == s_bsxlite_ready)
    {
      result = bsxlite_set_to_default(&s_bsxlite_instance);
      if (BSXLITE_OK != result)
      {
        printf("[CM33.IMU.Calib] calib reset failed (%d)\n", (int)result);
      }
      else
      {
        s_calib_acc = 0U;
        s_calib_gyr = 0U;
      }
    }

    Cy_GPIO_Write(CYBSP_USER_LED1_PORT,
                  CYBSP_USER_LED1_PIN,
                  CYBSP_LED_STATE_ON);

    if (true == s_fusion_status.imu_ready)
    {
      uint16_t loop_hz = (uint16_t)(1000U / TASK_SENSOR_HUB_FUSION_RATE_MS);
      uint16_t desired_sample_hz = s_sample_rate_hz;
      uint16_t sample_every = 1U;
      bool do_sample = false;
      if (0U == desired_sample_hz)
      {
        desired_sample_hz = 1U;
      }
      if (desired_sample_hz < loop_hz)
      {
        sample_every = (uint16_t)(loop_hz / desired_sample_hz);
        if (0U == sample_every)
        {
          sample_every = 1U;
        }
      }
      sample_divider++;
      if (sample_divider >= sample_every)
      {
        sample_divider = 0U;
        do_sample = true;
      }

      /* Read IMU and run fusion every tick so BSXLite timestamp delta stays in range. */
      i2c_imu_result = mtb_bmi270_read(&bmi270, &bmi270_data);

      if (CY_RSLT_SUCCESS != i2c_imu_result)
      {
        s_fusion_status.imu_read_fail++;
        if (s_fusion_status.imu_read_fail <= 5U)
        {
          printf("[CM33.IMU.Warn] BMI270 read failed (0x%08lX)\n",
                 (unsigned long)i2c_imu_result);
        }
        continue;
      }
      s_fusion_status.imu_read_ok++;
      s_fusion_status.loop_count++;

      acc_in.x = lsb_to_mps2(bmi270_data.sensor_data.acc.x,
                             ACC_RANGE_2G, bmi270.sensor.resolution);
      acc_in.y = lsb_to_mps2(bmi270_data.sensor_data.acc.y,
                             ACC_RANGE_2G, bmi270.sensor.resolution);
      acc_in.z = lsb_to_mps2(bmi270_data.sensor_data.acc.z,
                             ACC_RANGE_2G, bmi270.sensor.resolution);

      gyr_in.x = lsb_to_rps(bmi270_data.sensor_data.gyr.x,
                            GYR_RANGE_DPS, bmi270.sensor.resolution);
      gyr_in.y = lsb_to_rps(bmi270_data.sensor_data.gyr.y,
                            GYR_RANGE_DPS, bmi270.sensor.resolution);
      gyr_in.z = lsb_to_rps(bmi270_data.sensor_data.gyr.z,
                            GYR_RANGE_DPS, bmi270.sensor.resolution);

      if (s_swap_yz)
      {
        float t_acc = acc_in.y;
        acc_in.y = acc_in.z;
        acc_in.z = t_acc;
        t_acc = gyr_in.y;
        gyr_in.y = gyr_in.z;
        gyr_in.z = t_acc;
      }

      if (true == s_fusion_enabled)
      {
        result = bsxlite_do_step(&s_bsxlite_instance, xElapsedTime, &acc_in, &gyr_in,
                                 &bsxlite_fusion_out);
        if (result < BSXLITE_OK)
        {
          printf("[CM33.IMU.Error] do_step failed (%d)\n", (int)result);
        }
        if (result >= BSXLITE_OK)
        {
          s_calib_acc = bsxlite_fusion_out.accel_calibration_status;
          s_calib_gyr = bsxlite_fusion_out.gyro_calibration_status;
        }
      }

      if (true == do_sample)
      {
        s_fusion_sample.ax = acc_in.x;
        s_fusion_sample.ay = acc_in.y;
        s_fusion_sample.az = acc_in.z;
        s_fusion_sample.gx = gyr_in.x;
        s_fusion_sample.gy = gyr_in.y;
        s_fusion_sample.gz = gyr_in.z;
        s_fusion_sample.qw = bsxlite_fusion_out.rotation_vector.w;
        s_fusion_sample.qx = bsxlite_fusion_out.rotation_vector.x;
        s_fusion_sample.qy = bsxlite_fusion_out.rotation_vector.y;
        s_fusion_sample.qz = bsxlite_fusion_out.rotation_vector.z;
        if (true == s_stream_enabled)
        {
          if (true == s_fusion_enabled)
          {
            if (SENSOR_HUB_OUTPUT_MODE_QUATERNION == s_output_mode)
            {
              printf("[CM33.IMU.Quaternion] %f, %f, %f, %f\r\n",
                     bsxlite_fusion_out.rotation_vector.w,
                     bsxlite_fusion_out.rotation_vector.x,
                     bsxlite_fusion_out.rotation_vector.y,
                     bsxlite_fusion_out.rotation_vector.z);
            }
            else if (SENSOR_HUB_OUTPUT_MODE_EULER == s_output_mode)
            {
              printf("[CM33.IMU.Euler] %f, %f, %f, %f\r\n",
                     bsxlite_fusion_out.orientation.heading,
                     bsxlite_fusion_out.orientation.pitch,
                     bsxlite_fusion_out.orientation.roll,
                     bsxlite_fusion_out.orientation.yaw);
            }
            else if (SENSOR_HUB_OUTPUT_MODE_DATA == s_output_mode)
            {
              printf("[CM33.IMU.Data] acc=%.4f,%.4f,%.4f gyro=%.4f,%.4f,%.4f quat=%.6f,%.6f,%.6f,%.6f\r\n",
                     (double)s_fusion_sample.ax, (double)s_fusion_sample.ay, (double)s_fusion_sample.az,
                     (double)s_fusion_sample.gx, (double)s_fusion_sample.gy, (double)s_fusion_sample.gz,
                     (double)s_fusion_sample.qw, (double)s_fusion_sample.qx,
                     (double)s_fusion_sample.qy, (double)s_fusion_sample.qz);
            }
          }
        }
      }
    }

    s_fusion_status.fusion_enabled = s_fusion_enabled;
    s_fusion_status.stream_enabled = s_stream_enabled;
    s_fusion_status.touch_stream_enabled = s_touch_stream_enabled;
    s_fusion_status.output_mode = s_output_mode;
    s_fusion_status.stream_rate_hz = s_sample_rate_hz;
    s_fusion_status.sample_rate_hz = s_sample_rate_hz;
    s_fusion_status.calib_acc = s_calib_acc;
    s_fusion_status.calib_gyr = s_calib_gyr;
    s_fusion_status.calib_supported = s_calib_supported;
    s_fusion_status.swap_yz = s_swap_yz;
    /* Turn user led off.*/
    Cy_GPIO_Write(CYBSP_USER_LED1_PORT,
                  CYBSP_USER_LED1_PIN,
                  CYBSP_LED_STATE_OFF);

#if defined(MTB_CTP_GT911) && USE_TOUCH
    if (s_i2c_ready)
    {
      int touch_x = 0;
      int touch_y = 0;
      int16_t touch_x_to_send = touch_x_last;
      int16_t touch_y_to_send = touch_y_last;
      uint8_t touch_pressed_to_send = 0U;
      cy_rslt_t touch_result = mtb_gt911_get_single_touch(CYBSP_I2C_CONTROLLER_HW,
                                                          &CYBSP_I2C_CONTROLLER_context,
                                                          &touch_x, &touch_y);
      if (CY_RSLT_SUCCESS == touch_result)
      {
        s_fusion_status.touch_read_ok++;
        touch_x_to_send = (int16_t)touch_x;
        touch_y_to_send = (int16_t)touch_y;
        touch_pressed_to_send = 1U;
      }
      else
      {
        s_fusion_status.touch_read_fail++;
      }

      if ((0U != touch_first_publish) ||
          (touch_x_to_send != touch_x_last) ||
          (touch_y_to_send != touch_y_last) ||
          (touch_pressed_to_send != touch_pressed_last))
      {
        if (true == cm33_ipc_send_touch(touch_x_to_send, touch_y_to_send, touch_pressed_to_send))
        {
          s_fusion_status.touch_send_ok++;
          touch_x_last = touch_x_to_send;
          touch_y_last = touch_y_to_send;
          touch_pressed_last = touch_pressed_to_send;
          touch_first_publish = 0U;
          if (true == s_touch_stream_enabled)
          {
            printf("[CM33.Touch] x=%d y=%d pressed=%u\r\n", (int)touch_x_last, (int)touch_y_last, (unsigned int)touch_pressed_last);
          }
        }
        else
        {
          s_fusion_status.touch_send_fail++;
        }
      }
      s_fusion_status.touch_x = touch_x_last;
      s_fusion_status.touch_y = touch_y_last;
      s_fusion_status.touch_pressed = touch_pressed_last;
    }
#endif

    /* Update elapsed time.*/
    xElapsedTime += (TickType_t)(((xCurrWakeTime - xLastWakeTime) * portTICK_PERIOD_MS) * 1000U);
    xLastWakeTime = xCurrWakeTime;

    /* Wait for the next cycle.*/
    vTaskDelayUntil(&xCurrWakeTime,
                    (const TickType_t)TASK_SENSOR_HUB_FUSION_RATE_MS);
  }
}

bool sensor_hub_fusion_get_status(sensor_hub_fusion_status_t *out_status)
{
  if (NULL == out_status)
  {
    return false;
  }
  taskENTER_CRITICAL();
  *out_status = (sensor_hub_fusion_status_t)s_fusion_status;
  taskEXIT_CRITICAL();
  return true;
}

bool sensor_hub_fusion_get_sample(sensor_hub_sample_t *out_sample)
{
  if (NULL == out_sample)
  {
    return false;
  }
  taskENTER_CRITICAL();
  *out_sample = (sensor_hub_sample_t)s_fusion_sample;
  taskEXIT_CRITICAL();
  return true;
}

void sensor_hub_fusion_set_stream(bool enable)
{
  s_stream_enabled = enable;
}

void sensor_hub_fusion_set_swap_yz(bool enable)
{
  s_swap_yz = enable;
}

void sensor_hub_fusion_set_sample_rate(uint16_t rate_hz)
{
  uint16_t loop_hz = (uint16_t)(1000U / TASK_SENSOR_HUB_FUSION_RATE_MS);
  if (0U == rate_hz)
  {
    rate_hz = 1U;
  }
  if (rate_hz > loop_hz)
  {
    rate_hz = loop_hz;
  }
  s_sample_rate_hz = rate_hz;
}

void sensor_hub_fusion_set_touch_stream(bool enable)
{
  s_touch_stream_enabled = enable;
}

void sensor_hub_fusion_set_fusion_enabled(bool enable)
{
  s_fusion_enabled = enable;
}

void sensor_hub_fusion_set_output_mode(sensor_hub_output_mode_t mode)
{
  if ((SENSOR_HUB_OUTPUT_MODE_QUATERNION == mode) || (SENSOR_HUB_OUTPUT_MODE_EULER == mode) || (SENSOR_HUB_OUTPUT_MODE_DATA == mode))
  {
    s_output_mode = mode;
  }
}

bool sensor_hub_fusion_calib_status(uint8_t *acc, uint8_t *gyr, uint8_t *mag)
{
  bool ready = false;
  uint8_t acc_local = 0U;
  uint8_t gyr_local = 0U;
  bool supported = false;

  taskENTER_CRITICAL();
  ready = s_bsxlite_ready;
  acc_local = s_calib_acc;
  gyr_local = s_calib_gyr;
  supported = s_calib_supported;
  taskEXIT_CRITICAL();

  if (NULL != acc)
  {
    *acc = (true == supported) ? acc_local : 0U;
  }
  if (NULL != gyr)
  {
    *gyr = (true == supported) ? gyr_local : 0U;
  }
  if (NULL != mag)
  {
    *mag = 0U;
  }
  return ready;
}

bool sensor_hub_fusion_calib_reset(void)
{
  bool accepted = false;

  taskENTER_CRITICAL();
  if ((true == s_bsxlite_ready) && (true == s_calib_supported))
  {
    s_calib_reset_requested = true;
    accepted = true;
  }
  taskEXIT_CRITICAL();

  return accepted;
}

/*******************************************************************************
 * Function Name: create_sensor_hub_fusion_task
 *******************************************************************************
 * Summary:
 *  Function that creates "sensor_hub_fusion_task"
 *
 * Parameters:
 *  None
 *
 * Return:
 *  CY_RSLT_SUCCESS upon successful creation of the motion sensor task, else a
 *  non-zero value that indicates the error.
 *
 *******************************************************************************/
cy_rslt_t create_sensor_hub_fusion_task(void)
{

  BaseType_t status;

  /* Create the "sensor_hub_daq" Task */
  status = xTaskCreate(sensor_hub_fusion_task, "Sensor Hub Fusion",
                       TASK_SENSOR_HUB_FUSION_STACK_SIZE, NULL,
                       TASK_SENSOR_HUB_FUSION_PRIORITY, NULL);

  return (pdPASS == status) ? CY_RSLT_SUCCESS : APP_RSLT_FUSION_TASK_CREATE_FAILED;
}

/* [] END OF FILE */