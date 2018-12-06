/*
 * memwa2 rgb led driver
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
 * Responsible for the rgb led indicator.
 */

#include "led.h"
#include "config.h"

static RNG_HandleTypeDef g_rng_handle;

__weak void HAL_LED_MspInit()
{
  ;
}

void led_init()
{
    HAL_LED_MspInit();
}

uint32_t led_set(uint8_t red, uint8_t green, uint8_t blue)
{
    if(red)
    {
        LED_SET_R1_LOW();
    }
    else
    {
        LED_SET_R1_HIGH();   
    }

    if(green)
    {
        LED_SET_G1_LOW();
    }
    else
    {
        LED_SET_G1_HIGH();   
    }

    if(blue)
    {
        LED_SET_B1_LOW();
    }
    else
    {
        LED_SET_B1_HIGH();   
    }
}
