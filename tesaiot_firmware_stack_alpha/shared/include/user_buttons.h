/*******************************************************************************
 * File Name        : user_buttons.h
 *
 * Description      : Library for managing individual user buttons.
 *                    Each button has its own handle; events are delivered via callbacks.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 *
 *******************************************************************************/

#ifndef USER_BUTTONS_H
#define USER_BUTTONS_H

#include "user_buttons_types.h"
#include "cy_pdl.h"
#include "ipc_communication.h"

typedef struct user_buttons_context *user_buttons_t;

/*******************************************************************************
 * Function Name: user_buttons_init
 ********************************************************************************
 *
 * Initializes the user buttons module. Creates and starts BTN1/BTN2 with
 * board defaults. Must be called before cm33_ipc_pipe_start().
 *
 * Parameters:
 *  none
 *
 * Return :
 *  true  - Success
 *  false - Failure
 *
 *******************************************************************************/
bool user_buttons_init(void);

/*******************************************************************************
 * Function Name: user_buttons_create
 ********************************************************************************
 *
 * Creates a new user button instance.
 * Requires user_buttons_init() to have been called first.
 *
 * Parameters:
 *  button_id - BUTTON_ID_0 or BUTTON_ID_1
 *
 * Return :
 *  user_buttons_t - Handle to the instance, or NULL if failed
 *
 *******************************************************************************/
user_buttons_t user_buttons_create(button_id_t button_id);

/*******************************************************************************
 * Function Name: user_buttons_configure
 ********************************************************************************
 *
 * Configures the hardware and task parameters for a button.
 *
 * Parameters:
 *  handle     - Button handle
 *  port       - GPIO port hardware address
 *  pin        - GPIO pin number
 *  stack_size - Stack size for the internal bridge task
 *  priority   - Priority for the internal bridge task
 *
 * Return :
 *  true  - Success
 *  false - Failure
 *
 *******************************************************************************/
bool user_buttons_configure(user_buttons_t handle, GPIO_PRT_Type *port, uint32_t pin, uint32_t stack_size,
                            uint32_t priority);

/*******************************************************************************
 * Function Name: user_buttons_start
 ********************************************************************************
 *
 * Starts monitoring this specific button.
 *
 * Parameters:
 *  handle - Button handle
 *
 * Return :
 *  true  - Success
 *  false - Failure
 *
 *******************************************************************************/
bool user_buttons_start(user_buttons_t handle);

/*******************************************************************************
 * Function Name: user_buttons_stop
 ********************************************************************************
 *
 * Stops monitoring this specific button.
 *
 * Parameters:
 *  handle - Button handle
 *
 * Return :
 *  void
 *
 *******************************************************************************/
void user_buttons_stop(user_buttons_t handle);

/*******************************************************************************
 * Function Name: user_buttons_destroy
 ********************************************************************************
 *
 * Destroys the handle and releases memory.
 *
 * Parameters:
 *  handle - Button handle
 *
 * Return :
 *  void
 *
 *******************************************************************************/
void user_buttons_destroy(user_buttons_t handle);

typedef void (*user_button_event_cb_t)(user_buttons_t switch_handle, const button_event_t *evt);

/*******************************************************************************
 * Function Name: user_button_on_pressed
 ********************************************************************************
 *
 * Registers a callback for button press events.
 *
 * Parameters:
 *  id       - BUTTON_ID_0 or BUTTON_ID_1
 *  callback - Called with (switch_handle, evt) when the button is pressed
 *
 * Return :
 *  true  - Success
 *  false - Failure
 *
 *******************************************************************************/
bool user_button_on_pressed(button_id_t id, user_button_event_cb_t callback);

/*******************************************************************************
 * Function Name: user_button_on_released
 ********************************************************************************
 *
 * Registers a callback for button release events.
 *
 * Parameters:
 *  id       - BUTTON_ID_0 or BUTTON_ID_1
 *  callback - Called with (switch_handle, evt) when the button is released
 *
 * Return :
 *  true  - Success
 *  false - Failure
 *
 *******************************************************************************/
bool user_button_on_released(button_id_t id, user_button_event_cb_t callback);

/*******************************************************************************
 * Function Name: user_button_on_changed
 ********************************************************************************
 *
 * Registers a callback for both press and release events.
 *
 * Parameters:
 *  id       - BUTTON_ID_0 or BUTTON_ID_1
 *  callback - Called with (switch_handle, evt) on press or release; use evt->is_pressed to distinguish
 *
 * Return :
 *  true  - Success
 *  false - Failure
 *
 *******************************************************************************/
bool user_button_on_changed(button_id_t id, user_button_event_cb_t callback);

bool user_buttons_get_state(button_id_t id, uint32_t *press_count, bool *is_pressed);

#endif /* USER_BUTTONS_H */
