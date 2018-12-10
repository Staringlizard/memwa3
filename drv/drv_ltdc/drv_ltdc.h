/*
 * memwa2 display driver
 *
 * Copyright (c) 2016 Mathias Edman <mail@dicetec.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 * USA
 *
 */


#ifndef _DRV_LTDC_H
#define _DRV_LTDC_H

#include "stm32h7xx_hal.h"
#include "serv_term.h"

#define DRV_LTDC_ADDR_BUFFER0    0
#define DRV_LTDC_ADDR_BUFFER1    1
#define DRV_LTDC_ADDR_BUFFER2    2

typedef enum
{
    LTDC_MODE_VGA,
    LTDC_MODE_SVGA
} ltdc_mode_t;

typedef enum
{
    LTDC_COLOR_BLACK,
    LTDC_COLOR_WHITE,
    LTDC_COLOR_RED,
    LTDC_COLOR_CYAN,
    LTDC_COLOR_VIOLET,
    LTDC_COLOR_GREEN,
    LTDC_COLOR_BLUE,
    LTDC_COLOR_YELLOW,
    LTDC_COLOR_ORANGE,
    LTDC_COLOR_BROWN,
    LTDC_COLOR_LRED,
    LTDC_COLOR_GREY1,
    LTDC_COLOR_GREY2,
    LTDC_COLOR_LGREEN,
    LTDC_COLOR_LBLUE,
    LTDC_COLOR_GREY3,
    LTDC_COLOR_TEXT_BG,
    LTDC_COLOR_TEXT_FG,
    LTDC_COLOR_MARKER,
} ltdc_color_t;

void drv_ltdc_init(ltdc_mode_t ltdc_mode);
void *drv_ltdc_get_layer(uint8_t layer);
void drv_ltdc_set_memory(uint8_t layer, uint32_t memory);
void drv_ltdc_set_layer(uint8_t layer,
                    uint32_t window_x0,
                    uint32_t window_x1,
                    uint32_t window_y0,
                    uint32_t window_y1,
                    uint32_t buffer_x,
                    uint32_t buffer_y,
                    uint8_t alpha,
                    uint32_t pixel_format);
void drv_ltdc_activate_layer(uint8_t layer);
void drv_ltdc_deactivate_layer(uint8_t layer);
void drv_ltdc_fill_layer(uint8_t layer, uint32_t color);
void drv_ltdc_move_layer(uint8_t layer, uint32_t x, uint32_t y);
void drv_ltdc_flip_buffer(uint8_t **done_buffer_pp);
void drv_ltdc_set_clut_table(uint32_t *clut_p);
void drv_ltdc_enable_clut(uint8_t layer);
void drv_ltdc_disable_clut(uint8_t layer);

#endif
