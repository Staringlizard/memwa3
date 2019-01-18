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
#include "dev_tda19988.h"
#include "drv_ltdc.h"
#include "drv_sdram.h"


#define TDA19988        0x0301
#define TDA_VERSION     0x0000
#define TDA_VERSION_MSB 0x0002

#define I2C_CEC_ADDRESS    0x68
#define I2C_HDMI_ADDRESS   0xE0

#define TEST_FILE               "testfile"
#define TEST_PATTERN_REPS       1000
#define TEST_TDA19988_WRITE_REG 0x96
#define TEST_SDRAM_START_ADDR   SDRAM_ADDR
#define TEST_SDRAM_SIZE         0x800000
#define TEST_SCREEN_WIDTH       800
#define TEST_SCREEN_HEIGHT      600

extern void read_reg_8(uint8_t i2c_addr, uint16_t mem_addr, uint8_t *val_p);

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

    read_reg_8(TDA19988_ADDR_HDMI, TDA_VERSION, &data);
    version |= data;
    read_reg_8(TDA19988_ADDR_HDMI, TDA_VERSION_MSB, &data);
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

    return tda19988_status;
}

diag_status_t diag_video()
{
    diag_status_t diag_status = DIAG_STATUS_OK;
    tda19988_status_t tda19988_status = DIAG_TDA19988_STATUS_OK;

    /* tda19988 (I2C) functionality */
    tda19988_status = tda19988_run();

    if(tda19988_status != DIAG_TDA19988_STATUS_OK)
    {
        diag_status = DIAG_STATUS_ERROR_I2C;
        serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "diag video: NOK");
        goto exit;
    }
    else
    {
        serv_term_printf(SERV_TERM_PRINT_TYPE_INFO, "diag video: OK");
    }

exit:
    return diag_status;
}

diag_status_t diag_sdram()
{
    diag_status_t diag_status = DIAG_STATUS_OK;
    sdram_status_t sdram_status = DIAG_SDRAM_STATUS_OK;

    /* SRAM functionality */
    sdram_status = sdram_run();

    if(sdram_status != DIAG_SDRAM_STATUS_OK)
    {
        diag_status = DIAG_STATUS_ERROR_SDRAM;
        serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "diag sdram: NOK");
        goto exit;
    }
    else
    {
        serv_term_printf(SERV_TERM_PRINT_TYPE_INFO, "diag sdram: OK (press ctrl + F12 to restart)");
    }

exit:
    return diag_status;
}

diag_status_t diag_sdcard()
{
    diag_status_t diag_status = DIAG_STATUS_OK;
    sdcard_status_t sdcard_status = DIAG_SDCARD_STATUS_OK;

    /* SD functionality */
    sdcard_status = sdcard_run();

    if(sdcard_status != DIAG_SDCARD_STATUS_OK)
    {
        diag_status = DIAG_STATUS_ERROR_SDCARD;
        serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "diag sdcard: NOK");
        goto exit;
    }
    else
    {
        serv_term_printf(SERV_TERM_PRINT_TYPE_INFO, "diag sdcard: OK");
    }

exit:
    return diag_status;
}

