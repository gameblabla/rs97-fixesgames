#include <stdio.h>
#include <stdlib.h>
#include "SDL/SDL.h"

#include "tiles.h"


TILE::TILE(SDL_Surface *src,int x,int y,int dx,int dy)
{
	orig=src;
	r.x=x;
	r.y=y;
	r.w=dx;
	r.h=dy;
} /* TILE */ 


TILE::~TILE()
{
} /* ~TILE */ 


void TILE::draw(int x,int y,SDL_Surface *surface)
{
	SDL_Rect d;

	d.x=x;
	d.y=y;
	SDL_BlitSurface(orig,&r,surface,&d);
} /* draw */ 


void TILE::draw_with_offset(int x,int y,SDL_Surface *surface,int offset)
{
	SDL_Rect o,d;

	if (offset>0 && offset<r.w) {
		o.x=r.x;
		o.y=r.y;
		o.w=r.w-offset;
		o.h=r.h;
		d.x=x+offset;
		d.y=y;
		SDL_BlitSurface(orig,&o,surface,&d);
	} /* if */ 
	if (offset==0) {
		draw(x,y,surface);
	} /* if */ 
	if (offset<0) {
		o.x=r.x-offset;
		o.y=r.y;
		o.w=r.w+offset;
		o.h=r.h;
		d.x=x;
		d.y=y;
		SDL_BlitSurface(orig,&o,surface,&d);
	} /* if */ 

} /* draw_with_offset */ 
