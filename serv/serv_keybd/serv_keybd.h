/*
 * memwa2 keyboard utility
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


#ifndef _SERV_KEYBD_H
#define _SERV_KEYBD_H

#include "stm32h7xx_hal.h"
#include "serv_term.h"
#include "if.h"

#define SERV_KEYBD_SIMUL_KEYS 	6
#define SERV_KEYBD_MAP_SIZE     72

typedef enum
{
    SERV_KEYBD_STATE_RELEASED,
    SERV_KEYBD_STATE_PRESSED,
    SERV_KEYBD_STATE_LONG_PRESS,
} serv_keybd_state_t;

typedef void (*key_event_t)(uint8_t key, serv_keybd_state_t state);

void serv_keybd_init();
void serv_keybd_poll();
serv_keybd_state_t serv_keybd_key_state();
serv_keybd_state_t serv_keybd_get_shift_state();
serv_keybd_state_t serv_keybd_get_ctrl_state();
uint8_t *serv_keybd_get_key_array();
uint8_t serv_keybd_get_active_keys_hash();
void serv_keybd_reg_key_event_fp_cb(key_event_t event);
uint8_t serv_keybd_get_ascii(char c);
if_keybd_map_t *serv_keybd_get_default_map();
void serv_keybd_populate_map(uint8_t *conf_text, if_keybd_map_t *map_p);

#endif
