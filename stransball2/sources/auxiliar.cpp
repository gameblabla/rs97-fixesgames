
#ifdef _WIN32
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
#include "SDL_image.h"

#include "auxiliar.h"


#ifndef _WIN32
char *strupr(char *ptr)
{
    if (ptr!=0) {
	char *p = ptr;
        while(*p!=0) {
            *p=toupper(*p);
            p++;
        } /* while */
    } /* if */
    
    return ptr;
} 

#endif


SDL_Surface *load_maskedimage(char *imagefile,char *maskfile,char *path)
{
	char name[256];

	SDL_Surface *res;
    SDL_Surface *tmp;
	SDL_Surface *mask;

	sprintf(name,"%s%s",path,imagefile);
	tmp=IMG_Load(name);
	sprintf(name,"%s%s",path,imagefile);
	mask=IMG_Load(name);

    if (tmp==0 ||
		mask==0) return 0;

	res=SDL_DisplayFormatAlpha(tmp);

	/* Aplicar la máscara: */ 
	{
		int x,y;
		Uint8 r,g,b,a;
		Uint32 v;

		for(y=0;y<mask->h;y++) {
			for(x=0;x<mask->w;x++) {
				v=getpixel(res,x,y);
				SDL_GetRGBA(v,res->format,&r,&g,&b,&a);
				v=getpixel(mask,x,y);
				if (v!=0) a=255;
					 else a=0;
				v=SDL_MapRGBA(res->format,r,g,b,a);
				putpixel(res,x,y,v);
			} /* for */ 
		} /* for */ 
	}

	SDL_FreeSurface(tmp);
	SDL_FreeSurface(mask);

	return res;
} /* load_maskedimage */ 


void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
	SDL_Rect clip;
    int bpp = surface->format->BytesPerPixel;

	SDL_GetClipRect(surface,&clip);

	if (x<clip.x || x>=clip.x+clip.w ||
		y<clip.y || y>=clip.y+clip.h) return;

	if (x<0 || x>=surface->w ||
		y<0 || y>=surface->h) return;

    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}


Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;

	if (x<0 || x>=surface->w ||
		y<0 || y>=surface->h) return 0;

    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;
    }
}


void surface_fader(SDL_Surface *surface,float r_factor,float g_factor,float b_factor,float a_factor,SDL_Rect *r)
{
	SDL_Rect r2;
	int i,x,y,offs;
	Uint8 rtable[256],gtable[256],btable[256],atable[256];
	Uint8 *pixels;
	SDL_Surface *tmp;

	if (r==0) {
		r2.x=0;
		r2.y=0;
		r2.w=surface->w;
		r2.h=surface->h;
		r=&r2;
	} /* if */ 

	if (surface->format->BytesPerPixel!=4 ||
		(r_factor==1.0 &&
		 g_factor==1.0 &&
		 b_factor==1.0 &&
		 a_factor==1.0)) return;

	for(i=0;i<256;i++) {
		rtable[i]=(Uint8)(i*r_factor);
		gtable[i]=(Uint8)(i*g_factor);
		btable[i]=(Uint8)(i*b_factor);
		atable[i]=(Uint8)(i*a_factor);
	} /* for */ 

	if ((surface->flags&SDL_HWSURFACE)!=0) {
		/* HARDWARE SURFACE!!!: */ 
		tmp=SDL_CreateRGBSurface(SDL_SWSURFACE,surface->w,surface->h,32,0,0,0,0);
		SDL_BlitSurface(surface,0,tmp,0);
		SDL_LockSurface(tmp);
		pixels = (Uint8 *)(tmp->pixels);
	} else {
		SDL_LockSurface(surface);
		pixels = (Uint8 *)(surface->pixels);
	} /* if */ 

	for(y=r->y;y<r->y+r->h && y<surface->h;y++) {
		for(x=r->x,offs=y*surface->pitch+r->x*4;x<r->x+r->w && x<surface->w;x++,offs+=4) {
			pixels[offs+ROFFSET]=rtable[pixels[offs+ROFFSET]];
			pixels[offs+GOFFSET]=gtable[pixels[offs+GOFFSET]];
			pixels[offs+BOFFSET]=btable[pixels[offs+BOFFSET]];
			pixels[offs+AOFFSET]=atable[pixels[offs+AOFFSET]];
		} /* for */ 
	} /* for */ 

	if ((surface->flags&SDL_HWSURFACE)!=0) {
		/* HARDWARE SURFACE!!!: */ 
		SDL_UnlockSurface(tmp);
		SDL_BlitSurface(tmp,0,surface,0);
		SDL_FreeSurface(tmp);
	} else {
		SDL_UnlockSurface(surface);
	} /* if */ 


} /* surface_fader */ 

void rectangle(SDL_Surface *surface, int x, int y, int w, int h, Uint32 pixel)
{
	int i;

	for(i=0;i<w;i++) {
		putpixel(surface,x+i,y,pixel);
		putpixel(surface,x+i,y+h,pixel);
	} /* for */ 
	for(i=0;i<=h;i++) {
		putpixel(surface,x,y+i,pixel);
		putpixel(surface,x+w,y+i,pixel);
	} /* for */ 
} /* rectangle */ 

