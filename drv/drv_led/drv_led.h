/*
 * memwa2 random generator driver
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


#ifndef _DRV_LED_H
#define _DRV_LED_H

#include "stm32h7xx_hal.h"
#include "main.h"

void drv_led_init();
uint32_t drv_led_set(uint8_t red, uint8_t green, uint8_t blue);
void drv_led_set_red(uint8_t value);
void drv_led_set_green(uint8_t value);
void drv_led_set_blue(uint8_t value);
void drv_led_toggle_red();
void drv_led_toggle_green();
void drv_led_toggle_blue();
void drv_led_toggle_limit_red(uint16_t value);
void drv_led_toggle_limit_green(uint16_t value);
void drv_led_toggle_limit_blue(uint16_t value);

#endif
