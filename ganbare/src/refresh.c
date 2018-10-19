#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include "SDL_rotozoom.h"
#include "refresh.h"

#ifdef SCALING
#include "get_resolution.h"

void RefreshScreen(SDL_Surface* tmp)
{
	SDL_Surface* doble;
	doble = zoomSurface(tmp, screen_scale.w_scale, screen_scale.h_scale, 0);
	SDL_BlitSurface(doble, NULL, real_screen, &screen_position);
	SDL_Flip(real_screen);
	SDL_FreeSurface(doble);
}
#else
extern SDL_Surface *ScreenSurface;
void RefreshScreen(SDL_Surface* tmp)
{
	if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
	int x, y;
	uint32_t *s = tmp->pixels;
	uint32_t *d = ScreenSurface->pixels;
	for(uint8_t y = 0; y < 240; y++, s += 160, d += 320) 
		memmove(d, s, 1280); // double-line fix by pingflood, 2018
	/*for(y=0; y<240; y++){
		for(x=0; x<160; x++){
			*d++ = *s++;
		}
		d+= 160;
	}*/
	if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
	SDL_Flip(ScreenSurface);
}

#endif
