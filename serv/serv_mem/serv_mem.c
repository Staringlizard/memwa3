/*
 * Mem
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

#include "serv_mem.h"
#include "hal_conf.h"
#include "if.h"
#include "dev_is42s16400j.h"

/*
void *ff_memalloc(uint32_t size)
{
    return malloc(size);
}

void ff_memfree(void *mem_p)
{
    free(mem_p);
}
*/

void serv_mem_init()
{
    uint32_t i;

    dev_is42s16400j_init();
    hal_conf_mpu();

    /* Clean sdram */
    for(i = 0; i < SDRAM_SIZE; i++)
    {
        *(uint8_t *)(SDRAM_ADDR + i) = 0;
    }

    /* Give commodore computer (cc) some memory to work with */
    g_if_cc_emu.if_emu_cc_mem.mem_set_fp((uint8_t *)CC_RAM_BASE_ADDR, IF_MEM_CC_TYPE_RAM);
    g_if_cc_emu.if_emu_cc_mem.mem_set_fp((uint8_t *)CC_BROM_BASE_ADDR, IF_MEM_CC_TYPE_BROM);
    g_if_cc_emu.if_emu_cc_mem.mem_set_fp((uint8_t *)CC_CROM_BASE_ADDR, IF_MEM_CC_TYPE_CROM);
    g_if_cc_emu.if_emu_cc_mem.mem_set_fp((uint8_t *)CC_KROM_BASE_ADDR, IF_MEM_CC_TYPE_KROM);
    g_if_cc_emu.if_emu_cc_mem.mem_set_fp((uint8_t *)CC_IO_BASE_ADDR, IF_MEM_CC_TYPE_IO);
    g_if_cc_emu.if_emu_cc_mem.mem_set_fp((uint8_t *)CC_UTIL0_BASE_ADDR, IF_MEM_CC_TYPE_UTIL0); /* subscription read storage by emu */
    g_if_cc_emu.if_emu_cc_mem.mem_set_fp((uint8_t *)CC_UTIL1_BASE_ADDR, IF_MEM_CC_TYPE_UTIL1); /* subscription write storage by emu */
    g_if_cc_emu.if_emu_cc_mem.mem_set_fp((uint8_t *)CC_SPRITE0_BASE_ADDR, IF_MEM_CC_TYPE_SPRITE0); /* sprite virtual layer (background) by emu */
    g_if_cc_emu.if_emu_cc_mem.mem_set_fp((uint8_t *)CC_SPRITE1_BASE_ADDR, IF_MEM_CC_TYPE_SPRITE1); /* sprite virtual layer (forground) by emu */
    g_if_cc_emu.if_emu_cc_mem.mem_set_fp((uint8_t *)CC_SPRITE2_BASE_ADDR, IF_MEM_CC_TYPE_SPRITE2); /* sprite mapping by emu */

    /* Give disk drive (dd) some memory to work with */
    g_if_dd_emu.if_emu_dd_mem.mem_set_fp((uint8_t *)DD_ALL_BASE_ADDR, IF_MEM_DD_TYPE_ALL);
    g_if_dd_emu.if_emu_dd_mem.mem_set_fp((uint8_t *)DD_UTIL0_BASE_ADDR, IF_MEM_DD_TYPE_UTIL0);
    g_if_dd_emu.if_emu_dd_mem.mem_set_fp((uint8_t *)DD_UTIL1_BASE_ADDR, IF_MEM_DD_TYPE_UTIL1);
}

