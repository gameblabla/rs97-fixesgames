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


bool state_changepack_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard)
{
	if (SUBSTATE==0) {
		if (image!=0) SDL_FreeSurface(image);
		image=IMG_Load("graphics/tittle.png");

		{
			levelpacks.Delete();
                                                         
#ifdef _WIN32
                        /* Find files: */ 
                        WIN32_FIND_DATA finfo;
                        HANDLE h;

                        h=FindFirstFile("maps/*.lp",&finfo);
                        if (h!=INVALID_HANDLE_VALUE) {
                                char *tmp;

                                tmp=new char[strlen(finfo.cFileName)+1];
                                strcpy(tmp,finfo.cFileName);
                                levelpacks.Add(tmp);

                                while(FindNextFile(h,&finfo)==TRUE) {
                                    char *tmp;

                                    tmp=new char[strlen(finfo.cFileName)+1];
                                    strcpy(tmp,finfo.cFileName);
                                    levelpacks.Add(tmp);
                                } /* while */ 
                        } /* if */ 
#else
                        DIR *dp;
                        struct dirent *ep;
		  
                        dp = opendir ("maps");
                        if (dp != NULL)
                        {
                            while (ep = readdir (dp))
                            {
                                char *tmp;
                         
                                if (strlen(ep->d_name)>4 &&
                                    ep->d_name[strlen(ep->d_name)-3]=='.' &&
                                    ep->d_name[strlen(ep->d_name)-2]=='l' &&
                                    ep->d_name[strlen(ep->d_name)-1]=='p') {
                                    tmp=new char[strlen(ep->d_name)+1];
                                    strcpy(tmp,ep->d_name);
                                    levelpacks.Add(tmp);                                    
                                } /* if */
                               
                            }
                            (void) closedir (dp);
                        }
#endif                                 
                    act_levelpack=0;
		}
	} /* if */ 
	SDL_BlitSurface(image,0,screen,0);
	surface_fader(screen,0.5F,0.5F,0.5F,-1,0);

	font_print_centered(sx/2,(sy/2)-24,"CHOOSE LEVEL-PACK:",screen);

	{
		int i,y;
		char *tmp;

		i=0;
		y=(sy/2)-8;
		levelpacks.Rewind();
		while(levelpacks.Iterate(tmp)) {
			if (i==act_levelpack) {
				SDL_Rect r;
				r.x=0;
				r.y=y;
				r.w=SCREEN_X;
				r.h=8;
				SDL_FillRect(screen,&r,SDL_MapRGB(screen->format,255,0,0));
			} /* if */ 
			font_print_centered(sx/2,y,tmp,screen);
			i++;
			y+=8;
		} /* while */ 
	}

	if (SUBSTATE<32) {
		surface_fader(screen,float(SUBSTATE)/32.0F,float(SUBSTATE)/32.0F,float(SUBSTATE)/32.0F,-1,0);
		SUBSTATE++;
	} /* if */ 
	if (SUBSTATE==32) {
		if (keyboard[FIRE_KEY] && !old_keyboard[FIRE_KEY]) {
			/* change level-pack: */ 
			strcpy(levelpack,levelpacks[act_levelpack]);
			NLEVELS=-1;

			SUBSTATE2=0;
			SUBSTATE++;
		} /* if */ 
		if (keyboard[SDLK_ESCAPE] && !old_keyboard[SDLK_ESCAPE]) {
			SUBSTATE2=0;
			SUBSTATE++;
		} /* if */ 

		if (keyboard[SDLK_UP] && !old_keyboard[SDLK_UP]) {
			if (act_levelpack>0) act_levelpack--;
		} /* if */ 
		if (keyboard[SDLK_DOWN] && !old_keyboard[SDLK_DOWN]) {
			if (act_levelpack<(levelpacks.Length()-1)) act_levelpack++;
		} /* if */ 
	} /* if */ 
	if (SUBSTATE>32) {
		surface_fader(screen,float(64-SUBSTATE)/32.0F,float(64-SUBSTATE)/32.0F,float(64-SUBSTATE)/32.0F,-1,0);
		SUBSTATE++;
		if (SUBSTATE==64) {
			STATE=1;
			SUBSTATE=0;
		} /* if */ 
	} /* if */ 
	return true;
} /* state_changepack_cycle */ 
