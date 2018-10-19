/* sdltext.c: Font-rendering functions for SDL.
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

//DKS - won't need this anymore
/* Accept a bitmap as an 8-bit SDL surface and from it extract the
 * glyphs of a font. (See the documentation included in the Tile World
 * distribution for specifics regarding the bitmap layout.)
 */
//static int makefontfromsurface(fontinfo *pf, SDL_Surface *surface)
//{
//    char		brk[267];
//    unsigned char      *p;
//    unsigned char      *dest;
//    Uint8		foregnd, bkgnd;
//    int			pitch, wsum;
//    int			count, ch;
//    int			x, y, x0, y0, w;
//
//    if (surface->format->BytesPerPixel != 1)
//	return FALSE;
//
//    if (SDL_MUSTLOCK(surface))
//	SDL_LockSurface(surface);
//
//    pitch = surface->pitch;
//    p = surface->pixels;
//    foregnd = p[0];
//    bkgnd = p[pitch];
//    for (y = 1, p += pitch ; y < surface->h && *p == bkgnd ; ++y, p += pitch) ;
//    pf->h = y - 1;
//
//    wsum = 0;
//    ch = 32;
//    memset(pf->w, 0, sizeof pf->w);
//    memset(brk, 0, sizeof brk);
//    for (y = 0 ; y + pf->h < surface->h && ch < 256 ; y += pf->h + 1) {
//	p = surface->pixels;
//	p += y * pitch;
//	x0 = 1;
//	for (x = 1 ; x < surface->w ; ++x) {
//	    if (p[x] == bkgnd)
//		continue;
//	    w = x - x0;
//	    x0 = x + 1;
//	    pf->w[ch] = w;
//	    wsum += w;
//	    ++ch;
//	    if (ch == 127)
//		ch = 144;
//	    else if (ch == 154)
//		ch = 160;
//	    else if (ch == 256)
//		break;
//	}
//	brk[ch] = 1;
//    }
//
//    count = ch;
//    if (!(pf->memory = calloc(wsum, pf->h)))
//	memerrexit();
//
//    x0 = 1;
//    y0 = 1;
//    dest = pf->memory;
//    for (ch = 0 ; ch < 256 ; ++ch) {
//	pf->bits[ch] = dest;
//	if (pf->w[ch] == 0)
//	    continue;
//	if (brk[ch]) {
//	    x0 = 1;
//	    y0 += pf->h + 1;
//	}
//	p = surface->pixels;
//	p += y0 * pitch + x0;
//	for (y = 0 ; y < pf->h ; ++y, p += pitch)
//	    for (x = 0 ; x < pf->w[ch] ; ++x, ++dest)
//		*dest = p[x] == bkgnd ? 0 : p[x] == foregnd ? 2 : 1;
//	x0 += pf->w[ch] + 1;
//    }
//
//    if (SDL_MUSTLOCK(surface))
//	SDL_UnlockSurface(surface);
//
//    return TRUE;
//}'

/* Given a text and a maximum horizontal space to occupy, return
 * the amount of vertial space needed to render the entire text with
 * word-wrapping.
 */
//DKS - modified
/* Given a text and a maximum horizontal space to occupy, return
 * the amount of vertial space needed to render the entire text with
 * word-wrapping.
 */
 //DKS: addded "font" parameter and spacing param
 // spacing is number of pixels to space lines by
int measuremltext(char const *text, int len, int maxwidth,
                    SFont_Font *font, int spacing)
{
    int	brk, w, h, n;

    //DKS
    char tmpstr[2];

    if (len < 0)
	len = strlen((char const*)text);
    h = 0;
    brk = 0;

    for (n = 0, w = 0 ; n < len ; ++n) {
    tmpstr[0] = text[n];
    tmpstr[1] = '\0';
    w += SFont_TextWidth(font, tmpstr);

	if (isspace(text[n])) {
	    brk = w;
	} else if (w > maxwidth) {
	    h += SFont_TextHeight(font) + spacing;
	    if (brk) {
		w -= brk;
		brk = 0;
	    } else {
        tmpstr[0] = text[n];
        tmpstr[1] = '\0';
        w = SFont_TextWidth(font, tmpstr);

		brk = 0;
	    }
	}
    }
    if (w)
    h += SFont_TextHeight(font) + spacing;
    return h;
}

//DKS - don't need any of these anymore
///*
// * Render a single line of pixels of the given text to a locked
// * surface at scanline. w specifies the total number of pixels to
// * render. (Any pixels remaining after the last glyph has been
// * rendered are set to the background color.) y specifies the vertical
// * coordinate of the line to render relative to the font glyphs. A
// * separate function is supplied for each possible surface depth.
// */
//
//static void *drawtextscanline8(Uint8 *scanline, int w, int y, Uint32 *clr,
//			       unsigned char const *text, int len)
//{
//    unsigned char const	       *glyph;
//    int				n, x;
//
//    for (n = 0 ; n < len ; ++n) {
//	glyph = sdlg.font.bits[text[n]];
//	glyph += y * sdlg.font.w[text[n]];
//	for (x = 0 ; w && x < sdlg.font.w[text[n]] ; ++x, --w)
//	    scanline[x] = (Uint8)clr[glyph[x]];
//	scanline += x;
//    }
//    while (w--)
//	*scanline++ = (Uint8)clr[0];
//    return scanline;
//}
//
//static void *drawtextscanline16(Uint16 *scanline, int w, int y, Uint32 *clr,
//				unsigned char const *text, int len)
//{
//    unsigned char const	       *glyph;
//    int				n, x;
//
//    for (n = 0 ; n < len ; ++n) {
//	glyph = sdlg.font.bits[text[n]];
//	glyph += y * sdlg.font.w[text[n]];
//	for (x = 0 ; w && x < sdlg.font.w[text[n]] ; ++x, --w)
//	    scanline[x] = (Uint16)clr[glyph[x]];
//	scanline += x;
//    }
//    while (w--)
//	*scanline++ = (Uint16)clr[0];
//    return scanline;
//}
//
//static void *drawtextscanline24(Uint8 *scanline, int w, int y, Uint32 *clr,
//				unsigned char const *text, int len)
//{
//    unsigned char const	       *glyph;
//    Uint32			c;
//    int				n, x;
//
//    for (n = 0 ; n < len ; ++n) {
//	glyph = sdlg.font.bits[text[n]];
//	glyph += y * sdlg.font.w[text[n]];
//	for (x = 0 ; w && x < sdlg.font.w[text[n]] ; ++x, --w) {
//	    c = clr[glyph[x]];
//#if SDL_BYTEORDER == SDL_BIG_ENDIAN
//	    *scanline++ = (Uint8)(c >> 16);
//	    *scanline++ = (Uint8)(c >> 8);
//	    *scanline++ = (Uint8)c;
//#else
//	    *scanline++ = (Uint8)c;
//	    *scanline++ = (Uint8)(c >> 8);
//	    *scanline++ = (Uint8)(c >> 16);
//#endif
//	}
//    }
//    c = clr[0];
//    while (w--) {
//#if SDL_BYTEORDER == SDL_BIG_ENDIAN
//	*scanline++ = (Uint8)(c >> 16);
//	*scanline++ = (Uint8)(c >> 8);
//	*scanline++ = (Uint8)c;
//#else
//	*scanline++ = (Uint8)c;
//	*scanline++ = (Uint8)(c >> 8);
//	*scanline++ = (Uint8)(c >> 16);
//#endif
//    }
//    return scanline;
//}
//
//static void *drawtextscanline32(Uint32 *scanline, int w, int y, Uint32 *clr,
//				unsigned char const *text, int len)
//{
//    unsigned char const	       *glyph;
//    int				n, x;
//
//    for (n = 0 ; n < len ; ++n) {
//	glyph = sdlg.font.bits[text[n]];
//	glyph += y * sdlg.font.w[text[n]];
//	for (x = 0 ; w && x < sdlg.font.w[text[n]] ; ++x, --w)
//	    scanline[x] = clr[glyph[x]];
//	scanline += x;
//    }
//    while (w--)
//	*scanline++ = clr[0];
//    return scanline;
//}

/*
 * The main font-rendering functions.
 */

//DKS - modified
/* Draw a single line of text to the screen at the position given by
 * rect. The bitflags in the final argument control the placement of
 * text within rect and what colors to use.
 */
void drawtext(SDL_Surface *sur, SDL_Rect *rect, char const *text,
		     int len, int flags, SFont_Font *font, int spacing)
{

    Uint32     *clr;
    void       *p;
    void       *q;
    int		l, r;
    int		pitch, bpp, n, w, y;

    //dks
    char str_to_display[500] = "";

    if (text == '\0') {
        return;
    }

    if (len < 0)
	len = text ? strlen((char const*)text) : 0;

    //d>ks
    if (len > 499) {
        errmsg(NULL, "string too large passed to drawtext: sdltext.c");
        return;
    } else if (spacing < 0) {
        errmsg(NULL, "cannot pass negative line spacings to drawtext: sdltext.c");
        return;
    }

    w = 0;

    w = SFont_TextWidth(font, text);

    if (flags & PT_CALCSIZE) {
    rect->h = SFont_TextHeight(font + spacing);
	rect->w = w;
	return;
    }

    strcpy(str_to_display, text);

    if (w >= rect->w) {
        //text is too wide, limit it to fit
        for (n = len; ((n >= 0) && (w > rect->w)); --n) {
            str_to_display[n] = '\0';
            w = SFont_TextWidth(font, str_to_display);
        }
        //w = rect->w;
        l = r = 0;
    } else if (flags & PT_RIGHT) {
	l = rect->w - w;
	r = 0;
    } else if (flags & PT_CENTER) {
	l = (rect->w - w) / 2;
	r = (rect->w - w) - l;
    } else {
	l = 0;
	r = rect->w - w;
    }

    SFont_Write(sur, font, rect->x + l, rect->y, str_to_display);

    if (flags & PT_UPDATERECT) {
	//rect->y += y;
	//rect->h -= y;
    rect->y += (SFont_TextHeight(font) + spacing);
    rect->h -= (SFont_TextHeight(font) + spacing);
    }

}


//DKS - modified
/* Draw one or more lines of text to the screen at the position given by
 * rect. The text is broken up on whitespace whenever possible.
 */
void drawmultilinetext(SDL_Surface *sur, SDL_Rect *rect, char const *text,
			      int len, int flags, SFont_Font *font, int spacing)

{
    if (spacing < 0)
        spacing = 0;

    SDL_Rect	area;
    int		index, brkw, brkn;
    int		w, n;

    if (flags & PT_CALCSIZE) {
	//rect->h = measuremltext(text, len, rect->w);
	rect->h = measuremltext(text, len, rect->w, font, spacing);
	return;
    }

    if (len < 0)
	len = strlen((char const*)text);

    area = *rect;
    brkw = brkn = 0;
    index = 0;

    //DKS new
    char tmpstr[2];

    for (n = 0, w = 0 ; n < len ; ++n) {
//	w += sdlg.font.w[text[n]];
    tmpstr[0] = text[n];
    tmpstr[1] = '\0';
    w += SFont_TextWidth(font, tmpstr);
	if (isspace(text[n])) {
	    brkn = n;
	    brkw = w;
	} else if (w > rect->w) {
	    if (brkw) {
        drawtext(sur, &area, text + index, brkn - index,
				 flags | PT_UPDATERECT, font, spacing);
		index = brkn + 1;
		w -= brkw;
	    } else {
		drawtext(sur, &area, text + index, n - index,
				 flags | PT_UPDATERECT, font, spacing);

		index = n;
		tmpstr[0] = text[n];
		tmpstr[1] = '\0';
		w = SFont_TextWidth(font, tmpstr);
	    }
	    brkw = 0;
	}
    }
    if (w)
	drawtext(sur, &area, text + index, len - index,
            flags | PT_UPDATERECT, font, spacing);

    if (flags & PT_UPDATERECT) {
	*rect = area;
    } else {
//DKS - suspect code, disabling
//	while (area.h)
//	    drawtext(&area, NULL, 0, PT_UPDATERECT);
//	    drawtext(sur, &area, NULL, 0, PT_UPDATERECT, font, spacing);

    }
}


/*
 * The exported functions.
 */

//DKS - modified
///* Render a string of text.
// */
static void _puttext(SDL_Rect *rect, char const *text, int len, int flags)
{
    //DKS - disabled for now
}

//DKS - modified
/* Lay out the columns of the given table so that the entire table
 * fits within area (horizontally; no attempt is made to make it fit
 * vertically). Return an array of rectangles, one per column. This
 * function is essentially the same algorithm used within printtable()
 * in tworld.c
 */
static SDL_Rect *_measuretable(SDL_Rect const *area, tablespec const *table)
{
	//DKS - we don't use this at all, dunno why I modified it early on but should work
	
    SDL_Rect		       *colsizes;
    unsigned char const	       *p;
    int				sep, mlindex, mlwidth, diff;
    int				i, j, n, i0, c, w, x;

    if (!(colsizes = malloc(table->cols * sizeof *colsizes)))
	memerrexit();
    for (i = 0 ; i < table->cols ; ++i) {
	colsizes[i].x = 0;
	colsizes[i].y = area->y;
	colsizes[i].w = 0;
	colsizes[i].h = area->h;
    }

    mlindex = -1;
    mlwidth = 0;
    n = 0;
    for (j = 0 ; j < table->rows ; ++j) {
	for (i = 0 ; i < table->cols ; ++n) {
	    c = table->items[n][0] - '0';
	    if (c == 1) {
		w = 0;
		p = (unsigned char const*)table->items[n];
		for (p += 2 ; *p ; ++p)
		    //w += sdlg.font.w[*p];
		    w += SFont_TextHeight(sdlg.font_small);
		if (table->items[n][1] == '!') {
		    if (w > mlwidth || mlindex != i)
			mlwidth = w;
		    mlindex = i;
		} else {
		    if (w > colsizes[i].w)
			colsizes[i].w = w;
		}
	    }
	    i += c;
	}
    }

    //sep = sdlg.font.w[' '] * table->sep;
    sep = SFont_TextHeight(sdlg.font_small) * table->sep;

    w = -sep;
    for (i = 0 ; i < table->cols ; ++i)
	w += colsizes[i].w + sep;
    diff = area->w - w;
    if (diff < 0 && table->collapse >= 0) {
	w = -diff;
	if (colsizes[table->collapse].w < w)
	    //w = colsizes[table->collapse].w - sdlg.font.w[' '];
	w = colsizes[table->collapse].w - SFont_TextHeight(sdlg.font_small);
	colsizes[table->collapse].w -= w;
	diff += w;
    }

    if (diff > 0) {
	n = 0;
	for (j = 0 ; j < table->rows && diff > 0 ; ++j) {
	    for (i = 0 ; i < table->cols ; ++n) {
		c = table->items[n][0] - '0';
		if (c > 1 && table->items[n][1] != '!') {
		    w = sep;
		    p = (unsigned char const*)table->items[n];
		    for (p += 2 ; *p ; ++p)
			//w += sdlg.font.w[*p];
			w += SFont_TextHeight(sdlg.font_small);
		    for (i0 = i ; i0 < i + c ; ++i0)
			w -= colsizes[i0].w + sep;
		    if (w > 0) {
			if (table->collapse >= i && table->collapse < i + c)
			    i0 = table->collapse;
			else if (mlindex >= i && mlindex < i + c)
			    i0 = mlindex;
			else
			    i0 = i + c - 1;
			if (w > diff)
			    w = diff;
			colsizes[i0].w += w;
			diff -= w;
			if (diff == 0)
			    break;
		    }
		}
		i += c;
	    }
	}
    }
    if (diff > 0 && mlindex >= 0 && colsizes[mlindex].w < mlwidth) {
	mlwidth -= colsizes[mlindex].w;
	w = mlwidth < diff ? mlwidth : diff;
	colsizes[mlindex].w += w;
	diff -= w;
    }

    x = 0;
    for (i = 0 ; i < table->cols && x < area->w ; ++i) {
	colsizes[i].x = area->x + x;
	x += colsizes[i].w + sep;
	if (x >= area->w)
	    colsizes[i].w = area->x + area->w - colsizes[i].x;
    }
    for ( ; i < table->cols ; ++i) {
	colsizes[i].x = area->x + area->w;
	colsizes[i].w = 0;
    }

    return colsizes;
}

//DKS - modified
/* Render a single row of a table to the screen, using cols to locate
 * the entries in the individual columns.
 */
static int _drawtablerow(tablespec const *table, SDL_Rect *cols,
			 int *row, int flags)
{
//DKS - disabled for now
return TRUE;
}


/* Free the resources associated with a font.
 */
//DKS - modified
void freefont(void)
{
    if (sdlg.font_tiny->Surface) {
        
        SFont_FreeFont(sdlg.font_tiny);
    }
    if (sdlg.font_small->Surface) {
        
        SFont_FreeFont(sdlg.font_small);
    }
    if (sdlg.font_big->Surface) {
        
        SFont_FreeFont(sdlg.font_big);
    }
    if (sdlg.font_led_big->Surface) {
        
        SFont_FreeFont(sdlg.font_led_big);
    }
}

//DKS - won't need this anymore
///* Load the font contained in the given bitmap file. Error messages
// * will be displayed if complain is TRUE. The return value is TRUE if
// * the font was successfully retrieved.
// */
//int loadfontfromfile(char const *filename, int complain)
//{
//    SDL_Surface	       *bmp;
//    fontinfo		font;
//
//    bmp = SDL_LoadBMP(filename);
//    if (!bmp) {
//	if (complain)
//	    errmsg(filename, "can't load font bitmap: %s", SDL_GetError());
//	return FALSE;
//    }
//    if (!makefontfromsurface(&font, bmp)) {
//	if (complain)
//	    errmsg(filename, "invalid font file");
//	return FALSE;
//    }
//    SDL_FreeSurface(bmp);
//    freefont();
//    sdlg.font = font;
//    return TRUE;
//}

//DKS - modified
/* Initialize the module.
 */
int _sdltextinitialize(void)
{
    sdlg.puttextfunc = _puttext;
    sdlg.measuretablefunc = _measuretable;
    sdlg.drawtablerowfunc = _drawtablerow;
    return TRUE;
}
