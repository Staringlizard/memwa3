/*
 * memwa2 state machine
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

#include "stm32h7xx_hal.h"
#include "if.h"

typedef enum
{
	FSM_EVENT_TIMER_100MS,
	FSM_EVENT_TIMER_1000MS,
	FSM_EVENT_STATS_FPS,
	FSM_EVENT_STATS_LED,
	FSM_EVENT_SERIAL_READ,
	FSM_EVENT_SERIAL_WRITE,
	FSM_EVENT_TAPE_PLAY,
	FSM_EVENT_TAPE_MOTOR,
	FSM_EVENT_KEY,
	FSM_EVENT_FADE_DONE
} fsm_event_t;

void fsm_init();
void fsm_run();
void fsm_event(fsm_event_t e, uint32_t edata1, uint32_t edata2);
