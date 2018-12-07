/*
 * memwa2 tda19988 driver
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


#ifndef _TDA19988_H
#define _TDA19988_H

#include "stm32h7xx_hal.h"
#include "main.h"

typedef enum
{
	TDA19988_ADDR_HDMI,
	TDA19988_ADDR_CEC,
} tda19988_addr_t;

void tda19988_init();
void tda19988_configure();
void tda19988_wr_reg(tda19988_addr_t addr, uint8_t reg, uint8_t val);
uint8_t tda19988_rd_reg(tda19988_addr_t addr, uint8_t reg);
void tda19988_set_reg(tda19988_addr_t addr, uint8_t reg, uint8_t bits_to_set);
void tda19988_clear_reg(tda19988_addr_t addr, uint8_t reg, uint8_t bits_to_clear);
void tda19988_irq();

#endif
