#ifndef UI_THEME_H
#define UI_THEME_H

#include "lvgl.h"

#define UI_COLOR_BG lv_color_hex(0x0F172A)
#define UI_COLOR_SURFACE lv_color_hex(0x1E293B)
#define UI_COLOR_SURFACE_ALT lv_color_hex(0x111827)
#define UI_COLOR_BORDER lv_color_hex(0x334155)
#define UI_COLOR_TEXT lv_color_white()
#define UI_COLOR_TEXT_MUTED lv_color_hex(0x94A3B8)
#define UI_COLOR_PRIMARY lv_color_hex(0x38BDF8)
#define UI_COLOR_SUCCESS lv_color_hex(0x22C55E)
#define UI_COLOR_WARNING lv_color_hex(0xF59E0B)
#define UI_COLOR_DANGER lv_color_hex(0xEF4444)

#define UI_FONT_TITLE (&lv_font_montserrat_24)
#define UI_FONT_SECTION (&lv_font_montserrat_16)
#define UI_FONT_BODY (&lv_font_montserrat_14)
#define UI_FONT_VALUE (&lv_font_montserrat_16)

void ui_theme_apply_screen(lv_obj_t *obj);
void ui_theme_apply_header_title(lv_obj_t *obj);
void ui_theme_apply_card(lv_obj_t *obj);
void ui_theme_apply_value(lv_obj_t *obj);
void ui_theme_apply_button_primary(lv_obj_t *obj);
void ui_theme_apply_button_secondary(lv_obj_t *obj);
void ui_theme_apply_textarea(lv_obj_t *obj);
void ui_theme_apply_log_textarea(lv_obj_t *obj);

#endif
