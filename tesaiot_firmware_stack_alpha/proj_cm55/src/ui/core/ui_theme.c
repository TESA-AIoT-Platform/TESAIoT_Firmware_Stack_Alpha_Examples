#include "ui_theme.h"

void ui_theme_apply_screen(lv_obj_t *obj)
{
  if (NULL == obj)
  {
    return;
  }
  lv_obj_set_style_bg_color(obj, UI_COLOR_BG, 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
}

void ui_theme_apply_header_title(lv_obj_t *obj)
{
  if (NULL == obj)
  {
    return;
  }
  lv_obj_set_style_text_color(obj, UI_COLOR_TEXT, 0);
  lv_obj_set_style_text_font(obj, UI_FONT_TITLE, 0);
}

void ui_theme_apply_card(lv_obj_t *obj)
{
  if (NULL == obj)
  {
    return;
  }
  lv_obj_set_style_bg_color(obj, UI_COLOR_SURFACE, 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(obj, UI_COLOR_BORDER, 0);
  lv_obj_set_style_border_width(obj, 1, 0);
  lv_obj_set_style_radius(obj, 10, 0);
}

void ui_theme_apply_value(lv_obj_t *obj)
{
  if (NULL == obj)
  {
    return;
  }
  lv_obj_set_style_text_color(obj, UI_COLOR_TEXT, 0);
  lv_obj_set_style_text_font(obj, UI_FONT_VALUE, 0);
}

void ui_theme_apply_button_primary(lv_obj_t *obj)
{
  if (NULL == obj)
  {
    return;
  }
  lv_obj_set_style_bg_color(obj, UI_COLOR_PRIMARY, 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
  lv_obj_set_style_text_color(obj, UI_COLOR_BG, 0);
  lv_obj_set_style_radius(obj, 8, 0);
}

void ui_theme_apply_button_secondary(lv_obj_t *obj)
{
  if (NULL == obj)
  {
    return;
  }
  lv_obj_set_style_bg_color(obj, UI_COLOR_SURFACE_ALT, 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
  lv_obj_set_style_text_color(obj, UI_COLOR_TEXT, 0);
  lv_obj_set_style_border_color(obj, UI_COLOR_BORDER, 0);
  lv_obj_set_style_border_width(obj, 1, 0);
  lv_obj_set_style_radius(obj, 8, 0);
}

void ui_theme_apply_textarea(lv_obj_t *obj)
{
  if (NULL == obj)
  {
    return;
  }
  lv_obj_set_style_bg_color(obj, UI_COLOR_SURFACE_ALT, 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
  lv_obj_set_style_text_color(obj, UI_COLOR_TEXT, 0);
  lv_obj_set_style_text_font(obj, UI_FONT_BODY, 0);
  lv_obj_set_style_border_color(obj, UI_COLOR_BORDER, 0);
  lv_obj_set_style_border_width(obj, 1, 0);
  lv_obj_set_style_radius(obj, 8, 0);
}

void ui_theme_apply_log_textarea(lv_obj_t *obj)
{
  if (NULL == obj)
  {
    return;
  }
  ui_theme_apply_textarea(obj);
  lv_obj_set_style_text_color(obj, UI_COLOR_TEXT_MUTED, 0);
}
