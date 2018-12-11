/*
 * memwa2 main
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


/**
 * Main entry point. Handles main configuration of memory and setup.
 * It Aaso handles error codes and fw revision.
 */

#include "main.h"
#include "hal_conf.h"
#include "diag.h"
#include "usbh_core.h"
#include "usbh_hid_keybd.h"
#include "serv_term.h"
#include "serv_keybd.h"
#include "serv_misc.h"
#include "serv_storage.h"
#include "drv_joyst.h"
#include "drv_led.h"
#include "drv_sdcard.h"
#include "if.h"
#include "fsm.h"
#include "romcc.h"
#include "romdd.h"
#include "serv_mem.h"
#include "serv_audio.h"
#include "serv_video.h"

int main()
{
    //diag_status_t diag_status = DIAG_STATUS_OK;

    hal_conf_cache_inst_on();
    hal_conf_cache_data_on();

    __set_PRIMASK(0); /* Enable IRQ */

    HAL_Init();

    HAL_Delay(200);

    hal_conf_clk();
    drv_joyst_init();

    serv_mem_init();
    serv_storage_init();
    serv_storage_mount();
    serv_video_init();
    serv_audio_init();
    serv_keybd_init();
    serv_term_init();
    serv_misc_init();
/*
    diag_status = diag_run();
    if(diag_status != DIAG_STATUS_OK)
    {
        serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "Diagnostic failed with code %d!", diag_status);
        drv_led_set(1, 0, 0);
        while(1) {;}
    }

    if(drv_sdcard_inserted() == 1)
    {
        diag_status = diag_sdcard_run();
        if(diag_status != DIAG_STATUS_OK)
        {
            serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "Diagnostic failed with code %d!", diag_status);
            drv_led_set(1, 0, 0);
            while(1) {;}
        }
    }
*/
    serv_storage_read();
    serv_video_en();

    fsm_init();
    fsm_run();
}
