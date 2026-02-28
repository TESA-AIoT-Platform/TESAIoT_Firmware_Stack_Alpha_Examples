# Known Issues

## Keyboard Widget Slow Repaint

### Issue Description
The LVGL keyboard widget (`lv_keyboard`) repaints the entire keyboard on each button press, causing noticeable delays (approximately 1 second on this hardware).

### Root Cause
This is a known limitation in LVGL's `lv_btnmatrix` widget (which the keyboard uses internally). The widget invalidates and repaints the entire keyboard area on each button press, rather than only repainting the pressed button area.

### Reference
- GitHub Issue: https://github.com/lvgl/lvgl/issues/4295
- Affected Component: `lv_keyboard` (uses `lv_btnmatrix` internally)
- LVGL Version: 9.2.0

### Impact
- Slow visual feedback when typing on the keyboard
- Keys appear "frozen" for approximately 1 second after press
- Particularly noticeable on slower displays or hardware
- Affects user experience for fast typing

### Workarounds
1. **Current Implementation**: Transition time has been set to 1ms to minimize animation delays, but this doesn't solve the underlying repaint issue
2. **Custom Keyboard**: Implement a custom keyboard widget optimized for partial redraws (significant development effort)
3. **Wait for LVGL Fix**: Monitor LVGL updates for a fix to `lv_btnmatrix` partial redraw support

### Technical Details
- Location: `proj_cm55/src/ui/widgets/dropdown.c` - `keyboard_apply_dark_theme()`
- The keyboard widget uses `LV_PART_ITEMS` for button styling
- Transition time is set to 1ms, but full keyboard repaint still occurs
- The issue requires a fix in LVGL core library (`lv_btnmatrix` widget)

### Status
- **Status**: Known limitation, no workaround available in application code
- **Priority**: Medium (affects user experience but keyboard is functional)
- **Dependencies**: Requires fix in LVGL core library

## Keyboard Double Character Input

### Issue Description
When pressing a key on the keyboard widget, the character appears twice in the textarea (e.g., pressing "A" results in "AA").

### Root Cause Analysis
Based on investigation, the following potential causes have been ruled out:
1. **Not inserting text twice manually** - No custom event handlers call `lv_textarea_add_char()` or `lv_textarea_add_text()`
2. **Not registering input device twice** - Only one input device (touchpad/pointer) is registered in `lv_port_indev_init()`
3. **Touchpad read callback** - The `touchpad_read()` function correctly reports PRESSED/RELEASED states

### Likely Cause
The issue appears to be related to how LVGL's `lv_buttonmatrix` widget (used internally by the keyboard) handles `LV_EVENT_VALUE_CHANGED` events with pointer/touchpad input. The buttonmatrix may be sending VALUE_CHANGED events on both press and release, and the keyboard's default event handler (`lv_keyboard_def_event_cb`) attempts to filter release events by checking for `LV_BUTTONMATRIX_BUTTON_NONE`, but this filtering may not be working correctly in all cases.

### Reference
- Related to keyboard widget event handling
- LVGL Version: 9.2.0
- Input Device Type: Pointer (touchpad), not keypad

### Impact
- Characters appear twice in textarea when typing
- Affects user experience and typing speed

### Potential Workarounds
1. **Custom Keyboard Implementation**: Create a custom keyboard widget with event filtering
2. **Monitor LVGL Updates**: Check for fixes in future LVGL versions related to buttonmatrix event handling
3. **Event Handler Override**: Attempt to override keyboard event handler (complex, may break functionality)

### Status
- **Status**: Under investigation, suspected LVGL buttonmatrix widget behavior
- **Priority**: High (affects basic functionality)
- **Note**: This issue was reported as "solved" earlier but appears to persist
