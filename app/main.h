/*
 * memwa2 main
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


#ifndef _MAIN_H
#define _MAIN_H

#include "stm32h7xx_hal.h"
#include <stdio.h>

/* This must ALWAYS be uppdated for every release */
#define FW_REVISION             "V1.0.7"

void main_error(char *string_p, char *file_p, uint32_t line, uint32_t extra);
void main_warning(char *string_p, char *file_p, uint32_t line, uint32_t extra);
char *main_get_fw_revision();

#endif
