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


/**
 * Handles all communication with the HDMI transmitter.
 */

#include "tda19988.h"
#include "config.h"
#include "stm32h7xx_hal_i2c.h"

#define I2C_CEC_ADDRESS    0x68
#define I2C_HDMI_ADDRESS   0xE0
//#define I2C_ADDRESS      0x70
#define I2C_OWN_ADDRESS    0x00
//#define I2C_TIMING       0x40912732
#define I2C_TIMING       0x10C0ECFF

/* The device has two I2C interfaces CEC (0x34) and HDMI (0x70). This driver
 * needs access to both.
 */

/*
 * CEC - Register and Bit Definitions
 */

#define CEC_STATUS_REG 0xfe
#define CEC_STATUS_CONNECTED_MASK 0x02

#define CEC_ENABLE_REG 0xff
#define CEC_ENABLE_ALL_MASK 0x87

/*
 * HDMI - Pages
 *
 * The HDMI part is much bigger than the CEC part. Memory is accessed according
 * to page and address. Once the page is set, only the address needs to be
 * sent if accessing memory locations within the same page (you don't need to
 * send the page number every time).
 */

#define HDMI_CTRL_PAGE 0x00
#define HDMI_PPL_PAGE 0x02
#define HDMI_EDID_PAGE 0x09
#define HDMI_INFO_PAGE 0x10
#define HDMI_AUDIO_PAGE 0x11
#define HDMI_HDCP_OTP_PAGE 0x12
#define HDMI_GAMUT_PAGE 0x13

/* 
 * The page select register isn't part of a page. A dummy value of 0xff is
 * used to signfiy this in the code.
 */
#define HDMI_PAGELESS 0xff

/*
 * Control Page Registers and Bit Definitions
 */

#define HDMI_CTRL_REV_LO_REG 0x00
#define HDMI_CTRL_REV_HI_REG 0x02

#define HDMI_CTRL_RESET_REG 0x0a
#define HDMI_CTRL_RESET_DDC_MASK 0x02

#define HDMI_CTRL_DDC_CTRL_REG 0x0b
#define HDMI_CTRL_DDC_EN_MASK 0x00

#define HDMI_CTRL_DDC_CLK_REG 0x0c
#define HDMI_CTRL_DDC_CLK_EN_MASK 0x01

#define HDMI_CTRL_INTR_CTRL_REG 0x0f
#define HDMI_CTRL_INTR_EN_GLO_MASK 0x04

#define HDMI_CTRL_INT_REG 0x11
#define HDMI_CTRL_INT_EDID_MASK 0x02

/*
 * EDID Page Registers and Bit Definitions
 */

#define HDMI_EDID_DATA_REG 0x00

#define HDMI_EDID_DEV_ADDR_REG 0xfb
#define HDMI_EDID_DEV_ADDR 0xa0

#define HDMI_EDID_OFFSET_REG 0xfc
#define HDMI_EDID_OFFSET 0x00

#define HDMI_EDID_SEG_PTR_ADDR_REG 0xfc
#define HDMI_EDID_SEG_PTR_ADDR 0x00

#define HDMI_EDID_SEG_ADDR_REG 0xfe
#define HDMI_EDID_SEG_ADDR 0x00

#define HDMI_EDID_REQ_REG 0xfa
#define HDMI_EDID_REQ_READ_MASK 0x01

/*
 * HDCP and OTP
 */
#define HDMI_HDCP_OTP_DDC_CLK_REG 0x9a
#define HDMI_HDCP_OTP_DDC_CLK_MASK 0x27

/* this register/mask isn't documented but it has to be cleared/set */
#define HDMI_HDCP_OTP_SOME_REG 0x9b
#define HDMI_HDCP_OTP_SOME_MASK 0x02

/*
 * Pageless Registers
 */

#define HDMI_PAGE_SELECT_REG 0xff

static I2C_HandleTypeDef g_i2c_handle;

__weak void HAL_TDA19988_MspInit()
{
    ;
}

void tda19988_init()
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

    HAL_TDA19988_MspInit(); /* Configure IRQ */
}

void tda19988_configure()
{

}

void tda19988_set_page(uint8_t page)
{

}

void tda19988_wr_reg(tda19988_addr_t addr, uint8_t reg, uint8_t val)
{
    uint16_t i2c_addr;

    if(addr == TDA19988_ADDR_CEC)
    {
        i2c_addr = (uint16_t)I2C_CEC_ADDRESS;
    }
    else
    {
        i2c_addr = (uint16_t)I2C_HDMI_ADDRESS;
    }

    if(HAL_I2C_Master_WriteReg(&g_i2c_handle, i2c_addr, &reg, &val, 2000) != HAL_OK)
    {
        main_error("Failed to write I2C register!", __FILE__, __LINE__, reg);
    }
}

uint8_t tda19988_rd_reg(tda19988_addr_t addr, uint8_t reg)
{
    uint8_t val;
    uint16_t i2c_addr;

    if(addr == TDA19988_ADDR_CEC)
    {
        i2c_addr = (uint16_t)I2C_CEC_ADDRESS;
    }
    else
    {
        i2c_addr = (uint16_t)I2C_HDMI_ADDRESS;
    }

    //if(HAL_I2C_Master_ReadReg(&g_i2c_handle, (uint16_t)I2C_ADDRESS, &reg, &val, 2000) != HAL_OK)
    if(HAL_I2C_Mem_Read(&g_i2c_handle, i2c_addr, reg, 1, &val, 1, 2000) != HAL_OK)
    {
        main_error("Failed to read I2C register!", __FILE__, __LINE__, reg);
    }

    return val;
}

void tda19988_set_bits(tda19988_addr_t addr, uint8_t reg, uint8_t bits_to_set)
{
    tda19988_wr_reg(addr, reg, tda19988_rd_reg(addr, reg) | bits_to_set);
}

void tda19988_clear_bits(tda19988_addr_t addr, uint8_t reg, uint8_t bits_to_clear)
{
    tda19988_wr_reg(addr, reg, tda19988_rd_reg(addr, reg) & ~bits_to_clear);
}

void tda19988_irq()
{
    uint8_t val;

}
