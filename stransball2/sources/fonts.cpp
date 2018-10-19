#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL/SDL.h"
#include "SDL_image.h"

#include "auxiliar.h"

#define FONT_SIZE_X	6
#define FONT_SIZE_Y 8

/* Surfaces: */ 
SDL_Surface *font_sfc;


bool fonts_initialization(void)
{
	font_sfc=IMG_Load("graphics/font3.png");
	if (font_sfc==0) return false;

	/* Transparant color is BLACK: */ 
	SDL_SetColorKey(font_sfc,SDL_SRCCOLORKEY,SDL_MapRGB(font_sfc->format,0,0,0));

	return true;
}/* fonts_initialization */ 


void fonts_termination(void)
{
    SDL_FreeSurface(font_sfc);
} /* fonts_termination */ 


void font_print(int x,int y,char *text,SDL_Surface *surface)
{
	int i;
	SDL_Rect s,d;

	for(i=0;text[i]!=0 && x<surface->w;i++) {
		int row;

		row=int(text[i])/45;
		s.x=(int(text[i])-(row*45))*(FONT_SIZE_X+1)+1;
		s.y=(row)*(FONT_SIZE_Y+1)+1;
		s.h=FONT_SIZE_Y;
		s.w=FONT_SIZE_X;
		
		d.x=x+i*FONT_SIZE_X;
		d.y=y;
		SDL_BlitSurface(font_sfc,&s,surface,&d);
	} /* for */ 
} /* font_print */ 


void font_print_right(int x,int y,char *text,SDL_Surface *surface)
{
	int i;
	SDL_Rect s,d;

	x-=strlen(text)*FONT_SIZE_X;

	for(i=0;text[i]!=0 && x<surface->w;i++) {
		int row;

		row=int(text[i])/45;
		s.x=(int(text[i])-(row*45))*(FONT_SIZE_X+1)+1;
		s.y=(row)*(FONT_SIZE_Y+1)+1;
		s.h=FONT_SIZE_Y;
		s.w=FONT_SIZE_X;
		
		d.x=x+i*FONT_SIZE_X;
		d.y=y;
		SDL_BlitSurface(font_sfc,&s,surface,&d);
	} /* for */ 
} /* font_print_right */ 


void font_print_centered(int x,int y,char *text,SDL_Surface *surface)
{
	int i;
	SDL_Rect s,d;

	x-=strlen(text)*FONT_SIZE_X/2;

	for(i=0;text[i]!=0 && x<surface->w;i++) {
		int row;

		row=int(text[i])/45;
		s.x=(int(text[i])-(row*45))*(FONT_SIZE_X+1)+1;
		s.y=(row)*(FONT_SIZE_Y+1)+1;
		s.h=FONT_SIZE_Y;
		s.w=FONT_SIZE_X;
		
		d.x=x+i*FONT_SIZE_X;
		d.y=y;
		SDL_BlitSurface(font_sfc,&s,surface,&d);
	} /* for */ 
} /* font_print_centered */ 

