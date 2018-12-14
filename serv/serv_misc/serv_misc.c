/*
 * Mem
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

#include "serv_misc.h"
#include "if.h"
#include "hal_msp.h"
#include "drv_led.h"
#include "drv_crc.h"
#include "drv_rng.h"
#include "drv_timer.h"

void serv_misc_init()
{
    drv_led_init();
    drv_crc_init();
    drv_rng_init();
    drv_timer_init();
    
}

