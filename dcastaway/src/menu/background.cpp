#include<stdio.h>
#include<stdlib.h>

#include<SDL.h>
#include<SDL_image.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define DIV_X 3
#define DIV_Y 3

#define SX (SCREEN_WIDTH/DIV_X)
#define SY (SCREEN_HEIGHT/DIV_Y)

#define TX (DIV_X+1)
#define TY (DIV_Y+2)

#define SLICE 150
#define SLICE_SCROLL (20)

#define MAX 64

static SDL_Surface *sonic_surface[MAX];
static SDL_Surface *knuck_surface[MAX];

static int sonic_pngs=0;
static int knuck_pngs=0;

static int x[TX][TY];
static int y[TX][TY];
static int pos[TX][TY];

static Uint32 back_counter=0x72345678;

void init_background(void)
{
	SDL_Surface *tmp;
	char name[256];
	int i;
	for(i=0;i<MAX;i++)
	{
		sprintf(name,DATA_PREFIX "background0_%i.png",i);
		tmp=IMG_Load(name);
		if (!tmp)
			break;
		sonic_surface[i]=SDL_DisplayFormat(tmp);
		SDL_FreeSurface(tmp);
	}
	if (i<1)
		exit(-1);
	sonic_pngs=i;
	for(;i<MAX;i++)
		sonic_surface[i]=sonic_surface[i%sonic_pngs];

	for(i=0;i<MAX;i++)
	{
		sprintf(name,DATA_PREFIX "background1_%i.png",i);
		tmp=IMG_Load(name);
		if (!tmp)
			break;
		knuck_surface[i]=SDL_DisplayFormat(tmp);
		SDL_FreeSurface(tmp);
	}
	if (i<1)
		exit(-1);
	knuck_pngs=i;
	for(;i<MAX;i++)
		knuck_surface[i]=knuck_surface[i%knuck_pngs];
}

static void init_counters(Uint32 now)
{
	int i,j;
	for(j=0;j<TY;j++)
		for(i=0;i<TX;i++)
		{
			x[i][j]=(i*SX)+(((j&1)*SX)/2);
			y[i][j]=j*SY;
			pos[i][j]=(i+j)%MAX;
		}
}

void draw_background(SDL_Surface *screen)
{
	Uint32 now=SDL_GetTicks();
	Uint32 diff=now-back_counter;
	int i,j;

	if (diff>1000)
		init_counters(now);

	SDL_FillRect(screen,NULL,-1);
	for(j=0;j<TY;j++)
		for(i=0;i<TX;i++)
		{
			SDL_Rect r;
			SDL_Surface *s;
			if (!(j&1))
				s=sonic_surface[pos[i][j]];
			else
				s=knuck_surface[pos[i][j]];
			r.x=x[i][j];
			r.y=y[i][j];
			r.w=s->w;
			r.h=s->h;
			SDL_BlitSurface(s,NULL,screen,&r);
			pos[i][j]=(now/SLICE)%MAX;
		}

	if (x[0][0]<(-SX))
	{
		for(j=0;j<TY;j+=2)
			for(i=0;i<TX;i++)
				x[i][j]=i*SX;
	}
	else
	{
		for(j=0;j<TY;j+=2)
			for(i=0;i<TX;i++)
				x[i][j]-=(diff/SLICE_SCROLL);
	}
	
	if (x[0][1]>=(SX/2))
	{
		for(j=1;j<TY;j+=2)
			for(i=0;i<TX;i++)
				x[i][j]=(SX/2)+(i-1)*SX;
	}
	else
	{
		for(j=1;j<TY;j+=2)
			for(i=0;i<TX;i++)
				x[i][j]+=(diff/SLICE_SCROLL);
	}

	if (y[0][0]>=0)
	{
		for(j=0;j<TY;j++)
			for(i=0;i<TX;i++)
				y[i][j]=(j-2)*SY;
	}
	else
	{
		for(j=0;j<TY;j++)
			for(i=0;i<TX;i++)
				y[i][j]+=(diff/SLICE_SCROLL);
	}

	if (diff>SLICE_SCROLL)
		back_counter=now;
}


void quit_background(void)
{
	int i;
	for (i=0;i<sonic_pngs;i++)
		SDL_FreeSurface(sonic_surface[i]);
	for (i=0;i<knuck_pngs;i++)
		SDL_FreeSurface(knuck_surface[i]);
}
