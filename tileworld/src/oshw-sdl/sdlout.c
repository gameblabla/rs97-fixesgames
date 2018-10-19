/* sdlout.c: Creating the program's displays.
 *
 * Copyright (C) 2001-2006 by Brian Raiter, under the GNU General Public
 * License. No warranty. See COPYING for details.
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<SDL.h>
#include	"sdlgen.h"
#include	"../err.h"
#include	"../state.h"

//DKS - for sync() call
#include <unistd.h>

//DKS - modified
/* Space to leave between graphic objects.
 */
#ifdef PLATFORM_PC
#define	MARGINW		8
#define	MARGINH		8
#else
#define	MARGINW		1
#define	MARGINH		1
#endif //PLATFORM_PC

//DKS - shouldn't need these anymore
/* Size of the prompt icons.
 */
#ifdef PLATFORM_PC
#define	PROMPTICONW	16
#define	PROMPTICONH	10
#endif //PLATFORM_PC

/* The dimensions of the visible area of the map (in tiles).
 */
#define	NXTILES		9
#define	NYTILES		9

//DKS - shouldn't need this anymore.
/* Erase a rectangle (useful for when a surface is locked).
 */
#ifdef PLATFORM_PC
#define	fillrect(r)		(puttext((r), NULL, 0, PT_MULTILINE))
#endif

/* Get a generic tile image.
 */
#define	gettileimage(id)	(getcellimage(NULL, (id), Empty, -1))

/* Structure for holding information about the message display.
 */
typedef	struct msgdisplayinfo {
	char		msg[64];	/* text of the message */
	unsigned int	msglen;		/* length of the message */
	unsigned long	until;		/* when to erase the message */
	unsigned long	bolduntil;	/* when to dim the message */
} msgdisplayinfo;

/* The message display.
 */
static msgdisplayinfo	msgdisplay;

//DKS - shouldn't need these anymore
/* Some prompting icons.
 */
#ifdef PLATFORM_PC
static SDL_Surface     *prompticons = NULL;
#endif // PLATFORM_PC

/* TRUE means the program should attempt to run in fullscreen mode.
 */
//DKS - not sure why we need two instances of fullscreen (one elsewhere) but we
//      need to alter this anyway:
#ifdef PLATFORM_PC
static int		fullscreen = FALSE;
#else
static int		fullscreen = TRUE;
#endif // PLATFORM_PC

/* Coordinates specifying the placement of the various screen elements.
 */
//DKS - modified to constants
static const int	screenw = 320;
static const int    screenh = 240;
//DKS - new
static const int    screenbpp = 32;

static SDL_Rect		rinfoloc;
static SDL_Rect		locrects[8];

#define	displayloc	(locrects[0])
#define	titleloc	(locrects[1])
#define	infoloc		(locrects[2])
#define	invloc		(locrects[3])
#define	hintloc		(locrects[4])
#define	rscoreloc	(locrects[5])
#define	messageloc	(locrects[6])
#define	promptloc	(locrects[7])

/* TRUE means that the screen is in need of a full update.
 */
static int		fullredraw = TRUE;

/* Coordinates of the NW corner of the visible part of the map
 * (measured in quarter-tiles), or -1 if no map is currently visible.
 */
static int		mapvieworigin = -1;

/*
 * Display initialization functions.
 */

//DKS
#ifdef PLATFORM_PC
/* Set up a fontcolors structure, calculating the middle color from
 * the other two.
 */
static fontcolors makefontcolors(int rbkgnd, int gbkgnd, int bbkgnd,
		int rtext, int gtext, int btext)
{
	fontcolors	colors;

	colors.c[0] = SDL_MapRGB(sdlg.screen->format, rbkgnd, gbkgnd, bbkgnd);
	colors.c[2] = SDL_MapRGB(sdlg.screen->format, rtext, gtext, btext);
	colors.c[1] = SDL_MapRGB(sdlg.screen->format, (rbkgnd + rtext) / 2,
			(gbkgnd + gtext) / 2,
			(bbkgnd + btext) / 2);
	return colors;
}

/* Create three simple icons, to be used when prompting the user.
 */
static int createprompticons(void)
{
	static Uint8 iconpixels[] = {
		0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,2,2,1,0,0,0,0,0,0,
		0,0,0,0,1,2,2,2,2,1,0,0,0,0,0,0,
		0,0,1,2,2,2,2,2,2,1,0,0,0,0,0,0,
		1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
		1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
		0,0,1,2,2,2,2,2,2,1,0,0,0,0,0,0,
		0,0,0,0,1,2,2,2,2,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,2,2,1,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
		0,0,0,0,0,2,2,2,2,2,2,0,0,0,0,0,
		0,0,0,0,2,2,2,2,2,2,2,2,0,0,0,0,
		0,0,0,2,2,2,2,2,2,2,2,2,2,0,0,0,
		0,0,0,2,2,2,2,2,2,2,2,2,2,0,0,0,
		0,0,0,2,2,2,2,2,2,2,2,2,2,0,0,0,
		0,0,0,2,2,2,2,2,2,2,2,2,2,0,0,0,
		0,0,0,2,2,2,2,2,2,2,2,2,2,0,0,0,
		0,0,0,0,2,2,2,2,2,2,2,2,0,0,0,0,
		0,0,0,0,0,2,2,2,2,2,2,0,0,0,0,0,
		0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,2,2,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,2,2,2,2,1,0,0,0,0,
		0,0,0,0,0,0,1,2,2,2,2,2,2,1,0,0,
		2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,
		2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,
		0,0,0,0,0,0,1,2,2,2,2,2,2,1,0,0,
		0,0,0,0,0,0,1,2,2,2,2,1,0,0,0,0,
		0,0,0,0,0,0,1,2,2,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0
	};

	if (!prompticons) {
		prompticons = SDL_CreateRGBSurfaceFrom(iconpixels,
				PROMPTICONW, 3 * PROMPTICONH,
				8, PROMPTICONW, 0, 0, 0, 0);
		if (!prompticons) {
			warn("couldn't create SDL surface: %s", SDL_GetError());
			return FALSE;
		}
	}

	SDL_GetRGB(bkgndcolor(sdlg.dimtextclr), sdlg.screen->format,
			&prompticons->format->palette->colors[0].r,
			&prompticons->format->palette->colors[0].g,
			&prompticons->format->palette->colors[0].b);
	SDL_GetRGB(halfcolor(sdlg.dimtextclr), sdlg.screen->format,
			&prompticons->format->palette->colors[1].r,
			&prompticons->format->palette->colors[1].g,
			&prompticons->format->palette->colors[1].b);
	SDL_GetRGB(textcolor(sdlg.dimtextclr), sdlg.screen->format,
			&prompticons->format->palette->colors[2].r,
			&prompticons->format->palette->colors[2].g,
			&prompticons->format->palette->colors[2].b);

	return TRUE;
}
#endif //PLATFORM_PC

//DKS - modified
/* Calculate the placements of all the separate elements of the
 * display.
 */
static int layoutscreen(void)
{
	//    static char const  *scoretext = "888  DRAWN AND QUARTERED"
	//				    "   88,888  8,888,888  8,888,888";
	//    static char const  *hinttext = "Total Score  ";
	//    static char const  *rscoretext = "88888888";
	//    static char const  *chipstext = "Chips";
	//    static char const  *timertext = " 88888";
	//
	//    int			fullw, infow, rscorew, texth;
	//
	if (sdlg.wtile <= 0 || sdlg.htile <= 0)
		return FALSE;

	//DKS - these will all be hard-coded for now
	displayloc.x = 4;
	displayloc.y = 11;
	displayloc.w = NXTILES * sdlg.wtile;
	displayloc.h = NYTILES * sdlg.htile;

	titleloc.x = 0;
	titleloc.y = 0;
	titleloc.w = 0;
	titleloc.h = 0;

	infoloc.x = 0;
	infoloc.y = 0;
	infoloc.w = 0;
	infoloc.h = 0;

	rinfoloc.x = 0;
	rinfoloc.y = 0;
	rinfoloc.h = 0;
	rinfoloc.w = 0;


	invloc.x = 223;
	invloc.y = 166;
	invloc.w = 4 * sdlg.wtile;
	invloc.h = 2 * sdlg.htile;

	promptloc.x = 0;
	promptloc.y = 0;
	promptloc.h = 0;
	promptloc.w = 0;

	messageloc.x = 10;
	messageloc.y = 10;
	messageloc.h = 132;
	messageloc.w = 300;

	hintloc.x = 10;
	hintloc.y = 30;
	hintloc.h = 220;
	hintloc.w = 300;

	rscoreloc.x = 0;
	rscoreloc.y = 0;
	rscoreloc.h = 0;
	rscoreloc.w = 0;


	return TRUE;
}

//DKS - modified
/* Create or change the program's display surface.
 */
static int createdisplay(void)
{
	int	flags;

	if (sdlg.realscreen) {
		SDL_FreeSurface(sdlg.realscreen);
		sdlg.realscreen = NULL;
	}

	if (sdlg.screen) {
		SDL_FreeSurface(sdlg.screen);
		sdlg.screen = NULL;
	}

	//DKS
	//    flags = SDL_SWSURFACE | SDL_ANYFORMAT;
#ifdef PLATFORM_PC
	flags = SDL_SWSURFACE | SDL_ANYFORMAT;
#elif PLATFORM_GCW
	//    flags = SDL_SWSURFACE;
	flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
#else
	flags = SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN;
#endif

	//sdlg.realscreen = SDL_SetVideoMode(320, 240, 32, flags);
	sdlg.ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
	sdlg.realscreen = SDL_CreateRGBSurface(SDL_HWSURFACE, 320, 240, 32, 0, 0, 0, 0);
	sync();

	//DKS new
	sdlg.screen = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCCOLORKEY | SDL_RLEACCEL,
			sdlg.realscreen->w, sdlg.realscreen->h, sdlg.realscreen->format->BitsPerPixel,
			sdlg.realscreen->format->Rmask, sdlg.realscreen->format->Gmask,
			sdlg.realscreen->format->Bmask, sdlg.realscreen->format-> Amask);


	if (sdlg.realscreen->w != screenw || sdlg.realscreen->h != screenh)
		warn("requested a %dx%d display, got %dx%d instead",
				sdlg.realscreen->w, sdlg.realscreen->h);
	return TRUE;

}

//DKS - new
//This function overlays a black surface atop *sur, and the alpha channel is set to 
//level
void dimsurface(SDL_Surface *sur, int level)
{
	//DKS - altered for GCW
	SDL_Surface *tmpsur = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCCOLORKEY | SDL_RLEACCEL,
			sur->w, sur->h, sur->format->BitsPerPixel,
			sur->format->Rmask, sur->format->Gmask,
			sur->format->Bmask, sur->format-> Amask);

	SDL_FillRect(tmpsur, NULL, SDL_MapRGB(tmpsur->format, 0, 0, 0));
	SDL_SetAlpha(tmpsur, SDL_RLEACCEL | SDL_SRCALPHA, level);
	SDL_BlitSurface(tmpsur, NULL, sur, NULL);
	SDL_FreeSurface(tmpsur);
}

//DKS - modified
/* Wipe the display.
 */
void cleardisplay(void)
{
	SDL_FillRect(sdlg.screen, NULL, SDL_MapRGB(sdlg.screen->format, 0, 0, 0));
	fullredraw = TRUE;
	mapvieworigin = -1;
}


//DKS - modified
/*
 * Tile rendering functions.
 */

/* Copy a single tile to the position (xpos, ypos).
 */
static void drawfulltile(int xpos, int ypos, SDL_Surface *src)
{
	SDL_Rect	rect = { xpos, ypos, src->w, src->h };

	if (SDL_BlitSurface(src, NULL, sdlg.screen, &rect))
		warn("%s", SDL_GetError());
}

/* Copy a tile to the position (xpos, ypos) but clipped to the
 * displayloc rectangle.
 */
static void drawclippedtile(SDL_Rect const *rect, SDL_Surface *src)
{
	int	xoff, yoff, w, h;

	xoff = 0;
	if (rect->x < displayloc.x)
		xoff = displayloc.x - rect->x;
	yoff = 0;
	if (rect->y < displayloc.y)
		yoff = displayloc.y - rect->y;
	w = rect->w - xoff;
	if (rect->x + rect->w > displayloc.x + displayloc.w)
		w -= (rect->x + rect->w) - (displayloc.x + displayloc.w);
	h = rect->h - yoff;
	if (rect->y + rect->h > displayloc.y + displayloc.h)
		h -= (rect->y + rect->h) - (displayloc.y + displayloc.h);
	if (w <= 0 || h <= 0)
		return;

	{
		SDL_Rect srect = { xoff, yoff, w, h };
		SDL_Rect drect = { rect->x + xoff, rect->y + yoff, 0, 0 };

		if (SDL_BlitSurface(src, &srect, sdlg.screen, &drect))
			warn("%s", SDL_GetError());
	}
}

/*
 * Message display function.
 */

//DKS - modified
/* Refresh the message-display message. If update is TRUE, the screen
 * is updated immediately.
 */
static void displaymsg(int update)
{
	//DKS - don't need anymore
}


//DKS - modified
/* Change the current message-display message. msecs gives the number
 * of milliseconds to display the message, and bold specifies the
 * number of milliseconds to display the message highlighted.
 */
int setdisplaymsg(char const *msg, int msecs, int bold)
{
	if (!msg || !*msg) {
		*msgdisplay.msg = '\0';
		msgdisplay.msglen = 0;
	} else {
		msgdisplay.msglen = strlen(msg);
		if (msgdisplay.msglen >= sizeof msgdisplay.msg)
			msgdisplay.msglen = sizeof msgdisplay.msg - 1;
		memcpy(msgdisplay.msg, msg, msgdisplay.msglen);
		msgdisplay.msg[msgdisplay.msglen] = '\0';
		msgdisplay.until = SDL_GetTicks() + msecs;
		msgdisplay.bolduntil = SDL_GetTicks() + bold;
	}
	displaymsg(TRUE);
	return TRUE;
}

/*
 * The main display functions.
 */

//DKS - modified to not use CHAR_MZERO
/* Create a string representing a decimal number.
 */
static char const *decimal(long number, int places)
{
	static char		buf[32];
	char	       *dest = buf + sizeof buf;
	unsigned long	n;


	n = number >= 0 ? (unsigned long)number : (unsigned long)-(number + 1) + 1;
	*--dest = '\0';
	do {
		//	*--dest = CHAR_MZERO + n % 10;
		//DKS - modifed for our purposes (use regular ascii not the weird font.bmp ref#)
		*--dest = 48 + n % 10;
		n /= 10;
	} while (n);
	while (buf + sizeof buf - dest < places + 1)
		//	*--dest = CHAR_MZERO;
		*--dest = 48 + n % 10;
	if (number < 0)
		*--dest = '-';
	return dest;

}

//DKS - made a stub, pretty sure we don't need this anymore
/* Display an empty map view.
 */
static void displayshutter(void)
{
}

/* Render the view of the visible area of the map to the display, with
 * the view position centered on the display as much as possible. The
 * gamestate's map and the list of creatures are consulted to
 * determine what to render.
 */
static void displaymapview(gamestate const *state)
{
	SDL_Rect		rect;
	SDL_Surface	       *s;
	creature const     *cr;
	int			xdisppos, ydisppos;
	int			xorigin, yorigin;
	int			lmap, tmap, rmap, bmap;
	int			pos, x, y;

	if (state->statusflags & SF_SHUTTERED) {
		displayshutter();
		return;
	}

	xdisppos = state->xviewpos / 2 - (NXTILES / 2) * 4;
	ydisppos = state->yviewpos / 2 - (NYTILES / 2) * 4;
	if (xdisppos < 0)
		xdisppos = 0;
	if (ydisppos < 0)
		ydisppos = 0;
	if (xdisppos > (CXGRID - NXTILES) * 4)
		xdisppos = (CXGRID - NXTILES) * 4;
	if (ydisppos > (CYGRID - NYTILES) * 4)
		ydisppos = (CYGRID - NYTILES) * 4;
	xorigin = displayloc.x - (xdisppos * sdlg.wtile / 4);
	yorigin = displayloc.y - (ydisppos * sdlg.htile / 4);

	mapvieworigin = ydisppos * CXGRID * 4 + xdisppos;

	lmap = xdisppos / 4;
	tmap = ydisppos / 4;
	rmap = (xdisppos + 3) / 4 + NXTILES;
	bmap = (ydisppos + 3) / 4 + NYTILES;
	for (y = tmap ; y < bmap ; ++y) {
		if (y < 0 || y >= CXGRID)
			continue;
		for (x = lmap ; x < rmap ; ++x) {
			if (x < 0 || x >= CXGRID)
				continue;
			pos = y * CXGRID + x;
			rect.x = xorigin + x * sdlg.wtile;
			rect.y = yorigin + y * sdlg.htile;
			s = getcellimage(&rect,
					state->map[pos].top.id,
					state->map[pos].bot.id,
					(state->statusflags & SF_NOANIMATION) ?
					-1 : state->currenttime);
			drawclippedtile(&rect, s);
		}
	}

	lmap -= 2;
	tmap -= 2;
	rmap += 2;
	bmap += 2;
	for (cr = state->creatures ; cr->id ; ++cr) {
		if (cr->hidden)
			continue;
		x = cr->pos % CXGRID;
		y = cr->pos / CXGRID;
		if (x < lmap || x >= rmap || y < tmap || y >= bmap)
			continue;
		rect.x = xorigin + x * sdlg.wtile;
		rect.y = yorigin + y * sdlg.htile;
		s = getcreatureimage(&rect, cr->id, cr->dir, cr->moving, cr->frame);
		drawclippedtile(&rect, s);
	}
}

//DKS - modified
/* Render all the various nuggets of data that comprise the
 * information display. timeleft and besttime supply the current timer
 * value and the player's best recorded time as measured in seconds.
 * The level's title, number, password, and hint, the count of chips
 * needed, and the keys and boots in possession are all used as well
 * in creating the display.
 */
static void displayinfo(gamestate const *state, int timeleft, int besttime, int showhint)
{
	//DKS - disabled most of this, except for inventory
	int n;
	for (n = 0 ; n < 4 ; ++n) {
		drawfulltile(invloc.x + n * sdlg.wtile, invloc.y,
				gettileimage(state->keys[n] ? Key_Red + n : Empty));
		drawfulltile(invloc.x + n * sdlg.wtile, invloc.y + sdlg.htile,
				gettileimage(state->boots[n] ? Boots_Ice + n : Empty));
	}

	//draw level number as LEDs
	int number_drawn;
	char	tmpstr[4];

	if (state->game->number > 0) {
		number_drawn = (state->game->number < 999) ? state->game->number : 999;	
		sprintf(tmpstr, "%03d", number_drawn);
		SFont_Write(sdlg.screen, sdlg.font_led_big, 241, 25, tmpstr);
	} else {
		strcpy(tmpstr, "---");
		SFont_Write(sdlg.screen, sdlg.font_led_big, 241, 25, tmpstr);
	}


	//draw time left as LEDs
	if ((timeleft != TIME_NIL) && (timeleft >= 0)) {
		number_drawn = (timeleft < 999) ? timeleft : 999;
		sprintf(tmpstr, "%03d", number_drawn);
		SFont_Write(sdlg.screen, sdlg.font_led_big, 241, 70, tmpstr);
	} else {
		strcpy(tmpstr, "---");
		SFont_Write(sdlg.screen, sdlg.font_led_big, 241, 70, tmpstr);		
	}

	//draw chips remaining
	if (state->chipsneeded >= 0) {
		number_drawn = (state->chipsneeded < 999) ? state->chipsneeded : 999;
		sprintf(tmpstr, "%03d", number_drawn);
		SFont_Write(sdlg.screen, sdlg.font_led_big, 241, 116, tmpstr);

	} else {
		strcpy(tmpstr, "---");
		SFont_Write(sdlg.screen, sdlg.font_led_big, 241, 116, tmpstr);
	}

	if (state->statusflags & SF_INVALID) {
		SDL_Rect tmprect;
		tmprect.x = 4;
		tmprect.y = 4;
		tmprect.h = 148;
		tmprect.w = 312;
		SDL_BlitSurface(sdlg.infobg, &tmprect, sdlg.screen, &tmprect);
		drawmultilinetext(sdlg.screen, &messageloc, 
				"Sorry, this level cannot be played with the Lynx ruleset.", -1, 0, 
				sdlg.font_small, 1);  
	} else if (state->currenttime < 0 && state->game->unsolvable) {
		//DKS - in testing, I found these don't properly report 
		// unsolvable levels.  Since the database of them is so small,
		// I might not ever be bothered to fix this 11/07/07

		if (*state->game->unsolvable) {
			char buf[256];
			strcpy(buf, "This level is reported to be unsolvable: ");
			n = sizeof(buf) - strlen(buf) - 2;
			strncpy(buf, state->game->unsolvable, n);
			buf[255] = '\0';

			SDL_Rect tmprect;
			tmprect.x = 4;
			tmprect.y = 4;
			tmprect.h = 148;
			tmprect.w = 312;
			SDL_BlitSurface(sdlg.infobg, &tmprect, sdlg.screen, &tmprect);
			drawmultilinetext(sdlg.screen, &messageloc, buf, -1, 0, 
					sdlg.font_small, 1);  
		} else {
			//DKS - in testing, I found these don't properly report 
			// unsolvable levels.  Since the database of them is so small,
			// I might not ever be bothered to fix this 11/07/07

			SDL_Rect tmprect;
			tmprect.x = 4;
			tmprect.y = 4;
			tmprect.h = 148;
			tmprect.w = 312;
			SDL_BlitSurface(sdlg.infobg, &tmprect, sdlg.screen, &tmprect);
			drawmultilinetext(sdlg.screen, &messageloc, 
					"This level is reported to be unsolvable.", -1, 0, 
					sdlg.font_small, 1);
		}
	} else if ((state->statusflags & SF_SHOWHINT) && showhint)
	{
		SDL_Rect tmprect;
		tmprect.x = 4;
		tmprect.y = 4;
		tmprect.h = 232;
		tmprect.w = 312;

		SDL_BlitSurface(sdlg.hintbg, &tmprect, sdlg.screen, &tmprect);

		SFont_WriteCenter(sdlg.screen, sdlg.font_small, 10, "-HINT-");

		drawmultilinetext(sdlg.screen, &hintloc, 
				state->hinttext, -1, 0, 
				sdlg.font_small, 1);
	}
}

//DKS - modified
/* Display a prompt icon in the lower right-hand corner. completed is
 * -1, 0, or +1, depending on which icon is being requested.
 */
static int displayprompticon(int completed)
{
	//DKS - don't need anymore
	return TRUE;
}


/*
 * The exported functions.
 */

/* Given a pixel's coordinates, return the integer identifying the
 * tile's position in the map, or -1 if the pixel is not on the map view.
 */
int _windowmappos(int x, int y)
{
	if (mapvieworigin < 0)
		return -1;
	if (x < displayloc.x || y < displayloc.y)
		return -1;
	x = (x - displayloc.x) * 4 / sdlg.wtile;
	y = (y - displayloc.y) * 4 / sdlg.htile;
	if (x >= NXTILES * 4 || y >= NYTILES * 4)
		return -1;
	x = (x + mapvieworigin % (CXGRID * 4)) / 4;
	y = (y + mapvieworigin / (CXGRID * 4)) / 4;
	if (x < 0 || x >= CXGRID || y < 0 || y >= CYGRID) {
		warn("mouse moved off the map: (%d %d)", x, y);
		return -1;
	}
	return y * CXGRID + x;
}

//DKS - don't need this anymore.
/* Set the four main colors used to render text on the display.
 */
//void setcolors(long bkgnd, long text, long bold, long dim)
//{
//    int	bkgndr, bkgndg, bkgndb;
//
//    if (bkgnd < 0)
//	bkgnd = 0x000000;
//    if (text < 0)
//	text = 0xFFFFFF;
//    if (bold < 0)
//	bold = 0xFFFF00;
//    if (dim < 0)
//	dim = 0xC0C0C0;
//
//    if (bkgnd == text || bkgnd == bold || bkgnd == dim) {
//	errmsg(NULL, "one or more text colors matches the background color; "
//		     "color scheme left unchanged.");
//	return;
//    }
//
//    bkgndr = (bkgnd >> 16) & 255;
//    bkgndg = (bkgnd >> 8) & 255;
//    bkgndb = bkgnd & 255;
//
//    sdlg.textclr = makefontcolors(bkgndr, bkgndg, bkgndb,
//			(text >> 16) & 255, (text >> 8) & 255, text & 255);
//    sdlg.dimtextclr = makefontcolors(bkgndr, bkgndg, bkgndb,
//			(dim >> 16) & 255, (dim >> 8) & 255, dim & 255);
//    sdlg.hilightclr = makefontcolors(bkgndr, bkgndg, bkgndb,
//			(bold >> 16) & 255, (bold >> 8) & 255, bold & 255);
//
//    createprompticons();
//}

//DKS - modified
/* Create the game's display. state is a pointer to the gamestate
 * structure.
 */
// DKS -  show hint is new field saying whether or not to show the hint
//			if there is one.  This is to fix a problem in maps where the hint
//			is in the player's starting square and we're still in the level
//			select starting screen
int displaygame(void const *state, int timeleft, int besttime, int showhint)
{
	SDL_BlitSurface(sdlg.playbg, NULL, sdlg.screen, NULL);

	displaymapview(state);
	displayinfo(state, timeleft, besttime, showhint);
	displaymsg(FALSE);

	SDL_BlitSurface(sdlg.screen, NULL, sdlg.realscreen, NULL);
	//SDL_Flip(sdlg.realscreen);
	myflip();

	return TRUE;
}

//DKS - modified and added new parameters newbesttime and wasbesttime.
/* Update the display to acknowledge the end of game play. completed
 * is positive if the play was successful or negative if unsuccessful.
 * If the latter, then the other arguments can contain point values
 * that will be reported to the user.
 */
int displayendmessage(int basescore, int timescore, long totalscore,
		int completed, int newbesttime, int wasbesttime)
{
	SDL_Rect tmprect;
	int		fullscore;
	if (totalscore) {
		fullscore = timescore + basescore;

		tmprect.x = 0;
		tmprect.y = wasbesttime ? 15 : 100; 
		tmprect.w = sdlg.screen->w;
		tmprect.h = sdlg.screen->h;
		dimsurface(sdlg.screen, 160);
		drawtext(sdlg.screen, &tmprect, "Level Completed", -1, PT_CENTER,
				sdlg.font_big, 1);

		if (wasbesttime)
		{
			tmprect.y = 55;
			tmprect.x = 45;
			tmprect.w = sdlg.screen->w - tmprect.x - 1;
			tmprect.h = sdlg.screen->h - tmprect.y - 1;
			drawtext(sdlg.screen, &tmprect, "Time Bonus", -1, PT_UPDATERECT,
					sdlg.font_small, 1);
			drawtext(sdlg.screen, &tmprect, "Level Bonus", -1, PT_UPDATERECT,
					sdlg.font_small, 1);
			drawtext(sdlg.screen, &tmprect, "Level Score", -1, PT_UPDATERECT, 
					sdlg.font_small, 1);
			drawtext(sdlg.screen, &tmprect, "Total Score", -1, PT_UPDATERECT,
					sdlg.font_small, 1);
			tmprect.x = 0;
			tmprect.y = 55;

			tmprect.w = sdlg.screen->w - 45;
			tmprect.h = sdlg.screen->h - 45;
			drawtext(sdlg.screen, &tmprect, decimal(timescore, 4), -1,  
					PT_RIGHT | PT_UPDATERECT, sdlg.font_small, 1);
			drawtext(sdlg.screen, &tmprect, decimal(basescore, 5), -1, 
					PT_RIGHT | PT_UPDATERECT, sdlg.font_small, 1);
			drawtext(sdlg.screen, &tmprect, decimal(fullscore, 5), -1,  
					PT_RIGHT | PT_UPDATERECT, sdlg.font_small, 1);
			drawtext(sdlg.screen, &tmprect, decimal(totalscore, 7), -1, 
					PT_RIGHT | PT_UPDATERECT, sdlg.font_small, 1);

			char tmpstr[80];
			//draw the stopwatch graphic
			SDL_Rect dstrect;
			tmprect.x = 199;
			tmprect.y = 0;
			tmprect.w = 30;
			tmprect.h = 34;

			dstrect.x = 47;
			dstrect.y = 142;
			dstrect.w = tmprect.w;
			dstrect.h = tmprect.h;
			SDL_BlitSurface(sdlg.sprites, &tmprect, sdlg.screen, &dstrect);

			SFont_Write(sdlg.screen, sdlg.font_small, 100, 153, "NEW RECORD TIME!");  
			if (newbesttime <= 0)
			{
				sprintf(tmpstr, "%d SECONDS", abs(newbesttime));
				SFont_WriteCenter(sdlg.screen, sdlg.font_small, 190, tmpstr); 
			} else {
				sprintf(tmpstr, "%d SECONDS REMAINING", newbesttime);
				SFont_WriteCenter(sdlg.screen, sdlg.font_small, 190, tmpstr); 
			}
		}
	}

	return TRUE;
}


int displaytable(char const *title, tablespec const *table, int completed)
{
	//disabled for now
	return TRUE;
}

//DKS - modified
/* Render a table with embedded illustrations on the display. title is
 * a short string to display under the table. rows is an array of
 * count lines of text, each accompanied by one or two illustrations.
 * completed determines the prompt icon that will be displayed in the
 * lower right-hand corner.
 */
int displaytiletable(char const *title,
		tiletablerow const *rows, int count, int completed)
{
	//DKS - disabled for now
	return TRUE;
}

//DKS - modified
/* Render a table as a scrollable list on the display. One row is
 * highlighted as the current selection, initially set by the integer
 * pointed to by idx. The callback function inputcallback is called
 * repeatedly to determine how to move the selection and when to
 * leave. The row selected when the function returns is returned to
 * the caller through idx.
 */
int displaylist(char const *title, void const *tab, int *idx,
		int (*inputcallback)(int*))
{
	//DKS - disabled for now
}

//DKS - modified
/* Display a line of text, given by prompt, at the center of the display.
 * The callback function inputcallback is then called repeatedly to
 * obtain input characters, which are collected in input. maxlen sets an
 * upper limit to the length of the input so collected.
 */
int displayinputprompt(char const *prompt, char *input, int maxlen,
		int (*inputcallback)(void))
{
	//DKS - disabled for now

}

//DKS - modified, we only want to call createdisplay() once at the beginning
/* Create a display surface appropriate to the requirements of the
 * game.
 */
int creategamedisplay(void)
{
	if (!layoutscreen())
		return FALSE;
	cleardisplay();
	return TRUE;
}

//DKS - modified
/* Initialize the display with a generic surface capable of rendering
 * text.
 */
int _sdloutputinitialize(int _fullscreen)
{
	sdlg.windowmapposfunc = _windowmappos;
	fullscreen = _fullscreen;

	//DKS - these are now constants at top
	//    screenw = 640;
	//    screenh = 480;    promptloc.x = screenw - MARGINW - PROMPTICONW;

	//DKS - don't need anymore
	//    promptloc.y = screenh - MARGINH - PROMPTICONH;
	//    promptloc.w = PROMPTICONW;
	//    promptloc.h = PROMPTICONH;

	createdisplay();
	cleardisplay();

#ifndef PLATFORM_PC
	SDL_ShowCursor(SDL_DISABLE);
#endif

	SDL_Surface *font_img_sur, *tempsur;

	font_img_sur = IMG_Load("res/font_subatomic.png");
	if (!font_img_sur)
	{
		printf ( "font_img_sur IMG_Load error\n" );
		return FALSE;
	}
	tempsur = SDL_DisplayFormat(font_img_sur);
	sdlg.font_tiny = SFont_InitFont(tempsur);
	SDL_FreeSurface(font_img_sur);

	font_img_sur = IMG_Load("res/font_geekabyte_big.png");
	if (!font_img_sur)
	{
		printf ( "font_img_sur IMG_Load error\n" );
		return FALSE;
	}
	tempsur = SDL_DisplayFormat(font_img_sur);
	sdlg.font_big = SFont_InitFont(tempsur);
	SDL_FreeSurface(font_img_sur);

	font_img_sur = IMG_Load("res/font_geekabyte_small.png");
	if (!font_img_sur)
	{
		printf ( "font_img_sur IMG_Load error\n" );
		return FALSE;
	}
	tempsur = SDL_DisplayFormat(font_img_sur);
	sdlg.font_small = SFont_InitFont(tempsur);
	SDL_FreeSurface(font_img_sur);

	font_img_sur = IMG_Load("res/font_led_big.png");
	if (!font_img_sur)
	{
		printf ( "font_img_sur IMG_Load error\n" );
		return FALSE;
	}
	tempsur = SDL_DisplayFormat(font_img_sur);
	sdlg.font_led_big = SFont_InitFont(tempsur);
	SDL_FreeSurface(font_img_sur);

	//load menu background and playfield backgrounds
	tempsur = IMG_Load("res/menubg.png");
	if (!tempsur) 
	{
		printf ( "res/menubg.png IMG_Load error\n" );
		return FALSE;
	}
	sdlg.menubg = SDL_DisplayFormat(tempsur);
	SDL_FreeSurface(tempsur);

	tempsur = IMG_Load("res/playbg.png");
	if (!tempsur) 
	{
		printf ( "res/playbg.png IMG_Load error\n" );
		return FALSE;
	}
	sdlg.playbg = SDL_DisplayFormat(tempsur);
	SDL_FreeSurface(tempsur);	

	tempsur = IMG_Load("res/infobg.png");
	if (!tempsur) 
	{
		printf ( "res/infobg.png IMG_Load error\n" );
		return FALSE;
	}
	sdlg.infobg = SDL_DisplayFormat(tempsur);
	SDL_FreeSurface(tempsur);	
	SDL_SetAlpha(sdlg.infobg, SDL_SRCALPHA, 190);
	SDL_SetColorKey(sdlg.infobg, SDL_SRCCOLORKEY | SDL_RLEACCEL,
			SDL_MapRGB(sdlg.infobg->format, 255, 0, 255));

	tempsur = IMG_Load("res/hintbg.png");
	if (!tempsur) 
	{
		printf ( "res/hintbg.png IMG_Load error\n" );
		return FALSE;
	}
	sdlg.hintbg = SDL_DisplayFormat(tempsur);
	SDL_FreeSurface(tempsur);	
	SDL_SetAlpha(sdlg.hintbg, SDL_SRCALPHA, 190);
	SDL_SetColorKey(sdlg.hintbg, SDL_SRCCOLORKEY | SDL_RLEACCEL,
			SDL_MapRGB(sdlg.hintbg->format, 255, 0, 255));

	tempsur = IMG_Load("res/oopsbg.png");
	if (!tempsur) 
	{
		printf ( "res/oopsbg.png IMG_Load error\n" );
		return FALSE;
	}
	sdlg.oopsbg = SDL_DisplayFormat(tempsur);
	SDL_FreeSurface(tempsur);	
	SDL_SetAlpha(sdlg.oopsbg, SDL_SRCALPHA, 190);
	SDL_SetColorKey(sdlg.oopsbg, SDL_SRCCOLORKEY | SDL_RLEACCEL,
			SDL_MapRGB(sdlg.oopsbg->format, 255, 0, 255));

	tempsur = IMG_Load("res/sprites.png");
	if (!tempsur) 
	{
		printf ( "res/sprites.png IMG_Load error\n" );
		return FALSE;
	}
	SDL_SetColorKey(tempsur, SDL_SRCCOLORKEY | SDL_RLEACCEL,
			SDL_MapRGB(tempsur->format, 255, 0, 255));	
	sdlg.sprites = SDL_DisplayFormat(tempsur);
	SDL_FreeSurface(tempsur);	
	//#endif

	return TRUE;
}
