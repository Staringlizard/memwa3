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

#include "dev_mem.h"
#include "if.h"
#include "hal_msp.h"

static void mpu_config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;
  
  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes as WB for SDRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = SDRAM_ADDR;
  MPU_InitStruct.Size = MPU_REGION_SIZE_8MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void mem_init()
{
    uint32_t i;

    drv_sdram_init();
    mpu_config();

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
    g_if_cc_emu.if_emu_cc_mem.mem_set_fp((uint8_t *)CC_UTIL1_BASE_ADDR, IF_MEM_CC_TYPE_UTIL1); /* subscription read storage by emu */
    g_if_cc_emu.if_emu_cc_mem.mem_set_fp((uint8_t *)CC_UTIL2_BASE_ADDR, IF_MEM_CC_TYPE_UTIL2); /* subscription write storage by emu */
    g_if_cc_emu.if_emu_cc_mem.mem_set_fp((uint8_t *)CC_SPRITE1_BASE_ADDR, IF_MEM_CC_TYPE_SPRITE1); /* sprite virtual layer (background) by emu */
    g_if_cc_emu.if_emu_cc_mem.mem_set_fp((uint8_t *)CC_SPRITE2_BASE_ADDR, IF_MEM_CC_TYPE_SPRITE2); /* sprite virtual layer (forground) by emu */
    g_if_cc_emu.if_emu_cc_mem.mem_set_fp((uint8_t *)CC_SPRITE3_BASE_ADDR, IF_MEM_CC_TYPE_SPRITE3); /* sprite mapping by emu */

    /* Give disk drive (dd) some memory to work with */
    g_if_dd_emu.if_emu_dd_mem.mem_set_fp((uint8_t *)DD_ALL_BASE_ADDR, IF_MEM_DD_TYPE_ALL);
    g_if_dd_emu.if_emu_dd_mem.mem_set_fp((uint8_t *)DD_UTIL1_BASE_ADDR, IF_MEM_DD_TYPE_UTIL1);
    g_if_dd_emu.if_emu_dd_mem.mem_set_fp((uint8_t *)DD_UTIL2_BASE_ADDR, IF_MEM_DD_TYPE_UTIL2);
}

