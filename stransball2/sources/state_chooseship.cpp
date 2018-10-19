#ifdef _WIN32
#include "windows.h"
#else
#include <stddef.h>
#include <sys/types.h>
#include <dirent.h>
#include "ctype.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL/SDL.h"
#include "SDL_mixer.h"
#include "SDL_image.h"

#include "fonts.h"
#include "list.h"
#include "auxiliar.h"

#include "tiles.h"
#include "maps.h"
#include "transball.h"

#include "encoder.h"

#define MAXLEVELS	64

extern int NLEVELS;
extern int SCREEN_X,SCREEN_Y;
extern int STATE,SUBSTATE,SUBSTATE2;
extern int level,timer;
extern int ship_type;
extern int fuelfactor[3];
extern int used_fuel,remaining_fuel;
extern int n_shots,n_hits,enemies_destroyed;
extern int previous_high;
extern TRANSBALL *game;
extern int frames_per_sec;
extern FILE *replayfile;
extern int replaynum;
extern int replay_source;
extern SDLKey THRUST_KEY,ANTITHRUST_KEY,LEFT_KEY,RIGHT_KEY;
extern SDLKey FIRE_KEY,ATRACTOR_KEY;
extern SDLKey PAUSE_KEY;
extern bool pause;
extern unsigned char old_keyboard[SDLK_LAST];
extern SDL_Surface *image,*image2;
extern char edit_text[80];
extern int edit_position;
extern List<char> files;
extern int act_file;
extern int first_file;
extern bool refind_files;
extern int demotimer;
extern bool demoon;
extern int tittle_alpha;
extern List<char> levelpacks;
extern int act_levelpack;
extern char levelpack[256];
extern char *levelnames[MAXLEVELS];
extern char *leveltext[MAXLEVELS];
extern char *levelcode[MAXLEVELS];
extern int initialfuel[MAXLEVELS];


bool state_chooseship_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard)
{
	if (SUBSTATE==0) {
		if (image!=0) SDL_FreeSurface(image);
		if (image2!=0) SDL_FreeSurface(image);
		image=IMG_Load("graphics/tittle.png");
		image2=IMG_Load("graphics/tiles.png");
		SDL_SetColorKey(image2,SDL_SRCCOLORKEY,SDL_MapRGB(image2->format,0,0,0));
	} /* if */ 


	SDL_BlitSurface(image,0,screen,0);
	surface_fader(screen,0.5F,0.5F,0.5F,-1,0);

	font_print_centered(sx/2,40,"CHOOSE YOUR SHIP:",screen);
	font_print(100,60,"SHADOW RUNNER:",screen);
	font_print(100,70," - HIGH SPEED AND RETRO THRUSTERS.",screen);
	font_print(100,80," - LOW POWERED WEAPONS.",screen);

	font_print(100,100,"V-PANTHER 2:",screen);
	font_print(100,110," - MODERATE SPEED.",screen);
	font_print(100,120," - MEDIUM POWERED WEAPONS.",screen);

	font_print(100,140,"X-TERMINATOR:",screen);
	font_print(100,150," - LOW SPEED.",screen);
	font_print(100,160," - HIGH POWERED WEAPONS.",screen);

	rectangle(screen,60,56+40*ship_type,32,32,SDL_MapRGB(screen->format,255,255,255));

	{
		SDL_Rect r,d;

		r.x=96; r.y=272;
		r.w=32; r.h=32;
		d.x=60; d.y=60;
		SDL_BlitSurface(image2,&r,screen,&d);
		r.x=32; r.y=240;
		r.w=32; r.h=32;
		d.x=60; d.y=100;
		SDL_BlitSurface(image2,&r,screen,&d);
		r.x=96; r.y=336;
		r.w=32; r.h=32;
		d.x=60; d.y=140;
		SDL_BlitSurface(image2,&r,screen,&d);
	}
	
	if (SUBSTATE<32) {
		surface_fader(screen,float(SUBSTATE)/32.0F,float(SUBSTATE)/32.0F,float(SUBSTATE)/32.0F,-1,0);
		SUBSTATE++;
	} /* if */ 
	if (SUBSTATE>32) {
		surface_fader(screen,float(64-SUBSTATE)/32.0F,float(64-SUBSTATE)/32.0F,float(64-SUBSTATE)/32.0F,-1,0);
		SUBSTATE++;
		if (SUBSTATE==64) {
			SDL_FreeSurface(image);
			image=0;
			SDL_FreeSurface(image2);
			image2=0;

			if (SUBSTATE2==0) {
				STATE=5;
				SUBSTATE=0;
			} /* if */ 
			if (SUBSTATE2==1) {
				STATE=1;
				SUBSTATE=0;
			} /* if */ 
		} /* if */ 
	} /* if */ 

	if (SUBSTATE==32) {
		if (keyboard[SDLK_LEFT] && !old_keyboard[SDLK_LEFT] && ship_type>0) ship_type--;
		if (keyboard[SDLK_RIGHT] && !old_keyboard[SDLK_RIGHT] && ship_type<2) ship_type++;
		if (keyboard[SDLK_UP] && !old_keyboard[SDLK_UP] && ship_type>0) ship_type--;
		if (keyboard[SDLK_DOWN] && !old_keyboard[SDLK_DOWN] && ship_type<2) ship_type++;
		if (keyboard[SDLK_LCTRL] && !old_keyboard[SDLK_LCTRL]) {
			SUBSTATE++;
			SUBSTATE2=0;
		} /* if */ 
		if (keyboard[SDLK_ESCAPE] && !old_keyboard[SDLK_ESCAPE]) {
			SUBSTATE++;
			SUBSTATE2=1;
		} /* if */ 
	} /* if */ 	

	return true;
} /* state_chooseship_cycle */ 
