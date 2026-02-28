/*
What this example demonstrates
    - Fixed-height row containers make alignment visible.
    - MAIN axis: left/center/right placement.
    - CROSS axis: top/middle/bottom placement.
*/

#include "ui_layout_example03_alignment.h"
#include "ui_layout.h"
#include "ui_layout_examples_common.h"

/* Create a demo “box” that shows alignment.
 * Inside: three small boxes labeled A/B/C.
 */
static void make_align_box(lv_obj_t *parent, const char *caption,
                           lv_flex_align_t main_place,
                           lv_flex_align_t cross_place,
                           lv_flex_align_t track_place) {
  /* Caption */
  ui_ex_section_title(parent, caption);

  /* Container: fixed height so alignment is visible */
  lv_obj_t *box = ui_clean_flex_container(parent, LV_FLEX_FLOW_ROW, main_place,
                                          cross_place, track_place, 8, /* gap */
                                          8);                          /* pad */
  lv_obj_set_width(box, LV_PCT(100));
  lv_obj_set_height(box, 80);

  /* show bounds */
  ui_ex_debug_border(box);

  /* Items */
  lv_obj_t *a = ui_ex_box(box, "A", 44, 44);
  lv_obj_t *b = ui_ex_box(box, "B", 44, 44);
  lv_obj_t *c = ui_ex_box(box, "C", 44, 44);

  (void)a;
  (void)b;
  (void)c;
}

void ui_layout_example03_alignment(lv_obj_t *parent) {
  ui_ex_common_t ex =
      ui_ex_create(parent, "Example 03 - Alignment",
                   "Main/Cross/Track alignment in a fixed-height row");

  /* Section 1: MAIN axis differences (row main axis) */
  ui_ex_section_title(ex.body, "MAIN AXIS (ROW): START / CENTER / END");

  make_align_box(ex.body, "Main: START", LV_FLEX_ALIGN_START,
                 LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
  make_align_box(ex.body, "Main: CENTER", LV_FLEX_ALIGN_CENTER,
                 LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
  make_align_box(ex.body, "Main: END", LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER,
                 LV_FLEX_ALIGN_START);

  /* Spacer between sections */
  ui_ex_section_title(ex.body, "CROSS AXIS (ROW): START / CENTER / END");

  /* Section 2: CROSS axis differences */
  make_align_box(ex.body, "Cross: START", LV_FLEX_ALIGN_START,
                 LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  make_align_box(ex.body, "Cross: CENTER", LV_FLEX_ALIGN_START,
                 LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
  make_align_box(ex.body, "Cross: END", LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END,
                 LV_FLEX_ALIGN_START);

  /* Small pass note */
  lv_obj_t *note = lv_label_create(ex.body);
  lv_label_set_text(note, "Pass:\n- MAIN changes horizontal position\n- CROSS "
                          "changes vertical position inside the box");
  lv_obj_set_style_text_opa(note, LV_OPA_80, 0);
}
