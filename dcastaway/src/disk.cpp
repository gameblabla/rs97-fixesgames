#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>

#include "dcastaway.h"

extern SDL_Surface *screen;

#define MAX_DISK_IMAGES 8

static SDL_Surface *flopimg[MAX_DISK_IMAGES];

int disk_index=0;

extern unsigned short render_pal16_copy0;


int initDisk(void)
{
	char filename[256];
	SDL_Surface *tmp;
	int i;
	
	for(i=0;i<MAX_DISK_IMAGES+1;i++)
	{
		sprintf(filename,DATA_PREFIX "disk%i.png",i);
		tmp=IMG_Load(filename);
		if (tmp==NULL)
			return -1;
		flopimg[i]=SDL_DisplayFormat(tmp);
		SDL_SetColorKey(flopimg[i],SDL_SRCCOLORKEY | SDL_RLEACCEL, 0);
		SDL_FreeSurface(tmp);
	}
	return 0;
}

int quitDisk(void)
{
	int i;
	for (i=0;i<MAX_DISK_IMAGES;i++)
		SDL_FreeSurface(flopimg[i]);
}

#ifndef NO_RENDER
void drawDisk(void)
{
	static int chan=0;
	SDL_Rect r;
	r.x=screen_width-20;
	r.y=screen_height-20;
	r.w=20;
	r.h=20;
	SDL_FillRect(screen,&r,render_pal16_copy0);
	SDL_BlitSurface(flopimg[disk_index],NULL,screen,&r);
#ifndef USE_DOUBLE_BUFFER
	SDL_UpdateRect(screen, r.x, r.y, r.w, r.h);
#endif
	if (!((chan++)&3))
		disk_index=(disk_index+1)%MAX_DISK_IMAGES;
}

void drawDiskEmpty(void)
{
	SDL_Rect r;
	r.x=screen_width-20;
	r.y=screen_height-20;
	r.w=20;
	r.h=20;
	SDL_FillRect(screen,&r,render_pal16_copy0);
#ifndef USE_DOUBLE_BUFFER
	SDL_UpdateRect(screen,r.x,r.y,r.w,r.h);
#endif
}
#endif
