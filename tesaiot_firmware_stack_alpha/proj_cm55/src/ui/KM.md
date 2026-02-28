# UI Knowledge Management (Q&A)

This document tracks common project-specific issues, solutions, and architectural knowledge.

---

## 🛠 Build & Configuration

### Q: How to add a new directory to the compiler's include path?

**Problem:**  
Compilation fails with `fatal error: ui_layout.h: No such file or directory` when attempting to include headers from a new directory, or developers are forced to use brittle relative paths like `#include "../../core/ui_layout.h"`.

**Goal:**  
Enable headers in a specific folder to be included globally (e.g., `#include "ui_layout.h"`) regardless of the source file's location.

**Solution:**  
In your project `Makefile`, locate the `INCLUDES` variable and append the relative path to the target directory.

**Implementation:**
```makefile
# In proj_cm55/Makefile
INCLUDES += src/ui/core
```

**Note:** Paths should be relative to the `Makefile` location and should NOT include the `-I` prefix (the build system handles that automatically). Note that changing the Makefile requires a rebuild to resolve the error.

---

## 🎨 LVGL UI Development

### Q: Why is `LV_SYMBOL_SEARCH` (or other symbols) causing an "undeclared identifier" error?

**Problem:**  
You attempt to use a standard-sounding icon like `LV_SYMBOL_SEARCH`, but the compiler reports `Use of undeclared identifier`.

**Solution:**  
LVGL's built-in FontAwesome subset only includes a specific selection of icons to save memory. `SEARCH` is not included in the default build.  
- **Check `lv_symbol_def.h`** to see which symbols are available (e.g., `LV_SYMBOL_SETTINGS`, `LV_SYMBOL_OK`, `LV_SYMBOL_LIST`).
- **Use an alternative:** If a specific icon is missing, use a similar available one or import a custom font with the required Unicode glyph.

---

### Q: Why are `ACTUAL_DISP_HOR_RES` and `ACTUAL_DISP_VER_RES` undeclared in my new UI file?

**Problem:**  
Compilation fails when setting screen or container sizes to follow the display resolution.

**Solution:**  
These macros are project-specific and defined in the display porting layer.  
- **Required Include:** `#include "lv_port_disp.h"`
- This header bridges the hardware resolution to your code.

---

### Q: How do I make a container "shrink-wrap" its children?

**Problem:**  
A flex container (row or column) has a fixed height that leaves too much empty space or cuts off children if they grow.

**Solution:**  
Use the `LV_SIZE_CONTENT` constant for height or width. This tells LVGL to calculate the size based on the bounding box of the children plus padding.

**Implementation:**
```c
lv_obj_set_height(container, LV_SIZE_CONTENT);
```
*Note: Ensure the parent has enough space to accommodate the resulting size.*

---

### Q: Why is my scroll container not responding to drag/swipe?

**Problem:**  
You created a scroll container (e.g., using `ui_clean_scroll_container`), but it refuses to scroll when dragged.

**Solution:**  
In LVGL, an object must have the `LV_OBJ_FLAG_CLICKABLE` flag set to receive the touch/pointer events required to calculate drag distances and trigger scrolling.
- **Fix:** Ensure you call `lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE)` on your scroll container.

---
