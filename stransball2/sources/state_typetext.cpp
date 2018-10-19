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
int cpos = 97;


bool state_typetext_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard)
{
	SDL_Rect r;
	int i;
	int maxlen;

	if (SUBSTATE==0) {
		if (image!=0) SDL_FreeSurface(image);
		image=IMG_Load("graphics/tittle.png");
	} /* if */ 

	SDL_BlitSurface(image,0,screen,0);
	i=SUBSTATE;
	if (i>32) i=32;
	surface_fader(screen,float(i)/32.0F,float(i)/32.0F,float(i)/32.0F,-1,0);

	SUBSTATE++;
	if (SUBSTATE2==0) {
		font_print_centered(sx/2,40,"ENTER LEVEL CODE:",screen);
		maxlen=6;
	} /* if */ 
	if (SUBSTATE2==1) {
		font_print_centered(sx/2,40,"TYPE NEW FILE NAME:",screen);
		maxlen=32;
	} /* if */ 

	rectangle(screen,160-((maxlen+1)*3+2),52,(maxlen+1)*6+4,10,SDL_MapRGB(screen->format,255,255,255));

	font_print(160-(maxlen+1)*3,54,edit_text,screen);
	r.x=160-(maxlen+1)*3+(edit_position)*6;
	r.y=60;
	r.w=7;
	r.h=1;

	if (((SUBSTATE>>4)&0x01)!=0) SDL_FillRect(screen,&r,SDL_MapRGB(screen->format,255,255,255));

	if (keyboard[SDLK_DOWN] && !old_keyboard[SDLK_DOWN]) {
		cpos--;

		if (cpos == 96) cpos = 57;
		if (cpos == 47) cpos = 122;
	} /* if */

	if (keyboard[SDLK_UP] && !old_keyboard[SDLK_UP]) {
		cpos++;

		if (cpos == 58) cpos = 97;
		if (cpos == 123) cpos = 48;
	} /* if */

	if (edit_position>=maxlen) edit_position=maxlen-1;
	edit_text[edit_position] = (char)cpos;

	if (keyboard[SDLK_LEFT] && !old_keyboard[SDLK_LEFT]) {
		edit_position--;
		if (edit_position<0) edit_position=0;
		cpos = edit_text[edit_position];
	}

	if (keyboard[SDLK_RIGHT] && !old_keyboard[SDLK_RIGHT]) {
		edit_position++;
		if (edit_position>=maxlen) edit_position=maxlen-1;
		cpos = edit_text[edit_position];
		if (cpos == 0) {
			cpos = edit_text[edit_position-1];
			edit_text[edit_position+1] = 0;
		}
	}

	if ((keyboard[SDLK_RETURN] && !old_keyboard[SDLK_RETURN]) || (keyboard[SDLK_LCTRL] && !old_keyboard[SDLK_LCTRL])) {
		SDL_FreeSurface(image);
		image=0;

		if (SUBSTATE2==0) {
			level=0;
			for(i=0;i<NLEVELS;i++) {
				if (strcmp(edit_text,levelcode[i])==0) {
					level=i;
				} /* if */ 
			} /* for */ 
			
			if (level==0) {
				STATE=1;
				SUBSTATE=0;
			} else {
				int i;

				for(i=0;i<1000;i++) {
					char tmp[256];
					snprintf(tmp,sizeof(tmp),"%s/replay%.3i.rpl",replay_dir, i);
					remove(tmp);
				} /* for */ 
				STATE=4;
				SUBSTATE=0;
				replaynum=0;
			} /* if */ 
		} /* if */ 
		if (SUBSTATE2==1) {
			char tmp[256],tmp2[256];
			int i;
			bool found=false;

			for(i=0;i<edit_position && !found;i++) {
				if (edit_text[i]=='.') {
					edit_text[i+1]='r';
					edit_text[i+2]='p';
					edit_text[i+3]='l';
					edit_text[i+4]=0;
					found=true;
				} /* if */ 
			} /* for */ 
			if (!found) {
				edit_text[edit_position]='.';
				edit_text[edit_position+1]='r';
				edit_text[edit_position+2]='p';
				edit_text[edit_position+3]='l';
				edit_text[edit_position+4]=0;
			} /* if */ 
			STATE=7;
			SUBSTATE=0;
			snprintf(tmp,sizeof(tmp),"%s/%s",replay_dir,files[act_file]);
			snprintf(tmp2,sizeof(tmp),"%s/%s",replay_dir,edit_text);
			rename(tmp,tmp2);
		} /* if */  

		cpos = 97; // Reset cursor position to 'a'.
	} /* if */ 

	return true;
} /* state_typetext_cycle */ 
