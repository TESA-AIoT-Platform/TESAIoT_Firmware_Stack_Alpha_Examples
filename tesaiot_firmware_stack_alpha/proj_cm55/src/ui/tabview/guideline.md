Since you’re on **LVGL 9.2.0** and **1024×600**, this is the “native” way to create a **TabView with exactly two tabs** (and a settings-friendly layout). LVGL 9 uses:

* `lv_tabview_create(parent)`
* `lv_tabview_add_tab(tabview, "Name")`
* `lv_tabview_set_active(tabview, idx, LV_ANIM_...)`
* optional: `lv_tabview_set_tab_bar_position()` / `lv_tabview_set_tab_bar_size()` ([LVGL Docs][1])

## Minimal: TabView with 2 tabs (LVGL 9.2.0)

```c
#include "lvgl.h"

void settings_tabview_create(lv_obj_t * parent)
{
    lv_obj_t * tv = lv_tabview_create(parent);
    lv_obj_set_size(tv, lv_pct(100), lv_pct(100));

    /* Optional but common in settings UIs */
    lv_tabview_set_tab_bar_position(tv, LV_DIR_TOP);
    lv_tabview_set_tab_bar_size(tv, 48);

    /* Add two tabs */
    lv_obj_t * tab_general  = lv_tabview_add_tab(tv, "General");
    lv_obj_t * tab_advanced = lv_tabview_add_tab(tv, "Advanced");

    /* Put content into each tab */
    lv_obj_t * l1 = lv_label_create(tab_general);
    lv_label_set_text(l1, "General settings");
    lv_obj_align(l1, LV_ALIGN_TOP_LEFT, 16, 16);

    lv_obj_t * l2 = lv_label_create(tab_advanced);
    lv_label_set_text(l2, "Advanced settings");
    lv_obj_align(l2, LV_ALIGN_TOP_LEFT, 16, 16);

    /* Start on first tab (0 = General, 1 = Advanced) */
    lv_tabview_set_active(tv, 0, LV_ANIM_OFF);
}
```

## Practical: put TabView *under your Settings Top Bar* (56 px)

If you’re using the top bar we built earlier (height **56**), do this so the tabview doesn’t overlap it:

```c
#define TOPBAR_H 56

void settings_screen_create(void)
{
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_set_size(scr, 1024, 600);
    lv_scr_load(scr);

    /* 1) Create your top bar here (height 56) */
    // settings_topbar_t tb = settings_topbar_create(scr, "Settings", on_back, "Save", on_save);

    /* 2) TabView below it */
    lv_obj_t * tv = lv_tabview_create(scr);
    lv_obj_set_pos(tv, 0, TOPBAR_H);
    lv_obj_set_size(tv, 1024, 600 - TOPBAR_H);

    lv_tabview_set_tab_bar_position(tv, LV_DIR_TOP);
    lv_tabview_set_tab_bar_size(tv, 48);

    lv_obj_t * tab1 = lv_tabview_add_tab(tv, "General");
    lv_obj_t * tab2 = lv_tabview_add_tab(tv, "Advanced");

    /* Make each tab a nice vertical layout */
    lv_obj_set_flex_flow(tab1, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(tab1, 16, 0);
    lv_obj_set_style_row_gap(tab1, 12, 0);

    lv_obj_set_flex_flow(tab2, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(tab2, 16, 0);
    lv_obj_set_style_row_gap(tab2, 12, 0);

    lv_label_create(tab1);
    lv_label_set_text(lv_obj_get_child(tab1, 0), "General settings content...");

    lv_label_create(tab2);
    lv_label_set_text(lv_obj_get_child(tab2, 0), "Advanced settings content...");

    lv_tabview_set_active(tv, 0, LV_ANIM_OFF);
}
```

### Common settings-screen tip

If you place a big scrollable widget (like `lv_list`) inside a tab and you get “weird scrolling”, you usually want **only one scrollable layer** (either the tab content scrolls, or the child list scrolls—avoid both).

If you tell me what you want inside each tab (lists? toggles? sliders?) I’ll wire up a proper “General / Advanced” settings layout with the right scrolling behavior for LVGL 9.2.0.

[1]: https://docs.lvgl.io/9.2/widgets/tabview.html?utm_source=chatgpt.com "Tabview (lv_tabview) — LVGL documentation"
