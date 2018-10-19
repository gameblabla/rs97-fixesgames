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
extern char *replay_dir;
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


bool state_interphase_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard)
{
	char tmp[80];
	sprintf(tmp,"LEVEL %i",level+1);
	font_print_centered(sx/2,(sy/2)-24,tmp,screen);
	font_print_centered(sx/2,(sy/2)-16,leveltext[level],screen);
	font_print_centered(sx/2,(sy/2),levelcode[level],screen);

	if (SUBSTATE<32) {
		surface_fader(screen,float(SUBSTATE)/32.0F,float(SUBSTATE)/32.0F,float(SUBSTATE)/32.0F,-1,0);
		SUBSTATE++;
	} /* if */ 
	if (SUBSTATE==32) {
		if (keyboard[FIRE_KEY] && !old_keyboard[FIRE_KEY]) SUBSTATE++;
	} /* if */ 
	if (SUBSTATE>32) {
		surface_fader(screen,float(64-SUBSTATE)/32.0F,float(64-SUBSTATE)/32.0F,float(64-SUBSTATE)/32.0F,-1,0);
		SUBSTATE++;
		if (SUBSTATE==64) {
			int i;
			char tmp[256];
			char levelname[256];

			STATE=6;
			SUBSTATE=0;
			if (game=0) delete game;
			game=new TRANSBALL("graphics/","sound/","maps/",initialfuel[level],levelnames[level],ship_type);
			snprintf(tmp, sizeof(tmp), "%s/replay%.3i.rpl",replay_dir,replaynum++);
			replayfile=fopen(tmp,"wb+");
			fputc(32,replayfile);
			fputc(0,replayfile);
			/* level name: */ 
			for(i=0;i<256;i++) levelname[i]=0;
			strcpy(levelname,levelnames[level]);
			for(i=0;i<256;i++) fputc(levelname[i],replayfile);
			fputc(initialfuel[level],replayfile);
			fputc(ship_type,replayfile);

			timer=0;
		} /* if */ 
	} /* if */ 

	return true;
} /* state_interphase_cycle */ 
