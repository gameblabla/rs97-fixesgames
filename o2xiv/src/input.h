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
#ifndef INPUT_H
#define INPUT_H

#include "SDL.h"

#ifdef TARGET_GP2X
#ifndef GP2X_VK_UP

#define GP2X_VK_UP              0
#define GP2X_VK_DOWN            4
#define GP2X_VK_LEFT            2
#define GP2X_VK_RIGHT           6
#define GP2X_VK_UP_LEFT         1
#define GP2X_VK_UP_RIGHT        7
#define GP2X_VK_DOWN_LEFT       3
#define GP2X_VK_DOWN_RIGHT      5
#define GP2X_VK_CLICK           18
#define GP2X_VK_FA              12
#define GP2X_VK_FB              13
#define GP2X_VK_FX              14
#define GP2X_VK_FY              15
#define GP2X_VK_FL              10
#define GP2X_VK_FR              11
#define GP2X_VK_START           8
#define GP2X_VK_SELECT          9
#define GP2X_VK_VOL_UP          16
#define GP2X_VK_VOL_DOWN        17

#endif
#endif

#if defined(TARGET_GP2X)

#define GP2X_KB_UP     SDLK_UP
#define GP2X_KB_DOWN   SDLK_DOWN
#define GP2X_KB_LEFT   SDLK_LEFT
#define GP2X_KB_RIGHT  SDLK_RIGHT
#define GP2X_KB_L      SDLK_INSERT
#define GP2X_KB_R      SDLK_PAGEUP
#define GP2X_KB_Y      SDLK_HOME
#define GP2X_KB_X      SDLK_END
#define GP2X_KB_A      SDLK_DELETE
#define GP2X_KB_B      SDLK_PAGEDOWN
#define GP2X_KB_SELECT SDLK_BACKSLASH
#define GP2X_KB_START  SDLK_RETURN

#elif defined(TARGET_GCW_ZERO)

#define GP2X_KB_UP     SDLK_UP
#define GP2X_KB_DOWN   SDLK_DOWN
#define GP2X_KB_LEFT   SDLK_LEFT
#define GP2X_KB_RIGHT  SDLK_RIGHT
#define GP2X_KB_L      SDLK_TAB
#define GP2X_KB_R      SDLK_BACKSPACE
#define GP2X_KB_Y      SDLK_SPACE
#define GP2X_KB_X      SDLK_LSHIFT
#define GP2X_KB_A      SDLK_LCTRL
#define GP2X_KB_B      SDLK_LALT
#define GP2X_KB_SELECT SDLK_ESCAPE
#define GP2X_KB_START  /*SDLK_RETURN*/51

#endif

extern SDLKey last_key;
extern Uint8 *key_down;

extern Uint8 input[5];

void init_input( void );
void poll_input( void );

#endif

// kate: tab-width 4;
