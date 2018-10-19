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
#ifndef FONT_H
#define FONT_H

#include "SDL.h"

void char8x8( SDL_Surface *const surface, int x, int y, const char c, const Uint16 color );
void text8x8( SDL_Surface *const surface, int x, int y, const char text[], const Uint16 color );
void outline8x8( SDL_Surface *const surface, int x, int y, const char text[], const Uint16 color, const Uint16 outline );

#endif

// kate: tab-width 4;
