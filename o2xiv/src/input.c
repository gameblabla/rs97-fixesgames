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
#include "input.h"
#include "main.h"

#include "SDL.h"

#include <stdbool.h>

SDLKey last_key;
Uint8 *key_down;

Uint8 input[5];

SDL_Joystick *joy;

void init_input( void ) {
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	key_down = SDL_GetKeyState(NULL);
	
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1) {
		printf("failed to initialize SDL joystick subsystem: %s\n", SDL_GetError());
		return;
	}
	
	if (SDL_NumJoysticks() > 0) {
		joy = SDL_JoystickOpen(0);
		
		printf("found joystick: %s\n", SDL_JoystickName(0));
	}
}

void poll_input( void ) {
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
			
			case SDL_KEYDOWN:
				last_key = ev.key.keysym.sym;
				break;
				
#ifdef TARGET_GP2X
			case SDL_JOYBUTTONDOWN:
				switch (ev.jbutton.button) {
					case GP2X_VK_FY:
						last_key = GP2X_KB_Y;
						break;
					case GP2X_VK_FX:
						last_key = GP2X_KB_X;
						break;
					case GP2X_VK_FA:
						last_key = GP2X_KB_A;
						break;
					case GP2X_VK_FB:
						last_key = GP2X_KB_B;
						break;
					case GP2X_VK_FL:
						last_key = GP2X_KB_L;
						break;
					case GP2X_VK_FR:
						last_key = GP2X_KB_R;
						break;
					case GP2X_VK_SELECT:
						last_key = GP2X_KB_SELECT;
						break;
					case GP2X_VK_START:
						last_key = GP2X_KB_START;
						break;
				}
				break;
#endif /* TARGET_GP2X */
				
			case SDL_QUIT:
				exit(0);
				break;
				
			default:
				break;
		}
	}
	
	input[0] = key_down[GP2X_KB_UP];
	input[1] = key_down[GP2X_KB_RIGHT];
	input[2] = key_down[GP2X_KB_DOWN];
	input[3] = key_down[GP2X_KB_LEFT];
	input[4] = input[1] && input[3];
	
#ifdef TARGET_GP2X
	input[0] |= SDL_JoystickGetButton(joy, GP2X_VK_UP)
	         || SDL_JoystickGetButton(joy, GP2X_VK_UP_LEFT)
	         || SDL_JoystickGetButton(joy, GP2X_VK_UP_RIGHT);
	input[1] |= SDL_JoystickGetButton(joy, GP2X_VK_RIGHT)
	         || SDL_JoystickGetButton(joy, GP2X_VK_UP_RIGHT)
	         || SDL_JoystickGetButton(joy, GP2X_VK_DOWN_RIGHT);
	input[2] |= SDL_JoystickGetButton(joy, GP2X_VK_DOWN)
	         || SDL_JoystickGetButton(joy, GP2X_VK_DOWN_LEFT)
	         || SDL_JoystickGetButton(joy, GP2X_VK_DOWN_RIGHT);
	input[3] |= SDL_JoystickGetButton(joy, GP2X_VK_LEFT)
	         || SDL_JoystickGetButton(joy, GP2X_VK_UP_LEFT)
	         || SDL_JoystickGetButton(joy, GP2X_VK_DOWN_LEFT);
	input[4] |= SDL_JoystickGetButton(joy, GP2X_VK_CLICK);
	
	if (SDL_JoystickGetButton(joy, GP2X_VK_SELECT) && SDL_JoystickGetButton(joy, GP2X_VK_START))
		quit = true;
#endif /* TARGET_GP2X */
}

// kate: tab-width 4;
