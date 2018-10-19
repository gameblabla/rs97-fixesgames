#include "video.h"

#include <SDL.h>

SDL_Surface *ScreenSurface;
SDL_Surface *screen;
Uint32 curTicks;
Uint32 lastTicks = 0;
int blinkTimer = 0;
int blinkTimerSlow = 0;

int initSDL()
{
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK))
	{
		return -1;
	}

	SDL_WM_SetCaption("Triple Trapled", NULL);
	SDL_ShowCursor(SDL_DISABLE);

	//screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, SCREEN_BPP, SDL_HWSURFACE | SDL_DOUBLEBUF);
	ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
	screen = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_W, SCREEN_H, SCREEN_BPP, 0, 0, 0, 0);

	if(screen == NULL)
	{
		return -1;
	}

	if(SDL_NumJoysticks() > 0)
	{
		SDL_JoystickOpen(0);
	}

	return 0;
}

void deinitSDL()
{
	if(SDL_NumJoysticks() > 0)
	{
		SDL_JoystickClose(0);
	}

	SDL_Quit();
}

int frameLimiter()
{
	int t;

#if defined(NO_FRAMELIMIT)
	return 0;
#endif

	curTicks = SDL_GetTicks();
	t = curTicks - lastTicks;

	if(t >= 1000/FPS)
	{
		lastTicks = curTicks;
		return 0;
	}

	SDL_Delay(1);
	return 1;
}

void blinkTimersTick()
{
	if(++blinkTimer > 20)
	{
		blinkTimer = 0;
	}
	if(++blinkTimerSlow > 40)
	{
		blinkTimerSlow = 0;
	}
}

void clearScreen()
{
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
}
