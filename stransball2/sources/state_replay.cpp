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


bool state_replay_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard)
{
	int retval=0;

	SUBSTATE++;

	if (keyboard[PAUSE_KEY] && !old_keyboard[PAUSE_KEY]) {
		if (pause) pause=false;
			  else pause=true;
	} /* if */ 

	if (!pause) {
		int i;
		unsigned char tmp[SDLK_LAST];
		for(i=0;i<SDLK_LAST;i++) tmp[i]=0;
		tmp[THRUST_KEY]=fgetc(replayfile);
		tmp[ANTITHRUST_KEY]=fgetc(replayfile);
		tmp[LEFT_KEY]=fgetc(replayfile);
		tmp[RIGHT_KEY]=fgetc(replayfile);
		tmp[FIRE_KEY]=fgetc(replayfile);
		tmp[ATRACTOR_KEY]=fgetc(replayfile);
		retval=game->cycle(tmp);
		retval=fgetc(replayfile);
	} /* if */ 

	game->render(screen,sx,sy);

	if (pause) {
		surface_fader(screen,0.5F,0.5F,0.5F,-1,0);
		font_print_centered(sx/2,sy/2-16,"PAUSE",screen);
	} else {
		timer++;
	} /* if */ 

	/* print time */ 
	{
		char tmp[128];
		int min,sec,dec;

		dec=(timer*18)/10;
		sec=dec/100;
		dec=dec%100;
		min=sec/60;
		sec=sec%60;
		sprintf(tmp,"%.2i:%.2i'%.2i",min,sec,dec);
		font_print_right(sx,0,tmp,screen);
	}	

	if (((SUBSTATE>>5)&0x01)==0) {
		font_print_centered(sx/2,sy-10,"REPLAY",screen);
	} /* if */ 

	if (retval!=0 || keyboard[SDLK_ESCAPE]) {
		delete game;
		game=0;
		fclose(replayfile);
		replayfile=0;
		STATE=7;
		SUBSTATE=0;
	} /* if */ 
	return true;
} /* state_replay_cycle */ 
