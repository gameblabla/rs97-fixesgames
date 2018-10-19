#include "states.h"

#include <SDL.h>
#include "game.h"
#include "hiscore.h"
#include "main.h"
#include "scaler.h"
#include "title.h"
#include "video.h"

extern SDL_Surface *ScreenSurface;
State programStateActive = STATE_NONE;
State programStateNew = STATE_TITLE;

void checkState()
{
	if(programStateActive != programStateNew)
	{
		// Unload current state.
		switch(programStateActive)
		{
			case STATE_TITLE:
				titleUnload();
			break;
			case STATE_GAME:
				gameUnload();
			break;
			case STATE_HISCORE:
				hiscoreUnload();
			break;

			default:
			break;
		}
		// Load new state.
		switch(programStateNew)
		{
			case STATE_TITLE:
				titleLoad();
			break;
			case STATE_GAME:
				gameLoad();
			break;
			case STATE_HISCORE:
				hiscoreLoad();
			break;

			default:
			break;
		}

		programStateActive = programStateNew;
	}
}

void logic()
{
	checkState();

	switch(programStateActive)
	{
		case STATE_TITLE:
			titleLogic();
		break;
		case STATE_GAME:
			gameLogic();
		break;
		case STATE_HISCORE:
			hiscoreLogic();
		break;

		default:
		break;
	}
}

void draw()
{
	clearScreen();

	switch(programStateActive)
	{
		case STATE_TITLE:
			titleDraw();
		break;
		case STATE_GAME:
			gameDraw();
		break;
		case STATE_HISCORE:
			hiscoreDraw();
		break;

		default:
		break;
	}

	switch (scale)
	{
		case 1:
		break;
		case 2:
			upscale2((uint32_t *)screenScaled->pixels, (uint32_t *)screen->pixels);
		break;
	}

	//SDL_Flip(screenScaled);
	if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
	int x, y;
	uint32_t *s = screenScaled->pixels;
	uint32_t *d = ScreenSurface->pixels;
		for(uint8_t y2 = 0; y2 < 240; y2++, s += 160, d += 320) 
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
