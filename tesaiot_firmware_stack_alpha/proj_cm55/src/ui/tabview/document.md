# TabView Widget Documentation

## Overview

This module provides a high-level interface for creating and managing TabView widgets using LVGL 9.2.0. The TabView widget allows you to organize content into multiple tabs, making it ideal for settings screens and other applications that need to organize related content into separate sections.

The implementation creates a TabView with two tabs: **General** and **Advanced**, with a tab bar positioned at the top. Each tab is configured with a flexible column layout for easy content organization.

## Features

- **Two-tab layout**: Pre-configured with "General" and "Advanced" tabs
- **Flexible sizing**: Supports both parent-relative sizing and full-screen creation
- **Customizable layout**: Each tab uses flex column layout with padding and gap styling
- **Top-positioned tab bar**: Tab bar is positioned at the top with customizable height
- **Animation support**: Optional animations when switching between tabs
- **LVGL 9.2.0 compatible**: Uses native LVGL 9.2.0 APIs

## Requirements

- LVGL version 9.2.0 or later
- Display resolution: 1024 x 600 pixels (7-inch display)
- Header file: `lv_port_disp.h` (for display resolution constants)

## API Reference

### Macros

#### `TABVIEW_TOPBAR_HEIGHT`
```c
#define TABVIEW_TOPBAR_HEIGHT (56U)
```
Height of the top bar area (if used). Default: 56 pixels.

#### `TABVIEW_TAB_BAR_SIZE`
```c
#define TABVIEW_TAB_BAR_SIZE (48U)
```
Height of the tab bar. Default: 48 pixels.

#### `TABVIEW_TAB_BUTTON_WIDTH`
```c
#define TABVIEW_TAB_BUTTON_WIDTH (150U)
```
Fixed width of each tab button in pixels. Default: 150 pixels. This prevents tab buttons from stretching to fill available space.

### Type Definitions

#### `tabview_handle_t`
```c
typedef struct {
  lv_obj_t *tabview;
  lv_obj_t *tab_general;
  lv_obj_t *tab_advanced;
} tabview_handle_t;
```
Structure to hold references to the TabView and its tabs. This can be used to store tab references for later access.

### Functions

#### `tabview_create()`
```c
lv_obj_t *tabview_create(lv_obj_t *parent);
```

Creates a TabView widget with two tabs (General and Advanced) that takes up 100% of the parent size.

**Parameters:**
- `parent`: Parent object (usually a screen or container). Must not be NULL.

**Returns:**
- Pointer to the created TabView object on success
- `NULL` on failure (invalid parent or creation failure)

**Example:**
```c
lv_obj_t *screen = lv_obj_create(NULL);
lv_obj_t *tv = tabview_create(screen);
```

#### `tabview_screen_create()`
```c
lv_obj_t *tabview_screen_create(void);
```

Creates a full screen with TabView. The screen size matches the display resolution (1024x600 for 7-inch display). The TabView fills the entire screen, and the screen is automatically loaded.

**Parameters:**
- None

**Returns:**
- Pointer to the created screen object on success
- `NULL` on failure

**Example:**
```c
lv_obj_t *screen = tabview_screen_create();
// Screen is already loaded, no need to call lv_scr_load()
```

#### `tabview_set_active()`
```c
void tabview_set_active(lv_obj_t *tabview, uint32_t tab_id, lv_anim_enable_t anim_enable);
```

Sets the active tab in a TabView widget.

**Parameters:**
- `tabview`: TabView object. Must not be NULL.
- `tab_id`: Tab index (0 = General, 1 = Advanced)
- `anim_enable`: Enable animation when switching tabs (`LV_ANIM_ON` or `LV_ANIM_OFF`)

**Returns:**
- None

**Example:**
```c
// Switch to Advanced tab with animation
tabview_set_active(tv, 1, LV_ANIM_ON);

// Switch to General tab without animation
tabview_set_active(tv, 0, LV_ANIM_OFF);
```

### Example Functions

The module provides five example functions that demonstrate different usage patterns. These functions can be called directly from your application code (e.g., from `main.c`).

#### `tabview_example_1()`
```c
void tabview_example_1(void);
```

Basic TabView in an existing screen. Creates a screen and adds a TabView to it.

**Usage:**
```c
#include "ui/tabview/tabview.h"
tabview_example_1();
```

#### `tabview_example_2()`
```c
void tabview_example_2(void);
```

Full screen with TabView. Creates a new full screen using `tabview_screen_create()`.

**Usage:**
```c
#include "ui/tabview/tabview.h"
tabview_example_2();
```

#### `tabview_example_3()`
```c
void tabview_example_3(void);
```

Adding content to tabs. Demonstrates creating a TabView and adding custom content.

**Usage:**
```c
#include "ui/tabview/tabview.h"
tabview_example_3();
```

#### `tabview_example_4()`
```c
void tabview_example_4(void);
```

Switching between tabs programmatically. Creates a TabView screen and demonstrates programmatic tab switching with animation.

**Usage:**
```c
#include "ui/tabview/tabview.h"
tabview_example_4();
```

#### `tabview_example_5()`
```c
void tabview_example_5(void);
```

TabView with top bar. Creates a screen with a top bar and TabView positioned below it.

**Usage:**
```c
#include "ui/tabview/tabview.h"
tabview_example_5();
```

## Usage Examples

The following examples show how to use the TabView functions in your code. Note that ready-to-use example functions are also available (see [Example Functions](#example-functions) above) which can be called directly from `main.c`.

### Quick Start: Using Example Functions

The easiest way to get started is to use the provided example functions. Simply include the header and call the desired example:

```c
#include "ui/tabview/tabview.h"

/* In your main.c or initialization code */
tabview_example_1();  // Basic TabView in an existing screen
// or
tabview_example_2();  // Full screen with TabView
// or
tabview_example_3();  // Adding content to tabs
// or
tabview_example_4();  // Switching tabs programmatically
// or
tabview_example_5();  // TabView with top bar
```

### Example 1: Basic TabView in an Existing Screen

**Using the example function:**
```c
#include "ui/tabview/tabview.h"
tabview_example_1();
```

**Manual implementation:**
```c
#include "ui/tabview/tabview.h"
#include "lvgl.h"

void my_screen_init(void) {
    /* Create a screen */
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_size(scr, ACTUAL_DISP_HOR_RES, ACTUAL_DISP_VER_RES);
    lv_scr_load(scr);

    /* Create TabView */
    lv_obj_t *tv = tabview_create(scr);
    if (tv != NULL) {
        /* TabView is ready to use */
        /* You can now add custom content to the tabs */
    }
}
```

### Example 2: Full Screen with TabView

**Using the example function:**
```c
#include "ui/tabview/tabview.h"
tabview_example_2();
```

**Manual implementation:**
```c
#include "ui/tabview/tabview.h"

void create_settings_screen(void) {
    /* Create a full screen with TabView */
    lv_obj_t *screen = tabview_screen_create();
    if (screen != NULL) {
        /* Screen is already loaded and TabView is configured */
        /* Add your custom widgets to the tabs */
    }
}
```

### Example 3: Adding Content to Tabs

**Using the example function:**
```c
#include "ui/tabview/tabview.h"
tabview_example_3();
```

**Manual implementation:**
```c
#include "ui/tabview/tabview.h"
#include "lvgl.h"

void add_content_to_tabs(void) {
    lv_obj_t *screen = lv_scr_act();
    lv_obj_t *tv = tabview_create(screen);

    if (tv != NULL) {
        /* Get the tab content areas */
        /* Note: Store tab pointers when creating or access via lv_obj_get_child() */
        
        /* For now, create content directly in the tabview */
        /* You would typically store tab references when creating them */
    }
}
```

### Example 4: Switching Between Tabs Programmatically

**Using the example function:**
```c
#include "ui/tabview/tabview.h"
tabview_example_4();
```

**Manual implementation:**
```c
#include "ui/tabview/tabview.h"

void switch_tabs_example(void) {
    lv_obj_t *screen = tabview_screen_create();
    
    if (screen != NULL) {
        /* Get the TabView from the screen */
        lv_obj_t *tv = lv_obj_get_child(screen, 0);
        
        /* Switch to Advanced tab with animation */
        tabview_set_active(tv, 1, LV_ANIM_ON);
        
        /* Later, switch back to General tab */
        tabview_set_active(tv, 0, LV_ANIM_OFF);
    }
}
```

### Example 5: TabView with Top Bar

**Using the example function:**
```c
#include "ui/tabview/tabview.h"
tabview_example_5();
```

**Manual implementation:**

If you need to place a TabView below a top bar, you can position it manually:

```c
#include "ui/tabview/tabview.h"
#include "lvgl.h"

#define TOPBAR_HEIGHT 56

void create_screen_with_topbar(void) {
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_size(scr, ACTUAL_DISP_HOR_RES, ACTUAL_DISP_VER_RES);
    lv_scr_load(scr);

    /* Create your top bar here (height: TOPBAR_HEIGHT) */
    /* ... top bar code ... */

    /* Create TabView below the top bar */
    lv_obj_t *tv = lv_tabview_create(scr);
    lv_obj_set_pos(tv, 0, TOPBAR_HEIGHT);
    lv_obj_set_size(tv, ACTUAL_DISP_HOR_RES, ACTUAL_DISP_VER_RES - TOPBAR_HEIGHT);
    
    /* Configure tab bar */
    lv_tabview_set_tab_bar_position(tv, LV_DIR_TOP);
    lv_tabview_set_tab_bar_size(tv, TABVIEW_TAB_BAR_SIZE);
    
    /* Add tabs */
    lv_obj_t *tab1 = lv_tabview_add_tab(tv, "General");
    lv_obj_t *tab2 = lv_tabview_add_tab(tv, "Advanced");
    
    /* Configure tabs and add content */
    /* ... */
}
```

## Configuration

### Tab Bar Settings

The TabView is configured with the following default settings:

- **Tab bar position**: Top (`LV_DIR_TOP`)
- **Tab bar size**: 48 pixels (`TABVIEW_TAB_BAR_SIZE`)
- **Tab button width**: 150 pixels (`TABVIEW_TAB_BUTTON_WIDTH`) - Fixed width, buttons don't stretch
- **Initial tab**: General (index 0)
- **Animation**: Disabled on initial creation

### Tab Layout Settings

Each tab is configured with:

- **Layout**: Flex column (`LV_FLEX_FLOW_COLUMN`)
- **Padding**: 16 pixels on all sides
- **Row gap**: 12 pixels between items

### Display Resolution

The TabView uses the display resolution constants from `lv_port_disp.h`:

- **Horizontal resolution**: `ACTUAL_DISP_HOR_RES` (1024 pixels for 7" display)
- **Vertical resolution**: `ACTUAL_DISP_VER_RES` (600 pixels for 7" display)

To use a different display resolution, modify the constants in `lv_port_disp.h` or adjust the screen size manually.

## Customization

### Modifying Tab Content

The default implementation adds placeholder labels to each tab. You can customize the content by:

1. Storing tab references when creating them
2. Adding your own widgets to the tabs
3. Removing or modifying the default content

### Changing Tab Bar Appearance

After creating the TabView, you can modify the tab bar appearance using LVGL APIs:

```c
lv_obj_t *tv = tabview_create(parent);

/* Modify tab bar style */
lv_obj_t *tab_bar = lv_tabview_get_tab_bar(tv);
lv_obj_set_style_bg_color(tab_bar, lv_color_hex(0x0066CC), 0);
lv_obj_set_style_text_color(tab_bar, lv_color_white(), 0);
```

### Tab Button Width

The TabView automatically sets a fixed width for tab buttons to prevent them from stretching. By default, each button has a width of **150 pixels** (defined by `TABVIEW_TAB_BUTTON_WIDTH`).

To change the button width, modify the macro in `tabview.h`:

```c
#define TABVIEW_TAB_BUTTON_WIDTH (200U)  // Change from 150 to 200 pixels
```

The implementation automatically:
- Sets fixed width on each button: `lv_obj_set_width(btn, TABVIEW_TAB_BUTTON_WIDTH)`
- Disables flex grow to prevent stretching: `lv_obj_set_style_flex_grow(btn, 0, 0)`

This ensures tab buttons maintain their fixed width and leave empty space on the right side of the tab bar if the screen is wider than the combined button widths.

### Adding More Tabs

To add more tabs, you can access the TabView and use LVGL's `lv_tabview_add_tab()` function:

```c
lv_obj_t *tv = tabview_create(parent);
lv_obj_t *tab3 = lv_tabview_add_tab(tv, "Settings");
```

## Notes and Best Practices

### Scrolling Behavior

When placing scrollable widgets (like `lv_list`) inside a tab, be aware that you typically want **only one scrollable layer**. Either the tab content should scroll, or the child list should scroll—avoid having both scrollable simultaneously.

### Memory Management

- TabView objects are automatically managed by LVGL's object system
- When you delete a parent object, child objects (including TabView) are automatically deleted
- Always check return values for NULL when creating objects

### Performance Considerations

- Tab switching with animation (`LV_ANIM_ON`) provides smoother UX but uses more CPU
- For faster response, use `LV_ANIM_OFF` when programmatically switching tabs
- Large amounts of content in tabs may require scrollable containers

### Tab Access Pattern

Since the current implementation doesn't provide helper functions to retrieve tabs, consider:

1. Storing tab references in your own data structure when creating them
2. Using `lv_obj_get_child()` to access tab content (requires knowledge of object hierarchy)
3. Extending the module to return tab references in a structure

## Integration with Other Modules

This TabView module is designed to work with:

- **Display driver**: Uses `lv_port_disp.h` for resolution constants
- **LVGL core**: Requires LVGL 9.2.0
- **Touch input**: Works with touch events handled by LVGL's input device system

## Calling from main.c

The example functions can be called directly from `main.c` after including the header:

```c
#include "ui/tabview/tabview.h"

/* In your initialization code (e.g., after LVGL initialization) */
tabview_example_1();  // Call any of the example functions
```

Example integration in `main.c`:

```c
#include "ui/tabview/tabview.h"

/* After LVGL and display initialization */
if (vg_lite_init_success) {
    /* Run the Music demo */
    // lv_demo_music();

    /* Create the TabView */
    tabview_example_1();  // or tabview_example_2(), etc.
}
```

## See Also

- [LVGL TabView Documentation](https://docs.lvgl.io/9.2/widgets/tabview.html)
- `guideline.md` - Implementation guidelines and examples
- `tabview.h` - Header file with function prototypes and example function declarations
- `tabview.c` - Implementation source code with all example functions
