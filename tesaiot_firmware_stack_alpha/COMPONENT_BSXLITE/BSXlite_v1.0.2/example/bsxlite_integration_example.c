/**
 *
 * @file        integration_example.c
 *
 * @brief       Generic example for BSXlite integration
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../bsxlite_interface.h"
#include "file_operations.h"

/************************************************************************************************************/
/*                                         EXAMPLE MAIN                                                     */
/************************************************************************************************************/
/** integration example: main.c **/
int main()
{
    /** file operations: Input & Output files are defined  here **/
    char* input_file_name = "example/shortLog_bsxlite.txt";
    char* output_file_name = "example/out_bsxlite.txt";

    /** file operations:  Write header into the output file **/
    data_log_write_header(output_file_name);

    /** initialize input vars **/
    vector_3d_t accel_in, gyro_in;
    int32_t w_time_stamp = 0U;
    memset(&accel_in, 0x00, sizeof(accel_in));
    memset(&gyro_in, 0x00, sizeof(gyro_in));

    /** Initialize output variables **/
    bsxlite_out_t bsxlite_fusion_out;
    memset(&bsxlite_fusion_out, 0x00, sizeof(bsxlite_fusion_out));

    /** library return */
    bsxlite_return_t  result;

    /** library VERSION */
    bsxlite_version version_p;
    bsxlite_get_version(&version_p);

    /** library api call : initialize library **/
    bsxlite_instance_t instance = 0x00;
    result = bsxlite_init(&instance);

    if (result != BSXLITE_OK)
        return EXIT_FAILURE;

    /** file operations : read input log and iterate doStep api calls**/
    FILE* input_file = fopen(input_file_name, "r");

    if (input_file != NULL)
    {
        /** Get data and call do step function **/
        /* acc_data  : x,y,z to be provided in [meter/second^2] */
        /* gyro_data : x,y,z to be provided in [radians/sec]    */
        /* timestamp : to be provided in micro seconds          */
        while (fscanf(input_file, "%d\t%f\t%f\t%f\t%d\t%f\t%f\t%f\t\n", &w_time_stamp, &accel_in.x, &accel_in.y, &accel_in.z,
                      &w_time_stamp, &gyro_in.x, &gyro_in.y, &gyro_in.z) != EOF)
        {

            /** library api call: doStep **/
            result = bsxlite_do_step(&instance,
                                     w_time_stamp,
                                     &accel_in,
                                     &gyro_in,
                                     &(bsxlite_fusion_out));

            /** evaluate library return */
            if (result != BSXLITE_OK)
            {
                switch(result)
                {
                    case (BSXLITE_E_DOSTEPS_TSINTRADIFFOUTOFRANGE):
                        {
                            printf("Error: Subsequent time stamps in input data were found to be out of range from the expected sample rate!!!\n");
                            break;
                        }
                    case (BSXLITE_E_FATAL):
                        {
                            printf("Fatal Error: Process terminating!!!\n");
                            return EXIT_FAILURE;
                        }
                    case (BSXLITE_I_DOSTEPS_NOOUTPUTSRETURNABLE):
                        {
                            printf("Info: Sufficient memory not allocated for output,  all outputs cannot be returned because no memory provided!!!\n");
                            break;
                        }
                    default:
                        {
                            printf("Info: Unknown return \n");
                            break;
                        }
                }
            }

            /** file operations : log the output data into file */
            data_log_write_output(output_file_name, &accel_in, &gyro_in, w_time_stamp,
                                  &(bsxlite_fusion_out.rotation_vector), &(bsxlite_fusion_out.orientation),
                                  &(bsxlite_fusion_out.accel_calibration_status), &(bsxlite_fusion_out.gyro_calibration_status));
        }
        /** file operations : close the input file */
        fclose(input_file);
    }

    /** reset the module */
    result = bsxlite_set_to_default(&instance);
    if (result != BSXLITE_OK)
        return EXIT_FAILURE;


    printf("Process Done !!!\n");

    fflush(stdout);//lint !e534
    return EXIT_SUCCESS;
}
/*! @}*/
