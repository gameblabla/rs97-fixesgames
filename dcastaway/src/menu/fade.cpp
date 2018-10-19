#include"fade.h"


void fade16(SDL_Surface *screen, unsigned short n)
{
	int i,total=screen->pitch*screen->h/2;
	if (SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);
	unsigned short rs=screen->format->Rshift;
	unsigned short gs=screen->format->Gshift;
	unsigned short bs=screen->format->Bshift;
	unsigned short rm=screen->format->Rmask;
	unsigned short gm=screen->format->Gmask;
	unsigned short bm=screen->format->Bmask;
	unsigned short * buff=(unsigned short*)screen->pixels;
	for(i=0;i<total;i++)
	{
		register unsigned short r=(buff[i]&rm)>>rs;
		register unsigned short g=(buff[i]&gm)>>gs;
		register unsigned short b=(buff[i]&bm)>>bs;
		if (r>n)
			r-=n;
		else
			r=0;
		if (g>n)
			g-=n;
		else
			g=0;
		if (b>n)
			b-=n;
		else
			b=0;
		buff[i]=((r<<rs) | (g<<gs) | (b<<bs));
	}
	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
}
