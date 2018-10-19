/*
Spartak Chess based on stockfish engine.
Copyright (C) 2010 zear
Caanoo enums thanks to glezmen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef USE_SDL
#ifdef SDL_KEYS_CAANOO

#include <SDL/SDL.h>
#include "../game.h"

static const int CAANOO_JOYSTICK_MIN = -32768;
static const int CAANOO_JOYSTICK_MAX = 32767;
static const int JOY_DEADZONE = 2000;
static int joyHasMoved = 0;	/* If the axis was moved, wait for it to be placed to zero (or deadzone) position before registering the next move */

enum CaanooAxes {
	CAANOO_AXIS_X = 0,
	CAANOO_AXIS_Y = 1
};

enum CaanooButtons {
	CAANOO_BUTTON_A = 0,
	CAANOO_BUTTON_X = 1,
	CAANOO_BUTTON_B = 2,
	CAANOO_BUTTON_Y = 3,
	CAANOO_BUTTON_L = 4,
	CAANOO_BUTTON_R = 5,
	CAANOO_BUTTON_HOME = 6,
	CAANOO_BUTTON_1 = 8,
	CAANOO_BUTTON_2 = 9,
	CAANOO_BUTTON_JOY = 10
};


void ProcessEvent(eGame* game, const SDL_Event& e)
{
	switch(e.type)
	{
    case SDL_JOYAXISMOTION:
		switch(e.jaxis.axis)
		{
		case CAANOO_AXIS_X:
			if((joyHasMoved == 0) && (e.jaxis.value < -JOY_DEADZONE) && (e.jaxis.value > CAANOO_JOYSTICK_MIN))
			{
				game->Command('l');
				joyHasMoved = 1;
			}
			if((joyHasMoved == 0) && (e.jaxis.value > JOY_DEADZONE) && (e.jaxis.value < CAANOO_JOYSTICK_MAX))
			{
				game->Command('r');
				joyHasMoved = 1;
			}
			if((e.jaxis.value > -JOY_DEADZONE) && (e.jaxis.value < JOY_DEADZONE))
			{
				joyHasMoved = 0;
			}
			break;
		case CAANOO_AXIS_Y:
                        if((joyHasMoved == 0) && (e.jaxis.value < -JOY_DEADZONE) && (e.jaxis.value > CAANOO_JOYSTICK_MIN))
                        {
                                game->Command('u');
				joyHasMoved = 1;
                        }
                        if((joyHasMoved == 0) && (e.jaxis.value > JOY_DEADZONE) && (e.jaxis.value < CAANOO_JOYSTICK_MAX))
                        {
                                game->Command('d');
				joyHasMoved = 1;
                        }
                        if((e.jaxis.value > -JOY_DEADZONE) && (e.jaxis.value < JOY_DEADZONE))
                        {
                                joyHasMoved = 0;
                        }
			break;
		default:
			break;
		}
		break;
    case SDL_JOYBUTTONDOWN:
		switch(e.jbutton.button)
		{
		case CAANOO_BUTTON_B:            game->Command('a');             break;
		case CAANOO_BUTTON_X:            game->Command('b');             break;
		case CAANOO_BUTTON_L:            game->Command('n');             break;
		case CAANOO_BUTTON_R:            game->Command('g');             break;
		case CAANOO_BUTTON_HOME:         game->Command('f');             break;
		default: break;
		}
		break;
	default:
		break;
	}
}

#endif//SDL_KEYS_CAANOO
#endif//USE_SDL