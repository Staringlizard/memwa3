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

I2C_HandleTypeDef g_i2c_handle;

drv_i2c_status_t drv_i2c_init()
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
        serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "Failed to initialize I2C (error %d)!", ret);
        return DRV_I2C_STATUS_ERROR;
    }

    HAL_I2CEx_ConfigAnalogFilter(&g_i2c_handle, I2C_ANALOGFILTER_ENABLE);

    return DRV_I2C_STATUS_OK;
}

drv_i2c_status_t drv_i2c_wr_reg_16(uint8_t addr, uint8_t reg, uint16_t val)
{
    HAL_StatusTypeDef ret = HAL_OK;
    uint8_t array[2];

    /* Get endiness right */
    array[0] = val >> 8;
    array[1] = val & 0xFF;

    ret = HAL_I2C_Mem_Write(&g_i2c_handle, addr, reg, 1, array, 2, 2000);
    if(ret != HAL_OK)
    {
        serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "Failed to write I2C register (hal: %d, i2c: %d)!", ret, g_i2c_handle.ErrorCode);
        return DRV_I2C_STATUS_ERROR;
    }

    return DRV_I2C_STATUS_OK;
}

drv_i2c_status_t drv_i2c_wr_reg_8(uint8_t addr, uint8_t reg, uint8_t val)
{
    HAL_StatusTypeDef ret = HAL_OK;

    ret = HAL_I2C_Mem_Write(&g_i2c_handle, addr, reg, 1, &val, 1, 2000);
    if(ret != HAL_OK)
    {
        serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "Failed to write I2C register (hal: %d, i2c: %d)!", ret, g_i2c_handle.ErrorCode);
        return DRV_I2C_STATUS_ERROR;
    }

    return DRV_I2C_STATUS_OK;
}

drv_i2c_status_t drv_i2c_rd_reg_8(uint8_t addr, uint8_t reg, uint8_t *value_p)
{
    HAL_StatusTypeDef ret = HAL_OK;

    ret = HAL_I2C_Mem_Read(&g_i2c_handle, addr, reg, 1, value_p, 1, 2000);
    if(ret != HAL_OK)
    {
        serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "Failed to read I2C register (hal: %d, i2c: %d)!", ret, g_i2c_handle.ErrorCode);
        return DRV_I2C_STATUS_ERROR;
    }

    return DRV_I2C_STATUS_OK;
}

drv_i2c_status_t drv_i2c_mod_reg_8(uint8_t addr, uint8_t reg, uint8_t mask, drv_i2c_op_t op)
{
    HAL_StatusTypeDef ret = HAL_OK;
    uint8_t value = 0;
    uint8_t mod_value = 0;

    ret = HAL_I2C_Mem_Read(&g_i2c_handle, addr, reg, 1, &value, 1, 2000);
    if(ret != HAL_OK)
    {
        serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "Failed to read I2C register (hal: %d, i2c: %d)!", ret, g_i2c_handle.ErrorCode);
        return DRV_I2C_STATUS_ERROR;
    }

    if(op == DRV_I2C_OP_SET)
    {
        mod_value = value | mask;
    }
    else if(op == DRV_I2C_OP_CLEAR)
    {
        mod_value = value & ~mask;
    }

    ret = HAL_I2C_Mem_Write(&g_i2c_handle, addr, reg, 1, &mod_value, 1, 2000);
    if(ret != HAL_OK)
    {
        serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "Failed to write I2C register (hal: %d, i2c: %d)!", ret, g_i2c_handle.ErrorCode);
        return DRV_I2C_STATUS_ERROR;
    }

    return DRV_I2C_STATUS_OK;
}

drv_i2c_status_t drv_i2c_rd_blk(uint8_t addr, uint8_t reg, uint8_t *buf_p, uint8_t len)
{
    HAL_StatusTypeDef ret = HAL_OK;

    ret = HAL_I2C_Mem_Read(&g_i2c_handle, addr, reg, 1, buf_p, len, 2000);
    if(ret != HAL_OK)
    {
        serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "Failed to read I2C blk (hal: %d, i2c: %d)!", ret, g_i2c_handle.ErrorCode);
        return DRV_I2C_STATUS_ERROR;
    }

    return DRV_I2C_STATUS_OK;
}
