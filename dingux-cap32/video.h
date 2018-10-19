/* Caprice32 - Amstrad CPC Emulator
   (c) Copyright 1997-2004 Ulrich Doewich

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __VIDEO__H__
#define __VIDEO__H__

#include "cap32.h"
#include "SDL.h"

/* the pixel formats video plugins can support */
#define F8_BPP		1<<0
#define F15_BPP		1<<1
#define F15_BPP_REV	1<<2
#define F16_BPP		1<<3
#define F16_BPP_REV	1<<4
#define F24_BPP		1<<5
#define F24_BPP_REV	1<<6
#define F32_BPP		1<<7
#define F32_BPP_REV	1<<8

#define ALL		0xffffffff

# endif
