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
#include "thread.h"

#include <assert.h>
#include <dirent.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>

#ifndef DT_DIR
#define DT_DIR 4
#define DT_REG 8
#endif

bool file_selector = false;
bool show_hidden = false;

char *path = NULL;

unsigned int file_index = 0, temp_file_index = 0;
char *files[1000] = { "../", NULL };
unsigned int file_min = 0, file_max = 0;

void toggle_file_selector( void ) {
	file_selector = !file_selector;
	
	if (file_selector) {
		temp_file_index = file_index;
		SDL_LockMutex(files_modify_mutex);
	} else {
		SDL_UnlockMutex(files_modify_mutex);
	}
}

void toggle_file_visibility( void ) {
	// repopulate the directory if file visibility was changed
	populate_files(path, true);
}

void file_selector_input( void ) {
	switch (last_key) {
#if defined(TARGET_GP2X) || defined(TARGET_UNIX)
		case SDLK_ESCAPE:
			quit = true;
			break;
#endif
			
		case GP2X_KB_START:
			toggle_menu();
			
		case GP2X_KB_SELECT:
		case GP2X_KB_X:
			toggle_file_selector();
			need_redraw = true;
			break;
			
		case GP2X_KB_A:
		case GP2X_KB_B:
			// selected a directory or file?
			if (temp_file_index < file_min) {
				// ..?
				if (temp_file_index == 0) {
					char *new_path = dirname(path);
					strcpy(path, new_path);
					if (strcmp(path, "/") != 0)
						strcat(path, "/");
				} else {
					path = realloc(path, strlen(path) + strlen(files[temp_file_index]) + 1);
					strcat(path, files[temp_file_index]);
				}
				
				populate_files(path, true);
				temp_file_index = 0;
				
				force_image_load = true;
			} else {
				file_index = temp_file_index;
				toggle_file_selector();
			}
			
			need_redraw = true;
			break;
			
		case GP2X_KB_Y:
			temp_file_index = 0;
			delay += 150;
			need_redraw = true;
			break;
			
		case GP2X_KB_L:
		case GP2X_KB_R:
			temp_file_index = wrap_file_index(temp_file_index + (last_key == GP2X_KB_L ? -10 : 10), false);
			delay += 150;
			need_redraw = true;
			break;
			
		default:
			break;
	}
	
	if (input[0]) {
		temp_file_index = wrap_file_index(temp_file_index - 1, false);
		delay += 150;
		need_redraw = true;
	}
	if (input[2]) {
		temp_file_index = wrap_file_index(temp_file_index + 1, false);
		delay += 150;
		need_redraw = true;
	}
}

void draw_file_selector( void ) {
	SDL_FillRect(surface, NULL, BACKGROUND);
	
	if SDL_MUSTLOCK(surface) SDL_LockSurface(surface);
	
	text8x8(surface, 4, 10, shorten(path, 39), HEADER);

	int y = 25;
	for (int i = -10; i <= 10; i++) {
		if (temp_file_index + i < file_max) {
			if (i == 0) {
				text8x8(surface, 4, y, ">", SELECTED);
				text8x8(surface, 12, y, shorten(files[temp_file_index + i], 38), SELECTED);
			} else
				text8x8(surface, 12, y, shorten(files[temp_file_index + i], 38), NORMAL);
		}
		y += 10;
	}
	
#if 0
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 16; x++) {
			char8x8(surface, 160 + x * 10, y * 10, y * 16 + x, 0xffff);
		}
	}
#endif
	
	if SDL_MUSTLOCK(surface) SDL_UnlockSurface(surface);
}


int wrap_file_index( signed int index, bool only_files ) {
	int temp_min = only_files ? file_min : 0;
	
	if (file_max - temp_min == 0)
		return 0;
	
	while (index < temp_min)
		index += file_max - temp_min;
	while (index >= (signed)file_max)
		index -= file_max - temp_min;
	return index;
}


bool maybe_image( const char *filename ) {
	int temp = strlen(filename);
	return (temp >= 5 && (strcasecmp(&filename[temp - 5], ".jpeg") == 0 ||
	                      strcasecmp(&filename[temp - 5], ".tiff") == 0)) ||
	       (temp >= 4 && (strcasecmp(&filename[temp - 4], ".jpg") == 0 ||
	                      strcasecmp(&filename[temp - 4], ".jpe") == 0 ||
	                      strcasecmp(&filename[temp - 4], ".png") == 0 ||
	                      strcasecmp(&filename[temp - 4], ".tif") == 0 ||
	                      strcasecmp(&filename[temp - 4], ".tga") == 0 ||
	                      strcasecmp(&filename[temp - 4], ".gif") == 0 ||
	                      strcasecmp(&filename[temp - 4], ".bmp") == 0 ||
	                      strcasecmp(&filename[temp - 4], ".pcx") == 0 ||
	                      strcasecmp(&filename[temp - 4], ".pnm") == 0 ||
	                      strcasecmp(&filename[temp - 4], ".xpm") == 0 ||
	                      strcasecmp(&filename[temp - 4], ".xcf") == 0 ||
	                      strcasecmp(&filename[temp - 4], ".lbm") == 0));
}

static int strcmpq( const void *s1, const void *s2 ) {
	return strcmp(*(char * const *)s1, *(char * const *)s2);
}

void populate_files( const char *path, bool only_images ) {
	DIR *dir = opendir(path);
	
	file_min = file_max = 1;
	
	if (dir) {
		struct dirent *entry;
		
		while ((entry = readdir(dir)) != NULL && file_min < countof(files)) {
			if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {

				if(show_hidden || (!show_hidden && entry->d_name[0] != '.')) {
					files[file_min] = realloc(files[file_min], strlen(entry->d_name) + 1 + 1);
					strcpy(files[file_min], entry->d_name);
					strcat(files[file_min], "/");
					file_min++;
				}
			}
		}
		
		file_max = file_min;
		
		rewinddir(dir);
		
		while ((entry = readdir(dir)) != NULL && file_max < countof(files)) {
			if (entry->d_type == DT_REG) {
				if (!only_images || maybe_image(entry->d_name)) {
					files[file_max] = realloc(files[file_max], strlen(entry->d_name) + 1);
					strcpy(files[file_max], entry->d_name);
					file_max++;
				}
			}
		}
		closedir(dir);
		
		file_index = (file_min == file_max) ? 0 : file_min;
		
		qsort(&files[1], file_min - 1, sizeof(char *), strcmpq);
		qsort(&files[file_min], file_max - file_min, sizeof(char *), strcmpq);
	}
}

char *shorten( char *text, unsigned int len ) {
	assert(len > 10 && len <= 40);
	
	static char buffer[40 + 1];
	if (strlen(text) > len) {
		sprintf(buffer, "%.7s...%s", text, text + strlen(text) - (len - 10));
		return buffer;
	}
	
	return text;
}

// kate: tab-width 4;
