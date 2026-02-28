/*******************************************************************************
 * File Name        : cm33_ipc_communication.c
 *
 * Description      : IPC pipe communication setup for CM33 CPU.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 *
 *******************************************************************************/

#include "ipc_communication.h"

/*******************************************************************************
 * Global Variable(s)
 *******************************************************************************/
/* Create an array of endpoint structures */
static cy_stc_ipc_pipe_ep_t cm33_ipc_pipe_ep_array[CY_IPC_MAX_ENDPOINTS];

/* CB Array for EP1 */
static cy_ipc_pipe_callback_ptr_t ep1_cb_array[CY_IPC_CYPIPE_CLIENT_CNT];

/* Allocate and initialize semaphores for the system operations. */
CY_SECTION_SHAREDMEM static uint32_t ipc_sema_array[CY_IPC_SEMA_COUNT / CY_IPC_SEMA_PER_WORD];

/*******************************************************************************
 * Function Name: cm33_ipc_pipe_isr
 ********************************************************************************
 *
 * This is the interrupt service routine for the system pipe.
 *
 * Parameters:
 *  none
 *
 * Return :
 *  void
 *
 *******************************************************************************/
void cm33_ipc_pipe_isr(void)
{
  Cy_IPC_Pipe_ExecuteCallback(CM33_IPC_PIPE_EP_ADDR);
}

/*******************************************************************************
 * Function Name: cm33_ipc_communication_setup
 ********************************************************************************
 * Summary:
 * This function...
 * 1. Initializes IPC Semaphore.
 * 2. Configures IPC Pipe for CM33 to CM55 communication.
 *
 * Parameters:
 *  none
 *
 * Return :
 *  void
 *
 *******************************************************************************/
void cm33_ipc_communication_setup(void)
{
  /******************************************************/
  /* IPC pipe endpoint-1 and endpoint-2. CM33 <--> CM55 */
  /******************************************************/

  /* clang-format off */
  static const cy_stc_ipc_pipe_config_t cm33_ipc_pipe_config = {

      /* receiver endpoint CM33 */
      {
        .ipcNotifierNumber = CY_IPC_INTR_CYPIPE_EP1,         /* IPC interrupt notifier for EP1 */
        .ipcNotifierPriority = CY_IPC_INTR_CYPIPE_PRIOR_EP1, /* Interrupt priority for EP1 */
        .ipcNotifierMuxNumber = CY_IPC_INTR_CYPIPE_MUX_EP1,  /* Interrupt mux number for EP1 */
        .epAddress = CM33_IPC_PIPE_EP_ADDR,                  /* CM33 endpoint memory address */
        {
          .epChannel = CY_IPC_CHAN_CYPIPE_EP1,  /* IPC channel for EP1 */
          .epIntr = CY_IPC_INTR_CYPIPE_EP1,     /* Interrupt number for EP1 */
          .epIntrmask = CY_IPC_CYPIPE_INTR_MASK /* Interrupt release mask for EP1 */
        }
      },

      /* sender endpoint CM55 */
      {
        .ipcNotifierNumber = CY_IPC_INTR_CYPIPE_EP2,         /* IPC interrupt notifier for EP2 */
        .ipcNotifierPriority = CY_IPC_INTR_CYPIPE_PRIOR_EP2, /* Interrupt priority for EP2 */
        .ipcNotifierMuxNumber = CY_IPC_INTR_CYPIPE_MUX_EP2,  /* Interrupt mux number for EP2 */
        .epAddress = CM55_IPC_PIPE_EP_ADDR,                  /* CM55 endpoint memory address */
        {
          .epChannel = CY_IPC_CHAN_CYPIPE_EP2,  /* IPC channel for EP2 */
          .epIntr = CY_IPC_INTR_CYPIPE_EP2,     /* Interrupt number for EP2 */
          .epIntrmask = CY_IPC_CYPIPE_INTR_MASK /* Interrupt release mask for EP2 */
        }
      },
      .endpointClientsCount = CY_IPC_CYPIPE_CLIENT_CNT,     /* Number of pipe endpoint clients */
      .endpointsCallbacksArray = ep1_cb_array,              /* Array of per-client callback pointers */
      .userPipeIsrHandler = &cm33_ipc_pipe_isr};            /* User ISR invoked on pipe interrupt */
  /* clang-format on */

  Cy_IPC_Sema_Init(IPC0_SEMA_CH_NUM, CY_IPC_SEMA_COUNT, ipc_sema_array);

  (void)Cy_IPC_Sema_Set(IPC0_SEMA_CH_NUM, IPC_SEMA_INDEX_DEBUG_UART);

  Cy_IPC_Pipe_Config(cm33_ipc_pipe_ep_array);

  Cy_IPC_Pipe_Init(&cm33_ipc_pipe_config);
}