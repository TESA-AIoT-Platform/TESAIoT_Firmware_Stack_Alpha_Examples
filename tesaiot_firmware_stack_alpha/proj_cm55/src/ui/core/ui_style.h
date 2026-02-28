#ifndef UI_STYLE_H
#define UI_STYLE_H

#include "lvgl.h"

/* =========================================================================
 * Spacing scale (use everywhere instead of raw numbers)
 * ========================================================================= */

/* Extra small spacing */
#define UI_SPACE_XXS 2
#define UI_SPACE_XS 4

/* Normal spacing */
#define UI_SPACE_S 6
#define UI_SPACE_M 8
#define UI_SPACE_L 12
#define UI_SPACE_XL 16

/* Screen padding */
#define UI_PAD_SCREEN UI_SPACE_L

/* Common gaps */
#define UI_GAP_SMALL UI_SPACE_S
#define UI_GAP_NORMAL UI_SPACE_M
#define UI_GAP_LARGE UI_SPACE_L

/* =========================================================================
 * Radius scale
 * ========================================================================= */

#define UI_RADIUS_NONE 0
#define UI_RADIUS_S 6
#define UI_RADIUS_M 10
#define UI_RADIUS_L 12
#define UI_RADIUS_XL 16

/* Semantic radius */
#define UI_RADIUS_CARD UI_RADIUS_L
#define UI_RADIUS_PILL 999 /* Always round */

/* =========================================================================
 * Standard sizes
 * ========================================================================= */

/* Buttons */
#define UI_BTN_HEIGHT 40
#define UI_BTN_MIN_WIDTH 44

/* Fields / rows */
#define UI_ROW_HEIGHT 44

/* Cards */
#define UI_CARD_MIN_HEIGHT 72

/* Chips / pills */
#define UI_PILL_PAD_H 10
#define UI_PILL_PAD_V 6

/* =========================================================================
 * Opacity helpers
 * ========================================================================= */

#define UI_OPA_DISABLED LV_OPA_40
#define UI_OPA_MUTED LV_OPA_70
#define UI_OPA_OVERLAY LV_OPA_50

/* =========================================================================
 * Z-order / overlay helpers (optional, semantic)
 * ========================================================================= */

#define UI_Z_NORMAL 0
#define UI_Z_OVERLAY 10
#define UI_Z_MODAL 20

/* =========================================================================
 * Helper macros (optional)
 * ========================================================================= */

/* Clamp helper */
#define UI_CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

/* Min / Max */
#ifndef UI_MIN
#define UI_MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef UI_MAX
#define UI_MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#endif /* UI_STYLE_H */
