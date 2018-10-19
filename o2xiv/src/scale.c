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
#include "main.h"
#include "scale.h"

#include "SDL.h"

#include <assert.h>

static void fixed_scale_row_8( const Uint8 *src, Uint8 *const dst, fixed src_x, const fixed src_x_inc, const fixed src_x_inc_inv, const SDL_Palette *palette, const int dst_w );
static void fixed_scale_row_16( const Uint8 *src, Uint8 *const dst, fixed src_x, const fixed src_x_inc, const fixed src_x_inc_inv, const int dst_w );
static void fixed_scale_row_24( const Uint8 *src, Uint8 *const dst, fixed src_x, const fixed src_x_inc, const fixed src_x_inc_inv, const int dst_w );
static void fixed_scale_row_32( const Uint8 *src, Uint8 *const dst, fixed src_x, const fixed src_x_inc, const fixed src_x_inc_inv, const int dst_w );

static void convert_format( SDL_Surface *surface ) {
	Uint32 color;
	Uint8 r, g, b, a;
	
	Uint8 *pixel = surface->pixels;
	
	const int Bpp = surface->format->BytesPerPixel;
	
	const int Rshift = surface->format->Rshift,
	          Gshift = surface->format->Gshift,
	          Bshift = surface->format->Bshift,
	          Ashift = surface->format->Ashift;
	const int Rloss = surface->format->Rloss,
	          Gloss = surface->format->Gloss,
	          Bloss = surface->format->Bloss,
	          Aloss = surface->format->Aloss;
	
	surface->format->Rmask = (Bpp == 4) ? 0x000000ff : (Bpp == 3) ? 0x0000ff : 0xf800;
	surface->format->Gmask = (Bpp == 4) ? 0x0000ff00 : (Bpp == 3) ? 0x00ff00 : 0x07e0;
	surface->format->Bmask = (Bpp == 4) ? 0x00ff0000 : (Bpp == 3) ? 0xff0000 : 0x001f;
	surface->format->Amask = (Bpp == 4) ? 0xff000000 : 0;
	surface->format->Rshift = (Bpp == 4 || Bpp == 3) ? 0 : 11;
	surface->format->Gshift = (Bpp == 4 || Bpp == 3) ? 8 : 5;
	surface->format->Bshift = (Bpp == 4 || Bpp == 3) ? 16 : 0;
	surface->format->Ashift = (Bpp == 4) ? 24 : 0;
	surface->format->Rloss = (Bpp == 4 || Bpp == 3) ? 0 : 3;
	surface->format->Gloss = (Bpp == 4 || Bpp == 3) ? 0 : 2;
	surface->format->Bloss = (Bpp == 4 || Bpp == 3) ? 0 : 3;
	surface->format->Aloss = (Bpp == 4) ? 0 : 8;
	
	const int new_Rshift = surface->format->Rshift,
	          new_Gshift = surface->format->Gshift,
	          new_Bshift = surface->format->Bshift,
	          new_Ashift = surface->format->Ashift;
	const int new_Rloss = surface->format->Rloss,
	          new_Gloss = surface->format->Gloss,
	          new_Bloss = surface->format->Bloss,
	          new_Aloss = surface->format->Aloss;
	
	for (int y = surface->h; y; y--)
		for (int x = surface->w; x; x--) {
			color = 0;
			for (int b = Bpp - 1; b >= 0; b--) {
				color <<= 8;
				color |= *(pixel + b);
			}
			
			r = (color >> Rshift) << Rloss;
			g = (color >> Gshift) << Gloss;
			b = (color >> Bshift) << Bloss;
			a = (color >> Ashift) << Aloss;
			
			color = ((Uint32)r >> new_Rloss) << new_Rshift;
			color |= ((Uint32)g >> new_Gloss) << new_Gshift;
			color |= ((Uint32)b >> new_Bloss) << new_Bshift;
			color |= ((Uint32)a >> new_Aloss) << new_Ashift;
			
			for (int b = 0; b < Bpp; b++) {
				*(pixel + b) = color & 0xff;
				color >>= 8;
			}
			
			pixel += Bpp;
		}
}


void fixed_scale_8( SDL_Surface *src, SDL_Surface *dst, const fixed src_x, fixed src_y, const SDL_Rect *dst_area, const fixed scale, const fixed scale_inv ) {
	const int Bpp = 1, tempBpp = 3;
	
	assert(src->format->BytesPerPixel == Bpp);
	
	assert(dst->format->BytesPerPixel == 2);
	assert(dst->format->Rmask == 0xf800);
	assert(dst->format->Gmask == 0x07e0);
	assert(dst->format->Bmask == 0x001f);
	
	unsigned int       dst_y = dst_area->y;
	const unsigned int dst_y_max = dst_y + dst_area->h;
	
	const unsigned int dst_x = dst_area->x;
	const unsigned int dst_x_max = dst_x + dst_area->w;
	const unsigned int dst_w = dst_area->w;
	
	assert(dst_y < (unsigned)dst->h);
	assert(dst_y_max <= (unsigned)dst->h);
	assert(dst_x < (unsigned)dst->w);
	assert(dst_x_max <= (unsigned)dst->w);
	
	const fixed src_y_inc = scale_inv,
	            src_y_inc_inv = scale,
	            src_x_inc = scale_inv,
	            src_x_inc_inv = scale;
	
	assert(fixed_to_int(src_x) < (unsigned)src->w);
	assert(fixed_to_int(src_x + dst_w * src_x_inc) <= (unsigned)src->w);
	
	const Uint8 *src_pixels = (Uint8 *)src->pixels + fixed_to_int(src_x) * Bpp;
	
	Uint8 temp[dst_w * tempBpp], *temp_ = temp;
	fixed final[dst_w * tempBpp], *final_ = final;
	
	for (; dst_y < dst_y_max; dst_y++) {
		fixed multiplier = int_to_fixed(1);
		fixed src_y_left = src_y_inc;
		
		multiplier -= src_y & fixed_mask;
		if (multiplier > src_y_left)
			multiplier = src_y_left;
		if ((multiplier & fixed_mask) != 0) {
			assert(fixed_to_int(src_y) < (unsigned)src->h);
			
			fixed_scale_row_8(src_pixels + fixed_to_int(src_y) * src->pitch, temp, src_x & fixed_mask, src_x_inc, src_x_inc_inv, src->format->palette, dst_w);
			
			final_ += countof(final);
			temp_ += countof(temp);
			for (int b = dst_w * tempBpp; b; b--)
				*(--final_) = *(--temp_) * multiplier;
			
			src_y += multiplier;
			src_y_left -= multiplier;
		} else {
			memset(final, 0, sizeof(final));
		}
		
		//multiplier = int_to_fixed(1);
		while (src_y_left >= int_to_fixed(1)) {
			assert(fixed_to_int(src_y) < (unsigned)src->h);
			
			fixed_scale_row_8(src_pixels + fixed_to_int(src_y) * src->pitch, temp, src_x & fixed_mask, src_x_inc, src_x_inc_inv, src->format->palette, dst_w);
			
			final_ += countof(final);
			temp_ += countof(temp);
			for (int b = dst_w * tempBpp; b; b--)
				*(--final_) += int_to_fixed(*(--temp_));
			
			src_y += int_to_fixed(1);
			src_y_left -= int_to_fixed(1);
		}
		
		multiplier = src_y_left;
		if (multiplier) {
			assert(fixed_to_int(src_y) < (unsigned)src->h);
			
			fixed_scale_row_8(src_pixels + fixed_to_int(src_y) * src->pitch, temp, src_x & fixed_mask, src_x_inc, src_x_inc_inv, src->format->palette, dst_w);
			
			final_ += countof(final);
			temp_ += countof(temp);
			for (int b = dst_w * tempBpp; b; b--)
				*(--final_) += *(--temp_) * multiplier;
			
			src_y += multiplier;
		}
		
		Uint16 *dst_temp = (Uint16 *)((Uint8 *)dst->pixels + dst_y * dst->pitch) + dst_x_max;
		fixed *final_temp = final + countof(final);
		/*for (int x = countof(final); x; x--)
			*(--dst_temp) = fixed_to_int(fixed_mul(*(--final_temp), src_y_inc_inv));*/
		for (int x = countof(final); x; x -= tempBpp) {
			Uint8 b = fixed_to_int(fixed_mul(*(--final_temp), src_y_inc_inv)),
			      g = fixed_to_int(fixed_mul(*(--final_temp), src_y_inc_inv)),
			      r = fixed_to_int(fixed_mul(*(--final_temp), src_y_inc_inv));
			*(--dst_temp) = ((r << 8) & 0xf800) | ((g << 3) & 0x07e0) | (b >> 3);
		}
	}
}

static void fixed_scale_row_8( const Uint8 *src, Uint8 *const dst, fixed src_x, const fixed src_x_inc, const fixed src_x_inc_inv, const SDL_Palette *palette, const int dst_w ) {
	fixed r, g, b;
	
	for (int dst_x = 0; dst_x < dst_w; dst_x++) {
		fixed multiplier = int_to_fixed(1);
		fixed src_x_left = src_x_inc;
		
		multiplier -= (src_x & fixed_mask);
		if (multiplier > src_x_left)
			multiplier = src_x_left;
		if ((multiplier & fixed_mask) != 0) {
			Uint8 temp = *(src + fixed_to_int(src_x));
			r = (palette->colors[temp].r) * multiplier;
			g = (palette->colors[temp].g) * multiplier;
			b = (palette->colors[temp].b) * multiplier;
			
			src_x += multiplier;
			src_x_left -= multiplier;
		} else {
			r = g = b = 0;
		}
		
		//multiplier = int_to_fixed(1);
		while (src_x_left >= int_to_fixed(1)) {
			Uint8 temp = *(src + fixed_to_int(src_x));
			r += (palette->colors[temp].r) * int_to_fixed(1);
			g += (palette->colors[temp].g) * int_to_fixed(1);
			b += (palette->colors[temp].b) * int_to_fixed(1);
			
			src_x += int_to_fixed(1);
			src_x_left -= int_to_fixed(1);
		}
		
		multiplier = src_x_left;
		if (multiplier) {
			Uint16 temp = *(src + fixed_to_int(src_x));
			r += (palette->colors[temp].r) * multiplier;
			g += (palette->colors[temp].g) * multiplier;
			b += (palette->colors[temp].b) * multiplier;
			
			src_x += multiplier;
		}
		
		dst[dst_x * 3 + 0] = fixed_to_int(fixed_mul(r, src_x_inc_inv));
		dst[dst_x * 3 + 1] = fixed_to_int(fixed_mul(g, src_x_inc_inv));
		dst[dst_x * 3 + 2] = fixed_to_int(fixed_mul(b, src_x_inc_inv));
	}
}


void fixed_scale_16( SDL_Surface *src, SDL_Surface *dst, const fixed src_x, fixed src_y, const SDL_Rect *dst_area, const fixed scale, const fixed scale_inv ) {
	const int Bpp = 2, tempBpp = 3;
	
	assert(src->format->BytesPerPixel == Bpp);
	
	if (src->format->Rmask != 0xf800 ||
	    src->format->Gmask != 0x07e0 ||
	    src->format->Bmask != 0x001f )
		convert_format(src);
	
	assert(dst->format->BytesPerPixel == 2);
	assert(dst->format->Rmask == 0xf800);
	assert(dst->format->Gmask == 0x07e0);
	assert(dst->format->Bmask == 0x001f);
	
	unsigned int       dst_y = dst_area->y;
	const unsigned int dst_y_max = dst_y + dst_area->h;
	
	const unsigned int dst_x = dst_area->x;
	const unsigned int dst_x_max = dst_x + dst_area->w;
	const unsigned int dst_w = dst_area->w;
	
	assert(dst_y < (unsigned)dst->h);
	assert(dst_y_max <= (unsigned)dst->h);
	assert(dst_x < (unsigned)dst->w);
	assert(dst_x_max <= (unsigned)dst->w);
	
	const fixed src_y_inc = scale_inv,
	            src_y_inc_inv = scale,
	            src_x_inc = scale_inv,
	            src_x_inc_inv = scale;
	
	assert(fixed_to_int(src_x) < (unsigned)src->w);
	assert(fixed_to_int(src_x + dst_w * src_x_inc) <= (unsigned)src->w);
	
	const Uint8 *src_pixels = (Uint8 *)src->pixels + fixed_to_int(src_x) * Bpp;
	
	Uint8 temp[dst_w * tempBpp], *temp_ = temp;
	fixed final[dst_w * tempBpp], *final_ = final;
	
	for (; dst_y < dst_y_max; dst_y++) {
		fixed multiplier = int_to_fixed(1);
		fixed src_y_left = src_y_inc;
		
		multiplier -= src_y & fixed_mask;
		if (multiplier > src_y_left)
			multiplier = src_y_left;
		if ((multiplier & fixed_mask) != 0) {
			assert(fixed_to_int(src_y) < (unsigned)src->h);
			
			fixed_scale_row_16(src_pixels + fixed_to_int(src_y) * src->pitch, temp, src_x & fixed_mask, src_x_inc, src_x_inc_inv, dst_w);
			
			final_ += countof(final);
			temp_ += countof(temp);
			for (int b = dst_w * tempBpp; b; b--)
				*(--final_) = *(--temp_) * multiplier;
			
			src_y += multiplier;
			src_y_left -= multiplier;
		} else {
			memset(final, 0, sizeof(final));
		}
		
		//multiplier = int_to_fixed(1);
		while (src_y_left >= int_to_fixed(1)) {
			assert(fixed_to_int(src_y) < (unsigned)src->h);
			
			fixed_scale_row_16(src_pixels + fixed_to_int(src_y) * src->pitch, temp, src_x & fixed_mask, src_x_inc, src_x_inc_inv, dst_w);
			
			final_ += countof(final);
			temp_ += countof(temp);
			for (int b = dst_w * tempBpp; b; b--)
				*(--final_) += int_to_fixed(*(--temp_));
			
			src_y += int_to_fixed(1);
			src_y_left -= int_to_fixed(1);
		}
		
		multiplier = src_y_left;
		if (multiplier) {
			assert(fixed_to_int(src_y) < (unsigned)src->h);
			
			fixed_scale_row_16(src_pixels + fixed_to_int(src_y) * src->pitch, temp, src_x & fixed_mask, src_x_inc, src_x_inc_inv, dst_w);
			
			final_ += countof(final);
			temp_ += countof(temp);
			for (int b = dst_w * tempBpp; b; b--)
				*(--final_) += *(--temp_) * multiplier;
			
			src_y += multiplier;
		}
		
		Uint16 *dst_temp = (Uint16 *)((Uint8 *)dst->pixels + dst_y * dst->pitch) + dst_x_max;
		fixed *final_temp = final + countof(final);
		/*for (int x = countof(final); x; x--)
			*(--dst_temp) = fixed_to_int(fixed_mul(*(--final_temp), src_y_inc_inv));*/
		for (int x = countof(final); x; x -= tempBpp) {
			Uint8 b = fixed_to_int(fixed_mul(*(--final_temp), src_y_inc_inv)),
			      g = fixed_to_int(fixed_mul(*(--final_temp), src_y_inc_inv)),
			      r = fixed_to_int(fixed_mul(*(--final_temp), src_y_inc_inv));
			*(--dst_temp) = (r << 11) | (g << 5) | b;
		}
	}
}

static void fixed_scale_row_16( const Uint8 *src, Uint8 *const dst, fixed src_x, const fixed src_x_inc, const fixed src_x_inc_inv, const int dst_w ) {
	fixed r, g, b;
	
	for (int dst_x = 0; dst_x < dst_w; dst_x++) {
		fixed multiplier = int_to_fixed(1);
		fixed src_x_left = src_x_inc;
		
		multiplier -= (src_x & fixed_mask);
		if (multiplier > src_x_left)
			multiplier = src_x_left;
		if ((multiplier & fixed_mask) != 0) {
			Uint16 temp = *((Uint16 *)src + fixed_to_int(src_x));
			r = ((temp & 0xf800) >> 11) * multiplier;
			g = ((temp & 0x07e0) >> 5) * multiplier;
			b = (temp & 0x001f) * multiplier;
			
			src_x += multiplier;
			src_x_left -= multiplier;
		} else {
			r = g = b = 0;
		}
		
		//multiplier = int_to_fixed(1);
		while (src_x_left >= int_to_fixed(1)) {
			Uint16 temp = *((Uint16 *)src + fixed_to_int(src_x));
			r += ((temp & 0xf800) >> 11) * int_to_fixed(1);
			g += ((temp & 0x07e0) >> 5) * int_to_fixed(1);
			b += (temp & 0x001f) * int_to_fixed(1);
			
			src_x += int_to_fixed(1);
			src_x_left -= int_to_fixed(1);
		}
		
		multiplier = src_x_left;
		if (multiplier) {
			Uint16 temp = *((Uint16 *)src + fixed_to_int(src_x));
			r += ((temp & 0xf800) >> 11) * multiplier;
			g += ((temp & 0x07e0) >> 5) * multiplier;
			b += (temp & 0x001f) * multiplier;
			
			src_x += multiplier;
		}
		
		dst[dst_x * 3 + 0] = fixed_to_int(fixed_mul(r, src_x_inc_inv));
		dst[dst_x * 3 + 1] = fixed_to_int(fixed_mul(g, src_x_inc_inv));
		dst[dst_x * 3 + 2] = fixed_to_int(fixed_mul(b, src_x_inc_inv));
	}
}


void fixed_scale_24( SDL_Surface *src, SDL_Surface *dst, const fixed src_x, fixed src_y, const SDL_Rect *dst_area, const fixed scale, const fixed scale_inv ) {
	/* Working around for crashes on displays with a resolution of 480 pixels or higher - Gameblabla */
	SDL_Surface* tmp;
	tmp = SDL_DisplayFormat(src);
	if (tmp) SDL_SoftStretch(tmp, NULL, dst, (SDL_Rect*)dst_area);
	
	/*const int Bpp = 3;
	
	assert(src->format->BytesPerPixel == Bpp);
	
	if (src->format->Rmask != 0x0000ff ||
	    src->format->Gmask != 0x00ff00 ||
	    src->format->Bmask != 0xff0000)
		convert_format(src);
	
	assert(dst->format->BytesPerPixel == 2);
	assert(dst->format->Rmask == 0xf800);
	assert(dst->format->Gmask == 0x07e0);
	assert(dst->format->Bmask == 0x001f);
	
	unsigned int       dst_y = dst_area->y;
	const unsigned int dst_y_max = dst_y + dst_area->h;
	
	const unsigned int dst_x = dst_area->x;
	const unsigned int dst_x_max = dst_x + dst_area->w;
	const unsigned int dst_w = dst_area->w;
	
	assert(dst_y < (unsigned)dst->h);
	assert(dst_y_max <= (unsigned)dst->h);
	assert(dst_x < (unsigned)dst->w);
	assert(dst_x_max <= (unsigned)dst->w);
	
	const fixed src_y_inc = scale_inv,
	            src_y_inc_inv = scale,
	            src_x_inc = scale_inv,
	            src_x_inc_inv = scale;
	
	assert(fixed_to_int(src_x) < (unsigned)src->w);
	assert(fixed_to_int(src_x + dst_w * src_x_inc) <= (unsigned)src->w);
	
	const Uint8 *src_pixels = (Uint8 *)src->pixels + fixed_to_int(src_x) * Bpp;
	
	Uint8 temp[dst_w * Bpp], *temp_ = temp;
	fixed final[dst_w * Bpp], *final_ = final;
	
	for (; dst_y < dst_y_max; dst_y++) {
		fixed multiplier = int_to_fixed(1);
		fixed src_y_left = src_y_inc;
		
		multiplier -= src_y & fixed_mask;
		if (multiplier > src_y_left)
			multiplier = src_y_left;
		if ((multiplier & fixed_mask) != 0) {
			assert(fixed_to_int(src_y) < (unsigned)src->h);
			
			fixed_scale_row_24(src_pixels + fixed_to_int(src_y) * src->pitch, temp, src_x & fixed_mask, src_x_inc, src_x_inc_inv, dst_w);
			
			final_ += countof(final);
			temp_ += countof(temp);
			for (int b = dst_w * Bpp; b; b--)
				*(--final_) = *(--temp_) * multiplier;
			
			src_y += multiplier;
			src_y_left -= multiplier;
		} else {
			memset(final, 0, sizeof(final));
		}
		
		//multiplier = int_to_fixed(1);
		while (src_y_left >= int_to_fixed(1)) {
			assert(fixed_to_int(src_y) < (unsigned)src->h);
			
			fixed_scale_row_24(src_pixels + fixed_to_int(src_y) * src->pitch, temp, src_x & fixed_mask, src_x_inc, src_x_inc_inv, dst_w);
			
			final_ += countof(final);
			temp_ += countof(temp);
			for (int b = dst_w * Bpp; b; b--)
				*(--final_) += int_to_fixed(*(--temp_));
			
			src_y += int_to_fixed(1);
			src_y_left -= int_to_fixed(1);
		}
		
		multiplier = src_y_left;
		if (multiplier) {
			assert(fixed_to_int(src_y) < (unsigned)src->h);
			
			fixed_scale_row_24(src_pixels + fixed_to_int(src_y) * src->pitch, temp, src_x & fixed_mask, src_x_inc, src_x_inc_inv, dst_w);
			
			final_ += countof(final);
			temp_ += countof(temp);
			for (int b = dst_w * Bpp; b; b--)
				*(--final_) += *(--temp_) * multiplier;
			
			src_y += multiplier;
		}
		
		Uint16 *dst_temp = (Uint16 *)((Uint8 *)dst->pixels + dst_y * dst->pitch) + dst_x_max;
		fixed *final_temp = final + countof(final);
		for (int x = countof(final); x; x -= 3) 
		{
			Uint8 b = fixed_to_int(fixed_mul(*(--final_temp), src_y_inc_inv)),
			      g = fixed_to_int(fixed_mul(*(--final_temp), src_y_inc_inv)),
			      r = fixed_to_int(fixed_mul(*(--final_temp), src_y_inc_inv));
			*(--dst_temp) = ((r << 8) & 0xf800) | ((g << 3) & 0x07e0) | (b >> 3);
		}
	}*/
}

static void fixed_scale_row_24( const Uint8 *src, Uint8 *const dst, fixed src_x, const fixed src_x_inc, const fixed src_x_inc_inv, const int dst_w ) {
	const int Bpp = 3;
	
	fixed temp[Bpp];
	
	for (int dst_x = 0; dst_x < dst_w; dst_x++) {
		fixed multiplier = int_to_fixed(1);
		fixed src_x_left = src_x_inc;
		
		multiplier -= (src_x & fixed_mask);
		if (multiplier > src_x_left)
			multiplier = src_x_left;
		if ((multiplier & fixed_mask) != 0) {
			for (int b = 0; b < Bpp; b++)
				temp[b] = *(src + fixed_to_int(src_x) * Bpp + b) * multiplier;
			
			src_x += multiplier;
			src_x_left -= multiplier;
		} else {
			memset(temp, 0, sizeof(temp));
		}
		
		//multiplier = int_to_fixed(1);
		while (src_x_left >= int_to_fixed(1)) {
			for (int b = 0; b < Bpp; b++)
				temp[b] += int_to_fixed(*(src + fixed_to_int(src_x) * Bpp + b));
			
			src_x += int_to_fixed(1);
			src_x_left -= int_to_fixed(1);
		}
		
		multiplier = src_x_left;
		if (multiplier) {
			for (int b = 0; b < Bpp; b++)
				temp[b] += *(src + fixed_to_int(src_x) * Bpp + b) * multiplier;
			
			src_x += multiplier;
		}
		
		for (int b = 0; b < Bpp; b++)
			dst[dst_x * Bpp + b] = fixed_to_int(fixed_mul(temp[b], src_x_inc_inv));
	}
}


void fixed_scale_32( SDL_Surface *src, SDL_Surface *dst, const fixed src_x, fixed src_y, const SDL_Rect *dst_area, const fixed scale, const fixed scale_inv ) {
	SDL_Surface* tmp;
	tmp = SDL_DisplayFormatAlpha(src);
	if (tmp) SDL_SoftStretch(tmp, NULL, dst, NULL);
	/*const int Bpp = 4;
	
	assert(src->format->BytesPerPixel == Bpp);
	
	if (src->format->Rmask != 0x000000ff ||
	    src->format->Gmask != 0x0000ff00 ||
	    src->format->Bmask != 0x00ff0000 ||
	    src->format->Amask != 0xff000000)
		convert_format(src);
	
	assert(dst->format->BytesPerPixel == 2);
	assert(dst->format->Rmask == 0xf800);
	assert(dst->format->Gmask == 0x07e0);
	assert(dst->format->Bmask == 0x001f);
	
	unsigned int       dst_y = dst_area->y;
	const unsigned int dst_y_max = dst_y + dst_area->h;
	
	const unsigned int dst_x = dst_area->x;
	const unsigned int dst_x_max = dst_x + dst_area->w;
	const unsigned int dst_w = dst_area->w;
	
	assert(dst_y < (unsigned)dst->h);
	assert(dst_y_max <= (unsigned)dst->h);
	assert(dst_x < (unsigned)dst->w);
	assert(dst_x_max <= (unsigned)dst->w);
	
	const fixed src_y_inc = scale_inv,
	            src_y_inc_inv = scale,
	            src_x_inc = scale_inv,
	            src_x_inc_inv = scale;
	
	assert(fixed_to_int(src_x) < (unsigned)src->w);
	assert(fixed_to_int(src_x + dst_w * src_x_inc) <= (unsigned)src->w);
	
	const Uint8 *src_pixels = (Uint8 *)src->pixels + fixed_to_int(src_x) * Bpp;
	
	Uint8 temp[dst_w * Bpp], *temp_ = temp;
	fixed final[dst_w * Bpp], *final_ = final;
	
	for (; dst_y < dst_y_max; dst_y++) {
		fixed multiplier = int_to_fixed(1);
		fixed src_y_left = src_y_inc;
		
		multiplier -= src_y & fixed_mask;
		if (multiplier > src_y_left)
			multiplier = src_y_left;
		if ((multiplier & fixed_mask) != 0) {
			assert(fixed_to_int(src_y) < (unsigned)src->h);
			
			fixed_scale_row_32(src_pixels + fixed_to_int(src_y) * src->pitch, temp, src_x & fixed_mask, src_x_inc, src_x_inc_inv, dst_w);
			
			final_ += countof(final);
			temp_ += countof(temp);
			for (int b = dst_w * Bpp; b; b--)
				*(--final_) = *(--temp_) * multiplier;
			
			src_y += multiplier;
			src_y_left -= multiplier;
		} else {
			memset(final, 0, sizeof(final));
		}
		
		//multiplier = int_to_fixed(1);
		while (src_y_left >= int_to_fixed(1)) {
			assert(fixed_to_int(src_y) < (unsigned)src->h);
			
			fixed_scale_row_32(src_pixels + fixed_to_int(src_y) * src->pitch, temp, src_x & fixed_mask, src_x_inc, src_x_inc_inv, dst_w);
			
			final_ += countof(final);
			temp_ += countof(temp);
			for (int b = dst_w * Bpp; b; b--)
				*(--final_) += int_to_fixed(*(--temp_));
			
			src_y += int_to_fixed(1);
			src_y_left -= int_to_fixed(1);
		}
		
		multiplier = src_y_left;
		if (multiplier) {
			assert(fixed_to_int(src_y) < (unsigned)src->h);
			
			fixed_scale_row_32(src_pixels + fixed_to_int(src_y) * src->pitch, temp, src_x & fixed_mask, src_x_inc, src_x_inc_inv, dst_w);
			
			final_ += countof(final);
			temp_ += countof(temp);
			for (int b = dst_w * Bpp; b; b--)
				*(--final_) += *(--temp_) * multiplier;
			
			src_y += multiplier;
		}
		
		Uint16 *dst_temp = (Uint16 *)((Uint8 *)dst->pixels + dst_y * dst->pitch) + dst_x_max;
		fixed *final_temp = final + countof(final);
		for (int x = countof(final); x; x -= 4) {
			Uint8 a = fixed_to_int(fixed_mul(*(--final_temp), src_y_inc_inv));
			fixed src_y_inc_inv_temp = fixed_mul(src_y_inc_inv, int_div_int_to_fixed(a, 255)); //apply alpha channel //! doesn't work for xcf
			Uint8 b = fixed_to_int(fixed_mul(*(--final_temp), src_y_inc_inv_temp)),
			      g = fixed_to_int(fixed_mul(*(--final_temp), src_y_inc_inv_temp)),
			      r = fixed_to_int(fixed_mul(*(--final_temp), src_y_inc_inv_temp));
			*(--dst_temp) = ((r << 8) & 0xf800) | ((g << 3) & 0x07e0) | (b >> 3);
		}
	}*/
}

static void fixed_scale_row_32( const Uint8 *src, Uint8 *const dst, fixed src_x, const fixed src_x_inc, const fixed src_x_inc_inv, const int dst_w ) {
	const int Bpp = 4;
	
	fixed temp[Bpp];
	
	for (int dst_x = 0; dst_x < dst_w; dst_x++) {
		fixed multiplier = int_to_fixed(1);
		fixed src_x_left = src_x_inc;
		
		multiplier -= (src_x & fixed_mask);
		if (multiplier > src_x_left)
			multiplier = src_x_left;
		if ((multiplier & fixed_mask) != 0) {
			for (int b = 0; b < Bpp; b++)
				temp[b] = *(src + fixed_to_int(src_x) * Bpp + b) * multiplier;
			
			src_x += multiplier;
			src_x_left -= multiplier;
		} else {
			memset(temp, 0, sizeof(temp));
		}
		
		//multiplier = int_to_fixed(1);
		while (src_x_left >= int_to_fixed(1)) {
			for (int b = 0; b < Bpp; b++)
				temp[b] += int_to_fixed(*(src + fixed_to_int(src_x) * Bpp + b));
			
			src_x += int_to_fixed(1);
			src_x_left -= int_to_fixed(1);
		}
		
		multiplier = src_x_left;
		if (multiplier) {
			for (int b = 0; b < Bpp; b++)
				temp[b] += *(src + fixed_to_int(src_x) * Bpp + b) * multiplier;
			
			src_x += multiplier;
		}
		
		for (int b = 0; b < Bpp; b++)
			dst[dst_x * Bpp + b] = fixed_to_int(fixed_mul(temp[b], src_x_inc_inv));
	}
}

// kate: tab-width 4;
