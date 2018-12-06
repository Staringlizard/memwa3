/*
 * memwa2 configuration
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


#ifndef _CONFIG_H
#define _CONFIG_H

#include "stm32h7xx_hal_dma.h"
#include "stm32h7xx_hal_sd.h"
#include "stm32h7xx_hal_hcd.h"
#include "stm32h7xx_hal_i2c.h"
#include "stm32h7xx_hal_sdram.h"
#include "main.h"

#define SDCARD_CD_PIN   GPIO_PIN_7
#define SDCARD_CD_PORT  GPIOA   

#define SID_SET_CS_HIGH() \
  GPIOI->BSRRL = GPIO_PIN_6;

#define SID_SET_CS_LOW() \
  GPIOI->BSRRH = GPIO_PIN_6;

#define SID_ASSERT_READ() \
  GPIOI->BSRRL = GPIO_PIN_5;

#define SID_ASSERT_WRITE() \
  GPIOI->BSRRH = GPIO_PIN_5;

#define SID_SET_DATA(data) \
  GPIOH->BSRRH = 0xFF00; \
  GPIOH->BSRRL = data << 8;

#define SID_SET_ADDR(addr) \
  GPIOI->BSRRH = 0x0F80; \
  GPIOI->BSRRL = (addr & 0x1F) << 7;

#define SID_GET_DATA() \
  (GPIOH->IDR & 0xFF00) >> 8;

#define LED_SET_R1_HIGH() \
  GPIOC->BSRRL = GPIO_PIN_13;

#define LED_SET_R1_LOW() \
  GPIOC->BSRRH = GPIO_PIN_13;

#define LED_SET_G1_HIGH() \
  GPIOC->BSRRL = GPIO_PIN_14;

#define LED_SET_G1_LOW() \
  GPIOC->BSRRH = GPIO_PIN_14;

#define LED_SET_B1_HIGH() \
  GPIOC->BSRRL = GPIO_PIN_15;

#define LED_SET_B1_LOW() \
  GPIOC->BSRRH = GPIO_PIN_15;


void config_init();

#endif
