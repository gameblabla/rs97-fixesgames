/* sdltile.c: Functions for rendering tile images.
 *
 * Copyright (C) 2001-2006 by Brian Raiter, under the GNU General Public
 * License. No warranty. See COPYING for details.
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<assert.h>
#include	<SDL.h>
#include	"sdlgen.h"
#include	"../err.h"
#include	"../state.h"

//DKS
#include <stdint.h>

/* Direction offsets.
 */
#define	_NORTH	+ 0
#define	_WEST	+ 1
#define	_SOUTH	+ 2
#define	_EAST	+ 3

/* The total number of tile images.
 */
#define	NTILES		128

/* Flags that indicate the size and shape of an oversized
 * (transparent) tile image.
 */
#define	SIZE_EXTLEFT	0x01	/* image extended leftwards by one tile */
#define	SIZE_EXTRIGHT	0x02	/* image extended rightwards by one tile */
#define	SIZE_EXTUP	0x04	/* image extended upwards by one tile */
#define	SIZE_EXTDOWN	0x08	/* image extended downards by one tile */
#define	SIZE_EXTALL	0x0F	/* image is 3x3 tiles in size */

/* Structure providing pointers to the various tile images available
 * for a given id.
 */
typedef	struct tilemap {
	SDL_Surface	       *opaque[16];	/* one or more opaque images */
	SDL_Surface	       *transp[16];	/* one or more transparent images */
	char		celcount;	/* count of animated images */
	char		transpsize;	/* flags for the transparent size */
} tilemap;

/* Different types of tile images as stored in the large format bitmap.
 */
enum {
	TILEIMG_IMPLICIT = 0,	/* tile is not in the bitmap */
	TILEIMG_SINGLEOPAQUE,	/* a single opaque image */
	TILEIMG_OPAQUECELS,		/* one or more opaque images */
	TILEIMG_TRANSPCELS,		/* one or more transparent images */
	TILEIMG_CREATURE,		/* one of the creature formats */
	TILEIMG_ANIMATION		/* twelve transparent images */
};

/* Structure indicating where to find the various tile images in a
 * tile bitmap. The opaque and transp fields give the coordinates of
 * the tile in the small-format and masked-format bitmaps. For a
 * large-format bitmap, the ordering of the array determines the order
 * in which tile images are stored. The shape field defines what sort
 * of tile image is expected in the large-format bitmap.
 */
typedef	struct tileidinfo {
	int		id;		/* the tile ID */
	signed char	xopaque;	/* the coordinates of the opaque image */
	signed char	yopaque;	/*   (expressed in tiles, not pixels) */
	signed char	xtransp;	/* coordinates of the transparent image */
	signed char	ytransp;	/*   (also expressed in tiles) */
	int		shape;		/* enum values for the free-form bitmap */
} tileidinfo;

/* The list of tile images.
 */
static tileidinfo const tileidmap[NTILES] = {
	{ Empty,			 0,  0, -1, -1, TILEIMG_SINGLEOPAQUE },
	{ Slide_North,		 1,  2, -1, -1, TILEIMG_OPAQUECELS },
	{ Slide_West,		 1,  4, -1, -1, TILEIMG_OPAQUECELS },
	{ Slide_South,		 0, 13, -1, -1, TILEIMG_OPAQUECELS },
	{ Slide_East,		 1,  3, -1, -1, TILEIMG_OPAQUECELS },
	{ Slide_Random,		 3,  2, -1, -1, TILEIMG_OPAQUECELS },
	{ Ice,			 0, 12, -1, -1, TILEIMG_OPAQUECELS },
	{ IceWall_Northwest,	 1, 12, -1, -1, TILEIMG_OPAQUECELS },
	{ IceWall_Northeast,	 1, 13, -1, -1, TILEIMG_OPAQUECELS },
	{ IceWall_Southwest,	 1, 11, -1, -1, TILEIMG_OPAQUECELS },
	{ IceWall_Southeast,	 1, 10, -1, -1, TILEIMG_OPAQUECELS },
	{ Gravel,			 2, 13, -1, -1, TILEIMG_OPAQUECELS },
	{ Dirt,			 0, 11, -1, -1, TILEIMG_OPAQUECELS },
	{ Water,			 0,  3, -1, -1, TILEIMG_OPAQUECELS },
	{ Fire,			 0,  4, -1, -1, TILEIMG_OPAQUECELS },
	{ Bomb,			 2, 10, -1, -1, TILEIMG_OPAQUECELS },
	{ Beartrap,			 2, 11, -1, -1, TILEIMG_OPAQUECELS },
	{ Burglar,			 2,  1, -1, -1, TILEIMG_OPAQUECELS },
	{ HintButton,		 2, 15, -1, -1, TILEIMG_OPAQUECELS },
	{ Button_Blue,		 2,  8, -1, -1, TILEIMG_OPAQUECELS },
	{ Button_Green,		 2,  3, -1, -1, TILEIMG_OPAQUECELS },
	{ Button_Red,		 2,  4, -1, -1, TILEIMG_OPAQUECELS },
	{ Button_Brown,		 2,  7, -1, -1, TILEIMG_OPAQUECELS },
	{ Teleport,			 2,  9, -1, -1, TILEIMG_OPAQUECELS },
	{ Wall,			 0,  1, -1, -1, TILEIMG_OPAQUECELS },
	{ Wall_North,		 0,  6, -1, -1, TILEIMG_OPAQUECELS },
	{ Wall_West,		 0,  7, -1, -1, TILEIMG_OPAQUECELS },
	{ Wall_South,		 0,  8, -1, -1, TILEIMG_OPAQUECELS },
	{ Wall_East,		 0,  9, -1, -1, TILEIMG_OPAQUECELS },
	{ Wall_Southeast,		 3,  0, -1, -1, TILEIMG_OPAQUECELS },
	{ HiddenWall_Perm,		 0,  5, -1, -1, TILEIMG_IMPLICIT },
	{ HiddenWall_Temp,		 2, 12, -1, -1, TILEIMG_IMPLICIT },
	{ BlueWall_Real,		 1, 14, -1, -1, TILEIMG_OPAQUECELS },
	{ BlueWall_Fake,		 1, 15, -1, -1, TILEIMG_IMPLICIT },
	{ SwitchWall_Open,		 2,  6, -1, -1, TILEIMG_OPAQUECELS },
	{ SwitchWall_Closed,	 2,  5, -1, -1, TILEIMG_OPAQUECELS },
	{ PopupWall,		 2, 14, -1, -1, TILEIMG_OPAQUECELS },
	{ CloneMachine,		 3,  1, -1, -1, TILEIMG_OPAQUECELS },
	{ Door_Red,			 1,  7, -1, -1, TILEIMG_OPAQUECELS },
	{ Door_Blue,		 1,  6, -1, -1, TILEIMG_OPAQUECELS },
	{ Door_Yellow,		 1,  9, -1, -1, TILEIMG_OPAQUECELS },
	{ Door_Green,		 1,  8, -1, -1, TILEIMG_OPAQUECELS },
	{ Socket,			 2,  2, -1, -1, TILEIMG_OPAQUECELS },
	{ Exit,			 1,  5, -1, -1, TILEIMG_OPAQUECELS },
	{ ICChip,			 0,  2, -1, -1, TILEIMG_OPAQUECELS },
	{ Key_Red,			 6,  5,  9,  5, TILEIMG_TRANSPCELS },
	{ Key_Blue,			 6,  4,  9,  4, TILEIMG_TRANSPCELS },
	{ Key_Yellow,		 6,  7,  9,  7, TILEIMG_TRANSPCELS },
	{ Key_Green,		 6,  6,  9,  6, TILEIMG_TRANSPCELS },
	{ Boots_Ice,		 6, 10,  9, 10, TILEIMG_TRANSPCELS },
	{ Boots_Slide,		 6, 11,  9, 11, TILEIMG_TRANSPCELS },
	{ Boots_Fire,		 6,  9,  9,  9, TILEIMG_TRANSPCELS },
	{ Boots_Water,		 6,  8,  9,  8, TILEIMG_TRANSPCELS },
	{ Block_Static,		 0, 10, -1, -1, TILEIMG_IMPLICIT },
	{ Overlay_Buffer,		 2,  0, -1, -1, TILEIMG_IMPLICIT },
	{ Exit_Extra_1,		 3, 10, -1, -1, TILEIMG_SINGLEOPAQUE },
	{ Exit_Extra_2,		 3, 11, -1, -1, TILEIMG_SINGLEOPAQUE },
	{ Burned_Chip,		 3,  4, -1, -1, TILEIMG_SINGLEOPAQUE },
	{ Bombed_Chip,		 3,  5, -1, -1, TILEIMG_SINGLEOPAQUE },
	{ Exited_Chip,		 3,  9, -1, -1, TILEIMG_SINGLEOPAQUE },
	{ Drowned_Chip,		 3,  3, -1, -1, TILEIMG_SINGLEOPAQUE },
	{ Swimming_Chip _NORTH,	 3, 12, -1, -1, TILEIMG_SINGLEOPAQUE },
	{ Swimming_Chip _WEST,	 3, 13, -1, -1, TILEIMG_SINGLEOPAQUE },
	{ Swimming_Chip _SOUTH,	 3, 14, -1, -1, TILEIMG_SINGLEOPAQUE },
	{ Swimming_Chip _EAST,	 3, 15, -1, -1, TILEIMG_SINGLEOPAQUE },
	{ Chip _NORTH,		 6, 12,  9, 12, TILEIMG_CREATURE },
	{ Chip _WEST,		 6, 13,  9, 13, TILEIMG_IMPLICIT },
	{ Chip _SOUTH,		 6, 14,  9, 14, TILEIMG_IMPLICIT },
	{ Chip _EAST,		 6, 15,  9, 15, TILEIMG_IMPLICIT },
	{ Pushing_Chip _NORTH,	 6, 12,  9, 12, TILEIMG_CREATURE },
	{ Pushing_Chip _WEST,	 6, 13,  9, 13, TILEIMG_IMPLICIT },
	{ Pushing_Chip _SOUTH,	 6, 14,  9, 14, TILEIMG_IMPLICIT },
	{ Pushing_Chip _EAST,	 6, 15,  9, 15, TILEIMG_IMPLICIT },
	{ Block _NORTH,		 0, 14, -1, -1, TILEIMG_CREATURE },
	{ Block _WEST,		 0, 15, -1, -1, TILEIMG_IMPLICIT },
	{ Block _SOUTH,		 1,  0, -1, -1, TILEIMG_IMPLICIT },
	{ Block _EAST,		 1,  1, -1, -1, TILEIMG_IMPLICIT },
	{ Tank _NORTH,		 4, 12,  7, 12, TILEIMG_CREATURE },
	{ Tank _WEST,		 4, 13,  7, 13, TILEIMG_IMPLICIT },
	{ Tank _SOUTH,		 4, 14,  7, 14, TILEIMG_IMPLICIT },
	{ Tank _EAST,		 4, 15,  7, 15, TILEIMG_IMPLICIT },
	{ Ball _NORTH,		 4,  8,  7,  8, TILEIMG_CREATURE },
	{ Ball _WEST,		 4,  9,  7,  9, TILEIMG_IMPLICIT },
	{ Ball _SOUTH,		 4, 10,  7, 10, TILEIMG_IMPLICIT },
	{ Ball _EAST,		 4, 11,  7, 11, TILEIMG_IMPLICIT },
	{ Glider _NORTH,		 5,  0,  8,  0, TILEIMG_CREATURE },
	{ Glider _WEST,		 5,  1,  8,  1, TILEIMG_IMPLICIT },
	{ Glider _SOUTH,		 5,  2,  8,  2, TILEIMG_IMPLICIT },
	{ Glider _EAST,		 5,  3,  8,  3, TILEIMG_IMPLICIT },
	{ Fireball _NORTH,		 4,  4,  7,  4, TILEIMG_CREATURE },
	{ Fireball _WEST,		 4,  5,  7,  5, TILEIMG_IMPLICIT },
	{ Fireball _SOUTH,		 4,  6,  7,  6, TILEIMG_IMPLICIT },
	{ Fireball _EAST,		 4,  7,  7,  7, TILEIMG_IMPLICIT },
	{ Bug _NORTH,		 4,  0,  7,  0, TILEIMG_CREATURE },
	{ Bug _WEST,		 4,  1,  7,  1, TILEIMG_IMPLICIT },
	{ Bug _SOUTH,		 4,  2,  7,  2, TILEIMG_IMPLICIT },
	{ Bug _EAST,		 4,  3,  7,  3, TILEIMG_IMPLICIT },
	{ Paramecium _NORTH,	 6,  0,  9,  0, TILEIMG_CREATURE },
	{ Paramecium _WEST,		 6,  1,  9,  1, TILEIMG_IMPLICIT },
	{ Paramecium _SOUTH,	 6,  2,  9,  2, TILEIMG_IMPLICIT },
	{ Paramecium _EAST,		 6,  3,  9,  3, TILEIMG_IMPLICIT },
	{ Teeth _NORTH,		 5,  4,  8,  4, TILEIMG_CREATURE },
	{ Teeth _WEST,		 5,  5,  8,  5, TILEIMG_IMPLICIT },
	{ Teeth _SOUTH,		 5,  6,  8,  6, TILEIMG_IMPLICIT },
	{ Teeth _EAST,		 5,  7,  8,  7, TILEIMG_IMPLICIT },
	{ Blob _NORTH,		 5, 12,  8, 12, TILEIMG_CREATURE },
	{ Blob _WEST,		 5, 13,  8, 13, TILEIMG_IMPLICIT },
	{ Blob _SOUTH,		 5, 14,  8, 14, TILEIMG_IMPLICIT },
	{ Blob _EAST,		 5, 15,  8, 15, TILEIMG_IMPLICIT },
	{ Walker _NORTH,		 5,  8,  8,  8, TILEIMG_CREATURE },
	{ Walker _WEST,		 5,  9,  8,  9, TILEIMG_IMPLICIT },
	{ Walker _SOUTH,		 5, 10,  8, 10, TILEIMG_IMPLICIT },
	{ Walker _EAST,		 5, 11,  8, 11, TILEIMG_IMPLICIT },
	{ Water_Splash,		 3,  3, -1, -1, TILEIMG_ANIMATION },
	{ Bomb_Explosion,		 3,  6, -1, -1, TILEIMG_ANIMATION },
	{ Entity_Explosion,		 3,  7, -1, -1, TILEIMG_ANIMATION }
};

/* The heap of remembered surfaces.
 */
static SDL_Surface    **surfaceheap = NULL;
static int		surfacesused = 0;
static int		surfacesallocated = 0;

/* The directory of tile images.
 */
static tilemap		tileptr[NTILES];

/* An internal buffer surface.
 */
static SDL_Surface     *opaquetile = NULL;

//DKS - modified to use HW surfaces
static SDL_Surface *newsurface(int w, int h, int transparency)
{
	SDL_Surface	       *s;
	//
	if (transparency) {
#ifdef PLATFORM_GCW
		s = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCALPHA,
#else
				s = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCCOLORKEY,
#endif
					w, h, sdlg.screen->format->BitsPerPixel,
					sdlg.screen->format->Rmask,
					sdlg.screen->format->Gmask,
					sdlg.screen->format->Bmask,
					sdlg.screen->format->Amask);
				} else {
				s = SDL_CreateRGBSurface(SDL_HWSURFACE,
					w, h, sdlg.screen->format->BitsPerPixel,
					sdlg.screen->format->Rmask,
					sdlg.screen->format->Gmask,
					sdlg.screen->format->Bmask,
					sdlg.screen->format->Amask);
				}

				if (!s)
				die("couldn't create surface: %s", SDL_GetError());
				return s;
}


/* Add the given surface to the heap of remembered surfaces.
 */
static void remembersurface(SDL_Surface *surface)
{
	if (surfacesused >= surfacesallocated) {
		surfacesallocated += 256;
		xalloc(surfaceheap, surfacesallocated * sizeof *surfaceheap);
	}
	surfaceheap[surfacesused++] = surface;

}

/* Free all surfaces on the heap.
 */
static void freerememberedsurfaces(void)
{
	int	n;

	for (n = 0 ; n < surfacesused ; ++n)
		if (surfaceheap[n]) {
			SDL_FreeSurface(surfaceheap[n]);
		}

	free(surfaceheap);
	surfaceheap = NULL;
	surfacesused = 0;
	surfacesallocated = 0;
}

/* Set the size of one tile. FALSE is returned if the dimensions are
 * invalid.
 */
static int settilesize(int w, int h)
{
	if (w % 4 || h % 4) {
		warn("tile dimensions must be divisible by four");
		return FALSE;
	}
	sdlg.wtile = w;
	sdlg.htile = h;
	sdlg.cptile = w * h;

	opaquetile = newsurface(w, h, FALSE);
	remembersurface(opaquetile);
	return TRUE;
}

/*
 * Functions for using tile images.
 */

/* Overlay a transparent tile image into the given tile-sized buffer.
 * index supplies the index of the transparent image.
 */
static void addtransparenttile(SDL_Surface *dest, int id, int index)
{
	SDL_Surface	       *src;
	SDL_Rect		rect = { 0, 0, sdlg.wtile, sdlg.htile };

	src = tileptr[id].transp[index];
	if (tileptr[id].transpsize & SIZE_EXTLEFT)
		rect.x += sdlg.wtile;
	if (tileptr[id].transpsize & SIZE_EXTUP)
		rect.y += sdlg.htile;
	SDL_BlitSurface(src, &rect, dest, NULL);
}

/* Return a surface for the given creature or animation. rect is
 * assumed to point to an "integral" tile location, so if moving is
 * non-zero, the dir and moving values are used to adjust rect to
 * point to the exact mid-tile position. rect is also adjusted
 * appropriately when the creature's image is larger than a single
 * tile.
 */
static SDL_Surface *_getcreatureimage(SDL_Rect *rect,
		int id, int dir, int moving, int frame)
{
	SDL_Surface	       *s;
	tilemap const      *q;
	int			n;

	if (!rect)
		die("getcreatureimage() called without a rect");

	q = tileptr + id;
	if (!isanimation(id))
		q += diridx(dir);

	if (!q->transpsize || isanimation(id)) {
		if (moving > 0) {
			switch (dir) {
				case NORTH:	rect->y += moving * sdlg.htile / 8;	break;
				case WEST:	rect->x += moving * sdlg.wtile / 8;	break;
				case SOUTH:	rect->y -= moving * sdlg.htile / 8;	break;
				case EAST:	rect->x -= moving * sdlg.wtile / 8;	break;
			}
		}
	}
	if (q->transpsize) {
		if (q->transpsize & SIZE_EXTLEFT)
			rect->x -= sdlg.wtile;
		if (q->transpsize & SIZE_EXTUP)
			rect->y -= sdlg.htile;
	}

	n = q->celcount > 1 ? frame : 0;
	if (n >= q->celcount)
		die("requested cel #%d from a %d-cel sequence (%d+%d)",
				n, q->celcount, id, diridx(dir));
	s = q->transp[n] ? q->transp[n] : q->opaque[n];

	rect->w = s->w;
	rect->h = s->h;
	return s;
}

/* Return an image of a cell with the given tiles. If the top tile is
 * transparent, the appropriate composite image is constructed in the
 * overlay buffer. (If the top tile is opaque but has transparent
 * pixels, the image returned is constructed in a private surface). If
 * rect is not NULL, the width and height fields are filled in.
 */
static SDL_Surface *_getcellimage(SDL_Rect *rect,
		int top, int bot, int timerval)
{
	SDL_Surface	       *dest;
	int			nt, nb;

	if (!tileptr[top].celcount)
		die("map element %02X has no suitable image", top);

	if (rect) {
		rect->w = sdlg.wtile;
		rect->h = sdlg.htile;
	}

	nt = (timerval + 1) % tileptr[top].celcount;
	if (bot == Nothing || bot == Empty || !tileptr[top].transp[0]) {
		if (tileptr[top].opaque[nt])
			return tileptr[top].opaque[nt];
		SDL_BlitSurface(tileptr[Empty].opaque[0], NULL, opaquetile, NULL);
		addtransparenttile(opaquetile, top, nt);
		return opaquetile;
	}

	if (!tileptr[bot].celcount)
		die("map element %02X has no suitable image", bot);
	nb = (timerval + 1) % tileptr[bot].celcount;
	dest = tileptr[Overlay_Buffer].opaque[0];
	if (tileptr[bot].opaque[nb]) {
		SDL_BlitSurface(tileptr[bot].opaque[nb], NULL, dest, NULL);
	} else {
		SDL_BlitSurface(tileptr[Empty].opaque[0], NULL, dest, NULL);
		addtransparenttile(dest, bot, nb);
	}
	addtransparenttile(dest, top, nt);

	return dest;
}

/*
 * Functions for copying individual tiles.
 */

/* Return the color of the pixel at (x, y) on the given surface. (The
 * surface must be locked before calling this function.)
 */
static Uint32 pixelat(SDL_Surface *s, int x, int y)
{
	switch (s->format->BytesPerPixel) {
		case 1:
			return (Uint32)((Uint8*)s->pixels + y * s->pitch)[x];
		case 2:
			return (Uint32)((Uint16*)((Uint8*)s->pixels + y * s->pitch))[x];
		case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			return (Uint32)((((Uint8*)s->pixels + y * s->pitch)[x * 3 + 0] << 16)
					| (((Uint8*)s->pixels + y * s->pitch)[x * 3 + 1] << 8)
					| (((Uint8*)s->pixels + y * s->pitch)[x * 3 + 2] << 0));
#else
			return (Uint32)((((Uint8*)s->pixels + y * s->pitch)[x * 3 + 0] << 0)
					| (((Uint8*)s->pixels + y * s->pitch)[x * 3 + 1] << 8)
					| (((Uint8*)s->pixels + y * s->pitch)[x * 3 + 2] << 16));
#endif
		case 4:
			return ((Uint32*)((Uint8*)s->pixels + y * s->pitch))[x];
	}
	return 0;
}

//DKS - 1/22/14 - custom SDL blit routine I wrote quickly:
// PURPOSE: I want to replace the the ugly checkered shadow pattern of the game's sprites
//				with a solid alpha-blended shadow instead. Because of the nature of SDL 1.2
//				blits, I have very little control over blit blending and need to make this
//				routine to do a raw blit from one surface to another (basically a smart memcpy).
//				Essentially, SDL is stripping the src image's alpha channel when I need a way
//				to preserve it perfectly inside the dest image.
//				Note: for now, at least, it is unoptimized because it is only meant for 
//				initialization of game sprites.
// ASSUMPTIONS: Part of the src image of dimensions "rect" will be blit to a dest image
//						sized the exact same as the blitted sub-portion.
void RawBlitSurface(SDL_Surface *src, SDL_Rect *rect, SDL_Surface *dest)
{
	if (SDL_MUSTLOCK(src))
		SDL_LockSurface(src);
	if (SDL_MUSTLOCK(dest))
		SDL_LockSurface(dest);

	uint32_t *d = (uint32_t*)dest->pixels;
	uint32_t *s = (uint32_t*)src->pixels + rect->y * src->w + rect->x;
	int x_ctr,y_ctr;

	for (y_ctr = 0; y_ctr < rect->h; y_ctr++)
	{
		for (x_ctr = 0; x_ctr < rect->w; x_ctr++)
		{
			*d++ = *s++;
		}
		s += src->w - rect->w;
	}

	if (SDL_MUSTLOCK(src))
		SDL_UnlockSurface(src);
	if (SDL_MUSTLOCK(dest))
		SDL_UnlockSurface(dest);
}

/* Create a new surface containing a single tile without any
 * transparent pixels.
 */
static SDL_Surface *extractopaquetile(SDL_Surface *src,
		int ximg, int yimg, int wimg, int himg)
{
	SDL_Surface	       *dest;
	SDL_Rect		rect;

	rect.x = ximg;
	rect.y = yimg;
	rect.w = wimg;
	rect.h = himg;
	dest = newsurface(rect.w, rect.h, FALSE);

	SDL_BlitSurface(src, &rect, dest, NULL);
	return dest;
}

//dks - modified
/* Create a new surface containing a single tile with transparent
 * pixels, as indicated by the given color key.
 */
//static SDL_Surface *extractkeyedtile(SDL_Surface *src,
//				     int ximg, int yimg, int wimg, int himg,
//				     Uint32 transpclr)
//{
//    SDL_Surface	       *dest;
//    SDL_Surface	       *temp;
//    SDL_Rect		rect;
//
//    dest = newsurface(wimg, himg, TRUE);
//    SDL_FillRect(dest, NULL, SDL_MapRGBA(dest->format,
//					 0, 0, 0, SDL_ALPHA_TRANSPARENT));
//    SDL_SetColorKey(src, SDL_SRCCOLORKEY, transpclr);
//    rect.x = ximg;
//    rect.y = yimg;
//    rect.w = dest->w;
//    rect.h = dest->h;
//    SDL_BlitSurface(src, &rect, dest, NULL);
//    SDL_SetColorKey(src, 0, 0);
//
//    temp = dest;
//    dest = SDL_DisplayFormatAlpha(temp);
//    SDL_FreeSurface(temp);
//    if (!dest)
//	die("%s", SDL_GetError());
//    SDL_SetAlpha(dest, SDL_SRCALPHA | SDL_RLEACCEL, 0);
//   
//    return dest;
//}
///* Create a new surface containing a single tile with transparent
// * pixels, as indicated by the given color key.
// */
//DKS - FOLLOWING ONE WORKS, BUT COLOR KEYING NEEDS WORK:
//static SDL_Surface *extractkeyedtile(SDL_Surface *src,
//				     int ximg, int yimg, int wimg, int himg,
//				     Uint32 transpclr)
//{
//    SDL_Surface	       *dest;
//    SDL_Surface	       *temp;
//    SDL_Rect		rect;
//
//    dest = newsurface(wimg, himg, TRUE);
// 
//#ifdef PLATFORM_GCW
////    SDL_FillRect(dest, NULL, SDL_MapRGBA(dest->format,
////					 0, 0, 0, SDL_ALPHA_TRANSPARENT));
////    SDL_SetColorKey(src, SDL_SRCCOLORKEY, transpclr);
//
//	SDL_FillRect(dest, NULL, SDL_MapRGB(dest->format, 255, 0, 255));
//    SDL_SetColorKey(src, SDL_SRCCOLORKEY, transpclr);
//
//#else
//	 //DKS
// //I'm getting rid of alpha transparency for 16bit platforms
//	SDL_FillRect(dest, NULL, SDL_MapRGB(dest->format, 255, 0, 255));
//    SDL_SetColorKey(src, SDL_SRCCOLORKEY, transpclr);
//#endif
//    rect.x = ximg;
//    rect.y = yimg;
//    rect.w = dest->w;
//    rect.h = dest->h;
//    SDL_BlitSurface(src, &rect, dest, NULL);
//    SDL_SetColorKey(src, 0, 0);
//
//    temp = dest;
//#ifdef PLATFORM_GCW
//    dest = SDL_DisplayFormatAlpha(temp);
////    dest = SDL_DisplayFormat(temp);
//#else
//    dest = SDL_DisplayFormat(temp);
//#endif
//    SDL_FreeSurface(temp);
//    if (!dest)
//	die("%s", SDL_GetError());
//#ifdef PLATFORM_GCW
////    SDL_SetAlpha(dest, SDL_SRCALPHA | SDL_RLEACCEL, 0);
//	SDL_SetColorKey(dest, SDL_SRCCOLORKEY, SDL_MapRGB(dest->format, 255, 0, 255));
//#else
//	SDL_SetColorKey(dest, SDL_SRCCOLORKEY, SDL_MapRGB(dest->format, 255, 0, 255));
//#endif
//
//   
//    return dest;
//}
//DKS - FOLLOWING ONE CAUSES BLIT PROBLEMS:
///* Create a new surface containing a single tile with transparent
// * pixels, as indicated by the given color key.
// */
//static SDL_Surface *extractkeyedtile(SDL_Surface *src,
//				     int ximg, int yimg, int wimg, int himg,
//				     Uint32 transpclr)
//{
//    SDL_Surface	       *dest;
//    SDL_Surface	       *temp;
//    SDL_Rect		rect;
//
//    dest = newsurface(wimg, himg, TRUE);
//
//	 
// 
//#ifdef PLATFORM_GCW
////    SDL_FillRect(dest, NULL, SDL_MapRGBA(dest->format,
////					 0, 0, 0, SDL_ALPHA_TRANSPARENT));
////    SDL_SetAlpha(src, SDL_SRCALPHA | SDL_RLEACCEL, 0);
//	SDL_FillRect(dest, NULL, SDL_MapRGBA(dest->format, 0, 0, 0, 255));
//	 SDL_SetAlpha(src, 0, 0);
//    SDL_SetColorKey(src, SDL_SRCCOLORKEY, transpclr);
//
////	SDL_FillRect(dest, NULL, SDL_MapRGB(dest->format, 255, 0, 255));
////    SDL_SetColorKey(src, SDL_SRCCOLORKEY, transpclr);
//
//#else
//	 //DKS
// //I'm getting rid of alpha transparency for 16bit platforms
//	SDL_FillRect(dest, NULL, SDL_MapRGB(dest->format, 255, 0, 255));
//    SDL_SetColorKey(src, SDL_SRCCOLORKEY, transpclr);
//#endif
//    rect.x = ximg;
//    rect.y = yimg;
//    rect.w = dest->w;
//    rect.h = dest->h;
//    SDL_BlitSurface(src, &rect, dest, NULL);
//    temp = dest;
//#ifdef PLATFORM_GCW
//	 SDL_SetColorKey(dest, 0, 0);
//    SDL_SetColorKey(src, 0, 0);
////    SDL_SetAlpha(dest, SDL_SRCALPHA | SDL_RLEACCEL, 255);
////    dest = SDL_DisplayFormatAlpha(temp);
////    SDL_SetAlpha(dest,0 , 0);
//#else
//    SDL_SetColorKey(src, 0, 0);
//    dest = SDL_DisplayFormat(temp);
//#endif
//
//    SDL_FreeSurface(temp);
//    if (!dest)
//	die("%s", SDL_GetError());
//#ifdef PLATFORM_GCW
////	SDL_SetColorKey(dest, SDL_SRCCOLORKEY, SDL_MapRGB(dest->format, 255, 0, 255));
//#else
//	SDL_SetColorKey(dest, SDL_SRCCOLORKEY, SDL_MapRGB(dest->format, 255, 0, 255));
//#endif
//
//   
//    return dest;
//}
static SDL_Surface *extractkeyedtile(SDL_Surface *src,
		int ximg, int yimg, int wimg, int himg,
		Uint32 transpclr)
{
	SDL_Surface	       *dest;
	SDL_Surface	       *temp;
	SDL_Rect		rect;

	temp = newsurface(wimg, himg, TRUE);

#ifdef PLATFORM_GCW
	SDL_SetColorKey(temp, 0, 0);
#else
	//DKS
	//I'm getting rid of alpha transparency for 16bit platforms
	SDL_FillRect(temp, NULL, SDL_MapRGB(dest->format, 255, 0, 255));
	SDL_SetColorKey(src, SDL_SRCCOLORKEY, transpclr);
#endif
	rect.x = ximg;
	rect.y = yimg;
	rect.w = temp->w;
	rect.h = temp->h;

#ifdef PLATFORM_GCW
	RawBlitSurface(src, &rect, temp);
	SDL_SetAlpha(temp, SDL_SRCALPHA, 0);
	dest = SDL_DisplayFormatAlpha(temp);
	SDL_SetColorKey(dest, 0, 0);
#else
	SDL_BlitSurface(src, &rect, temp, NULL);
	dest = SDL_DisplayFormat(temp);
	SDL_SetColorKey(dest, SDL_SRCCOLORKEY, transpclr);
#endif
	SDL_FreeSurface(temp);
	if (!dest)
		die("%s", SDL_GetError());
#ifdef PLATFORM_GCW
#else
	SDL_SetColorKey(dest, SDL_SRCCOLORKEY, SDL_MapRGB(dest->format, 255, 0, 255));
#endif


	return dest;
}

/* Create a new surface containing a single tile. Pixels with the
 * given transparent color are replaced with the corresponding pixels
 * from the empty tile.
 */
static SDL_Surface *extractemptytile(SDL_Surface *src,
		int ximg, int yimg, int wimg, int himg,
		Uint32 transpclr)
{
	SDL_Surface	       *dest;
	SDL_Surface	       *temp;
	SDL_Rect		rect;

	dest = newsurface(wimg, himg, FALSE);

	if (tileptr[Empty].opaque[0])
		SDL_BlitSurface(tileptr[Empty].opaque[0], NULL, dest, NULL);
	SDL_SetColorKey(src, SDL_SRCCOLORKEY, transpclr);
	rect.x = ximg;
	rect.y = yimg;
	rect.w = dest->w;
	rect.h = dest->h;
	SDL_BlitSurface(src, &rect, dest, NULL);
	SDL_SetColorKey(src, 0, 0);

	temp = dest;
	dest = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);
	if (!dest)
		die("%s", SDL_GetError());


	return dest;
}

//dks - this should never get called now
///* Create a new surface containing a single tile with transparent
// * pixels, as indicated by the mask tile.
// */
//static SDL_Surface *extractmaskedtile(SDL_Surface *src,
//				      int ximg, int yimg, int wimg, int himg,
//				      int xmask, int ymask)
//{
//    SDL_Surface	       *dest;
//    SDL_Surface	       *temp;
//    SDL_Rect		rect;
//    unsigned char      *s, *d;
//    Uint32		transp, black;
//    int			x, y;
//
//    rect.x = ximg;
//    rect.y = yimg;
//    rect.w = wimg;
//    rect.h = himg;
//    dest = newsurface(rect.w, rect.h, TRUE);
//    SDL_BlitSurface(src, &rect, dest, NULL);
//
//    black = SDL_MapRGB(src->format, 0, 0, 0);
//    transp = SDL_MapRGBA(dest->format, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
//
//    if (SDL_MUSTLOCK(src))
//	SDL_LockSurface(src);
//    if (SDL_MUSTLOCK(dest))
//	SDL_LockSurface(dest);
//    d = (Uint8*)dest->pixels;
//    s = (Uint8*)src->pixels + ymask * src->pitch
//			    + xmask * src->format->BytesPerPixel;
//    for (y = 0 ; y < dest->h ; ++y) {
//	for (x = 0 ; x < dest->w ; ++x) {
//	    if (pixelat(src, xmask + x, ymask + y) == black)
//		((Uint32*)d)[x] = transp;
//	}
//	s += src->pitch;
//	d += dest->pitch;
//    }
//    if (SDL_MUSTLOCK(src))
//	SDL_UnlockSurface(src);
//    if (SDL_MUSTLOCK(dest))
//	SDL_UnlockSurface(dest);
//
//    temp = dest;
//    dest = SDL_DisplayFormatAlpha(temp);
//    SDL_FreeSurface(temp);
//    if (!dest)
//	die("%s", SDL_GetError());
//	//dks debug
//    printf("extractopaquetile\n");
//    SDL_SetAlpha(dest, SDL_SRCALPHA | SDL_RLEACCEL, 0);
//    return dest;
//}
/* Create a new surface containing a single tile with transparent
 * pixels, as indicated by the mask tile.
 */
static SDL_Surface *extractmaskedtile(SDL_Surface *src,
		int ximg, int yimg, int wimg, int himg,
		int xmask, int ymask)
{
	//dks - debug
	printf("extractmaskedtile is being called when it shouldn't anymore.\n");


	SDL_Surface	       *dest;
	SDL_Surface	       *temp;
	SDL_Rect		rect;
	unsigned char      *s, *d;
	Uint32		transp, black;
	int			x, y;

	rect.x = ximg;
	rect.y = yimg;
	rect.w = wimg;
	rect.h = himg;
	dest = newsurface(rect.w, rect.h, TRUE);
	SDL_BlitSurface(src, &rect, dest, NULL);

	black = SDL_MapRGB(src->format, 0, 0, 0);
	transp = SDL_MapRGBA(dest->format, 0, 0, 0, SDL_ALPHA_TRANSPARENT);

	if (SDL_MUSTLOCK(src))
		SDL_LockSurface(src);
	if (SDL_MUSTLOCK(dest))
		SDL_LockSurface(dest);
	d = (Uint8*)dest->pixels;
	s = (Uint8*)src->pixels + ymask * src->pitch
		+ xmask * src->format->BytesPerPixel;
	for (y = 0 ; y < dest->h ; ++y) {
		for (x = 0 ; x < dest->w ; ++x) {
			if (pixelat(src, xmask + x, ymask + y) == black)
				((Uint32*)d)[x] = transp;
		}
		s += src->pitch;
		d += dest->pitch;
	}
	if (SDL_MUSTLOCK(src))
		SDL_UnlockSurface(src);
	if (SDL_MUSTLOCK(dest))
		SDL_UnlockSurface(dest);

	temp = dest;
	dest = SDL_DisplayFormatAlpha(temp);
	SDL_FreeSurface(temp);
	if (!dest)
		die("%s", SDL_GetError());

	SDL_SetAlpha(dest, SDL_SRCALPHA | SDL_RLEACCEL, 0);
	return dest;
}


/*
 * Reading the small format.
 */

/* Transfer the tiles to the tileptr array, using tileidmap to
 * identify and locate the individual tile images. Any magenta pixels
 * in tiles that are allowed to have transparencies are made
 * transparent.
 */
static int initsmalltileset(SDL_Surface *tiles)
{
	//dks debug
	printf("small tileset is being called when it shouldn't anymore.\n");

	SDL_Surface	       *s;
	Uint32		magenta;
	int			id, n;

	magenta = SDL_MapRGB(tiles->format, 255, 0, 255);

	for (n = 0 ; n < (int)(sizeof tileidmap / sizeof *tileidmap) ; ++n) {
		id = tileidmap[n].id;
		tileptr[id].opaque[0] = NULL;
		tileptr[id].transp[0] = NULL;
		tileptr[id].celcount = 0;
		tileptr[id].transpsize = 0;
		if (tileidmap[n].xtransp >= 0) {
			s = extractkeyedtile(tiles, tileidmap[n].xopaque * sdlg.wtile,
					tileidmap[n].yopaque * sdlg.htile,
					sdlg.wtile, sdlg.htile, magenta);
			if (!s)
				return FALSE;
			remembersurface(s);
			tileptr[id].celcount = 1;
			tileptr[id].opaque[0] = NULL;
			tileptr[id].transp[0] = s;
		} else if (tileidmap[n].xopaque >= 0) {
			s = extractopaquetile(tiles, tileidmap[n].xopaque * sdlg.wtile,
					tileidmap[n].yopaque * sdlg.htile,
					sdlg.wtile, sdlg.htile);
			if (!s)
				return FALSE;
			remembersurface(s);
			tileptr[id].celcount = 1;
			tileptr[id].opaque[0] = s;
			tileptr[id].transp[0] = NULL;
		}
	}

	return TRUE;
}

/*
 * Reading the masked format.
 */

/* Individually transfer the tiles to a one-dimensional array. The
 * black pixels in the maskimage are copied onto the indicated section
 * as transparent pixels. Then fill in the values of the tileptr
 * array, using tileidmap to identify the individual tile images.
 */
static int initmaskedtileset(SDL_Surface *tiles)
{
	//dks debug
	printf("masked tileset is being called when it shoudln't anymore.\n");

	SDL_Surface	       *s;
	int			id, n;

	for (n = 0 ; n < (int)(sizeof tileidmap / sizeof *tileidmap) ; ++n) {
		id = tileidmap[n].id;
		tileptr[id].celcount = 0;
		tileptr[id].opaque[0] = NULL;
		tileptr[id].transp[0] = NULL;
		tileptr[id].transpsize = 0;
		if (tileidmap[n].xopaque >= 0) {
			s = extractopaquetile(tiles, tileidmap[n].xopaque * sdlg.wtile,
					tileidmap[n].yopaque * sdlg.htile,
					sdlg.wtile, sdlg.htile);
			if (!s)
				return FALSE;
			remembersurface(s);
			tileptr[id].celcount = 1;
			tileptr[id].opaque[0] = s;
		}
		if (tileidmap[n].xtransp >= 0) {
			s = extractmaskedtile(tiles,
					tileidmap[n].xtransp * sdlg.wtile,
					tileidmap[n].ytransp * sdlg.htile,
					sdlg.wtile,
					sdlg.htile,
					(tileidmap[n].xtransp + 3) * sdlg.wtile,
					tileidmap[n].ytransp * sdlg.htile);
			if (!s)
				return FALSE;
			remembersurface(s);
			tileptr[id].celcount = 1;
			tileptr[id].transp[0] = s;
		}
	}

	return TRUE;
}

/*
 * Reading the large format.
 */

/* Copy a sequence of count tiles from the given surface at the
 * position indicated by rect into separate surfaces, stored in the
 * array at ptrs. transpclr indicates the color of the pixels to
 * replace with the corresponding pixels from the Empty tile.
 */
static int extractopaquetileseq(SDL_Surface *tiles, SDL_Rect const *rect,
		int count, SDL_Surface **ptrs,
		Uint32 transpclr)
{
	int	x, n;

	for (n = 0, x = rect->x ; n < count ; ++n, x += rect->w) {
		ptrs[n] = extractemptytile(tiles,
				x, rect->y, rect->w, rect->h,
				transpclr);
		if (!ptrs[n])
			return FALSE;
		remembersurface(ptrs[n]);
	}
	return TRUE;
}

/* Copy a sequence of count tiles from the given surface at the
 * position indicated by rect into separate surfaces, stored in the
 * array at ptrs. transpclr indicates the color of the pixels to make
 * transparent.
 */
static int extracttransptileseq(SDL_Surface *tiles, SDL_Rect const *rect,
		int count, SDL_Surface** ptrs,
		Uint32 transpclr)
{
	int	x, n;

	for (n = count - 1, x = rect->x ; n >= 0 ; --n, x += rect->w) {
		ptrs[n] = extractkeyedtile(tiles,
				x, rect->y, rect->w, rect->h,
				transpclr);
		if (!ptrs[n])
			return FALSE;
		remembersurface(ptrs[n]);
	}
	return TRUE;
}

/* Extract the tile images for a single tile type from the given
 * surface at the given coordinates. shape identifies the arrangement
 * of tile image(s) that may be present. transpclr indicates the color
 * of the pixels that are to be treated as transparent. pd points to a
 * pointer to the buffer where the tile images are to be copied. Upon
 * return, the pointer at pd is updated to point to the first byte in
 * the buffer after the copied tile images.
 */
static int extracttileimage(SDL_Surface *tiles, int x, int y, int w, int h,
		int id, int shape, Uint32 transpclr)
{
	SDL_Rect	rect;
	int		n;

	rect.x = x;
	rect.y = y;
	rect.w = sdlg.wtile;
	rect.h = sdlg.htile;

	switch (shape) {
		case TILEIMG_SINGLEOPAQUE:
			if (h != 1 || w != 1) {
				warn("outsized single tiles not permitted (%02X=%dx%d)", id, w, h);
				return FALSE;
			}
			tileptr[id].transpsize = 0;
			tileptr[id].celcount = 1;
			extractopaquetileseq(tiles, &rect, 1, tileptr[id].opaque, transpclr);
			break;

		case TILEIMG_OPAQUECELS:
			if (h != 1) {
				warn("outsized map tiles not permitted (%02X=%dx%d)", id, w, h);
				return FALSE;
			}
			tileptr[id].transpsize = 0;
			tileptr[id].celcount = w;
			extractopaquetileseq(tiles, &rect, w, tileptr[id].opaque, transpclr);
			break;

		case TILEIMG_TRANSPCELS:
			if (h != 1) {
				warn("outsized map tiles not permitted (%02X=%dx%d)", id, w, h);
				return FALSE;
			}
			tileptr[id].transpsize = 0;
			tileptr[id].celcount = w;
			extracttransptileseq(tiles, &rect, w, tileptr[id].transp, transpclr);
			break;

		case TILEIMG_ANIMATION:
			if (h == 2 || (h == 3 && w % 3 != 0)) {
				warn("off-center animation not permitted (%02X=%dx%d)", id, w, h);
				return FALSE;
			}
			if (h == 3) {
				tileptr[id].transpsize = SIZE_EXTALL;
				tileptr[id].celcount = w / 3;
				rect.w = 3 * sdlg.wtile;
				rect.h = 3 * sdlg.htile;
			} else {
				tileptr[id].transpsize = 0;
				tileptr[id].celcount = w;
				rect.w = sdlg.wtile;
				rect.h = sdlg.htile;
			}
			extracttransptileseq(tiles, &rect, tileptr[id].celcount,
					tileptr[id].transp, transpclr);
			if (tileptr[id].celcount < 12) {
				for (n = 11 ; n >= 0 ; --n)
					tileptr[id].transp[n]
						= tileptr[id].transp[(n * tileptr[id].celcount) / 12];
				tileptr[id].celcount = 12;
			}
			break;

		case TILEIMG_CREATURE:
			if (h == 1) {
				if (w == 1) {
					tileptr[id].transpsize = 0;
					tileptr[id].celcount = 1;
					extracttransptileseq(tiles, &rect, 1,
							tileptr[id].transp, transpclr);
					tileptr[id + 1] = tileptr[id];
					tileptr[id + 2] = tileptr[id];
					tileptr[id + 3] = tileptr[id];
				} else if (w == 2) {
					tileptr[id].transpsize = 0;
					tileptr[id].celcount = 1;
					extracttransptileseq(tiles, &rect, 1,
							tileptr[id].transp, transpclr);
					rect.x += sdlg.wtile;
					tileptr[id + 1].transpsize = 0;
					tileptr[id + 1].celcount = 1;
					extracttransptileseq(tiles, &rect, 1,
							tileptr[id + 1].transp, transpclr);
					tileptr[id + 2] = tileptr[id];
					tileptr[id + 3] = tileptr[id + 1];
				} else if (w == 4) {
					for (n = 0 ; n < 4 ; ++n) {
						tileptr[id + n].transpsize = 0;
						tileptr[id + n].celcount = 1;
						extracttransptileseq(tiles, &rect, 1,
								tileptr[id + n].transp, transpclr);
						rect.x += sdlg.wtile;
					}
				} else {
					warn("invalid packing of creature tiles (%02X=%dx%d)",
							id, w, h);
					return FALSE;
				}
			} else if (h == 2) {
				if (w == 1) {
					tileptr[id].transpsize = 0;
					tileptr[id].celcount = 1;
					extracttransptileseq(tiles, &rect, 1,
							tileptr[id].transp, transpclr);
					tileptr[id + 1].transpsize = 0;
					tileptr[id + 1].celcount = 1;
					rect.y += sdlg.htile;
					extracttransptileseq(tiles, &rect, 1,
							tileptr[id + 1].transp, transpclr);
					tileptr[id + 2] = tileptr[id];
					tileptr[id + 3] = tileptr[id + 1];
				} else if (w == 2) {
					tileptr[id].transpsize = 0;
					tileptr[id].celcount = 1;
					extracttransptileseq(tiles, &rect, 1,
							tileptr[id].transp, transpclr);
					rect.x += sdlg.wtile;
					tileptr[id + 1].transpsize = 0;
					tileptr[id + 1].celcount = 1;
					extracttransptileseq(tiles, &rect, 1,
							tileptr[id + 1].transp, transpclr);
					rect.x -= sdlg.wtile;
					rect.y += sdlg.htile;
					tileptr[id + 2].transpsize = 0;
					tileptr[id + 2].celcount = 1;
					extracttransptileseq(tiles, &rect, 1,
							tileptr[id + 2].transp, transpclr);
					rect.x += sdlg.wtile;
					tileptr[id + 3].transpsize = 0;
					tileptr[id + 3].celcount = 1;
					extracttransptileseq(tiles, &rect, 1,
							tileptr[id + 3].transp, transpclr);
				} else if (w == 8) {
					tileptr[id].transpsize = 0;
					tileptr[id].celcount = 4;
					extracttransptileseq(tiles, &rect, 4,
							tileptr[id].transp, transpclr);
					rect.x += 4 * sdlg.wtile;
					tileptr[id + 1].transpsize = 0;
					tileptr[id + 1].celcount = 4;
					extracttransptileseq(tiles, &rect, 4,
							tileptr[id + 1].transp, transpclr);
					rect.x -= 4 * sdlg.wtile;
					rect.y += sdlg.htile;
					tileptr[id + 2].transpsize = 0;
					tileptr[id + 2].celcount = 4;
					extracttransptileseq(tiles, &rect, 4,
							tileptr[id + 2].transp, transpclr);
					rect.x += 4 * sdlg.wtile;
					tileptr[id + 3].transpsize = 0;
					tileptr[id + 3].celcount = 4;
					extracttransptileseq(tiles, &rect, 4,
							tileptr[id + 3].transp, transpclr);
				} else if (w == 16) {
					rect.w = sdlg.wtile;
					rect.h = 2 * sdlg.htile;
					tileptr[id].transpsize = SIZE_EXTDOWN;
					tileptr[id].celcount = 4;
					extracttransptileseq(tiles, &rect, 4,
							tileptr[id].transp, transpclr);
					rect.x += 4 * sdlg.wtile;
					tileptr[id + 2].transpsize = SIZE_EXTUP;
					tileptr[id + 2].celcount = 4;
					extracttransptileseq(tiles, &rect, 4,
							tileptr[id + 2].transp, transpclr);
					rect.x += 4 * sdlg.wtile;
					rect.w = 2 * sdlg.wtile;
					rect.h = sdlg.htile;
					tileptr[id + 1].transpsize = SIZE_EXTRIGHT;
					tileptr[id + 1].celcount = 4;
					extracttransptileseq(tiles, &rect, 4,
							tileptr[id + 1].transp, transpclr);
					rect.y += sdlg.htile;
					tileptr[id + 3].transpsize = SIZE_EXTLEFT;
					tileptr[id + 3].celcount = 4;
					extracttransptileseq(tiles, &rect, 4,
							tileptr[id + 3].transp, transpclr);
				} else {
					warn("invalid packing of creature tiles (%02X=%dx%d)",
							id, w, h);
					return FALSE;
				}
			} else {
				warn("invalid packing of creature tiles (%02X=%dx%d)", id, w, h);
				return FALSE;
			}
			break;
	}

	return TRUE;
}

/* Extract the large-format tile images from the given surface. The
 * surface is scanned to find the delimiter pixels. Upon return, the
 * pointers to each tile image is stored in the appropriate field of
 * the tileptr array.
 */
static int initlargetileset(SDL_Surface *tiles)
{

	SDL_Rect	       *tilepos = NULL;
	Uint32		transpclr;
	int			row, nextrow;
	int			n, x, y, w, h;

	if (SDL_MUSTLOCK(tiles))
		SDL_LockSurface(tiles);

	transpclr = pixelat(tiles, 1, 0);
	for (w = 1 ; w < tiles->w ; ++w)
		if (pixelat(tiles, w, 0) != transpclr)
			break;
	if (w == tiles->w) {
		warn("Can't find tile separators");
		return FALSE;
	}
	if (w % 4 != 0) {
		warn("Tiles must have a width divisible by 4.");
		return FALSE;
	}
	for (h = 1 ; h < tiles->h ; ++h)
		if (pixelat(tiles, 0, h) != transpclr)
			break;
	--h;
	if (h % 4 != 0) {
		warn("Tiles must have a height divisible by 4.");
		return FALSE;
	}

	if (!settilesize(w, h))
		return FALSE;

	xalloc(tilepos, (sizeof tileidmap / sizeof *tileidmap) * sizeof *tilepos);

	row = 0;
	nextrow = sdlg.htile + 1;
	h = 1;
	x = 0;
	y = 0;
	for (n = 0 ; n < (int)(sizeof tileidmap / sizeof *tileidmap) ; ++n) {
		if (tileidmap[n].shape == TILEIMG_IMPLICIT)
			continue;
findwidth:
		w = 0;
		for (;;) {
			++w;
			if (x + w * sdlg.wtile >= tiles->w) {
				w = 0;
				break;
			}
			if (pixelat(tiles, x + w * sdlg.wtile, row) != transpclr)
				break;
		}
		if (!w) {
			row = nextrow;
			++nextrow;
			y += 1 + h * sdlg.htile;
			h = 0;
			do {
				++h;
				if (y + h * sdlg.htile >= tiles->h) {
					h = 0;
					break;
				}
				nextrow += sdlg.htile;
			} while (pixelat(tiles, 0, nextrow) == transpclr);
			if (!h) {
				warn("incomplete tile set: missing %02X", tileidmap[n].id);
				goto failure;
			}
			x = 0;
			goto findwidth;
		}
		tilepos[n].x = x + 1;
		tilepos[n].y = y + 1;
		tilepos[n].w = w;
		tilepos[n].h = h;
		x += w * sdlg.wtile;
	}

	if (SDL_MUSTLOCK(tiles))
		SDL_UnlockSurface(tiles);

	tileptr[Empty].transpsize = 0;
	tileptr[Empty].celcount = 1;
	tileptr[Empty].opaque[0] = extractopaquetile(tiles, 1, 1,
			sdlg.wtile, sdlg.htile);
	tileptr[Empty].transp[0] = NULL;
	remembersurface(tileptr[Empty].opaque[0]);

	for (n = 1 ; n < (int)(sizeof tileidmap / sizeof *tileidmap) ; ++n) {
		if (tileidmap[n].shape == TILEIMG_IMPLICIT)
			continue;
		if (!extracttileimage(tiles,
					tilepos[n].x, tilepos[n].y,
					tilepos[n].w, tilepos[n].h,
					tileidmap[n].id, tileidmap[n].shape, transpclr))
			goto failure;
	}

	extracttileimage(tiles, 1, 1, 1, 1, Overlay_Buffer,
			TILEIMG_SINGLEOPAQUE, transpclr);
	tileptr[Block_Static].celcount = 1;
	tileptr[Block_Static].opaque[0] = tileptr[Block].transp[0];
	tileptr[Block_Static].transp[0] = NULL;
	tileptr[HiddenWall_Perm] = tileptr[Empty];
	tileptr[HiddenWall_Temp] = tileptr[Empty];
	tileptr[BlueWall_Fake] = tileptr[BlueWall_Real];

	free(tilepos);
	return TRUE;

failure:
	freetileset();
	free(tilepos);
	return FALSE;
}

/*
 * The exported functions.
 */

/* Free all memory allocated for the current set of tile images.
 */
void freetileset(void)
{
	int	m, n;

	for (n = 0 ; n < (int)(sizeof tileptr / sizeof *tileptr) ; ++n) {
		tileptr[n].celcount = 0;
		tileptr[n].transpsize = 0;
		for (m = 0 ; m < 16 ; ++m) {
			tileptr[n].opaque[m] = NULL;
			tileptr[n].transp[m] = NULL;
		}
	}
	sdlg.wtile = 0;
	sdlg.htile = 0;
	sdlg.cptile = 0;
	opaquetile = NULL;
	freerememberedsurfaces();
}

//DKS - modified
/* Load the set of tile images stored in the given bitmap. Error
 * messages will be displayed if complain is TRUE. The return value is
 * TRUE if the tiles were successfully identified and loaded into
 * memory.
 */
int loadtileset(char const *filename, int complain)
{
	//DKS - added this so tiles only get loaded once
	if (images_loaded) 
		return TRUE;

	SDL_Surface	       *tiles = NULL;

	//dks
	SDL_Surface  	*tmpsur = NULL;

	int			f, w, h;

	//DKS changing this to PNG
	//    tiles = SDL_LoadBMP(filename);
	tiles = IMG_Load(filename);

	if (!tiles) {
		if (complain)
			errmsg(filename, "cannot read bitmap: %s", SDL_GetError());
		return FALSE;
	}
	if (tiles->format->palette && sdlg.screen->format->palette) {
		//dks debug
		printf("This shouldn't happen on GP2X.\n");
		SDL_SetColors(sdlg.screen, tiles->format->palette->colors,
				0, tiles->format->palette->ncolors);

	}

#ifdef PLATFORM_GCW
	tmpsur = SDL_DisplayFormatAlpha(tiles);
#else
	tmpsur = SDL_DisplayFormat(tiles);
#endif

	SDL_FreeSurface(tiles);
	tiles = tmpsur;

	if (tiles->w % 2 != 0) {
		freetileset();
		f = initlargetileset(tiles);
	} else if (tiles->w % 13 == 0 && tiles->h % 16 == 0) {
		w = tiles->w / 13;
		h = tiles->h / 16;
		freetileset();
		f = settilesize(w, h) && initmaskedtileset(tiles);
	} else if (tiles->w % 7 == 0 && tiles->h % 16 == 0) {
		w = tiles->w / 7;
		h = tiles->h / 16;
		freetileset();
		f = settilesize(w, h) && initsmalltileset(tiles);
	} else {
		if (complain)
			errmsg(filename, "image file has invalid dimensions (%dx%d)",
					tiles->w, tiles->h);
		f = FALSE;
	}

	SDL_FreeSurface(tiles);
	return f;
}

/* Initialization.
 */
int _sdltileinitialize(void)
{
	sdlg.getcreatureimagefunc = _getcreatureimage;
	sdlg.getcellimagefunc = _getcellimage;
	return TRUE;
}
