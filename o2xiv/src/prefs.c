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
#include "prefs.h"

#include <stdbool.h>
#include <stdio.h>

bool auto_fit = true, auto_rotate = false;
Uint8 fit_pref = 0;

void load_prefs( void ) {
	FILE *f = fopen("o2xiv.pref", "rb");
	
	if (f) {
		fit_pref = fgetc(f);
		auto_fit = fgetc(f);
		auto_rotate = fgetc(f);
		
		fclose(f);
	}
}

void save_prefs( void ) {
	FILE *f = fopen("o2xiv.pref", "wb");
	
	if (f) {
		fputc(fit_pref, f);
		fputc(auto_fit, f);
		fputc(auto_rotate, f);
		
		fclose(f);
	}
}

// kate: tab-width 4;
