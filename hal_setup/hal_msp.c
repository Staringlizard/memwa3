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



/**
 * This file contains the main configuraiton for the hw.
 * Instead of doing all configuration in every single driver, it
 * is being done here by using "weak" keyword.
 * The file also contains the clock setup and cache etc. that is
 * being called by the startup sequence.
 */

#include "hal_msp.h"

void HAL_CRC_MspInit(CRC_HandleTypeDef *hcrc)
{
    __HAL_RCC_CRC_CLK_ENABLE();
}

void HAL_SD_MspInit(SD_HandleTypeDef *hsd)
{
    GPIO_InitTypeDef GPIO_Init;

    /* Enable GPIOs clock */
    __GPIOA_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();

    /* Configure sdcard functionality */
    __HAL_RCC_SDMMC1_CLK_ENABLE();

    GPIO_Init.Mode = GPIO_MODE_AF_PP;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_Init.Alternate = GPIO_AF12_SDMMC1;

    GPIO_Init.Pin = GPIO_PIN_8 |    /* SD DAT0 */
                    GPIO_PIN_9 |    /* SD DAT1 */
                    GPIO_PIN_10 |   /* SD DAT2 */
                    GPIO_PIN_11 |   /* SD DAT3 */
                    GPIO_PIN_12;    /* SD CLK */
    HAL_GPIO_Init(GPIOC, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_2;     /* SD CMD */
    HAL_GPIO_Init(GPIOD, &GPIO_Init);

    GPIO_Init.Mode = GPIO_MODE_INPUT;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_Init.Pin = GPIO_PIN_7;     /* R3 PA7 SD CD */
    HAL_GPIO_Init(GPIOA, &GPIO_Init);

    __HAL_RCC_SDMMC1_FORCE_RESET();
    __HAL_RCC_SDMMC1_RELEASE_RESET();
}

void  HAL_HCD_MspInit(HCD_HandleTypeDef *hhcd)
{
    GPIO_InitTypeDef GPIO_Init;

    /* Enable GPIOs clock */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_Init.Mode = GPIO_MODE_AF_PP;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_Init.Alternate = GPIO_AF12_OTG2_FS;

    GPIO_Init.Pin = GPIO_PIN_14 |    /* OTG_HS_DM */
                    GPIO_PIN_15;     /* OTG_HS_DP */
    HAL_GPIO_Init(GPIOB, &GPIO_Init);

    /* Enable USB HS Clocks */
    __HAL_RCC_USB1_OTG_HS_CLK_ENABLE();

    /* Set USBHS Interrupt to the lowest priority */
    HAL_NVIC_SetPriority(OTG_HS_IRQn, 5, 0);

    /* Enable USBHS Interrupt */
    HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
}

void HAL_PCD_MspInit(PCD_HandleTypeDef *hpcd)
{
    GPIO_InitTypeDef GPIO_Init;

    /* Configure USB FS GPIOs */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_Init.Mode = GPIO_MODE_AF_PP;
    GPIO_Init.Pull = GPIO_NOPULL;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_Init.Alternate = GPIO_AF10_OTG1_FS;

    /* Configure DM DP Pins */
    GPIO_Init.Pin = GPIO_PIN_11 | /* OTG_FS_DM */
                    GPIO_PIN_12;  /* OTG_FS_DP */
    HAL_GPIO_Init(GPIOA, &GPIO_Init); 

    /* Enable USB FS Clock */
    __HAL_RCC_USB2_OTG_FS_CLK_ENABLE();

    /* Set USBFS Interrupt priority */
    HAL_NVIC_SetPriority(OTG_FS_IRQn, 6, 0);

    /* Enable USBFS Interrupt */
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef GPIO_Init;

    /* Enable GPIOs clock */
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /* Enable I2C clock */
    __HAL_RCC_I2C4_CLK_ENABLE();

    GPIO_Init.Mode = GPIO_MODE_AF_OD;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_Init.Alternate = GPIO_AF4_I2C4;

    GPIO_Init.Pin = GPIO_PIN_12 |    /* I2C4_SCL */
                    GPIO_PIN_13;     /* I2C4_SDA */
    HAL_GPIO_Init(GPIOD, &GPIO_Init);
}

void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram)
{
    GPIO_InitTypeDef GPIO_Init;

    /* Enable GPIOs clock */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    /* Enable FMC clock */
    __HAL_RCC_FMC_CLK_ENABLE();

    GPIO_Init.Mode = GPIO_MODE_AF_PP;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_Init.Alternate = GPIO_AF12_FMC;
  
    GPIO_Init.Pin = GPIO_PIN_2 | /* M4 PC2 FMC_SDNE0 */
                    GPIO_PIN_3; /* M5 PC3 FMC_SDCKE0 */
    HAL_GPIO_Init(GPIOC, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_0 | /* B12 PD0 FMC_D2 */
                    GPIO_PIN_1 | /* C12 PD1 FMC_D3 */
                    GPIO_PIN_8 | /* P15 PD8 FMC_D13 */
                    GPIO_PIN_9 | /* P14 PD9 FMC_D14 */
                    GPIO_PIN_10 | /* N15 PD10 FMC_D15 */
                    GPIO_PIN_14 | /* M14 PD14 FMC_D0 */
                    GPIO_PIN_15; /* L14 PD15 FMC_D1 */
    HAL_GPIO_Init(GPIOD, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_0 | /* A4 PE0 FMC_NBL0 */
                    GPIO_PIN_1 | /* A3 PE1 FMC_NBL1 */
                    GPIO_PIN_7 | /* R8 PE7 FMC_D4 */
                    GPIO_PIN_8 | /* P8 PE8 FMC_D5 */
                    GPIO_PIN_9 | /* P9 PE9 FMC_D6 */
                    GPIO_PIN_10 | /* R9 PE10 FMC_D7 */
                    GPIO_PIN_11 | /* P10 PE11 FMC_D8 */
                    GPIO_PIN_12 | /* R10 PE12 FMC_D9 */
                    GPIO_PIN_13 | /* N11 PE13 FMC_D10 */
                    GPIO_PIN_14 | /* P11 PE14 FMC_D11 */
                    GPIO_PIN_15; /* R11 PE15 FMC_D12 */
    HAL_GPIO_Init(GPIOE, &GPIO_Init);
    
    GPIO_Init.Pin = GPIO_PIN_0 | /* E2 PF0 FMC_A0 */
                    GPIO_PIN_1 | /* H3 PF1 FMC_A1 */
                    GPIO_PIN_2 | /* H2 PF2 FMC_A2 */
                    GPIO_PIN_3 | /* J2 PF3 FMC_A3 */
                    GPIO_PIN_4 | /* J3 PF4 FMC_A4 */
                    GPIO_PIN_5 | /* K3 PF5 FMC_A5 */
                    GPIO_PIN_11 | /* R6 PF11 FMC_SDNRAS */
                    GPIO_PIN_12 | /* P6 PF12 FMC_A6 */
                    GPIO_PIN_13 | /* N6 PF13 FMC_A7 */
                    GPIO_PIN_14 | /* R7 PF14 FMC_A8 */
                    GPIO_PIN_15; /* P7 PF15 FMC_A9 */
    HAL_GPIO_Init(GPIOF, &GPIO_Init);   

    GPIO_Init.Pin = GPIO_PIN_0 | /* N7 PG0 FMC_A10 */
                    GPIO_PIN_1 | /* M7 PG1 FMC_A11 */
                    GPIO_PIN_4 | /* K14 PG4 FMC_A14 */
                    GPIO_PIN_5 | /* K13 PG5 FMC_A15 */
                    GPIO_PIN_8 | /* H14 PG8 FMC_SDCLK */
                    GPIO_PIN_15; /* B7 PG15 FMC_SDNCAS */
    HAL_GPIO_Init(GPIOG, &GPIO_Init);  
    
    GPIO_Init.Pin = GPIO_PIN_5; /* J4 PH5 FMC_SDNWE */
    HAL_GPIO_Init(GPIOH, &GPIO_Init);  
}

void HAL_LTDC_MspInit(LTDC_HandleTypeDef *hltdc)
{
    GPIO_InitTypeDef GPIO_Init;

    /* Enable the LTDC and DMA2D clocks */
    __HAL_RCC_LTDC_CLK_ENABLE();

    /* Enable GPIOs clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();

    GPIO_Init.Mode = GPIO_MODE_AF_PP;
    GPIO_Init.Pull = GPIO_NOPULL;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;

    /* Watch out, LTDC has 2 alternative func mappings */
    GPIO_Init.Alternate = GPIO_AF14_LTDC;

    GPIO_Init.Pin = GPIO_PIN_1 | /* N2 PA1 LCD_R2 */
                    GPIO_PIN_2 | /* P2 PA2 LCD_R1 */
                    GPIO_PIN_3 | /* R2 PA3 LCD_B5 */
                    GPIO_PIN_4 | /* N4 PA4 LCD_VSYNC */
                    GPIO_PIN_5 | /* P4 PA5 LCD_R4 */
                    GPIO_PIN_6; /* P3 PA6 LCD_G2 */
    HAL_GPIO_Init(GPIOA, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_8 | /* A5 PB8 LCD_B6 */
                    GPIO_PIN_9 | /* B4 PB9 LCD_B7 */
                    GPIO_PIN_10 | /* R12 PB10 LCD_G4 */
                    GPIO_PIN_11; /* R13 PB11 LCD_G5 */
    HAL_GPIO_Init(GPIOB, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_0 | /* M2 PC0 LCD_R5 */
                    GPIO_PIN_6 | /* H15 PC6 LCD_HSYNC */
                    GPIO_PIN_7; /* G15 PC7 LCD_G6 */
    HAL_GPIO_Init(GPIOC, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_6; /* B11 PD6 LCD_B2 */
    HAL_GPIO_Init(GPIOD, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_4 | /* B1 PE4 LCD_B0 */
                    GPIO_PIN_5 | /* B2 PE5 LCD_G0 */
                    GPIO_PIN_6;/* B3 PE6 LCD_G1 */
    HAL_GPIO_Init(GPIOE, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_10; /* L1 PF10 LCD_DE */
    HAL_GPIO_Init(GPIOF, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_6 | /* J15 PG6 LCD_R7 */
                    GPIO_PIN_7 | /* J14 PG7 LCD_CLK */
                    GPIO_PIN_11 | /* B9 PG11 LCD_B3 */
                    GPIO_PIN_12; /* B8 PG12 LCD_B1 */ 
    HAL_GPIO_Init(GPIOG, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_2; /* F4 PH2 LCD_R0 */
    HAL_GPIO_Init(GPIOH, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_2 | /* C14 PI2 LCD_G7 */
                    GPIO_PIN_4; /* D4 PI4 LCD_B4 */
    HAL_GPIO_Init(GPIOI, &GPIO_Init);


    GPIO_Init.Alternate = GPIO_AF9_LTDC;

    GPIO_Init.Pin = GPIO_PIN_10; /* B10 PG10 LCD_G3 */

    HAL_GPIO_Init(GPIOG, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_0 | /* R5 PB0 LCD_R3 */
                    GPIO_PIN_1; /* R4 PB1 LCD_R6 */
    HAL_GPIO_Init(GPIOB, &GPIO_Init);

    /* Set LTDC Interrupt priority */
    HAL_NVIC_SetPriority(LTDC_IRQn, 0xE, 0);
    HAL_NVIC_SetPriority(LTDC_ER_IRQn, 0xD, 0);
    
    /* Enable LTDC Interrupt */
    HAL_NVIC_EnableIRQ(LTDC_IRQn);
    HAL_NVIC_EnableIRQ(LTDC_ER_IRQn);
}

void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng)
{
  /* RNG Peripheral clock enable */
  __HAL_RCC_RNG_CLK_ENABLE();
}

void HAL_SIDBUS_MspInit() /* Memwa2 specific */
{
    GPIO_InitTypeDef GPIO_Init;

    /* Enable GPIOs clock */
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Pull = GPIO_NOPULL;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;

    /* Sidbus data lines */
    GPIO_Init.Pin = GPIO_PIN_8 | /* M12 PH8 SIDBUS D0 */
                    GPIO_PIN_9 | /* M13 PH9 SIDBUS D1 */
                    GPIO_PIN_10 | /* L13 PH10 SIDBUS D2 */
                    GPIO_PIN_11 | /* L12 PH11 SIDBUS D3 */
                    GPIO_PIN_12 | /* K12 PH12 SIDBUS D4 */
                    GPIO_PIN_13 | /* E12 PH13 SIDBUS D5 */
                    GPIO_PIN_14 | /* E13 PH14 SIDBUS D6 */
                    GPIO_PIN_15; /* D13 PH15 SIDBUS D7 */
    HAL_GPIO_Init(GPIOH, &GPIO_Init);

    /* Sidbus address and ctrl lines */
    GPIO_Init.Pin = GPIO_PIN_5 | /* C4 PI5 SIDBUS RW */
                    GPIO_PIN_6 | /* C3 PI6 SIDBUS CS */
                    GPIO_PIN_7 | /* C2 PI7 SIDBUS A0 */
                    GPIO_PIN_8 | /* D2 PI8 SIDBUS A1 */ 
                    GPIO_PIN_9 | /* D3 PI9 SIDBUS A2 */
                    GPIO_PIN_10 | /* E3 PI10 SIDBUS A3 */
                    GPIO_PIN_11; /* E4 PI11 SIDBUS A4 */
    HAL_GPIO_Init(GPIOI, &GPIO_Init);

    /* Sidbus clock external interrupt */
    GPIO_Init.Mode = GPIO_MODE_IT_FALLING;
    GPIO_Init.Pull = GPIO_NOPULL;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_Init.Pin = GPIO_PIN_13; /* A8 PG13 SIDBUS CLK */
    HAL_GPIO_Init(GPIOG, &GPIO_Init);

    EXTI->FTSR1 &= ~GPIO_PIN_13; /* Disable sidbus irq directly */

    /*
     * Enable and set interrupt.
     * This irq line is connected directly to sid clk.
     * This is needed since it is only possible to write when
     * clk is high.
     */
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 2);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void HAL_JOYST_MspInit() /* Memwa2 specific */
{
    GPIO_InitTypeDef GPIO_Init;

    /* Enable GPIOs clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    GPIO_Init.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_Init.Pin = GPIO_PIN_10; /* D15 PA10 JOY_A5 FIRE */
    HAL_GPIO_Init(GPIOA, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_5; /* A6 PB5 JOY_A3 LEFT */
    HAL_GPIO_Init(GPIOB, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_2 | /* A2 PE2 JOY_A1 UP */
                    GPIO_PIN_3; /* A1 PE3 JOY_A2 DOWN */
    HAL_GPIO_Init(GPIOE, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_6 | /* K2 PF6 JOY_B3 LEFT */
                    GPIO_PIN_7 | /* K1 PF7 JOY_B4 RIGHT */
                    GPIO_PIN_8 | /* L3 PF8 JOY_B1 UP */
                    GPIO_PIN_9; /* L2 PF9 JOY_B2 DOWN */
    HAL_GPIO_Init(GPIOF, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_14; /* A7 PG14 JOY_A4 RIGHT */
    HAL_GPIO_Init(GPIOG, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_4; /* H4 PH4 JOY_B5 FIRE */
    HAL_GPIO_Init(GPIOH, &GPIO_Init);


    /* Enable and set interrupt for joystick A and B */
    HAL_NVIC_SetPriority(EXTI2_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);
    HAL_NVIC_SetPriority(EXTI3_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);
    HAL_NVIC_SetPriority(EXTI4_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void HAL_LED_MspInit() /* Memwa2 specific */
{
    GPIO_InitTypeDef GPIO_Init;
    
    /* Enable GPIOs clock */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_Init.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_Init.Pin = GPIO_PIN_13 |    /* D1 PC13 LED_R1 */
                    GPIO_PIN_14 |    /* E1 PC14 LED_G1 */
                    GPIO_PIN_15;     /* F1 PC15 LED_B1 */
    HAL_GPIO_Init(GPIOC, &GPIO_Init);
}
