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
#include "game.h"

#include "encoder.h"


#define MAXLEVELS	64

int NLEVELS=-1;


extern int SCREEN_X,SCREEN_Y,PIXEL_SIZE;
int STATE=0,SUBSTATE=0,SUBSTATE2=0;
int level=0,timer=0;
int ship_type=1;

/* Statistics: */ 
extern int fuelfactor[3];
int used_fuel=0,remaining_fuel=0;
int n_shots=0,n_hits=0,enemies_destroyed=0;
int previous_high=-1;

char *levelnames[MAXLEVELS]={0,0,0,0,0,0,0,0,
							 0,0,0,0,0,0,0,0,
							 0,0,0,0,0,0,0,0,
							 0,0,0,0,0,0,0,0,
							 0,0,0,0,0,0,0,0,
							 0,0,0,0,0,0,0,0,
							 0,0,0,0,0,0,0,0,
							 0,0,0,0,0,0,0,0};
char *leveltext[MAXLEVELS]={0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0};
char *levelcode[MAXLEVELS]={0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0};
int initialfuel[MAXLEVELS]={0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0};

/* Game: */ 
/* playmode = 0 : normal */ 
/* playmode = 1 : recording */ 
/* playmode = 0 : replaying */ 
extern TRANSBALL *game;

/* Frames per second counter: */ 
extern int frames_per_sec;

FILE *replayfile;
int replaynum;
int replay_source=0;
char *replay_dir;
char *high_dir;

SDLKey THRUST_KEY=SDLK_UP,ANTITHRUST_KEY=SDLK_DOWN,LEFT_KEY=SDLK_LEFT,RIGHT_KEY=SDLK_RIGHT;
SDLKey FIRE_KEY=SDLK_LCTRL,ATRACTOR_KEY=SDLK_LALT;
SDLKey PAUSE_KEY=SDLK_RETURN;
bool pause=false;

unsigned char old_keyboard[SDLK_LAST];
SDL_Surface *image=0,*image2=0;	/* For the tittle screen, etc. */ 

/* Edit variables: */ 
char edit_text[80];
int edit_position;

/* Replay variables: */ 
List<char> files;
int act_file;
int first_file;
bool refind_files;

/* Demo variables: */ 
int demotimer;
bool demoon;
int tittle_alpha;

/* Actual level-pack: */ 
List<char> levelpacks;
int act_levelpack;
char levelpack[256]="transball.lp";


bool gamecycle(SDL_Surface *screen,int sx,int sy)
{
	int i;
	unsigned char *keyboard;
	SDL_PumpEvents();
	keyboard = SDL_GetKeyState(NULL);

	if (NLEVELS==-1) {
		/* Load level info: */ 
		FILE *fp;
		char tmp[256];

		strcpy(tmp,"maps/");
		strcat(tmp,levelpack);

		decode(tmp,"/tmp/decoding.tmp");

		fp=fopen("/tmp/decoding.tmp","r+");
		if (fp!=0) {
			fscanf(fp,"%i",&NLEVELS);

			for(i=0;i<NLEVELS;i++) {
				fscanf(fp,"%s %s",tmp,tmp);
				if (levelnames[i]!=0) delete levelnames[i];
				levelnames[i]=new char[strlen(tmp)+1];
				strcpy(levelnames[i],tmp);

				fscanf(fp,"%s ",tmp);
				fgets(tmp,128,fp);
				if (leveltext[i]!=0) delete leveltext[i];
				leveltext[i]=new char[strlen(tmp)+1];
				strcpy(leveltext[i],tmp);

				fscanf(fp,"%s %s",tmp,tmp);
				if (levelcode[i]!=0) delete levelcode[i];
				levelcode[i]=new char[strlen(tmp)+1];
				strcpy(levelcode[i],tmp);

				fscanf(fp,"%s %i",tmp,&initialfuel[i]);
			} /* for */ 

			fclose(fp);
		} /* if */ 

		remove("/tmp/decoding.tmp");
	} /* if */ 

	if (NLEVELS==-1) return false;

	switch(STATE) {
	case 0:	if (!state_logo_cycle(screen,sx,sy,keyboard)) return false;
			break;

	case 1: if (!state_mainmenu_cycle(screen,sx,sy,keyboard)) return false;
			break;

	case 2: if (!state_instructions_cycle(screen,sx,sy,keyboard)) return false;
			break;

	case 3: if (!state_typetext_cycle(screen,sx,sy,keyboard)) return false;
			break;

	case 4: if (!state_chooseship_cycle(screen,sx,sy,keyboard)) return false;
			break;

	case 5: if (!state_interphase_cycle(screen,sx,sy,keyboard)) return false;
			break;

	case 6: if (!state_game_cycle(screen,sx,sy,keyboard)) return false;
			break;

	case 7: if (!state_replaymanager_cycle(screen,sx,sy,keyboard)) return false;
			break;

	case 8: if (!state_replay_cycle(screen,sx,sy,keyboard)) return false;
			break;

	case 9: if (!state_gameover_cycle(screen,sx,sy,keyboard)) return false;
			break;

	case 10: if (!state_endsequence_cycle(screen,sx,sy,keyboard)) return false;
			 break;

	case 11: if (!state_keyredefinition_cycle(screen,sx,sy,keyboard)) return false;
			 break;

	case 12: if (!state_changepack_cycle(screen,sx,sy,keyboard)) return false;
	 		 break;

	case 13: if (!state_levelfinished_cycle(screen,sx,sy,keyboard)) return false;
			 break;

	} /* switch */ 

	/* Print the FPS: */ 
//	{
//		char tmp[80];
//		sprintf(tmp,"%i FPS",frames_per_sec);
//		font_print_right(sx,sy-8,tmp,screen);
//	}

	for(i=0;i<SDLK_LAST;i++) old_keyboard[i]=keyboard[i];
	return true;	
} /* gamecycle */ 
