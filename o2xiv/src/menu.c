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
#include "SDL.h"

#include "files.h"
#include "fixed.h"
#include "font.h"
#include "input.h"
#include "main.h"
#include "menu.h"
#include "prefs.h"

bool menu = false;

unsigned int menu_select = 2;

void toggle_menu( void ) {
	menu = !menu;
}

void menu_input( void ) {
	switch (last_key) {
#if defined(TARGET_GP2X) || defined(TARGET_UNIX)
		case SDLK_ESCAPE:
			quit = true;
			break;
#endif
			
		case GP2X_KB_SELECT:
			toggle_file_selector();
			
		case GP2X_KB_START:
		case GP2X_KB_Y:
		case GP2X_KB_X:
			toggle_menu();
			need_redraw = true;
			break;
			
		case GP2X_KB_A:
		case GP2X_KB_B:
			switch (menu_select) {
				case 2:
					set_scale(int_to_fixed(1));
					break;
				case 3:
					set_scale(fit_inside);
					fit_pref = 0;
					break;
				case 4:
					set_scale(fit_outside);
					fit_pref = 1;
					break;
				
				case 6:
					auto_fit = !auto_fit;
					break;
				case 7:
					auto_rotate = !auto_rotate;
					break;
				
				case 9:
					toggle_file_selector();
					break;
				case 10:
					show_hidden = !show_hidden;
					toggle_file_visibility();
					break;
				case 11:
					toggle_menu();
					break;
				case 12:
					quit = true;
					break;
			}
			need_redraw = true;
			break;
			
		default:
			break;
	}
	
	if (input[0]) {
		switch (--menu_select) {
			case 1:
				menu_select = 12;
				break;
			case 5:
			case 8:
				menu_select--;
				break;
		}
		delay += 150;
		need_redraw = true;
	}
	if (input[2]) {
		switch (++menu_select) {
			case 5:
			case 8:
				menu_select++;
				break;
			case 13:
				menu_select = 2;
				break;
		}
		delay += 150;
		need_redraw = true;
	}
}

void draw_menu( void ) {
	static char items[][14] = {
		"__ Open2xIV __",
		"",
		"Original size",
		"Fit inside",
		"Fit outside",
		"",
		"\177 Auto-fit",
		"\177 Auto-rotate",
		"",
		"Browse...",
		"\177 Show hidden",
		"Return",
		"Quit"
	};
	items[6][0] = auto_fit ? '\014' : '\015';
	items[7][0] = auto_rotate ? '\014' : '\015';
	items[10][0] = show_hidden ? '\014' : '\015';
	
	SDL_Rect temp_rect = { 8, 15, 8 + 14 * 8 + 8, 10 + 10 * 10 + 3 * 6 + 10 };
	SDL_FillRect(surface, &temp_rect, 0x0000);
	
	if SDL_MUSTLOCK(surface) SDL_LockSurface(surface);
	
	int y = 25;
	for (unsigned int i = 0; i < sizeof(items) / sizeof(*items); i++) {
		if (items[i][0]) {
			if (i == 0)
				text8x8(surface, 16, y, items[i], HEADER);
			else if (i == menu_select)
				text8x8(surface, 16, y, items[i], SELECTED);
			else
				text8x8(surface, 16, y, items[i], NORMAL);
			y += 10;
		} else {
			y += 6;
		}
	}
	
	if SDL_MUSTLOCK(surface) SDL_UnlockSurface(surface);
}

// kate: tab-width 4;
