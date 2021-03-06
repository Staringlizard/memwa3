/*
 * memwa2 bootloader
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


#include "bl.h"
#include "drv_ltdc.h"
#include "drv_led.h"
#include "drv_sdcard.h"
#include "ff.h"
#include "stm32h7xx_hal_flash.h"
#include "stm32h7xx_hal_rcc.h"

#define BUFFER_SIZE        32

/* Base address of the Flash sectors Bank 1 */
#define ADDR_FLASH_SECTOR_0_BANK1     ((uint32_t)0x08000000) /* Base @ of Sector 0, 128 Kbytes */
#define ADDR_FLASH_SECTOR_1_BANK1     ((uint32_t)0x08020000) /* Base @ of Sector 1, 128 Kbytes */
#define ADDR_FLASH_SECTOR_2_BANK1     ((uint32_t)0x08040000) /* Base @ of Sector 2, 128 Kbytes */
#define ADDR_FLASH_SECTOR_3_BANK1     ((uint32_t)0x08060000) /* Base @ of Sector 3, 128 Kbytes */
#define ADDR_FLASH_SECTOR_4_BANK1     ((uint32_t)0x08080000) /* Base @ of Sector 4, 128 Kbytes */
#define ADDR_FLASH_SECTOR_5_BANK1     ((uint32_t)0x080A0000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6_BANK1     ((uint32_t)0x080C0000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7_BANK1     ((uint32_t)0x080E0000) /* Base @ of Sector 7, 128 Kbytes */

/* Base address of the Flash sectors Bank 2 */
#define ADDR_FLASH_SECTOR_0_BANK2     ((uint32_t)0x08100000) /* Base @ of Sector 0, 128 Kbytes */
#define ADDR_FLASH_SECTOR_1_BANK2     ((uint32_t)0x08120000) /* Base @ of Sector 1, 128 Kbytes */
#define ADDR_FLASH_SECTOR_2_BANK2     ((uint32_t)0x08140000) /* Base @ of Sector 2, 128 Kbytes */
#define ADDR_FLASH_SECTOR_3_BANK2     ((uint32_t)0x08160000) /* Base @ of Sector 3, 128 Kbytes */
#define ADDR_FLASH_SECTOR_4_BANK2     ((uint32_t)0x08180000) /* Base @ of Sector 4, 128 Kbytes */
#define ADDR_FLASH_SECTOR_5_BANK2     ((uint32_t)0x081A0000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6_BANK2     ((uint32_t)0x081C0000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7_BANK2     ((uint32_t)0x081E0000) /* Base @ of Sector 7, 128 Kbytes */

#define FLASH_USER_START_ADDR   ADDR_FLASH_SECTOR_1_BANK1
#define FW_PATH_AND_NAME        "0:/target.bin"
#define FW_PATH                 "0:/"

typedef void (*jump_function_t)();

static uint32_t get_sector(uint32_t addr)
{
  uint32_t sector = 0;

  if((addr < ADDR_FLASH_SECTOR_1_BANK1) && (addr >= ADDR_FLASH_SECTOR_0_BANK1))
  {
    sector = FLASH_SECTOR_0;
  }
  else if((addr < ADDR_FLASH_SECTOR_2_BANK1) && (addr >= ADDR_FLASH_SECTOR_1_BANK1))
  {
    sector = FLASH_SECTOR_1;
  }
  else if((addr < ADDR_FLASH_SECTOR_3_BANK1) && (addr >= ADDR_FLASH_SECTOR_2_BANK1))
  {
    sector = FLASH_SECTOR_2;
  }
  else if((addr < ADDR_FLASH_SECTOR_4_BANK1) && (addr >= ADDR_FLASH_SECTOR_3_BANK1))
  {
    sector = FLASH_SECTOR_3;
  }
  else if((addr < ADDR_FLASH_SECTOR_5_BANK1) && (addr >= ADDR_FLASH_SECTOR_4_BANK1))
  {
    sector = FLASH_SECTOR_4;
  }
  else if((addr < ADDR_FLASH_SECTOR_6_BANK1) && (addr >= ADDR_FLASH_SECTOR_5_BANK1))
  {
    sector = FLASH_SECTOR_5;
  }
  else if((addr < ADDR_FLASH_SECTOR_7_BANK1) && (addr >= ADDR_FLASH_SECTOR_6_BANK1))
  {
    sector = FLASH_SECTOR_6;
  }
  else /* (addr < FLASH_END_ADDR) && (addr >= ADDR_FLASH_SECTOR_7) */
  {
    sector = FLASH_SECTOR_7;
  }
  return sector;
}

static void erase_flash()
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t first_sector = 0;
    uint32_t nbr_sectors = 0;
    uint32_t error_sector = 0;
    HAL_StatusTypeDef res_flash;

    first_sector = get_sector(FLASH_USER_START_ADDR);
    nbr_sectors = get_sector(ADDR_FLASH_SECTOR_7_BANK1) - first_sector + 1;

    EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Banks         = FLASH_BANK_1;
    EraseInitStruct.Sector        = first_sector;
    EraseInitStruct.NbSectors     = nbr_sectors;

    /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
     DCRST and ICRST bits in the FLASH_CR register. */
    res_flash = HAL_FLASHEx_Erase(&EraseInitStruct, &error_sector);
    if(res_flash != HAL_OK)
    {
        while(1) {;}
    }
}

static void jump_to_application()
{
    uint32_t jump_addr = *(uint32_t *)(FLASH_USER_START_ADDR + 4);
    jump_function_t jump_function = (jump_function_t)jump_addr;

    HAL_RCC_DeInit();
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    __set_MSP(*(uint32_t*)FLASH_USER_START_ADDR);

    jump_function();

}

static void config_clks()
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

static void flash_file(FIL *fd_p, char *file_p)
{
    HAL_StatusTypeDef res_flash;
    FRESULT res_fs;

    uint8_t buffer_p[BUFFER_SIZE];
    uint32_t bytes_read = 0;
    uint32_t read_cnt = 0;

    __set_PRIMASK(1); /* Disable interrupts */

    HAL_FLASH_Unlock();

    drv_led_set(0, 1, 0);
    erase_flash();

    do
    {
        res_fs = f_read(fd_p, buffer_p, BUFFER_SIZE, (UINT *)&bytes_read);

        if(res_fs != FR_OK)
        {
            while(1){;}
        }

        res_flash = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,
                                      FLASH_USER_START_ADDR + read_cnt,
                                      (uint64_t)(uint32_t)buffer_p);
        if(res_flash != HAL_OK)
        {
            while(1){;}
        }

        read_cnt += bytes_read;
    } while(bytes_read == BUFFER_SIZE);

    res_fs = f_lseek((FIL *)fd_p, 0);

    if(res_fs != FR_OK)
    {
        while(1){;}
    }

    read_cnt = 0;

    drv_led_set(0, 0, 1);
    do
    {
        uint32_t i;

        res_fs = f_read(fd_p, buffer_p, BUFFER_SIZE, (UINT *)&bytes_read);

        if(res_fs != FR_OK)
        {
            while(1){;}
        }

        for(i = 0; i < bytes_read; i++)
        {
            if(buffer_p[i] != *(__IO uint8_t *)(FLASH_USER_START_ADDR + read_cnt + i))
            {
                while(1){;}
            }
        }

        read_cnt += bytes_read;
    } while(bytes_read == BUFFER_SIZE);

    HAL_FLASH_Lock();
}

static void fw_update()
{
    char fw_file_p[] = {FW_PATH_AND_NAME};
    FRESULT res_fs;
    FATFS fatfs;
    FIL fd;

    res_fs = f_mount(&fatfs, (TCHAR const*)FW_PATH, 1);

    if(res_fs != FR_OK)
    {
        while(1){;}
    }

    /* Check if firmware present on sdcard */
    res_fs = f_open(&fd, fw_file_p, FA_READ);

    if(res_fs == FR_OK)
    {
        flash_file(&fd, fw_file_p);
    }

    f_close(&fd);

    f_unlink(fw_file_p); /* Remove fw file upon success */

    f_mount(NULL, NULL, 0); /* Unmount */
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
    GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;
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
}

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) /* Will be called from HAL_Init */
{
    /* Configure the SysTick to have interrupt in ms time basis */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
    HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority ,0);

    return HAL_OK;
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

void HardFault_Handler(void)
{
    while(1){;}
}

int main()
{
    HAL_Init();
    HAL_Delay(200);
    config_clks();
    drv_sdcard_init();

    /* Card inserted ? */
    if(drv_sdcard_inserted())
    {
        /* Check for fw and flash it if found */
        fw_update();
    }

    jump_to_application();
}
