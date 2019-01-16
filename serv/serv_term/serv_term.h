/*
 * Term
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


#ifndef _SERV_TERM_H
#define _SERV_TERM_H

#include "stm32h7xx_hal.h"
#include <string.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define serv_term_printf(type, str, args...) \
	switch(type) \
	{ \
		case SERV_TERM_PRINT_TYPE_INFO: \
			printf("I[%s:%d]: ", __FILENAME__, __LINE__); \
			break; \
		case SERV_TERM_PRINT_TYPE_WARNING: \
			printf("W[%s:%d]: ", __FILENAME__, __LINE__); \
			break; \
		case SERV_TERM_PRINT_TYPE_ERROR: \
			printf("E[%s:%d]: ", __FILENAME__, __LINE__); \
			break; \
		case SERV_TERM_PRINT_TYPE_DEBUG: \
			printf("D[%s:%d]: ", __FILENAME__, __LINE__); \
			break; \
	} \
	printf(str, ##args); \
	printf("\n");

typedef enum
{
	SERV_TERM_PRINT_TYPE_INFO,
	SERV_TERM_PRINT_TYPE_WARNING,
	SERV_TERM_PRINT_TYPE_ERROR,
	SERV_TERM_PRINT_TYPE_DEBUG
} serv_term_print_type_t;

void serv_term_init();
char *serv_term_get_row(uint32_t row);
uint32_t serv_term_get_rows();
void serv_term_clear_rows();
void serv_term_receive(uint8_t *buf_p, uint32_t len);

#endif
