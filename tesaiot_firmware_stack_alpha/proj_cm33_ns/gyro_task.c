#include "gyro_task.h"
#include "cm33_ipc_pipe.h"

#include "FreeRTOS.h"
#include "cy_syslib.h"
#include "cybsp.h"
#include "task.h"
#include <string.h>

TaskHandle_t gyro_task_handle;

static uint32_t s_prng_state = 0U;

static float gyro_generate_random_value(void) {
  s_prng_state = (s_prng_state * 1103515245U) + 12345U;
  uint32_t random_int = (s_prng_state >> 16U) & 0x7FFFU;
  float normalized = ((float)random_int) / 16383.0f;
  return (GYRO_DATA_MIN + normalized * (GYRO_DATA_MAX - GYRO_DATA_MIN));
}

void gyro_task(void *arg) {
  gyro_data_t gyro_data;
  uint32_t sequence = 0U;

  (void)arg;

  s_prng_state = (uint32_t)xTaskGetTickCount();

  vTaskDelay(pdMS_TO_TICKS(1000U));

  while (true) {
    gyro_data.ax = gyro_generate_random_value();
    gyro_data.ay = gyro_generate_random_value();
    gyro_data.az = gyro_generate_random_value();

    if (cm33_ipc_send_gyro_data(&gyro_data, sequence)) {
      sequence++;
    }

    vTaskDelay(pdMS_TO_TICKS(GYRO_SEND_INTERVAL_MS));
  }
}
