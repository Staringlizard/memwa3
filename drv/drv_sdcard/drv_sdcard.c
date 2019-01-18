/*
 * memwa2 sdcard driver
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
 * Responsible for SDRAM.
 */

#include "hal_conf.h"
#include "drv_sdcard.h"
#include "serv_term.h"

SD_HandleTypeDef g_sd_handle;

void drv_sdcard_init()
{
    uint32_t res = HAL_SD_ERROR_NONE;

    g_sd_handle.Instance = SDMMC1;
    HAL_SD_DeInit(&g_sd_handle);

    g_sd_handle.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    g_sd_handle.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    g_sd_handle.Init.BusWide = SDMMC_BUS_WIDE_4B;
    g_sd_handle.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
    g_sd_handle.Init.ClockDiv = 4; /* 4 Gives 25Mhz clk */
    res = HAL_SD_Init(&g_sd_handle);

    if(res != HAL_SD_ERROR_NONE)
    {
        serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "failed to initiate sdcard (inserted?) errno %ld!", res);
    }
}

uint8_t drv_sdcard_inserted()
{
    if((SDCARD_CD_PORT->IDR & SDCARD_CD_PIN) != (uint32_t)GPIO_PIN_RESET)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
