/*

a simple game like the Mr.Driller.(c) namco


2000/7/7,adas


coding is restarted from 2000/8/28
*/

#include "geometry.h"
#include "Cffont.h"
#include "CTime.h"
#include "CInput.h"
#include "CBmps.h"
#include "CWavs.h"

#include "SDL.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

CWavs *wavs;
#define WAVMAX	100
char *wavfile="wav.txt";
char *wavdir="./system/wav/";

SDL_Surface *ScreenSurface;
//ススムくん
CBmps *playerbmps=NULL;
//char *playerbmpsdir="./player/";
char *playerbmpsfile="player.txt";
#define PLAYER_NUM	256

// static variables

SDL_Surface *screen;

CTime gametime;

#define WALKSPEED	4
#define WALKSPEED_Y	6

//#define PREFALLTIME	48
#define PREFALLTIME	72

#define NO_BLOCK	100
#define AIR_BLOCK 20
#define HARD_BLOCK 5
#define CLEAR_BLOCK 4

#define	BLOCK_NUM 256
CBmps* cbmps_blocks;
char *blockfile="blockbg.txt";

Cffont *font=NULL;
#define FONT_FILE_FOR_SIZE_INNER(SIZE) "engrave" #SIZE ".bmp"
#define FONT_FILE_FOR_SIZE(SIZE) FONT_FILE_FOR_SIZE_INNER(SIZE)
char *fontfile=FONT_FILE_FOR_SIZE(FONT_SIZE);
#undef FONT_FILE_FOR_SIZE_INNER
#undef FONT_FILE_FOR_SIZE

#define	CHARACTER_NUM 256
CBmps* cbmps_character;
char *characterfile="charabg.txt";

#define	OTHER_GRAPHICS_NUM 256
CBmps* cbmps_other;
char *otherfile="otherbg.txt";



#define BLOCKSTATE_NONE	0
#define BLOCKSTATE_FALLING	1
#define BLOCKSTATE_PREFALL	2
#define BLOCKSTATE_EXTINGUISHING	3
#define BLOCKSTATE_FALLFINISHED	4
#define BLOCKSTATE_EXTINGUISHED	5

typedef struct{
	int state;
	int type;
	int left;//for falling(dy)
	int lefttime;// for prefall wait time
	int done;
	int done_sub;

	int unsetlock;

	int destroycount;//for X block

	int shape;//for soft blocks

	int extinguishingframe;


	int player_dug;

}TBlockState;

#define GAME_STAGE_WIDTH 9
#define GAME_STAGE_HEIGHT (100+10)
#define STAGE_START_Y	5
TBlockState gamestage[GAME_STAGE_WIDTH][GAME_STAGE_HEIGHT];

#define STAGE_WIDTH 9


/**
game variables
**/

int my_x;
int my_y;

int dig;
int vx;
int vy;

int dig_graphic;

#define PENALTY_FRAMES	60
int penaltyframe;
int penaltybgnum;

int movingframe=0;


int fallingframe=0;


int direction;
#define DIR_DOWN	0
#define DIR_UP	1
#define DIR_LEFT	2
#define DIR_RIGHT	3
#define DIR_NONE	4


#define SCOREMEMBER 10
typedef struct{
	char name[SCOREMEMBER+1][4];
	int time[SCOREMEMBER+1];
	int depth[SCOREMEMBER+1];
	int score[SCOREMEMBER+1];

}THighScore;

THighScore highscoredata;
THighScore fasttimedata;

THighScore *entrydata;

char *scorefile="drill.scr";
char *highscoreformat="%3s\n%10d\n%10d\n%10d\n";
char *fasttimefile="drilltime.scr";


typedef struct{

	int x;
	int y;
	SDL_Surface *bmp;
	unsigned int clock;

}CTileScroll;

CTileScroll tscroll;



//キーを押しつづけてる？
int repeat_x;
int repeat_y;
int climbing;
int destroyed=0;
int dig_repeat=0;


//score

int my_depth;
int my_score;
int my_air;

int got_air;

int my_dead;
int my_deadcount;

int my_clear;
int my_clearcount;

int airdowncount;
int airdownspeed;
int airplus;

int my_fps;

int my_time;

//int scorecount;
int scoreplus;
int scorerest;

int lap_showing;
int lapcount;
char lapstring[1024];

int airminus;
int airminuscount;



/** game setting**/
int setting_fullscreen=0;
char setting_bitmapdir[1024]="./";
int setting_climbwait;
int setting_airdecreasewait;
int setting_defaultFPS;
int setting_fpsincreasespeed;
int setting_airinterval;
int setting_airdownspeed;
int setting_joyconfirm;
int setting_joycancel;
int setting_joyaxismax;

int setting_joyenabled;
int setting_joysticknumber;
char setting_playerdir[1024]="./player/";

/* joysticj control */
//SDL_Joystick *joystick=NULL;
CInput *gameinput=NULL;

/**

name entry
**/
char nameentry_moji[] =
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz .";
int nameentry_x;
int nameentry_vx;
int nameentry_x_wait;
int name_x;
int space_repeat;
int bs_repeat;

int entry_number;
int entry_number_time;



void draw(void);
void initialize(void);
void finalize(void);
void load_graphic(void);
void set_stage(int,int,int);
void set_stage_startingpoint(void);
void game_ready(void);
void draw_me(void);

int mainloop();
void move(void);
int keyread(void);

int erase_block(void);
void clear_blockflag(void);
void destroy(int ,int );
void search(int ,int ,int );

void blockprocess(void);
void prefallcheck(void );
void setprefall(int ,int ,int ,int);
void search_fall(int ,int ,int ,int *,int *);
void search_number(int ,int ,int ,int *);

void draw_screen(void);
void stage_clear(void);
void drawback(void);
void other_move(void);
void draw_air(void);
int atarihantei(void);
void self_destroy(int x,int y);

void unsetprefall(int x,int y,int type);
void leftcheck(int x,int y,int type,int *checkleft,int *);
void setleft(int x,int y,int type,int left);

void unsetprefall(int x,int y,int type);

int hanabiset(int x,int y,int n);
void drawhanabi(void);
void clearhanabi(void);

void drawTilescroll(void);
void moveTilescroll(void);
int title(void);
int highscore(void);

int THighScoreLoad(THighScore *p,char *filename);
int THighScoreSave(THighScore *p,char *filename);
void THighScoreAdd(THighScore *p,char *name,int time,int depth,int score);
void THighScoreSortByTime(THighScore *p);
void THighScoreSortByScore(THighScore *p);
void THighScoreSortByDepth(THighScore *p);
void THighScoreSwap(THighScore *p,int i,int j);

void erase_check_recursive(int x,int y,int type,int *answer,int *number);
void seterase_recursive(int x,int y,int type);
void erase_check(void);

void gameover(void);
void draw_other(void);

void gameclear(void);
void load_setting(void);

void nameentry(void);
void draw_nameentry(void);
int move_nameentry(void);
int init_nameentry(int score,int depth,int playtime);

void joy_init(void);
void joy_final(void);

void unsetprefallfinished(int x,int y,int type);


int BlitForBlock(SDL_Surface *p,SDL_Surface *dest,int num,int x,int y);
void set_shape(void);


int main(int argc, char **argv) {
	(void)argc;
	(void)argv;

	load_setting();

	initialize();
	load_graphic();

	THighScoreLoad(&highscoredata,scorefile);
	THighScoreLoad(&fasttimedata,fasttimefile);

	while (1) {
		CWavsPlayMusicStyle(wavs, 0);
		if (!title()) break;

		CWavsPlayMusicStyle(wavs, 1);
		game_ready();
		if (!mainloop()) break;
	}

	THighScoreSave(&highscoredata, scorefile);
	THighScoreSave(&fasttimedata, fasttimefile);
	finalize();

	return 0;
}


void load_setting(void){

	FILE *fp;
	char buf[4096];
	char *ignore="#\r\n";
	char *inifile="drill.ini";
	char * p ,*q;

	fp=fopen(inifile,"rb");
	if(NULL==fp){
		fprintf(stderr,"ERROR:cant open %s\n",inifile);
		return;
	}


	setting_fullscreen=0;
	strcpy(setting_bitmapdir,"./system/bmp/");
	setting_defaultFPS=60;
	setting_airdecreasewait=50;
	setting_climbwait=9;
	setting_fpsincreasespeed=2;
	setting_airinterval=10;
	setting_airdownspeed=3;
	setting_joyconfirm=0;
	setting_joycancel=1;
	setting_joyaxismax=10000;
	setting_joyenabled=1;
	setting_joysticknumber=0;
	do{
		if(NULL==(fgets(buf,4096,fp)))break;
		if(NULL!=strchr(ignore,*buf))continue;

		p=strtok(buf,"\t\r\t\n #");
		q=strtok(NULL,"\t\r\t\n #");

		if(!strcmp(p,"FULLSCREEN")){
			if(!strcmp(q,"YES"))setting_fullscreen=1;else setting_fullscreen=0;
		}
		if(!strcmp(p,"DEFAULT_FPS")){
			if(q)setting_defaultFPS=atoi(q);
		}
		if(!strcmp(p,"FPS_INCREASE_SPEED")){
			if(q)setting_fpsincreasespeed=atoi(q);
		}
		if(!strcmp(p,"AIR_DECREASE_WAIT")){
			if(q)setting_airdecreasewait=atoi(q);
		}
		if(!strcmp(p,"AIR_DOWN_SPEED")){
			if(q)setting_airdownspeed=atoi(q);
		}
		if(!strcmp(p,"CLIMB_WAIT")){
			if(q)setting_climbwait=atoi(q);
		}
		if(!strcmp(p,"AIR_INTERVAL")){
			if(q)setting_airinterval=atoi(q);
		}
		if(!strcmp(p,"JOY_CONFIRM")){
			if(q)setting_joyconfirm=atoi(q);
		}
		if(!strcmp(p,"JOY_CANCEL")){
			if(q)setting_joycancel=atoi(q);
		}
		if(!strcmp(p,"JOY_CANCEL")){
			if(q)setting_joyaxismax=atoi(q);
		}
		if(!strcmp(p,"JOY_ENABLED")){
			if(!strcmp(q,"YES"))setting_joyenabled=1;else setting_joyenabled=0;
		}

		if(!strcmp(p,"JOYSTICK_NUMBER")){
			if(q)setting_joysticknumber=atoi(q);
		}
		if(!strcmp(p,"BITMAPSDIR")){
			if(q==NULL){
				strcpy(setting_bitmapdir,"./bmp/");
			}else{
				strcpy(setting_bitmapdir,q);
			}
		}
		if(!strcmp(p,"PLAYERDIR")){
			if(q==NULL){
				strcpy(setting_playerdir,"./player/");
			}else{
				strcpy(setting_playerdir,q);
			}
		}
	}while(1);
	fclose(fp);


}

void gameover(void){


	my_time=1000000;//for no clear time
	printf("%d %d %d\n",my_score,my_depth*100+my_y/TILE_SIZE-STAGE_START_Y+1,my_time);

	if(init_nameentry(my_score,my_depth*100+my_y/TILE_SIZE-STAGE_START_Y+1,my_time) < 0);else
		nameentry();


}
void gameclear(void){

	printf("%d %d %d\n",my_score,my_depth*100+my_y/TILE_SIZE-STAGE_START_Y+1,my_time);

	if(init_nameentry(my_score,my_depth*100+my_y/TILE_SIZE-STAGE_START_Y+1,my_time) < 0);else
		nameentry();


}

/*死の判定、非0で死*/
int atarihantei(void){

	int mapx,mapy;
	TBlockState *p;
	int deltax,deltay;
	deltax=my_x%TILE_SIZE;
	deltay=my_y%TILE_SIZE;
	mapx=my_x/TILE_SIZE;
	mapy=my_y/TILE_SIZE;
	if(my_x%TILE_SIZE>=TILE_SIZE/2)mapx++;
	if(my_y%TILE_SIZE>=TILE_SIZE/2)mapy++;
	p=&gamestage[mapx][mapy];
	if(p->type==AIR_BLOCK && p->left < TILE_SIZE/2){
		p->type=NO_BLOCK;
		p->state=BLOCKSTATE_NONE;
		p->player_dug=1;
		airplus+=22;
		got_air++;
		scorerest+=got_air*10;
		scoreplus+=got_air/2+1;

		CWavsPlay(wavs,11);

	}else
	if(p->type!=NO_BLOCK && p->state!=BLOCKSTATE_EXTINGUISHING && p->state!=BLOCKSTATE_EXTINGUISHED){
		if(p->left < TILE_SIZE/2){
		my_dead=1;my_deadcount=0;vx=0;
		return 1;
		}
	}else
	if(mapx>0 && my_x%TILE_SIZE==TILE_SIZE/2 &&
		gamestage[mapx-1][mapy].type!=NO_BLOCK &&
		gamestage[mapx-1][mapy].type!=AIR_BLOCK &&
		gamestage[mapx-1][mapy].state!=BLOCKSTATE_EXTINGUISHING &&
		gamestage[mapx-1][mapy].state!=BLOCKSTATE_EXTINGUISHED
		){
		if(p->left < TILE_SIZE/2){
			my_dead=1;my_deadcount=0;vx=0;
			return 1;
		}
	}
	if(my_air==0){my_dead=1;my_deadcount=0;vx=0;return 2;}
	//補正
	if(deltay==0 && deltax <= (5*TILE_SIZE)/12 && mapx<STAGE_WIDTH &&
		gamestage[mapx+1][mapy].type!=NO_BLOCK &&
		gamestage[mapx+1][mapy].type!=AIR_BLOCK &&
		gamestage[mapx+1][mapy].state!=BLOCKSTATE_EXTINGUISHING &&
		gamestage[mapx+1][mapy].state!=BLOCKSTATE_EXTINGUISHED
		){
		my_x=mapx*TILE_SIZE;
		if(deltax > TILE_SIZE/4){
			penaltyframe=PENALTY_FRAMES;
			if(direction==DIR_LEFT)
				penaltybgnum=43;//left escape
			else
				penaltybgnum=34;
		}
	}
	if(deltay==0 && deltax >= (7*TILE_SIZE)/12 && mapx>0 &&
		gamestage[mapx-1][mapy].type!=NO_BLOCK &&
		gamestage[mapx-1][mapy].type!=AIR_BLOCK &&
		gamestage[mapx-1][mapy].state!=BLOCKSTATE_EXTINGUISHING &&
		gamestage[mapx-1][mapy].state!=BLOCKSTATE_EXTINGUISHED
		){
		my_x=(mapx)*TILE_SIZE;
		if(deltax < (TILE_SIZE*3)/4){
			penaltyframe=PENALTY_FRAMES;
			if(direction==DIR_LEFT)
				penaltybgnum=44;//left escape
			else
				penaltybgnum=33;
		}
	}

	return 0;
}




void other_move(void){

	if(my_y>=(STAGE_START_Y-1)*TILE_SIZE)airdowncount++;
	if(airdowncount>airdownspeed){
		airdowncount=0;
		my_air--;
	}
	if(airplus>0){
		airplus--;
		my_air++;
	}
	if(my_air<=0)my_air=0;
	if(my_air>=100)my_air=100;


	if(scoreplus>0){
		if(scorerest>scoreplus){

			my_score+=scoreplus;
			scorerest-=scoreplus;
		}else{
			my_score+=scorerest;
			scorerest=0;
			scoreplus=0;
		}
	}


}

void move(void){
	int mapx,mapy;
	int digx,digy;
	TBlockState *q;

	erase_block();
	erase_check();

	if(dig_graphic){
		dig_graphic++;
		if(dig_graphic>=9)dig_graphic=0;
	}


	if(climbing){
		my_y+=vy;
		if(my_y<0)my_y=0;
		if(my_y%TILE_SIZE==0){
//			my_x+=vx*7;//for walkspeed=4
//			my_x+=vx*5;//for walkspeed=6
			if(vx>0)my_x+=(7*TILE_SIZE)/12;else my_x-=(7*TILE_SIZE)/12;
			climbing=0;
			repeat_x=0;
		}else vy=-WALKSPEED_Y;
		return;
	}else{
		vx=0;
		vy=0;
		dig=0;
		if(!my_dead && !my_clear && penaltyframe==0){
			CInputUpdate(gameinput,0);
		}else{
			CInputUpdate(gameinput,1);
		}
		if ( gameinput->button[BUTTON_UP]){direction=DIR_UP;}
		if ( gameinput->button[BUTTON_DOWN]) {direction=DIR_DOWN;}
		if ( gameinput->button[BUTTON_RIGHT]) {vx= WALKSPEED;direction=DIR_RIGHT;}
		if ( gameinput->button[BUTTON_LEFT]) {vx=-WALKSPEED;direction=DIR_LEFT;}

		if ( gameinput->button[BUTTON_0] && direction!=DIR_NONE && my_y%TILE_SIZE==0){
			if(dig_repeat)dig=0;else{
				dig=1;
				dig_graphic=1;
				dig_repeat=1;
			}
		}else{
			dig_repeat=0;
		}
	}
	if(vx==0)repeat_x=0;

	if(penaltyframe>0){
		penaltyframe--;
	}

	mapx=my_x/TILE_SIZE;
	mapy=my_y/TILE_SIZE;
	if(my_x%TILE_SIZE >= TILE_SIZE/2)mapx++;
	if(my_y%TILE_SIZE >= TILE_SIZE/2)mapy++;
	q=&gamestage[mapx][mapy+1];

	if(my_y%TILE_SIZE==0){

		if(q->type==NO_BLOCK||q->type==AIR_BLOCK||q->state==BLOCKSTATE_EXTINGUISHING){
			vx=0;
			vy=WALKSPEED_Y;
			dig=0;
			my_x=mapx*TILE_SIZE;
			penaltyframe=0;
		}
	}else{
		vx=0;
		vy=WALKSPEED_Y;
		dig=0;
		penaltyframe=0;
	}


	if(my_x%TILE_SIZE >= TILE_SIZE/4 && my_x%TILE_SIZE < (TILE_SIZE*3)/4){
		if(direction==DIR_RIGHT||direction==DIR_LEFT)dig=0;
	}

	if(vx!=0 && my_x%TILE_SIZE==0){

		mapx=my_x/TILE_SIZE;
		mapy=my_y/TILE_SIZE;

		if(vx>0)mapx++;
		if(vx<0)mapx--;

		if(mapx<0)vx=0; else
		if(mapx>=STAGE_WIDTH)vx=0; else
		if(gamestage[mapx][mapy].type==AIR_BLOCK){
			;
		}else if(
			gamestage[mapx][mapy].state==BLOCKSTATE_EXTINGUISHING||
			gamestage[mapx][mapy].state==BLOCKSTATE_EXTINGUISHED){
			;
		}else if(gamestage[mapx][mapy].type!=NO_BLOCK){

			if(gamestage[my_x/TILE_SIZE][mapy-1].type==NO_BLOCK &&
				gamestage[my_x/TILE_SIZE][mapy-1].state!=BLOCKSTATE_EXTINGUISHING &&
				(gamestage[mapx][mapy-1].type==NO_BLOCK||gamestage[mapx][mapy-1].type==AIR_BLOCK)
				){
				repeat_x+=vx;
				if(repeat_x>WALKSPEED*setting_climbwait||repeat_x<-WALKSPEED*setting_climbwait){
					climbing=1;

					vy=-WALKSPEED_Y;
				}else vx=0;
			}else vx=0;

		}

	}

	if(dig){
		digx=my_x/TILE_SIZE;
		if(my_x%TILE_SIZE >= TILE_SIZE/2)digx++;
		digy=my_y/TILE_SIZE;
		if(direction==DIR_DOWN){
			digy++;
		}else
		if(direction==DIR_UP){
			digy--;
		}else
		if(direction==DIR_RIGHT){
			digx++;
		}else
		if(direction==DIR_LEFT){
			digx--;
		}
		destroyed=0;
		if(digx<0 || digx>=STAGE_WIDTH || digy<0 || digy>=GAME_STAGE_HEIGHT);else{
			if(gamestage[digx][digy].type!=NO_BLOCK &&
			gamestage[digx][digy].state!=BLOCKSTATE_FALLING &&
			gamestage[digx][digy].state!=BLOCKSTATE_EXTINGUISHING &&
			gamestage[digx][digy].state!=BLOCKSTATE_EXTINGUISHED

			){
				if(gamestage[digx][digy].type==CLEAR_BLOCK){
					stage_clear();
				}else{
					self_destroy(digx,digy);
					destroyed=1;
					repeat_x=0;
				}
			}
		}

	}

	if(my_y<0)vy=1;

	if(vx!=0)movingframe++;else movingframe=0;
	if(!climbing)my_x+=vx;
	if(vy>0)fallingframe++;else fallingframe=0;
	my_y+=vy;

	if(my_x<0)my_x=0;
	if(my_y<0)my_y=0;





	prefallcheck();
	blockprocess();



}

int keyread(void){

	SDL_Event ev;
	static int screennumber=0;
	SDLKey *key;
	char buf[1000];

	while(SDL_PollEvent(&ev)){
		switch(ev.type){
			case SDL_QUIT:
				return 1;
			case SDL_KEYDOWN: {
				key=&(ev.key.keysym.sym);

				if(*key==293){//F12
					sprintf(buf,"screenshot%04d.bmp",screennumber++);
					if(screennumber>9999)screennumber=0;
					SDL_SaveBMP(screen,buf);
				}

				if(*key==51){
					return 1;
				}
				if(*key==27){//ESC
					return 1;
				}
			}break;
		}
	}

	return 0;
}

void drawback(void){
	int gy,y;
	for(gy=-my_y%TILE_SIZE,y=0;y<11;++y,gy+=TILE_SIZE){
		CBmpsBlit(cbmps_character,screen,102,0,gy);
	}
}

int mainloop(){
	int y;

	char buf[4096];
	CTimeChangeFPS(&gametime,my_fps);

	set_shape();

	do{
		if(my_y==(STAGE_START_Y-1)*TILE_SIZE)CTimeReset(&gametime);
		set_shape();
		if(keyread())return 0;

		if(lap_showing){
			lapcount++;
			if(lapcount>300)lap_showing=0;
		}

		if(airminus){
			airminuscount++;
			if(airminuscount>80)airminus=0;
		}

		if(my_dead){
			my_deadcount++;
			if(my_deadcount>250){
				gameover();
				break;
			}
		}

		if(my_clear){
			my_clearcount++;
			if(my_clearcount>300){
				gameclear();
				break;
			}
		}

		move();
		other_move();
		if(!my_dead && !my_clear)atarihantei();

		if(!gametime.isDelay){

//			SDL_FillRect(screen,NULL,SDL_MapRGB(screen->format,0,0,0));
			drawback();
			draw();
			draw_me();

			draw_other();

			drawhanabi();

			draw_screen();
			draw_air();

			y=my_depth*100+my_y/TILE_SIZE-STAGE_START_Y+1;

			sprintf(buf,"%4d m.",y<0?0:y);
			CffontBlitxy(font,buf,screen,DEPTH_X,DEPTH_Y);

			sprintf(buf,"%6d0",my_score);
			CffontBlitxy(font,buf,screen,SCORE_X,SCORE_Y);

			sprintf(buf,"%3d%%",my_air);
			CffontBlitxy(font,buf,screen,AIR_NUM_X,AIR_NUM_Y);

			//SDL_Flip(screen);
      {
        if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
        int x, y;
        uint32_t *s = screen->pixels;
        uint32_t *d = ScreenSurface->pixels;
		for(uint8_t y2 = 0; y2 < 240; y2++, s += 160, d += 320) 
			memmove(d, s, 1280); // double-line fix by pingflood, 2018
        /*for(y=0; y<240; y++){
          for(x=0; x<160; x++){
            *d++ = *s++;
          }
          d+= 160;
        }*/
        if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
        SDL_Flip(ScreenSurface);
      }
//			SDL_UpdateRect(screen,0,0,0,0);
		}
		CTimeWait(&gametime);
	}while(1);
	return 1;
}

void draw_other(void){
	static unsigned int frame = 0;
	frame++;

	if (my_dead == 1) {
		if (my_deadcount > 20) {
			int x = (STAGE_WIDTH * TILE_SIZE - cbmps_character->bmp[70]->w) / 2;
			int y = (7 * TILE_SIZE - cbmps_character->bmp[70]->h) / 2;
			CBmpsBlit(cbmps_character, screen, 70, x, y);
		}
	}

	if (my_clear == 1) {
		if (my_clearcount > 20) {
			int x = (STAGE_WIDTH * TILE_SIZE - cbmps_character->bmp[72]->w) / 2;
			int y = (7 * TILE_SIZE - cbmps_character->bmp[72]->h) / 2;
			CBmpsBlit(cbmps_character, screen, 72, x, y);
		}
	}

	if (lap_showing) {
		if (lapcount % 40 < 25) {
			CffontBlitxy(font, lapstring, screen, 0, 0);
		}
	}

	if (my_y > TILE_SIZE * 90) {
		SDL_Surface *label = cbmps_character->bmp[120 + my_depth];
		int x = (STAGE_WIDTH * TILE_SIZE - label->w) / 2;
		int y = 104 * TILE_SIZE - my_y + (5 * TILE_SIZE - label->h) / 2;
		CBmpsBlit(cbmps_character, screen, 120 + my_depth, x, y);
	}

	if (my_air < 25 && (frame % 12) < 6) {
		int x = (STAGE_WIDTH * TILE_SIZE - cbmps_character->bmp[111]->w) / 2;
		int y = (4 * TILE_SIZE - cbmps_character->bmp[111]->h) / 2;
		CBmpsBlit(cbmps_character, screen, 111, x, y);
	}

	if (airminus) {
		int x = (STAGE_WIDTH * TILE_SIZE - cbmps_character->bmp[110]->w) / 2;
		int y = (14 * TILE_SIZE - cbmps_character->bmp[110]->h) / 2;
		CBmpsBlit(cbmps_character, screen, 110, x, y);
	}
}

void draw_air(void) {
	SDL_Surface *bar = cbmps_character->bmp[103];
	int width = (my_air * bar->w) / 100;
	SDL_Rect src = { 0, 0, width, bar->h };
	SDL_Rect dest = { AIR_BAR_X, AIR_BAR_Y, width, bar->h };
	if (my_air > 0) {
		SDL_BlitSurface(bar, &src, screen, &dest);
	}
}

void draw_me(void){

//	CBmpsBlit(cbmps_character,screen,0,my_x*48,3*48);
	int i,j;


	switch(direction){
		case DIR_RIGHT:i=30;break;
		case DIR_LEFT:i=40;break;
		case DIR_DOWN:i=10;break;
		case DIR_UP:i=20;break;
		default:
			assert(0);
			i=10;
	}
	if(dig_graphic==0 && movingframe>0){
		j=movingframe/2;
		j%=4;
		i+=j;
		i+=5;
	}


	i+=dig_graphic/3;//frameeach
	if(penaltyframe>0)i=penaltybgnum;

	if(fallingframe>20){

		i=110+(fallingframe%12)/6;
	}


	if(my_dead==1){
		i=100;
		if(my_deadcount>30)i=101;
	}

	CBmpsBlit(playerbmps,screen,i,my_x-TILE_SIZE/4,4*TILE_SIZE-TILE_SIZE/4);
}

void draw_screen(void){

	CBmpsBlit(cbmps_character, screen, 100, SCREEN_HEIGHT - TILE_SIZE, 0);

}

void stage_clear(void){

	int colors[]={4,3,4,4,2,4,3,3,4,4,1};
//	int colors[]={1,1,1,1,1,4,3,2,3,4,1};
//	int percent[]={50,52,54,56,58,60,63,64,66,68,10};
	int percent[]={50,55,60,65,30,65,70,75,80,80,10};
	int blockstyle[]={1,3,3,4,1,4,4,3,4,4,1};

	int i,x,y;

	for (i = 0, y = my_y / TILE_SIZE - 4; i < 9; i++, y++) {
		for (x = 0; x < 9; x++) {
			if (gamestage[x][y].type != NO_BLOCK) {
				hanabiset(x, i - 3, 15);
			}
		}
	}

	my_depth++;
	if(my_depth>=10){my_clear=1;my_clearcount=0;}
	set_stage(colors[my_depth],percent[my_depth],blockstyle[my_depth]);
	climbing=0;



	my_y=-2*TILE_SIZE;
	airdownspeed-=setting_airdownspeed;
	airdowncount=0;
	scorerest+=my_depth*100;
	scoreplus+=my_depth*5;
	my_fps+=setting_fpsincreasespeed;

	my_time+=gametime.clock/10;

	fallingframe=20;

	sprintf(lapstring,"%2d00m Passing %02d'%02d'%02d",
		my_depth,
		gametime.clock/6000/10,
		(gametime.clock/100/10)%60,
		(gametime.clock/10)%100
		);
	lap_showing=1;
	lapcount=0;
//	CTimeReset(&gametime);
	CTimeChangeFPS(&gametime,my_fps);

}

void game_ready(void){

	my_x=4*TILE_SIZE;
//	my_y=(STAGE_START_Y-1)*TILE_SIZE;
//	my_y=0;
	my_y=-2*TILE_SIZE;

	set_stage(4,50,1);
//	set_stage(3,95);

	set_stage_startingpoint();
	direction=DIR_DOWN;

	climbing=0;
	airdownspeed=setting_airdecreasewait;
	airdowncount=0;
	got_air=0;
	my_score=0;
	scoreplus=0;
	scorerest=0;
	my_air=100;
	my_depth=0;
//	my_depth=9;
	my_dead=0;
	my_deadcount=0;
	clearhanabi();
	my_clear=0;
	my_clearcount=0;
	my_fps=setting_defaultFPS;
	my_time=0;

	lap_showing=0;

	dig_graphic=0;

	penaltybgnum=0;
	penaltyframe=0;
	fallingframe=60;
	CTimeChangeFPS(&gametime,my_fps);
	CTimeReset(&gametime);
}

void initialize(void){

	srand(time(NULL));

	if(setting_fullscreen)
		screen=CScreenInitDefaultHW();
	else
		screen=CScreenInitDefault();

	cbmps_blocks=CBmpsInit(BLOCK_NUM);
	cbmps_character=CBmpsInit(CHARACTER_NUM);
	cbmps_other=CBmpsInit(OTHER_GRAPHICS_NUM);



	playerbmps=CBmpsInit(PLAYER_NUM);


	#define FONT_INIT_FUNC_FOR_SIZE_INNER(SIZE) CffontInitDefault ## SIZE
	#define FONT_INIT_FUNC_FOR_SIZE(SIZE) FONT_INIT_FUNC_FOR_SIZE_INNER(SIZE)
	font=FONT_INIT_FUNC_FOR_SIZE(FONT_SIZE)(fontfile);
	#undef FONT_INIT_FUNC_FOR_SIZE_INNER
	#undef FONT_INIT_FUNC_FOR_SIZE

	CAudioInitDefault();
	wavs=CWavsInit(WAVMAX);

	joy_init();




}

void finalize(void){

	CBmpsFree(cbmps_blocks);
	CBmpsFree(cbmps_character);
	CBmpsFree(cbmps_other);


	CBmpsFree(playerbmps);
	playerbmps=NULL;


	CffontFree(font);

	CWavsFree(wavs);
	CAudioClose();
	joy_final();

}


//and sound
void load_graphic(void){

	char filename[1024];

	CBmpsLoadFromFileWithDir(cbmps_blocks,blockfile,setting_bitmapdir);
	CBmpsLoadFromFileWithDir(cbmps_character,characterfile,setting_bitmapdir);
	CBmpsLoadFromFileWithDir(cbmps_other,otherfile,setting_bitmapdir);


	sprintf(filename,"%s%s",setting_playerdir,playerbmpsfile);
	CBmpsLoadFromFileWithDir(playerbmps,filename,setting_playerdir);


	if(setting_fullscreen){

		CBmpsConvert(cbmps_blocks);
		CBmpsConvert(cbmps_character);
		CBmpsConvert(cbmps_other);
	}
	tscroll.bmp=cbmps_character->bmp[52];
	tscroll.x=0;
	tscroll.y=0;
	tscroll.clock=0;


	CWavsLoadFromFileWithDir(wavs,wavfile,wavdir);


}


void set_stage_startingpoint(void){

	int i;
	int num_xy;
	TBlockState *p;

	const struct{
		int x;
		int y;
		int n;
	}xy[]={
		{0,0,HARD_BLOCK},{1,0,HARD_BLOCK},{2,0,HARD_BLOCK},{3,0,HARD_BLOCK},{4,0,0},{5,0,HARD_BLOCK},{6,0,HARD_BLOCK},{7,0,HARD_BLOCK},{8,0,HARD_BLOCK},
		{0,1,HARD_BLOCK},{1,1,HARD_BLOCK},{2,1,HARD_BLOCK},{3,1,HARD_BLOCK},{4,1,1},{5,1,HARD_BLOCK},{6,1,HARD_BLOCK},{7,1,HARD_BLOCK},{8,1,HARD_BLOCK},
		{0,2,HARD_BLOCK},{1,2,HARD_BLOCK},{2,2,2},{3,2,2},{4,2,2},{5,2,2},{6,2,2},{7,2,HARD_BLOCK},{8,2,HARD_BLOCK},
		{0,3,HARD_BLOCK},{1,3,HARD_BLOCK},{2,3,3},{3,3,3},{4,3,3},{5,3,3},{6,3,3},{7,3,HARD_BLOCK},{8,3,HARD_BLOCK},
		{0,4,0},{1,4,0},{2,4,0},{3,4,0},{4,4,0},{5,4,0},{6,4,0},{7,4,0},{8,4,0},
	};
	num_xy=sizeof(xy)/sizeof(xy[0]);

	for(i=0;i<num_xy;++i){
		p=&(gamestage[xy[i].x][xy[i].y+STAGE_START_Y]);
		p->type=xy[i].n;
		p->state=BLOCKSTATE_NONE;
		p->destroycount=0;
	}

}
void set_stage(int number,int percentage,int blockstyle){

	int x,y;
	TBlockState *p;
	int range;

	/*通常のブロックを敷き詰める*/
	for(y=0;y<GAME_STAGE_HEIGHT;++y){
		for(x=0;x<GAME_STAGE_WIDTH;++x){
			p=&(gamestage[x][y]);

			p->type=((double)rand()/RAND_MAX)*number;
			if(p->type>=4)p->type=0;
			p->destroycount=0;
			p->state=BLOCKSTATE_NONE;
			p->left=0;
			p->lefttime=0;
			p->shape=0;
		}
	}
	/*硬いブロックをばら撒く*/
	for(y=10;y<GAME_STAGE_HEIGHT-6;++y){

		if(((double)rand()/RAND_MAX)*100 > percentage)continue;
		range=((double)rand()/RAND_MAX)*4;

		x=((double)rand()/RAND_MAX)*GAME_STAGE_WIDTH;

		while(range>0){
			range--;
			if(x+range>=GAME_STAGE_WIDTH)continue;
			p=&(gamestage[x+range][y]);
			p->type=HARD_BLOCK;
			p->state=BLOCKSTATE_NONE;
			p->left=0;
			p->destroycount=0;
			p->player_dug=0;
		}
	}

	/*airの設置及び周りに硬いブロックを配置*/
	for(y=11;y<GAME_STAGE_HEIGHT-10;y+=(setting_airinterval+my_depth)){
		x=((double)rand()/RAND_MAX)*(GAME_STAGE_WIDTH-2)+1;
		p=&(gamestage[x][y]);
		p->type=AIR_BLOCK;
		p->state=BLOCKSTATE_NONE;
		p->destroycount=0;
		//添え字チェックはしなくても良いようにAirを配置

		if(blockstyle==4)gamestage[x-1][y].type=HARD_BLOCK;
		if(blockstyle==4)gamestage[x+1][y].type=HARD_BLOCK;
		if(blockstyle==4||blockstyle==1||blockstyle==3)gamestage[x][y-1].type=HARD_BLOCK;
		if(blockstyle==4||blockstyle==2||blockstyle==3)gamestage[x][y+1].type=HARD_BLOCK;



	}

	/*クリアブロックを配置*/
	for(y=95+STAGE_START_Y;y<100+STAGE_START_Y;++y){
		for(x=0;x<GAME_STAGE_WIDTH;++x){
			p=&(gamestage[x][y]);

			p->type=CLEAR_BLOCK;
			p->state=BLOCKSTATE_NONE;
			p->left=0;
		}
	}
	/*スタート余白を設置*/

	for(y=0;y<STAGE_START_Y;++y){
		for(x=0;x<GAME_STAGE_WIDTH;++x){
			p=&(gamestage[x][y]);

			p->type=NO_BLOCK;
			p->state=BLOCKSTATE_NONE;
			p->left=0;
		}
	}

}

void draw(void){


	int x,y,gy;

	int delta_y;



	int vibration[]={
		-1,-2,-3,-4,-3,-2,-1,0,
		1,2,3,4,3,2,1,0,
		-1,-2,-3,-4,-3,-2,-1,0,
		1,2,3,4,3,2,1,0,
		-1,-2,-3,-4,-3,-2,-1,0,
		1,2,3,4,3,2,1,0,
		-1,-2,-3,-4,-3,-2,-1,0,
		1,2,3,4,3,2,1,0,
		-1,-2,-3,-4,-3,-2,-1,0,
		1,2,3,4,3,2,1,0,
	};





	TBlockState *p;

	int alpha=0;
	delta_y=my_y%TILE_SIZE;

	for(gy=-delta_y,y=my_y/TILE_SIZE-4;y<=my_y/TILE_SIZE+7;++y,gy+=TILE_SIZE){
		if(y<0||y>=GAME_STAGE_HEIGHT)continue;
		for(x=0;x<STAGE_WIDTH;x++){
		alpha=0;
			p=&(gamestage[x][y]);
			if(p->type==NO_BLOCK)continue;

			if(p->type==HARD_BLOCK){
				alpha=p->destroycount;
//				CBmpsBlit(cbmps_blocks,screen,p->type+alpha,x*TILE_SIZE+p->lefttime%6-3,gy-p->left);
			}else if(p->type==AIR_BLOCK){
//				CBmpsBlit(cbmps_blocks,screen,p->type+alpha,x*TILE_SIZE+p->lefttime%6-3,gy-p->left);


			}else{
				switch(p->state){
					case BLOCKSTATE_FALLING:
					BlitForBlock(cbmps_blocks->bmp[p->type],screen,p->shape,x*TILE_SIZE,gy-p->left);
					break;

					case BLOCKSTATE_PREFALL:
					if(p->lefttime<66){
						BlitForBlock(cbmps_blocks->bmp[p->type],screen,p->shape,x*TILE_SIZE+vibration[p->lefttime],gy);
					}else{
						BlitForBlock(cbmps_blocks->bmp[p->type],screen,p->shape,x*TILE_SIZE,gy);
					}
					break;

					case BLOCKSTATE_EXTINGUISHING:
						CBmpsBlit(cbmps_character,screen,109-p->extinguishingframe/4,x*TILE_SIZE,gy);
					break;

					case BLOCKSTATE_EXTINGUISHED:
						CBmpsBlit(cbmps_character,screen,109,x*TILE_SIZE,gy);
					break;


					default:
				BlitForBlock(cbmps_blocks->bmp[p->type],screen,p->shape,x*TILE_SIZE,gy);
				}


				continue;


			}

			switch(p->state){
				case BLOCKSTATE_FALLING:
				CBmpsBlit(cbmps_blocks,screen,p->type+alpha,x*TILE_SIZE,gy-p->left);
				break;

				case BLOCKSTATE_PREFALL:
				if(p->lefttime<66){
					CBmpsBlit(cbmps_blocks,screen,p->type+alpha,x*TILE_SIZE+vibration[p->lefttime],gy);
				}else{
					CBmpsBlit(cbmps_blocks,screen,p->type+alpha,x*TILE_SIZE,gy);
				}
				break;

				case BLOCKSTATE_EXTINGUISHING:
					CBmpsBlit(cbmps_character,screen,109-p->extinguishingframe/4,x*TILE_SIZE,gy);
				break;

				case BLOCKSTATE_EXTINGUISHED:
					CBmpsBlit(cbmps_character,screen,109,x*TILE_SIZE,gy);
				break;
				default:
				CBmpsBlit(cbmps_blocks,screen,p->type+alpha,x*TILE_SIZE,gy);
			}

		}
	}
}


//unsetlock もクリア
void clear_blockflag(void){
	int x,y;
	TBlockState *p;
	for(y=0;y<GAME_STAGE_HEIGHT;++y)
	for(x=0;x<GAME_STAGE_WIDTH;++x){
		p=&(gamestage[x][y]);
		p->done=0;
		p->unsetlock=0;

	}
}

void clear_blockflag_sub(void){
	int x,y;
	TBlockState *p;
	for(y=0;y<GAME_STAGE_HEIGHT;++y)
	for(x=0;x<GAME_STAGE_WIDTH;++x){
		p=&(gamestage[x][y]);
		p->done_sub=0;

	}
}


//ブロックを消すときに同時に消すブロックをサーチ
void search(int x,int y,int type){
	TBlockState *p;

	p=&(gamestage[x][y]);
	if(p->done)return;
	if(p->type!=type)return;
	if(p->state==BLOCKSTATE_FALLING)return;
	p->done=1;
	p->state = BLOCKSTATE_EXTINGUISHING;
	p->player_dug=1;
//	p->extinguishingframe = 15;
	p->extinguishingframe = 8;
	if(x>0)search(x-1,y,type);
	if(y>0)search(x,y-1,type);
	if(x<GAME_STAGE_WIDTH-1)search(x+1,y,type);
	if(y<GAME_STAGE_HEIGHT-1)search(x,y+1,type);
}

void self_destroy(int x,int y){

	TBlockState *p;
	clear_blockflag();
	p=&(gamestage[x][y]);
	if(p->type==HARD_BLOCK){

		CWavsPlay(wavs,13);

		p->destroycount++;
		if(p->destroycount>=5){
			p->state=BLOCKSTATE_EXTINGUISHING;
//			p->extinguishingframe=15;
			p->extinguishingframe=8;
			my_air-=20;
			airminus=1;airminuscount=0;
//			erase_block();
			p->player_dug=1;


			scoreplus+=1;
			scorerest+=1;
//			my_score+=1;
		}
		return;
	}
	destroy(x,y);
	scoreplus++;
	scorerest++;
//	my_score++;
}
void destroy(int x,int y){

	TBlockState *p;
	clear_blockflag();
	p=&(gamestage[x][y]);
	if(p->type==AIR_BLOCK)return;
	if(p->state==BLOCKSTATE_EXTINGUISHING) return;
	search(x,y,p->type);
//	erase_block();

	CWavsPlay(wavs,12);

}
int erase_block(void){
	TBlockState *p;
	int x,y;
	int res=0;
	for(y=0;y<GAME_STAGE_HEIGHT;++y)
	for(x=0;x<GAME_STAGE_WIDTH;++x){

		p=&(gamestage[x][y]);
		if(p->state == BLOCKSTATE_EXTINGUISHED){
			res=1;
			p->state=BLOCKSTATE_NONE;
			p->type=NO_BLOCK;


//			hanabiset(x,y);

		}
	}
	return res;
}


//check and return whether these blocks can fall
//checksheat is 1 when calling search
//4つ以上で消えるときのカウント
void search_number(int x,int y,int type,int *number){

	TBlockState *p;

	p=&gamestage[x][y];
	if(p->done)return;
	if(p->type != type)return;
	p->done=1;
	if(p->state==BLOCKSTATE_FALLING)return;
	(*number)++;

	if(x>0)search_number(x-1,y,type,number);
	if(x<GAME_STAGE_WIDTH-1)search_number(x+1,y,type,number);
	if(y>0)search_number(x,y-1,type,number);
	if(y<GAME_STAGE_HEIGHT-1)search_number(x,y+1,type,number);
}





//
void search_fall(int x,int y,int type,int *checksheat,int *checkleft){

	TBlockState *p,*q;
	p=&gamestage[x][y];

	if(p->done)return;
	if(p->type != type)return;
	p->done=1;
	if(p->state==BLOCKSTATE_FALLING)return;
	q=&gamestage[x][y+1];
	if(q->type != NO_BLOCK && q->type != type){
		if(q->state!=BLOCKSTATE_PREFALL && q->state!=BLOCKSTATE_FALLING)*checksheat&=0;
	}else{

			;
	}



	if(p->state==BLOCKSTATE_NONE)p->left=PREFALLTIME;
	if(q->state==BLOCKSTATE_PREFALL)p->left=q->left;
	if(p->left>*checkleft)*checkleft=p->left;

	if(x>0)search_fall(x-1,y,type,checksheat,checkleft);
	if(x<GAME_STAGE_WIDTH-1)search_fall(x+1,y,type,checksheat,checkleft);
	if(y>0)search_fall(x,y-1,type,checksheat,checkleft);
	if(y<GAME_STAGE_HEIGHT-1)search_fall(x,y+1,type,checksheat,checkleft);


}

void setprefall(int x,int y,int type,int left){
	TBlockState *p=&gamestage[x][y];

	if(p->done_sub)return;
	if(p->type != type)return;
	p->done_sub=1;

	if(p->state==BLOCKSTATE_FALLING)return;

/*
	p->state=BLOCKSTATE_FALLING;
	p->left=48;
*/


	p->lefttime=left;
	/*
	p->lefttime=PREFALLTIME;
	if(p->state==BLOCKSTATE_FALLFINISHED)p->lefttime=1;
	if(q->state==BLOCKSTATE_PREFALL) p->lefttime=q->lefttime;
	*/
	p->state=BLOCKSTATE_PREFALL;

	if(x>0)setprefall(x-1,y,type,left);
	if(x<GAME_STAGE_WIDTH-1)setprefall(x+1,y,type,left);
	if(y>0)setprefall(x,y-1,type,left);
	if(y<GAME_STAGE_HEIGHT-1)setprefall(x,y+1,type,left);

}
void unsetprefall(int x,int y,int type){
	TBlockState *p=&gamestage[x][y];

	if(p->done)return;
	if(p->type != type)return;
	p->done=1;
	if(p->state==BLOCKSTATE_FALLING)return;
	if(p->state==BLOCKSTATE_EXTINGUISHED||p->state==BLOCKSTATE_EXTINGUISHING)return;
	p->state=BLOCKSTATE_NONE;
	p->lefttime=PREFALLTIME;
	p->unsetlock=1;
	if(p->type==AIR_BLOCK)return;

	if(x>0)unsetprefall(x-1,y,type);
	if(x<GAME_STAGE_WIDTH-1)unsetprefall(x+1,y,type);
	if(y>0)unsetprefall(x,y-1,type);
	if(y<GAME_STAGE_HEIGHT-1)unsetprefall(x,y+1,type);

}
void unsetprefallfinished(int x,int y,int type){
	TBlockState *p=&gamestage[x][y];

	if(p->done)return;
	if(p->type != type)return;
	p->done=1;
	if(p->state==BLOCKSTATE_FALLING)return;
	if(p->state==BLOCKSTATE_EXTINGUISHED||p->state==BLOCKSTATE_EXTINGUISHING)return;
	p->state=BLOCKSTATE_FALLFINISHED;
	p->lefttime=PREFALLTIME;
	p->unsetlock=1;
	if(p->type==AIR_BLOCK)return;

	if(x>0)unsetprefallfinished(x-1,y,type);
	if(x<GAME_STAGE_WIDTH-1)unsetprefallfinished(x+1,y,type);
	if(y>0)unsetprefallfinished(x,y-1,type);
	if(y<GAME_STAGE_HEIGHT-1)unsetprefallfinished(x,y+1,type);

}



//連鎖チェック
void erase_check_recursive(int x,int y,int type,int *answer,int *number){
	TBlockState *p=&gamestage[x][y];

	if(p->done)return;
	if(p->type != type)return;
	p->done=1;
	if(p->state==BLOCKSTATE_FALLING)return;
	(*number)++;
//	if(p->state==BLOCKSTATE_EXTINGUISHED||p->state==BLOCKSTATE_EXTINGUISHING)(*answer)=1;
	if(p->state==BLOCKSTATE_FALLFINISHED)(*answer)=1;

	if(x>0)erase_check_recursive(x-1,y,type,answer,number);
	if(x<GAME_STAGE_WIDTH-1)erase_check_recursive(x+1,y,type,answer,number);
	if(y>0)erase_check_recursive(x,y-1,type,answer,number);
	if(y<GAME_STAGE_HEIGHT-1)erase_check_recursive(x,y+1,type,answer,number);



}

void seterase_recursive(int x,int y,int type){
	TBlockState *p=&gamestage[x][y];

	if(p->done_sub)return;
	if(p->type != type)return;
	p->done_sub=1;
	if(p->state==BLOCKSTATE_FALLING)return;
	if(p->state==BLOCKSTATE_EXTINGUISHING||p->state==BLOCKSTATE_EXTINGUISHED){
		if(p->extinguishingframe<8)return;
	}
	p->state=BLOCKSTATE_EXTINGUISHING;
	p->extinguishingframe=20;
	p->left=0;
	p->lefttime=0;
	p->player_dug=0;
	if(x>0)seterase_recursive(x-1,y,type);
	if(x<GAME_STAGE_WIDTH-1)seterase_recursive(x+1,y,type);
	if(y>0)seterase_recursive(x,y-1,type);
	if(y<GAME_STAGE_HEIGHT-1)seterase_recursive(x,y+1,type);

}
void erase_check(void){
	int x,y;

	clear_blockflag_sub();
	clear_blockflag();

	for(y=GAME_STAGE_HEIGHT-5;y>=1;y--)
	for(x=0;x<GAME_STAGE_WIDTH;x++){
		int answer, number;
		TBlockState *p=&gamestage[x][y];

		if(p->done)continue;
		if(p->type == NO_BLOCK||p->type==AIR_BLOCK)continue;
		if(p->state == BLOCKSTATE_FALLING /*|| p->state == BLOCKSTATE_PREFALL*/)continue;

		answer=0;number=0;
		erase_check_recursive(x,y,p->type,&answer,&number);
		if(answer == 1 && number>3){
			seterase_recursive(x,y,p->type);
			CWavsPlay(wavs,12);
		}
	}
}

void prefallcheck(void ){

	int x,y,check,lefttime;
	TBlockState *p,*q;
	clear_blockflag_sub();
	clear_blockflag();
	for(y=GAME_STAGE_HEIGHT-5;y>=1;y--)
	for(x=0;x<GAME_STAGE_WIDTH;x++){
		p=&gamestage[x][y];
		if(p->type == NO_BLOCK)continue;
		if(p->state == BLOCKSTATE_FALLING
		||p->state==BLOCKSTATE_EXTINGUISHING
		||p->state==BLOCKSTATE_EXTINGUISHED)continue;

		//if(p->state == BLOCKSTATE_FALLFINISHED)left=1;else left=PREFALLTIME;
		q=&gamestage[x][y+1];
		if(p->unsetlock)continue;
		if(q->type==NO_BLOCK){


			if(p->state==BLOCKSTATE_NONE){
				p->lefttime=0;
				if(q->player_dug)p->lefttime=PREFALLTIME;
				p->state=BLOCKSTATE_PREFALL;
			}else if(p->state==BLOCKSTATE_FALLFINISHED){
				p->lefttime=0;
				p->state=BLOCKSTATE_PREFALL;
			}else {
				p->state=BLOCKSTATE_PREFALL;

			}
		}/*else if(q->type==p->type){

			p->state=q->state;
			p->state=q->state;


		}*/
		else if(q->state==BLOCKSTATE_PREFALL){
			//if(p->state==BLOCKSTATE_NONE){
				p->state=BLOCKSTATE_PREFALL;
				p->lefttime=q->lefttime;
			//}
		}else
		if(q->state==BLOCKSTATE_EXTINGUISHING
		||q->state==BLOCKSTATE_FALLFINISHED||
		q->state==BLOCKSTATE_EXTINGUISHED){
			if(p->state!=BLOCKSTATE_FALLFINISHED){

				p->lefttime=PREFALLTIME;
				p->state=BLOCKSTATE_NONE;
				unsetprefall(x,y,p->type);

			}else{
				unsetprefallfinished(x,y,p->type);
//				p->state=BLOCKSTATE_PREFALL;
			}
		}
		else {
				p->lefttime=PREFALLTIME;
				p->state=BLOCKSTATE_NONE;
			unsetprefall(x,y,p->type);
		}



	}


	clear_blockflag();
	clear_blockflag_sub();
	for(y=GAME_STAGE_HEIGHT-5;y>=1;y--)
	for(x=0;x<GAME_STAGE_WIDTH;x++){
		p=&gamestage[x][y];
		if(p->done)continue;
		if(p->type==NO_BLOCK)continue;
		if(p->state!=BLOCKSTATE_PREFALL)continue;
		lefttime=p->lefttime;
		check=1;
		leftcheck(x,y,p->type,&lefttime,&check);
		setleft(x,y,p->type,lefttime);
	}



}

//ぐらぐらをあわせる処理
void leftcheck(int x,int y,int type,int *checkleft,int *check){

	TBlockState *p,*q;
	int lefttime;
	p=&gamestage[x][y];
	q=&gamestage[x][y+1];


	if(p->done)return;
	if(p->type != type)return;
	p->done=1;
//	if(p->state!=BLOCKSTATE_PREFALL)return;
	if(p->state==BLOCKSTATE_FALLING||p->state==BLOCKSTATE_EXTINGUISHING)return;

	if(p->lefttime>(*checkleft)){
		*checkleft=p->lefttime;
//		*check=0;
	}
	if(q->type!=p->type){

		if(q->type != NO_BLOCK //&&
//			q->state!=BLOCKSTATE_EXTINGUISHING &&
//			q->state!=BLOCKSTATE_EXTINGUISHED
			){

			if(q->state==BLOCKSTATE_PREFALL && p->state==BLOCKSTATE_PREFALL){
				if(q->lefttime>(*checkleft)) *checkleft=q->lefttime;

			}else{
				/* if(q->state==BLOCKSTATE_NONE||q->state==BLOCKSTATE_FALLFINISHED
				||q->state==BLOCKSTATE_FALLING){
				*/
				lefttime=0;
				if(q->player_dug)lefttime=PREFALLTIME;
				if(lefttime>(*checkleft)) *checkleft=lefttime;
				*check=0;

			}

		}

	}

	if(p->type==AIR_BLOCK)return;

	if(x>0)leftcheck(x-1,y,type,checkleft,check);
	if(x<GAME_STAGE_WIDTH-1)leftcheck(x+1,y,type,checkleft,check);
	if(y>0)leftcheck(x,y-1,type,checkleft,check);
	if(y<GAME_STAGE_HEIGHT-1)leftcheck(x,y+1,type,checkleft,check);


}

void setleft(int x,int y,int type,int left){
	TBlockState *p=&gamestage[x][y];

	if(p->done_sub)return;
	if(p->type != type)return;
	p->done_sub=1;

//	if(p->state==BLOCKSTATE_FALLING||p->state==BLOCKSTATE_EXTINGUISHING)return;
	if(p->state!=BLOCKSTATE_PREFALL)return;

	p->lefttime=left;
/*	if(
		q->state!=BLOCKSTATE_EXTINGUISHING &&
		q->type!=NO_BLOCK &&
		q->state==BLOCKSTATE_PREFALL)q->lefttime=left;
*/
	if(p->type==AIR_BLOCK)return;

	if(x>0)setleft(x-1,y,type,left);
	if(x<GAME_STAGE_WIDTH-1)setleft(x+1,y,type,left);
	if(y>0)setleft(x,y-1,type,left);
	if(y<GAME_STAGE_HEIGHT-1)setleft(x,y+1,type,left);

}

//block processing
void blockprocess(void){
	int x,y;
	TBlockState *p,*q;

	clear_blockflag();
	for(y=GAME_STAGE_HEIGHT-1;y>=0;y--)
	for(x=0;x<GAME_STAGE_WIDTH;x++){
		p=&gamestage[x][y];

		if(p->type==NO_BLOCK)continue;

		switch(p->state){


			case BLOCKSTATE_PREFALL:{

				p->lefttime--;
				if(p->lefttime<=0){
					p->lefttime=0;
					q=&gamestage[x][y+1];
//					if(q->state==BLOCKSTATE_EXTINGUISHING)hanabiset(x,y+1,q->extinguishingframe);

					*q=*p;

					p->type=NO_BLOCK;
					p->state=BLOCKSTATE_NONE;

					q->state=BLOCKSTATE_FALLING;
					q->left=TILE_SIZE-WALKSPEED_Y;

				}
			}break;


			case BLOCKSTATE_FALLING:{

				p->left-=WALKSPEED_Y;
				if(p->left<WALKSPEED_Y){//finshing.

					p->left=0;
					p->lefttime=1;
					p->state=BLOCKSTATE_FALLFINISHED;
				}


			}break;

			case BLOCKSTATE_EXTINGUISHING:{

				p->extinguishingframe--;
				if(p->extinguishingframe==0)p->state=BLOCKSTATE_EXTINGUISHED;

			}break;

			case BLOCKSTATE_FALLFINISHED:{
			}break;
		}
	}
}


#define HANABIMAX 100
typedef struct{

	int x;
	int y;
	int time;
	int avail;
}Thanabi;
Thanabi hanabi[HANABIMAX];
//爆発
void clearhanabi(void){
	int i;
	Thanabi init={0,0,0,0};
	for(i=0;i<HANABIMAX;++i){

		hanabi[i]=init;

	}
}
void drawhanabi(void){
	Thanabi* p;
	int gy;
	int i;

	for(i=0;i<HANABIMAX;++i){
		p=&hanabi[i];
		if(!p->avail)continue;

		p->time--;
		if(p->time<=0){

			p->avail=0;
			continue;
		}
		gy=(p->y+4)*TILE_SIZE-my_y;
		if(gy>-TILE_SIZE && gy<SCREEN_WIDTH){
			//draw
			CBmpsBlit(cbmps_character,screen,109-p->time/3,p->x*TILE_SIZE,gy);

		}else{
			p->avail=0;
		}
	}
}

int hanabiset(int x,int y,int n){

	int i=0;

	do{
		if(hanabi[i].avail!=0)i++;else{

			hanabi[i].x=x;
			hanabi[i].y=y;
			hanabi[i].time=n;
			hanabi[i].avail=1;

			return 0;
		}
	}while(i<HANABIMAX);

	return 1;
}




void moveTilescroll(void){

	CTileScroll *p;
	unsigned int interval=66;//milisec
	p=&tscroll;

	if(p->clock+interval<gametime.clock){
		p->x--;
		if(p->x<=-(p->bmp->w))p->x=0;
		p->y--;
		if(p->y<=-(p->bmp->h))p->y=0;

		p->clock=gametime.clock;
	}else{
		if(p->clock>gametime.clock+interval*5)p->clock=gametime.clock;
	}

}
void drawTilescroll(void){

	int x,y;

	for(y=tscroll.y;y<screen->h;y+=tscroll.bmp->h){
		for(x=tscroll.x;x<screen->w;x+=tscroll.bmp->w){

			CBmpsBlit(cbmps_character,screen,52,x,y);
		}
	}

}

int title(void){
//	int joy_up=0,joy_down=0,joy_left=0,joy_right=0,joy_space=0,joy_cancel=0;
	int y = 0, i;
	int dx[]={
		0,-1,-2,-3,-4,-5,-6,-7,-8,-9,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,
		0,1,2,3,4,5,6,7,8,9,10,9,8,7,6,5,4,3,2,1
	};
	int nx=0,maxx;
	int wait=0;

	maxx=sizeof(dx)/sizeof(int);

	CTimeReset(&gametime);
	CTimeChangeFPS(&gametime,FPS_MAX);
	do{
//		joy_space=joy_cancel=joy_up=joy_down=0;
		if(keyread())return 0;;
		moveTilescroll();

		CInputUpdate(gameinput,0);

	//	keys = SDL_GetKeyState(NULL);
	/*
		if(joystick && SDL_JoystickGetButton(joystick, setting_joyconfirm) == SDL_PRESSED)joy_space=1;
		if(joystick && SDL_JoystickGetButton(joystick, setting_joycancel) == SDL_PRESSED)joy_cancel=1;
		if(joystick && (int)SDL_JoystickGetAxis(joystick, 1) > setting_joyaxismax)joy_down=1;
		if(joystick && (int)SDL_JoystickGetAxis(joystick, 1) < -setting_joyaxismax)joy_up=1;
	*/	/*
		if ( keys['q'] == SDL_PRESSED ){return 0;}//game end
		*/
		if ( gameinput->button[BUTTON_0] == SDL_PRESSED ){
			CWavsPlay(wavs,10);
			switch(y){
				case 0:{
					SDL_Delay(500);return 2;
				}break;

				case 1:{
					SDL_Delay(500);
					if(!highscore())return 0;
					SDL_Delay(500);
					y=0;
				}break;

				case 2:{
					return 0;
				}break;

			}
		}
/*
		if ( joy_cancel||keys['s'] == SDL_PRESSED ){
			if(!highscore())return 0;
			SDL_Delay(100);
		}
*/
		if (!wait&&( gameinput->button[BUTTON_UP]) ){
			if(y>0){
				y--;
				CWavsPlay(wavs,10);
			}
			wait=15;
		}
		if (!wait&&( gameinput->button[BUTTON_DOWN]) ){
			if(y<2){
				y++;
				CWavsPlay(wavs,10);
			}
			wait=15;
		}
		wait--;if(wait<=0)wait=0;

		if(!gametime.isDelay){

			drawTilescroll();
			CBmpsBlit(cbmps_character,screen,50,0,0);
			for(i=0;i<3;++i){
				int mx = MAIN_MENU_OPTIONS_X;
				int my = MAIN_MENU_OPTIONS_Y + i * MAIN_MENU_OPTIONS_DY;
				if (y == i) mx += dx[nx++];
				CBmpsBlit(cbmps_character, screen, 60 + i, mx, my);
			}
			nx=nx%maxx;
			//SDL_Flip(screen);
      {
        if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
        int x, y;
        uint32_t *s = screen->pixels;
        uint32_t *d = ScreenSurface->pixels;
        for(y=0; y<240; y++){
          for(x=0; x<160; x++){
            *d++ = *s++;
          }
          d+= 160;
        }
        if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
        SDL_Flip(ScreenSurface);
      }
			//SDL_UpdateRect(screen,0,0,0,0);
		}
		CTimeWait(&gametime);
	}while(1);
	return 0;
}







int highscoretemp_score;
int highscoretemp_depth;
int highscoretemp_playtime;
int highscoretemp_entrynumber_score;
int highscoretemp_entrynumber_time;
char highscoretemp_name[4];


int init_nameentry(int score,int depth,int playtime){
	int i;

	nameentry_x=0;
	nameentry_x_wait=0;
	space_repeat=0;
	bs_repeat=0;
	nameentry_vx=0;
	name_x=0;

	highscoretemp_score=score;
	highscoretemp_depth=depth;
	highscoretemp_playtime=playtime;

	THighScoreAdd(&highscoredata,"___",playtime,depth,score);
	THighScoreSortByTime(&highscoredata);
	THighScoreSortByDepth(&highscoredata);
	THighScoreSortByScore(&highscoredata);

	for(i=0;i<SCOREMEMBER+1;++i){
		if(!strcmp(highscoredata.name[i],"___"))break;
	}
	highscoretemp_entrynumber_score=i;

	THighScoreAdd(&fasttimedata,"___",playtime,depth,score);
	THighScoreSortByDepth(&fasttimedata);
	THighScoreSortByScore(&fasttimedata);
	THighScoreSortByTime(&fasttimedata);

	for(i=0;i<SCOREMEMBER+1;++i){
		if(!strcmp(fasttimedata.name[i],"___"))break;
	}
	highscoretemp_entrynumber_time=i;

	if(highscoretemp_entrynumber_score==SCOREMEMBER&&
	highscoretemp_entrynumber_time==SCOREMEMBER)return -1;//highsore is not marked!

	strcpy(highscoretemp_name,"___");
	CWavsPlayMusicStyle(wavs,2);

	return i;
}

int move_nameentry(void) {
	if (nameentry_x_wait == 0) {
		CInputUpdate(gameinput,0);

		if ( (gameinput->button[BUTTON_RIGHT]) && nameentry_x<(int)sizeof(nameentry_moji)-2 ) {nameentry_x_wait=8;nameentry_vx=1;}
		if ( (gameinput->button[BUTTON_LEFT]) && nameentry_x>0 ) {nameentry_x_wait=8;nameentry_vx=-1;}

		if ( gameinput->button[BUTTON_2] ){
			if(!strcmp(highscoretemp_name,"___"))strcpy(highscoretemp_name,"S.H");
			strcpy(highscoredata.name[highscoretemp_entrynumber_score],highscoretemp_name);
			strcpy(fasttimedata.name[highscoretemp_entrynumber_time],highscoretemp_name);

			return 1;
		}
		if (gameinput->button[BUTTON_0]){
			if (!space_repeat) {
				if(name_x<3){
					highscoretemp_name[name_x]=nameentry_moji[nameentry_x];
					name_x++;
				}
				space_repeat=1;
			}
		}else{
			space_repeat=0;
		}
		if ( gameinput->button[BUTTON_1] ){
			if(!bs_repeat){
				if(name_x>0)name_x--;
				highscoretemp_name[name_x]=' ';
				bs_repeat=1;
			}
		}else{
			bs_repeat=0;
		}
	}

	if(nameentry_x_wait>0){
		nameentry_x_wait--;
		if(nameentry_x_wait==0){
			nameentry_x+=nameentry_vx;
			nameentry_vx=0;
		}
	}

	return 0;
}

void draw_nameentry(void) {
	CffontBlitxy(font, nameentry_moji, screen,
			HIGHSCORE_MARK_X - nameentry_x * FONT_SIZE, HIGHSCORE_MARK_Y);
	CBmpsBlit(cbmps_character, screen, 53, 0, 0);
}

void nameentry(void){

	char buffer[4096];
	char *scoreboard="NAME DEPTH   SCORE  TIME";


	CTimeReset(&gametime);
	CTimeChangeFPS(&gametime,FPS_MAX);
	do{
		if(keyread()){
			strcpy(highscoredata.name[highscoretemp_entrynumber_score],"S.H");
			strcpy(fasttimedata.name[highscoretemp_entrynumber_time],"S.H");
			break;
		}
		moveTilescroll();

		if(move_nameentry())break;
		if(!gametime.isDelay){

			drawTilescroll();

			if(highscoretemp_depth!=1000){
				sprintf(buffer," %3s %4d  %6d0  --'--'--",
					highscoretemp_name,
					highscoretemp_depth,
					highscoretemp_score
					);
			}else{
				sprintf(buffer," %3s %4d  %6d0  %02d'%02d'%02d",
					highscoretemp_name,
					highscoretemp_depth,
					highscoretemp_score,
					highscoretemp_playtime/6000,
					(highscoretemp_playtime/100)%60,
					highscoretemp_playtime%100

				);
			}
			CffontBlitxy(font, buffer, screen,
					HIGHSCORE_TEXT_X, HIGHSCORE_SCORE_Y);
			CffontBlitxy(font, scoreboard, screen,
					HIGHSCORE_TEXT_X, HIGHSCORE_LABEL_Y);

			draw_nameentry();

			//SDL_Flip(screen);
      {
        if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
        int x, y;
        uint32_t *s = screen->pixels;
        uint32_t *d = ScreenSurface->pixels;
        for(y=0; y<240; y++){
          for(x=0; x<160; x++){
            *d++ = *s++;
          }
          d+= 160;
        }
        if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
        SDL_Flip(ScreenSurface);
      }
			//SDL_UpdateRect(screen,0,0,0,0);
		}
		CTimeWait(&gametime);
	}while(1);
}

int highscore(void){
	int i;
	char buf[SCOREMEMBER][1024];
	char buf_fast[SCOREMEMBER][1024];
	char *scoreboard="NAME DEPTH   SCORE";
	char *scoreboard_fast="NAME   SCORE   TIME";
	int mode=0;//0--highscore 1--fasttime


//	int joy_space=0;
	for(i=0;i<SCOREMEMBER;++i){
		sprintf(buf[i]," %3s %4d  %6d0",
			highscoredata.name[i],
			highscoredata.depth[i],
			highscoredata.score[i]
		);

	}
	for(i=0;i<SCOREMEMBER;++i){
		sprintf(buf_fast[i]," %3s %6d0 %02d'%02d'%02d",
			fasttimedata.name[i],
			fasttimedata.score[i],
			fasttimedata.time[i]/6000,
			(fasttimedata.time[i]/100)%60,
			fasttimedata.time[i]%100

			);


	}
	CTimeReset(&gametime);
	CTimeChangeFPS(&gametime,setting_defaultFPS);
	do{
//		joy_space=0;
		if(keyread())return 0;
		moveTilescroll();

//		keys = SDL_GetKeyState(NULL);
		CInputUpdate(gameinput,0);
//		if(joystick && SDL_JoystickGetButton(joystick, setting_joyconfirm) == SDL_PRESSED)joy_space=1;
		if ( gameinput->button[BUTTON_0] ){SDL_Delay(100);return 1;}//exit highscore
		if( gameinput->button[BUTTON_RIGHT] && mode<1){
			mode++;
		}
		if( gameinput->button[BUTTON_LEFT] && mode>0){
			mode--;
		}


		if(!gametime.isDelay){

			drawTilescroll();

			if(mode==0){
				CBmpsBlit(cbmps_character,screen,51,0,0);
				CffontBlitxy(font,scoreboard,screen,SCOREBOARD_X,SCOREBOARD_LABEL_Y);
			}else{
				CBmpsBlit(cbmps_character,screen,54,0,0);
				CffontBlitxy(font,scoreboard_fast,screen,SCOREBOARD_X,SCOREBOARD_LABEL_Y);
			}

			for(i=0;i<SCOREMEMBER;++i){
				int y = SCOREBOARD_SCORE_Y + i * SCOREBOARD_SCORE_DY;
				if(mode==0){
					CffontBlitxy(font,buf[i],screen,SCOREBOARD_X,y);
				}else{
					CffontBlitxy(font,buf_fast[i],screen,SCOREBOARD_X,y);
				}
			}

			//SDL_Flip(screen);
      {
        if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
        int x, y;
        uint32_t *s = screen->pixels;
        uint32_t *d = ScreenSurface->pixels;
        for(y=0; y<240; y++){
          for(x=0; x<160; x++){
            *d++ = *s++;
          }
          d+= 160;
        }
        if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
        SDL_Flip(ScreenSurface);
      }
			//SDL_UpdateRect(screen,0,0,0,0);
		}
		CTimeWait(&gametime);
	}while(1);

	return 0;
}




void THighScoreSwap(THighScore *p,int i,int j){
	char name[4];
	int time,score,depth;
	strcpy(name,p->name[j]);
	time=p->time[j];
	score=p->score[j];
	depth=p->depth[j];

	strcpy(p->name[j],p->name[i]);
	p->time[j]=p->time[i];
	p->score[j]=p->score[i];
	p->depth[j]=p->depth[i];

	strcpy(p->name[i],name);
	p->time[i]=time;
	p->score[i]=score;
	p->depth[i]=depth;
}

void THighScoreSortByTime(THighScore *p){
	int i,j;

	for(j=0;j<SCOREMEMBER;++j)
	for(i=j+1;i<SCOREMEMBER+1;++i){
		if(p->time[i]<p->time[j])THighScoreSwap(p,i,j);
	}

}
void THighScoreSortByDepth(THighScore *p){
	int i,j;

	for(j=0;j<SCOREMEMBER;++j)
	for(i=j+1;i<SCOREMEMBER+1;++i){
		if(p->depth[i]>p->depth[j])THighScoreSwap(p,i,j);
	}

}
void THighScoreSortByScore(THighScore *p){
	int i,j;

	for(j=0;j<SCOREMEMBER;++j)
	for(i=j+1;i<SCOREMEMBER+1;++i){
		if(p->score[i]>p->score[j])THighScoreSwap(p,i,j);
	}

}

void THighScoreAdd(THighScore *p,char *name,int time,int depth,int score){

	strcpy(p->name[SCOREMEMBER],name);
	p->time[SCOREMEMBER]=time;
	p->depth[SCOREMEMBER]=depth;
	p->score[SCOREMEMBER]=score;

}

int THighScoreSave(THighScore *p,char *filename){

	FILE *fp;
	int i;

	fp=fopen(filename,"wb");
	if(fp==NULL){
		fprintf(stderr,"cant open %s",filename);
		return -1;
	}
	for(i=0;i<SCOREMEMBER;++i){

		fprintf(fp,highscoreformat,
			(p->name[i]),
			(p->time[i]),
			(p->depth[i]),
			(p->score[i])
			);

	}
	fclose(fp);
	return 0;
}

int THighScoreLoad(THighScore *p,char *filename){
	int i;
	FILE *fp;
	THighScore init={
		{"ZEN","K.K","IKU","FKD","IWA","M.N","TT ","KOB","SAD","ADA","DUM"},
		{100000,100000,100000,100000,100000,100000,100000,100000,100000,100000,100000},
		{100,90,80,70,60,50,40,30,20,10,0},
		{100,90,80,70,60,50,40,30,20,10,0}
	};

	fp=fopen(filename,"rb");
	if(fp==NULL){
		printf("create new save file\n");
		*p=init;
		return THighScoreSave(&init,filename);
	}
	for(i=0;i<SCOREMEMBER;++i){

		fscanf(fp,highscoreformat,
			&(p->name[i]),
			&(p->time[i]),
			&(p->depth[i]),
			&(p->score[i])
			);

	}
	fclose(fp);
	return 0;
}



void joy_init(void){
	//0番目のジョイスティックをオープン
	/*
	SDL_Init(SDL_INIT_JOYSTICK);
	joystick = SDL_JoystickOpen(0);
	if ( joystick == NULL ) {
		printf("Couldn't open joystick \n");
	}
	*/

	gameinput=CInputInit(setting_joysticknumber,setting_joyenabled);
	CInputDefaultSetting(gameinput);
	CInputHoldButtons(gameinput);
	CInputUnholdArrows(gameinput);
	CInputSetMinAxis(gameinput,setting_joyaxismax);
	return ;
}
void joy_final(void){
	//SDL_JoystickClose(joystick);

	CInputFree(gameinput);
	gameinput=NULL;

}



int BlitForBlock(SDL_Surface *p,SDL_Surface *dest,int num,int x,int y){
	SDL_Rect dr = { x, y, TILE_SIZE, TILE_SIZE };
	SDL_Rect rects[] = {
		{             0,             0, TILE_SIZE, TILE_SIZE },
		{     TILE_SIZE,             0, TILE_SIZE, TILE_SIZE },
		{             0, 3 * TILE_SIZE, TILE_SIZE, TILE_SIZE },
		{     TILE_SIZE, 3 * TILE_SIZE, TILE_SIZE, TILE_SIZE },

		{ 3 * TILE_SIZE,             0, TILE_SIZE, TILE_SIZE },
		{ 2 * TILE_SIZE,             0, TILE_SIZE, TILE_SIZE },
		{ 3 * TILE_SIZE, 3 * TILE_SIZE, TILE_SIZE, TILE_SIZE },
		{ 2 * TILE_SIZE, 3 * TILE_SIZE, TILE_SIZE, TILE_SIZE },

		{             0,     TILE_SIZE, TILE_SIZE, TILE_SIZE },
		{     TILE_SIZE,     TILE_SIZE, TILE_SIZE, TILE_SIZE },
		{             0, 2 * TILE_SIZE, TILE_SIZE, TILE_SIZE },
		{     TILE_SIZE, 2 * TILE_SIZE, TILE_SIZE, TILE_SIZE },

		{ 3 * TILE_SIZE,     TILE_SIZE, TILE_SIZE, TILE_SIZE },
		{ 2 * TILE_SIZE,     TILE_SIZE, TILE_SIZE, TILE_SIZE },
		{ 3 * TILE_SIZE, 2 * TILE_SIZE, TILE_SIZE, TILE_SIZE },
		{ 2 * TILE_SIZE, 2 * TILE_SIZE, TILE_SIZE, TILE_SIZE },
	};

	if(p==NULL)return(-1);

	SDL_BlitSurface(p,&rects[num],dest,&dr);

	return 0;
}

//形を整える
void set_shape(void){

	int x,y;
	TBlockState *p;


	for(y=0;y<GAME_STAGE_HEIGHT;++y){
		for(x=0;x<GAME_STAGE_WIDTH;++x){
			p=&(gamestage[x][y]);
			if(p->type==NO_BLOCK||p->type==HARD_BLOCK||p->type==AIR_BLOCK)continue;

			if(p->state==BLOCKSTATE_FALLING
				||p->state==BLOCKSTATE_PREFALL
				||p->state==BLOCKSTATE_EXTINGUISHING
			)continue;
			p->shape=0;

			if(x>0  &&gamestage[x-1][y].type==p->type)p->shape+=4;
			if(x<GAME_STAGE_WIDTH-1  && gamestage[x+1][y].type==p->type)p->shape+=1;
			if(y>0  && gamestage[x][y-1].type==p->type)p->shape+=2;
			if(y<GAME_STAGE_HEIGHT-1  && gamestage[x][y+1].type==p->type)p->shape+=8;
		}
	}

}



