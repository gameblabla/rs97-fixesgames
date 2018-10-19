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


bool state_game_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard)
{
	int retval=0;

	if (keyboard[PAUSE_KEY] && !old_keyboard[PAUSE_KEY]) {
		if (pause) pause=false;
			  else pause=true;
	} /* if */ 

	if (!pause) {
		retval=game->cycle(keyboard);
		if (replayfile!=0) {
			fputc(keyboard[THRUST_KEY],replayfile);
			fputc(keyboard[ANTITHRUST_KEY],replayfile);
			fputc(keyboard[LEFT_KEY],replayfile);
			fputc(keyboard[RIGHT_KEY],replayfile);
			fputc(keyboard[FIRE_KEY],replayfile);
			fputc(keyboard[ATRACTOR_KEY],replayfile);
			fputc(retval,replayfile);
		} /* if */ 
	} /* if */ 

	game->render(screen,sx,sy);

	if (pause) {
		surface_fader(screen,0.5F,0.5F,0.5F,-1,0);
		font_print_centered(sx/2,sy/2-16,"PAUSE",screen);
	} else {
		timer++;
	} /* if */ 

	/* Print time */ 
	{
		char tmp[128];
		int min,sec,dec;

		dec=(timer*18)/10;
		sec=dec/100;
		dec=dec%100;
		min=sec/60;
		sec=sec%60;
		sprintf(tmp,"%.2i:%.2i",min,sec);
		font_print_right(sx,0,tmp,screen);
	}

	if (retval!=0) {
		/* Retrieve statistics: */ 
//				timmer=game->get_statistics(0);
		used_fuel=game->get_statistics(1);
		remaining_fuel=game->get_statistics(2);
		n_shots=game->get_statistics(3);
		n_hits=game->get_statistics(4);
		enemies_destroyed=game->get_statistics(5);

		delete game;
		game=0;
		fclose(replayfile);
		replayfile=0;
	} /* if */ 
	if (retval==1) {
		STATE=5;
		SUBSTATE=0;
	} /* if */ 
	if (retval==2) {
		STATE=13;
		SUBSTATE=0;
	} /* if */ 
	if (retval==3) {
		STATE=9;
		SUBSTATE=0;
	} /* if */ 

	return true;
} /* state_game_cycle */ 
