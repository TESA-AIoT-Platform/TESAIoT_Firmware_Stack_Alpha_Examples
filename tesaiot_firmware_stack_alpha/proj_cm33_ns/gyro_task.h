#ifndef SOURCE_GYRO_TASK_H_
#define SOURCE_GYRO_TASK_H_

#include "FreeRTOS.h"
#include "ipc_communication.h"
#include "gyro_task_config.h"
#include "task.h"

extern TaskHandle_t gyro_task_handle;

void gyro_task(void *arg);

#endif
