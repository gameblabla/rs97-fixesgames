/* sdlgen.h: The internal shared definitions of the SDL OS/hardware layer.
 *
 * Copyright (C) 2001-2006 by Brian Raiter, under the GNU General Public
 * License. No warranty. See COPYING for details.
 */

#ifndef	_sdlgen_h_
#define	_sdlgen_h_

#include	<SDL.h>
#include	"../gen.h"
#include	"../oshw.h"

//DKS
#include  "SFont.h"
#include <SDL_image.h>

//DKS - won't be needing this anymore
/* Structure to hold the definition of a font.
 */
//typedef	struct fontinfo {
//    signed char		h;		/* height of each character */
//    signed char		w[256];		/* width of each character */
//    void	       *memory;		/* memory allocated for the font */
//    unsigned char      *bits[256];	/* pointers to each glyph */
//} fontinfo;


//DKS - won't be needing this anymore
/* Structure to hold a font's colors.
 */
//typedef	struct fontcolors { Uint32 c[3]; } fontcolors;
//
//#define	bkgndcolor(fc)	((fc).c[0])	/* the background color */
//#define	halfcolor(fc)	((fc).c[1])	/* the antialiasing color */
//#define	textcolor(fc)	((fc).c[2])	/* the main color of the glyphs */

/* Flags to the puttext function.
 */
#define	PT_CENTER	0x0001		/* center the text horizontally */
#define	PT_RIGHT	0x0002		/* right-align the text */
#define	PT_MULTILINE	0x0004		/* span lines & break at whitespace */
#define	PT_UPDATERECT	0x0008		/* return the unused area in rect */
#define	PT_CALCSIZE	0x0010		/* determine area needed for text */
#define	PT_DIM		0x0020		/* draw using the dim text color */
#define	PT_HILIGHT	0x0040		/* draw using the bold text color */

/*
 * Values global to this module. All the globals are placed in here,
 * in order to minimize pollution of the main module's namespace.
 */

//DKS - modified:
typedef	struct oshwglobals
{
	/*
	 * Shared variables.
	 */

	short		wtile;		/* width of one tile in pixels */
	short		htile;		/* height of one tile in pixels */
	short		cptile;		/* size of one tile in pixels */

	//DKS - removed
	//    fontcolors		textclr;	/* color triplet for normal text */
	//    fontcolors		dimtextclr;	/* color triplet for dim text */
	//    fontcolors		hilightclr;	/* color triplet for bold text */

	//DKS - I want needed more control over graphics. All tileworld functions 
	//	drew directly to this surface.  I will make this a generic SDL surface
	//	and the real screen will be a different surface.
	SDL_Surface	     *screen;		// the surface to which original tileworld
	// functions draw (tile drawing, mainly)

	SDL_Surface		*ScreenSurface;	//this is a new surface that now represents
	SDL_Surface		*realscreen;	//this is a new surface that now represents
	//the real framebuffer

	SDL_Surface		*playbg;	//this holds the pre-rendered background for the playscreen
	SDL_Surface		*menubg;	//this holds the pre-rendered background for the main menu
	SDL_Surface		*infobg;	//in-game background for displaying messages
	SDL_Surface		*sprites;	//this holds misc. sprites I added to the UI for GP2X	
	SDL_Surface		*oopsbg;	//holds background for "OOPS at level end
	SDL_Surface		*hintbg;	//holds background for hints
	//	SDL_Surface		*digits;	//this holds the LED digits

	//DKS added joystick handling
	SDL_Joystick    *joy;

	//DKS - replaced
	//fontinfo		font;		/* the font */

	//DKS new fonts:
	//    SFont_Font  *font1_dim;     // gray
	//    SFont_Font  *font1_hilite;  // white

	SFont_Font	*font_tiny;	// tiny 8 pixel font
	SFont_Font  *font_small; // standard small font
	SFont_Font	*font_big;	// larger font
	//	SFont_Font	*font_led_small;	//LEDs, small 	(21 pixels high)
	SFont_Font	*font_led_big;		//LEDs, big		(28 pixels high

	//DKS - added analog deadzone for joysticks (only GCW Zero for now)
	int deadzone; // will be initialized to some sensible default like 8000 inside the code.

	/*
	 * Shared functions.
	 */

	/* Process all pending events. If wait is TRUE and no events are
	 * currently pending, the function blocks until an event arrives.
	 */
	void (*eventupdatefunc)(int wait);

	/* A callback function, to be called every time a keyboard key is
	 * pressed or released. scancode is an SDL key symbol. down is
	 * TRUE if the key was pressed or FALSE if it was released.
	 */
	void (*keyeventcallbackfunc)(int scancode, int down);

	//DKS - added for GP2X
	/* A callback function, to be called every time a keyboard key is
	 * pressed or released. scancode is an SDL key symbol. down is
	 * TRUE if the key was pressed or FALSE if it was released.
	 */
	void (*joybuttoneventcallbackfunc)(int buttoncode, int down);

	/* A callback function, to be called when a mouse button is
	 * pressed or released. xpos and ypos give the mouse's location.
	 * button is the number of the mouse button. down is TRUE if the
	 * button was pressed or FALSE if it was released.
	 */
	void (*mouseeventcallbackfunc)(int xpos, int ypos, int button, int down);

	/* Given a pixel's coordinates, return an integer identifying the
	 * tile on the map view display under that pixel, or -1 if the
	 * pixel is not within the map view.
	 */
	int (*windowmapposfunc)(int x, int y);

	/* Return a pointer to an image of a cell with the two given
	 * tiles. If the top image is transparent, the composite image is
	 * created using the overlay buffer. (Thus the caller should be
	 * done using the image returned before calling this function
	 * again.) timerval should hold the time of the game, for
	 * rendering animated cell tiles, or -1 if the game has not
	 * started.
	 */
	SDL_Surface* (*getcellimagefunc)(SDL_Rect *rect,
			int top, int bot, int timerval);

	/* Return a pointer to a tile image for the given creature or
	 * animation sequence with the specified direction, sub-position,
	 * and animation frame.
	 */
	SDL_Surface* (*getcreatureimagefunc)(SDL_Rect *rect, int id, int dir,
			int moving, int frame);

	/* Display a line (or more) of text in the program's font. The
	 * text is clipped to area if necessary. If area is taller than
	 * the font, the topmost line is used. len specifies the number of
	 * characters to render; -1 can be used if text is NUL-terminated.
	 * flags is some combination of PT_* flags defined above. When the
	 * PT_CALCSIZE flag is set, no drawing is done; instead the w and
	 * h fields of area area changed to define the smallest rectangle
	 * that encloses the text that would have been rendered. (If
	 * PT_MULTILINE is also set, only the h field is changed.) If
	 * PT_UPDATERECT is set instead, then the h field is changed, so
	 * as to exclude the rectangle that was drawn in.
	 */
	void (*puttextfunc)(SDL_Rect *area, char const *text, int len, int flags);

	/* Determine the widths necessary to display the columns of the
	 * given table. area specifies an enclosing rectangle for the
	 * complete table. The return value is an array of rectangles, one
	 * for each column of the table. The rectangles y-coordinates and
	 * heights are taken from area, and the x-coordinates and widths
	 * are calculated so as to best render the columns of the table in
	 * the given space. The caller has the responsibility of freeing
	 * the returned array.
	 */
	SDL_Rect *(*measuretablefunc)(SDL_Rect const *area,
			tablespec const *table);

	/* Draw a single row of the given table. cols is an array of
	 * rectangles, one for each column. Each rectangle is altered by
	 * the function as per puttext's PT_UPDATERECT behavior. row
	 * points to an integer indicating the first table entry of the
	 * row to display; upon return, this value is updated to point to
	 * the first entry following the row. flags can be set to PT_DIM
	 * and/or PT_HIGHLIGHT; the values will be applied to every entry
	 * in the row.
	 */
	int (*drawtablerowfunc)(tablespec const *table, SDL_Rect *cols,
			int *row, int flags);

	//	//DKS new for mikmod sound
	//	void (*updatesoundfunc)(void);

} oshwglobals;

/* oshw's structure of globals.
 */
extern oshwglobals sdlg;

/* Some convenience macros for the above functions.
 */
#define eventupdate		(*sdlg.eventupdatefunc)
#define	keyeventcallback	(*sdlg.keyeventcallbackfunc)
//DKS new
#ifdef PLATFORM_GP2X
#define joybuttoneventcallback (*sdlg.joybuttoneventcallbackfunc)
#endif
#define	mouseeventcallback	(*sdlg.mouseeventcallbackfunc)
#define	windowmappos		(*sdlg.windowmapposfunc)
#define	puttext			(*sdlg.puttextfunc)
#define	measuretable		(*sdlg.measuretablefunc)
#define	drawtablerow		(*sdlg.drawtablerowfunc)
#define	createscroll		(*sdlg.createscrollfunc)
#define	scrollmove		(*sdlg.scrollmovefunc)
#define	getcreatureimage	(*sdlg.getcreatureimagefunc)
#define	getcellimage		(*sdlg.getcellimagefunc)
//#define updatesound			(*sdlg.updatesoundfunc)
/* The initialization functions for the various modules.
 */
extern int _sdltimerinitialize(int showhistogram);
extern int _sdlresourceinitialize(void);
extern int _sdltextinitialize(void);
extern int _sdltileinitialize(void);
extern int _sdlinputinitialize(void);
extern int _sdloutputinitialize(int fullscreen);
extern int _sdlsfxinitialize(int silence, int soundbufsize);


//DKS - added this to ensure tile images are loaded only once
extern int		images_loaded;

//DKS - added these so I could access the sdltext.c functions
extern int measuremltext(char const *text, int len, int maxwidth,
		SFont_Font *font, int spacing);
extern void drawtext(SDL_Surface *sur, SDL_Rect *rect, char const *text,
		int len, int flags, SFont_Font *font, int spacing);
extern void drawmultilinetext(SDL_Surface *sur, SDL_Rect *rect, char const *text,
		int len, int flags, SFont_Font *font, int spacing);


//DKS - new function that draws a LED digit at x,y on dst_sur
extern void drawdigit(int x, int y, int digit, SDL_Surface *dst_sur);

//DKS -This function overlays an black surface atop *sur, and the alpha channel 
//is set to level before it is blitted to it 
extern void dimsurface(SDL_Surface *sur, int level);

//DKS - new function waits milsecs but updates sound every 10ms too
extern void controlleddelay(int milsecs);

//DKS - start game music
extern void playgamesongs(void);

//DKS - new function called during main menu to play menu song		
extern void playmenusong(void);

//DKS next three functions defined in sdlsfx.c, used in main menu
extern void enablemusic(void);

extern void disablemusic(void);

extern int ismusicenabled(void);

//DKS set music volume 0 - 100 percent of total volume 
void setmusicvolume(int newvol);


#endif
