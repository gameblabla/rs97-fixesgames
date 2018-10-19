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
extern char *high_dir;
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


bool state_levelfinished_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard)
{
	char tmp[80];

	if (SUBSTATE==0) {
		/* UPDATE THE HIGHSCORES: */ 
		int current_time=timer;
		int best_time=-1;
		int i,j;
		char highname[256],filename2[256];

		previous_high=-1;

		j=0;

		snprintf(highname, sizeof(highname), "%s/", high_dir);
		j+=strlen(highname);
		for(i=0;levelpack[i]!='.' && levelpack[i]!=0;i++) highname[j++]=levelpack[i];
		highname[j++]='-';
		sprintf(highname+j,"%.3i",level);
		j+=3;
		highname[j++]='-';
		if (ship_type==0) highname[j++]='S';
		if (ship_type==0) highname[j++]='R';
		if (ship_type==1) highname[j++]='V';
		if (ship_type==1) highname[j++]='P';
		if (ship_type==2) highname[j++]='X';
		if (ship_type==2) highname[j++]='T';
		highname[j++]='.';
		highname[j++]='r';
		highname[j++]='p';
		highname[j++]='l';
		highname[j++]=0;


		{
			int retval;
			int s,l;
			char levelname[256];
			
			retval=replay_parameters(highname,&s,&l,levelname);

			if (retval==2) {
				previous_high=best_time=l;
			} /* if */ 
		}

		if (best_time==-1 || best_time>current_time) {
			snprintf(filename2,sizeof(filename2),"%s/replay%.3i.rpl",replay_dir,replaynum-1);
			replay_copy(filename2,highname);
		} /* if */ 
	} /* if */ 

	sprintf(tmp,"LEVEL %i COMPLETED",level+1);
	font_print_centered(sx/2,(sy/2)-32,tmp,screen);

	/* print time */ 
	{
		char tmp[128];
		int min,sec,dec;

		dec=(timer*18)/10;
		sec=dec/100;
		dec=dec%100;
		min=sec/60;
		sec=sec%60;
		sprintf(tmp,"TIME: %.2i:%.2i'%.2i",min,sec,dec);
		font_print_centered(sx/2,(sy/2)-16,tmp,screen);

		if (previous_high!=-1) {
			dec=(previous_high*18)/10;
			sec=dec/100;
			dec=dec%100;
			min=sec/60;
			sec=sec%60;
			sprintf(tmp,"PREVIOUS HIGH: %.2i:%.2i'%.2i",min,sec,dec);
			font_print_centered(sx/2,(sy/2)-8,tmp,screen);
		} else {
			font_print_centered(sx/2,(sy/2)-8,"PREVIOUS HIGH: --",screen);
		} /* if */ 
	}

	/* print other statistics: */ 
	{
		sprintf(tmp,"FUEL, INITIAL: %i, USED: %i, REMAINING: %i",initialfuel[level]*fuelfactor[ship_type],used_fuel,remaining_fuel);
		font_print_centered(sx/2,(sy/2)+8,tmp,screen);

		sprintf(tmp,"SHOTS: %i, HITS: %i, ACCURACY: %.2f%%",n_shots,n_hits,(n_shots==0 ? 0.0F:100.0F*float(n_hits)/float(n_shots)));
		font_print_centered(sx/2,(sy/2)+16,tmp,screen);

		sprintf(tmp,"ENEMIES DESTROYED: %i",enemies_destroyed);
		font_print_centered(sx/2,(sy/2)+24,tmp,screen);
	}

	font_print_centered(sx/2,(sy/2)+40,"A -     CONTINUE",screen);
	font_print_centered(sx/2,(sy/2)+48,"R - REPEAT LEVEL",screen);

	if (SUBSTATE<32) {
		surface_fader(screen,float(SUBSTATE)/32.0F,float(SUBSTATE)/32.0F,float(SUBSTATE)/32.0F,-1,0);
		SUBSTATE++;
	} /* if */ 
	if (SUBSTATE==32) {
		if (keyboard[SDLK_LCTRL] && !old_keyboard[SDLK_LCTRL]) {
			SUBSTATE++;
			SUBSTATE2=0;
		} /* if */ 
		if (keyboard[SDLK_BACKSPACE] && !old_keyboard[SDLK_BACKSPACE]) {
			SUBSTATE++;
			SUBSTATE2=1;
		} /* if */ 
	} /* if */ 
	if (SUBSTATE>32) {
		if (SUBSTATE2==0) {
			level++;
			if (level>=NLEVELS) {
				STATE=10;
				SUBSTATE=0;
			} else {
				STATE=5;
				SUBSTATE=0;
			} /* if */ 
		} else {
			STATE=5;
			SUBSTATE=0;
		} /* if */ 
	} /* if */ 

	return true;
} /* state_levelfinished_cycle */ 
