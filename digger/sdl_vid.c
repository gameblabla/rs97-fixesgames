/*
 * ---------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42, (c) Poul-Henning Kamp): Maxim
 * Sobolev <sobomax@altavista.net> wrote this file. As long as you retain
 * this  notice you can  do whatever you  want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * 
 * Maxim Sobolev
 * --------------------------------------------------------------------------- 
 */

/* malloc() and friends */
#include <stdlib.h>
/* Lovely SDL */
#include <SDL.h>

#include "def.h"
#include "hardware.h"
#include "title_gz.h"
#include "icon.h"

extern Uint3 *vgatable[];
extern Uint3 *ascii2vga[];

Uint3 **sprites = vgatable;
Uint3 **alphas = ascii2vga;
SDL_Surface *ScreenSurface;
Sint4 xratio = 2;
Sint4 yratio = 2;
Sint4 yoffset = 0;
Sint4 hratio = 2;
Sint4 wratio = 2;
#define virt2scrx(x) (x*xratio)
#define virt2scry(y) (y*yratio+yoffset)
#define virt2scrw(w) (w*wratio)
#define virt2scrh(h) (h*hratio)

/* palette1, normal intensity */
SDL_Color vga16_pal1[] = \
{{0,0,0,0},{0,0,128,0},{0,128,0,0},{0,128,128,0},{128,0,0,0},{128,0,128,0} \
,{128,64,0,0},{128,128,128,0},{64,64,64,0},{0,0,255,0},{0,255,0,0} \
,{0,255,255,0},{255,0,0,0},{255,0,255,0},{255,255,0,0},{255,255,255,0}};
/* palette1, high intensity */
SDL_Color vga16_pal1i[] = \
{{0,0,0,0},{0,0,255,0},{0,255,0,0},{0,255,255,0},{255,0,0,0},{255,0,255,0} \
,{255,128,0,0},{196,196,196,0},{128,128,128,0},{128,128,255,0},{128,255,128,0} \
,{128,255,255,0},{255,128,128,0},{255,128,255,0},{255,255,128,0},{255,255,255,0}};
/* palette2, normal intensity */
SDL_Color vga16_pal2[] = \
{{0,0,0,0},{0,128,0,0},{128,0,0,0},{128,64,0,0},{0,0,128,0},{0,128,128,0} \
,{128,0,128,0},{128,128,128,0},{64,64,64,0},{0,255,0,0},{255,0,0,0} \
,{255,255,0,0},{0,0,255,0},{0,255,255,0},{255,0,255,0},{255,255,255,0}};
/* palette2, high intensity */
SDL_Color vga16_pal2i[] = \
{{0,0,0,0},{0,255,0,0},{255,0,0,0},{255,128,0,0},{0,0,255,0},{0,255,255,0} \
,{255,0,255,0},{196,196,196,0},{128,128,128,0},{128,255,128,0},{255,128,128,0} \
,{255,255,128,0},{128,128,255,0},{128,255,255,0},{255,128,255,0},{255,255,255,0}};

SDL_Color *npalettes[] = {vga16_pal1, vga16_pal2};
SDL_Color *ipalettes[] = {vga16_pal1i, vga16_pal2i};
Sint4	currpal=0;

Uint32	addflag=0;

SDL_Surface *scr = NULL;
SDL_Surface *screen = NULL;
SDL_Surface *icon = NULL;

/* Data structure holding pending updates */
struct PendNode {
	void *prevnode;
	void *nextnode;
	SDL_Rect rect;
};

struct PendNode *First=NULL, *Last=NULL;

int pendnum = 0;

SDL_Surface *ch2bmap(Uint3 *sprite, Sint4 w, Sint4 h)
{
	Sint4 realw, realh;
	SDL_Surface *tmp;

	realw = virt2scrw(w*4);
	realh = virt2scrh(h);
	tmp = SDL_CreateRGBSurfaceFrom(sprite, realw, realh, 8, realw, 0, 0, 0, 0);
	SDL_SetColors(tmp, screen->format->palette->colors, 0, screen->format->palette->ncolors);
	
	return(tmp);
}

void graphicsoff(void)
{
//	if(icon)
//		SDL_FreeSurface(icon);
	if(screen)
		SDL_FreeSurface(screen);
	if(scr)
		SDL_FreeSurface(scr);
	SDL_Quit();
}

bool setmode(void)
{
	ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_SWSURFACE);
	//if((scr = SDL_SetVideoMode(320, 240, 16, SDL_SWSURFACE)) == NULL)
	if((scr = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0, 0, 0, 0)) == NULL)
		return(FALSE);
	if((screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 400, 8, 0, 0, 0, 0)) == NULL)
		return(FALSE);
	return(TRUE);
}

void switchmode(void)
{
	Uint32 saved;
	SDL_Surface *tmp = NULL;
	SDL_Surface *oldscreen;
	
	vgageti(0, 0, (Uint3 *)&tmp, 80, 200);
	oldscreen = screen;
	saved = addflag;

	if(addflag == 0)
		addflag = SDL_FULLSCREEN;
	else
		addflag = 0;
	if(setmode() == FALSE) {
		addflag = saved;
		if(setmode() == FALSE) {
			fprintf(stderr, "Fatal: failed to change videomode and"\
				"fallback mode failed as well. Exitting.\n");
			exit(1);
		}
	}

	SDL_SetColors(screen, tmp->format->palette->colors, 0, tmp->format->palette->ncolors);
	vgaputi(0, 0, (Uint3 *)&tmp, 80, 200);
	SDL_FreeSurface(tmp);
	SDL_FreeSurface(oldscreen);
}


void vgainit(void)
{
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	if(setmode() == FALSE) {
		fprintf(stderr, "Couldn't set 640x480x8 video mode: %s\n", SDL_GetError());
		exit(1);
        }
#ifndef _DINGOO
	icon = SDL_CreateRGBSurfaceFrom(Icon, 64, 64, 8, 64, 0, 0, 0, 0);
	SDL_SetColorKey(icon, SDL_SRCCOLORKEY, 247);
	icon->format->palette->colors = IconPalette;
	SDL_WM_SetCaption("DIGGER", NULL);
	SDL_WM_SetIcon(icon, NULL);
#endif//_DINGOO
	SDL_LockSurface(scr);
	memset(scr->pixels, 0, scr->pitch*240);
	SDL_UnlockSurface(scr);
	SDL_ShowCursor(0);
}

void vgaclear(void)
{
	SDL_Surface *tmp = NULL;
	
	vgageti(0, 0, (Uint3 *)&tmp, 80, 200);
	memset(tmp->pixels, 0x00, tmp->w*tmp->h);
	vgaputi(0, 0, (Uint3 *)&tmp, 80, 200);
	SDL_FreeSurface(tmp);
}
void setpal(SDL_Color *pal)
{
	SDL_SetColors(screen, pal, 0, 16);
}
	
void vgainten(Sint4 inten)
{
	if(inten == 1)
		setpal(ipalettes[currpal]);
	else
		setpal(npalettes[currpal]);
}

void vgapal(Sint4 pal)
{
	setpal(npalettes[pal]);
	currpal = pal;
}

inline Uint4 BGR565(Uint3 r, Uint3 g, Uint3 b) { return (((r&~7) << 8)|((g&~3) << 3)|(b >> 3)); }

void doscreenupdate(void)
{
	struct PendNode *p;

	for(p=First;p!=NULL;)
	{
//		SDL_UpdateRect(screen,p->rect.x,p->rect.y,p->rect.w,p->rect.h);
		First = p->nextnode;
		free(p);
		p = First;
	}
	pendnum = 0;
	SDL_LockSurface(screen);
	SDL_LockSurface(scr);
	int x, y;
	Uint3 c;
	int r, g, b;
	Uint3* p1;
	Uint4* p2;
	for(y = 0; y < 200; ++y)
	{
		for(x = 0; x < 320; ++x)
		{
			p1 = (Uint3*)screen->pixels + y*2*screen->pitch + x*2;
			c = *(p1 + 0);
			r = vga16_pal1[c].r;
			g = vga16_pal1[c].g;
			b = vga16_pal1[c].b;
			c = *(p1 + 1);
			r += vga16_pal1[c].r;
			g += vga16_pal1[c].g;
			b += vga16_pal1[c].b;
			c = *(p1 + screen->pitch + 0);
			r += vga16_pal1[c].r;
			g += vga16_pal1[c].g;
			b += vga16_pal1[c].b;
			c = *(p1 + screen->pitch + 1);
			r += vga16_pal1[c].r;
			g += vga16_pal1[c].g;
			b += vga16_pal1[c].b;
			p2 = (Uint4*)((Uint3*)scr->pixels + (y + 20)*scr->pitch + x*2);
			*p2 = BGR565(r/4, g/4, b/4);
		}
	}
	SDL_UnlockSurface(scr);
	SDL_UnlockSurface(screen);
	//SDL_Flip(scr);
    {
      if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
      int x, y;
      uint32_t *s = (uint32_t*)scr->pixels;
      uint32_t *d = (uint32_t*)ScreenSurface->pixels;
		for(uint8_t y2 = 0; y2 < 240; y2++, s += 160, d += 320) 
			memmove(d, s, 1280); // double-line fix by pingflood, 2018
      if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
      SDL_Flip(ScreenSurface);
    }
}

void vgaputi(Sint4 x, Sint4 y, Uint3 *p, Sint4 w, Sint4 h)
{
	SDL_Surface *tmp;
	SDL_Palette *reserv;
	struct PendNode *new, *ptr;

	new = malloc(sizeof (struct PendNode));
	memset(new, 0x00, (sizeof (struct PendNode)));
	new->rect.x = virt2scrx(x);
	new->rect.y = virt2scry(y);
	new->rect.w = virt2scrw(w*4);
	new->rect.h = virt2scrh(h);

	memcpy(&tmp, p, (sizeof (SDL_Surface *)));
	reserv = tmp->format->palette;
	tmp->format->palette = screen->format->palette;
	SDL_BlitSurface(tmp, NULL, screen, &new->rect);
//	SDL_StretchSurfaceBlit(tmp, NULL, screen, &new->rect);
	tmp->format->palette = reserv;
/* 
 * Following piece of code comparing already pending updates with current with
 * main goal to prevent redrawing overlapping rectangles several times.
 */ 
	
	for(ptr=First;ptr!=NULL;ptr=ptr->nextnode) {
		if((new->rect.x >= ptr->rect.x) &&
		   (new->rect.y >= ptr->rect.y) &&
		   ((new->rect.x+new->rect.w) <= (ptr->rect.x+ptr->rect.w)) &&
		   ((new->rect.y+new->rect.h) <= (ptr->rect.y+ptr->rect.h))) {
			free(new);
			return;
		} else if((new->rect.x <= ptr->rect.x) &&
		   (new->rect.y <= ptr->rect.y) &&
		   ((new->rect.x+new->rect.w) >= (ptr->rect.x+ptr->rect.w)) &&
		   ((new->rect.y+new->rect.h) >= (ptr->rect.y+ptr->rect.h))) {
			ptr->rect.x = new->rect.x;
			ptr->rect.y = new->rect.y;
			ptr->rect.w = new->rect.w;
			ptr->rect.h = new->rect.h;
			free(new);
			return;
		}
	}
			
	if (pendnum == 0)
		First = new;
	else {
		Last->nextnode = new;
		new->prevnode = Last;
	}
	
	Last = new;
	pendnum++;
}

void vgageti(Sint4 x, Sint4 y, Uint3 *p, Sint4 w, Sint4 h)
{
	SDL_Surface *tmp;
	SDL_Rect src;
	
	memcpy(&tmp, p, (sizeof (SDL_Surface *)));
	if (tmp != NULL)
		SDL_FreeSurface(tmp); /* Destroy previously allocated bitmap */

	src.x = virt2scrx(x);
	src.y = virt2scry(y);
	src.w = virt2scrw(w*4);
	src.h = virt2scrh(h);

	tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, src.w, src.h, 8, 0, 0, 0, 0);
	SDL_SetColors(tmp, screen->format->palette->colors, 0, screen->format->palette->ncolors);
	SDL_BlitSurface(screen, &src, tmp, NULL);
	memcpy(p, &tmp, (sizeof (SDL_Surface *)));
}

Sint4 vgagetpix(Sint4 x, Sint4 y)
{	
	SDL_Surface *tmp = NULL;
	Uint4 xi,yi;
	Uint4 i = 0;
	Sint4 rval = 0;
	Uint8 *pixels;

	if ((x > 319) || (y > 199))
	       return (0xff);

	vgageti(x, y, (Uint3 *)&tmp, 1, 1);
	pixels = (Uint8 *)tmp->pixels;
	for (yi=0;yi<tmp->h;yi++)
		for (xi=0;xi<tmp->w;xi++)
			if (pixels[i++])
				rval |= 0x80 >> xi;

	SDL_FreeSurface(tmp);

	return(rval & 0xee);
}

void vgaputim(Sint4 x, Sint4 y, Sint4 ch, Sint4 w, Sint4 h)
{
	SDL_Surface *tmp;
	SDL_Surface *mask;
	SDL_Surface *scr = NULL;
	Uint8   *tmp_pxl, *mask_pxl, *scr_pxl;
	Sint4 realsize;
	Sint4 i;

	tmp = ch2bmap(sprites[ch*2], w, h);
	mask = ch2bmap(sprites[ch*2+1], w, h);
	vgageti(x, y, (Uint3 *)&scr, w, h);
	realsize = scr->w * scr->h;
	tmp_pxl = (Uint8 *)tmp->pixels;
	mask_pxl = (Uint8 *)mask->pixels;
	scr_pxl = (Uint8 *)scr->pixels;
	for(i=0;i<realsize;i++)
		if(tmp_pxl[i] != 0xff)
			scr_pxl[i] = (scr_pxl[i] & mask_pxl[i]) | \
				tmp_pxl[i];

	vgaputi(x, y, (Uint3 *)&scr, w, h);
	tmp->pixels = NULL;   /* We should NULL'ify these ppointers, or the VGLBitmapDestroy */
	mask->pixels = NULL;  /* will shoot itself in the foot by trying to dellocate statically */
	SDL_FreeSurface(tmp);/* allocated arrays */
	SDL_FreeSurface(mask);
	SDL_FreeSurface(scr);
}

void vgawrite(Sint4 x, Sint4 y, Sint4 ch, Sint4 c)
{
	SDL_Surface *tmp;
	Uint8 *copy;
	Uint8 color;
	Sint4 w=3, h=12, size;
	Sint4 i;

	if(((ch - 32) >= 0x5f) || (ch < 32))
		return;
	tmp = ch2bmap(alphas[ch-32], w, h);
	size = tmp->w*tmp->h;
	copy = malloc(size);
	memcpy(copy, tmp->pixels, size);

	for(i = size;i!=0;) {
		i--;
		color = copy[i];
		if (color==10) {
			if (c==2)
				color=12;
            		else {
              			if (c==3)
				color=14;
			}
          	} else
			if (color==12) {
				if (c==1)
					color=2;
				else
					if (c==2)
						color=4;
					else
						if (c==3)
							color=6;
			}
		copy[i] = color;
	}
	tmp->pixels = copy;
	vgaputi(x, y, (Uint3 *)&tmp, w, h);
	SDL_FreeSurface(tmp);
}

void vgatitle(void)
{
	SDL_Surface *tmp=NULL;

	vgageti(0, 0, (Uint3 *)&tmp, 80, 200);
	gettitle(tmp->pixels);
	vgaputi(0, 0, (Uint3 *)&tmp, 80, 200);
	SDL_FreeSurface(tmp);
}

void gretrace(void)
{
}

void savescreen(void)
{
/*	FILE *f;
	int i;
	
	f=fopen("screen.saw", "w");
	
	for(i=0;i<(VGLDisplay->Xsize*VGLDisplay->Ysize);i++)
		fputc(VGLDisplay->Bitmap[i], f);
	fclose(f);*/
}

/* 
 * Depreciated functions, necessary only to avoid "Undefined symbol:..." compiler
 * errors.
 */
 
void cgainit(void) {}
void cgaclear(void) {}
void cgatitle(void) {}
void cgawrite(Sint4 x, Sint4 y, Sint4 ch, Sint4 c) {}
void cgaputim(Sint4 x, Sint4 y, Sint4 ch, Sint4 w, Sint4 h) {}
void cgageti(Sint4 x, Sint4 y, Uint3 *p, Sint4 w, Sint4 h) {}
void cgaputi(Sint4 x, Sint4 y, Uint3 *p, Sint4 w, Sint4 h) {}
void cgapal(Sint4 pal) {}
void cgainten(Sint4 inten) {}
Sint4 cgagetpix(Sint4 x, Sint4 y) {return(0);}
