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
#include "files.h"
#include "font.h"
#include "input.h"
#include "main.h"
#include "menu.h"
#include "prefs.h"
#include "scale.h"
#include "thread.h"

#include <SDL/SDL.h>
#include <SDL/SDL_rotozoom.h>

#include <assert.h>
#include <libgen.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#ifndef strdup
#define strdup(src) (strcpy(malloc(strlen(src) + 1), src))
#endif

SDL_Surface *ScreenSurface;
int main( int argc, char *argv[] );

void set_rotation( int new_rotation );
void calculate_auto_fit( void );

void redraw( void );
void rotate( void );

bool quit = false;

bool need_redraw = true;
int delay;

#define NOTE_SHOW  0x01
#define NOTE_DELAY 0x02
#define NOTE_MENU  0x04
int note_flags[3] = { NOTE_MENU, 0, NOTE_MENU };
Uint32 note_delay[3];
char note[3][50];

SDL_Surface *surface, *image, *scaled, *scaled_rot[4];

fixed fit_outside, fit_inside;

fixed scale = int_to_fixed(1), scale_inv = int_to_fixed(1); // multiplicative inverse
int rotation = 0; // 0 is 0°, 1 is 90°, 2 is 180°, 3 is 270°

SDL_Rect pan_rect, drawn_rect, scaled_rect, rotated_rect, surface_rect;

int main( int argc, char *argv[] ) {
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) == -1) {
		printf("%s:%d\n", __FILE__, __LINE__);
		return -1;
	}

	#ifdef ARCADE_MINI
	ScreenSurface = SDL_SetVideoMode(480, 272, 16, SDL_HWSURFACE);
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 432, 272, 16, 0,0,0,0);
	#else
	ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0,0,0,0);
	#endif
	if (!surface) {
		printf("%s:%d\n", __FILE__, __LINE__);
		return -1;
	}
	
#ifdef TARGET_GP2X
	SDL_ShowCursor(0);
#endif /* TARGET_GP2X */
	
	scaled_rot[0] = SDL_CreateRGBSurfaceFrom(malloc(surface->w * surface->h * 2), surface->w, surface->h, 16, surface->w * 2, 0, 0, 0, 0);
	scaled_rot[1] = SDL_CreateRGBSurfaceFrom(malloc(surface->h * surface->w * 2), surface->h, surface->w, 16, surface->h * 2, 0, 0, 0, 0);
	scaled_rot[2] = SDL_CreateRGBSurfaceFrom(malloc(surface->w * surface->h * 2), surface->w, surface->w, 16, surface->w * 2, 0, 0, 0, 0);
	scaled_rot[3] = scaled_rot[1];
	
	init_input();
	
	// parse parameter (file or directory)
	if (argc > 1) {
		struct stat temp;
		stat(argv[1], &temp);
		
		if (S_ISREG(temp.st_mode) && maybe_image(argv[1])) {
			char *path_temp = strdup(argv[1]);
			path = strdup(dirname(path_temp));
			if (strcmp(path, "/") != 0)
				strcat(path, "/");
			free(path_temp);
			
			populate_files(path, true);
			
			path_temp = strdup(argv[1]);
			char *filename = basename(path_temp);
			
			for (file_index = file_min; file_index < file_max; file_index++)
				if (strcmp(filename, files[file_index]) == 0)
					break;
			assert(file_index < file_max);
			
			free(path_temp);
		} else if (S_ISDIR(temp.st_mode)) {
			bool slash = argv[1][strlen(argv[1]) - 1] != '/';
			
			path = malloc(strlen(argv[1]) + 3 + 1);
			sprintf(path, "%s%s", argv[1], slash ? "/" : "");
			
			populate_files(path, true);
			
			toggle_file_selector();
		}
	}
	
	if (path == NULL) {
#ifdef TARGET_GP2X
		path = malloc(10); // arbitrary initial value, just don't overflow it yet
		strcpy(path, "/mnt/sd/");
#elif defined(TARGET_UNIX) || defined(TARGET_GCW_ZERO)
		{
			int path_len = strlen(getenv("HOME"));
			
			path = malloc(path_len + 1);
			strcpy(path, getenv("HOME"));
			// If the path does not end with a slash, add one. The file chooser
			// requires this.
			if (path_len == 0 || (path_len > 0 && path[path_len - 1] != '/')) {
				path = realloc(path, path_len + 2);
				strcat(path, "/");
			}
		}
#endif
		
		populate_files(path, true);
		
		toggle_file_selector();
	}
	
	image_modify_mutex = SDL_CreateMutex();
	files_modify_mutex = SDL_CreateMutex();
	SDL_Thread *load_images_thread = SDL_CreateThread(load_images, NULL);
	
	SDL_LockMutex(image_modify_mutex);
	
	load_prefs();
	
	while (!quit) {
		delay = SDL_GetTicks() + 20;
		
		poll_input();
		
		// handle input
		if (file_selector) {
			file_selector_input();
		} else if (menu) {
			menu_input();
		} else { // viewing input
			switch (last_key) {
#if defined(TARGET_GP2X) || defined(TARGET_UNIX)
				case SDLK_ESCAPE:
					quit = true;
					break;
#endif
					
				case GP2X_KB_SELECT:
					toggle_file_selector();
					need_redraw = true;
					break;
					
				case GP2X_KB_START:
					toggle_menu();
					need_redraw = true;
					break;
					
				case GP2X_KB_Y:
				case GP2X_KB_X:
				{
					fixed ratio = last_key == GP2X_KB_Y ? int_div_int_to_fixed(5, 4) : int_div_int_to_fixed(4, 5);
					set_scale(fixed_mul(scale, ratio));
					need_redraw = true;
					break;
				}
				
				case GP2X_KB_A:
				case GP2X_KB_B:
					set_rotation(rotation + (last_key == GP2X_KB_A ? 3 : 1));
					set_scale(auto_fit ? (fit_pref == 1 ? fit_outside : fit_inside) : scale);
					need_redraw = true;
					break;
					
				case GP2X_KB_L:
				case GP2X_KB_R:
					file_index = wrap_file_index(file_index + (last_key == GP2X_KB_L ? -1 : 1), true);
					break;
					
				default:
					break;
			}
			
			const int scroll = 10;
			
			if (input[0] || input[1] || input[2] || input[3]) {
				pan_rect.x += scroll * (input[(1 + rotation) % 4] - input[(3 + rotation) % 4]);
				pan_rect.y += scroll * (input[(2 + rotation) % 4] - input[(0 + rotation) % 4]);
				
				need_redraw = true;
			}
			if (input[4])
				need_redraw = true;
		}
		last_key = 0;
		
		// hide notes on timeout
		for (unsigned int i = 0; i < countof(note); i++)
			if (note_flags[i] & NOTE_DELAY && note_delay[i] < SDL_GetTicks() && note_delay[i]) {
				note_flags[i] &= ~NOTE_DELAY;
				note_delay[i] = 0;
				
				need_redraw = true;
			}
		
		if (need_redraw) {
			need_redraw = false;
			
			if (image)
				redraw();
			else
				SDL_FillRect(surface, NULL, 0);
			
			if SDL_MUSTLOCK(surface) SDL_LockSurface(surface);
			
			// display notes
			for (unsigned int i = 0; i < countof(note_delay); i++) {
				if (note_flags[i] & (NOTE_SHOW | NOTE_DELAY | (menu ? NOTE_MENU : 0)) || note_delay[i]) {
					int x = i & 1 ? 2 : surface->w - strlen(shorten(note[i], 39)) * 8 - 2,
						y = i > 0 ? 240 - 8 - 2 : 2;
					outline8x8(surface, x, y, shorten(note[i], 39), 0xffff, 0x0000);
					
					if (note_flags[i] & NOTE_DELAY && note_delay[i] == 0)
						note_delay[i] = SDL_GetTicks() + 1000;
				}
			}
			
			if SDL_MUSTLOCK(surface) SDL_UnlockSurface(surface);
			
			if (file_selector) {
				draw_file_selector();
			} else if (menu) {
				draw_menu();
			}
			
			if SDL_MUSTLOCK(surface) SDL_LockSurface(surface);
			
			if (input[4]) {
				outline8x8(surface, 4, 240 - 10, "Open2xIV v0.10", 0xffff, 0x0000);
				
				const char buffer[] = "!sseldniM yb dedoc"; // sneaky... not really
				static int j = 0; j++;
				for (unsigned int i = 0; i < strlen(buffer); i++) {
					char8x8(surface, surface->w - i * 8 - 12 - 1, 240 - 10,     buffer[i], 0xffff);
					char8x8(surface, surface->w - i * 8 - 12,     240 - 10 - 1, buffer[i], 0xffff);
					char8x8(surface, surface->w - i * 8 - 12 + 1, 240 - 10,     buffer[i], 0xffff);
					char8x8(surface, surface->w - i * 8 - 12,     240 - 10 + 1, buffer[i], 0xffff);
					char8x8(surface, surface->w - i * 8 - 12,     240 - 10,     buffer[i], (i + j) % 3 == 2 ? 0xd000 : ((i + j) % 3 == 1 ? 0x0340 : 0x001a));
				}
				
				delay += 100;
				need_redraw = true;
			}
			
			if SDL_MUSTLOCK(surface) SDL_UnlockSurface(surface);
			
			//static int j = 0; char buffer[8]; sprintf(buffer, "%d", j++); text8x8(surface, 0, 0, buffer, 0xffff); // frame counter
			
			//SDL_Flip(surface);
			SDL_SoftStretch(surface, NULL, ScreenSurface, NULL);
			SDL_Flip(ScreenSurface);
		}
		
		SDL_UnlockMutex(image_modify_mutex);
		
		delay -= SDL_GetTicks();
		if (delay > 0)
			SDL_Delay(delay);
		
		SDL_LockMutex(image_modify_mutex);
		
		if (image_update) {
			if (image_loaded) {
				sprintf(note[0], "%s %d/%d", files[file_index], file_index - file_min + 1, file_max - file_min);
				note_flags[0] |= NOTE_DELAY;
				note_delay[0] = 0;
				
				note_flags[1] &= ~NOTE_SHOW;
			}
			
			if (image_loading) {
				strcpy(note[1], "Loading...");
				note_flags[1] |= NOTE_SHOW;
			}
			
			if (image_loaded && image == NULL) {
				note_flags[0] |= NOTE_SHOW;
				
				strcpy(note[1], "Nothing to display!");
				note_flags[1] |= NOTE_SHOW;
				
				strcpy(note[2], "");
			} else {
				note_flags[0] &= ~NOTE_SHOW;
			}
			
			if (image_loaded && image != NULL) {
				set_rotation(auto_rotate && image->h > image->w ? 3 : 0);
				set_scale(auto_fit ? (fit_pref == 1 ? fit_outside : fit_inside) : scale);
			}
			
			image_update = false;
			need_redraw = true;
		}
		
	}
	
	save_prefs();
	
	if (file_selector)
		SDL_UnlockMutex(files_modify_mutex);
	
	SDL_UnlockMutex(image_modify_mutex);
	
	thread_die = true;
	SDL_WaitThread(load_images_thread, NULL);
	
	SDL_DestroyMutex(files_modify_mutex);
	SDL_DestroyMutex(image_modify_mutex);
	
	free(scaled_rot[0]->pixels); // yeah, this is wrong, but I'm lazy
	SDL_FreeSurface(scaled_rot[0]);
	free(scaled_rot[1]->pixels);
	SDL_FreeSurface(scaled_rot[1]);
	free(scaled_rot[2]->pixels);
	SDL_FreeSurface(scaled_rot[2]);
	SDL_FreeSurface(surface);
	
	SDL_Quit();
	
	return 0;
}

void set_rotation( int new_rotation ) {
	if (!image_loaded)
		return;
	
	rotation = new_rotation % 4;
	scaled = scaled_rot[rotation];
	
	calculate_auto_fit();
}

#define SDL_LOCKIFMUST(s) (SDL_MUSTLOCK(s) ? SDL_LockSurface(s) : 0)
#define SDL_UNLOCKIFMUST(s) { if(SDL_MUSTLOCK(s)) SDL_UnlockSurface(s); }

int invert_surface_vertical(SDL_Surface *surface)
{
    Uint8 *t;
    register Uint8 *a, *b;
    Uint8 *last;
    register Uint16 pitch;
    
    if( SDL_LOCKIFMUST(surface) < 0 )
        return -2;

    /* do nothing unless at least two lines */
    if(surface->h < 2) {
        SDL_UNLOCKIFMUST(surface);
        return 0;
    }

    /* get a place to store a line */
    pitch = surface->pitch;
    t = (Uint8*)malloc(pitch);

    if(t == NULL) {
        SDL_UNLOCKIFMUST(surface);
        return -2;
    }

    /* get first line; it's about to be trampled */
    memcpy(t,surface->pixels,pitch);

    /* now, shuffle the rest so it's almost correct */
    a = (Uint8*)surface->pixels;
    last = a + pitch * (surface->h - 1);
    b = last;

    while(a < b) {
        memcpy(a,b,pitch);
        a += pitch;
        memcpy(b,a,pitch);
        b -= pitch;
    }

    /* in this shuffled state, the bottom slice is too far down */
    memmove( b, b+pitch, last-b );

    /* now we can put back that first row--in the last place */
    memcpy(last,t,pitch);

    /* everything is in the right place; close up. */
    free(t);
    SDL_UNLOCKIFMUST(surface);

    return 0;
}


void rotate( void ) 
{
	const int Bpp = 2;
	
	Uint8 *src = scaled->pixels,
	      *dst = scaled_rot[0]->pixels;
	
	if (rotation == 0) {
		return;
	} else if (rotation == 2) {
		const int src_w = surface->w, src_h = surface->h;
		const int dst_pitch = surface->w * Bpp;
		dst += src_h * dst_pitch;
		for (int y = src_h; y; y--) 
		{
			// dst -= dst_extra;
			for (int x = src_w; x; x--) {
				dst -= Bpp;
				*(Uint16 *)dst = *(Uint16 *)src;
				src += Bpp;
			}
		}
		return;
	} else {
		const int src_w = surface->h, src_h = surface->w,
		          dst_w = surface->w, dst_h = surface->h;
		
		const int dst_pitch = surface->w * Bpp;
		
		switch (rotation)
		{
			case 1:
				dst += dst_w * Bpp;
				for (int y = src_h; y; y--) {
					dst -= Bpp;
					for (int x = src_w; x; x--) {
						*(Uint16 *)dst = *(Uint16 *)src;
						src += Bpp;
						dst += dst_pitch;
					}
					// src += src_extra;
					dst -= dst_h * dst_pitch;
				}
				return;
			case 3:
				for (int y = src_h; y; y--) {
					dst += dst_h * dst_pitch;
					for (int x = src_w; x; x--) {
						dst -= dst_pitch;
						*(Uint16 *)dst = *(Uint16 *)src;
						src += Bpp;
					}
					// src += src_extra;
					dst += Bpp;
				}
				return;
			default:
				assert(false);
				return;
		}
	}
}

void calculate_auto_fit( void ) {
	fit_outside = int_to_fixed(scaled->w) / image->w;
	fit_inside  = int_to_fixed(scaled->h) / image->h;
	if (fit_outside < fit_inside) {
		fixed temp = fit_outside;
		fit_outside = fit_inside;
		fit_inside = temp;
	}
}

void set_scale( fixed new_scale ) {
	if (!image_loaded)
		return;
	
	new_scale = min(max(new_scale, int_div_int_to_fixed(1, 100)), int_to_fixed(4));
	
	// prepare for recentering
	int center_x = 0, center_y = 0;
	if (scaled_rect.w != 0) {
		center_x = fixed_mul((pan_rect.x + scaled_rect.w / 2), fixed_div(new_scale, scale));
		center_y = fixed_mul((pan_rect.y + scaled_rect.h / 2), fixed_div(new_scale, scale));
	}
	
	if (scale != new_scale) {
		if (scale < (int_div_int_to_fixed(1, 10)))
			sprintf(note[2], "%ix%i %.1f%%", image->w, image->h, fixed_to_float(new_scale * 100));
		else
			sprintf(note[2], "%ix%i %.0f%%", image->w, image->h, fixed_to_float(new_scale * 100));
		note_flags[2] |= NOTE_DELAY;
		note_delay[2] = 0;
	}
	
	scale = new_scale;
	scale_inv = int_div_fixed_to_fixed(1, scale);
	
	// panning maximums (dimensions of scaled image)
	pan_rect.w = fixed_mul(image->w, scale);
	pan_rect.h = fixed_mul(image->h, scale);
	
	// for blitting from scaled surface, which is max of 320x240
	scaled_rect.w = min(pan_rect.w, scaled->w);
	scaled_rect.h = min(pan_rect.h, scaled->h);
	
	// recenter the panning when image larger than 320x240
	pan_rect.x = max(center_x - scaled_rect.w / 2, 0);
	pan_rect.y = max(center_y - scaled_rect.h / 2, 0);
	
	drawn_rect.w = drawn_rect.h = 0;
	
	// printf("%dx%d\t%3.1f%%\t%d°\t%dx%d\n", image->w, image->h, fixed_to_float(scale * 100), rotation * 90, pan_rect.w, pan_rect.h);
}

void redraw( void ) 
{
	void (*fixed_scale)( SDL_Surface *src, SDL_Surface *dst, const fixed src_x, fixed src_y, const SDL_Rect *dst_area, const fixed scale, const fixed scale_inv ) = &fixed_scale_16;
	
	switch (image->format->BytesPerPixel) {
		case 4:
			fixed_scale = &fixed_scale_32;
			break;
		case 3:
			fixed_scale = &fixed_scale_24;
			break;
		case 2:
			fixed_scale = &fixed_scale_16;
			break;
		case 1:
			fixed_scale = &fixed_scale_8;
			break;
		default:
			assert(0);
			break;
	}
	
	// keep the panning within the image
	pan_rect.x = max(min(pan_rect.x, pan_rect.w - scaled_rect.w), 0);
	pan_rect.y = max(min(pan_rect.y, pan_rect.h - scaled_rect.h), 0);
	
	if SDL_MUSTLOCK(image) SDL_LockSurface(image);
	if SDL_MUSTLOCK(scaled) SDL_LockSurface(scaled);
		
	fixed_scale(image, scaled, pan_rect.x * scale_inv, pan_rect.y * scale_inv, &scaled_rect, scale, scale_inv);
	
	if SDL_MUSTLOCK(image) SDL_UnlockSurface(image);
	if SDL_MUSTLOCK(scaled) SDL_UnlockSurface(scaled);
	
	drawn_rect = pan_rect;
	
	// swap width and height for 90 and 270
	if (rotation == 1 || rotation == 3) {
		rotated_rect.w = scaled_rect.h;
		rotated_rect.h = scaled_rect.w;
	} else {
		rotated_rect.w = scaled_rect.w;
		rotated_rect.h = scaled_rect.h;
	}
	
	// upper left corner of scaled image
	rotated_rect.x = (rotation == 1 || rotation == 2) ? scaled_rot[0]->w - rotated_rect.w : 0;
	rotated_rect.y = (rotation == 3 || rotation == 2) ? scaled_rot[0]->h - rotated_rect.h : 0;
	
	// center the scaled image when smaller than 320x240
	surface_rect.x = (scaled_rot[0]->w - rotated_rect.w) / 2;
	surface_rect.y = (scaled_rot[0]->h - rotated_rect.h) / 2;
	
	rotate();
	
	SDL_FillRect(surface, NULL, 0);
	SDL_BlitSurface(scaled_rot[0], &rotated_rect, surface, &surface_rect);
}

// kate: tab-width 4;
