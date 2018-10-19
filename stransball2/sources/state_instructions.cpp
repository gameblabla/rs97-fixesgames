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


bool state_instructions_cycle(SDL_Surface *screen,int sx,int sy,unsigned char *keyboard)
{
	if (SUBSTATE==0) {
		if (image!=0) SDL_FreeSurface(image);
		image=IMG_Load("graphics/tittle.png");
	} /* if */ 
	SDL_BlitSurface(image,0,screen,0);
	surface_fader(screen,0.5F,0.5F,0.5F,-1,0);
	if (SUBSTATE<64) {
		font_print(16,10,"              SUPER TRANSBALL 2 V1.5",screen);

		font_print(16,30,"  THE FUTURE, THE SUN DOES NOT SHINE ANYMORE",screen);
		font_print(16,40,"AND THE ONLY ENERGY SOURCE ARE \"THE SPHERES\".",screen);
		font_print(16,50,"THE SPHERES CONTAIN ENOUGH ENERGY FOR A PLANET",screen);
		font_print(16,60,"TO SURVIVE. THEY ARE CHARGED WITH THE ENERGY",screen);
		font_print(16,70,"OF OTHER STARS.",screen);
		font_print(16,90,"  AS THE HUMANS ARE HIGHLY DEPENDENT ON THE",screen);
		font_print(16,100,"SPHERES, AN OPORTUNIST CIVILIZATION HAS STOLEN",screen);
		font_print(16,110,"ALL THE CHARGED SPHERES FROM THE EARTH EXPECTING",screen);
		font_print(16,120,"TO COLONIZE THE PLANET. AS A LAST CHANCE, THE",screen);
		font_print(16,130,"LAST ENERGY HAS BEEN TRANSFERED TO A SCOUT SHIP",screen);
		font_print(16,140,"AND SENT TO RECOVER THE SPHERES...",screen);

		font_print(16,160,"YOU COMMAND THIS SHIP, YOU ARE THE LAST CHANCE",screen);
		font_print(16,170,"FOR THE SURVIVAL OF THE HUMAN RACE...",screen);
	} /* if */ 
	if (SUBSTATE>64 && SUBSTATE<128) {
		font_print(16,10,"              SUPER TRANSBALL 2 V1.5",screen);

		font_print(16,30,"  THE CONTROLS OF THE SHIP ARE [REDEFINIBLE]:",screen);
		font_print(16,50,"  UP    - PROPULSORS",screen);
		font_print(16,60,"  DOWN  - RETROPROPULSORS [NOT ALL THE SHIPS]",screen);
		font_print(16,70,"  LEFT  - TURN LEFT",screen);
		font_print(16,80,"  RIGHT - TURN RIGHT",screen);
		font_print(16,90,"  A     - FIRE",screen);
		font_print(16,100,"  B     - BALL ATRACTOR",screen);

		font_print(16,120,"TO ACHIEVE YOUR GOAL, YOU CAN CHOOSE BETWEEN",screen);
		font_print(16,130,"THREE DIFFERENT SHIPS, EACH ONE WITH ITS OWN",screen);
		font_print(16,140,"CHARACTERISTICS:",screen);
		font_print(16,160," THE SHADOW RUNNER: HIGH SPEED, FEW WEAPONS",screen);
		font_print(16,170," THE V-PANTHER 2  : MEDIUM SPEED, MEDIUM WEAPONS",screen);
		font_print(16,180," THE X-TERMINATOR : LOW SPEED, DEFINITIVE WEAPONS",screen);
	} /* if */ 

	if (SUBSTATE>128 && SUBSTATE<192) {
		font_print(16,10,"              SUPER TRANSBALL 2 V1.5",screen);

		font_print(16,30, "GUIDE[1]:",screen);
		font_print(16,50, " - YOUR SHIP IS UNDER THE EFFECT OF THE GRAVITY.",screen);
		font_print(16,60, " - WITH YOUR THRUSTER, YOU MUST AVOID TO COLLIDE",screen);
		font_print(16,70, "   WITH THE BACKGROUND.",screen);
		font_print(16,80, " - YOU MUST FIND THE WHITE BALL, THEN USE THE",screen);
		font_print(16,90, "   ATRACTOR OVER IT DURING A WHILE. IT WILL TURN",screen);
		font_print(16,100,"   BLUE. ",screen);
		font_print(16,110," - WHEN THE BALL IS BLUE, YOUR SHIP ATRACTS IT.",screen);
		font_print(16,120," - TO COMPLETE A LEVEL YOU MUST CARRY THE BALL",screen);
		font_print(16,130,"   TO THE UPPER PART.",screen);
		font_print(16,140," - YOU CAN KILL THE MANY ENEMIES YOU WILL FIND",screen);
		font_print(16,150,"   USING YOUR CANONS, SEVERAL SHOTS ARE NEEDED",screen);
		font_print(16,160,"   TO KILL THE ENEMIES [DEPENDING ON THE SHIP].",screen);
	} /* if */ 

	if (SUBSTATE>192 && SUBSTATE<256) {
		font_print(16,10,"              SUPER TRANSBALL 2 V1.5",screen);

		font_print(16,30, "GUIDE[2]:",screen);
		font_print(16,50, " - YOU WILL FIND DOORS THAT OPEN OR CLOSE WHEN",screen);
		font_print(16,60, "   YOU TAKE THE BALL.",screen);
		font_print(16,70, " - SOME OTHER DOORS ARE ACTIVATED BY SWITCHES.",screen);
		font_print(16,80, " - TO PRESS A SWITCH, YOU MUST MAKE THE BALL TO",screen);
		font_print(16,90, "   BOUNCE ON IT.",screen);
		font_print(16,100," - YOUR THRUSTER AND WEAPONS USE FUEL, YOU MUST ",screen);
		font_print(16,110,"   RECHARGE IT WHENEVER POSSIBLE.",screen);
		font_print(16,120," - SOME LEVELS MAY SEEM VERY DIFFICULT THE FIRST",screen);
		font_print(16,130,"   TIME, BUT WITH PRACTICE, EVERY LEVEL BECOMES",screen);
		font_print(16,140,"   EASY... :]",screen);
	} /* if */ 

	if (SUBSTATE>256 && SUBSTATE<320) {
		font_print(16,10,"              SUPER TRANSBALL 2 V1.5",screen);

		font_print(16,30,"THIS GAME IS INSPIRED ON THARA ZHRUSTA FOR THE",screen);
		font_print(16,40,"AMIGA 500, BUT UNFORTUNATELY CANNOT COMPARE WITH",screen);
		font_print(16,50,"IT...",screen);

		font_print(16,100,"                BRAIN 2002-2005",screen);
		font_print(16,110,"             SANTI ONTA~ON VILLAR",screen);
		font_print(16,120,"         SEE README FILE FOR MORE INFO",screen);
		font_print(16,140,"     GCW ZERO PORT 2015 ARTUR \"ZEAR\" ROJEK",screen);
	} /* if */ 
	if ((SUBSTATE%64)<32) {
		surface_fader(screen,float((SUBSTATE%64))/32.0F,float((SUBSTATE%64))/32.0F,float((SUBSTATE%64))/32.0F,-1,0);
		SUBSTATE++;
	} /* if */ 
	if ((SUBSTATE%64)==32) {
		if (keyboard[FIRE_KEY] && !old_keyboard[FIRE_KEY]) SUBSTATE++;
	} /* if */ 
	if ((SUBSTATE%64)>32) {
		surface_fader(screen,float(64-(SUBSTATE%64))/32.0F,float(64-(SUBSTATE%64))/32.0F,float(64-(SUBSTATE%64))/32.0F,-1,0);
		SUBSTATE++;
		if (SUBSTATE==320) {
			SDL_FreeSurface(image);
			image=0;

			STATE=1;
			SUBSTATE=0;
		} /* if */ 
	} /* if */ 

	return true;
} /* state_instructions_cycle */ 
