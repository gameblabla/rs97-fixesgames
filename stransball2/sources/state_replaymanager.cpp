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


bool state_replaymanager_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard)
{
	if (SUBSTATE==0) {
		if (image!=0) SDL_FreeSurface(image);
		image=IMG_Load("graphics/tittle.png");
		refind_files=true;
		SUBSTATE2=0;
	} /* if */ 

	if (refind_files) {

		refind_files=false;
		files.Delete();
#ifdef _WIN32
		/* Find files: */ 
		WIN32_FIND_DATA finfo;
		HANDLE h;

		if (replay_source==0) h=FindFirstFile("replays/*.rpl",&finfo);
						 else h=FindFirstFile("high/*.rpl",&finfo);
		if (h!=INVALID_HANDLE_VALUE) {
			char *tmp;

			tmp=new char[strlen(finfo.cFileName)+1];
			strcpy(tmp,finfo.cFileName);
			files.Add(tmp);

			while(FindNextFile(h,&finfo)==TRUE) {
				char *tmp;

				tmp=new char[strlen(finfo.cFileName)+1];
				strcpy(tmp,finfo.cFileName);
				files.Add(tmp);
			} /* while */ 
		} /* if */ 
#else
		DIR *dp;
		struct dirent *ep;
		  
		if (replay_source==0) dp = opendir (replay_dir);
						 else dp = opendir (high_dir);
		if (dp != NULL)
		 {
			while (ep = readdir (dp))
			 {
                            char *tmp;
                         
                            if (strlen(ep->d_name)>4 &&
                                ep->d_name[strlen(ep->d_name)-4]=='.' &&
                                ep->d_name[strlen(ep->d_name)-3]=='r' &&
                                ep->d_name[strlen(ep->d_name)-2]=='p' &&
                                ep->d_name[strlen(ep->d_name)-1]=='l') {
                                tmp=new char[strlen(ep->d_name)+1];
                                strcpy(tmp,ep->d_name);
                                files.Add(tmp);                                    
                            } /* if */
                               
			 }
			(void) closedir (dp);
		 }
#endif                          
		first_file=0;
		act_file=0;		
	} /* if */ 

	SDL_BlitSurface(image,0,screen,0);
	surface_fader(screen,0.5F,0.5F,0.5F,-1,0);


	if (replay_source==0) font_print_centered(sx/2,20,"REPLAY FILES [L - CHANGE SOURCE]:",screen);
					 else font_print_centered(sx/2,20,"HIGH SCORE FILES [L - CHANGE SOURCE]:",screen);
	if (replay_source!=0) font_print_centered(sx/2,30,"PRESS FIRE TO VIEW OR R TO REBUILD",screen);
	
	{
		char *tmp;
		int i;

		files.Rewind();
		i=0;
		while(files.Iterate(tmp)) {
			if (i>=first_file &&
				i<first_file+18) {
				char *sname[3]={"SH.RNR.","PNTR.2","X-TRM."};
				int ship,length;
				char tmp2[256],tmp3[256];

				if (i==act_file) {
					SDL_Rect r;
					r.x=2;
					r.y=42+(i-first_file)*10-1;
					r.w=strlen(tmp)*6+2;
					r.h=9;
					SDL_FillRect(screen,&r,SDL_MapRGB(screen->format,255,0,0));
				} /* if */ 
				font_print(4,42+(i-first_file)*10,tmp,screen);
				
				if (replay_source==0) snprintf(tmp2, sizeof(tmp2), "%s/%s",replay_dir, tmp);
								 else snprintf(tmp2, sizeof(tmp2), "%s/%s",high_dir, tmp);
				replay_parameters(tmp2,&ship,&length,tmp3);
				{
					int min,sec,dec;

					dec=(length*18)/10;
					sec=dec/100;
					dec=dec%100;
					min=sec/60;
					sec=sec%60;
					sprintf(tmp2,"%.2i:%.2i'%.2i %s",min,sec,dec,sname[ship]);
					font_print(220,42+(i-first_file)*10,tmp2,screen);
				}

			} /* if */ 
			i++;
		} /* while */ 
	} 

	rectangle(screen,-1,40,321,185,SDL_MapRGB(screen->format,255,255,255));
	rectangle(screen,214,40,0,185,SDL_MapRGB(screen->format,255,255,255));

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

			if (SUBSTATE2==0) {
				STATE=1;
				SUBSTATE=0;
			} /* if */ 
			if (SUBSTATE2==1) {
				int i;
				char tmp[256];
				char levelname[256];
				int v1,v2;
				int fuel;
				STATE=8;
				SUBSTATE=0;

				if (replay_source==0) snprintf(tmp,sizeof(tmp), "%s/%s", replay_dir, files[act_file]);
								 else snprintf(tmp, sizeof(tmp), "%s/%s",high_dir, files[act_file]);
				replayfile=fopen(tmp,"rb");
				v1=fgetc(replayfile);
				v2=fgetc(replayfile);	// To maintain compatibility with a previous version

				level=0;
				for(i=0;i<256;i++) levelname[i]=fgetc(replayfile);
				fuel=fgetc(replayfile);

				ship_type=fgetc(replayfile);
				game=new TRANSBALL("graphics/","sound/","maps/",fuel,levelname,ship_type);

				timer=0;
			} /* if */ 
		} /* if */ 
	} /* if */ 

	if (SUBSTATE==32) {
		if ((keyboard[SDLK_UP] && !old_keyboard[SDLK_UP]) ||
			(keyboard[SDLK_LEFT] && !old_keyboard[SDLK_LEFT])) {
			if (act_file>0) act_file--;
			if (act_file<first_file) first_file=act_file;
		} /* if */ 

		if ((keyboard[SDLK_DOWN] && !old_keyboard[SDLK_DOWN]) ||
			(keyboard[SDLK_RIGHT] && !old_keyboard[SDLK_RIGHT])) {
			if (act_file<(files.Length()-1)) act_file++;
			if (act_file>=(first_file+18)) first_file=act_file-17;
		} /* if */ 

		if (keyboard[SDLK_PAGEUP] && !old_keyboard[SDLK_PAGEUP]) {
			act_file-=18;
			if (act_file<0) act_file=0;
			if (act_file<first_file) first_file=act_file;
		} /* if */ 

		if (keyboard[SDLK_PAGEDOWN] && !old_keyboard[SDLK_PAGEDOWN]) {
			act_file+=18;
			if (act_file>=files.Length()) act_file=(files.Length()-1);
			if (act_file>=(first_file+18)) first_file=act_file-17;
		} /* if */ 

		if (keyboard[SDLK_LCTRL] && !old_keyboard[SDLK_LCTRL]) {
			SUBSTATE2=1;
			if (files.EmptyP()) SUBSTATE2=0;
			SUBSTATE++;
		} /* if */ 

		if (keyboard[SDLK_ESCAPE] && !old_keyboard[SDLK_ESCAPE]) {
			SUBSTATE2=0;
			SUBSTATE++;
		} /* if */ 

		if (keyboard[SDLK_TAB] && !old_keyboard[SDLK_TAB]) {
			if (replay_source==0) replay_source=1;
							 else replay_source=0;
			act_file=0;
			first_file=0;
			refind_files=true;
		} /* if */ 
		
		if (keyboard[SDLK_BACKSPACE] && !old_keyboard[SDLK_BACKSPACE] && replay_source==1) {
			{
				List<char> replays;
				char *replay;
				char complete_replayname[256];

				/* look for files: */ 
#ifdef _WIN32
				WIN32_FIND_DATA finfo;
				HANDLE h;

				h=FindFirstFile("replays/*.rpl",&finfo);
				if (h!=INVALID_HANDLE_VALUE) {
					char *tmp;

					tmp=new char[strlen(finfo.cFileName)+1];
					strcpy(tmp,finfo.cFileName);
					files.Add(tmp);

					while(FindNextFile(h,&finfo)==TRUE) {
						char *tmp;

						tmp=new char[strlen(finfo.cFileName)+1];
						strcpy(tmp,finfo.cFileName);
						replays.Add(tmp);
					} /* while */ 
				} /* if */ 
#else
				DIR *dp;
				struct dirent *ep;
				  
				dp = opendir (replay_dir);
				if (dp != NULL)
				 {
					while (ep = readdir (dp))
					 {
									char *tmp;
                         
									if (strlen(ep->d_name)>4 &&
										ep->d_name[strlen(ep->d_name)-4]=='.' &&
										ep->d_name[strlen(ep->d_name)-3]=='r' &&
										ep->d_name[strlen(ep->d_name)-2]=='p' &&
										ep->d_name[strlen(ep->d_name)-1]=='l') {
										tmp=new char[strlen(ep->d_name)+1];
										strcpy(tmp,ep->d_name);
										replays.Add(tmp);                                    
									} /* if */
                               
					 }
					(void) closedir (dp);
				 }
#endif                  
				
				/* Check if this replay is a highscore: */ 
				replays.Rewind();
				while(replays.Iterate(replay)) {
					int level;
					int ship,current_time,previous_high=-1;
					char levelname[256];
					
					snprintf(complete_replayname, sizeof(complete_replayname), "%s/%s",replay_dir, replay);
					if (replay_parameters(complete_replayname,&ship,&current_time,levelname)==2) {
						for(level=0;level<NLEVELS;level++) {
							if (strcmp(levelname,levelnames[level])==0) {
								/* the replay corresponds to level 'i' of the current level pack: */ 

								/* UPDATE THE HIGHSCORES: */ 
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
								if (ship==0) highname[j++]='S';
								if (ship==0) highname[j++]='R';
								if (ship==1) highname[j++]='V';
								if (ship==1) highname[j++]='P';
								if (ship==2) highname[j++]='X';
								if (ship==2) highname[j++]='T';
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
										previous_high=l;
									} /* if */ 
								}

								if (previous_high==-1 || previous_high>current_time) {
									snprintf(filename2, sizeof(filename2), "%s/%s",replay_dir, replay);
									replay_copy(filename2,highname);
								} /* if */ 

							} /* if */ 
						} /* if */ 

					} /* if */ 

				} /* while */ 
				
			}
			act_file=0;
			first_file=0;
			refind_files=true;
		} /* if */ 

	} /* if */ 

	return true;
} /* state_replaymanager_cycle */ 
