/*
 * memwa2 state machine
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

#include "fsm.h"
#include "serv_video.h"
#include "serv_keybd.h"
#include "serv_storage.h"
#include "serv_mem.h"
#include "dev_tda19988.h"
#include "drv_led.h"
#include "version.h"

#define MAX_EXEC_CYCLES		400
#define MAX_FILTER          28

#define KEY_C           0x06
#define KEY_RETURN      0x28
#define KEY_ESC		    0x29
#define KEY_BS          0x2A
#define KEY_F1		    0x3A
#define KEY_F2		    0x3B
#define KEY_F3		    0x3C
#define KEY_F4		    0x3D
#define KEY_F5		    0x3E
#define KEY_F6		    0x3F
#define KEY_F7		    0x40
#define KEY_F8		    0x41
#define KEY_F9		    0x42
#define KEY_F10		    0x43
#define KEY_F11		    0x44
#define KEY_F12		    0x45
#define KEY_PG_UP       0x4B
#define KEY_DEL         0x4c
#define KEY_PG_DOWN     0x4E
#define KEY_ARROW_RIGHT 0x4F
#define KEY_ARROW_LEFT  0x50
#define KEY_ARROW_DOWN  0x51
#define KEY_ARROW_UP    0x52

#define COLOR_TXT_FG        12
#define COLOR_TXT_BG        0
#define COLOR_MARKER_FG     6
#define COLOR_MARKER_BG     0

#define INFO_SEPARATOR_DISTANCE  120

#define STATES_MAX              4

typedef void (*transition_t)();

typedef enum
{
    FSM_STATE_INIT,
	FSM_STATE_EMU,
    FSM_STATE_LIST,
    FSM_STATE_TERM
} state_t;

typedef struct
{
    uint32_t pos_phy;
    uint32_t pos_list;
} marker_t;

typedef struct
{
    serv_storage_file_t *files_p;
    uint32_t files;
} file_list_t;

/* Transition prototypes */
static void t_default();
static void t_init2emu();
static void t_emu2list();
static void t_list2emu();
static void t_emu2term();
static void t_term2emu();
static void t_emu2emu();
static void t_list2list();
static void t_term2term();

static state_t g_state = FSM_STATE_INIT;
static uint8_t g_disk_drive_on;
static uint8_t g_lock_freq_pal = 1;
static uint8_t g_limit_frame_rate = 0; /* Emulator can half its emulated frame rate to gain performance */
static uint8_t g_tape_play;
static serv_storage_file_t *g_files_p;
static serv_storage_file_t *g_files_filt_p;
static uint32_t g_files_filt;
static uint32_t g_files;
static char g_filter_p[MAX_FILTER];
static uint32_t g_filter_cnt;
static marker_t g_marker;
static uint32_t g_page;
static file_list_t g_current_file_list;
static uint32_t g_fps;
static uint8_t g_led;
static uint32_t g_term_row;
static uint8_t g_info;

static transition_t trans_pp[STATES_MAX][STATES_MAX] =
{
                    /* STATE_INIT */        /* STATE_EMU */         /* STATE_LIST */        /* STATE_TERM */
/* STATE_INIT */    {t_default,             t_init2emu,             t_emu2list,             t_emu2term},
/* STATE_EMU */     {t_default,             t_emu2emu,              t_emu2list,             t_emu2term},
/* STATE_LIST */    {t_default,             t_list2emu,             t_list2list,            t_default},
/* STATE_TERM */    {t_default,             t_term2emu,             t_default,              t_term2term},
};

static void t_default()
{

}

static void t_init2emu()
{
    serv_video_set_layer_size(SERV_VIDEO_LAYER_MISC, SERV_VIDEO_SCREEN_WIDTH, SERV_VIDEO_SCREEN_HEIGHT/16);
}

static void t_emu2list()
{
    serv_video_set_layer_size(SERV_VIDEO_LAYER_MISC, SERV_VIDEO_SCREEN_WIDTH, SERV_VIDEO_SCREEN_HEIGHT);
}

static void t_list2emu()
{
    serv_video_set_layer_size(SERV_VIDEO_LAYER_MISC, SERV_VIDEO_SCREEN_WIDTH, SERV_VIDEO_SCREEN_HEIGHT/16);
}

static void t_emu2term()
{
    serv_video_set_layer_size(SERV_VIDEO_LAYER_MISC, SERV_VIDEO_SCREEN_WIDTH, SERV_VIDEO_SCREEN_HEIGHT);
}

static void t_term2emu()
{
    serv_video_set_layer_size(SERV_VIDEO_LAYER_MISC, SERV_VIDEO_SCREEN_WIDTH, SERV_VIDEO_SCREEN_HEIGHT/16);
}

static void t_emu2emu()
{

}

static void t_list2list()
{
    
}

static void t_term2term()
{
    
}

static void change_state(state_t state)
{
    trans_pp[g_state][state]();
    g_state = state;
}

static void filter_changed(char key, uint8_t add)
{
    if(add)
    {
        if(g_filter_cnt < MAX_FILTER)
        {
            g_files_filt = g_files;

            g_filter_p[g_filter_cnt] = key;
            g_filter_cnt++;

            serv_storage_files_filter(g_files_p, &g_files_filt, &g_files_filt_p, g_filter_p);
            g_current_file_list.files_p = g_files_filt_p;
            g_current_file_list.files = g_files_filt;
        }
    }
    else
    {
        if(g_filter_cnt > 0)
        {
            g_files_filt = g_files;

            g_filter_cnt--;
            g_filter_p[g_filter_cnt] = '\0';

            if(g_filter_cnt == 0)
            {
                g_current_file_list.files = g_files;
                g_current_file_list.files_p = g_files_p;
            }
            else
            {
                serv_storage_files_filter(g_files_p, &g_files_filt, &g_files_filt_p, g_filter_p);
                g_current_file_list.files_p = g_files_filt_p;
                g_current_file_list.files = g_files_filt;
            }
        }
    }
}

static void draw_list_member(uint32_t member)
{
    char text_p[128];

    sprintf(text_p, "%s [%ldKb]",
            g_current_file_list.files_p[member].fname_p,
            g_current_file_list.files_p[member].size/1024);

    serv_video_draw_text(SERV_VIDEO_LAYER_MISC,
                         COLOR_TXT_FG,
                         COLOR_TXT_BG,
                         text_p,
                         SERV_VIDEO_MISC_WIDTH/2 - strlen(text_p)*SERV_VIDEO_FONT_WIDTH/2,
                         member % SERV_VIDEO_MISC_ROWS*SERV_VIDEO_ROW_HEIGHT);
}

static void draw_list()
{
    uint32_t i;
    uint32_t files = SERV_VIDEO_MISC_ROWS;

    if(g_current_file_list.files < SERV_VIDEO_MISC_ROWS)
    {
        files = g_current_file_list.files;
    }

    serv_video_clear_layer(SERV_VIDEO_LAYER_MISC);

    for(i = g_page*SERV_VIDEO_MISC_ROWS; i < (g_page+1)*SERV_VIDEO_MISC_ROWS; i++)
    {
        if(files == 0)
        {
            break;
        }
        files--;
        draw_list_member(i);
    }
}

static void draw_term(uint32_t row_start)
{
    char *row_p;
    uint32_t row_cnt = 0;

    serv_video_clear_layer(SERV_VIDEO_LAYER_MISC);

    row_p = serv_term_get_row(row_start);

    while(row_p != NULL && row_cnt < SERV_VIDEO_MISC_ROWS)
    {
        serv_video_draw_text(SERV_VIDEO_LAYER_MISC,
             COLOR_TXT_FG,
             COLOR_TXT_BG,
             row_p,
             SERV_VIDEO_MISC_WIDTH/2 - strlen(row_p)*SERV_VIDEO_FONT_WIDTH/2,
             row_cnt*SERV_VIDEO_ROW_HEIGHT);

        row_cnt++;
        row_start++;
        row_p = serv_term_get_row(row_start);
    }
}

static void clear_filter()
{
    uint32_t i;

    /* clear filter */
    for(i = 0; i < g_filter_cnt; i++)
    {
        g_filter_p[i] = 0x00;
    }

    g_filter_cnt = 0;

    /* clear filter */
    for(i = 0; i < g_files_filt; i++)
    {
        memset(&g_files_filt_p[i], 0x00, sizeof(serv_storage_file_t));
    }

    g_files_filt = 0;
}

static void draw_marker()
{
    if(g_current_file_list.files)
    {
        char text_p[128];

        sprintf(text_p, "%s [%ldKb]",
                g_current_file_list.files_p[g_marker.pos_list].fname_p,
                g_current_file_list.files_p[g_marker.pos_list].size/1024);

        serv_video_draw_text(SERV_VIDEO_LAYER_MISC,
                             COLOR_MARKER_FG,
                             COLOR_MARKER_BG,
                             text_p,
                             SERV_VIDEO_MISC_WIDTH/2 - strlen(text_p)*SERV_VIDEO_FONT_WIDTH/2,
                             g_marker.pos_phy*SERV_VIDEO_ROW_HEIGHT);
    }
}

static void reset_emu()
{
    g_if_cc_emu.if_emu_cc_op.op_reset_fp();
    g_if_dd_emu.if_emu_dd_op.op_reset_fp();
}

static void reset_mcu()
{
    NVIC_SystemReset();
}

static void list_up()
{
    if(g_marker.pos_phy != 0)
    {
        draw_list_member(g_marker.pos_list);
        g_marker.pos_phy--;
        g_marker.pos_list--;
        draw_marker();
    }
    else
    {
        /* going to the last element in visible list */
        draw_list_member(g_marker.pos_list);
        g_marker.pos_phy = SERV_VIDEO_MISC_ROWS - 1;
        g_marker.pos_list += SERV_VIDEO_MISC_ROWS - 1;

        /* Adjust marker so that it never position below last file */
        if(g_marker.pos_phy >= (g_current_file_list.files - g_page*SERV_VIDEO_MISC_ROWS))
        {
            g_marker.pos_phy = (g_current_file_list.files - g_page*SERV_VIDEO_MISC_ROWS) - 1;
            g_marker.pos_list = g_current_file_list.files - 1;
        }
        draw_marker();
    }
}

static void list_down()
{
    if(g_marker.pos_phy != (SERV_VIDEO_MISC_ROWS - 1) &&
       g_marker.pos_phy < (g_current_file_list.files - g_page*SERV_VIDEO_MISC_ROWS) - 1)
    {
        draw_list_member(g_marker.pos_list);
        g_marker.pos_phy++;
        g_marker.pos_list++;
        draw_marker();
    }
    else
    {
        /* going to the first element in visible list */
        draw_list_member(g_marker.pos_list);
        g_marker.pos_phy = 0;
        if(g_marker.pos_list > SERV_VIDEO_MISC_ROWS - 1)
        {
            uint32_t elements_left_in_list = g_current_file_list.files - g_page*SERV_VIDEO_MISC_ROWS - 1;
            uint32_t sub = 0;

            /* Need to check how much can be substracted */
            if(elements_left_in_list < SERV_VIDEO_MISC_ROWS - 1)
            {
                sub = elements_left_in_list;
            }
            else
            {
                sub = SERV_VIDEO_MISC_ROWS - 1;
            }
            g_marker.pos_list -= sub;
        }
        else
        {
            g_marker.pos_list = 0;
        }
        draw_marker();
    }
}

static void list_pg_up()
{
    if(g_page > 0)
    {
        g_page--;
        g_marker.pos_list -= SERV_VIDEO_MISC_ROWS;

        draw_list();
        draw_marker();
    }
}

static void list_pg_down()
{
    if(g_page < (g_current_file_list.files/SERV_VIDEO_MISC_ROWS))
    {
        g_page++;
        g_marker.pos_list += SERV_VIDEO_MISC_ROWS;

        /* Adjust marker so that it never position below last file */
        if(g_marker.pos_phy >= (g_current_file_list.files - g_page*SERV_VIDEO_MISC_ROWS))
        {
            g_marker.pos_phy = (g_current_file_list.files - g_page*SERV_VIDEO_MISC_ROWS) - 1;
            g_marker.pos_list = g_current_file_list.files - 1;
        }
        draw_list();
        draw_marker();
    }
}

static void list_filter_changed(uint8_t add, char c)
{
    g_page = 0;
    g_marker.pos_phy = 0;
    g_marker.pos_list = 0;
    filter_changed(serv_keybd_get_ascii(c), add);
    draw_list();
    draw_marker();
}

static void fade_complete_cb(uint8_t layer, serv_video_fade_t fade)
{
    fsm_event(FSM_EVENT_FADE_DONE, layer, fade);
}

static void scan_files_cb(uint32_t files)
{
    fsm_event(FSM_EVENT_SCAN_FILES, files, 0);
}

static void key_event_cb(uint8_t key, serv_keybd_state_t state)
{
    fsm_event(FSM_EVENT_KEY, key, (uint32_t)state);
}

static void draw_info_field()
{
    char tmp_p[64];
    char *cc_ver_p;
    char *dd_ver_p;

    sprintf(tmp_p, "emu fps: %ld ", g_fps);
    serv_video_draw_text(SERV_VIDEO_LAYER_MISC,
                         14,
                         0,
                         tmp_p,
                         0,
                         0);

    sprintf(tmp_p, "disk drive on: %s", g_disk_drive_on ? "yes":"no ");

    serv_video_draw_text(SERV_VIDEO_LAYER_MISC,
                         g_led ? 15 : 14,
                         0,
                         tmp_p,
                         INFO_SEPARATOR_DISTANCE*1,
                         0);

    sprintf(tmp_p, "lock freq: %s", g_lock_freq_pal ? "yes":"no ");

    serv_video_draw_text(SERV_VIDEO_LAYER_MISC,
                         14,
                         0,
                         tmp_p,
                         INFO_SEPARATOR_DISTANCE*2,
                         0);

    sprintf(tmp_p, "skip frames: %s", g_limit_frame_rate ? "yes":"no ");

    serv_video_draw_text(SERV_VIDEO_LAYER_MISC,
                         14,
                         0,
                         tmp_p,
                         INFO_SEPARATOR_DISTANCE*3,
                         0);

    sprintf(tmp_p, "tape play: %s", g_tape_play ? "yes":"no ");

    serv_video_draw_text(SERV_VIDEO_LAYER_MISC,
                         14,
                         0,
                         tmp_p,
                         INFO_SEPARATOR_DISTANCE*4,
                         0);

    sprintf(tmp_p, "host version: %s", FW_VERSION);

    serv_video_draw_text(SERV_VIDEO_LAYER_MISC,
                         14,
                         0,
                         tmp_p,
                         SERV_VIDEO_SCREEN_WIDTH - strlen(tmp_p)*SERV_VIDEO_FONT_WIDTH,
                         0);

    g_if_cc_emu.if_emu_cc_ver.ver_get_fp(&cc_ver_p);
    g_if_dd_emu.if_emu_dd_ver.ver_get_fp(&dd_ver_p);
    sprintf(tmp_p, "emu version cc/dd: %s/%s", cc_ver_p, dd_ver_p);

    serv_video_draw_text(SERV_VIDEO_LAYER_MISC,
                         14,
                         0,
                         tmp_p,
                         SERV_VIDEO_SCREEN_WIDTH - strlen(tmp_p)*SERV_VIDEO_FONT_WIDTH,
                         SERV_VIDEO_FONT_HEIGHT);
}

void fsm_init()
{
    g_if_cc_emu.if_emu_cc_op.op_init_fp();
    g_if_dd_emu.if_emu_dd_op.op_init_fp();

    serv_video_reg_fade_cb(fade_complete_cb);
    serv_storage_reg_scan_files_cb(scan_files_cb);
    serv_keybd_reg_key_event_fp_cb(key_event_cb);

    g_if_cc_emu.if_emu_cc_display.display_lock_frame_rate_fp(g_lock_freq_pal);
    g_if_cc_emu.if_emu_cc_display.display_limit_frame_rate_fp(g_limit_frame_rate);

    change_state(FSM_STATE_EMU);
}

void fsm_state_emu(fsm_event_t e, uint32_t edata1, uint32_t edata2)
{
    switch(e)
    {
    case FSM_EVENT_RUN:
        if(g_disk_drive_on)
        {
            g_if_cc_emu.if_emu_cc_op.op_run_fp(25);
            g_if_dd_emu.if_emu_dd_op.op_run_fp(25);
        }
        else
        {
            g_if_cc_emu.if_emu_cc_op.op_run_fp(MAX_EXEC_CYCLES);
        }
        serv_video_fade(SERV_VIDEO_LAYER_EMU, SERV_VIDEO_FADE_UP);
        serv_video_fade(SERV_VIDEO_LAYER_MISC, SERV_VIDEO_FADE_UP);
        tda19988_irq_poll();
        serv_keybd_poll();
        break;
    case FSM_EVENT_TIMER_100MS:
        if(g_if_cc_emu.if_emu_cc_time.time_tenth_second_fp != NULL)
        {
            g_if_cc_emu.if_emu_cc_time.time_tenth_second_fp();
        }
        break;
    case FSM_EVENT_TIMER_1000MS:
        {
            if(g_info)
            {
                draw_info_field();
            }
            g_led = 0;
        }
        break;
    case FSM_EVENT_STATS_FPS:
        g_fps = edata1;
        break;
    case FSM_EVENT_STATS_LED:
        g_led = edata1;
        break;
    case FSM_EVENT_SERIAL_READ:
        break;
    case FSM_EVENT_SERIAL_WRITE:
        break;
    case FSM_EVENT_TAPE_PLAY:
        break;
    case FSM_EVENT_TAPE_MOTOR:
        break;
    case FSM_EVENT_KEY:
        if((serv_keybd_state_t)edata2 == SERV_KEYBD_STATE_PRESSED ||
           (serv_keybd_state_t)edata2 == SERV_KEYBD_STATE_RELEASED)
        {
            /* Do not care about long press */
            g_if_cc_emu.if_emu_cc_ue.ue_keybd_fp(serv_keybd_get_key_array(),
                                                 6,
                                                 serv_keybd_get_shift_state(),
                                                 serv_keybd_get_ctrl_state());
        }

        if(serv_keybd_get_ctrl_state() && (serv_keybd_state_t)edata2 == SERV_KEYBD_STATE_PRESSED)
        {
            switch(edata1)
            {
            case KEY_ESC:
                change_state(FSM_STATE_LIST);
                serv_video_clear_layer(SERV_VIDEO_LAYER_MISC);
                drv_led_set(0, 1, 0);
                break;
            case KEY_F1:
                g_info = !g_info;
                serv_video_clear_layer(SERV_VIDEO_LAYER_MISC);
                if(g_info)
                {
                    draw_info_field();
                }
                break;
            case KEY_F2:
                g_disk_drive_on = !g_disk_drive_on;
                break;
            case KEY_F3:
                g_lock_freq_pal = !g_lock_freq_pal;
                g_if_cc_emu.if_emu_cc_display.display_lock_frame_rate_fp(g_lock_freq_pal);
                break;
            case KEY_F4:
                g_limit_frame_rate = !g_limit_frame_rate;
                g_if_cc_emu.if_emu_cc_display.display_limit_frame_rate_fp(g_limit_frame_rate);
                break;
            case KEY_F5:
                g_tape_play = !g_tape_play;
                if(g_tape_play)
                {
                    g_if_cc_emu.if_emu_cc_tape_drive.tape_drive_play_fp();
                }
                else
                {
                    g_if_cc_emu.if_emu_cc_tape_drive.tape_drive_stop_fp();
                }
                break;
            case KEY_F6:
                break;
            case KEY_F9:
                break;
            case KEY_F10:
                change_state(FSM_STATE_TERM);
                serv_video_clear_layer(SERV_VIDEO_LAYER_MISC);
                drv_led_set(0, 1, 0);
                break;
            case KEY_F11:
                reset_emu();
                break;
            case KEY_F12:
                reset_mcu();
                break;
            }
        }
        break;
    default:
        break;
    }
}

void fsm_state_list(fsm_event_t e, uint32_t edata1, uint32_t edata2)
{
    switch(e)
    {
    case FSM_EVENT_RUN:
        serv_video_fade(SERV_VIDEO_LAYER_EMU, SERV_VIDEO_FADE_DOWN);
        tda19988_irq_poll();
        serv_keybd_poll();
        break;
    case FSM_EVENT_KEY:
        if((serv_keybd_state_t)edata2 == SERV_KEYBD_STATE_RELEASED)
        {
            /* Do not care if a key was released */
            break;
        }

        switch(edata1)
        {
        case KEY_RETURN:
            if(serv_storage_load_file(&g_current_file_list.files_p[g_marker.pos_list]) == 0)
            {
                change_state(FSM_STATE_EMU);
                serv_video_clear_layer(SERV_VIDEO_LAYER_MISC);
            }
            break;
        case KEY_ESC:
            change_state(FSM_STATE_EMU);
            serv_video_clear_layer(SERV_VIDEO_LAYER_MISC);
            break;
        case KEY_BS:
            list_filter_changed(0, 0);
            break;
        case KEY_ARROW_UP:
            list_up();
            break;
        case KEY_ARROW_DOWN:
            list_down();
            break;
        case KEY_PG_UP:
            list_pg_up();
            break;
        case KEY_PG_DOWN:
            list_pg_down();
            break;
        default:
            break;
        }

        if(edata1 >= 0x04 && edata1 <= 0x27)
        {
            list_filter_changed(1, edata1);
        }
        break;
    case FSM_EVENT_FADE_DONE:
        if(edata1 == SERV_VIDEO_LAYER_EMU &&
           edata2 == SERV_VIDEO_FADE_DOWN)
        {
            if(g_current_file_list.files_p == NULL)
            {
                g_page = 0;
                serv_storage_scan_files(&g_files_p, &g_files);
            }
            g_current_file_list.files_p = g_files_p;
            g_current_file_list.files = g_files;
            draw_list();
            clear_filter();
            draw_marker();
        }
        break;
    case FSM_EVENT_SCAN_FILES:
        serv_video_draw_load_prog(edata1);
    default:
        break;
    }
}

void fsm_state_term(fsm_event_t e, uint32_t edata1, uint32_t edata2)
{
    switch(e)
    {
    case FSM_EVENT_RUN:
        serv_video_fade(SERV_VIDEO_LAYER_EMU, SERV_VIDEO_FADE_DOWN);
        tda19988_irq_poll();
        serv_keybd_poll();
        break;
    case FSM_EVENT_KEY:
        if((serv_keybd_state_t)edata2 == SERV_KEYBD_STATE_RELEASED)
        {
            /* Do not care if a key was released */
            break;
        }

        switch(edata1)
        {
        case KEY_RETURN:
            {
                serv_term_receive((uint8_t *)"\r\n", 2);
            }
            break;
        case KEY_ESC:
            change_state(FSM_STATE_EMU);
            serv_video_clear_layer(SERV_VIDEO_LAYER_MISC);
            break;
        case KEY_ARROW_UP:
            if(g_term_row >= 1)
            {
                g_term_row -= 1;
                draw_term(g_term_row);
            }
            break;
        case KEY_ARROW_DOWN:
            g_term_row += 1;
            draw_term(g_term_row);
            break;
        case KEY_PG_UP:
            if(g_term_row >= SERV_VIDEO_MISC_ROWS)
            {
                g_term_row -= SERV_VIDEO_MISC_ROWS;
                draw_term(g_term_row);
            }
            else if(g_term_row > 0)
            {
                g_term_row = 0;
                draw_term(g_term_row);
            }
            break;
        case KEY_PG_DOWN:
            g_term_row += SERV_VIDEO_MISC_ROWS;
            draw_term(g_term_row);
            break;
        case KEY_DEL:
            serv_video_clear_layer(SERV_VIDEO_LAYER_MISC);
            serv_term_clear_rows();
            g_term_row = 0;
            break;
        case KEY_F12:
            reset_mcu();
            break;
        default:
            {
                char tmp = serv_keybd_get_ascii(edata1);
                serv_term_receive((uint8_t *)&tmp, 1);
            }
            break;
        }
        break;
    case FSM_EVENT_FADE_DONE:
        if(edata1 == SERV_VIDEO_LAYER_EMU &&
           edata2 == SERV_VIDEO_FADE_DOWN)
        {
            /* Position the last row in the middle of the screen */
            if(serv_term_get_rows() >= SERV_VIDEO_MISC_ROWS/2)
            {
                g_term_row = serv_term_get_rows() - SERV_VIDEO_MISC_ROWS/2;
            }
            else
            {
                g_term_row = 0;
            }
            draw_term(g_term_row);
        }
        break;
    case FSM_EVENT_TEXT_ROW:
            /* Position the last row in the middle of the screen */
            if(serv_term_get_rows() >= SERV_VIDEO_MISC_ROWS/2)
            {
                g_term_row = serv_term_get_rows() - SERV_VIDEO_MISC_ROWS/2;
            }
            else
            {
                g_term_row = 0;
            }
            draw_term(g_term_row);
        break;
    default:
        break;
    }
}

void fsm_event(fsm_event_t e, uint32_t edata1, uint32_t edata2)
{
    switch(g_state)
    {
        case FSM_STATE_EMU:
            fsm_state_emu(e, edata1, edata2);
            break;
        case FSM_STATE_LIST:
            fsm_state_list(e, edata1, edata2);
            break;
        case FSM_STATE_TERM:
            fsm_state_term(e, edata1, edata2);
            break;
        default:
            break;
    }
}
