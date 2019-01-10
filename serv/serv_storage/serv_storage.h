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


#ifndef _SERV_STORAGE_H
#define _SERV_STORAGE_H

#include "stm32h7xx_hal.h"
#include "serv_term.h"
#include "ff.h"
#include <stdio.h>
#include <ctype.h>

#define MAX_FNAME_LEN   48

typedef void (*scanned_files_t)(uint32_t tot_files);

typedef enum
{
	SERV_STORAGE_FILE_TYPE_T64,
	SERV_STORAGE_FILE_TYPE_PRG,
	SERV_STORAGE_FILE_TYPE_TAP,
	SERV_STORAGE_FILE_TYPE_D64
} serv_storage_file_type_t;

typedef struct
{
    char fname_p[MAX_FNAME_LEN];
    serv_storage_file_type_t type;
    uint32_t size;
} serv_storage_file_t;

void serv_storage_init();
void serv_storage_mount();
void serv_storage_read_config();
void serv_storage_scan_files_cb(scanned_files_t cb);
void serv_storage_unscan_files();
uint8_t serv_storage_scan_files(serv_storage_file_t **entries_pp, uint32_t *files_p);
void serv_storage_files_filter(serv_storage_file_t *entries_p, uint32_t *files_p, serv_storage_file_t **result_pp, char *filter_p);
uint8_t serv_storage_load_file(serv_storage_file_t *file_p);

#endif
