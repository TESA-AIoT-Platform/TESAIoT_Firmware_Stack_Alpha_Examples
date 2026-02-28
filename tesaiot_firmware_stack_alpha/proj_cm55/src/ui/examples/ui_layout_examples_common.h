#ifndef UI_LAYOUT_EXAMPLES_COMMON_H
#define UI_LAYOUT_EXAMPLES_COMMON_H

#include "lvgl.h"

/* A simple standard layout for every example:
 * - root: full-screen container
 * - header: title + optional subtitle
 * - body: main demo content container
 */
typedef struct {
  lv_obj_t *root;
  lv_obj_t *header;
  lv_obj_t *body;
  lv_obj_t *title_lbl;
  lv_obj_t *subtitle_lbl; /* can be NULL if subtitle is not used */
} ui_ex_common_t;

/* Create the standard example scaffold under `parent`.
 *
 * - Creates root container sized to 100% x 100%.
 * - Adds a header with title (and optional subtitle).
 * - Adds a body container filling remaining space.
 *
 * Parameters:
 * - parent: screen/container to attach to
 * - title: required title text
 * - subtitle: can be NULL
 */
ui_ex_common_t ui_ex_create(lv_obj_t *parent, const char *title,
                            const char *subtitle);

/* Convenience: create a section caption label inside a container */
lv_obj_t *ui_ex_section_title(lv_obj_t *parent, const char *text);

/* Convenience: create a simple labeled button (centered label) */
lv_obj_t *ui_ex_button(lv_obj_t *parent, const char *text, lv_coord_t w,
                       lv_coord_t h);

/* Convenience: create a visible “box” with a centered label (useful for flex
 * grow demos) */
lv_obj_t *ui_ex_box(lv_obj_t *parent, const char *text, lv_coord_t w,
                    lv_coord_t h);

/* Convenience: apply a thin border to an object to visualize bounds
 * (demo/debug) */
void ui_ex_debug_border(lv_obj_t *obj);

#endif /* UI_LAYOUT_EXAMPLES_COMMON_H */
