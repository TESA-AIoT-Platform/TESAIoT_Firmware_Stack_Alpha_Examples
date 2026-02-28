/**
 *
 * @file        file_operations.h
 *
 * @brief
 *
 *
 */

/*!
 * @defgroup integration_example_support
 * @brief
 * @{*/

#ifndef EXAMPLE_FILE_OPERATIONS_H_
#define EXAMPLE_FILE_OPERATIONS_H_

/********************************************************/
/* header includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../bsxlite_interface.h"
/********************************************************/
/* (extern) variable declarations */

/********************************************************/
/* function prototype declarations */
void data_log_write_header(const char* file_out);
void data_log_write_output(const char* file_out, const vector_3d_t *input_accel, const vector_3d_t *input_gyro, const int32_t gyro_time, const quaternion_t *rotation_imu,
                           const euler_angles_t *orientation,
                           const uint8_t *accel_calib_accur, const uint8_t *gyro_calib_accur);

/********************************************************/
/* inline function definitions */

#endif /* EXAMPLE_FILE_OPERATIONS_H_ */

/** @}*/
