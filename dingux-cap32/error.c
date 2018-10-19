#include "psp_sdl.h"
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

show_error_and_exit(char* error_message){
	Uint32 subsystem_init;
	subsystem_init = SDL_WasInit(SDL_INIT_EVERYTHING);

	if(subsystem_init & SDL_INIT_VIDEO){
		// video is initialized
		TTF_Init();
		SDL_Surface* screen = SDL_SetVideoMode(PSP_SDL_SCREEN_WIDTH, /*PSP_SDL_SCREEN_HEIGHT*/480, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);

		TTF_Font* font = TTF_OpenFont("./assets/arial.ttf", 12);
		SDL_Color fg_color = {255,0,0};

		SDL_Surface* text_surface = TTF_RenderText_Blended(font, error_message, fg_color);

		SDL_FillRect(screen,NULL,SDL_MapRGB(screen->format,0x0,0x0,0x0));
		SDL_BlitSurface(text_surface, NULL, screen, NULL);
		SDL_Flip(screen);

		SDL_Delay(2500);
		SDL_FreeSurface(text_surface);
		TTF_CloseFont(font);
		TTF_Quit();
		SDL_Quit();
		exit(0);
	}
}
