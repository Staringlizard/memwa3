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


#ifndef _SERV_MEM_H
#define _SERV_MEM_H

#include "stm32h7xx_hal.h"
#include "drv_sdram.h"
#include "serv_term.h"
#include <stdio.h>

#define CC_DISP_BUFFER0_ADDR   (SDRAM_ADDR)
#define CC_DISP_BUFFER1_ADDR   (CC_DISP_BUFFER0_ADDR + IF_MEMORY_CC_SCREEN_BUFFER0_SIZE)
#define CC_DISP_BUFFER2_ADDR   (CC_DISP_BUFFER1_ADDR + IF_MEMORY_CC_SCREEN_BUFFER1_SIZE)
#define CC_SPRITE0_BASE_ADDR   (CC_DISP_BUFFER2_ADDR + IF_MEMORY_CC_SCREEN_BUFFER2_SIZE)
#define CC_SPRITE1_BASE_ADDR   (CC_SPRITE0_BASE_ADDR + IF_MEMORY_CC_SPRITE0_SIZE)
#define CC_SPRITE2_BASE_ADDR   (CC_SPRITE1_BASE_ADDR + IF_MEMORY_CC_SPRITE1_SIZE)
#define CC_RAM_BASE_ADDR       (CC_SPRITE2_BASE_ADDR + IF_MEMORY_CC_SPRITE2_SIZE)

/* These can have same memory space, since they are loaded at different address */
#define CC_KROM_BASE_ADDR      (CC_RAM_BASE_ADDR + IF_MEMORY_CC_RAM_SIZE)
#define CC_BROM_BASE_ADDR      (CC_RAM_BASE_ADDR + IF_MEMORY_CC_RAM_SIZE)
#define CC_CROM_BASE_ADDR      (CC_RAM_BASE_ADDR + IF_MEMORY_CC_RAM_SIZE)

/* CC IO address needs to be unique, lets use RAM_3 for this memory */
#define CC_IO_BASE_ADDR        0x38000000
#define CC_UTIL0_BASE_ADDR     (CC_CROM_BASE_ADDR + IF_MEMORY_CC_CROM_SIZE)
#define CC_UTIL1_BASE_ADDR     (CC_UTIL0_BASE_ADDR + IF_MEMORY_CC_UTIL0_SIZE)
#define DD_ALL_BASE_ADDR       (CC_UTIL1_BASE_ADDR + IF_MEMORY_CC_UTIL1_SIZE)
#define DD_UTIL0_BASE_ADDR     (DD_ALL_BASE_ADDR + IF_MEMORY_DD_ALL_SIZE)
#define DD_UTIL1_BASE_ADDR     (DD_UTIL0_BASE_ADDR + IF_MEMORY_DD_UTIL0_SIZE)

/*
 * This can get all the remaining memory to support as
 * many filenames as possible.
 */
#define CC_FILES_SIZE    	   (0x1C0000)
#define CC_FILES_FILTERED_SIZE (0x40000)
#define CC_FILES_ADDR    	   (DD_UTIL1_BASE_ADDR + IF_MEMORY_DD_UTIL1_SIZE)
#define CC_FILES_FILTERED_ADDR (CC_FILES_ADDR + CC_FILES_SIZE)

#define CC_BROM_LOAD_ADDR      0x0000A000
#define CC_CROM_LOAD_ADDR      0x0000D000
#define CC_KROM_LOAD_ADDR      0x0000E000

#define DD_DOS_LOAD_ADDR       0x0000C000

void serv_mem_init();

#endif
