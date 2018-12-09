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

#include "dev_video.h"
#include "if.h"
#include "dev_mem.h"
#include "drv_ltdc.h"
#include "tda19988.h"

extern tda19988_vm_t videomode_list[];

void video_init()
{
    tda19988_vm_t tda19988_vm = {0};
    tda19988_vm = videomode_list[10];

    tda19988_init();
    tda19988_configure();
    tda19988_init_encoder(&tda19988_vm);

    drv_ltdc_init(DISP_MODE_SVGA);
    drv_ltdc_deactivate_layer(0);
    drv_ltdc_deactivate_layer(1);

    drv_ltdc_set_memory(MEM_ADDR_BUFFER0, CC_DISP_BUFFER1_ADDR);
    drv_ltdc_set_memory(MEM_ADDR_BUFFER1, CC_DISP_BUFFER2_ADDR);
    drv_ltdc_set_memory(MEM_ADDR_BUFFER2, CC_DISP_BUFFER3_ADDR);

	g_if_cc_emu.if_emu_cc_display.display_layer_set_fp((uint8_t *)CC_DISP_BUFFER2_ADDR);
}

