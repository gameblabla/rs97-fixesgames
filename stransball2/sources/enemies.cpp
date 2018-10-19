#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#include "SDL/SDL.h"
#include "SDL_mixer.h"
#include "sge.h"

#include "list.h"

#include "tiles.h"
#include "maps.h"

#include "auxiliar.h"

#define FACTOR 512


void ENEMY::draw_bullet(SDL_Surface *screen,TILE **tiles,int map_x,int map_y)
{
	if (state>=0) {
		tiles[tile]->draw((x/FACTOR)-map_x-8,(y/FACTOR)-map_y-8,screen);
	} else {
		int i;

		i=(-state)/8;
		tiles[243+i]->draw((x/FACTOR)-map_x-8,(y/FACTOR)-map_y-8,screen);
	} /* if */ 
} /* draw_bullet */ 

bool ENEMY::cycle_bullet(int sx,int sy,bool collision)
{
	if (state!=0) state--;

	if (x<0 || x>sx*FACTOR ||
		y<0 || y>sy*FACTOR) return false;
	
	if (state==0 && collision) state=-1;
	if (state<=-40) return false;

	if (state>=0) {
		x+=speed_x;
		y+=speed_y;
	} /* if */ 
	return true;
} /* cycle_bullet */ 


bool ENEMY::cycle_canon(int ship_x,int ship_y,List<ENEMY> *enemies,Mix_Chunk *S_shot)
{
	if (state==0) {
		switch(direction) {
		case 0:
			if (ship_x>=(x-8) && ship_x<=(x+24) && ship_y<y && ship_y>y-160) {
				ENEMY *e;
				e=new ENEMY();
				e->type=0;
				e->state=12;
				e->x=(x+8)*FACTOR;
				e->y=(y+0)*FACTOR;
				e->speed_x=0;
				e->speed_y=-FACTOR;
				e->life=1;
				e->tile=344;

				enemies->Add(e);
				state=128;
				if (S_shot!=0) Mix_PlayChannel(-1,S_shot,0);		
			} /* if */ 
			break;
		case 1:
			if (ship_x>=(x-8) && ship_x<=(x+24) && ship_y>y && ship_y<y+160) {
				ENEMY *e;
				e=new ENEMY();
				e->type=0;
				e->state=12;
				e->x=(x+8)*FACTOR;
				e->y=(y+16)*FACTOR;
				e->speed_x=0;
				e->speed_y=FACTOR;
				e->life=1;
				e->tile=344;

				enemies->Add(e);
				state=128;
				if (S_shot!=0) Mix_PlayChannel(-1,S_shot,0);		
			} /* if */ 
			break;
		case 2:
			if (ship_y>=(y-8) && ship_y<=(y+24) && ship_x>x && ship_x<x+160) {
				ENEMY *e;
				e=new ENEMY();
				e->type=0;
				e->state=12;
				e->x=(x+16)*FACTOR;
				e->y=(y+7)*FACTOR;
				e->speed_x=FACTOR;
				e->speed_y=0;
				e->life=1;
				e->tile=344;

				enemies->Add(e);
				state=128;
				if (S_shot!=0) Mix_PlayChannel(-1,S_shot,0);
			} /* if */ 
			break;
		case 3: 
			if (ship_y>=(y-8) && ship_y<=(y+24) && ship_x<x && ship_x>x-160) {
				ENEMY *e;
				e=new ENEMY();
				e->type=0;
				e->state=12;
				e->x=(x+0)*FACTOR;
				e->y=(y+7)*FACTOR;
				e->speed_x=-FACTOR;
				e->speed_y=0;
				e->life=1;
				e->tile=344;

				enemies->Add(e);
				state=128;
				if (S_shot!=0) Mix_PlayChannel(-1,S_shot,0);		
			} /* if */ 
			break;
		} /* switch */ 
	} else {
		if (state>0) state--;
		if (state<0) return false;
	} /* if */ 
	return true;
} /* cycle_canon */ 


bool ENEMY::cycle_fastcanon(int ship_x,int ship_y,List<ENEMY> *enemies,Mix_Chunk *S_shot)
{
	if (state==0) {
		switch(direction) {
		case 0:
			if (ship_x>=(x-8) && ship_x<=(x+24) && ship_y<y && ship_y>y-160) {
				ENEMY *e;
				e=new ENEMY();
				e->type=0;
				e->state=8;
				e->x=(x+8)*FACTOR;
				e->y=(y+0)*FACTOR;
				e->speed_x=0;
				e->speed_y=-FACTOR*3;
				e->life=1;
				e->tile=344;

				enemies->Add(e);
				state=64;
				if (S_shot!=0) Mix_PlayChannel(-1,S_shot,0);
			} /* if */ 
			break;
		case 1:
			if (ship_x>=(x-8) && ship_x<=(x+24) && ship_y>y && ship_y<y+160) {
				ENEMY *e;
				e=new ENEMY();
				e->type=0;
				e->state=8;
				e->x=(x+8)*FACTOR;
				e->y=(y+16)*FACTOR;
				e->speed_x=0;
				e->speed_y=FACTOR*3;
				e->life=1;
				e->tile=344;

				enemies->Add(e);
				state=64;
				if (S_shot!=0) Mix_PlayChannel(-1,S_shot,0);
			} /* if */ 
			break;
		case 2:
			if (ship_y>=(y-8) && ship_y<=(y+24) && ship_x>x && ship_x<x+160) {
				ENEMY *e;
				e=new ENEMY();
				e->type=0;
				e->state=8;
				e->x=(x+16)*FACTOR;
				e->y=(y+7)*FACTOR;
				e->speed_x=FACTOR*3;
				e->speed_y=0;
				e->life=1;
				e->tile=344;

				enemies->Add(e);
				state=64;
				if (S_shot!=0) Mix_PlayChannel(-1,S_shot,0);
			} /* if */ 
			break;
		case 3: 
			if (ship_y>=(y-8) && ship_y<=(y+24) && ship_x<x && ship_x>x-160) {
				ENEMY *e;
				e=new ENEMY();
				e->type=0;
				e->state=8;
				e->x=(x+0)*FACTOR;
				e->y=(y+7)*FACTOR;
				e->speed_x=-FACTOR*3;
				e->speed_y=0;
				e->life=1;
				e->tile=344;

				enemies->Add(e);
				state=64;
				if (S_shot!=0) Mix_PlayChannel(-1,S_shot,0);
			} /* if */ 
			break;
		} /* switch */ 
	} else {
		if (state>0) state--;
		if (state<0) return false;
	} /* if */ 
	return true;
} /* cycle_fastcanon */ 


void ENEMY::draw_tank(SDL_Surface *screen,TILE **tiles,int map_x,int map_y)
{
	int tmp=0;

	if ((state2&0x8)==0) tmp=2;

	SDL_Surface *tank_sfc=SDL_CreateRGBSurface(SDL_SRCALPHA,32,16,32,0,0,0,0);
	SDL_Surface *tank_sfc2=SDL_CreateRGBSurface(SDL_SRCALPHA,48,48,32,0,0,0,0);
	SDL_Surface *tank_sfc3=SDL_CreateRGBSurface(SDL_SRCALPHA,48,48,32,0,0,0,0);
	SDL_Rect d;

	SDL_SetColorKey(tank_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(tank_sfc->format,0,0,0));
	SDL_SetColorKey(tank_sfc2,SDL_SRCCOLORKEY,SDL_MapRGB(tank_sfc2->format,0,0,0));
	SDL_SetColorKey(tank_sfc3,SDL_SRCCOLORKEY,SDL_MapRGB(tank_sfc3->format,0,0,0));

	if (tank_type<3) {
		tiles[282+4*tank_type+tmp]->draw(0,0,tank_sfc);
		tiles[283+4*tank_type+tmp]->draw(16,0,tank_sfc);
	} else {
		tiles[461+((state/2)%4)*2]->draw(0,0,tank_sfc);
		tiles[462+((state/2)%4)*2]->draw(16,0,tank_sfc);
	} /* if */ 

	sge_transform(tank_sfc,tank_sfc2, float(tank_angle), 1.0F, 1.0F, 16, 8, 24, 24, 0);

	/* Turret: */ 
	if (tank_type<3) {
		if (turret_angle<37) tiles[271]->draw(16,8,tank_sfc3);
		if (turret_angle>=37 && turret_angle<53) tiles[270]->draw(16,8,tank_sfc3);
		if (turret_angle>=53 && turret_angle<75) tiles[269]->draw(16,8,tank_sfc3);
		if (turret_angle>=75 && turret_angle<105) tiles[268]->draw(16,8,tank_sfc3);
		if (turret_angle>=105 && turret_angle<127) tiles[267]->draw(16,8,tank_sfc3);
		if (turret_angle>=127 && turret_angle<143) tiles[266]->draw(16,8,tank_sfc3);
		if (turret_angle>=143) tiles[265]->draw(16,8,tank_sfc3);
		tiles[262+tank_type]->draw(16,8,tank_sfc3);
	} else {
		SDL_Surface *canon_sfc=SDL_CreateRGBSurface(SDL_SRCALPHA,16,16,32,0,0,0,0);
		SDL_Surface *canon_sfc2=SDL_CreateRGBSurface(SDL_SRCALPHA,32,32,32,0,0,0,0);
		SDL_Rect d;

		SDL_SetColorKey(canon_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(canon_sfc->format,0,0,0));
		SDL_SetColorKey(canon_sfc2,SDL_SRCCOLORKEY,SDL_MapRGB(canon_sfc2->format,0,0,0));

		tiles[334]->draw(0,0,canon_sfc);

		sge_transform(canon_sfc,canon_sfc2, float(-turret_angle), 0.75F, 0.75F, 0, 4, 16, 16, 0);

		d.x=8;
		d.y=0;
		SDL_BlitSurface(canon_sfc2,0,tank_sfc3,&d);

		SDL_FreeSurface(canon_sfc);
		SDL_FreeSurface(canon_sfc2);

		tiles[460]->draw(16,6,tank_sfc3);
	} /* if */ 

	SDL_BlitSurface(tank_sfc2,0,tank_sfc3,0);

	d.x=(x-map_x)-24;
	d.y=(y-map_y)-16;
	SDL_BlitSurface(tank_sfc3,0,screen,&d);

/*
	{
		int i;
		for(i=0;i<180;i++) {
			putpixel(screen,x-map_x+int(cos(float((i*3.141592F)/180.0F))*FACTOR*12)/FACTOR,
							y-map_y-int(sin(float((i*3.141592F)/180.0F))*FACTOR*12)/FACTOR,SDL_MapRGB(screen->format,255,255,255));
		}
	}
*/

	SDL_FreeSurface(tank_sfc);
	SDL_FreeSurface(tank_sfc2);
	SDL_FreeSurface(tank_sfc3);
	
} /* draw_tank */ 


void ENEMY::draw_destroyedtank(SDL_Surface *screen,TILE **tiles,int map_x,int map_y)
{
	int tmp=0;

	if ((state2&0x8)==0) tmp=2;

	SDL_Surface *tank_sfc=SDL_CreateRGBSurface(SDL_SRCALPHA,32,16,32,0,0,0,0);
	SDL_Surface *tank_sfc2=SDL_CreateRGBSurface(SDL_SRCALPHA,48,48,32,0,0,0,0);
	SDL_Surface *tank_sfc3=SDL_CreateRGBSurface(SDL_SRCALPHA,48,48,32,0,0,0,0);
	SDL_Rect d;

	SDL_SetColorKey(tank_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(tank_sfc->format,0,0,0));
	SDL_SetColorKey(tank_sfc2,SDL_SRCCOLORKEY,SDL_MapRGB(tank_sfc2->format,0,0,0));
	SDL_SetColorKey(tank_sfc3,SDL_SRCCOLORKEY,SDL_MapRGB(tank_sfc3->format,0,0,0));

	if (tank_type<3) {
		tiles[282+4*tank_type+tmp]->draw(0,0,tank_sfc);
		tiles[283+4*tank_type+tmp]->draw(16,0,tank_sfc);
	} else {
		tiles[461+((state/2)%4)*2]->draw(0,0,tank_sfc);
		tiles[462+((state/2)%4)*2]->draw(16,0,tank_sfc);
	} /* if */ 

	sge_transform(tank_sfc,tank_sfc2, float(tank_angle), 1.0F, 1.0F, 16, 8, 24, 24, 0);

	/* Turret: */ 
	if (tank_type<3) {
		if (state<48) tiles[248+state/8]->draw(16,8,tank_sfc3);
	} else {
		if (state<48) tiles[248+state/8]->draw(16,6,tank_sfc3);
	} /* if */ 

	SDL_BlitSurface(tank_sfc2,0,tank_sfc3,0);

	d.x=(x-map_x)-24;
	d.y=(y-map_y)-16;
	SDL_BlitSurface(tank_sfc3,0,screen,&d);

	SDL_FreeSurface(tank_sfc);
	SDL_FreeSurface(tank_sfc2);
	SDL_FreeSurface(tank_sfc3);
	
} /* draw_destroyedtank */ 


bool ENEMY::cycle_tank(int ship_x,int ship_y,int ship_sx,int ship_sy,int gdist1,int gdist2,bool lcol,bool rcol,List<ENEMY> *enemies,Mix_Chunk *S_shot)
{
	int old_tank_angle=tank_angle;

	/* Tracks motion: angle, gravity, collision and movement */ 
	if (gdist1>gdist2) {
		float dif=float(gdist1-gdist2)/16.0F;
		float radians=float(atan2(dif,1));

		tank_angle=int((-radians*180)/3.141592F);
	} /* if */ 

	if (gdist1<gdist2) {
		float dif=float(gdist2-gdist1)/16.0F;
		float radians=float(atan2(dif,1));

		tank_angle=int((radians*180)/3.141592F);
	} /* if */ 

	if (gdist1==gdist2) tank_angle=0;

	if (abs(old_tank_angle-tank_angle)>2) {
		if (old_tank_angle<tank_angle) tank_angle=old_tank_angle+2;
								  else tank_angle=old_tank_angle-2;
	} /* if */ 

	if (((gdist1+gdist2)/2)>0) {
		y++;
		gdist1--;
		gdist2--;
	} /* if */ 
	if (((gdist1+gdist2)/2)<0) {
		y--;
		gdist1++;
		gdist2++;
	} /* if */ 

	state2++;

	if (((gdist1+gdist2)/2)==0) {
		if (tank_type<3) {
			switch(state) {
			case 1:
				if ((state2&0x03)==0) {
					if (rcol) {
						state=-1;
						x--;
					} else {
						x++;
					} /* if */ 
				} /* if */ 
				break;
			case -1:
				if ((state2&0x03)==0) {
					if (lcol) {
						state=1;
						x++;
					} else {
						x--;
					} /* if */ 
				} /* if */ 
				break;
			} /* switch */ 
		} else {
			if ((state2&0x03)==0) {
				if (x+32<ship_x && !rcol) {
					x++;
					state++;
				} /* if */ 
				if (x-32>ship_x && !lcol) {
					x--;
					state--;
					if (state<0) state+=256;
				} /* if */ 
			} /* if */ 
		} /* if */ 

	} /* if */ 

	/* Turret's angle: */ 
	{
		int dx=ship_x-x;
		int dy=ship_y-y;
		float radians;

		if (tank_type<3) {
			radians=float(atan2(-float(dy),float(dx)));
			turret_angle=int((radians*180)/3.141592F);
		} else {
			int i;
			float angle_to_ship;
			int desired_turret_angle;
			
			radians=float(atan2(float(dy),float(dx)));
			angle_to_ship=radians;
			
			if (ship_sx==0 && ship_sy==0) {
				desired_turret_angle=int((radians*180)/3.141592F);
			} else {
				float alpha=0,best_alpha=0,min_error=10000;
				float s_sx=float(ship_sx)/FACTOR,s_sy=float(ship_sy)/FACTOR;

				float error=0;
				float d,ls,lb;
				float b_sx,b_sy;
				float min,max;

				min=3.5779242F;
				max=6.0213847F;

				/* Copute the error given an angle "alpha": */ 
				for(alpha=min;alpha<max;alpha+=0.02F) {
					b_sx=float(cos(alpha));
					b_sy=float(sin(alpha));

					d=s_sy*b_sx-s_sx*b_sy;
					if (d!=0) {
						ls=(dx*b_sy-dy*b_sx)/d;
						lb=(s_sy*dx-s_sx*dy)/d;

						if (lb>0) {
							error=float(fabs(ls-lb));
						} else {
							error=10000;
						} /* if */ 
					} else {
						error=10000;
					} /* if */ 

					if (error<min_error) {
						best_alpha=alpha;
						min_error=error;
					} /* if */ 
				} /* for */ 

				if (angle_to_ship<0) angle_to_ship+=6.283184F;
				if (best_alpha<0) best_alpha+=6.283184F;

				if (float(fabs(angle_to_ship-best_alpha))>(3.141592F/2) &&
					float(fabs(angle_to_ship-best_alpha))<((3.141592F*3)/2)) {
					float d=angle_to_ship-best_alpha;

					if (d>0 && d<3.141592F) best_alpha=angle_to_ship-(3.141592F/2);
									   else best_alpha=angle_to_ship+(3.141592F/2);
				} /* if */ 

				desired_turret_angle=int((best_alpha*180)/3.141592F);
				radians=best_alpha;
			} /* if */ 

			while (desired_turret_angle<0) desired_turret_angle+=360;
			while (desired_turret_angle>=360) desired_turret_angle-=360;
			desired_turret_angle=360-desired_turret_angle;
			for(i=0;i<2;i++) {
				if (turret_angle<0) turret_angle+=360;
				if (turret_angle>=360) turret_angle-=360;
				if (desired_turret_angle!=turret_angle) {
					if (((desired_turret_angle-turret_angle)<180 &&
						 (desired_turret_angle-turret_angle)>0) ||
						 (desired_turret_angle-turret_angle)<-180) {
						turret_angle++;
					} else {
						turret_angle--;
					} /* if */ 
				} /* if */ 
			} /* for */ 
			if (turret_angle<0 || turret_angle>270) turret_angle=0;
			if (turret_angle>180) turret_angle=180;
			radians=float((turret_angle*3.141592F)/180.0F);
		} /* if */ 

		if ((tank_type<3 && state2>=128) ||
			(tank_type==3 && state2>=96)) {
			if (turret_angle>15 && turret_angle<175) {
				if ((dx*dx)+(dy*dy)<30000) {
					/* Fire!: */ 
					ENEMY *e;
					e=new ENEMY();
					e->type=0;
					e->state=8;
					e->speed_x=int(cos(radians)*FACTOR);
					e->speed_y=-int(sin(radians)*FACTOR);
					if (tank_type<3) {
						e->x=x*FACTOR+(e->speed_x*6);
						e->y=y*FACTOR+(e->speed_y*6);
					} else {
						e->x=x*FACTOR+(e->speed_x*12);
						e->y=y*FACTOR+(e->speed_y*12);
					} /* if */ 
					e->life=1;
					e->tile=344;

					enemies->Add(e);
					if (S_shot!=0) Mix_PlayChannel(-1,S_shot,0);
				} /* if */ 
			} /* if */ 
			state2=0;
		} /* if */ 
	}

	return true;
} /* cycle_tank */ 




void ENEMY::draw_directionalcanon(SDL_Surface *screen,TILE **tiles,int tile,int map_x,int map_y)
{
	if (state>=0) {
		SDL_Surface *canon_sfc=SDL_CreateRGBSurface(SDL_SRCALPHA,16,16,32,0,0,0,0);
		SDL_Surface *canon_sfc2=SDL_CreateRGBSurface(SDL_SRCALPHA,32,32,32,0,0,0,0);
		SDL_Rect d;

		SDL_SetColorKey(canon_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(canon_sfc->format,0,0,0));
		SDL_SetColorKey(canon_sfc2,SDL_SRCCOLORKEY,SDL_MapRGB(canon_sfc2->format,0,0,0));

		tiles[254]->draw(0,0,canon_sfc);

		sge_transform(canon_sfc,canon_sfc2, float(turret_angle), 0.75F, 0.75F, 0, 4, 16, 16, 0);
		tiles[tile]->draw(8,8,canon_sfc2);

		d.x=(x-map_x)-8;
		d.y=(y-map_y)-8;
		SDL_BlitSurface(canon_sfc2,0,screen,&d);

		SDL_FreeSurface(canon_sfc);
		SDL_FreeSurface(canon_sfc2);
	} /* if */ 
} /* draw_directionalcanon */ 




bool ENEMY::cycle_directionalcanon(int ship_x,int ship_y,List<ENEMY> *enemies,Mix_Chunk *S_shot)
{
	/* Turret's angle: */ 
	int dx=ship_x-(x+8);
	int dy=ship_y-(y+8);
	float radians=float(atan2(float(dy),float(dx)));
	int desired_turret_angle;

	if (state>=0) {
		turret_angle=int((radians*180)/3.141592F);
		if (turret_angle<0) turret_angle+=360;
		desired_turret_angle=turret_angle;

		switch(direction) {
		case 0:
			if (turret_angle>=345 || turret_angle<90) turret_angle=345;
			if (turret_angle<205) turret_angle=205;
			break;
		case 1:
			if (turret_angle<15 || turret_angle>=270) turret_angle=15;
			if (turret_angle>=175) turret_angle=175;
			break;
		case 2:
			if (turret_angle>=75 && turret_angle<180) turret_angle=75;
			if (turret_angle>=180 && turret_angle<285) turret_angle=285;
			break;
		case 3:
			if (turret_angle<105) turret_angle=105;
			if (turret_angle>=255) turret_angle=255;
			break;
		} /* switch */ 

		state++;

		if (state>=128) {
			if (turret_angle==desired_turret_angle) {
				if ((dx*dx)+(dy*dy)<30000) {
					/* Fire!: */ 
					ENEMY *e;
					e=new ENEMY();
					e->type=0;
					e->state=8;
					e->speed_x=int(cos(radians)*FACTOR);
					e->speed_y=int(sin(radians)*FACTOR);
					e->x=(x+8)*FACTOR+(e->speed_x*8);
					e->y=(y+8)*FACTOR+(e->speed_y*8);
					e->life=1;
					e->tile=344;

					enemies->Add(e);
					if (S_shot!=0) Mix_PlayChannel(-1,S_shot,0);
				} /* if */ 
			} /* if */ 
			state=0;
		} /* if */ 
	} else {
		return false;
	} /* if */ 

	return true;
} /* cycle_directionalcanon */ 


void ENEMY::draw_explosion(SDL_Surface *screen,TILE **tiles,int map_x,int map_y)
{
	int frames[6]={240,241,260,261,280,281};

	if (state<=47) {
		tiles[frames[state/8]]->draw(x-map_x,y-map_y,screen);
	} /* if */ 
} /* draw_explosion */ 


bool ENEMY::cycle_explosion(void)
{
	state++;
	if (state>=48) return false;
	return true;
} /* draw_explosion */ 



void ENEMY::draw_directionalcanon2(SDL_Surface *screen,TILE **tiles,int tile,int map_x,int map_y)
{
	if (state>=0) {
		SDL_Surface *canon_sfc=SDL_CreateRGBSurface(SDL_SRCALPHA,16,16,32,0,0,0,0);
		SDL_Surface *canon_sfc2=SDL_CreateRGBSurface(SDL_SRCALPHA,32,32,32,0,0,0,0);
		SDL_Rect d;

		SDL_SetColorKey(canon_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(canon_sfc->format,0,0,0));
		SDL_SetColorKey(canon_sfc2,SDL_SRCCOLORKEY,SDL_MapRGB(canon_sfc2->format,0,0,0));

		tiles[334]->draw(0,0,canon_sfc);

		sge_transform(canon_sfc,canon_sfc2, float(turret_angle), 0.75F, 0.75F, 0, 4, 16, 16, 0);
		tiles[tile]->draw(8,8,canon_sfc2);

		d.x=(x-map_x)-8;
		d.y=(y-map_y)-8;
		SDL_BlitSurface(canon_sfc2,0,screen,&d);

		SDL_FreeSurface(canon_sfc);
		SDL_FreeSurface(canon_sfc2);
	} /* if */ 
} /* draw_directionalcanon2 */ 




bool ENEMY::cycle_directionalcanon2(int ship_x,int ship_y,int ship_sx,int ship_sy,List<ENEMY> *enemies,Mix_Chunk *S_shot)
{
	/* Turret's angle: */ 
	int i;
	int dx=ship_x-(x+8);
	int dy=ship_y-(y+8);
	float radians=float(atan2(float(dy),float(dx)));
	float angle_to_ship=radians;
	int desired_turret_angle;

	if (state>=0) {

		if (ship_sx==0 && ship_sy==0) {
			desired_turret_angle=int((radians*180)/3.141592F);
		} else {
			float alpha=0,best_alpha=0,min_error=10000;
			float s_sx=float(ship_sx)/FACTOR,s_sy=float(ship_sy)/FACTOR;

			float error=0;
			float d,ls,lb;
			float b_sx,b_sy;
			float min,max;

			switch(direction) {
			case 0:
				min=3.5779242F;
				max=6.0213847F;
				break;
			case 1:
				min=0.2617993F;
				max=3.0543256F;
				break;
			case 2:
				min=-1.3089967F;
				max=1.3089967F;
				break;
			case 3:
				min=1.8325953F;
				max=4.4505887F;
				break;
			} /* switch */ 

			/* Copute the error given an angle "alpha": */ 
			for(alpha=min;alpha<max;alpha+=0.02F) {
				b_sx=float(cos(alpha));
				b_sy=float(sin(alpha));

				d=s_sy*b_sx-s_sx*b_sy;
				if (d!=0) {
					ls=(dx*b_sy-dy*b_sx)/d;
					lb=(s_sy*dx-s_sx*dy)/d;

					if (lb>0) {
						error=float(fabs(ls-lb));
					} else {
						error=10000;
					} /* if */ 
				} else {
					error=10000;
				} /* if */ 

				if (error<min_error) {
					best_alpha=alpha;
					min_error=error;
				} /* if */ 
			} /* for */ 

			if (angle_to_ship<0) angle_to_ship+=6.283184F;
			if (best_alpha<0) best_alpha+=6.283184F;

			if (float(fabs(angle_to_ship-best_alpha))>1.0F &&
				float(fabs(angle_to_ship-best_alpha))<5.283184F) {
				float d=angle_to_ship-best_alpha;

				if (d>0 && d<3.141592F) best_alpha=angle_to_ship-1.0F;
								   else best_alpha=angle_to_ship+1.0F;
			} /* if */ 

			desired_turret_angle=int((best_alpha*180)/3.141592F);
			radians=best_alpha;
		} /* if */ 

		if (desired_turret_angle<0) desired_turret_angle+=360;
		if (desired_turret_angle>=360) desired_turret_angle-=360;
		for(i=0;i<2;i++) {
			if (turret_angle<0) turret_angle+=360;
			if (turret_angle>=360) turret_angle-=360;
			if (desired_turret_angle!=turret_angle) {
				if (((desired_turret_angle-turret_angle)<180 &&
					 (desired_turret_angle-turret_angle)>0) ||
					(desired_turret_angle-turret_angle<-180)) {
					turret_angle++;
				} else {
					turret_angle--;
				} /* if */ 
			} /* if */ 
		} /* for */ 
		if (turret_angle<0) turret_angle+=360;
		if (turret_angle>=360) turret_angle-=360;

		switch(direction) {
		case 0: /* up */ 
			if (turret_angle>=345 || turret_angle<90) turret_angle=345;
			if (turret_angle<205) turret_angle=205;
			break;
		case 1: /* down */ 
			if (turret_angle<15 || turret_angle>=270) turret_angle=15;
			if (turret_angle>=175) turret_angle=175;
			break;
		case 2: /* right */ 
			if (turret_angle>=75 && turret_angle<180) turret_angle=75;
			if (turret_angle>=180 && turret_angle<285) turret_angle=285;
			break; 
		case 3: /* left */ 
			if (turret_angle<105) turret_angle=105;
			if (turret_angle>=255) turret_angle=255;
			break;
		} /* switch */ 

		state++;

		if (state>=128) {
			if (turret_angle==desired_turret_angle) {
				if ((dx*dx)+(dy*dy)<30000) {
					/* Fire!: */ 
					ENEMY *e;
					e=new ENEMY();
					e->type=0;
					e->state=8;
					e->speed_x=int(cos(radians)*FACTOR);
					e->speed_y=int(sin(radians)*FACTOR);
					e->x=(x+8)*FACTOR+(e->speed_x*8);
					e->y=(y+8)*FACTOR+(e->speed_y*8);
					e->life=1;
					e->tile=344;

					enemies->Add(e);
					if (S_shot!=0) Mix_PlayChannel(-1,S_shot,0);
				} /* if */ 
			} /* if */ 
			state=0;
		} /* if */ 
	} else {
		return false;
	} /* if */ 

	return true;
} /* cycle_directionalcanon2 */ 

