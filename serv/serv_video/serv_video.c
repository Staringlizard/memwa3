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

#include "serv_video.h"
#include "if.h"
#include "serv_mem.h"
#include "drv_ltdc.h"
#include "drv_rng.h"
#include "dev_tda19988.h"

#define SCREEN_X2
#define SCREEN_WIDTH            SERV_VIDEO_SCREEN_WIDTH
#define SCREEN_HEIGHT           SERV_VIDEO_SCREEN_HEIGHT
#define MISC_WIDTH              SERV_VIDEO_MISC_WIDTH
#define MISC_HEIGHT             SERV_VIDEO_MISC_HEIGHT
#define UPPER_BORDER            32
#define LOWER_BORDER            50
#define LEFT_BORDER             44
#define RIGHT_BORDER            36
#define EMU_WIDTH               (320 + LEFT_BORDER + RIGHT_BORDER)
#define EMU_HEIGHT              (200 + UPPER_BORDER + LOWER_BORDER)

#define FADE_STEP               1
#define FADE_MAX_UP_EMU         255
#define FADE_MAX_UP_MENU        255
#define FADE_MAX_DOWN_EMU       100
#define FADE_MAX_DOWN_MENU      0

#define COLOR_BG                0
#define COLOR_TRANS             0

#define FONT_HEIGHT             SERV_VIDEO_FONT_HEIGHT
#define FONT_WIDTH              SERV_VIDEO_FONT_WIDTH
#define ROW_HEIGHT              SERV_VIDEO_ROW_HEIGHT

#define BAR_WIDTH               SERV_VIDEO_MISC_WIDTH

#define LAYER_MISC              SERV_VIDEO_LAYER_MISC
#define LAYER_EMU               SERV_VIDEO_LAYER_EMU

extern tda19988_vm_t videomode_list[];

//static uint32_t g_current_bar_lines;

static uint8_t g_font[][SERV_VIDEO_FONT_WIDTH] =
{
 {0x00, 0x00, 0x00, 0x00, 0x00} // 20 
,{0x00, 0x00, 0x5f, 0x00, 0x00} // 21 !
,{0x00, 0x07, 0x00, 0x07, 0x00} // 22 "
,{0x14, 0x7f, 0x14, 0x7f, 0x14} // 23 #
,{0x24, 0x2a, 0x7f, 0x2a, 0x12} // 24 $
,{0x23, 0x13, 0x08, 0x64, 0x62} // 25 %
,{0x36, 0x49, 0x55, 0x22, 0x50} // 26 &
,{0x00, 0x05, 0x03, 0x00, 0x00} // 27 '
,{0x00, 0x1c, 0x22, 0x41, 0x00} // 28 (
,{0x00, 0x41, 0x22, 0x1c, 0x00} // 29 )
,{0x14, 0x08, 0x3e, 0x08, 0x14} // 2a *
,{0x08, 0x08, 0x3e, 0x08, 0x08} // 2b +
,{0x00, 0x50, 0x30, 0x00, 0x00} // 2c ,
,{0x08, 0x08, 0x08, 0x08, 0x08} // 2d -
,{0x00, 0x60, 0x60, 0x00, 0x00} // 2e .
,{0x20, 0x10, 0x08, 0x04, 0x02} // 2f /
,{0x3e, 0x51, 0x49, 0x45, 0x3e} // 30 0
,{0x00, 0x42, 0x7f, 0x40, 0x00} // 31 1
,{0x42, 0x61, 0x51, 0x49, 0x46} // 32 2
,{0x21, 0x41, 0x45, 0x4b, 0x31} // 33 3
,{0x18, 0x14, 0x12, 0x7f, 0x10} // 34 4
,{0x27, 0x45, 0x45, 0x45, 0x39} // 35 5
,{0x3c, 0x4a, 0x49, 0x49, 0x30} // 36 6
,{0x01, 0x71, 0x09, 0x05, 0x03} // 37 7
,{0x36, 0x49, 0x49, 0x49, 0x36} // 38 8
,{0x06, 0x49, 0x49, 0x29, 0x1e} // 39 9
,{0x00, 0x36, 0x36, 0x00, 0x00} // 3a :
,{0x00, 0x56, 0x36, 0x00, 0x00} // 3b ;
,{0x08, 0x14, 0x22, 0x41, 0x00} // 3c <
,{0x11, 0x11, 0x11, 0x11, 0x11} // 3d =
,{0x00, 0x41, 0x22, 0x14, 0x08} // 3e >
,{0x02, 0x01, 0x51, 0x09, 0x06} // 3f ?
,{0x32, 0x49, 0x79, 0x41, 0x3e} // 40 @
,{0x7e, 0x11, 0x11, 0x11, 0x7e} // 41 A
,{0x7f, 0x49, 0x49, 0x49, 0x36} // 42 B
,{0x3e, 0x41, 0x41, 0x41, 0x22} // 43 C
,{0x7f, 0x41, 0x41, 0x22, 0x1c} // 44 D
,{0x7f, 0x49, 0x49, 0x49, 0x41} // 45 E
,{0x7f, 0x09, 0x09, 0x09, 0x01} // 46 F
,{0x3e, 0x41, 0x49, 0x49, 0x7a} // 47 G
,{0x7f, 0x08, 0x08, 0x08, 0x7f} // 48 H
,{0x00, 0x41, 0x7f, 0x41, 0x00} // 49 I
,{0x20, 0x40, 0x41, 0x3f, 0x01} // 4a J
,{0x7f, 0x08, 0x14, 0x22, 0x41} // 4b K
,{0x7f, 0x40, 0x40, 0x40, 0x40} // 4c L
,{0x7f, 0x02, 0x0c, 0x02, 0x7f} // 4d M
,{0x7f, 0x04, 0x08, 0x10, 0x7f} // 4e N
,{0x3e, 0x41, 0x41, 0x41, 0x3e} // 4f O
,{0x7f, 0x09, 0x09, 0x09, 0x06} // 50 P
,{0x3e, 0x41, 0x51, 0x21, 0x5e} // 51 Q
,{0x7f, 0x09, 0x19, 0x29, 0x46} // 52 R
,{0x46, 0x49, 0x49, 0x49, 0x31} // 53 S
,{0x01, 0x01, 0x7f, 0x01, 0x01} // 54 T
,{0x3f, 0x40, 0x40, 0x40, 0x3f} // 55 U
,{0x1f, 0x20, 0x40, 0x20, 0x1f} // 56 V
,{0x3f, 0x40, 0x38, 0x40, 0x3f} // 57 W
,{0x63, 0x14, 0x08, 0x14, 0x63} // 58 X
,{0x07, 0x08, 0x70, 0x08, 0x07} // 59 Y
,{0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z
,{0x00, 0x7f, 0x41, 0x41, 0x00} // 5b [
,{0x02, 0x04, 0x08, 0x10, 0x20} // 5c ¥
,{0x00, 0x41, 0x41, 0x7f, 0x00} // 5d ]
,{0x04, 0x02, 0x01, 0x02, 0x04} // 5e ^
,{0x40, 0x40, 0x40, 0x40, 0x40} // 5f _
,{0x00, 0x01, 0x02, 0x04, 0x00} // 60 `
,{0x20, 0x54, 0x54, 0x54, 0x78} // 61 a
,{0x7f, 0x48, 0x44, 0x44, 0x38} // 62 b
,{0x38, 0x44, 0x44, 0x44, 0x20} // 63 c
,{0x38, 0x44, 0x44, 0x48, 0x7f} // 64 d
,{0x38, 0x54, 0x54, 0x54, 0x18} // 65 e
,{0x08, 0x7e, 0x09, 0x01, 0x02} // 66 f
,{0x0c, 0x52, 0x52, 0x52, 0x3e} // 67 g
,{0x7f, 0x08, 0x04, 0x04, 0x78} // 68 h
,{0x00, 0x44, 0x7d, 0x40, 0x00} // 69 i
,{0x20, 0x40, 0x44, 0x3d, 0x00} // 6a j 
,{0x7f, 0x10, 0x28, 0x44, 0x00} // 6b k
,{0x00, 0x41, 0x7f, 0x40, 0x00} // 6c l
,{0x7c, 0x04, 0x18, 0x04, 0x78} // 6d m
,{0x7c, 0x08, 0x04, 0x04, 0x78} // 6e n
,{0x38, 0x44, 0x44, 0x44, 0x38} // 6f o
,{0x7c, 0x14, 0x14, 0x14, 0x08} // 70 p
,{0x08, 0x14, 0x14, 0x18, 0x7c} // 71 q
,{0x7c, 0x08, 0x04, 0x04, 0x08} // 72 r
,{0x48, 0x54, 0x54, 0x54, 0x20} // 73 s
,{0x04, 0x3f, 0x44, 0x40, 0x20} // 74 t
,{0x3c, 0x40, 0x40, 0x20, 0x7c} // 75 u
,{0x1c, 0x20, 0x40, 0x20, 0x1c} // 76 v
,{0x3c, 0x40, 0x30, 0x40, 0x3c} // 77 w
,{0x44, 0x28, 0x10, 0x28, 0x44} // 78 x
,{0x0c, 0x50, 0x50, 0x50, 0x3c} // 79 y
,{0x44, 0x64, 0x54, 0x4c, 0x44} // 7a z
,{0x00, 0x08, 0x36, 0x41, 0x00} // 7b {
,{0x00, 0x00, 0x7f, 0x00, 0x00} // 7c |
,{0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
,{0x10, 0x08, 0x08, 0x10, 0x08} // 7e ←
,{0x78, 0x46, 0x41, 0x46, 0x78} // 7f →
};

static uint8_t g_alpha[2] = {0};
static uint32_t g_fade_stamp_ms[2] = {0};
static uint8_t g_fade_max_up[2] = {FADE_MAX_UP_EMU, FADE_MAX_UP_MENU};
static uint8_t g_fade_max_down[2] = {FADE_MAX_DOWN_EMU, FADE_MAX_DOWN_MENU};
static uint32_t g_fade_step_ms = FADE_STEP;
static fade_complete_cb_t g_fade_complete_cb;

void serv_video_init()
{
    tda19988_vm_t tda19988_vm = {0};
    tda19988_vm = videomode_list[10];

    tda19988_init();
    tda19988_configure();
    tda19988_init_encoder(&tda19988_vm);

    drv_ltdc_set_memory(DRV_LTDC_BUFFER_0, CC_DISP_BUFFER0_ADDR);
    drv_ltdc_set_memory(DRV_LTDC_BUFFER_1, CC_DISP_BUFFER1_ADDR);
    drv_ltdc_set_memory(DRV_LTDC_BUFFER_2, CC_DISP_BUFFER2_ADDR);

    drv_ltdc_init(LTDC_MODE_SVGA);

    /* Careful here so that the bus does not overload */
    drv_ltdc_set_layer(LAYER_MISC,
        SCREEN_WIDTH/2 - MISC_WIDTH/2,
        SCREEN_WIDTH/2 + MISC_WIDTH/2,
        0,
        MISC_HEIGHT,
        MISC_WIDTH,
        MISC_HEIGHT,
        g_alpha[LAYER_MISC],
        LTDC_PIXEL_FORMAT_L8);

#ifdef SCREEN_X2
    drv_ltdc_set_layer(LAYER_EMU,
        (SCREEN_WIDTH-EMU_WIDTH*2)/2,
        (SCREEN_WIDTH-EMU_WIDTH*2)/2 + EMU_WIDTH*2,
        (SCREEN_HEIGHT-EMU_HEIGHT*2)/2,
        (SCREEN_HEIGHT-EMU_HEIGHT*2)/2 + EMU_HEIGHT*2,
        EMU_WIDTH*2,
        EMU_HEIGHT*2,
        g_alpha[LAYER_EMU],
        LTDC_PIXEL_FORMAT_L8);
#else
    drv_ltdc_set_layer(LAYER_EMU,
        (SCREEN_WIDTH-EMU_WIDTH)/2,
        (SCREEN_WIDTH-EMU_WIDTH)/2 + EMU_WIDTH,
        (SCREEN_HEIGHT-EMU_HEIGHT)/2,
        (SCREEN_HEIGHT-EMU_HEIGHT)/2 + EMU_HEIGHT,
        EMU_WIDTH,
        EMU_HEIGHT,
        g_alpha[LAYER_EMU],
        LTDC_PIXEL_FORMAT_L8);
#endif

    drv_ltdc_deactivate_layer(LAYER_MISC);
    drv_ltdc_deactivate_layer(LAYER_EMU);
    drv_ltdc_fill_layer(LAYER_MISC, COLOR_BG);
    drv_ltdc_fill_layer(LAYER_EMU, 0);

    drv_ltdc_set_trans_color(LAYER_MISC, COLOR_TRANS);

	g_if_cc_emu.if_emu_cc_display.display_layer_set_fp((uint8_t *)CC_DISP_BUFFER0_ADDR);
}

void serv_video_set_layer_size(uint8_t layer, uint32_t width, uint32_t height)
{
    drv_ltdc_change_size(layer, width, height);
}

void serv_video_en()
{
    drv_ltdc_deactivate_layer(LAYER_MISC);
    drv_ltdc_deactivate_layer(LAYER_EMU);

    drv_ltdc_enable_clut(LAYER_MISC);
    drv_ltdc_enable_clut(LAYER_EMU);

    drv_ltdc_activate_layer(LAYER_MISC);
    drv_ltdc_activate_layer(LAYER_EMU);
}

void serv_video_flip_buffer(uint8_t **buffer_pp)
{
    /* set ltdc to display the buffer */
    drv_ltdc_flip_buffer(SERV_VIDEO_LAYER_EMU, *buffer_pp);

    /* And shift buffer, so that emu will start drawing on the new buffer */
    switch(drv_ltdc_get_buffer(*buffer_pp))
    {
        case DRV_LTDC_BUFFER_0:
            *buffer_pp = (uint8_t *)CC_DISP_BUFFER1_ADDR;
            break;
        case DRV_LTDC_BUFFER_1:
            *buffer_pp = (uint8_t *)CC_DISP_BUFFER0_ADDR;
            break;
        default:
            break;
    }
}

void serv_video_fade(uint8_t layer, serv_video_fade_t fade)
{
    if(fade == SERV_VIDEO_FADE_UP && g_alpha[layer] == g_fade_max_up[layer])
    {
        return;
    }
    else if(fade == SERV_VIDEO_FADE_DOWN && g_alpha[layer] == g_fade_max_down[layer])
    {
        return;
    }

    if(g_fade_stamp_ms[layer] < HAL_GetTick())
    {
        g_fade_stamp_ms[layer] = HAL_GetTick() + g_fade_step_ms;
        if(fade == SERV_VIDEO_FADE_UP)
        {
            g_alpha[layer]++;

            if(g_alpha[layer] == g_fade_max_up[layer] &&
               g_fade_complete_cb != NULL)
            {
                g_fade_complete_cb(layer, fade);
            }
        }
        else if(fade == SERV_VIDEO_FADE_DOWN)
        {
            g_alpha[layer]--;

            if(g_alpha[layer] == g_fade_max_down[layer] &&
               g_fade_complete_cb != NULL)
            {
                g_fade_complete_cb(layer, fade);
            }
        }
    }

    drv_ltdc_set_alpha(layer, g_alpha[layer]);
}

void serv_video_reg_fade_cb(fade_complete_cb_t cb)
{
    g_fade_complete_cb = cb;
}

void serv_video_draw_text(uint8_t layer, uint8_t fg, uint8_t bg, char *txt_p, uint32_t x, uint32_t y)
{
    uint8_t *bmap_pp[256];
    uint32_t cnt = 0;
    uint32_t i;
    uint32_t k;
    uint32_t line;
    uint8_t *canvas_p;

    /* No need to draw outside of screen */
    if(y > MISC_HEIGHT)
    {
        return;
    }

    canvas_p = drv_ltdc_get_layer(LAYER_MISC);

    while(txt_p[cnt] != '\0')
    {
        bmap_pp[cnt] = g_font[txt_p[cnt] - 0x20];
        cnt++;

        if(cnt >= 256)
        {
            break;
        }
    }

    for(line = 0; line < FONT_HEIGHT; line++)
    {
        for(i = 0; i < cnt; i++)
        {
            for(k = 0; k < FONT_WIDTH; k++)
            {
                if(bmap_pp[i][k] & (1<<line)) /* draw pixel */
                {
                    canvas_p[x + k + i*FONT_WIDTH + (line + y)*MISC_WIDTH] = fg;
                }
                else
                {
                    canvas_p[x + k + i*FONT_WIDTH + (line + y)*MISC_WIDTH] = bg;
                }
            }
        }
    }
}

void serv_video_clear_layer(uint8_t layer)
{
    drv_ltdc_fill_layer(layer, COLOR_BG);
}

void serv_video_draw_load_prog(uint32_t prog)
{
    char text_p[32];
    sprintf(text_p, "Files scanned: %ld", prog);
    serv_video_draw_text(LAYER_MISC, 14, 0, text_p, SCREEN_WIDTH/2 - strlen(text_p)*FONT_WIDTH/2, SCREEN_HEIGHT/4);
}
