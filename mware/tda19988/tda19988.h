/*
 * memwa2 tda19988 driver
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


#ifndef _TDA19988_H
#define _TDA19988_H

#include "stm32h7xx_hal.h"
#include "main.h"

/*
 * Video mode flags.
 */

#define VID_PHSYNC	0x0001
#define VID_NHSYNC	0x0002
#define VID_PVSYNC	0x0004
#define VID_NVSYNC	0x0008
#define VID_INTERLACE	0x0010
#define VID_DBLSCAN	0x0020
#define VID_CSYNC	0x0040
#define VID_PCSYNC	0x0080
#define VID_NCSYNC	0x0100
#define VID_HSKEW	0x0200
#define VID_BCAST	0x0400
#define VID_PIXMUX	0x1000
#define VID_DBLCLK	0x2000
#define VID_CLKDIV2	0x4000

/*
 * These macros help the modelines below fit on one line.
 */
#define HP VID_PHSYNC
#define HN VID_NHSYNC
#define VP VID_PVSYNC
#define VN VID_NVSYNC
#define I VID_INTERLACE
#define DS VID_DBLSCAN


#define M(nm,hr,vr,clk,hs,he,ht,vs,ve,vt,f) \
	{ clk, hr, hs, he, ht, vr, vs, ve, vt, f, nm } 

typedef struct {
	int dot_clock;		/* Dot clock frequency in kHz. */
	int hdisplay;
	int hsync_start;
	int hsync_end;
	int htotal;
	int vdisplay;
	int vsync_start;
	int vsync_end;
	int vtotal;
	int flags;		/* Video mode flags; see below. */
	const char *name;
	int hskew;
} tda19988_vm_t;

#define TDA19988_ADDR_HDMI 	0xE0
#define TDA19988_ADDR_CEC 	0x68

void tda19988_init();
void tda19988_init_encoder(tda19988_vm_t *mode);
void tda19988_configure();
void tda19988_irq();

#endif
