/**
 * LED controller: drives the three user LEDs (red, green, blue) on the CM55 core
 * using BSP pin defines or P16 fallback. Depends on cybsp_init() for GPIO setup.
 */
#include "led_controller.h"
#include "cy_pdl.h"
#include "cybsp.h"

#if defined(CYBSP_USER_LED1_PORT) && defined(CYBSP_USER_LED2_PORT) && defined(CYBSP_USER_LED3_PORT)
#define LED_USE_BSP_ALL 1   /* All three LEDs have BSP port/pin defines. */
#elif defined(CYBSP_USER_LED_PORT)
#define LED_USE_BSP_RED 1   /* Only default user LED (red) has BSP define; green/blue use P16 fallback. */
#endif

#if defined(LED_USE_BSP_ALL)
#define LED_PORT(id)   ((id) == LED_RED ? CYBSP_USER_LED1_PORT : (id) == LED_GREEN ? CYBSP_USER_LED2_PORT : CYBSP_USER_LED3_PORT)  /* BSP port for id. */
#define LED_PIN(id)    ((id) == LED_RED ? CYBSP_USER_LED1_PIN : (id) == LED_GREEN ? CYBSP_USER_LED2_PIN : CYBSP_USER_LED3_PIN)       /* BSP pin for id. */
#elif defined(LED_USE_BSP_RED)
#define LED_PORT(id)   ((id) == LED_RED ? CYBSP_USER_LED_PORT : GPIO_PRT16)                       /* Red from BSP; green/blue on P16. */
#define LED_PIN(id)    ((id) == LED_RED ? CYBSP_USER_LED_PIN : (uint32_t)((id) == LED_GREEN ? 6U : 5U))  /* Pin 7 for red; 6/5 for green/blue. */
#else
#define LED_PORT(id)   (GPIO_PRT16)                                                                                                   /* P16 for all LEDs. */
#define LED_PIN(id)    ((uint32_t)((id) == LED_RED ? 7U : (id) == LED_GREEN ? 6U : 5U))                                               /* P16 pin 7/6/5 for red/green/blue. */
#endif

/**
 * Returns true if id is in range [0, LED_COUNT).
 */
static bool led_id_valid(led_id_t id)
{
  return (id < LED_COUNT);
}

/**
 * Optional init; GPIO is configured by cybsp_init(). Returns true for compatibility.
 */
bool led_controller_init(void)
{
  return true;
}

/**
 * Set one LED on or off. Invalid id is ignored (no-op).
 */
void led_controller_set(led_id_t id, bool on)
{
  if (!led_id_valid(id))
  {
    return;
  }
  /* Drive the pin high or low per BSP LED state. */
  {
    uint32_t pin_val = on ? (uint32_t)CYBSP_LED_STATE_ON : (uint32_t)CYBSP_LED_STATE_OFF;
    Cy_GPIO_Write(LED_PORT(id), LED_PIN(id), pin_val);
  }
}

/**
 * Toggle one LED. Invalid id is ignored (no-op).
 */
void led_controller_toggle(led_id_t id)
{
  if (!led_id_valid(id))
  {
    return;
  }
  /* Invert current GPIO output level. */
  Cy_GPIO_Inv(LED_PORT(id), LED_PIN(id));
}

/**
 * Read current state of one LED. Returns true if on, false if off or id invalid.
 */
bool led_controller_get(led_id_t id)
{
  if (!led_id_valid(id))
  {
    return false;
  }
  return (CYBSP_LED_STATE_ON == Cy_GPIO_Read(LED_PORT(id), LED_PIN(id)));
}

/**
 * Set all three user LEDs on or off.
 */
void led_controller_set_all(bool on)
{
  led_controller_set(LED_RED, on);
  led_controller_set(LED_GREEN, on);
  led_controller_set(LED_BLUE, on);
}

/**
 * Turn all user LEDs off.
 */
void led_controller_off_all(void)
{
  led_controller_set_all(false);
}
