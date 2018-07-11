/*
    FreeBlocks -  A simple puzzle game, similar to Tetris Attack
    Copyright (C) 2012 Justin Jacobs

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

#include <SDL/SDL.h>
#include <stdlib.h>
#include <time.h>

#include "block.h"
#include "draw.h"
#include "game.h"
#include "menu.h"
#include "sys.h"

extern SDL_Surface* real_screen;

int main(int argc, char *argv[]) 
{
	uint32_t y, x;
    uint32_t *s;
    uint32_t *d;
    srand(time(NULL));

    if(!sysInit()) return 1;
    if(!sysLoadFiles()) return 1;

    menuInit();
    gameTitle();

    while(!quit) {
        startTimer = SDL_GetTicks();

        sysInput();
        gameLogic();
        drawEverything();

        // Update the screen

		s = (uint32_t*)screen->pixels;
		d = (uint32_t*)real_screen->pixels;
		for(y=0; y<240; y++)
		{
			for(x=0; x<160; x++)
			{
				*d++ = *s++;
			}
			d+= 160;
		}
		SDL_Flip(real_screen);
        
        // Limit the frame rate
        endTimer = SDL_GetTicks();
        deltaTimer = endTimer - startTimer;
        if(deltaTimer < (1000/FPS))
            SDL_Delay((1000/FPS)-deltaTimer);
    }
    blockCleanup();
    sysCleanup();
}
