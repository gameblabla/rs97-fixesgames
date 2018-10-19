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
#include "SDL_image.h"

#include "files.h"
#include "image.h"
#include "main.h"
#include "thread.h"

bool thread_die = false; // wake up, time to die!

bool image_loading = false, // is thread loading current image?
     image_loaded = false,  // is thread done loading? (if true and image==NULL then loading failed)
     image_update = false;  // is either image_loading or image_loaded set?
SDL_mutex *image_modify_mutex,
          *files_modify_mutex;

bool force_image_load = false;

static bool maybe_jpg( const char *filename ) {
	int temp = strlen(filename);
	return (temp >= 5 && (strcasecmp(&filename[temp - 5], ".jpeg") == 0)) ||
	       (temp >= 4 && (strcasecmp(&filename[temp - 4], ".jpg") == 0 ||
	                      strcasecmp(&filename[temp - 4], ".jpe") == 0));
}

int load_images( void *nothing ) {
	(void) nothing;
	
	unsigned int cur_index = 0, // which indexes are actually loaded
	             next_index = 0,
	             prev_index = 0;
	SDL_Surface *cur_image = NULL,
	            *next_image = NULL,
	            *prev_image = NULL;
	char *filename = NULL;
	
	while (!thread_die) {
		int delay = SDL_GetTicks() + 50;
		
		SDL_LockMutex(files_modify_mutex);
		
		// changed directory, so loaded images are invalid
		if (force_image_load) {
			cur_index = next_index = prev_index = 0;
			force_image_load = false;
		}
		
		unsigned int temp_next_index = wrap_file_index(file_index + 1, true), // which indexes *should* actually be loaded
		             temp_prev_index = wrap_file_index(file_index - 1, true);
		
		if (cur_index != file_index && file_index != 0) {
			image_loading = true;
			image_loaded = false;
			image_update = true;
			
			// cur is now next?
			if (cur_index == temp_next_index && cur_image) {
				SDL_FreeSurface(next_image);
				next_index = cur_index;
				next_image = cur_image;
				next_image->refcount++;
			}
			
			// cur is now prev?
			if (cur_index == temp_prev_index && cur_image) {
				SDL_FreeSurface(prev_image);
				prev_index = cur_index;
				prev_image = cur_image;
				prev_image->refcount++;
			}
			
			SDL_FreeSurface(cur_image);
			cur_image = NULL;
			
			// use next? else free it
			if (next_index == file_index && next_image) {
				cur_index = next_index;
				cur_image = next_image;
				cur_image->refcount++;
			} else if (next_index != temp_next_index) {
				SDL_FreeSurface(next_image);
				next_index = 0;
				next_image = NULL;
			}
			
			// use prev? else free it (if we didn't just get it)
			if (prev_index == file_index && prev_image && cur_image == NULL) {
				cur_index = prev_index;
				cur_image = prev_image;
				cur_image->refcount++;
			} else if (prev_index != temp_prev_index) {
				SDL_FreeSurface(prev_image);
				prev_index = 0;
				prev_image = NULL;
			}
			
			if (cur_image == NULL) {
				cur_index = file_index;
				filename = realloc(filename, strlen(path) + strlen(files[cur_index]) + 1);
				strcat(strcpy(filename, path), files[cur_index]);
				cur_image = maybe_jpg(files[cur_index]) ? jpeg_load(filename) : IMG_Load(filename);
			}
			
			SDL_LockMutex(image_modify_mutex);
			// now we're free to muck around with image
			
			SDL_FreeSurface(image);
			image = cur_image;
			if (image)
				image->refcount++;
			
			image_loading = false;
			image_loaded = true;
			image_update = true;
			
			SDL_UnlockMutex(image_modify_mutex);
			
			if (image)
				delay = SDL_GetTicks() + 500; // let the scaler breathe for a bit
			
		} else if (next_index != temp_next_index && temp_next_index != 0) {
			SDL_FreeSurface(next_image);
			
			next_index = temp_next_index;
			filename = realloc(filename, strlen(path) + strlen(files[next_index]) + 1);
			strcat(strcpy(filename, path), files[next_index]);
			next_image = maybe_jpg(files[next_index]) ? jpeg_load(filename) : IMG_Load(filename);
			
			if (next_image)
				delay = SDL_GetTicks() + 100;
			
		} else if (prev_index != temp_prev_index && temp_prev_index != 0 && temp_prev_index != temp_next_index) {
			SDL_FreeSurface(prev_image);
			
			prev_index = temp_prev_index;
			filename = realloc(filename, strlen(path) + strlen(files[prev_index]) + 1);
			strcat(strcpy(filename, path), files[prev_index]);
			prev_image = maybe_jpg(files[prev_index]) ? jpeg_load(filename) : IMG_Load(filename);
			
			if (prev_image)
				delay = SDL_GetTicks() + 100;
		}
		
		SDL_UnlockMutex(files_modify_mutex);
		
		delay -= SDL_GetTicks();
		if (delay > 0 && cur_index == file_index)
			SDL_Delay(delay);
	};
	
	SDL_FreeSurface(image);
	SDL_FreeSurface(cur_image);
	SDL_FreeSurface(next_image);
	SDL_FreeSurface(prev_image);
	
	free(filename);
	
	return 0;
}

// kate: tab-width 4;
