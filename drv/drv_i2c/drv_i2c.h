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

typedef enum
{
	DRV_I2C_STATUS_OK,
	DRV_I2C_STATUS_ERROR
} drv_i2c_status_t;

typedef enum
{
	DRV_I2C_OP_SET,
	DRV_I2C_OP_CLEAR
} drv_i2c_op_t;

drv_i2c_status_t drv_i2c_init();
drv_i2c_status_t drv_i2c_wr_reg_8(uint8_t addr, uint8_t reg, uint8_t val);
drv_i2c_status_t drv_i2c_wr_reg_16(uint8_t addr, uint8_t reg, uint16_t val);
drv_i2c_status_t drv_i2c_rd_reg_8(uint8_t addr, uint8_t reg, uint8_t *value);
drv_i2c_status_t drv_i2c_mod_reg_8(uint8_t addr, uint8_t reg, uint8_t mask, drv_i2c_op_t op);
drv_i2c_status_t drv_i2c_rd_blk(uint8_t addr, uint8_t reg, uint8_t *buf_p, uint8_t len);

#endif
