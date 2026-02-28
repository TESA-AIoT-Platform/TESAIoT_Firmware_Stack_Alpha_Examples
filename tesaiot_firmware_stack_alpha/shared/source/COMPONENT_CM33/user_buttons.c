/*******************************************************************************
 * File Name        : user_buttons.c
 *
 * Description      : Implementation of individual handle-based User Buttons.
 *                    Each button has its own handle; events are delivered via callbacks.
 *
 * Author           : Asst.Prof.Santi Nuratch, Ph.D
 *                    Thailand Embedded Systems Association (TESA)
 *
 *******************************************************************************/

#include "user_buttons.h"
#include "FreeRTOS.h"
#include "cybsp.h"
#include "ipc_communication.h"
#include "queue.h"
#include "task.h"
#include "user_buttons_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOTAL_BUTTONS (2U)

/*******************************************************************************
 * Global Variable(s)
 *******************************************************************************/
static bool s_initialized = false;
static bool s_init_started = false;

struct user_buttons_context
{
  uint32_t button_id;
  GPIO_PRT_Type *port;
  uint32_t pin;
  uint32_t stack_size;
  uint32_t priority;
  uint32_t press_count;
  QueueHandle_t event_q;
  TaskHandle_t bridge_task;
  bool running;
};

/* Global registry of button handles for the ISR */
static user_buttons_t s_button_registry[MAX_TOTAL_BUTTONS] = {0};
static uint32_t s_registered_count = 0;

#define MAX_USER_CALLBACKS (8U)
typedef struct
{
  button_id_t id;
  user_button_event_cb_t cb;
  bool used;
} user_cb_entry_t;

static user_cb_entry_t s_pressed_cbs[MAX_USER_CALLBACKS] = {0};
static user_cb_entry_t s_released_cbs[MAX_USER_CALLBACKS] = {0};
static user_cb_entry_t s_change_cbs[MAX_USER_CALLBACKS] = {0};

static void invoke_callbacks_for_id(user_buttons_t handle, const button_event_t *evt, user_cb_entry_t *arr)
{
  const button_id_t id = (button_id_t)handle->button_id;
  for (uint32_t i = 0U; i < MAX_USER_CALLBACKS; i++)
  {
    if (arr[i].used && (id == arr[i].id))
    {
      arr[i].cb(handle, evt);
    }
  }
}

static bool add_user_cb(user_cb_entry_t *arr, button_id_t id, user_button_event_cb_t cb)
{
  for (uint32_t i = 0U; i < MAX_USER_CALLBACKS; i++)
  {
    if (!arr[i].used)
    {
      arr[i].id = id;
      arr[i].cb = cb;
      arr[i].used = true;
      return true;
    }
  }
  return false;
}

/*******************************************************************************
 * Function Name: user_button_bridge_task
 ********************************************************************************
 *
 * Bridge task that receives button events from the queue and invokes
 * registered callbacks (pressed, released, changed) with (handle, evt).
 *
 * Parameters:
 *  pvParameters - user_buttons_t handle for the button instance
 *
 * Return :
 *  void
 *
 *******************************************************************************/
static void user_button_bridge_task(void *pvParameters)
{
  user_buttons_t handle = (user_buttons_t)pvParameters;
  button_event_t raw_event;

  while (1)
  {
    if (xQueueReceive(handle->event_q, &raw_event, portMAX_DELAY) == pdPASS)
    {
      raw_event.press_count = handle->press_count;

      if (raw_event.is_pressed)
      {
        invoke_callbacks_for_id(handle, &raw_event, s_pressed_cbs);
        invoke_callbacks_for_id(handle, &raw_event, s_change_cbs);
      }
      else
      {
        invoke_callbacks_for_id(handle, &raw_event, s_released_cbs);
        invoke_callbacks_for_id(handle, &raw_event, s_change_cbs);
      }
    }
  }
}

/*******************************************************************************
 * Function Name: buttons_shared_interrupt_handler
 ********************************************************************************
 *
 * Shared interrupt service routine for all user button GPIO interrupts.
 * Queues events for deferred processing by the bridge task.
 *
 * Parameters:
 *  none
 *
 * Return :
 *  void
 *
 *******************************************************************************/
static void buttons_shared_interrupt_handler(void)
{
  for (uint32_t i = 0; i < s_registered_count; i++)
  {
    user_buttons_t btn = s_button_registry[i];
    if (btn == NULL || !btn->running)
      continue;

    if (0u != Cy_GPIO_GetInterruptStatus(btn->port, btn->pin))
    {
      bool is_pressed = (Cy_GPIO_Read(btn->port, btn->pin) == 0);

      if (is_pressed)
      {
        btn->press_count++;
      }

      button_event_t event = {.button_id = btn->button_id, .is_pressed = is_pressed, .press_count = btn->press_count};

      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      xQueueSendFromISR(btn->event_q, &event, &xHigherPriorityTaskWoken);

      Cy_GPIO_ClearInterrupt(btn->port, btn->pin);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  }
}

/*******************************************************************************
 * Function Name: user_buttons_init
 ********************************************************************************
 * Summary:
 * Creates and configures BTN1 (and BTN2 if enabled) with board defaults and
 * starts monitoring. Idempotent; subsequent calls return true without re-init.
 *
 * Parameters:
 *  none
 *
 * Return :
 *  true  - Success
 *  false - Failure
 *
 *******************************************************************************/
bool user_buttons_init(void)
{
  if (s_initialized)
  {
    return true;
  }
  s_init_started = true;

  user_buttons_t btn1 = user_buttons_create(BUTTON_ID_0);
  if (btn1 == NULL)
  {
    return false;
  }

  (void)user_buttons_configure(btn1, CYBSP_USER_BTN1_PORT, CYBSP_USER_BTN1_PIN, 512, tskIDLE_PRIORITY + 2);
  if (!user_buttons_start(btn1))
  {
    user_buttons_destroy(btn1);
    return false;
  }

#ifdef CYBSP_USER_BTN2_ENABLED
  user_buttons_t btn2 = user_buttons_create(BUTTON_ID_1);
  if (btn2 != NULL)
  {
    (void)user_buttons_configure(btn2, CYBSP_USER_BTN2_PORT, CYBSP_USER_BTN2_PIN, 512, tskIDLE_PRIORITY + 2);
    if (!user_buttons_start(btn2))
    {
      user_buttons_destroy(btn2);
    }
  }
#endif

  s_initialized = true;
  return true;
}

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
user_buttons_t user_buttons_create(button_id_t button_id)
{
  if (!s_init_started || s_registered_count >= MAX_TOTAL_BUTTONS)
  {
    return NULL;
  }

  struct user_buttons_context *handle = pvPortMalloc(sizeof(struct user_buttons_context));
  if (handle == NULL)
  {
    return NULL;
  }

  memset(handle, 0, sizeof(struct user_buttons_context));
  handle->button_id = (uint32_t)button_id;
  handle->event_q = xQueueCreate(5, sizeof(button_event_t));

  if (handle->event_q == NULL)
  {
    vPortFree(handle);
    return NULL;
  }

  s_button_registry[s_registered_count++] = handle;
  return handle;
}

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
 *  false - Failure (handle or port NULL)
 *
 *******************************************************************************/
bool user_buttons_configure(user_buttons_t handle, GPIO_PRT_Type *port, uint32_t pin, uint32_t stack_size,
                            uint32_t priority)
{
  if (handle == NULL || port == NULL)
  {
    return false;
  }

  handle->port = port;
  handle->pin = pin;
  handle->stack_size = stack_size;
  handle->priority = priority;
  return true;
}

/*******************************************************************************
 * Function Name: user_buttons_start
 ********************************************************************************
 *
 * Starts monitoring this specific button (GPIO interrupt and bridge task).
 *
 * Parameters:
 *  handle - Button handle
 *
 * Return :
 *  true  - Success
 *  false - Failure
 *
 *******************************************************************************/
bool user_buttons_start(user_buttons_t handle)
{
  if (handle == NULL || handle->running || handle->port == NULL)
    return false;

  /* Hardware setup */
  Cy_GPIO_ClearInterrupt(handle->port, handle->pin);
  Cy_GPIO_SetInterruptEdge(handle->port, handle->pin, CY_GPIO_INTR_BOTH);

  /* Shared Interrupt setup (idempotent) */
  cy_stc_sysint_t intrCfg = {.intrSrc = CYBSP_USER_BTN_IRQ, .intrPriority = 7U};
  (void)Cy_SysInt_Init(&intrCfg, &buttons_shared_interrupt_handler);

  /* Task creation */
  char task_name[16];
  snprintf(task_name, sizeof(task_name), "Btn%luBridge", handle->button_id);

  if (xTaskCreate(user_button_bridge_task, task_name, (configSTACK_DEPTH_TYPE)handle->stack_size, handle,
                  handle->priority, &handle->bridge_task) != pdPASS)
  {
    return false;
  }

  NVIC_EnableIRQ(intrCfg.intrSrc);
  handle->running = true;
  return true;
}

/*******************************************************************************
 * Function Name: user_buttons_stop
 ********************************************************************************
 *
 * Stops monitoring this specific button (disables interrupt, deletes bridge task).
 *
 * Parameters:
 *  handle - Button handle
 *
 * Return :
 *  void
 *
 *******************************************************************************/
void user_buttons_stop(user_buttons_t handle)
{
  if (handle == NULL || !handle->running)
    return;

  /* Disable interrupt for this specific pin if possible,
   * but usually we just mark it as not running so the ISR ignores it.
   * To be safer, we could set interrupt edge to NONE.
   */
  Cy_GPIO_SetInterruptEdge(handle->port, handle->pin, CY_GPIO_INTR_DISABLE);

  if (handle->bridge_task != NULL)
  {
    vTaskDelete(handle->bridge_task);
    handle->bridge_task = NULL;
  }
  handle->running = false;
}

/*******************************************************************************
 * Function Name: user_buttons_destroy
 ********************************************************************************
 *
 * Destroys the handle and releases memory. Stops the button if running.
 *
 * Parameters:
 *  handle - Button handle
 *
 * Return :
 *  void
 *
 *******************************************************************************/
void user_buttons_destroy(user_buttons_t handle)
{
  if (handle == NULL)
    return;
  user_buttons_stop(handle);
  vQueueDelete(handle->event_q);

  /* Remove from registry */
  for (uint32_t i = 0; i < s_registered_count; i++)
  {
    if (s_button_registry[i] == handle)
    {
      s_button_registry[i] = s_button_registry[s_registered_count - 1];
      s_button_registry[s_registered_count - 1] = NULL;
      s_registered_count--;
      break;
    }
  }
  vPortFree(handle);
}

/*******************************************************************************
 * Function Name: user_button_on_pressed
 ********************************************************************************
 *
 * Registers a callback for button press events.
 *
 * Parameters:
 *  id       - BUTTON_ID_0 or BUTTON_ID_1
 *  callback - Called with (const button_event_t *evt) when the button is pressed
 *
 * Return :
 *  true  - Success
 *  false - Failure (not initialized, callback NULL, id invalid)
 *
 *******************************************************************************/
bool user_button_on_pressed(button_id_t id, user_button_event_cb_t callback)
{
  if (callback == NULL || id >= BUTTON_ID_MAX)
  {
    return false;
  }
  if (!s_initialized)
  {
    return false;
  }
  return add_user_cb(s_pressed_cbs, id, callback);
}

/*******************************************************************************
 * Function Name: user_button_on_released
 ********************************************************************************
 *
 * Registers a callback for button release events.
 *
 * Parameters:
 *  id       - BUTTON_ID_0 or BUTTON_ID_1
 *  callback - Called with (const button_event_t *evt) when the button is released
 *
 * Return :
 *  true  - Success
 *  false - Failure
 *
 *******************************************************************************/
bool user_button_on_released(button_id_t id, user_button_event_cb_t callback)
{
  if (callback == NULL || id >= BUTTON_ID_MAX)
  {
    return false;
  }
  if (!s_initialized)
  {
    return false;
  }
  return add_user_cb(s_released_cbs, id, callback);
}

/*******************************************************************************
 * Function Name: user_button_on_changed
 ********************************************************************************
 *
 * Registers a callback for both press and release events.
 * Use evt->is_pressed to distinguish.
 *
 * Parameters:
 *  id       - BUTTON_ID_0 or BUTTON_ID_1
 *  callback - Called with (const button_event_t *evt) on press or release
 *
 * Return :
 *  true  - Success
 *  false - Failure
 *
 *******************************************************************************/
bool user_button_on_changed(button_id_t id, user_button_event_cb_t callback)
{
  if (callback == NULL || id >= BUTTON_ID_MAX)
  {
    return false;
  }
  if (!s_initialized)
  {
    return false;
  }
  return add_user_cb(s_change_cbs, id, callback);
}

bool user_buttons_get_state(button_id_t id, uint32_t *press_count, bool *is_pressed)
{
  if (id >= BUTTON_ID_MAX)
  {
    return false;
  }
  for (uint32_t i = 0U; i < s_registered_count; i++)
  {
    user_buttons_t h = s_button_registry[i];
    if ((NULL != h) && (h->button_id == (uint32_t)id))
    {
      if (NULL != press_count)
      {
        *press_count = h->press_count;
      }
      if (NULL != is_pressed)
      {
        *is_pressed = (NULL != h->port) && (Cy_GPIO_Read(h->port, h->pin) == 0U);
      }
      return true;
    }
  }
  return false;
}
