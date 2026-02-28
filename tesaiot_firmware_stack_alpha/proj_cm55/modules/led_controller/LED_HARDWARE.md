# LED controller hardware

Hardware mapping and behavior of the `led_controller` module (CM55 user LEDs).

## Pin mapping

| GPIO        | BSP alias (when defined)     | led_id_t   | Color  |
|------------|------------------------------|------------|--------|
| P16[5]     | CYBSP_USER_LED3, CYBSP_LED_BLUE  | LED_BLUE   | Blue   |
| P16[6]     | CYBSP_USER_LED2, CYBSP_LED_GREEN | LED_GREEN  | Green  |
| P16[7]     | CYBSP_USER_LED1, CYBSP_USER_LED, CYBSP_LED_RED | LED_RED    | Red    |

Port is `GPIO_PRT16` when using the P16 fallback (see below). The BSP may use the same port or a different one per LED depending on the board configuration.

## Build-time configuration

The module chooses port/pin at compile time:

1. **LED_USE_BSP_ALL** – If `CYBSP_USER_LED1_PORT`, `CYBSP_USER_LED2_PORT`, and `CYBSP_USER_LED3_PORT` are all defined, each LED uses its BSP port and pin (`CYBSP_USER_LEDn_PORT` / `CYBSP_USER_LEDn_PIN`).
2. **LED_USE_BSP_RED** – If only `CYBSP_USER_LED_PORT` is defined, the red LED uses that BSP port/pin; green and blue use `GPIO_PRT16` with pins 6 and 5 respectively.
3. **P16 fallback** – Otherwise all three LEDs use `GPIO_PRT16`: red = pin 7, green = pin 6, blue = pin 5.

## Polarity

On/off levels follow the BSP: `CYBSP_LED_STATE_ON` and `CYBSP_LED_STATE_OFF` (from `cybsp_types.h`). The default BSP uses 1 for on and 0 for off.

## Dependencies

- **cybsp_init()** – Must be called before using the LED controller so GPIO is configured.
- **cybsp.h**, **cy_pdl.h** – For port/pin defines and `Cy_GPIO_*` APIs.

## Invalid LED id

Only `led_id_t` values in `[0, LED_COUNT)` (i.e. LED_RED, LED_GREEN, LED_BLUE) are valid. For an invalid id: `led_controller_set` and `led_controller_toggle` do nothing; `led_controller_get` returns `false`.
