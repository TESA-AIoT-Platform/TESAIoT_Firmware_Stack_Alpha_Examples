/*******************************************************************************
 * File Name        : screen.c
 *
 * Description      : Implementation of screen utilities
 *
 *******************************************************************************/

#include "screen.h"
#include "lvgl.h"

/*******************************************************************************
 * Function Name: screen_create_dark
 ********************************************************************************
 * Summary:
 *  Create a dark screen with gradient background.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  lv_obj_t*: Pointer to the created screen object
 *
 *******************************************************************************/
lv_obj_t *screen_create_dark(void) {
  /* Create a screen */
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_size(scr, ACTUAL_DISP_HOR_RES, ACTUAL_DISP_VER_RES);
  lv_scr_load(scr);

  /* Set screen background with gradient (dark blue/purple to black) */
  lv_obj_set_style_bg_color(scr, lv_color_hex(0xFF0000), 0);
  lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0x00FF00), 0);
  lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_VER, 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_100, 0);

  return scr;
}

/*******************************************************************************
 * Function Name: screen_create_centered_container
 ********************************************************************************
 * Summary:
 *  Create a centered flex container that centers its children both vertically
 *  and horizontally.
 *
 * Parameters:
 *  parent: Parent object (usually a screen)
 *
 * Return:
 *  lv_obj_t*: Pointer to the created container object
 *
 *******************************************************************************/
lv_obj_t *screen_create_centered_container(lv_obj_t *parent) {
  if (parent == NULL) {
    return NULL;
  }

  /* Create flex layout container to center content */
  lv_obj_t *container = lv_obj_create(parent);
  lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(container, 0, 0);
  lv_obj_set_style_pad_all(container, 0, 0);
  lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_size(container, LV_PCT(100), LV_PCT(100));
  lv_obj_center(container);

  return container;
}

/* [] END OF FILE */
