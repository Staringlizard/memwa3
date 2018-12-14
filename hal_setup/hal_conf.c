/*
 * memwa2 configuration
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

#include "hal_conf.h"
#include "drv_sdram.h"

void hal_conf_clk()
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /*
     * Supply configuration update enable
     */
    MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);

    /*
     * The voltage scaling allows optimizing the power consumption when the device is
     * clocked below the maximum system frequency, to update the voltage scaling value
     * regarding system frequency refer to product datasheet.
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    while ((PWR->D3CR & (PWR_D3CR_VOSRDY)) != PWR_D3CR_VOSRDY) {;}

    __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);

    /* 
     * Initializes the CPU, AHB and APB busses clocks 
     *
     * Below givs:
     * SysClk:      400Mhz
     * CPU1:        400Mhz
     * CPU2:        200Mhz
     * AXI:         200Mhz
     * HCLK3:       200Mhz
     * APB3:        100Mhz
     * AHB 1,2:     200Mhz
     * APB1:        100Mhz
     * APB1 Timer:  200Mhz
     * APB2:        100Mhz
     * APB2 Timer:  200Mhz
     * AHB4:        200Mhz
     * APB4:        100Mhz
     *
     * RNG:         48Mhz
     * I2C4:        100Mhz
     * FMC:         200Mhz
     * LTDC:        40Mhz
     * SDMMC:       200Mhz
     * USB:         48Mhz
     */

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 100;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        while(1) {;}
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        while(1) {;}
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_RNG
                              |RCC_PERIPHCLK_SDMMC|RCC_PERIPHCLK_I2C4
                              |RCC_PERIPHCLK_USB|RCC_PERIPHCLK_FMC;
    PeriphClkInitStruct.PLL3.PLL3M = 2;
    PeriphClkInitStruct.PLL3.PLL3N = 20;
    PeriphClkInitStruct.PLL3.PLL3P = 2;
    PeriphClkInitStruct.PLL3.PLL3Q = 2;
    PeriphClkInitStruct.PLL3.PLL3R = 2;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_2;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_D1HCLK;
    PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
    PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_HSI48;
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
    PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_D3PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        while(1) {;}
    }

    /* Activate CSI clock mondatory for I/O Compensation Cell */
    __HAL_RCC_CSI_ENABLE();

    /* Enable SYSCFG clock mondatory for I/O Compensation Cell */
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    /* Enables the I/O Compensation Cell */
    HAL_EnableCompensationCell();

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 1, 1);
}

void hal_conf_mpu()
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

void hal_conf_cache_inst_on()
{
    SCB_EnableICache();
}

void hal_conf_cache_data_on()
{
    SCB_EnableDCache();
}

void hal_conf_tda19988()
{
    GPIO_InitTypeDef GPIO_Init;

    /* Enable GPIOs clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_Init.Mode = GPIO_MODE_IT_FALLING;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_Init.Pin = GPIO_PIN_15; /* A13 PA15 TDA19988 IRQ */
    HAL_GPIO_Init(GPIOA, &GPIO_Init);

    /* Enable and set interrupt for tda19988 */
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    /* Enable 1.8 Volt */
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_Init.Pin = GPIO_PIN_9; /* E15 PA9 1.8V_EN */
    HAL_GPIO_Init(GPIOA, &GPIO_Init);
}

void hal_conf_is42s16400j()
{
    /* Swap so that SDRAM can be found at 0x60000000 instead of 0xC0000000 */
    HAL_SetFMCMemorySwappingConfig(FMC_SWAPBMAP_SDRAM_SRAM);
}

void hal_conf_mos658x()
{
    GPIO_InitTypeDef GPIO_Init;

    /* Enable GPIOs clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();

	/* Enable sid clk */
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_Init.Pin = GPIO_PIN_8; /* F15 PA8 SID_CLK_EN */
    HAL_GPIO_Init(GPIOA, &GPIO_Init);

    PWR_EN_SID_CLK();
}
