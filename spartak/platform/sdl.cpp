/*
Spartak Chess based on stockfish engine.
Copyright (C) 2010 djdron

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

#ifdef _CAANOO
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif

#include "../game.h"
#include "../ui/dialog.h"
#include "../io.h"

static bool sdl_inited = false;
static SDL_Surface* screen = NULL;
static SDL_Surface* offscreen = NULL;
static SDL_Surface* ScreenSurface=NULL;

#ifdef SDL_USE_JOYSTICK
static SDL_Joystick* joy = NULL;
#endif//SDL_USE_JOYSTICK

static bool Init()
{
#ifdef SDL_USE_JOYSTICK
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) < 0)
		return false;
	if(SDL_NumJoysticks() > 0)
		joy = SDL_JoystickOpen(0);
#else//SDL_USE_JOYSTICK
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		return false;
#endif//SDL_USE_JOYSTICK
	SDL_ShowCursor(SDL_DISABLE);
	SDL_WM_SetCaption("Spartak Chess (Stockfish)", NULL);
	sdl_inited = true;
	ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_SWSURFACE);
  screen = SDL_CreateRGBSurface(SDL_SWSURFACE,320,240,16,0,0,0,0);
	if(!screen)
		return false;
	SDL_ShowCursor(SDL_DISABLE);
	offscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16,
						screen->format->Rmask,
						screen->format->Gmask,
						screen->format->Bmask,
						screen->format->Amask);
	if(!offscreen)
		return false;
	return true;
}
static void Done()
{
#ifdef SDL_USE_JOYSTICK
	if(joy)
	    SDL_JoystickClose(joy);
#endif//SDL_USE_JOYSTICK
	if(offscreen)
		SDL_FreeSurface(offscreen);
	if(screen)
		SDL_FreeSurface(screen);
	if(sdl_inited)
		SDL_Quit();
}
inline word BGR565(byte r, byte g, byte b) { return (((r&~7) << 8)|((g&~3) << 3)|(b >> 3)); }
static void UpdateScreen(eGame* game)
{
	if(!game->Desktop().Update())
		return;
	SDL_LockSurface(offscreen);
	eRGBA* data = game->Desktop().Buffer();
	word* scr = (word*)offscreen->pixels;
	for(int y = 0; y < 240; ++y)
	{
		for(int x = 0; x < 320; ++x)
		{
			eRGBA c(*data++);
			*scr++ = BGR565(c.r, c.g, c.b);
		}
		scr += offscreen->pitch - 320*2;
	}
	SDL_UnlockSurface(offscreen);
	SDL_BlitSurface(offscreen, NULL, screen, NULL);
	//SDL_Flip(screen);
  SDL_SoftStretch(screen, NULL, ScreenSurface, NULL);
  SDL_Flip(ScreenSurface);
}

void ProcessEvent(eGame* game, const SDL_Event& e);

int main(int argc, char* argv[])
{
//	xLog::Open();
    if(!Init())
    {
		Done();
        return 1;
    }
	eGame* game = new eGame;
    bool quit = false;
    while(!quit)
    {
    	SDL_Event e;
    	while(SDL_PollEvent(&e))
    	{
    		switch(e.type)
    		{
    		case SDL_QUIT:
				quit = true;
				break;
    		case SDL_KEYDOWN:
    		case SDL_KEYUP:
#ifdef SDL_USE_JOYSTICK
			case SDL_JOYAXISMOTION:
    		case SDL_JOYBUTTONDOWN:
    		case SDL_JOYBUTTONUP:
#endif//SDL_USE_JOYSTICK
    			ProcessEvent(game, e);
    			break;
    		}
    	}
    	if(game->Update())
    	{
    		UpdateScreen(game);
    		SDL_Delay(15);
    	}
    	else
    		quit = true;
    }
    delete game;
    Done();
    return 0;
}

#endif//USE_SDL
