/*
 * O2xIV
 * Copyright (C) 2008 Carl Reinke
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef MAIN_H
#define MAIN_H

#include "fixed.h"

#include <stdbool.h>

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

#define countof(x) (sizeof(x) / sizeof(*x))

#define BACKGROUND 0x0000 /* 0x110a */
#define HEADER     0x8518
#define SELECTED   0xffff
#define NORMAL     0x6d12

extern bool quit;

extern bool need_redraw;

extern int delay;

extern SDL_Surface *surface, *image;

extern fixed fit_outside, fit_inside;

void set_scale( fixed new_scale );

#endif

// kate: tab-width 4;
