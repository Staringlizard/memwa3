/*
 * memwa2 timer driver
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


#ifndef _DRV_TIMER_H
#define _DRV_TIMER_H

#include "stm32h7xx_hal.h"
#include "serv_term.h"

void drv_timer_init();
void drv_timer_tick();
uint32_t drv_timer_get_ms();
void drv_timer_3_set(uint32_t value);
void drv_timer_3_tick();
uint32_t drv_timer_3_get();

#endif
