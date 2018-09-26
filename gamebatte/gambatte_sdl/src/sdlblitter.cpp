/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aam�s                                    *
 *   aamas@stud.ntnu.no                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <time.h>
#include "sdlblitter.h"

#include "scalebuffer.h"
#include "../menu.h"
#include "../scaler.h"

SDL_Surface *ScreenSurface=NULL;

SdlBlitter::SdlBlitter(const bool startFull, const Uint8 scale, const bool yuv) :
screen(NULL),
surface(NULL),
overlay(NULL),
startFlags(SDL_HWSURFACE | SDL_DOUBLEBUF | (startFull ? SDL_FULLSCREEN : 0)),
scale(scale),
scaler(1),
yuv(yuv)
{}

SdlBlitter::~SdlBlitter() {
	if (overlay) {
		SDL_UnlockYUVOverlay(overlay);
		SDL_FreeYUVOverlay(overlay);
	}
	
	if (surface != screen)
		SDL_FreeSurface(surface);
}

void SdlBlitter::setBufferDimensions(const unsigned int width, const unsigned int height) {
	//surface = screen = SDL_SetVideoMode(width * scale, height * scale, SDL_GetVideoInfo()->vfmt->BitsPerPixel == 16 ? 16 : 32, screen ? screen->flags : startFlags);
	//surface = screen = SDL_SetVideoMode(320, 240, 16, screen ? screen->flags : startFlags);
	ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
	surface = screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0, 0, 0, 0);

	menu_set_screen(screen);
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 16, 0, 0, 0, 0);
	//fprintf(stderr, "surface w: %d, h: %d, pitch: %d, bpp: %d\n", surface->w, surface->h, surface->pitch, surface->format->BitsPerPixel);
	//fprintf(stderr, "hwscreen w: %d, h: %d, pitch: %d, bpp %d\n", screen->w, screen->h, screen->pitch, screen->format->BitsPerPixel);
	//surface = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, screen->format->BitsPerPixel, 0, 0, 0, 0);
	/*
	if (scale > 1 && screen) {
		if (yuv) {
			if ((overlay = SDL_CreateYUVOverlay(width * 2, height, SDL_UYVY_OVERLAY, screen)))
				SDL_LockYUVOverlay(overlay);
		} else
			surface = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, screen->format->BitsPerPixel, 0, 0, 0, 0);
	}
	*/
}

const SdlBlitter::PixelBuffer SdlBlitter::inBuffer() const {
	PixelBuffer pb;
	if (overlay) {
		pb.pixels = overlay->pixels[0];
		pb.format = UYVY;
		pb.pitch = overlay->pitches[0] >> 2;
	} else if (surface) {
		pb.pixels = (Uint8*)(surface->pixels) + surface->offset;
		pb.format = surface->format->BitsPerPixel == 16 ? RGB16 : RGB32;
		pb.pitch = surface->pitch / surface->format->BytesPerPixel;
	}
	
	return pb;
}

template<typename T>
inline void SdlBlitter::swScale() {
	scaleBuffer<T>((T*)((Uint8*)(surface->pixels) + surface->offset), (T*)((Uint8*)(screen->pixels) + screen->offset), surface->w, surface->h, screen->pitch / screen->format->BytesPerPixel, scale);
}

static int frames = 0;
static clock_t old_time = 0;
static int fps = 0;
	
void SdlBlitter::draw() {
	clock_t cur_time;
	size_t offset;
	++frames;
	cur_time = SDL_GetTicks();

	if (cur_time > old_time + 1000) {
		fps = frames;
		frames = 0;
		old_time = cur_time;
	}
	if (!screen || !surface)
		return;
	
	switch(scaler) {
		case 0:		/* Ayla's fullscreen scaler */
			SDL_LockSurface(screen);
			SDL_LockSurface(surface);
			fullscreen_upscale((uint32_t*)screen->pixels, (uint32_t*)surface->pixels);
			SDL_UnlockSurface(surface);
			SDL_UnlockSurface(screen);
			break;
		case 1:		/* Ayla's 1.5x scaler */
			SDL_LockSurface(screen);
			SDL_LockSurface(surface);
			offset = (2 * (320 - 240) / 2) + ((240 - 216) / 2) * screen->pitch;
			scale15x((uint32_t*)((uint8_t *)screen->pixels + offset), (uint32_t*)surface->pixels);
			SDL_UnlockSurface(surface);
			SDL_UnlockSurface(screen);
			break;
		case 2:		/* no scaler */
		default:
			SDL_Rect dst;
			dst.x = (screen->w - surface->w) / 2;
			dst.y = (screen->h - surface->h) / 2;
			dst.w = surface->w;
			dst.h = surface->h;
			SDL_BlitSurface(surface, NULL, screen, &dst);
			break;
	}
	
	/*
	if (!overlay && surface != screen) {
		if (surface->format->BitsPerPixel == 16)
			swScale<Uint16>();
		else
			swScale<Uint32>();
	}
	*/
	
	show_fps(screen, fps);
}

void SdlBlitter::present() {
	if (!screen || !surface)
		return;
	if (overlay) {
		SDL_Rect dstr = { 0, 0, screen->w, screen->h };
		SDL_UnlockYUVOverlay(overlay);
		SDL_DisplayYUVOverlay(overlay, &dstr);
		SDL_LockYUVOverlay(overlay);
	} else {
		//SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
		//SDL_Flip(screen);
		//SDL_SoftStretch(screen, NULL, ScreenSurface, NULL);
		if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
		int x, y;
		uint32_t *s = (uint32_t*)screen->pixels;
		uint32_t *d = (uint32_t*)ScreenSurface->pixels;
		for(uint8_t y = 0; y < 240; y++, s += 160, d += 320) 
			memmove((uint32_t*)d, (uint32_t*)s, 1280);
		if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
		SDL_Flip(ScreenSurface);
	}
}

void SdlBlitter::toggleFullScreen() {
	//if (screen)
	//	screen = SDL_SetVideoMode(screen->w, screen->h, screen->format->BitsPerPixel, screen->flags ^ SDL_FULLSCREEN);
	//menu_set_screen(screen);	
}
