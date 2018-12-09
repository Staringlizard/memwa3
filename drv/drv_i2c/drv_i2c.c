/*
 * i2c driver
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

#include "drv_i2c.h"
#include "stm32h7xx_hal_i2c.h"

#define I2C_OWN_ADDRESS    0x00
//#define I2C_TIMING       0x40912732
#define I2C_TIMING       0x10C0ECFF

static I2C_HandleTypeDef g_i2c_handle;

void drv_i2c_init()
{
    HAL_StatusTypeDef ret = HAL_OK;

    g_i2c_handle.Instance = I2C4;
    g_i2c_handle.Init.Timing = I2C_TIMING;
    g_i2c_handle.Init.OwnAddress1 = I2C_OWN_ADDRESS;
    g_i2c_handle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    g_i2c_handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    g_i2c_handle.Init.OwnAddress2 = I2C_OWN_ADDRESS;
    g_i2c_handle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    g_i2c_handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    g_i2c_handle.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    ret = HAL_I2C_Init(&g_i2c_handle);
    if(ret != HAL_OK)
    {
        main_error("Failed to initialize I2C!", __FILE__, __LINE__, ret);
    }

    HAL_I2CEx_ConfigAnalogFilter(&g_i2c_handle, I2C_ANALOGFILTER_ENABLE);
}

void drv_i2c_wr_reg(uint8_t addr, uint8_t reg, uint8_t val)
{
    if(HAL_I2C_Mem_Write(&g_i2c_handle, addr, reg, 1, &val, 1, 2000) != HAL_OK)
    {
        main_error("Failed to write I2C register!", __FILE__, __LINE__, reg);
    }
}

uint8_t drv_i2c_rd_reg(uint8_t addr, uint8_t reg)
{
    uint8_t val;

    if(HAL_I2C_Mem_Read(&g_i2c_handle, addr, reg, 1, &val, 1, 2000) != HAL_OK)
    {
        main_error("Failed to read I2C register!", __FILE__, __LINE__, reg);
    }

    return val;
}

void drv_i2c_set_reg(uint8_t addr, uint8_t reg, uint8_t bits_to_set)
{
    drv_i2c_wr_reg(addr, reg, drv_i2c_rd_reg(addr, reg) | bits_to_set);
}

void drv_i2c_clear_reg(uint8_t addr, uint8_t reg, uint8_t bits_to_clear)
{
    drv_i2c_wr_reg(addr, reg, drv_i2c_rd_reg(addr, reg) & ~bits_to_clear);
}
