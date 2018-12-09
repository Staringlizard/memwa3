/*
 * Storage
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

#include "dev_storage.h"
#include "if.h"
#include "ff.h"
#include "romcc.h"
#include "romdd.h"
#include "dev_mem.h"

#define BUFFER_SIZE        0x1000

static char *g_rom_path_pp[4] =
{
  "0:/rom/cc_brom.bin",
  "0:/rom/cc_crom.bin",
  "0:/rom/cc_krom.bin",
  "0:/rom/dd_dos.bin"
};

static char *g_conf_path_pp[2] =
{
  "0:/conf/palette.cfg",
  "0:/conf/key.cfg"
};

static FATFS g_fatfs;
static char g_sd_path_p[] = {"0:/"};
static uint32_t g_clut_a[16];
//static if_keybd_map_t g_keybd_map_a[DEFAULT_KEY_MAX + 1];

void storage_init()
{
    uint32_t bytes_read = 0;
    uint32_t read_cnt = 0;
    FIL fd;
    FRESULT res;

    drv_sdcard_init();

    if(f_mount(&g_fatfs, (TCHAR const*)g_sd_path_p, 1) != FR_OK)
    {
        main_error("Failed to mount filesystem!", __FILE__, __LINE__, 0);
    }

   /* Try and load basic rom from sd card */
    res = f_open(&fd, g_rom_path_pp[0], FA_READ);

    if(res == FR_OK)
    {
        read_cnt = 0;
        while(read_cnt != IF_MEMORY_CC_BROM_ACTUAL_SIZE)
        {
            f_read(&fd, (uint8_t *)(CC_BROM_BASE_ADDR + CC_BROM_LOAD_ADDR + read_cnt), BUFFER_SIZE, (UINT *)&bytes_read);
            if(res != FR_OK || bytes_read == 0)
            {
                main_warning("Error reading rom file (brom)!", __FILE__, __LINE__, read_cnt);
                break;
            }
            read_cnt += bytes_read;
        }
        f_close(&fd);
    }
    else
    {
        /* No file found, load the default one */
        memcpy((uint8_t *)(CC_BROM_BASE_ADDR + CC_BROM_LOAD_ADDR), rom_cc_get_memory(ROM_CC_SECTION_BROM), IF_MEMORY_CC_BROM_ACTUAL_SIZE);
    }
 
     /* Try and load character rom from sd card */
    res = f_open(&fd, g_rom_path_pp[1], FA_READ);

    if(res == FR_OK)
    {
        read_cnt = 0;
        while(read_cnt != IF_MEMORY_CC_CROM_ACTUAL_SIZE)
        {
            f_read(&fd, (uint8_t *)(CC_CROM_BASE_ADDR + CC_CROM_LOAD_ADDR + read_cnt), BUFFER_SIZE, (UINT *)&bytes_read);
            if(res != FR_OK || bytes_read == 0)
            {
                main_warning("Error reading rom file (crom)!", __FILE__, __LINE__, read_cnt);
                break;
            }
            read_cnt += bytes_read;
        }
        f_close(&fd);
    }
    else
    {
        /* No file found, load the default one */
        memcpy((uint8_t *)(CC_CROM_BASE_ADDR + CC_CROM_LOAD_ADDR), rom_cc_get_memory(ROM_CC_SECTION_CROM), IF_MEMORY_CC_CROM_ACTUAL_SIZE);
    }

     /* Try and load kernal rom from sd card */
    res = f_open(&fd, g_rom_path_pp[2], FA_READ);

    if(res == FR_OK)
    {
        read_cnt = 0;
        while(read_cnt != IF_MEMORY_CC_KROM_ACTUAL_SIZE)
        {
            f_read(&fd, (uint8_t *)(CC_KROM_BASE_ADDR + CC_KROM_LOAD_ADDR + read_cnt), BUFFER_SIZE, (UINT *)&bytes_read);
            if(res != FR_OK || bytes_read == 0)
            {
                main_warning("Error reading rom file (krom)!", __FILE__, __LINE__, read_cnt);
                break;
            }
            read_cnt += bytes_read;
        }
        f_close(&fd);
    }
    else
    {
        /* No file found, load the default one */
        memcpy((uint8_t *)(CC_KROM_BASE_ADDR + CC_KROM_LOAD_ADDR), rom_cc_get_memory(ROM_CC_SECTION_KROM), IF_MEMORY_CC_KROM_ACTUAL_SIZE);
    }

     /* Try and load dos rom from sd card */
    res = f_open(&fd, g_rom_path_pp[3], FA_READ);

    if(res == FR_OK)
    {
        read_cnt = 0;
        while(read_cnt != IF_MEMORY_DD_DOS_ACTUAL_SIZE)
        {
            res = f_read(&fd, (uint8_t *)(DD_ALL_BASE_ADDR + DD_DOS_LOAD_ADDR + read_cnt), BUFFER_SIZE, (UINT *)&bytes_read);
            if(res != FR_OK || bytes_read == 0)
            {
                main_warning("Error reading rom file (dos)!", __FILE__, __LINE__, read_cnt);
                break;
            }
            read_cnt += bytes_read;
        }
        f_close(&fd);
    }
    else
    {
        /* No file found, load the default one */
        memcpy((uint8_t *)(DD_ALL_BASE_ADDR + DD_DOS_LOAD_ADDR), rom_dd_get_memory(ROM_DD_SECTION_DOS), IF_MEMORY_DD_DOS_ACTUAL_SIZE);
    }

     /* Try and load palette conf from sd card */

    res = f_open(&fd, g_conf_path_pp[0], FA_READ);

    if(res == FR_OK)
    {
        uint8_t palette_string_p[256];
        char *colors_pp[28];
        uint8_t colors_cnt = 0;
        char delimiter_p[2] = "\n";
        uint32_t color;

        memset(colors_pp, 0x00, 16 * sizeof(char *));
        memset(palette_string_p, 0x00, 256);

        f_read(&fd, (uint8_t *)palette_string_p, 256, (UINT *)&bytes_read);

        if(res != FR_OK || bytes_read == 0)
        {
            main_warning("Error reading file (palette)!", __FILE__, __LINE__, 0);
        }

        colors_pp[colors_cnt] = strtok((char *)palette_string_p, delimiter_p);
        colors_cnt++;
        while(colors_pp[colors_cnt-1] != NULL)
        {
            /* Get the color asciihex string */
            colors_pp[colors_cnt] = strtok(NULL, delimiter_p);

            /* Get the 32 bit value */
            color = strtoul(colors_pp[colors_cnt], NULL, 16);

            /* Add to collection */
            g_clut_a[colors_cnt] = color;

            colors_cnt++;
        }

        f_close(&fd);

        //drv_ltdc_set_clut_table(g_clut_a);
    }
    else
    {
        ; /* No file found, the default clut table will be used in disp.c */
    }

     /* Try and load key conf from sd card */
    res = f_open(&fd, g_conf_path_pp[1], FA_READ);

    if(res == FR_OK)
    {
        uint8_t key_string_p[1024]; /* 16 chars max on one row */

        f_read(&fd, (uint8_t *)key_string_p, 1024, (UINT *)&bytes_read);

        if(res != FR_OK || bytes_read == 0)
        {
            main_warning("Error reading file (palette)!", __FILE__, __LINE__, 0);
        }

        //dev_keybd_populate_map(key_string_p, g_keybd_map_a);

        f_close(&fd);
    }

}

