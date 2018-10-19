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
#ifndef FILES_H
#define FILES_H

#include <stdbool.h>

extern bool file_selector;
extern bool show_hidden;

extern char *path;

extern unsigned int file_index;
extern char *files[1000];
extern unsigned int file_min, file_max;

void toggle_file_selector( void );
void toggle_file_visibility( void );

void file_selector_input( void );
void draw_file_selector( void );

int wrap_file_index( int index, bool files_only );

bool maybe_image( const char *filename );

void populate_files( const char *path, bool only_images );

char *shorten( char *text, unsigned int len );

#endif

// kate: tab-width 4;
