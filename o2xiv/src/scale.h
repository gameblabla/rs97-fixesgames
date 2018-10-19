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
#ifndef SCALE_H
#define SCALE_H

#include "SDL.h"

#include "fixed.h"

void fixed_scale_8( SDL_Surface *src, SDL_Surface *dst, const fixed src_x, fixed src_y, const SDL_Rect *dst_area, const fixed scale, const fixed scale_inv );
void fixed_scale_16( SDL_Surface *src, SDL_Surface *dst, const fixed src_x, fixed src_y, const SDL_Rect *dst_area, const fixed scale, const fixed scale_inv );
void fixed_scale_24( SDL_Surface *src, SDL_Surface *dst, const fixed src_x, fixed src_y, const SDL_Rect *dst_area, const fixed scale, const fixed scale_inv );
void fixed_scale_32( SDL_Surface *src, SDL_Surface *dst, const fixed src_x, fixed src_y, const SDL_Rect *dst_area, const fixed scale, const fixed scale_inv );

#endif

// kate: tab-width 4;
