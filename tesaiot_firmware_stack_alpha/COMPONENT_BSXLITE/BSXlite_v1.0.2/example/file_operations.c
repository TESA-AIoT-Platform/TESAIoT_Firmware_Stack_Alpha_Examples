/**
 *
 * @file        file_operations.c
 *
 * @brief
 *
 *
 */

/*!
 * @defgroup integration_support
 * @brief
 * @{*/

/********************************************************/
/* system header includes */

/********************************************************/
/* own header files */
#include "file_operations.h"

/********************************************************/
/* constant definitions */

/********************************************************/
/* global variables */

/********************************************************/
/* local/static variables */

/********************************************************/
/* functions */
/************************************************************************************************************/
/*                                         HELPER FILE\LOG WRITE FUNCTIONS                                  */
/************************************************************************************************************/
/** File read/write operations **/
void data_log_write_header(const char* file_out)
{
    FILE* output_file = fopen(file_out, "w");
    fprintf(output_file,
            "gyroTimeStamp\t,"
            "accRawData.x\t,accRawData.y\t,accRawData.z\t,"
            "gyroRawData.x\t,gyroRawData.y\t,gyroRawData.z\t,"
            "gameRotationQuat.x\t,gameRotationQuat.y\t,gameRotationQuat.z\t,gameRotationQuat.w\t,"
            "orientEuler.heading\t,orientEuler.pitch\t,orientEuler.roll\t,orientEuler.yaw\t,"
            "accelerationCalibrationAccuracy\t, gyroCalibrationAccuracy\t,\n");
    fclose(output_file);
}

void data_log_write_output(const char* file_out, const vector_3d_t *input_accel,
                           const vector_3d_t *input_gyro,
                           const int32_t gyro_time, const quaternion_t *rotation_imu,
                           const euler_angles_t *orientation,
                           const uint8_t *accel_calib_accur,
                           const uint8_t *gyro_calib_accur)
{
    FILE* output_file = fopen(file_out, "a");
    fprintf(output_file, "%d,"
            "%5.6f,%5.6f,%5.6f,"
            "%5.6f,%5.6f,%5.6f,"
            "%5.6f,%5.6f,%5.6f,%5.6f,"
            "%5.6f,%5.6f,%5.6f,%5.6f,"
            "%u,%u\n",
            gyro_time, input_accel->x, input_accel->y,
            input_accel->z,
            input_gyro->x, input_gyro->y, input_gyro->z,
            rotation_imu->x,
            rotation_imu->y, rotation_imu->z, rotation_imu->w,
            orientation->heading,
            orientation->pitch, orientation->roll,
            orientation->yaw,
            *accel_calib_accur, *gyro_calib_accur);
    fclose(output_file);
}
