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

#include "drv_led.h"
#include "hal_conf.h"

static uint8_t g_red;
static uint8_t g_green;
static uint8_t g_blue;
static uint16_t g_red_cnt;
static uint16_t g_green_cnt;
static uint16_t g_blue_cnt;
static uint16_t g_red_limit;
static uint16_t g_green_limit;
static uint16_t g_blue_limit;

__weak void HAL_LED_MspInit()
{
  ;
}

void drv_led_init()
{
    HAL_LED_MspInit();
}

void drv_led_set(uint8_t red, uint8_t green, uint8_t blue)
{
    g_red = red;
    g_green = green;
    g_blue = blue;

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

void drv_led_set_red(uint8_t value)
{
    g_red = value;
    drv_led_set(g_red, g_green, g_blue);
}

void drv_led_set_green(uint8_t value)
{
    g_green = value;
    drv_led_set(g_red, g_green, g_blue);
}

void drv_led_set_blue(uint8_t value)
{
    g_blue = value;
    drv_led_set(g_red, g_green, g_blue);
}

void drv_led_toggle_red()
{
    if(g_red_cnt >= g_red_limit)
    {
        g_red = !g_red;
        drv_led_set(g_red, g_green, g_blue);
        g_red_cnt = 0;
    }
    else
    {
        g_red_cnt++;
    }
}

void drv_led_toggle_green()
{
    if(g_green_cnt >= g_green_limit)
    {
        g_green = !g_green;
        drv_led_set(g_red, g_green, g_blue);
        g_green_cnt = 0;
    }
    else
    {
        g_green_cnt++;
    }
}

void drv_led_toggle_blue()
{
    if(g_blue_cnt >= g_blue_limit)
    {
        g_blue = !g_blue;
        drv_led_set(g_red, g_green, g_blue);
        g_blue_cnt = 0;
    }
    else
    {
        g_blue_cnt++;
    }
}

void drv_led_toggle_limit_red(uint16_t value)
{
    g_red_limit = value;
}

void drv_led_toggle_limit_green(uint16_t value)
{
    g_green_limit = value;
}

void drv_led_toggle_limit_blue(uint16_t value)
{
    g_blue_limit = value;
}
