#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "math.h"
#include "SDL/SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "sge.h"

#include "auxiliar.h"
#include "list.h"


#include "fonts.h"
#include "tiles.h"
#include "maps.h"
#include "transball.h"

#define FACTOR	512

extern int SCREEN_X,SCREEN_Y;
char buffer[256];
int fuelfactor[3]={64,64,96};
int shotfuel[3]={40,64,96};

extern SDLKey THRUST_KEY,ANTITHRUST_KEY,LEFT_KEY,RIGHT_KEY;
extern SDLKey FIRE_KEY,ATRACTOR_KEY;
extern SDLKey PAUSE_KEY;


TRANSBALL::TRANSBALL(char *gpath,char *spath,char *mpath,int fuelv,char *mapf,int ship_typev)
{
	int i;

	m_gpath=new char[strlen(gpath)+1];
	strcpy(m_gpath,gpath);

	m_spath=new char[strlen(spath)+1];
	strcpy(m_spath,spath);

	m_mpath=new char[strlen(mpath)+1];
	strcpy(m_mpath,mpath);

    tiles_sfc = IMG_Load(g_filename("tiles.png"));
    if (tiles_sfc==0) return;
    mask_sfc = IMG_Load(g_filename("tiles-mask.png"));
    if (mask_sfc==0) return;

	SDL_SetColorKey(tiles_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(tiles_sfc->format,0,0,0));
	SDL_SetColorKey(mask_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(mask_sfc->format,0,0,0));

	tiles=new TILE *[500];
	tiles_mask=new TILE *[500];
	for(i=0;i<500;i++) {
		int x,y;

		x=(i%20)*16;
		y=(i/20)*16;
		tiles[i]=new TILE(tiles_sfc,x,y,16,16);
		tiles_mask[i]=new TILE(mask_sfc,x,y,16,16);
	} /* for */ 

	S_start=0;
	S_shipshot=0;
	S_shot=0;
	S_switch=0;
	S_takeball=0;
	S_explosion=0;
	S_enemyhit=0;
	S_thrust=0;
	S_fuel=0;
	thrust_channel=-1;

	map=new TRANSBALL_MAP(m_filename(mapf));
	map->set_sounds(0,0,0,0);

	load_sounds();

	map_x=0;
	map_y=0;
	ship_type=ship_typev;
	ship_x=(map->get_sx()*8)*FACTOR;
	ship_y=(32)*FACTOR;
	ship_angle=0;
	ship_speed_x=0;
	ship_speed_y=0;
	ship_state=0;
	ship_anim=0;
	ship_atractor=0;
	atractor_particles=0;

	{
		int x=0,y=0;

		map->get_ball_position(&x,&y);

		ball_state=-32;
		ball_x=x*16*FACTOR;
		ball_y=(y*16-6)*FACTOR;
		ball_speed_x=0;
		ball_speed_y=0;
	}

	fuel=fuelv*fuelfactor[ship_type];

	fade_factor=0;
	fade_state=1;

	start=true;

	nstars=map->get_sx()*8;
	star_x=new int[nstars];
	star_y=new int[nstars];
	star_color=new int[nstars];

	for(i=0;i<nstars;i++) {
		star_color[i]=rand()%255;
		star_x[i]=rand()%(map->get_sx()*16);
		star_y[i]=rand()%(25600);
		star_y[i]=160-int(sqrt(star_y[i]));
	} /* for */ 

	fuel_used=0;
	n_shots=0;
	n_hits=0;
	enemies_destroyed=0;
} /* TRANSBALL */ 


TRANSBALL::~TRANSBALL()
{
	int i;

	delete star_x;
	delete star_y;
	delete star_color;
	star_color=0;
	star_x=0;
	star_y=0;
	nstars=0;

	delete map;
	for(i=0;i<240;i++) {
		delete tiles[i];
		delete tiles_mask[i];
		tiles[i]=0;
		tiles_mask[i]=0;
	} /* for */ 
	delete tiles;
	delete tiles_mask;

    SDL_FreeSurface(tiles_sfc);
	SDL_FreeSurface(mask_sfc);

	free_sounds();

} /* ~TRANSBALL */ 


void TRANSBALL::free_sounds()
{
	Mix_FreeChunk(S_start);
	Mix_FreeChunk(S_shipshot);
	Mix_FreeChunk(S_shot);
	Mix_FreeChunk(S_switch);
	Mix_FreeChunk(S_takeball);
	Mix_FreeChunk(S_explosion);
	Mix_FreeChunk(S_enemyhit);
	Mix_FreeChunk(S_thrust);
	Mix_FreeChunk(S_fuel);
	S_start=0;
	S_shipshot=0;
	S_shot=0;
	S_switch=0;
	S_takeball=0;
	S_explosion=0;
	S_enemyhit=0;
	S_thrust=0;
	S_fuel=0;
	thrust_channel=-1;
	map->set_sounds(0,0,0,0);
} /* free_sounds */ 


void TRANSBALL::load_sounds()
{
	S_start=Mix_LoadWAV(s_filename("start.wav"));
	S_shipshot=Mix_LoadWAV(s_filename("shipshot.wav"));
	S_shot=Mix_LoadWAV(s_filename("shot.wav"));
	S_switch=Mix_LoadWAV(s_filename("switch.wav"));
	S_takeball=Mix_LoadWAV(s_filename("takeball.wav"));
	S_explosion=Mix_LoadWAV(s_filename("explosion.wav"));
	S_enemyhit=Mix_LoadWAV(s_filename("enemyhit.wav"));
	S_thrust=Mix_LoadWAV(s_filename("thrust.wav"));
	S_fuel=Mix_LoadWAV(s_filename("fuel.wav"));
	thrust_channel=-1;
	if (map!=0) map->set_sounds(S_shot,S_explosion,S_enemyhit,S_switch);
} /* load_sounds */  



char *TRANSBALL::g_filename(char *file)
{
	sprintf(buffer,"%s%s",m_gpath,file);
	return buffer;
} /* g_filename */ 


char *TRANSBALL::s_filename(char *file)
{
	sprintf(buffer,"%s%s",m_spath,file);
	return buffer;
} /* s_filename */ 


char *TRANSBALL::m_filename(char *file)
{
	sprintf(buffer,"%s%s",m_mpath,file);
	return buffer;
} /* m_filename */ 


void TRANSBALL::render(SDL_Surface *screen,int sx,int sy)
{
	SDL_FillRect(screen,0,0);

	/* Stars: */ 
	{
		int i;

		for(i=0;i<nstars;i++) {
			int x,y;
			x=star_x[i]-map_x/2;
			y=star_y[i]-map_y/2;
			if (x>=0 && x<sx &&
				y>=0 && y<sy) {
				putpixel(screen,x,y,SDL_MapRGB(screen->format,star_color[i],star_color[i],star_color[i]));
			} /* if */ 
		} /* for */ 
	}

	/* Render Map: */ 
	{
		int dx=((ship_x/FACTOR)-(SCREEN_X/2))-map_x;
		int dy=((ship_y/FACTOR)-int(SCREEN_Y/2.4))-map_y;

		if (dx>8) dx=8;
		if (dx<-9) dx=-8;
		if (dy>8) dy=8;
		if (dy<-9) dy=-8;

		map_x+=dx;
		map_y+=dy;

		if (map_x>((map->get_sx()*16)-sx)) map_x=((map->get_sx()*16)-sx);
		if (map_x<0) map_x=0;

		if (map_y>((map->get_sy()*16)-sy)) map_y=((map->get_sy()*16)-sy);
		if (map_y<0) map_y=0;

		map->draw_map(screen,tiles,map_x,map_y,sx,sy);
	}

	/* Render ball: */ 
	if (ball_state<0) tiles[320]->draw((ball_x/FACTOR)-map_x,(ball_y/FACTOR)-map_y,screen);
				 else tiles[321]->draw((ball_x/FACTOR)-map_x,(ball_y/FACTOR)-map_y,screen);

	/* Render ship: */ 
	switch(ship_state) {
	case 0: /* NORMAL SHIP STATE: */ 
		{
			SDL_Surface *rotate_sfc=SDL_CreateRGBSurface(SDL_SRCALPHA,32,32,32,0,0,0,0);
			SDL_Surface *rotate_sfc2=SDL_CreateRGBSurface(SDL_SRCALPHA,64,64,32,0,0,0,0);
			SDL_Rect s,d;

			{
				int i;

				for(i=0;i<atractor_particles;i++) {
					if (atractor_p_color[i]==0) {
						int v=(rand()%192)+64;
						atractor_p_color[i]=SDL_MapRGB(screen->format,int(v*0.1),int(v*0.7),v);
					} /* if */ 
					putpixel(screen,(atractor_p_x[i]/FACTOR)-map_x,(atractor_p_y[i]/FACTOR)-map_y,atractor_p_color[i]);
				} /* for */ 
			}

			if (rotate_sfc==0) return;
			if (rotate_sfc2==0) return;

			/* Transparant color is BLACK: */ 
			SDL_SetColorKey(rotate_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(rotate_sfc->format,0,0,0));
			SDL_SetColorKey(rotate_sfc2,SDL_SRCCOLORKEY,SDL_MapRGB(rotate_sfc2->format,0,0,0));

			switch(ship_type) {
			case 0:
				if (ship_anim<6) {
					s.x=96+(ship_anim*32);
					s.y=272;
				} else {
					s.x=128+(ship_anim-6)*32;
					s.y=304;
				} /* if */ 
				break;
			case 1:
				s.x=32+(ship_anim*32);
				s.y=240;
				break;
			case 2:
				s.x=96+(ship_anim*32);
				s.y=336;
				break;
			} /* if */ 

			s.w=32;
			s.h=32;
			d.x=0;
			d.y=0;
			SDL_BlitSurface(tiles_sfc,&s,rotate_sfc,&d);
			SDL_FillRect(rotate_sfc2,0,0);
			sge_transform(rotate_sfc,rotate_sfc2, float(ship_angle), 0.5F, 0.5F, 16, 14, 32, 32, 0);
			d.x=((ship_x/FACTOR)-32)-map_x;
			d.y=((ship_y/FACTOR)-32)-map_y;
			SDL_BlitSurface(rotate_sfc2,0,screen,&d);
		
			SDL_FreeSurface(rotate_sfc);
			SDL_FreeSurface(rotate_sfc2);

		}
		break;
	case 1:
		{
			int frame=ship_anim/8;
			SDL_Rect s,d;

			if (frame<6) {
				s.x=(frame%2)*16;
				s.y=192+(frame/2)*16;
				s.w=16;
				s.h=16;
				d.x=((ship_x/FACTOR)-8)-map_x;
				d.y=((ship_y/FACTOR)-8)-map_y;
				SDL_BlitSurface(tiles_sfc,&s,screen,&d);
			} /* if */ 
		}
		break;
	} /* switch */ 

	/* SHIP BULLETS: */ 
	{
		SHIP_BULLET *b;

		bullets.Rewind();
		while(bullets.Iterate(b)) {
			switch(ship_type) {
			case 0:
				if (b->state<8) {
					tiles[242]->draw(b->x/FACTOR-map_x,b->y/FACTOR-map_y,screen);
				} else {
					tiles[399+(b->state/8)]->draw(b->x/FACTOR-map_x,b->y/FACTOR-map_y,screen);
				} /* if */ 
				break;
			case 1:
				tiles[242+(b->state/8)]->draw(b->x/FACTOR-map_x,b->y/FACTOR-map_y,screen);
				break;
			case 2:
				if (b->state<8) {
					SDL_Surface *rotate_sfc=SDL_CreateRGBSurface(SDL_SRCALPHA,16,16,32,0,0,0,0);
					SDL_Surface *rotate_sfc2=SDL_CreateRGBSurface(SDL_SRCALPHA,32,32,32,0,0,0,0);
					SDL_Rect s,d;
					float radians=float(atan2(b->speed_x,b->speed_y));

					if (rotate_sfc==0) return;
					if (rotate_sfc2==0) return;

					/* Transparant color is BLACK: */ 
					SDL_SetColorKey(rotate_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(rotate_sfc->format,0,0,0));
					SDL_SetColorKey(rotate_sfc2,SDL_SRCCOLORKEY,SDL_MapRGB(rotate_sfc2->format,0,0,0));

					s.x=64;
					s.y=288;
					s.w=16;
					s.h=16;
					d.x=0;
					d.y=0;
					SDL_BlitSurface(tiles_sfc,&s,rotate_sfc,&d);
					SDL_FillRect(rotate_sfc2,0,0);
					sge_transform(rotate_sfc,rotate_sfc2,180-((radians*180)/3.141592F), 1.0F, 1.0F, 8, 8, 16, 16, 0);
					d.x=(b->x/FACTOR)-8-map_x;
					d.y=(b->y/FACTOR)-8-map_y;
					SDL_BlitSurface(rotate_sfc2,0,screen,&d);
				
					SDL_FreeSurface(rotate_sfc);
					SDL_FreeSurface(rotate_sfc2);
				} else {
					tiles[379+(b->state/8)]->draw(b->x/FACTOR-map_x,b->y/FACTOR-map_y,screen);
				} /* if */ 
				break;
			} /* switch */ 
		} /* while */ 
	}

	/* STATUS: */ 
	{
		SDL_Rect r;
		float f;

		r.x=2;
		r.y=2;
		r.w=52;
		r.h=1;
		SDL_FillRect(screen,&r,SDL_MapRGB(screen->format,255,255,255));
		r.y=9;
		SDL_FillRect(screen,&r,SDL_MapRGB(screen->format,255,255,255));
		r.y=2;
		r.w=1;
		r.h=8;
		SDL_FillRect(screen,&r,SDL_MapRGB(screen->format,255,255,255));
		r.x=53;
		SDL_FillRect(screen,&r,SDL_MapRGB(screen->format,255,255,255));

		r.x=3;
		r.y=3;
		r.w=fuel/fuelfactor[ship_type];
		r.h=6;
		f=float(fuel)/(fuelfactor[ship_type]*30.0F);
		if (f>=1.0F) f=1.0F;
		SDL_FillRect(screen,&r,SDL_MapRGB(screen->format,int(255*(1-f*f)),int(200*sqrt(f)),0));
	}

	if (fade_factor!=1.0) {
		surface_fader(screen,fade_factor,fade_factor,fade_factor,-1,0);
	} /* if */ 

} /* render */ 


int TRANSBALL::cycle(unsigned char *keyboard)
{
	int i;

	if (start) {
		if (S_start!=0) Mix_PlayChannel(-1,S_start,0);
		start=false;
	} /* if */ 

	/* Fader: */ 
	if (fade_state==1) {
		fade_factor+=0.05F;
		if (fade_factor>=1.0F) {
			fade_factor=1.0F;
			fade_state=0;
		} /* if */ 
	} /* if */ 
	if (fade_state==2) {
		fade_factor-=0.05F;
		if (fade_factor<=0.0F) {
			fade_factor=0.0F;
			fade_state=0;
		} /* if */ 
	} /* if */ 

	switch(ship_state) {
	case 0: /* NORMAL SHIP STATE: */ 
		/* Ship control: */ 
		if (keyboard[LEFT_KEY]) {
			ship_angle-=4;
			if (ship_angle<0) ship_angle+=360;
		} /* if */ 

		if (keyboard[RIGHT_KEY]) {
			ship_angle+=4;
			if (ship_angle>36) ship_angle-=360;
		} /* if */ 

		if (keyboard[THRUST_KEY] && fuel>0) {
			float radian_angle=((ship_angle-90.0F)*3.141592F)/180.0F;
			switch(ship_type) {
			case 0:
				ship_speed_x+=int(cos(radian_angle)*24);
				ship_speed_y+=int(sin(radian_angle)*24);
				break;
			case 1:
				ship_speed_x+=int(cos(radian_angle)*18);
				ship_speed_y+=int(sin(radian_angle)*18);
				break;
			case 2:
				ship_speed_x+=int(cos(radian_angle)*11);
				ship_speed_y+=int(sin(radian_angle)*11);
				break;
			} /* switch */ 
			if (ship_speed_x>4*FACTOR) ship_speed_x=4*FACTOR;
			if (ship_speed_x<-4*FACTOR) ship_speed_x=-4*FACTOR;
			if (ship_speed_y>4*FACTOR) ship_speed_y=4*FACTOR;
			if (ship_speed_y<-4*FACTOR) ship_speed_y=-4*FACTOR;
			fuel--;
			fuel_used++;
			ship_anim++;
			if (ship_anim>=6) ship_anim=1;
			if (thrust_channel==-1 && S_thrust!=0) thrust_channel=Mix_PlayChannel(-1,S_thrust,-1);
		} else {
			if (!keyboard[ANTITHRUST_KEY] || ship_type!=0) {
				if (thrust_channel!=-1) Mix_HaltChannel(thrust_channel);
				thrust_channel=-1;
				ship_anim=0;
			} /* if */ 
		} /* if */ 

		if (keyboard[ANTITHRUST_KEY] && !keyboard[THRUST_KEY] && fuel>0) {
			float radian_angle=((ship_angle-90.0F)*3.141592F)/180.0F;
			switch(ship_type) {
			case 0:
				ship_speed_x-=int(cos(radian_angle)*8);
				ship_speed_y-=int(sin(radian_angle)*8);
				if (ship_speed_x>4*FACTOR) ship_speed_x=4*FACTOR;
				if (ship_speed_x<-4*FACTOR) ship_speed_x=-4*FACTOR;
				if (ship_speed_y>4*FACTOR) ship_speed_y=4*FACTOR;
				if (ship_speed_y<-4*FACTOR) ship_speed_y=-4*FACTOR;
				fuel--;
				fuel_used++;
				if (ship_anim<6) {
					ship_anim=6;
				} else {
					ship_anim++;
				} /* if */ 
				if (ship_anim>=11) ship_anim=6;
				if (thrust_channel==-1 && S_thrust!=0) thrust_channel=Mix_PlayChannel(-1,S_thrust,-1);
				break;
			case 1:
				break;
			case 2:
				break;
			} /* switch */ 
		} else {
			if (!keyboard[THRUST_KEY]) {
				if (thrust_channel!=-1) Mix_HaltChannel(thrust_channel);
				thrust_channel=-1;
				ship_anim=0;
			} /* if */ 
		} /* if */ 

		// Atractor:
		if (keyboard[ATRACTOR_KEY]) {
			ship_atractor++;
			if (ship_atractor>4) ship_atractor=1;

			if (atractor_particles<MAX_ATRACTOR_P) {
				atractor_p_x[atractor_particles]=ship_x+(rand()%(16*FACTOR))-8*FACTOR;
				atractor_p_y[atractor_particles]=ship_y+(rand()%(16*FACTOR))+16*FACTOR;
				atractor_p_speed[atractor_particles]=float(5+(rand()%5))/10.0F;
				atractor_p_color[atractor_particles]=0;
				atractor_particles++;
			} /* if */ 

		} else {
			ship_atractor=0;
			if (atractor_particles>0) atractor_particles-=8;
			if (atractor_particles<0) atractor_particles=0;
		} /* if */ 

		for(i=0;i<atractor_particles;i++) {
			atractor_p_x[i]+=int(ship_speed_x*0.9);
			atractor_p_y[i]+=int(ship_speed_y*0.9);
			atractor_p_x[i]=int(ship_x*(1.0-atractor_p_speed[i])+atractor_p_x[i]*atractor_p_speed[i]);
			atractor_p_y[i]=int(ship_y*(1.0-atractor_p_speed[i])+atractor_p_y[i]*atractor_p_speed[i]);
			if (abs(ship_x-atractor_p_x[i])<2*FACTOR &&
				abs(ship_y-atractor_p_y[i])<2*FACTOR) {
				atractor_p_x[i]=ship_x+(rand()%(16*FACTOR))-8*FACTOR;
				atractor_p_y[i]=ship_y+(rand()%(16*FACTOR))+16*FACTOR;
				atractor_p_speed[i]=float(5+(rand()%5))/10.0F;
				atractor_p_color[i]=0;
			} /* if */ 
		} /* for */ 

		if (keyboard[FIRE_KEY] && !old_keyboard[FIRE_KEY] && fuel>=shotfuel[ship_type]) {
			float radian_angle=((ship_angle-90.0F)*3.141592F)/180.0F;
			SHIP_BULLET *b=new SHIP_BULLET();

			n_shots++;

			b->x=ship_x-8*FACTOR;
			b->y=ship_y-8*FACTOR;
			switch(ship_type) {
			case 0:
				b->speed_x=int(cos(radian_angle)*4*FACTOR);
				b->speed_y=int(sin(radian_angle)*4*FACTOR);
				fuel-=shotfuel[ship_type];
				fuel_used+=shotfuel[ship_type];
				break;
			case 1:
				b->speed_x=int(cos(radian_angle)*3*FACTOR);
				b->speed_y=int(sin(radian_angle)*3*FACTOR);
				fuel-=shotfuel[ship_type];
				fuel_used+=shotfuel[ship_type];
				break;
			case 2:
				b->speed_x=int(cos(radian_angle)*2*FACTOR);
				b->speed_y=int(sin(radian_angle)*2*FACTOR);
				fuel-=shotfuel[ship_type];
				fuel_used+=shotfuel[ship_type];
				break;
			} /* switch */ 
			b->state=0;
			bullets.Add(b);
			if (S_shipshot!=0) Mix_PlayChannel(-1,S_shipshot,0);
		} /* if */ 
		break;
	case 1: /* SHIP EXPLODING */ 
		if (thrust_channel!=-1) Mix_HaltChannel(thrust_channel);
		thrust_channel=-1;
		ship_anim++;
		if (ship_anim>=64) fade_state=2;
		if (ship_anim>=96) return 1;
		break;
	} /* switch */ 

	if ((keyboard[SDLK_ESCAPE]) || (keyboard[51])){
		return 3;
	} /* if */ 

	/* Ship cinematics: */ 
	if (ship_speed_x>0) ship_speed_x--;
	if (ship_speed_x<0) ship_speed_x++;
	ship_speed_y+=2;

	if (ship_speed_x>4*FACTOR) ship_speed_x=4*FACTOR;
	if (ship_speed_x<-4*FACTOR) ship_speed_x=-4*FACTOR;
	if (ship_speed_y>4*FACTOR) ship_speed_y=4*FACTOR;
	if (ship_speed_y<-4*FACTOR) ship_speed_y=-4*FACTOR;
	ship_x+=ship_speed_x;
	ship_y+=ship_speed_y;

	if ((ship_x/FACTOR)<0) {
		ship_x=0;
		ship_speed_x=0;
	} /* if */ 
	if ((ship_y/FACTOR)<0) {
		ship_y=0;
		ship_speed_y=0;
	} /* if */ 
	if ((ship_x/FACTOR)>(map->get_sx()*16)) {
		ship_x=(map->get_sx()*16)*FACTOR;
		ship_speed_x=0;
	} /* if */ 
	if ((ship_y/FACTOR)>(map->get_sy()*16)) {
		ship_y=(map->get_sy()*16)*FACTOR;
		ship_speed_y=0;
	} /* if */ 

	/* Ball cinematics: */ 
	if (ball_speed_x>0) ball_speed_x--;
	if (ball_speed_x<0) ball_speed_x++;

	{
		int bx=(ball_x/FACTOR)+8;
		int by=(ball_y/FACTOR)+8;
		int sx=ship_x/FACTOR;
		int sy=(ship_y/FACTOR)+8;

		if (ship_atractor!=0 && 
			bx>sx-8 && bx<sx+8 &&
			by>sy && by<sy+32 &&
			ball_state<0) {
			ball_state++;
			if (ball_state==0) {
				if (S_takeball!=0) Mix_PlayChannel(-1,S_takeball,0);
				map->ball_taken();
			} /* if */ 
		} else {
			if (ball_state<0) ball_state=-32;
		} /* if */ 

		if (ball_state==0) {
			int xdif=(ball_x/FACTOR)-((ship_x/FACTOR));
			int ydif=(ball_y/FACTOR)-((ship_y/FACTOR));
			int totdif;
			xdif*=xdif;
			ydif*=ydif;
			totdif=xdif+ydif;
			if (totdif<10000) {
				if ((ship_x-8*FACTOR)<ball_x) ball_speed_x-=2;
				if ((ship_x-8*FACTOR)>ball_x) ball_speed_x+=2;
				if ((ship_y-8*FACTOR)<ball_y) ball_speed_y-=2;
				if ((ship_y-8*FACTOR)>ball_y) ball_speed_y+=2;
			} /* if */ 
			if (totdif<4000) {
				if ((ship_x-8*FACTOR)<ball_x) ball_speed_x-=2;
				if ((ship_x-8*FACTOR)>ball_x) ball_speed_x+=2;
				if ((ship_y-8*FACTOR)<ball_y) ball_speed_y-=2;
				if ((ship_y-8*FACTOR)>ball_y) ball_speed_y+=2;
			} /* if */ 
			if (totdif<1000) {
				if ((ship_x-8*FACTOR)<ball_x) ball_speed_x-=2;
				if ((ship_x-8*FACTOR)>ball_x) ball_speed_x+=2;
				if ((ship_y-8*FACTOR)<ball_y) ball_speed_y-=2;
				if ((ship_y-8*FACTOR)>ball_y) ball_speed_y+=2;
			} /* if */ 
			if (totdif<100) {
				if ((ship_x-8*FACTOR)<ball_x) ball_speed_x-=2;
				if ((ship_x-8*FACTOR)>ball_x) ball_speed_x+=2;
				if ((ship_y-8*FACTOR)<ball_y) ball_speed_y-=2;
				if ((ship_y-8*FACTOR)>ball_y) ball_speed_y+=2;
			} /* if */ 
		} /* if */ 

		bx=(ball_x/FACTOR);
		by=(ball_y/FACTOR);

		if (tile_map_collision(tiles_mask[360],bx,by)) {
			if (ball_speed_y>0) {
				ball_speed_y=int(-0.75*ball_speed_y);
				map->ball_collision(bx+8,by+12);
			} else {
				if (tile_map_collision(tiles_mask[360],bx,by-1)) ball_speed_y-=2;
			} /* if */ 
		} else {
			ball_speed_y+=2;
		} /* if */ 

		if (tile_map_collision(tiles_mask[340],bx,by)) {
			if (ball_speed_y<0) {
				ball_speed_y=int(-0.75*ball_speed_y);
				map->ball_collision(bx+8,by+4);
			} else {
				ball_speed_y+=2;
			} /* if */ 
		} /* if */ 

		if (tile_map_collision(tiles_mask[342],bx,by)) {
			if (ball_speed_x>0) {
				ball_speed_x=int(-0.75*ball_speed_x);
				map->ball_collision(bx+12,by+8);
			} else {
				ball_speed_x-=2;
			} /* if */ 
		} /* if */ 

		if (tile_map_collision(tiles_mask[362],bx,by)) {
			if (ball_speed_x<0) {
				ball_speed_x=int(-0.75*ball_speed_x);
				map->ball_collision(bx+4,by+8);
			} else {
				ball_speed_x+=2;
			} /* if */ 
		} /* if */ 
	}
	if (ball_speed_x>4*FACTOR) ball_speed_x=4*FACTOR;
	if (ball_speed_x<-4*FACTOR) ball_speed_x=-4*FACTOR;
	if (ball_speed_y>4*FACTOR) ball_speed_y=4*FACTOR;
	if (ball_speed_y<-4*FACTOR) ball_speed_y=-4*FACTOR;
	ball_x+=ball_speed_x;
	ball_y+=ball_speed_y;

	if ((ball_x/FACTOR)<0) {
		ball_x=0;
		ball_speed_x=0;
	} /* if */ 
	if ((ball_y/FACTOR)<0 && ball_state>=0) {
		fade_state=2;
		ball_speed_y=-FACTOR;
		ball_state++;
		if (ball_state>=32) return 2;
	} /* if */ 
	if ((ball_x/FACTOR)>((map->get_sx()-1)*16)) {
		ball_x=((map->get_sx()-1)*16)*FACTOR;
		ball_speed_x=0;
	} /* if */ 
	if ((ball_y/FACTOR)>((map->get_sy()-1)*16)) {
		ball_y=((map->get_sy()-1)*16)*FACTOR;
		ball_speed_y=0;
	} /* if */ 

	/* Bullets: */ 
	{
		List<SHIP_BULLET> deletelist;
		SHIP_BULLET *b;

		bullets.Rewind();
		while(bullets.Iterate(b)) {
 			if (b->state==0) {
				b->x+=b->speed_x;
				b->y+=b->speed_y;

				if (tile_map_collision(tiles_mask[242],b->x/FACTOR,b->y/FACTOR)) {
					int ship_strength[3]={1,2,4};
					int retv;

					b->state++;
					retv=map->shipbullet_collision((b->x/FACTOR)+8,(b->y/FACTOR)+8,ship_strength[ship_type]);
					if (retv!=0) n_hits++;
					if (retv==2) enemies_destroyed++;
				} else {
					if (b->x<-8*FACTOR || b->x>(map->get_sx()*16*FACTOR)+8*FACTOR ||
						b->y<-8*FACTOR || b->y>(map->get_sy()*16*FACTOR)+8*FACTOR) deletelist.Add(b);
				} /* if */ 
			} else {
				b->state++;
				if (b->state>=40) deletelist.Add(b);
			} /* if */ 
		} /* while */ 

		while(!deletelist.EmptyP()) {
			b=deletelist.ExtractIni();
			bullets.DeleteElement(b);
			delete b;
		} /* while */ 
	}

	/* Map Cycle: */ 
	map->cycle(ship_x/FACTOR,ship_y/FACTOR,ship_speed_x,ship_speed_y,tiles_mask);

	/* Collision detection: */ 
	if (ship_state==0 && ship_map_collision()) {
		ship_speed_x/=4;
		ship_speed_y/=4;
		ship_state=1;
		ship_anim=0;
		if (S_explosion!=0) Mix_PlayChannel(-1,S_explosion,0);
	} /* if */ 

	/* Fuel recharge: */ 
	if (map->ship_in_fuel_recharge(ship_x/FACTOR,ship_y/FACTOR)) {
		int old_fuel=fuel;
		fuel+=8;
		if (fuel>50*fuelfactor[ship_type]) fuel=50*fuelfactor[ship_type];
		if ((old_fuel/fuelfactor[ship_type])<((fuel-1)/fuelfactor[ship_type])) Mix_PlayChannel(-1,S_fuel,0);
	} /* if */ 

	for(i=0;i<SDLK_LAST;i++) old_keyboard[i]=keyboard[i];
	return 0;
} /* cycle */ 


bool TRANSBALL::ship_map_collision(void)
{
	bool collision=false;
	SDL_Surface *rotate_sfc=SDL_CreateRGBSurface(SDL_SRCALPHA,32,32,32,0,0,0,0);
	SDL_Surface *rotate_sfc2=SDL_CreateRGBSurface(SDL_SRCALPHA,64,64,32,0,0,0,0);
	SDL_Surface *map_sfc=SDL_CreateRGBSurface(SDL_SRCALPHA,64,64,32,0,0,0,0);
	SDL_Rect s,d;

	if (rotate_sfc==0) return false;
	if (rotate_sfc2==0) return false;
	if (map_sfc==0) return false;

	SDL_SetColorKey(rotate_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(rotate_sfc->format,0,0,0));
	SDL_SetColorKey(rotate_sfc2,SDL_SRCCOLORKEY,SDL_MapRGB(rotate_sfc2->format,0,0,0));
	SDL_SetColorKey(map_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(map_sfc->format,0,0,0));

	switch(ship_type) {
	case 0:
		if (ship_anim<6) {
			s.x=96+(ship_anim*32);
			s.y=272;
		} else {
			s.x=128+(ship_anim-6)*32;
			s.y=304;
		} /* if */ 
		break;
	case 1:
		s.x=32+(ship_anim*32);
		s.y=240;
		break;
	case 2:
		s.x=96+(ship_anim*32);
		s.y=336;
		break;
	} /* if */ 

	s.w=32;
	s.h=32;
	d.x=0;
	d.y=0;
	SDL_BlitSurface(mask_sfc,&s,rotate_sfc,&d);
	SDL_FillRect(rotate_sfc2,0,0);
	sge_transform(rotate_sfc,rotate_sfc2, float(ship_angle), 0.5F, 0.5F, 16, 14, 32, 32, 0);
	d.x=((ship_x/FACTOR)-32);
	d.y=((ship_y/FACTOR)-32);

	map->draw_map(map_sfc,tiles_mask,d.x,d.y,64,64);

	/* Collision detection: */ 
	{
		sge_cdata *ship_c=sge_make_cmap(rotate_sfc2);
		sge_cdata *back_c=sge_make_cmap(map_sfc);

		collision=(sge_cmcheck(ship_c,0,0,back_c,0,0) ? true:false);
		if (collision) {
			collision=true;
		} /* if */ 

		sge_destroy_cmap(ship_c);
		sge_destroy_cmap(back_c);
	}

	SDL_FreeSurface(rotate_sfc);
	SDL_FreeSurface(rotate_sfc2);
	SDL_FreeSurface(map_sfc);

	return collision;
} /* ship_map_collision */ 


bool TRANSBALL::tile_map_collision(TILE *t,int bx,int by)
{
	bool collision=false;
	SDL_Surface *tile_sfc=SDL_CreateRGBSurface(SDL_SRCALPHA,32,32,32,0,0,0,0);
	SDL_Surface *map_sfc=SDL_CreateRGBSurface(SDL_SRCALPHA,32,32,32,0,0,0,0);

	if (tile_sfc==0) return false;
	if (map_sfc==0) return false;

	SDL_SetColorKey(tile_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(tile_sfc->format,0,0,0));
	SDL_SetColorKey(map_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(map_sfc->format,0,0,0));

	t->draw(8,8,tile_sfc);
	map->draw_map(map_sfc,tiles_mask,bx-8,by-8,32,32);

	/* Collision detection: */ 
	{
		sge_cdata *tile_c=sge_make_cmap(tile_sfc);
		sge_cdata *back_c=sge_make_cmap(map_sfc);

		collision=(sge_cmcheck(tile_c,0,0,back_c,0,0) ? true:false);

		sge_destroy_cmap(tile_c);
		sge_destroy_cmap(back_c);
	}

	SDL_FreeSurface(tile_sfc);
	SDL_FreeSurface(map_sfc);

	return collision;
} /* ball_map_collision */ 



int TRANSBALL::get_statistics(int num)
{
	switch(num) {
	case 0: /* TIME */ 
		return 0;
		break;

	case 1: /* FUEL USED */ 
		return fuel_used;
		break;

	case 2: /* FUEL REMAINING */ 
		return fuel;
		break;

	case 3: /* SHOTS: */ 
		return n_shots;
		break;

	case 4: /* HITS: */ 
		return n_hits;
		break;

	case 5: /* ENEMIES DESTROYED: */ 
		return enemies_destroyed;
		break;
	} /* switch */ 

	return -1;
} /* TRANSBALL::get_statistics */ 

