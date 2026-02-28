/*******************************************************************************
 * File Name        : tabview.h
 *
 * Description      : Header file for LVGL TabView widget
 *
 *******************************************************************************/

#ifndef TABVIEW_H
#define TABVIEW_H

#include "lv_port_disp.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Macros
 *******************************************************************************/
#define TABVIEW_TOPBAR_HEIGHT (56U)
#define TABVIEW_TAB_BAR_SIZE (48U)
#define TABVIEW_TAB_BUTTON_WIDTH (150U)

/*******************************************************************************
 * Type Definitions
 *******************************************************************************/
typedef struct {
  lv_obj_t *tabview;
  lv_obj_t *tab_general;
  lv_obj_t *tab_advanced;
} tabview_handle_t;

/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/**
 * @brief Create a TabView widget with two tabs (General and Advanced)
 *
 * @param parent Parent object (usually a screen)
 * @return lv_obj_t* Pointer to the created tabview object
 */
lv_obj_t *tabview_create(lv_obj_t *parent);

/**
 * @brief Create a full settings screen with TabView
 *
 * Creates a new screen and places a TabView below a top bar area.
 * The TabView takes up the remaining screen space.
 *
 * @return lv_obj_t* Pointer to the created screen object
 */
lv_obj_t *tabview_screen_create(void);

/**
 * @brief Set the active tab
 *
 * @param tabview TabView object
 * @param tab_id Tab index (0 = General, 1 = Advanced)
 * @param anim_enable Enable animation (LV_ANIM_ON or LV_ANIM_OFF)
 */
void tabview_set_active(lv_obj_t *tabview, uint32_t tab_id,
                        lv_anim_enable_t anim_enable);

/*******************************************************************************
 * Example Functions
 *******************************************************************************/

/**
 * @brief Example 1: Basic TabView in an Existing Screen
 *
 * Creates a TabView on the current active screen.
 */
void tabview_example_1(void);

/**
 * @brief Example 2: Full Screen with TabView
 *
 * Creates a new full screen with TabView.
 */
void tabview_example_2(void);

/**
 * @brief Example 3: Adding Content to Tabs
 *
 * Creates a TabView and adds custom content to the tabs.
 */
void tabview_example_3(void);

/**
 * @brief Example 4: Switching Between Tabs Programmatically
 *
 * Creates a TabView screen and demonstrates programmatic tab switching.
 */
void tabview_example_4(void);

/**
 * @brief Example 5: TabView with Top Bar
 *
 * Creates a screen with a top bar and TabView below it.
 */
void tabview_example_5(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TABVIEW_H */
