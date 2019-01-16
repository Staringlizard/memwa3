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

#include "serv_storage.h"
#include "serv_term.h"
#include "romcc.h"
#include "romdd.h"
#include "drv_sdcard.h"
#include "drv_ltdc.h"
#include "serv_mem.h"
#include "serv_keybd.h"
#include "ff.h"
#include "if.h"
#include <ctype.h>
#include <stdlib.h>

#define BUFFER_SIZE        0x1000
#define MAX_CLUT_SIZE      0x10
#define MAX_FILES          (CC_FILES_SIZE/sizeof(serv_storage_file_t))
#define MAX_FILES_FILTERED (CC_FILES_FILTERED_SIZE/sizeof(serv_storage_file_t))

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

static char *g_prog_path_pp[1] =
{
  "0:/prog"
};

static FATFS g_fatfs;
static char g_sd_path_p[] = {"0:/"};
static uint32_t g_clut_p[MAX_CLUT_SIZE];
static if_keybd_map_t g_keybd_map_p[SERV_KEYBD_MAP_SIZE + 1];
static serv_storage_file_t *g_file_list_p = (serv_storage_file_t *)CC_FILES_ADDR;
static serv_storage_file_t *g_file_list_filterd_p = (serv_storage_file_t *)CC_FILES_FILTERED_ADDR;
static uint32_t g_file_list_cnt;
static uint32_t g_sel_file;
static uint32_t g_sel_page;
static FIL *g_fd_p;
static scanned_files_t g_scanned_files_fp;

static char *get_filename_ext(char *filename)
{
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

static int strcicmp(char const *a, char const *b, int len)
{
    len--;
    for(;; a++, b++, len--) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if(d != 0 || !*a || !len)
            return d;
    }
}

static uint8_t process_file(FIL *fd_p, serv_storage_file_type_t type)
{
    FRESULT res = FR_OK;
    uint8_t return_val = 1;
    uint32_t bytes_read;
    uint16_t start_address;

    /* only one file can be opened at a time */
    if(g_fd_p != NULL)
    {
        f_close(g_fd_p);
        free(g_fd_p);
        g_fd_p = NULL;
    }
    g_fd_p = fd_p;

    switch(type)
    {
        case SERV_STORAGE_FILE_TYPE_T64:
            {
                uint8_t header_p[0x40];
                uint8_t entry_p[0x20];
                uint16_t end_address;
                uint16_t entry_size;
                uint32_t entry_offset;

                uint8_t i;

                /* Read header which is not used atm */
                res = f_read(fd_p, header_p, 0x40, (void *)&bytes_read);

                if(res != FR_OK || bytes_read != 0x40)
                {
                    return_val = 0;
                    goto exit;
                }

                /* Get number of files in container */
                //files = header_p[0x24];

                /* Just step through the first file */
                for(i = 0; i < 1; i++)
                {
                    /* Go to the start position for the entries */
                    res = f_lseek(fd_p, 0x40 + 0x20 * i);
                    if(res != FR_OK)
                    {
                        return_val = 0;
                        goto exit;
                    }

                    /* Read entry information */
                    res = f_read(fd_p, entry_p, 0x20, (void *)&bytes_read);

                    if(res != FR_OK || bytes_read != 0x20)
                    {
                        return_val = 0;
                        goto exit;
                    }

                    /*
                     * From T64 specification:
                     * In reality any value that is not a $00 is seen as a
                     * PRG file.
                     */
                    if(entry_p[1] == 0x00)
                    {
                        /* This file is not PRG */
                        return_val = 0;
                        goto exit;
                    }

                    /* Get start address */
                    start_address = entry_p[0x3] << 8;
                    start_address += entry_p[0x2];

                    /*
                     * From T64 specification:
                     * If the file is a snapshot, the address will be 0.
                     */
                    if(start_address == 0x0)
                    {
                        serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "T64 snapshots are not supported!");
                    }

                    /* Get end address */
                    end_address = entry_p[0x5] << 8;
                    end_address += entry_p[0x4];

                    /*
                     * From T64 specification:
                     * As mentioned previously, there are faulty T64 files around
                     * with the 'end load address' value set to $C3C6.
                     */
                    if(end_address == 0xC3C6)
                    {
                        serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "T64 file is corrupt!");
                    }

                    /* Calculate size of entry */
                    entry_size = end_address - start_address;

                    entry_offset = entry_p[0xb] << 24;
                    entry_offset += entry_p[0xa] << 16;
                    entry_offset += entry_p[0x9] << 8;
                    entry_offset += entry_p[0x8];

                    /* Go to the start position for the entry in file */
                    res = f_lseek(fd_p, entry_offset);
                    if(res != FR_OK)
                    {
                        return_val = 0;
                        goto exit;
                    }

                    /* Start writing into the ram memory, which we gave to emulator at startup */
                    res = f_read(fd_p, (uint8_t *)(CC_RAM_BASE_ADDR + start_address), entry_size, (void *)&bytes_read);

                    if(res != FR_OK || bytes_read != entry_size)
                    {
                        return_val = 0;
                        goto exit;
                    }
                }
            }
            break;
        case SERV_STORAGE_FILE_TYPE_PRG:
            {
                uint8_t entry_p[0x4];

                /* Read entry information */
                res = f_read(fd_p, entry_p, 2, (void *)&bytes_read);

                if(res != FR_OK || bytes_read != 2)
                {
                    return_val = 0;
                    goto exit;
                }

                start_address = entry_p[0] + (entry_p[1] << 8);

                /* These kind of file is just loaded right into emu memory */
                res = f_read(fd_p, (uint8_t *)(CC_RAM_BASE_ADDR + start_address), 0x10000, (void *)&bytes_read);

                if(res != FR_OK || bytes_read == 0)
                {
                    serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "Failed to read PRG file!");
                    return_val = 0;
                    goto exit;
                }
            }
            break;
        case SERV_STORAGE_FILE_TYPE_TAP:
            g_if_cc_emu.if_emu_cc_tape_drive.tape_drive_load_fp((uint32_t *)fd_p);
            break;
        case SERV_STORAGE_FILE_TYPE_D64:
            g_if_dd_emu.if_emu_dd_disk_drive.disk_drive_load_fp((uint32_t *)fd_p);
            break;
    }

exit:
    return return_val;
}

void serv_storage_init()
{
    drv_sdcard_init();
}

void serv_storage_mount()
{
    if(f_mount(&g_fatfs, (TCHAR const*)g_sd_path_p, 1) != FR_OK)
    {
        serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "Failed to mount filesystem!");
    }
}

void serv_storage_read_config()
{
    uint32_t bytes_read = 0;
    uint32_t read_cnt = 0;
    FIL fd;
    FRESULT res;

   /* Try and load basic rom from sd card */
    res = f_open(&fd, g_rom_path_pp[0], FA_READ);

    if(res == FR_OK)
    {
        read_cnt = 0;
        while(read_cnt != IF_MEMORY_CC_BROM_ACTUAL_SIZE)
        {
            res = f_read(&fd, (uint8_t *)(CC_BROM_BASE_ADDR + CC_BROM_LOAD_ADDR + read_cnt), BUFFER_SIZE, (UINT *)&bytes_read);
            if(res != FR_OK || bytes_read == 0)
            {
                serv_term_printf(SERV_TERM_PRINT_TYPE_WARNING, "Error reading rom file (brom)!");
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
            res = f_read(&fd, (uint8_t *)(CC_CROM_BASE_ADDR + CC_CROM_LOAD_ADDR + read_cnt), BUFFER_SIZE, (UINT *)&bytes_read);
            if(res != FR_OK || bytes_read == 0)
            {
                serv_term_printf(SERV_TERM_PRINT_TYPE_WARNING, "Error reading rom file (crom)!");
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
                serv_term_printf(SERV_TERM_PRINT_TYPE_WARNING, "Error reading rom file (krom)!");
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
                serv_term_printf(SERV_TERM_PRINT_TYPE_WARNING, "Error reading rom file (dos)!");
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

        memset(colors_pp, 0x00, 28 * sizeof(char *));
        memset(palette_string_p, 0x00, 256);

        res = f_read(&fd, (uint8_t *)palette_string_p, 256, (UINT *)&bytes_read);

        if(res != FR_OK || bytes_read == 0)
        {
            serv_term_printf(SERV_TERM_PRINT_TYPE_WARNING, "Error reading file %s!", g_conf_path_pp[0]);
        }

        colors_pp[colors_cnt] = strtok((char *)palette_string_p, delimiter_p);
        colors_cnt++;
        while(colors_pp[colors_cnt-1] != NULL)
        {
            /* Get the color asciihex string */
            if(colors_cnt < MAX_CLUT_SIZE)
            {
                colors_pp[colors_cnt] = strtok(NULL, delimiter_p);
                /* Get the 32 bit value */
                color = strtoul(colors_pp[colors_cnt], NULL, 16);

                /* Add to collection */
                g_clut_p[colors_cnt] = color;
            }
            else
            {
                break;
            }

            colors_cnt++;
        }

        f_close(&fd);

        drv_ltdc_set_clut_table(g_clut_p);
    }
    else
    {
        ; /* No file found, the default clut table will be used */
    }

     /* Try and load key conf from sd card */
    res = f_open(&fd, g_conf_path_pp[1], FA_READ);

    if(res == FR_OK)
    {
        uint8_t key_string_p[1024]; /* 16 chars max on one row */

        res = f_read(&fd, (uint8_t *)key_string_p, 1024, (UINT *)&bytes_read);

        if(res != FR_OK || bytes_read == 0)
        {
            serv_term_printf(SERV_TERM_PRINT_TYPE_WARNING, "Error reading file %s!", g_conf_path_pp[1]);
        }

        serv_keybd_populate_map(key_string_p, g_keybd_map_p);

        g_if_cc_emu.if_emu_cc_ue.ue_keybd_map_set_fp(g_keybd_map_p);

        f_close(&fd);
    }
    else
    {
        ; /* No file found, the default keybd table will be used */
    }
}

void serv_storage_reg_scan_files_cb(scanned_files_t cb)
{
    g_scanned_files_fp = cb;
}

void serv_storage_unscan_files()
{
    uint32_t i;

    for(i = 0; i < g_file_list_cnt; i++)
    {
        memset(g_file_list_p + i, 0x00, sizeof(serv_storage_file_t));
    }

    g_file_list_cnt = 0;
    g_sel_file = 0;
    g_sel_page = 0;
}

uint8_t serv_storage_scan_files(serv_storage_file_t **entries_pp, uint32_t *files_p)
{
    FRESULT res;
    uint8_t return_val = 1;
    DIR dir;
    FILINFO fno;

#ifdef USE_LFN
    char lfn[sizeof(((serv_storage_file_t *)NULL)->fname_p)] = {0};

    fno.lfname = lfn;
    fno.lfsize = sizeof(((serv_storage_file_t *)NULL)->fname_p);
#endif

    if(g_file_list_cnt != 0)
    {
        /* No need to reload it */
        return FR_OK;
    }
    else
    {
        serv_storage_unscan_files();
    }

    res = f_opendir(&dir, g_prog_path_pp[0]);

    if(res == FR_DISK_ERR && drv_sdcard_inserted())
    {
        /* Perhaps the sd card was removed and inserted? Try one more time */
        drv_sdcard_init();
        res = f_opendir(&dir, g_prog_path_pp[0]);
    }

    if(res == FR_OK)
    {
        (void)f_readdir(&dir, &fno); /* remove "." */
        (void)f_readdir(&dir, &fno); /* remove ".." */
        for(;;)
        {
            memset(lfn, 0x00, sizeof(lfn)); /* fno.lfname is same as lfn */

            res = f_readdir(&dir, &fno);
            if(res != FR_OK || fno.fname[0] == 0)
            {
                break;
            }
            else
            {
                char *ext_p;
#ifdef USE_LFN
                if(strlen(fno.lfname) > 0)
                {
                    memcpy(g_file_list_p[g_file_list_cnt].fname_p, fno.lfname, strlen(fno.lfname) + 1);
                }
                else
                {
                    memcpy(g_file_list_p[g_file_list_cnt].fname_p, fno.fname, strlen(fno.fname) + 1);
                }
#else
                memcpy(g_file_list_p[g_file_list_cnt].fname_p, fno.fname, strlen(fno.fname) + 1);
#endif
                g_file_list_p[g_file_list_cnt].size = fno.fsize;
                ext_p = get_filename_ext(g_file_list_p[g_file_list_cnt].fname_p);
                if(strcicmp(ext_p, "t64", 3) == 0)
                {
                    g_file_list_p[g_file_list_cnt].type = SERV_STORAGE_FILE_TYPE_T64;
                }
                else if(strcicmp(ext_p, "prg", 3) == 0)
                {
                    g_file_list_p[g_file_list_cnt].type = SERV_STORAGE_FILE_TYPE_PRG;
                }
                else if(strcicmp(ext_p, "tap", 3) == 0)
                {
                    g_file_list_p[g_file_list_cnt].type = SERV_STORAGE_FILE_TYPE_TAP;
                }
                else if(strcicmp(ext_p, "d64", 3) == 0)
                {
                    g_file_list_p[g_file_list_cnt].type = SERV_STORAGE_FILE_TYPE_D64;
                }

                g_file_list_cnt++;

                if(g_file_list_cnt >= MAX_FILES)
                {
                    break;
                }

                if(g_scanned_files_fp != NULL && (g_file_list_cnt % 20) == 0)
                {
                    g_scanned_files_fp(g_file_list_cnt);
                }
            }
        }
        f_closedir(&dir);
    }

    if(res == FR_OK)
    {
        return_val = 0;
        *files_p = g_file_list_cnt;
        *entries_pp = g_file_list_p;

        serv_term_printf(SERV_TERM_PRINT_TYPE_INFO, "Found %ld files when scanning %s", g_file_list_cnt, g_prog_path_pp[0]);
    }
    else
    {
        return_val = 1;
    }

    return return_val;
}

void serv_storage_files_filter(serv_storage_file_t *entries_p, uint32_t *files_p, serv_storage_file_t **result_pp, char *filter_p)
{
    uint32_t i;
    uint32_t cnt = 0;

    for(i = 0; i < *files_p; i++)
    {
        if(strlen(&entries_p[i].fname_p[0]) >= strlen(filter_p) && (strcicmp(filter_p, &entries_p[i].fname_p[0], strlen(filter_p)) == 0))
        {
            g_file_list_filterd_p[cnt] = entries_p[i];
            cnt++;

            if(cnt >= MAX_FILES_FILTERED)
            {
                break;
            }
        }
    }

    *files_p = cnt;
    *result_pp = g_file_list_filterd_p;
}

uint8_t serv_storage_load_file(serv_storage_file_t *file_p)
{
    FRESULT res = FR_OK;
    FIL *fd_p;
    char path_p[256] = {0};
    sprintf(path_p, "%s/%s", g_prog_path_pp[0], file_p->fname_p);

    /* Create new fd and try and open file */
    fd_p = (FIL *)calloc(1, sizeof(FIL));
    res = f_open(fd_p, path_p, FA_READ);

    if(res == FR_DISK_ERR && drv_sdcard_inserted())
    {
        /* Perhaps the sd card was removed and inserted? Try one more time */
        drv_sdcard_init();
        res = f_open(fd_p, path_p, FA_READ);
    }

    if(res != FR_OK)
    {
        free(fd_p);
        fd_p = NULL;
        serv_term_printf(SERV_TERM_PRINT_TYPE_WARNING, "Unable to open %s (errno %d)!", path_p, res);
        return 1;
    }

    serv_term_printf(SERV_TERM_PRINT_TYPE_INFO, "Sucessfully opened %s!", path_p);
    process_file(fd_p, file_p->type);

    return 0;
}
