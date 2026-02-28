/*******************************************************************************
 * File Name        : cm55_ipc_communication.c
 *
 * Description      : IPC pipe communication setup for CM55 CPU.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 *
 *******************************************************************************/

#include "ipc_communication.h"
#include "cy_sysint.h"

/*******************************************************************************
 * Global Variable(s)
 *******************************************************************************/
/* Create an array of endpoint structures */
static cy_stc_ipc_pipe_ep_t cm55_ipc_pipe_array[CY_IPC_MAX_ENDPOINTS];

/* CB Array for EP2 */
static cy_ipc_pipe_callback_ptr_t ep2_cb_array[CY_IPC_CYPIPE_CLIENT_CNT];

/*******************************************************************************
 * Function Name: Cy_SysIpcPipeIsrCm55
 ********************************************************************************
 * Summary:
 * This is the interrupt service routine for the system pipe.
 *
 * Parameters:
 *  none
 *
 * Return :
 *  void
 *
 *******************************************************************************/
void Cy_SysIpcPipeIsrCm55(void)
{
  Cy_IPC_Pipe_ExecuteCallback(CM55_IPC_PIPE_EP_ADDR);
}

/*******************************************************************************
 * Function Name: cm55_ipc_communication_setup
 ********************************************************************************
 * Summary:
 *  This function configures IPC Pipe for CM55 to CM33 communication.
 *
 * Parameters:
 *  none
 *
 * Return :
 *  void
 *
 *******************************************************************************/
void cm55_ipc_communication_setup(void)
{

  /******************************************************/
  /* IPC pipe endpoint-1 and endpoint-2. CM55 <--> CM33 */
  /******************************************************/

  /* clang-format off */
    static const cy_stc_ipc_pipe_config_t cm55_ipc_pipe_config =
    {
			/* receiver endpoint CM55 */
			{
				.ipcNotifierNumber     = CY_IPC_INTR_CYPIPE_EP2,  			/* IPC interrupt notifier for EP2 */
				.ipcNotifierPriority   = CY_IPC_INTR_CYPIPE_PRIOR_EP2,  /* Interrupt priority for EP2 */
				.ipcNotifierMuxNumber  = CY_IPC_INTR_CYPIPE_MUX_EP2,    /* Interrupt mux number for EP2 */
				.epAddress             = CM55_IPC_PIPE_EP_ADDR,         /* CM55 endpoint memory address */
				{
					.epChannel         = CY_IPC_CHAN_CYPIPE_EP2,   /* IPC channel for EP2 */
					.epIntr            = CY_IPC_INTR_CYPIPE_EP2,   /* Interrupt number for EP2 */
					.epIntrmask        = CY_IPC_CYPIPE_INTR_MASK   /* Interrupt release mask for EP2 */
				}
			},

			/* sender endpoint CM33 */
			{
				.ipcNotifierNumber     = CY_IPC_INTR_CYPIPE_EP1,  			/* IPC interrupt notifier for EP1 */
				.ipcNotifierPriority   = CY_IPC_INTR_CYPIPE_PRIOR_EP1,  /* Interrupt priority for EP1 */
				.ipcNotifierMuxNumber  = CY_IPC_INTR_CYPIPE_MUX_EP1,    /* Interrupt mux number for EP1 */
				.epAddress             = CM33_IPC_PIPE_EP_ADDR,         /* CM33 endpoint memory address */
				{
					.epChannel         = CY_IPC_CHAN_CYPIPE_EP1,   /* IPC channel for EP1 */
					.epIntr            = CY_IPC_INTR_CYPIPE_EP1,   /* Interrupt number for EP1 */
					.epIntrmask        = CY_IPC_CYPIPE_INTR_MASK   /* Interrupt release mask for EP1 */
				}
			},
    .endpointClientsCount          = CY_IPC_CYPIPE_CLIENT_CNT,   /* Number of pipe endpoint clients */
    .endpointsCallbacksArray       = ep2_cb_array,               /* Array of per-client callback pointers */
    .userPipeIsrHandler            = &Cy_SysIpcPipeIsrCm55       /* User ISR invoked on pipe interrupt */
    };
  /* clang-format on */

  Cy_IPC_Pipe_Config(cm55_ipc_pipe_array);

  Cy_IPC_Pipe_Init(&cm55_ipc_pipe_config);

  {
    cy_stc_sysint_t ep2_intr_cfg = {
      .intrSrc      = (IRQn_Type)CY_IPC_INTR_CYPIPE_MUX_EP2,
      .intrPriority = (uint32_t)CY_IPC_INTR_CYPIPE_PRIOR_EP2
    };
    (void)Cy_SysInt_Init(&ep2_intr_cfg, &Cy_SysIpcPipeIsrCm55);
    NVIC_EnableIRQ((IRQn_Type)CY_IPC_INTR_CYPIPE_MUX_EP2);
  }
}