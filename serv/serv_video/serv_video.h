/*
 * Video
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


#ifndef _SERV_VIDEO_H
#define _SERV_VIDEO_H

#include "stm32h7xx_hal.h"
#include "serv_term.h"
#include <stdio.h>

#define SERV_VIDEO_SCREEN_WIDTH            800
#define SERV_VIDEO_SCREEN_HEIGHT           600
#define SERV_VIDEO_FONT_HEIGHT             8
#define SERV_VIDEO_FONT_WIDTH              5
#define SERV_VIDEO_ROW_HEIGHT              (SERV_VIDEO_FONT_HEIGHT + 2)
#define SERV_VIDEO_LAYER_EMU               0
#define SERV_VIDEO_LAYER_MISC              1
#define SERV_VIDEO_MISC_WIDTH              SERV_VIDEO_SCREEN_WIDTH
#define SERV_VIDEO_MISC_HEIGHT             SERV_VIDEO_SCREEN_HEIGHT
#define SERV_VIDEO_MISC_ROWS               (SERV_VIDEO_SCREEN_HEIGHT/SERV_VIDEO_ROW_HEIGHT)

typedef enum
{
	SERV_VIDEO_FADE_UP,
	SERV_VIDEO_FADE_DOWN
} serv_video_fade_t;

typedef void (*fade_complete_cb_t)(uint8_t layer, serv_video_fade_t fade);

void serv_video_init();
void serv_video_set_layer_size(uint8_t layer, uint32_t width, uint32_t height);
void serv_video_en();
void serv_video_flip_buffer(uint8_t **buffer_pp);
void serv_video_fade(uint8_t layer, serv_video_fade_t fade);
void serv_video_reg_fade_cb(fade_complete_cb_t cb);
void serv_video_draw_text(uint8_t layer, uint8_t fg, uint8_t bg, char *txt_p, uint32_t x, uint32_t y);
void serv_video_clear_layer(uint8_t layer);
void serv_video_draw_load_prog(uint32_t prog);

#endif
