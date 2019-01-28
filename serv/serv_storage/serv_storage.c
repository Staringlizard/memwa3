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

#define CC_RAM_OFFSET_MAX_SONG_NO  0x02
#define SID_BASIC_START_ADDR    0x0801
#define SID_DRIVER_START_ADDR   0x0820
#define BUFFER_SIZE             0x1000
#define MAX_CLUT_SIZE           0x10
#define MAX_FILES               (CC_FILES_SIZE/sizeof(serv_storage_file_t))
#define MAX_FILES_FILTERED      (CC_FILES_FILTERED_SIZE/sizeof(serv_storage_file_t))

typedef struct
{
    uint16_t start_addr_c64;
} __attribute__((packed)) prg_t;

typedef struct
{
    uint16_t ver;
    uint16_t entries;
    uint16_t entries_used;
    uint16_t reserved;
    char name[0x18];
} __attribute__((packed)) t64_dir_t;

typedef struct
{
    uint8_t file_type_c64;
    uint8_t file_type_1541;
    uint16_t start_addr_c64;
    uint16_t end_addr_c64;
    uint16_t reserved1;
    uint32_t start_addr_file;
    uint32_t reserved2;
    char c64_file_name[0x10];
} __attribute__((packed)) t64_entry_t;

typedef struct
{
    char* sign[0x20];
    t64_dir_t dir;
} __attribute__((packed)) t64_header_t;

typedef struct
{
    char magic_id[4];
    uint16_t ver;
    uint16_t start_addr_file;
    uint16_t start_addr_c64;
    uint16_t init_addr;
    uint16_t play_addr;
    uint16_t songs;
    uint16_t start_song;
    uint32_t speed;
    char name[32];
    char author[32];
    char released[32];
    uint16_t flags;
    uint8_t start_page;
    uint8_t page_length;
    uint8_t second_sid_addr;
    uint8_t third_sid_addr;
} __attribute__((packed)) sid_t;

static char *g_rom_path_pp[] =
{
  "0:/rom/cc_brom.bin",
  "0:/rom/cc_crom.bin",
  "0:/rom/cc_krom.bin",
  "0:/rom/dd_dos.bin"
};

static char *g_conf_path_pp[] =
{
  "0:/conf/palette.cfg",
  "0:/conf/key.cfg"
};

static char *g_load_path_pp[] =
{
  "0:/prog",
  "0:/sid"
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
static uint8_t g_index;

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

static uint8_t process_file(FIL *fd_p, serv_storage_file_type_t type, uint32_t size)
{
    FRESULT res = FR_OK;
    uint8_t return_val = 0;
    uint32_t bytes_read;

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
            t64_header_t header = {0};
            t64_entry_t entry = {0};

            uint8_t i;

            /* Read header which is not used atm */
            res = f_read(fd_p, &header, 0x40, (void *)&bytes_read);

            if(res != FR_OK || bytes_read != 0x40)
            {
                return_val = 1;
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
                    return_val = 1;
                    goto exit;
                }

                /* Read entry information */
                res = f_read(fd_p, &entry, 0x20, (void *)&bytes_read);

                if(res != FR_OK || bytes_read != 0x20)
                {
                    return_val = 1;
                    goto exit;
                }

                /*
                 * From T64 specification:
                 * In reality any value that is not a $00 is seen as a
                 * PRG file.
                 */
                if(entry.file_type_1541 == 0x00)
                {
                    /* This file is not PRG */
                    return_val = 1;
                    goto exit;
                }

                /*
                 * From T64 specification:
                 * If the file is a snapshot, the address will be 0.
                 */
                if(entry.start_addr_c64 == 0x0)
                {
                    serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "T64 snapshots are not supported!");
                }

                /*
                 * From T64 specification:
                 * As mentioned previously, there are faulty T64 files around
                 * with the 'end load address' value set to $C3C6.
                 */
                if(entry.end_addr_c64 == 0xC3C6)
                {
                    serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "T64 file is corrupt!");
                }

                /* Go to the start position for the entry in file */
                res = f_lseek(fd_p, entry.start_addr_file);
                if(res != FR_OK)
                {
                    return_val = 1;
                    goto exit;
                }

                /* Start writing into the ram memory, which we gave to emulator at startup */
                res = f_read(fd_p, (uint8_t *)(CC_RAM_BASE_ADDR + entry.start_addr_c64), entry.end_addr_c64 - entry.start_addr_c64, (void *)&bytes_read);

                if(res != FR_OK || bytes_read != entry.end_addr_c64 - entry.start_addr_c64)
                {
                    return_val = 1;
                    goto exit;
                }
            }
        }
        break;
        case SERV_STORAGE_FILE_TYPE_PRG:
        {
            prg_t prg;

            /* Read entry information */
            res = f_read(fd_p, &prg, 2, (void *)&bytes_read);

            if(res != FR_OK || bytes_read != 2)
            {
                return_val = 1;
                goto exit;
            }

            /* These kind of file is just loaded right into emu memory */
            res = f_read(fd_p, (uint8_t *)(CC_RAM_BASE_ADDR + prg.start_addr_c64), 0x10000, (void *)&bytes_read);

            if(res != FR_OK || bytes_read == 0)
            {
                serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "Failed to read PRG file!");
                return_val = 1;
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
        case SERV_STORAGE_FILE_TYPE_SID:
        {
            sid_t *sid_p;
            prg_t sid_prg;
            uint16_t endian;
            uint8_t *sid_payload_p;
            uint8_t *file_data_p = calloc(1, size);

            res = f_read(fd_p, file_data_p, size, (void *)&bytes_read);

            if(res != FR_OK || bytes_read == 0)
            {
                serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "Failed to read SID file!");
                return_val = 1;
                goto exit;
            }

            sid_p = (sid_t *)file_data_p;

            endian = sid_p->ver;
            sid_p->ver = (endian >> 8) | (endian << 8);

            endian = sid_p->start_addr_file;
            sid_p->start_addr_file = (endian >> 8) | (endian << 8);

            endian = sid_p->start_addr_c64;
            sid_p->start_addr_c64 = (endian >> 8) | (endian << 8);

            endian = sid_p->init_addr;
            sid_p->init_addr = (endian >> 8) | (endian << 8);

            endian = sid_p->play_addr;
            sid_p->play_addr = (endian >> 8) | (endian << 8);

            endian = sid_p->songs;
            sid_p->songs = (endian >> 8) | (endian << 8);

            endian = sid_p->start_song;
            sid_p->start_song = (endian >> 8) | (endian << 8);

            endian = sid_p->speed;
            sid_p->speed = (endian >> 8) | (endian << 8);

            endian = sid_p->flags;
            sid_p->flags = (endian >> 8) | (endian << 8);

            sid_payload_p = &file_data_p[sid_p->start_addr_file];

            if((sid_p->flags & 0x0C) != 0x04)
            {
                serv_term_printf(SERV_TERM_PRINT_TYPE_WARNING, "Sid is not PAL, it will not be played at correct speed!");
            }

            if((sid_p->init_addr >= 0xA000 &&
                sid_p->init_addr <= 0xBFFF) ||
                (sid_p->init_addr >= 0xD000 &&
                sid_p->init_addr <= 0xFFFF))
            {
                serv_term_printf(SERV_TERM_PRINT_TYPE_ERROR, "Invalid sid file! (init function located at 0x%x!", sid_p->init_addr);
                return_val = 1;
                goto exit;
            }

            /* Go to the start position for the entry in file */
            res = f_lseek(fd_p, sid_p->start_addr_file);

            if(res != FR_OK)
            {
                return_val = 1;
                goto exit;
            }

            if(sid_p->start_addr_c64 == 0)
            {
                prg_t sid_driver_prg;
                prg_t basic_run_prg;

                uint8_t sid_driver_p[] =
                {
                    (SID_DRIVER_START_ADDR & 0xFF), (SID_DRIVER_START_ADDR >> 8),
                                0xa9, 0x00, 0x85, 0x2a, 0xaa, 0xa8, 0xa5, 0x2a, 0x20, 0xad, 0xde, 0x78, 0xa9, 0x7f,
                    0x8d, 0x0d, 0xdc, 0x8d, 0x0d, 0xdd, 0xa9, 0x01, 0x8d, 0x1a, 0xd0, 0xa9, 0x1b, 0xa2, 0x08, 0xa0,
                    0x14, 0x8d, 0x11, 0xd0, 0x8e, 0x16, 0xd0, 0x8c, 0x18, 0xd0, 0xa9, 0x7b, 0xa2, 0x08, 0xa0, 0x7e,
                    0x8d, 0x14, 0x03, 0x8e, 0x15, 0x03, 0x8c, 0x12, 0xd0, 0xad, 0x0d, 0xdc, 0xad, 0x0d, 0xdd, 0x0e,
                    0x19, 0xd0, 0x58, 0xa9, 0x27, 0xc5, 0xcb, 0xd0, 0x11, 0xa5, 0x02, 0xc5, 0x2a, 0xf0, 0x17, 0xe6,
                    0x2a, 0xa9, 0x40, 0xc5, 0xcb, 0xd0, 0xfa, 0x4c, 0x26, 0x08, 0x4c, 0x61, 0x08, 0x20, 0xff, 0xff,
                    0x0e, 0x19, 0xd0, 0x4c, 0x31, 0xea, 0xa9, 0x00, 0x85, 0x2a, 0x4c, 0x6f, 0x08
                };

                uint8_t basic_run_p[] =
                {
                    (SID_BASIC_START_ADDR & 0xFF), (SID_BASIC_START_ADDR >> 8),
                                0x0f, 0x08, 0x00, 0x00, 0x99, 0x22, 0x4e, 0x3d, 0x4e, 0x45, 0x58, 0x54, 0x22, 0x00,
                    0x19, 0x08, 0x0a, 0x00, 0x9e, 0x32, 0x30, 0x38, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                };

                sid_driver_p[0x0B] = sid_p->init_addr & 0xFF;
                sid_driver_p[0x0C] = sid_p->init_addr >> 8;

                sid_driver_p[0x5E] = sid_p->play_addr & 0xFF;
                sid_driver_p[0x5F] = sid_p->play_addr >> 8;

                /* Handle sid payload as a normal prg */
                sid_prg.start_addr_c64 = (sid_payload_p[1] << 8) | sid_payload_p[0];
                basic_run_prg.start_addr_c64 = (basic_run_p[1] << 8) | basic_run_p[0];
                sid_driver_prg.start_addr_c64 = (sid_driver_p[1] << 8) | sid_driver_p[0];
                memcpy((uint8_t *)(CC_RAM_BASE_ADDR + sid_prg.start_addr_c64), &sid_payload_p[2], size - sizeof(sid_t));
                memcpy((uint8_t *)(CC_RAM_BASE_ADDR + basic_run_prg.start_addr_c64), &basic_run_p[2], sizeof(basic_run_p) - 2);
                memcpy((uint8_t *)(CC_RAM_BASE_ADDR + sid_driver_prg.start_addr_c64), &sid_driver_p[2], sizeof(sid_driver_p) - 2);
                memcpy((uint8_t *)(CC_RAM_BASE_ADDR + CC_RAM_OFFSET_MAX_SONG_NO), (uint8_t *)&sid_p->songs - 1, 1);
            }

            free(file_data_p);
        }
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

uint8_t serv_storage_get_max_scan_dirs()
{
    return sizeof(g_load_path_pp) / sizeof(char *);
}

uint8_t serv_storage_scan_files(serv_storage_file_t **entries_pp, uint32_t *files_p, uint8_t index)
{
    FRESULT res;
    uint8_t return_val = 1;
    DIR dir;
    FILINFO fno;

    g_index = index;

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

    res = f_opendir(&dir, g_load_path_pp[index]);

    if(res == FR_DISK_ERR && drv_sdcard_inserted())
    {
        /* Perhaps the sd card was removed and inserted? Try one more time */
        drv_sdcard_init();
        res = f_opendir(&dir, g_load_path_pp[index]);
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
                else if(strcicmp(ext_p, "sid", 3) == 0)
                {
                    g_file_list_p[g_file_list_cnt].type = SERV_STORAGE_FILE_TYPE_SID;
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

        serv_term_printf(SERV_TERM_PRINT_TYPE_INFO, "Found %ld files when scanning %s", g_file_list_cnt, g_load_path_pp[g_index]);
    }
    else
    {
        serv_term_printf(SERV_TERM_PRINT_TYPE_INFO, "Error when opening %s", g_load_path_pp[g_index]);
        g_file_list_cnt = 0;
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
    sprintf(path_p, "%s/%s", g_load_path_pp[g_index], file_p->fname_p);

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

    if(process_file(fd_p, file_p->type, file_p->size) != 0)
    {
        serv_term_printf(SERV_TERM_PRINT_TYPE_WARNING, "Could not process file %s!", path_p);
        return 1;
    }
    else
    {
        serv_term_printf(SERV_TERM_PRINT_TYPE_INFO, "Sucessfully opened and processed %s!", path_p);
        return 0;
    }

    return 0;
}
