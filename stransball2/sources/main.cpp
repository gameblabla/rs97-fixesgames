#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#else
#include <sys/time.h>
#include <time.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "SDL/SDL.h"
#include "SDL_mixer.h"
#include "sge.h"

#include "fonts.h"
#include "list.h"

#include "tiles.h"
#include "maps.h"
#include "transball.h"
#include "game.h"

#include "homedir.h"

/*						GLOBAL VARIABLES INITIALIZATION:							*/ 
SDL_Surface *ScreenSurface;
int SCREEN_X=320;
int SCREEN_Y=240;
int PIXEL_SIZE=1;
int COLOUR_DEPTH=32;

#ifdef _WIN32
//bool fullscreen=true;
bool fullscreen=true;
#else
bool fullscreen=false;
#endif

/* Redrawing constant: */ 
const int REDRAWING_PERIOD=18;

/* Frames per second counter: */ 
int frames_per_sec=0;
int frames_per_sec_tmp=0;
int init_time=0;


/* Surfaces: */ 
SDL_Surface *screen_sfc=0,*buffer_screen_sfc=0;

TRANSBALL *game=0;


#ifndef _WIN32
struct timeval init_tick_count_value;

void setupTickCount()
{
	gettimeofday(&init_tick_count_value, NULL);
}

long GetTickCount()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval diff;
	diff.tv_sec = now.tv_sec - init_tick_count_value.tv_sec;
	diff.tv_usec = now.tv_usec - init_tick_count_value.tv_usec;
	if (diff.tv_usec < 0)
	{
		diff.tv_sec--;
		diff.tv_usec+=1000000;
	}
	return diff.tv_sec*1000 + diff.tv_usec/1000;
}
#endif

void pause(unsigned int time)
{
	unsigned int initt=GetTickCount();

	while((GetTickCount()-initt)<time);
} /* pause */ 


SDL_Surface* initializeSDL(int moreflags)
{
	SDL_Surface *screen;

	int flags = SDL_HWSURFACE/* | SDL_TRIPLEBUF*/ | SDL_HWPALETTE | moreflags;
	//int flags = SDL_HWPALETTE|moreflags;
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0) return 0;
	atexit(SDL_Quit);
	SDL_WM_SetCaption("Super Transball 2 v1.5", 0);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_EnableUNICODE(1);

	//screen = SDL_SetVideoMode(SCREEN_X*PIXEL_SIZE, SCREEN_Y*PIXEL_SIZE, COLOUR_DEPTH, flags);
	ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
	screen = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_X*PIXEL_SIZE, SCREEN_Y*PIXEL_SIZE, COLOUR_DEPTH, 0,0,0,0);
	buffer_screen_sfc = SDL_CreateRGBSurface(0,SCREEN_X,SCREEN_Y,COLOUR_DEPTH,0,0,0,0);

	pause(200);

	if (Mix_OpenAudio(22050, AUDIO_S16, 2, 1024)) {
		return 0;
	} /* if */ 

	return screen;
} /* initializeSDL */ 


void finalizeSDL()
{
	SDL_FreeSurface(buffer_screen_sfc);

	Mix_CloseAudio();
	SDL_Quit();
} /* finalizeSDL */ 



#ifdef _WIN32
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int nCmdShow)
{
#else
int main(int argc, char** argv)
{
	setupTickCount();
#endif

	int time,act_time;
	SDL_Event event;
    bool quit = false;

	get_home();

	if (!load_configuration()) save_configuration();

	time=init_time=GetTickCount();
	screen_sfc = initializeSDL((fullscreen ? SDL_FULLSCREEN : 0));
	if (screen_sfc==0) return 0;

	if (!fonts_initialization()) return 0;

	while (!quit) {
		while( SDL_PollEvent( &event ) ) {
            switch( event.type ) {
                /* Keyboard event */
                case SDL_KEYDOWN:
					{
						SDLMod modifiers;

						modifiers=SDL_GetModState();

						if (event.key.keysym.sym==SDLK_F12) quit = true;

						
						if ((modifiers&KMOD_ALT)!=0) {
							if (event.key.keysym.sym==SDLK_RETURN) {
								/* Toogle FULLSCREEN mode: */ 
								finalizeSDL();
								if (game!=0) game->free_sounds();
								if (fullscreen) fullscreen=false;
										   else fullscreen=true;
								screen_sfc = initializeSDL((fullscreen ? SDL_FULLSCREEN : 0));
								if (screen_sfc==0) return 0;
								/* Reload: */ 
								if (game!=0) game->load_sounds();
								save_configuration();
							} /* if */ 

							if (event.key.keysym.sym==SDLK_1) {
								/* x1 windowed mode: */ 
								finalizeSDL();
								if (game!=0) game->free_sounds();
								PIXEL_SIZE=1;
								screen_sfc = initializeSDL((fullscreen ? SDL_FULLSCREEN : 0));
								if (screen_sfc==0) return 0;
								/* Reload: */ 
								if (game!=0) game->load_sounds();
								save_configuration();
							} /* if */ 

							if (event.key.keysym.sym==SDLK_2) {
								/* x2 mode: */ 
								finalizeSDL();
								if (game!=0) game->free_sounds();
								PIXEL_SIZE=2;
								screen_sfc = initializeSDL((fullscreen ? SDL_FULLSCREEN : 0));
								if (screen_sfc==0) return 0;
								/* Reload: */ 
								if (game!=0) game->load_sounds();
								save_configuration();
							} /* if */ 

							if (event.key.keysym.sym==SDLK_3) {
								/* x3 mode: */ 
								finalizeSDL();
								if (game!=0) game->free_sounds();
								PIXEL_SIZE=3;
								screen_sfc = initializeSDL((fullscreen ? SDL_FULLSCREEN : 0));
								if (screen_sfc==0) return 0;
								/* Reload: */ 
								if (game!=0) game->load_sounds();
								save_configuration();
							} /* if */ 

							if (event.key.keysym.sym==SDLK_4) {
								/* x4 mode: */ 
								finalizeSDL();
								if (game!=0) game->free_sounds();
								PIXEL_SIZE=4;
								screen_sfc = initializeSDL((fullscreen ? SDL_FULLSCREEN : 0));
								if (screen_sfc==0) return 0;
								/* Reload: */ 
								if (game!=0) game->load_sounds();
								save_configuration();
							} /* if */ 
						} /* if */ 

					}

                    break;

                /* SDL_QUIT event (window close) */
                case SDL_QUIT:
                    quit = true;
                    break;
            } /* switch */ 
        } /* while */ 

		act_time=GetTickCount();
		if (act_time-time>=REDRAWING_PERIOD)
		{

			frames_per_sec_tmp+=1;
			if ((act_time-init_time)>=1000) {
				frames_per_sec=frames_per_sec_tmp;
				frames_per_sec_tmp=0;
				init_time=act_time;
			} /* if */ 

			time+=REDRAWING_PERIOD;
			if ((act_time-time)>2*REDRAWING_PERIOD) time=act_time;
		
			if (PIXEL_SIZE==1) {
				if (!gamecycle(screen_sfc,SCREEN_X,SCREEN_Y)) quit=true;	
			} else {
				if (!gamecycle(buffer_screen_sfc,SCREEN_X,SCREEN_Y)) quit=true;	
				sge_transform(buffer_screen_sfc,screen_sfc,0,float(PIXEL_SIZE),float(PIXEL_SIZE),0,0,0,0,0);	
			} /* if */ 
			//SDL_Flip(screen_sfc);
			if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
			int x, y;
			uint32_t tmp;
			uint32_t *s = (uint32_t*)screen_sfc->pixels;
			uint16_t *d = (uint16_t*)ScreenSurface->pixels;
			SDL_SoftStretch(screen_sfc, NULL, ScreenSurface, NULL);
			/*for(y=0; y<240; y++){
				for(x=0; x<320; x++){
					tmp = *s++;
					*d++ = SDL_MapRGB(ScreenSurface->format, (tmp >> 16) & 0xff, (tmp >> 8) & 0xff, tmp & 0xff);
				}
				d+= 320;
			}*/
			if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
			SDL_Flip(ScreenSurface);
		}
		SDL_Delay(1);
	}

	fonts_termination();

	finalizeSDL();

	free_home();

	return 0;
}


