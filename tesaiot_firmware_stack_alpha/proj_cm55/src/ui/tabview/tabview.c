/*******************************************************************************
 * File Name        : tabview.c
 *
 * Description      : Implementation of LVGL TabView widget
 *
 *******************************************************************************/

#include "tabview.h"
#include "lvgl.h"
#include <stdio.h>

/*******************************************************************************
 * Function Name: tabview_create
 ********************************************************************************
 * Summary:
 *  Create a TabView widget with two tabs (General and Advanced).
 *  The TabView takes up 100% of the parent size.
 *
 * Parameters:
 *  parent: Parent object (usually a screen or container)
 *
 * Return:
 *  lv_obj_t*: Pointer to the created tabview object
 *
 *******************************************************************************/
lv_obj_t *tabview_create(lv_obj_t *parent) {
  if (parent == NULL) {
    return NULL;
  }

  /* Create the TabView */
  lv_obj_t *tv = lv_tabview_create(parent);
  if (tv == NULL) {
    return NULL;
  }

  /* Set size to 100% of parent */
  lv_obj_set_size(tv, lv_pct(100), lv_pct(100));

  /* Configure tab bar position and size */
  lv_tabview_set_tab_bar_position(tv, LV_DIR_TOP);
  lv_tabview_set_tab_bar_size(tv, TABVIEW_TAB_BAR_SIZE);

  /* Add two tabs */
  lv_obj_t *tab_general = lv_tabview_add_tab(tv, "General");
  lv_obj_t *tab_advanced = lv_tabview_add_tab(tv, "Advanced");

  if (tab_general == NULL || tab_advanced == NULL) {
    lv_obj_del(tv);
    return NULL;
  }

  /* Set fixed width for tab buttons */
  lv_obj_t *tab_bar = lv_tabview_get_tab_bar(tv);
  uint32_t child_cnt = lv_obj_get_child_cnt(tab_bar);

  for (uint32_t i = 0; i < child_cnt; i++) {
    lv_obj_t *btn = lv_obj_get_child(tab_bar, i);
    lv_obj_set_width(btn, TABVIEW_TAB_BUTTON_WIDTH);
    /* Prevent stretching by disabling flex grow */
    lv_obj_set_style_flex_grow(btn, 0, 0);
  }

  /* Configure General tab layout */
  lv_obj_set_flex_flow(tab_general, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(tab_general, 16, 0);
  lv_obj_set_style_pad_row(tab_general, 12, 0);

  /* Configure Advanced tab layout */
  lv_obj_set_flex_flow(tab_advanced, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(tab_advanced, 16, 0);
  lv_obj_set_style_pad_row(tab_advanced, 12, 0);

  /* Add default content to General tab */
  lv_obj_t *label1 = lv_label_create(tab_general);
  lv_label_set_text(label1, "General settings content...");

  /* Add default content to Advanced tab */
  lv_obj_t *label2 = lv_label_create(tab_advanced);
  lv_label_set_text(label2, "Advanced settings content...");

  /* Start on first tab (0 = General) */
  lv_tabview_set_active(tv, 0, LV_ANIM_OFF);

  return tv;
}

/*******************************************************************************
 * Function Name: tabview_screen_create
 ********************************************************************************
 * Summary:
 *  Create a full screen with TabView. The TabView is positioned below
 *  a top bar area (if needed) and takes up the remaining screen space.
 *  Screen size matches the display resolution (1024x600 for 7" display).
 *
 * Parameters:
 *  void
 *
 * Return:
 *  lv_obj_t*: Pointer to the created screen object
 *
 *******************************************************************************/
lv_obj_t *tabview_screen_create(void) {
  /* Create a new screen */
  lv_obj_t *scr = lv_obj_create(NULL);
  if (scr == NULL) {
    return NULL;
  }

  /* Set screen size to match display resolution */
  lv_obj_set_size(scr, ACTUAL_DISP_HOR_RES, ACTUAL_DISP_VER_RES);

  /* Load the screen */
  lv_scr_load(scr);

  /* Create TabView */
  lv_obj_t *tv = lv_tabview_create(scr);
  if (tv == NULL) {
    lv_obj_del(scr);
    return NULL;
  }

  /* Position TabView below top bar (if top bar exists, use TOPBAR_H, otherwise
   * start at 0) */
  lv_obj_set_pos(tv, 0, 0);
  lv_obj_set_size(tv, ACTUAL_DISP_HOR_RES, ACTUAL_DISP_VER_RES);

  /* Configure tab bar position and size */
  lv_tabview_set_tab_bar_position(tv, LV_DIR_TOP);
  lv_tabview_set_tab_bar_size(tv, TABVIEW_TAB_BAR_SIZE);

  /* Add two tabs */
  lv_obj_t *tab1 = lv_tabview_add_tab(tv, "General");
  lv_obj_t *tab2 = lv_tabview_add_tab(tv, "Advanced");

  if (tab1 == NULL || tab2 == NULL) {
    lv_obj_del(scr);
    return NULL;
  }

  /* Set fixed width for tab buttons */
  lv_obj_t *tab_bar = lv_tabview_get_tab_bar(tv);
  uint32_t child_cnt = lv_obj_get_child_cnt(tab_bar);

  for (uint32_t i = 0; i < child_cnt; i++) {
    lv_obj_t *btn = lv_obj_get_child(tab_bar, i);
    lv_obj_set_width(btn, TABVIEW_TAB_BUTTON_WIDTH);
    /* Prevent stretching by disabling flex grow */
    lv_obj_set_style_flex_grow(btn, 0, 0);
  }

  /* Configure General tab layout */
  lv_obj_set_flex_flow(tab1, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(tab1, 16, 0);
  lv_obj_set_style_pad_row(tab1, 12, 0);

  /* Configure Advanced tab layout */
  lv_obj_set_flex_flow(tab2, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(tab2, 16, 0);
  lv_obj_set_style_pad_row(tab2, 12, 0);

  /* Add default content to General tab */
  lv_obj_t *label1 = lv_label_create(tab1);
  lv_label_set_text(label1, "General settings content...");

  /* Add default content to Advanced tab */
  lv_obj_t *label2 = lv_label_create(tab2);
  lv_label_set_text(label2, "Advanced settings content...");

  /* Start on first tab (0 = General) */
  lv_tabview_set_active(tv, 0, LV_ANIM_OFF);

  return scr;
}

/*******************************************************************************
 * Function Name: tabview_set_active
 ********************************************************************************
 * Summary:
 *  Set the active tab in a TabView
 *
 * Parameters:
 *  tabview: TabView object
 *  tab_id: Tab index (0 = General, 1 = Advanced)
 *  anim_enable: Enable animation (LV_ANIM_ON or LV_ANIM_OFF)
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void tabview_set_active(lv_obj_t *tabview, uint32_t tab_id,
                        lv_anim_enable_t anim_enable) {
  if (tabview == NULL) {
    return;
  }

  lv_tabview_set_active(tabview, tab_id, anim_enable);
}

/*******************************************************************************
 * Example Functions
 *******************************************************************************/

/*******************************************************************************
 * Function Name: tabview_example_1
 ********************************************************************************
 * Summary:
 *  Example 1: Basic TabView in an Existing Screen
 *  Creates a screen and adds a TabView to it.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void tabview_example_1(void) {
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

/*******************************************************************************
 * Function Name: tabview_example_2
 ********************************************************************************
 * Summary:
 *  Example 2: Full Screen with TabView
 *  Creates a new full screen with TabView.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void tabview_example_2(void) {
  /* Create a full screen with TabView */
  lv_obj_t *screen = tabview_screen_create();
  if (screen != NULL) {
    /* Screen is already loaded and TabView is configured */
    /* Add your custom widgets to the tabs */
  }
}

/*******************************************************************************
 * Function Name: tabview_example_3
 ********************************************************************************
 * Summary:
 *  Example 3: Adding Content to Tabs
 *  Creates a TabView and adds custom content to the tabs.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void tabview_example_3(void) {
  /* Create a screen */
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_size(scr, ACTUAL_DISP_HOR_RES, ACTUAL_DISP_VER_RES);
  lv_scr_load(scr);

  /* Create TabView */
  lv_obj_t *tv = tabview_create(scr);

  if (tv != NULL) {
    /* Get tab content areas by storing references during creation */
    /* For demonstration, this shows the basic TabView setup */
    /* Note: In practice, you should store tab references when creating them */
    /* Direct tab access via lv_obj_get_child() is complex */
    /* This example shows the basic setup, customize as needed */
  }
}

/*******************************************************************************
 * Function Name: tabview_example_4
 ********************************************************************************
 * Summary:
 *  Example 4: Switching Between Tabs Programmatically
 *  Creates a TabView screen and demonstrates programmatic tab switching.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void tabview_example_4(void) {
  /* Create a full screen with TabView */
  lv_obj_t *screen = tabview_screen_create();

  if (screen != NULL) {
    /* Get the TabView from the screen */
    /* The TabView is typically the first child of the screen */
    lv_obj_t *tv = lv_obj_get_child(screen, 0);

    if (tv != NULL) {
      /* Initially start on General tab (already set by tabview_screen_create)
       */
      /* Switch to Advanced tab with animation */
      tabview_set_active(tv, 1, LV_ANIM_ON);

      /* Later, you can switch back to General tab */
      /* tabview_set_active(tv, 0, LV_ANIM_OFF); */
    }
  }
}

/*******************************************************************************
 * Function Name: tabview_example_5
 ********************************************************************************
 * Summary:
 *  Example 5: TabView with Top Bar
 *  Creates a screen with a top bar and TabView below it.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void tabview_example_5(void) {
  /* Create a screen */
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_size(scr, ACTUAL_DISP_HOR_RES, ACTUAL_DISP_VER_RES);
  lv_scr_load(scr);

  /* Create a simple top bar (height: TABVIEW_TOPBAR_HEIGHT) */
  lv_obj_t *topbar = lv_obj_create(scr);
  lv_obj_set_size(topbar, ACTUAL_DISP_HOR_RES, TABVIEW_TOPBAR_HEIGHT);
  lv_obj_set_pos(topbar, 0, 0);
  lv_obj_set_style_bg_color(topbar, lv_color_hex(0x0066CC), 0);
  lv_obj_set_style_border_width(topbar, 0, 0);

  /* Add a label to the top bar */
  lv_obj_t *topbar_label = lv_label_create(topbar);
  lv_label_set_text(topbar_label, "Settings");
  lv_obj_set_style_text_color(topbar_label, lv_color_white(), 0);
  lv_obj_center(topbar_label);

  /* Create TabView below the top bar */
  lv_obj_t *tv = lv_tabview_create(scr);
  lv_obj_set_pos(tv, 0, TABVIEW_TOPBAR_HEIGHT);
  lv_obj_set_size(tv, ACTUAL_DISP_HOR_RES,
                  ACTUAL_DISP_VER_RES - TABVIEW_TOPBAR_HEIGHT);

  /* Configure tab bar */
  lv_tabview_set_tab_bar_position(tv, LV_DIR_TOP);
  lv_tabview_set_tab_bar_size(tv, TABVIEW_TAB_BAR_SIZE);

  /* Add tabs */
  lv_obj_t *tab1 = lv_tabview_add_tab(tv, "General");
  lv_obj_t *tab2 = lv_tabview_add_tab(tv, "Advanced");

  if (tab1 != NULL && tab2 != NULL) {
    /* Set fixed width for tab buttons */
    lv_obj_t *tab_bar = lv_tabview_get_tab_bar(tv);
    uint32_t child_cnt = lv_obj_get_child_cnt(tab_bar);

    for (uint32_t i = 0; i < child_cnt; i++) {
      lv_obj_t *btn = lv_obj_get_child(tab_bar, i);
      lv_obj_set_width(btn, TABVIEW_TAB_BUTTON_WIDTH);
      /* Prevent stretching by disabling flex grow */
      lv_obj_set_style_flex_grow(btn, 0, 0);
    }

    /* Configure General tab layout */
    lv_obj_set_flex_flow(tab1, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(tab1, 16, 0);
    lv_obj_set_style_pad_row(tab1, 12, 0);

    /* Configure Advanced tab layout */
    lv_obj_set_flex_flow(tab2, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(tab2, 16, 0);
    lv_obj_set_style_pad_row(tab2, 12, 0);

    /* Add content to General tab */
    lv_obj_t *label1 = lv_label_create(tab1);
    lv_label_set_text(label1, "General settings content...");

    /* Add content to Advanced tab */
    lv_obj_t *label2 = lv_label_create(tab2);
    lv_label_set_text(label2, "Advanced settings content...");

    /* Start on first tab (0 = General) */
    lv_tabview_set_active(tv, 0, LV_ANIM_OFF);
  }
}

/* [] END OF FILE */
