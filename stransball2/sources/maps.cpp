#include <stdio.h>
#include <stdlib.h>
#include "SDL/SDL.h"
#include "SDL_mixer.h"
#include "sge.h"

#include "auxiliar.h"
#include "list.h"

#include "tiles.h"
#include "maps.h"

#define EMPTY_ROWS	8
#define FACTOR	512


TRANSBALL_MAP::TRANSBALL_MAP(char *file)
{
	FILE *fp;
	int i;
	int switchnumber=1;

	S_shot=0;
	S_explosion=0;
	S_enemyhit=0;
	S_switch=0;

	fp=fopen(file,"r");
	if (fp==0) {
		sx=0;
		sy=0;
		map=0;
		return;
	} /* if */ 
	
	fscanf(fp,"%i %i",&sx,&sy);
	sy+=EMPTY_ROWS;

	map=new int[sx*sy];

	for(i=0;i<sx*EMPTY_ROWS;i++) map[i]=-1;
	for(i=sx*EMPTY_ROWS;i<sx*sy;i++) {
		fscanf(fp,"%i",&(map[i]));
		map[i]--;
	} /* for */ 

	/* Look for enemies, doors, etc.: */ 
	for(i=0;i<sx*sy;i++) {

		/* ENEMIES: */ 
		if ((map[i]>=176 && map[i]<180) || (map[i]>=196 && map[i]<200) ||
			(map[i]>=216 && map[i]<220) || (map[i]>=236 && map[i]<240) ||
			(map[i]==154 || map[i]==155 || map[i]==174 || map[i]==175) ||
			(map[i]==386 || map[i]==387 || map[i]==406 || map[i]==407)) {
			int x,y,direction;

			x=(i%sx)*16;
			y=(i/sx)*16;

			if (map[i]==176 || map[i]==216 || map[i]==236 || map[i]==196 || map[i]==154 || map[i]==386) direction=0;
			if (map[i]==177 || map[i]==217 || map[i]==237 || map[i]==197 || map[i]==155 || map[i]==387) direction=1;
			if (map[i]==178 || map[i]==218 || map[i]==238 || map[i]==198 || map[i]==174 || map[i]==406) direction=2;
			if (map[i]==179 || map[i]==219 || map[i]==239 || map[i]==199 || map[i]==175 || map[i]==407) direction=3;

			if ((map[i]>=176 && map[i]<180) ||
				(map[i]>=216 && map[i]<220) || (map[i]>=236 && map[i]<240)) {
				/* CANON: */ 
				ENEMY *e;
				e=new ENEMY();
				e->type=1;
				e->state=0;
				e->life=4;
				e->x=x;
				e->y=y;
				e->direction=direction;
				enemies.Add(e);
			} /* if */ 

			if (map[i]>=196 && map[i]<200) {
				/* FAST CANON: */ 
				ENEMY *e;
				e=new ENEMY();
				e->type=2;
				e->state=0;
				e->life=8;
				e->x=x;
				e->y=y;
				e->direction=direction;
				enemies.Add(e);
			} /* if */ 

			if (map[i]==154 || map[i]==155 || map[i]==174 || map[i]==175) {
				/* DIRECTIONAL CANON: */ 
				ENEMY *e;
				e=new ENEMY();
				e->type=3;
				e->state=i%128;
				e->life=12;
				e->x=x;
				e->y=y;
				e->direction=direction;
				e->turret_angle=0;
				enemies.Add(e);
			} /* if */ 
			if (map[i]==386 || map[i]==387 || map[i]==406 || map[i]==407) {
				/* DIRECTIONAL CANON 2: */ 
				ENEMY *e;
				e=new ENEMY();
				e->type=7;
				e->state=i%128;
				e->life=12;
				e->x=x;
				e->y=y;
				e->direction=direction;
				e->tank_angle=0;
				e->turret_angle=0;
				enemies.Add(e);
			} /* if */ 
		} /* if */ 

		/* DOORS: */ 
		if (map[i]==113) {
			DOOR *d;

			d=new DOOR();
			d->x=i%sx;
			d->y=i/sx;
			d->action=0;
			fscanf(fp,"%i %i",&(d->state),&(d->event));
			doors.Add(d);
		} /* if */ 

		/* SWITCHES: */ 
		if ((map[i]>=116 && map[i]<120) || 
			(map[i]>=136 && map[i]<140) ||
			(map[i]>=156 && map[i]<160)) {
			SWITCH *s;

			s=new SWITCH();
			s->x=i%sx;
			s->y=i/sx;
			s->number=switchnumber++;
			s->state=0;
			switches.Add(s);
		} /* if */ 

		/* FUEL RECHARGES: */ 
		if (map[i]==132) {
			FUELRECHARGE *f;

			f=new FUELRECHARGE();
			f->x=i%sx;
			f->y=i/sx;
			fuel_recharges.Add(f);
		} /* if */ 
	} /* for */ 


	/* Tanks: */ 
	{
		int ntanks;

		fscanf(fp,"%i",&ntanks);
		for(i=0;i<ntanks;i++) {
			ENEMY *e;
			int x,y,type;

			fscanf(fp,"%i %i %i",&x,&y,&type);
			x*=16;
			y*=16;

			e=new ENEMY();
			e->type=4;
			e->state=1;
			e->state2=0;
			e->x=x;
			e->y=y;
			e->life=10;
			e->tank_type=type;
			e->tank_angle=0;
			e->turret_angle=90;
			enemies.Add(e);
		} /* for */ 
	}

	background_type=0,
	fscanf(fp,"%i",&background_type);

	fclose(fp);

	animtimer=0;
	animflag=0;
} /* TRANSBALL_MAP */ 


TRANSBALL_MAP::~TRANSBALL_MAP()
{
	delete[] map;
} /* ~TRANSBALL_MAP */ 


void TRANSBALL_MAP::set_sounds(Mix_Chunk *S_shotv,Mix_Chunk *S_explosionv,Mix_Chunk *S_enemyhitv,Mix_Chunk *S_switchv)
{
	S_shot=S_shotv;
	S_explosion=S_explosionv;
	S_enemyhit=S_enemyhitv;
	S_switch=S_switchv;
} /* set_sounds */ 


void TRANSBALL_MAP::get_ball_position(int *x,int *y)
{
	int i;

	for(i=0;i<sx*sy;i++) {
		if (map[i]==110) {
			*x=i%sx;
			*y=i/sx;
			return;
		} /* if */ 
	} /* for */ 
} /* get_ball_position */ 



void TRANSBALL_MAP::draw_map(SDL_Surface *surface,TILE **tiles,int x,int y,int ww,int wh)
{
	draw_map_enemy(surface,tiles,x,y,ww,wh,0);
} /* draw_map */ 

void TRANSBALL_MAP::draw_map_noenemies(SDL_Surface *surface,TILE **tiles,int x,int y,int ww,int wh)
{
	int step_x=0,step_y=0;
	int act_x,act_y;
	int i,j;

	if (tiles==0) return;

	step_x=tiles[0]->get_sx();
	step_y=tiles[0]->get_sy();

	/* Draw Background: */ 
	for(j=0,act_y=-int(y*0.75F);j<sy;j++,act_y+=step_y) {
		if (act_y>-step_y && act_y<wh) {
			for(i=0,act_x=-int(x*0.75F);i<sx;i++,act_x+=step_x) {
				if (act_x>-step_x && act_x<ww) {
					switch(background_type) {
					case 0:
						if (j==10) tiles[294]->draw(act_x,act_y,surface);
						if (j>10) tiles[314]->draw(act_x,act_y,surface);
						break;
					case 1:
						if (j==10) tiles[295]->draw(act_x,act_y,surface);
						if (j>10) tiles[315]->draw(act_x,act_y,surface);
						break;
					case 2:
						if (j==10) tiles[335]->draw(act_x,act_y,surface);
						if (j>10) {
							if (((j>>1)&0x03)==0) {
								if (animflag<2) {
									int t[12]={316,317,318,319,
											   336,337,338,339,
													   358,359,
													   378,379};
									int step;
									step=(animtimer+animflag*24)/4;
									if (step>11) step=11;
									tiles[t[step]]->draw(act_x,act_y,surface);
								} else {
									tiles[316]->draw(act_x,act_y,surface);
								} /* if */ 
							} else {
								tiles[275]->draw(act_x,act_y,surface);
							} /* if */ 
						} /* if */ 
						break;
					} /* switch */ 
				} /* if */ 
			} /* for */ 
		} /* if */ 
	} /* for */ 

	/* Draw map: */ 
	for(j=0,act_y=-y;j<sy;j++,act_y+=step_y) {
		if (act_y>=-step_y && act_y<wh) {
			for(i=0,act_x=-x;i<sx;i++,act_x+=step_x) {
				if (act_x>=-step_x && act_x<ww) {
					int piece=-1;
					
					if (j>=0) piece=map[i+j*sx];

					piece=animpiece(piece);
					if (piece>=0) {
						if (piece==113 || piece==114) {
							/* Door: */ 
							DOOR *d;
							List<DOOR> l;
							int state=0;

							l.Instance(doors);
							l.Rewind();
							while(l.Iterate(d)) {
								if ((d->x==i || d->x==i-1) && d->y==j) state=d->state;
							} /* while */ 

							if (piece==113) {
								tiles[piece]->draw_with_offset(act_x,act_y,surface,-state);
							} else {
								tiles[piece]->draw_with_offset(act_x,act_y,surface,state);
							} /* if */ 
						} else {
							if ((piece>=116 && piece<120) || 
								(piece>=136 && piece<140) ||
								(piece>=156 && piece<160)) {
								/* Switch: */ 
								SWITCH *s;
								List<SWITCH> l;

								l.Instance(switches);
								l.Rewind();
								while(l.Iterate(s)) {
									if (s->x==i && s->y==j) {
										if (s->state!=0) {
											tiles[piece+140]->draw(act_x,act_y,surface);
										} else {
											tiles[piece]->draw(act_x,act_y,surface);
										} /* if */ 
									} /* if */ 
								} /* while */ 
							} else {
								tiles[piece]->draw(act_x,act_y,surface);
							} /* if */ 
						} /* if */ 

					} /* if */ 
				} /* if */ 
			} /* for */ 
		} /* if */ 
	} /* for */ 

	{
		List<SMOKE> l;
		SMOKE *s;
		SDL_Rect r;
		SDL_Surface *smoke_sfc=SDL_CreateRGBSurface(SDL_SRCALPHA,16,16,32,0,0,0,0);
		SDL_SetColorKey(smoke_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(smoke_sfc->format,0,0,0));

		l.Instance(smokes);
		l.Rewind();
		while(l.Iterate(s)) {
			int tile;
			int alpha;

			tile=((s->timer)>>3)%3;
			tiles[272+tile]->draw(0,0,smoke_sfc);
			r.x=(s->x/FACTOR)-x;
			r.y=(s->y/FACTOR)-y;
			if (r.x>-16 && r.x<ww &&
				r.y>-16 && r.y<wh) {
				alpha=255-s->timer;
				alpha=(alpha*alpha)/(255);
				if (alpha<0) alpha=0;
				if (alpha>255) alpha=255;
				SDL_SetAlpha(smoke_sfc,SDL_SRCALPHA,alpha);
				SDL_BlitSurface(smoke_sfc,0,surface,&r);
			} /* if */ 
		} /* while */ 

		SDL_FreeSurface(smoke_sfc);
	}

} /* draw_map_noenemies */ 


void TRANSBALL_MAP::draw_map_enemy(SDL_Surface *surface,TILE **tiles,int x,int y,int ww,int wh,ENEMY *enemy)
{
	ENEMY *e;
	List<ENEMY> l;

	if (tiles==0) return;

	draw_map_noenemies(surface,tiles,x,y,ww,wh);

	l.Instance(enemies);
	l.Rewind();
	while(l.Iterate(e)) {
		if (e!=enemy) {
			switch(e->type) {
			case 0:
				if (e->x>(-16+x)*FACTOR && e->x<(ww+x)*FACTOR &&
					e->y>(-16+y)*FACTOR && e->y<(wh+y)*FACTOR) e->draw_bullet(surface,tiles,x,y);
				break;
			case 3:
				if (e->x>(-16+x) && e->x<(ww+x) &&
					e->y>(-16+y) && e->y<(wh+y)) e->draw_directionalcanon(surface,tiles,map[(e->x)/16+(e->y/16)*sx],x,y);
				break;
			case 4:
				if (e->x>(-32+x) && e->x<(ww+x+32) &&
					e->y>(-32+y) && e->y<(wh+y+32)) e->draw_tank(surface,tiles,x,y);
				break;
			case 5:
				if (e->x>(-32+x) && e->x<(ww+x+32) &&
					e->y>(-32+y) && e->y<(wh+y+32)) e->draw_destroyedtank(surface,tiles,x,y);
				break;
			case 6:
				if (e->x>(-16+x) && e->x<(ww+x) &&
					e->y>(-16+y) && e->y<(wh+y)) e->draw_explosion(surface,tiles,x,y);
				break;
			case 7:
				if (e->x>(-16+x) && e->x<(ww+x) &&
					e->y>(-16+y) && e->y<(wh+y)) e->draw_directionalcanon2(surface,tiles,map[(e->x)/16+(e->y/16)*sx],x,y);
				break;
			} /* switch */ 
		} /* if */ 
	} /* while */ 

} /* draw_map_enemy */ 


int TRANSBALL_MAP::animpiece(int piece)
{
	if (piece<0) return piece;

	if (piece==110 && animtimer>16) return 111;
	if (piece==110 && animtimer>8) return 112;

	if (piece==64 && animtimer>16) return 66;
	if (piece==64 && animtimer>8) return 65;

	if (piece==67 && animflag&0x01==1) return 69;
	if (piece==68 && animflag&0x01==1) return 70;

	if (piece==26 && animtimer>12 && (animflag&0x01)==0) return 24;
	if (piece==26 && animtimer>12 && (animflag&0x01)==1) return 25;

	if (piece==146 && animtimer>12 && (animflag&0x01)==0) return 144;
	if (piece==146 && animtimer>12 && (animflag&0x01)==1) return 145;

	if (piece==27 && (animflag&0x01)==1) return 28;
	if (piece==147 && (animflag&0x01)==1) return 148;

	if (piece==115 && animflag>3) return -1;
	if (piece==130 && animflag>3) return -1;

	if (piece==32 && animflag>3) return 30;
	if (piece==33 && animflag>3) return 31;
	if (piece==36 && animflag>3) return 34;
	if (piece==37 && animflag>3) return 35;

	if (piece==422 && animflag>3) return 420;
	if (piece==423 && animflag>3) return 421;

	if (piece==162 && animflag>3) return 160;
	if (piece==163 && animflag>3) return 161;
	if (piece==166 && animflag>3) return 164;
	if (piece==167 && animflag>3) return 165;

	if (piece==76 && animtimer<=12 && (animflag&0x03)==0) return -1;
	if (piece==76 && animtimer>12 && (animflag&0x03)==0)  return 79;
	if (piece==76 && animtimer<=12 && (animflag&0x03)==1) return 78;
	if (piece==76 && animtimer>12 && (animflag&0x03)==1)  return 77;
	if (piece==76 && animtimer>12 && (animflag&0x03)==2)  return 77;
	if (piece==76 && animtimer<=12 && (animflag&0x03)==3) return 78;
	if (piece==76 && animtimer>12 && (animflag&0x03)==3)  return 79;

	if (piece==150 && animtimer<=12 && (animflag&0x03)==0) return -1;
	if (piece==150 && animtimer>12 && (animflag&0x03)==0)  return 153;
	if (piece==150 && animtimer<=12 && (animflag&0x03)==1) return 152;
	if (piece==150 && animtimer>12 && (animflag&0x03)==1)  return 151;
	if (piece==150 && animtimer>12 && (animflag&0x03)==2)  return 151;
	if (piece==150 && animtimer<=12 && (animflag&0x03)==3) return 152;
	if (piece==150 && animtimer>12 && (animflag&0x03)==3)  return 153;


	return piece;
} /* animpiece */ 


void TRANSBALL_MAP::cycle(int ship_x,int ship_y,int ship_speed_x,int ship_speed_y,TILE **masks)
{
	ENEMY *e;
	List<ENEMY> newenemies,enemiestodelete;
	List<ENEMY> l;

	/* Tile animations: */ 
	animtimer++;
	if (animtimer>24) {
		animflag++;
		if (animflag<0 || animflag>7) animflag=0;
		animtimer=0;
		}

	/* Enemies: */ 
	{
		SDL_Surface *enemy_sfc=SDL_CreateRGBSurface(SDL_SRCALPHA,64,64,32,0,0,0,0);
		SDL_Surface *back_sfc=SDL_CreateRGBSurface(SDL_SRCALPHA,64,64,32,0,0,0,0);

		if (enemy_sfc==0) return;
		if (back_sfc==0) return;

		SDL_SetColorKey(enemy_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(enemy_sfc->format,0,0,0));
		SDL_SetColorKey(back_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(back_sfc->format,0,0,0));

		l.Instance(enemies);
		l.Rewind();
		while(l.Iterate(e)) {
			bool collision=false;

			switch(e->type) {
			case 0:
				/* BULLET: */ 
				SDL_FillRect(enemy_sfc,0,0);
				SDL_FillRect(back_sfc,0,0);
				e->draw_bullet(enemy_sfc,masks,(e->x/FACTOR)-32,(e->y/FACTOR)-32);
				draw_map_enemy(back_sfc,masks,(e->x/FACTOR)-32,(e->y/FACTOR)-32,64,64,e);
				/* Collision detection: */ 
				{
					sge_cdata *enemy_c=sge_make_cmap(enemy_sfc);
					sge_cdata *back_c=sge_make_cmap(back_sfc);

					collision=(sge_cmcheck(enemy_c,0,0,back_c,0,0) ? true:false);

					sge_destroy_cmap(enemy_c);
					sge_destroy_cmap(back_c);
				}
				if (!e->cycle_bullet(sx*16,sy*16,collision)) {
					enemiestodelete.Add(e);
				} /* if */ 
				break;
			case 1:
				if (!e->cycle_canon(ship_x,ship_y,&newenemies,S_shot)) {
					enemiestodelete.Add(e);
				} /* if */ 
				break;
			case 2:
				if (!e->cycle_fastcanon(ship_x,ship_y,&newenemies,S_shot)) {
					enemiestodelete.Add(e);
				} /* if */ 
				break;
			case 3:
				if (!e->cycle_directionalcanon(ship_x,ship_y,&newenemies,S_shot)) {
					enemiestodelete.Add(e);
				} /* if */ 
				break;
			case 4:
				{
					int i;
					int gdist1=-1,gdist2=-1;
					bool lcol=false,rcol=false;
					/* TANK: */ 
					SDL_FillRect(enemy_sfc,0,0);
					SDL_FillRect(back_sfc,0,0);
					e->draw_tank(enemy_sfc,masks,e->x-32,e->y-32);
					draw_map_noenemies(back_sfc,masks,e->x-32,e->y-32,64,64);

					/* Compute the distance of the tracks to the ground: */ 
					for(i=32;i<64 && (gdist1==-1 || gdist2==-1);i++) {
						if (getpixel(back_sfc,24,i)!=0 && gdist1==-1) gdist1=i-45;
						if (getpixel(back_sfc,40,i)!=0 && gdist2==-1) gdist2=i-45;
					} /* for */ 
					if (gdist1==-1) gdist1=19;
					if (gdist2==-1) gdist2=19;
					if (getpixel(back_sfc,16,28)!=0) lcol=true;
					if (getpixel(back_sfc,48,28)!=0) rcol=true;

					e->cycle_tank(ship_x,ship_y,ship_speed_x,ship_speed_y,gdist1,gdist2,lcol,rcol,&newenemies,S_shot);
				}
				break;
			case 5:
				if (e->state<48) e->state++;
				break;
			case 6:
				if (!e->cycle_explosion()) {
					enemiestodelete.Add(e);
				} /* if */ 
				break;
			case 7:
				if (!e->cycle_directionalcanon2(ship_x,ship_y,ship_speed_x,ship_speed_y,&newenemies,S_shot)) {
					enemiestodelete.Add(e);
				} /* if */ 
				break;

			} /* switch */ 

		} /* while */ 

		SDL_FreeSurface(enemy_sfc);
		SDL_FreeSurface(back_sfc);
	}

	while(!enemiestodelete.EmptyP()) {
		e=enemiestodelete.ExtractIni();
		enemies.DeleteElement(e);
		delete e;
	} /* while */ 

	while(!newenemies.EmptyP()) {
		e=newenemies.ExtractIni();
		enemies.Insert(e);
	} /* while */ 

	/* Doors: */ 
	{
		DOOR *d;
		doors.Rewind();
		while(doors.Iterate(d)) {
			if (d->action==-1) {
				if (d->state>0) d->state--;
						   else d->action=0;
			} /* if */ 
			if (d->action==1) {
				if (d->state<14) d->state++;
						   else d->action=0;
			} /* if */ 
		} /* while */ 
	}

	/* Switches: */ 
	{
		SWITCH *s;
		switches.Rewind();
		while(switches.Iterate(s)) {
			if (s->state>0) s->state--;
		} /* while */ 
	}

	/* Smoke: */ 
	{
		List<SMOKE> l,todelete;
		List<SMOKESOURCE> l2,todelete2;
		SMOKESOURCE *ss;
		SMOKE *s;

		l2.Instance(smokesources);
		l2.Rewind();
		while(l2.Iterate(ss)) {
			ss->timer++;
			if (ss->timer>256) {
				todelete2.Add(ss);
			} else {
				int chance;

				chance=ss->timer;
				chance=(chance*chance)/256;
				chance/=16;

				if ((rand()%(chance+2))==0) {
					SMOKE *s;
			
					s=new SMOKE();
					s->x=ss->x*FACTOR;
					s->y=ss->y*FACTOR;
					s->speed_x=((rand()%(1+FACTOR/16))-(FACTOR/32))+ss->speed_x;
					s->speed_y=((rand()%(1+FACTOR/16))-(FACTOR/32))+ss->speed_y;
					s->desired_x=(rand()%(FACTOR/4))-FACTOR/8;
					s->desired_y=((rand()%(1+FACTOR/4))-(FACTOR/8))-FACTOR/4;
					s->timer=0;
					smokes.Add(s);
				} /* if */ 

			} /* if */ 
		} /* while */ 

		while(!todelete2.EmptyP()) {
			ss=todelete2.ExtractIni();
			smokesources.DeleteElement(ss);
			delete ss;
		} /* while */ 

		l.Instance(smokes);
		l.Rewind();
		while(l.Iterate(s)) {
			s->timer++;
			s->x+=s->speed_x;
			s->y+=s->speed_y;
			if (s->speed_x>s->desired_x) s->speed_x-=2;
			if (s->speed_x<s->desired_x) s->speed_x+=2;
			if (s->speed_y>s->desired_y) s->speed_y-=1;
			if (s->speed_y<s->desired_y) s->speed_y+=1;
			if (s->timer>255 || s->y<-8*FACTOR) {
				todelete.Add(s);
			} /* if */ 
		} /* while */ 

		while(!todelete.EmptyP()) {
			s=todelete.ExtractIni();
			smokes.DeleteElement(s);
			delete s;
		} /* while */ 
	}

} /* cycle */ 


int TRANSBALL_MAP::shipbullet_collision(int x,int y,int strength)
{
	ENEMY *e;
	List<ENEMY> l;
	ENEMY *selected=0;
	int mindistance=-1;
	int tolerance;
	int retval=0;

	l.Instance(enemies);
	l.Rewind();
	while(l.Iterate(e)) {
		int ex,ey,distance;
		ex=e->x;
		ey=e->y;
		if (e->type==0) {
			ex/=FACTOR;
			ey/=FACTOR;
		} /* if */ 
		if (e->type==1 ||
			e->type==2 || e->type==3 || e->type==7) {
			ex+=8;
			ey+=8;
		} /* if */ 
		distance=(x-ex)*(x-ex)+(y-ey)*(y-ey);

		tolerance=100;
		if (e->type==3 || e->type==7 || (e->type==4 && e->tank_type==3)) tolerance=200;

		if (((mindistance==-1 && distance<tolerance) ||
			distance<mindistance) && 
			(e->type==0 ||
			 e->type==1 ||
			 e->type==2 ||
			 e->type==3 ||
			 e->type==4 ||
			 e->type==7)) {
			selected=e;
			mindistance=distance;
		} /* if */ 
	} /* while */ 

	if (selected!=0 && S_enemyhit!=0) {
		Mix_PlayChannel(-1,S_enemyhit,0);
		retval=1;
	} /* if */ 

	if (selected!=0 && selected->collision(strength)) {
		int generate_smoke=-1;
		
		if (selected->type!=0) retval=2; /* If it's not a bullet, then you have destroyed an enemy */ 

		if (selected->type==1 ||
			selected->type==2 ||
			selected->type==3 ||
			selected->type==7) {
			int x,y,i;

			x=selected->x/16;
			y=selected->y/16;
			i=x+y*sx;

			if (map[i]==154 || map[i]==386) {
				map[i]=180;
				generate_smoke=0;
			} /* if */ 
			if (map[i]==155 || map[i]==387) {
				map[i]=181;
				generate_smoke=1;
			} /* if */ 
			if (map[i]==174 || map[i]==406) {
				map[i]=182;
				generate_smoke=2;
			} /* if */ 
			if (map[i]==175 || map[i]==407) {
				map[i]=183;
				generate_smoke=3;
			} /* if */ 
			if (map[i]>=176 && map[i]<180) {
				generate_smoke=map[i]-176;
				map[i]-=6;
			} /* if */ 
			if (map[i]>=196 && map[i]<200) {
				generate_smoke=map[i]-196;
				map[i]-=16;
			} /* if */ 
			if (map[i]>=216 && map[i]<220) {
				generate_smoke=map[i]-216;
				map[i]-=36;
			} /* if */ 
			if (map[i]>=236 && map[i]<240) {
				generate_smoke=map[i]-236;
				map[i]-=36;
			} /* if */ 
			if (generate_smoke==0) {
				SMOKESOURCE *ss;
				ss=new SMOKESOURCE();
				ss->x=selected->x+6;
				ss->y=selected->y+6;
				ss->speed_x=0;
				ss->speed_y=-FACTOR/4;
				ss->timer=0;
				smokesources.Add(ss);
			} /* if */ 
			if (generate_smoke==1) {
				SMOKESOURCE *ss;
				ss=new SMOKESOURCE();
				ss->x=selected->x+6;
				ss->y=selected->y+6;
				ss->speed_x=0;
				ss->speed_y=FACTOR/4;
				ss->timer=0;
				smokesources.Add(ss);
			} /* if */ 
			if (generate_smoke==2) {
				SMOKESOURCE *ss;
				ss=new SMOKESOURCE();
				ss->x=selected->x+6;
				ss->y=selected->y+6;
				ss->speed_x=FACTOR/4;
				ss->speed_y=0;
				ss->timer=0;
				smokesources.Add(ss);
			} /* if */ 
			if (generate_smoke==3) {
				SMOKESOURCE *ss;
				ss=new SMOKESOURCE();
				ss->x=selected->x+6;
				ss->y=selected->y+6;
				ss->speed_x=-FACTOR/4;
				ss->speed_y=0;
				ss->timer=0;
				smokesources.Add(ss);
			} /* if */ 
			if (S_explosion!=0) Mix_PlayChannel(-1,S_explosion,0);
		} /* if */ 

		if (selected->type!=4 && selected->type!=5 && selected->type!=6) {
//			enemies.DeleteElement(selected);
//			delete selected;
			if (selected->type==0) {
				selected->type=6;
				selected->state=0;
			} else {
				selected->state=-1;
			} /* if */ 
		} else {
			if (selected->type==4) {
				if (S_explosion!=0) Mix_PlayChannel(-1,S_explosion,0);
				selected->type=5;
				selected->state=0;
				generate_smoke=0;
				if (generate_smoke==0) {
					SMOKESOURCE *ss;
					ss=new SMOKESOURCE();
					ss->x=selected->x;
					ss->y=selected->y;
					ss->speed_x=0;
					ss->speed_y=-FACTOR/4;
					ss->timer=0;
					smokesources.Add(ss);
				} /* if */ 
			} /* if */ 
		} /* if */ 

	} /* if */ 

	return retval;
} /* shipbullet_collision */ 


void TRANSBALL_MAP::ball_collision(int x,int y)
{
	SWITCH *s;
	List<SWITCH> l;
	SWITCH *selected=0;
	int mindistance=-1;

	l.Instance(switches);
	l.Rewind();
	while(l.Iterate(s)) {
		int distance=(x-(s->x*16+8))*(x-(s->x*16+8)) + (y-(s->y*16+8))*(y-(s->y*16+8));
		if ((mindistance==-1 && distance<64) ||
			distance<mindistance) {
			selected=s;
			mindistance=distance;
		} /* if */ 
	} /* while */ 

	if (selected!=0) {
		DOOR *d;

		selected->state=16;
		if (S_switch!=0) Mix_PlayChannel(-1,S_switch,0);
		doors.Rewind();
		while(doors.Iterate(d)) {
			if (d->event==selected->number) d->activate();
		} /* while */ 

	} /* if */  
} /* ball_collision */ 


void TRANSBALL_MAP::ball_taken(void)
{
	DOOR *d;
	doors.Rewind();
	while(doors.Iterate(d)) {
		/* The doors with event==0 are activated when the ball is taken */ 
		if (d->event==0) d->activate();
	} /* while */ 
} /* ball_taken */ 


bool TRANSBALL_MAP::ship_in_fuel_recharge(int ship_x,int ship_y)
{
	FUELRECHARGE *f;
	
	fuel_recharges.Rewind();
	while(fuel_recharges.Iterate(f)) {
		if (ship_x>=f->x*16 && ship_x<(f->x*16+32) &&
			ship_y>=f->y*16 && ship_y<(f->y*16+32)) return true;
	} /* while */ 

	return false;
} /* ship_in_fuel_recharge */ 
