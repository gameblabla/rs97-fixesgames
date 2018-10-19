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


bool state_endsequence_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard)
{
	if (strcmp(levelpack,"levels.lp")==0) {
		font_print(16,10,"                CONGRATULATIONS!",screen);

		font_print(16,30,"YOU HAVE RECOVERED ALL THE STOLEN SPHERES!!",screen);
		font_print(16,40,"YOU HAVE GIVEN THE HUMAN KIND ANOTHER CHANCE",screen);
		font_print(16,50,"TO SURVIVE. BUT THIS IS NOT THE END, A WAR HAS",screen);
		font_print(16,60,"STARTED BETWEEN THE HUMANS AND THE ALIEN RACE...",screen);

		font_print(16,80,"MORE CHALLENGES AWAIT YOU IN TRANSBALL 3...",screen);
	} else {
		font_print(16,10,"                CONGRATULATIONS!",screen);

		font_print(16,30,"YOU HAVE FINISHED THIS LEVEL-PACK!!",screen);
		font_print(16,40,"BUT...",screen);
		font_print(16,50,"HAVE YOU ALREADY FINISHED ALL THE OTHER PACKS?",screen);
	} /* if */ 

	if (SUBSTATE<32) {
		surface_fader(screen,float(SUBSTATE)/32.0F,float(SUBSTATE)/32.0F,float(SUBSTATE)/32.0F,-1,0);
		SUBSTATE++;
	} /* if */ 
	if (SUBSTATE==32) {
		if (keyboard[FIRE_KEY] && !old_keyboard[FIRE_KEY]) {
			SUBSTATE++;
		} /* if */ 
	} /* if */ 
	if (SUBSTATE>32) {
		surface_fader(screen,float(64-SUBSTATE)/32.0F,float(64-SUBSTATE)/32.0F,float(64-SUBSTATE)/32.0F,-1,0);
		SUBSTATE++;
		if (SUBSTATE==64) {
			if (SUBSTATE2==0) {
				STATE=1;
				SUBSTATE=0;
			} /* if */ 
		} /* if */ 
	} /* if */ 

	return true;
} /* state_endsequence_cycle */ 
