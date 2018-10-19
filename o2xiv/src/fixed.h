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
#ifndef FIXED_H
#define FIXED_H

#include "SDL.h"

#define fixed_bits 16
#define fixed_mask ((1 << fixed_bits) - 1)
typedef Uint32 fixed;
typedef Uint64 fixed_dbl;

#define fixed_to_float(fixed_) ((float)(fixed_) / (1 << fixed_bits))
#define int_to_fixed(int_) ((fixed)(int_) << fixed_bits)
#define fixed_to_int(fixed_) ((fixed_) >> fixed_bits)

#define fixed_mul(fixed1, fixed2) (((fixed_dbl)(fixed1) * (fixed2)) >> fixed_bits)
#define fixed_div(fixed1, fixed2) (((fixed_dbl)(fixed1) << fixed_bits) / (fixed2))

#define int_div_int_to_fixed(int1, int2) (int_to_fixed(int1) / (int2))
#define int_div_fixed_to_fixed(int_, fixed_) (fixed_div(int_to_fixed(int_), (fixed_)))

#endif

// kate: tab-width 4;
