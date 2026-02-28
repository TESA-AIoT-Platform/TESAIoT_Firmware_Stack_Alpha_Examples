/*******************************************************************************
 * File Name        : screen.h
 *
 * Description      : Header file for screen utilities
 *
 *******************************************************************************/

#ifndef SCREEN_H
#define SCREEN_H

#include "lv_port_disp.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/**
 * @brief Create a dark screen with gradient background
 *
 * @return lv_obj_t* Pointer to the created screen object
 */
lv_obj_t *screen_create_dark(void);

/**
 * @brief Create a centered flex container
 *
 * Creates a transparent flex container that centers its children both
 * vertically and horizontally.
 *
 * @param parent Parent object (usually a screen)
 * @return lv_obj_t* Pointer to the created container object
 */
lv_obj_t *screen_create_centered_container(lv_obj_t *parent);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SCREEN_H */
