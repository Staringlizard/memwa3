/*
 * memwa2 joystick driver
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



#ifndef _DRV_I2C_H
#define _DRV_I2C_H

#include "stm32h7xx_hal.h"

void drv_i2c_init();
void drv_i2c_wr_reg(uint8_t addr, uint8_t reg, uint8_t val);
uint8_t drv_i2c_rd_reg(uint8_t addr, uint8_t reg);
void drv_i2c_set_reg(uint8_t addr, uint8_t reg, uint8_t bits_to_set);
void drv_i2c_clear_reg(uint8_t addr, uint8_t reg, uint8_t bits_to_clear);

#endif
