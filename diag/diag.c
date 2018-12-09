/*
 * memwa2 diagnostic
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
 * This file can run diagnistic for many parts of the hw including ltdc,
 * sdram and sdcard.
 */

#include "diag.h"
#include "ff.h"
#include "tda19988.h"
#include "drv_ltdc.h"
#include "drv_sdram.h"

#define TDA19988        0x0301
#define MKREG(page, addr)   (((page) << 8) | (addr))
#define TDA_VERSION     MKREG(0x00, 0x00)
#define TDA_VERSION_MSB     MKREG(0x00, 0x02)

#define I2C_CEC_ADDRESS    0x68
#define I2C_HDMI_ADDRESS   0xE0

#define TEST_FILE               "testfile"
#define TEST_PATTERN_REPS       1000
#define TEST_TDA19988_WRITE_REG  0x96
#define TEST_SDRAM_START_ADDR   SDRAM_ADDR
#define TEST_SDRAM_SIZE         0x800000
#define TEST_SCREEN_WIDTH       800
#define TEST_SCREEN_HEIGHT      600

static FATFS fatfs;
static FIL fil;
static char sd_path[] = {"0:/"};
static char sd_path_and_file[] = {"0:/"TEST_FILE};
static char test_pattern[] = {"abcdefghijklmnopqrstuvxyz1234567890"};
static char test_pattern_cmp[sizeof(test_pattern)];

typedef enum
{
    DIAG_SDRAM_STATUS_OK,
    DIAG_SDRAM_STATUS_ERROR,
} sdram_status_t;

typedef enum
{
    DIAG_SDCARD_STATUS_OK,
    DIAG_SDCARD_STATUS_ERROR_MOUNT,
    DIAG_SDCARD_STATUS_ERROR_OPEN,
    DIAG_SDCARD_STATUS_ERROR_READ,
    DIAG_SDCARD_STATUS_ERROR_WRITE,
    DIAG_SDCARD_STATUS_ERROR_CORRUPT,
} sdcard_status_t;

typedef enum
{
    DIAG_TDA19988_STATUS_OK,
    DIAG_TDA19988_STATUS_ERROR_READ,
    DIAG_TDA19988_STATUS_ERROR_WRITE,
    DIAG_TDA19988_STATUS_ERROR_INITVAL
} tda19988_status_t;

static sdram_status_t sdram_run()
{
    sdram_status_t sdram_status = DIAG_SDRAM_STATUS_OK;
    uint32_t i;

    /* Write to sdram */
    for(i = 1; i <= TEST_SDRAM_SIZE; i++)
    {
        *(uint8_t *)(TEST_SDRAM_START_ADDR + i -1) = test_pattern[i % 35];
    }

    for(i = 1; i <= TEST_SDRAM_SIZE; i++)
    {
        if(*(uint8_t *)(TEST_SDRAM_START_ADDR + i -1) != test_pattern[i % 35])
        {
            sdram_status = DIAG_SDRAM_STATUS_ERROR;
            goto exit;
        }
    }

    /* Zero the memory */
    for(i = 1; i <= TEST_SDRAM_SIZE; i++)
    {
        *(uint8_t *)(TEST_SDRAM_START_ADDR + i -1) = 0;
    }

exit:
    return sdram_status;
}

static sdcard_status_t sdcard_run()
{
    uint32_t bytes_written;
    uint32_t bytes_read;
    uint32_t i;
    uint32_t j;
    sdcard_status_t sdcard_status = DIAG_SDCARD_STATUS_OK;
    FRESULT fresult = FR_OK;

    if(f_mount(&fatfs, (TCHAR const*)sd_path, 1) != FR_OK)
    {
        sdcard_status = DIAG_SDCARD_STATUS_ERROR_MOUNT;
        goto exit;
    }

    if(f_open(&fil, TEST_FILE, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
    {
        sdcard_status = DIAG_SDCARD_STATUS_ERROR_OPEN;
        goto exit;
    }

    for(i = 0; i < TEST_PATTERN_REPS; i++)
    {
        fresult = f_write(&fil, test_pattern, sizeof(test_pattern), (void *)&bytes_written);
          
        if((bytes_written == 0) || (fresult != FR_OK))
        {
            sdcard_status = DIAG_SDCARD_STATUS_ERROR_WRITE;
            goto close;
        }
    }

    f_close(&fil);

    if(f_open(&fil, TEST_FILE, FA_READ) != FR_OK)
    {
        sdcard_status = DIAG_SDCARD_STATUS_ERROR_OPEN;
        goto remove;
    }

    for(i = 0; i < TEST_PATTERN_REPS; i++)
    {
        fresult = f_read(&fil, test_pattern_cmp, sizeof(test_pattern), (void *)&bytes_read);
          
        if((bytes_read == 0) || (fresult != FR_OK))
        {
            sdcard_status = DIAG_SDCARD_STATUS_ERROR_READ;
            goto close;
        }

        for(j = 0; j < sizeof(test_pattern); j++)
        {
            if(test_pattern_cmp[j] != test_pattern[j])
            {
                sdcard_status = DIAG_SDCARD_STATUS_ERROR_CORRUPT;
                goto close;
            }
        }
    }
close:
    f_close(&fil);
remove:
    f_unlink(sd_path_and_file);
    f_mount(NULL, "", 0);
exit:
    return sdcard_status;
}

static tda19988_status_t tda19988_run()
{
    tda19988_status_t tda19988_status = DIAG_TDA19988_STATUS_OK;
    uint16_t version = 0;
    uint8_t data;

    data = tda19988_rd_reg(I2C_HDMI_ADDRESS,  TDA_VERSION);
    version |= data;
    data = tda19988_rd_reg(I2C_HDMI_ADDRESS,  TDA_VERSION_MSB);
    version |= (data << 8);

    /* Clear feature bits */
    version = version & ~0x30;

    /* Check Chip revision */
    switch (version)
    {
        case TDA19988:
            tda19988_status = DIAG_TDA19988_STATUS_OK;
            break;
        default:
            tda19988_status = DIAG_TDA19988_STATUS_ERROR_READ;
            break;
    }

exit:
    return tda19988_status;
}

static void ltdc_run()
{
    uint32_t *ltdc_p = (uint32_t *)TEST_SDRAM_START_ADDR;
    uint32_t i;
    uint32_t k;
    uint32_t j;
    uint32_t color;

    drv_ltdc_disable_clut(0);
    drv_ltdc_disable_clut(1);

    drv_ltdc_set_memory(0, TEST_SDRAM_START_ADDR);

    drv_ltdc_set_layer(0,
                   0,
                   TEST_SCREEN_WIDTH,
                   0,
                   TEST_SCREEN_HEIGHT,
                   TEST_SCREEN_WIDTH,
                   TEST_SCREEN_HEIGHT,
                   255,
                   LTDC_PIXEL_FORMAT_ARGB8888);

    for(i = 0; i < TEST_SCREEN_HEIGHT; i++)
    {
        color = 0x800000;
        for(k = 0; k < 24; k++)
        {
            for(j = 0; j < TEST_SCREEN_WIDTH/24; j++)
            {
                *(ltdc_p + i * TEST_SCREEN_WIDTH + k * TEST_SCREEN_WIDTH/24 + j) = color | 0xFF000000;
            }
            color >>= 1;
        }
    }

    drv_ltdc_activate_layer(0);
}   

diag_status_t diag_run()
{
    diag_status_t diag_status = DIAG_STATUS_OK;
    sdram_status_t sdram_status = DIAG_SDRAM_STATUS_OK;
    tda19988_status_t tda19988_status = DIAG_TDA19988_STATUS_OK;

    /* SRAM functionality */
    sdram_status = sdram_run();

    if(sdram_status != DIAG_SDRAM_STATUS_OK)
    {
        diag_status = DIAG_STATUS_ERROR_SDRAM;
        goto exit;
    }

    /* ADV7511 (I2C) functionality */
    tda19988_status = tda19988_run();

    if(tda19988_status != DIAG_TDA19988_STATUS_OK)
    {
        diag_status = DIAG_STATUS_ERROR_I2C;
        goto exit;
    }

    /* Display test screen for optical inspection */
    ltdc_run();

exit:
    return diag_status;
}

diag_status_t diag_sdcard_run()
{
    diag_status_t diag_status = DIAG_STATUS_OK;
    sdcard_status_t sdcard_status = DIAG_SDCARD_STATUS_OK;

    /* SD functionality */
    sdcard_status = sdcard_run();

    if(sdcard_status != DIAG_SDCARD_STATUS_OK)
    {
        diag_status = DIAG_STATUS_ERROR_SDCARD;
        goto exit;
    }

exit:
    return diag_status;
}

