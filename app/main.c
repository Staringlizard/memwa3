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
#include "config.h"
#include "diag.h"
#include "usbh_core.h"
#include "usbh_hid_keybd.h"
#include "dev_term.h"
#include "dev_keybd.h"
#include "if.h"
#include "romcc.h"
#include "romdd.h"
#include "stage.h"

#include "sm.h"
#include "dev_mem.h"
#include "dev_audio.h"
#include "dev_video.h"

static uint8_t tmp = 0;
int main()
{
    diag_status_t diag_status = DIAG_STATUS_OK;

    config_cache_inst_on();
    config_cache_data_on();

    __set_PRIMASK(0); /* Enable IRQ */

    HAL_Init();

    HAL_Delay(200);

    config_clk();
    drv_joyst_init();

#if 0
    /* Card inserted ? */
    if(drv_sdcard_inserted() == 0)
    {
        /* If not insterted, then run full diagnostic */
        diag_status = diag_run();

        if(diag_status != DIAG_STATUS_OK)
        {
            dev_term_printf("Diagnostic did not pass!", __FILE__, __LINE__, (uint32_t)diag_status);
            while(1){;}
        }

        while(1)
        {
            /* If not, then just try again */
            if(drv_sdcard_inserted() == 1)
            {
                drv_sdcard_init();
                HAL_Delay(1000);
                diag_status = diag_sdcard_run();
                if(diag_status != DIAG_STATUS_OK)
                {
                    dev_term_printf(DEV_TERM_PRINT_TYPE_ERROR, "Diagnostic did not pass!", __FILE__, __LINE__, (uint32_t)diag_status);
                }
                break;
            }
        }
    }
#endif
    dev_mem_init();
    dev_storage_init();
    dev_storage_mount();
    dev_storage_read();
    dev_video_init();
    dev_audio_init();
    dev_keybd_init();
    dev_term_init();
    dev_misc_init();

    stage_init(CC_STAGE_FILES_ADDR);
    stage_select_layer(0);

    sm_init();
    sm_run();
}


char *main_get_fw_revision()
{
    return FW_REVISION;
}
