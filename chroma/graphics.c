/*  
    graphics.c

    Copyright (C) 2010 Amf

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version. 

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <libintl.h>
#include <locale.h>

#include "chroma.h"
#include "level.h"
#include "colours.h"
#include "graphics.h"
#include "util.h"
#include "sdlfont.h"
#include "menu.h"
#include "xmlparser.h"

struct graphics* pdisplaygraphics = NULL;

extern char *piece_name[];
extern char options_graphics[];
extern struct colours* pdisplaycolours;
extern struct level* plevelcurrent;

extern int options_sdl_size_x;
extern int options_sdl_size_y;
extern int options_graphic_level;
extern int options_debug;
#ifdef XOR_COMPATIBILITY
extern int options_xor_display;
#endif
extern int screen_width;
extern int screen_height;
extern int screen_flags;

extern int font_height;
extern int font_height_game;
extern int font_size_menu;
extern int font_size_game;

void graphics_createfromfont(struct graphics *pgraphics);
SDL_Surface* graphics_scaleimage(SDL_Surface*, int, int);

struct graphics* graphics_load(char *filename, int partial);

#define SIZE_HUGE 1000000

void graphics_init()
{
    char filename[FILENAME_MAX];
    char directory[FILENAME_MAX];

    if(pdisplaygraphics != NULL)
	graphics_delete(pdisplaygraphics);

    pdisplaygraphics = graphics_load(options_graphics, 0);

    if(pdisplaygraphics == NULL)
    {
        /* Revert to default */
	getfilename("graphics", directory, 0, LOCATION_SYSTEM);
        sprintf(filename, "%s%s%s", directory, "/", GRAPHICS_DEFAULT);
        pdisplaygraphics = graphics_load(filename, 0);

        /* If we can't even load the default, use a curses based scheme */
        if(pdisplaygraphics == NULL)
            pdisplaygraphics = graphics_load(NULL, 0);
    }
}

void graphics_delete(struct graphics* pgraphics)
{
    int i, j;
    struct graphicssize* psize;
    struct graphicssize* psizetmp;
    struct shadow* pshadow;
    struct shadow* pshadowtmp;

    for(i = 0; i < PIECE_MAX; i ++)
    {
	for(j = 0; j < IMAGE_MAX; j ++)
	{
	    if(pgraphics->image[i][j] != NULL && !(pgraphics->image_flags[i] & GRAPHICS_CLONE))
	        SDL_FreeSurface(pgraphics->image[i][j]);
	}
    }

    if(pgraphics->title != NULL)
	free(pgraphics->title);

    psize = pgraphics->sizes;
    while(psize != NULL)
    {
	psizetmp = psize;
	psize = psize->next;
	free(psizetmp);
    }

    pshadow = pgraphics->shadows;
    while(pshadow != NULL)
    {
	pshadowtmp = pshadow;
	pshadow = pshadow->next;
	free(pshadowtmp);
    }

    free(pgraphics);
}

void graphics_createfromfont(struct graphics* pgraphics)
{
    int piece;
    struct SDL_Surface* psurface;
    struct SDL_Surface*  psurfacetmp;
    SDL_Rect drect;
    char buffer[4];
    int fg, bg, tg;
    int size_x, size_y;
    int reverse;
    int bold;

    /* Slightly smaller to ensure j's and g's fit within piece */
    font_set_size(pgraphics->size_y * 0.8);

    pgraphics->small_size_x = font_height_game;
    pgraphics->small_size_y = font_height_game;

    if(pdisplaycolours == NULL)
        return;

    /* Create pieces */
    for(piece = 0; piece < PIECE_MAX; piece ++)
    {
	size_x = pgraphics->size_x;
	size_y = pgraphics->size_y;

	if(piece == PIECE_PLAYER_ONE || piece == PIECE_PLAYER_TWO)
	    size_x = pgraphics->size_x * 2;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        psurface = SDL_CreateRGBSurface(screen_flags, size_x, size_y, 32, 0xff000000, 0xff0000, 0xff00, 0xff);
#else
        psurface = SDL_CreateRGBSurface(screen_flags, size_x, size_y, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
#endif
        SDL_FillRect(psurface, NULL, SDL_MapRGBA(psurface->format, 0, 0, 0, 255) );

	fg = pdisplaycolours->foreground[piece];
	bg = pdisplaycolours->background[piece];
        reverse = pdisplaycolours->reverse[piece];
        bold = pdisplaycolours->bold[piece];

	/* Player pieces are special. The swapped in piece always takes player
	   one's colours, which we force here */
	if(piece == PIECE_PLAYER_TWO)
	{
	    fg = pdisplaycolours->foreground[PIECE_PLAYER_ONE];
    	    bg = pdisplaycolours->background[PIECE_PLAYER_ONE];
            reverse = pdisplaycolours->reverse[PIECE_PLAYER_ONE];
            bold = pdisplaycolours->bold[PIECE_PLAYER_ONE];
	}

	if(reverse)
	{
	    tg = fg; fg = bg; bg = tg; 
	}
	if(fg == -1)
	    fg = 0;
	if(bg == -1 && (piece == PIECE_SPACE || isexplosion(piece)))
	    bg = 0;

        if(bg != -1)
        {
            drect.x = 0;
            drect.y = 0;
            drect.w = pgraphics->size_x;
            drect.h = pgraphics->size_y;
            SDL_FillRect(psurface, &drect, SDL_MapRGB(psurface->format, bg & 1 ? 255 : 0,  bg & 2 ? 255 : 0, bg & 4 ? 255 : 0));
        }
	sprintf(buffer, "%c", pdisplaycolours->character[piece]);
        psurfacetmp = font_render(buffer, fg);
        drect.x = (pgraphics->size_x - psurfacetmp->w) / 2;
        drect.y = (pgraphics->size_y - psurfacetmp->h) / 2;
        SDL_BlitSurface(psurfacetmp, NULL, psurface, &drect);
        SDL_FreeSurface(psurfacetmp);

	/* Player pieces get a second image for when they're swapped out.
	   The swapped out piece always takes player two's colours. */
	if(piece == PIECE_PLAYER_ONE || piece == PIECE_PLAYER_TWO)
	{
	    fg = pdisplaycolours->foreground[PIECE_PLAYER_TWO];
	    bg = pdisplaycolours->background[PIECE_PLAYER_TWO];

            if(pdisplaycolours->reverse[PIECE_PLAYER_TWO])
            {   
                tg = fg; fg = bg; bg = tg;
            }
            if(fg == -1)
                fg = 0;

            if(bg != -1)
            {
                drect.x = pgraphics->size_x;
                drect.y = 0;
                drect.w = pgraphics->size_x;
                drect.h = pgraphics->size_y;
                SDL_FillRect(psurface, &drect, SDL_MapRGB(psurface->format, bg & 1 ? 255 : 0,  bg & 2 ? 255 : 0, bg & 4 ? 255 : 0));
            }
    	    sprintf(buffer, "%c", pdisplaycolours->character[piece]);
            psurfacetmp = font_render(buffer, fg);
            drect.x = pgraphics->size_x + ((pgraphics->size_x - psurfacetmp->w) / 2);
            drect.y = (pgraphics->size_y - psurfacetmp->h) / 2;
            SDL_BlitSurface(psurfacetmp, NULL, psurface, &drect);
            SDL_FreeSurface(psurfacetmp);

	}

	if(bg == -1 || piece == PIECE_PLAYER_ONE || piece == PIECE_PLAYER_TWO)
	    pgraphics->image[piece][IMAGE_PIECE] = SDL_DisplayFormatAlpha(psurface);
	else
	    pgraphics->image[piece][IMAGE_PIECE] = SDL_DisplayFormat(psurface);
	SDL_FreeSurface(psurface);

    }

    /* Create small pieces */
    font_set_size(font_size_game);

    for(piece = 0; piece < PIECE_MAX; piece ++)
    {
        /* but only for the pieces we need */
        if(piece != PIECE_PLAYER_ONE && piece != PIECE_PLAYER_TWO &&
                piece != PIECE_STAR && piece != PIECE_DOOR)
            continue;

	size_x = pgraphics->small_size_x;
	size_y = pgraphics->small_size_y;

	if(piece == PIECE_PLAYER_ONE || piece == PIECE_PLAYER_TWO)
	    size_x = pgraphics->size_x * 2;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        psurface = SDL_CreateRGBSurface(screen_flags, size_x, size_y, 32, 0xff000000, 0xff0000, 0xff00, 0xff);
#else
        psurface = SDL_CreateRGBSurface(screen_flags, size_x, size_y, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
#endif
        SDL_FillRect(psurface, NULL, SDL_MapRGBA(psurface->format, 0, 0, 0, 255) );

	fg = pdisplaycolours->foreground[piece];
	bg = pdisplaycolours->background[piece];
        reverse = pdisplaycolours->reverse[piece];
        bold = pdisplaycolours->bold[piece];

	/* Player pieces are special. The swapped in piece always takes player
	   one's colours, which we force here */
	if(piece == PIECE_PLAYER_TWO)
	{
	    fg = pdisplaycolours->foreground[PIECE_PLAYER_ONE];
    	    bg = pdisplaycolours->background[PIECE_PLAYER_ONE];
            reverse = pdisplaycolours->reverse[PIECE_PLAYER_ONE];
            bold = pdisplaycolours->bold[PIECE_PLAYER_ONE];
	}

	if(reverse)
	{
	    tg = fg; fg = bg; bg = tg; 
	}
	if(fg == -1)
	    fg = 0;
	if(bg == -1)
	    bg = 0;

        drect.x = 0;
        drect.y = 0;
        drect.w = font_height_game;
        drect.h = font_height_game;
        SDL_FillRect(psurface, &drect, SDL_MapRGB(psurface->format, bg & 1 ? 255 : 0,  bg & 2 ? 255 : 0, bg & 4 ? 255 : 0));

	sprintf(buffer, "%c", pdisplaycolours->character[piece]);
        psurfacetmp = font_render(buffer, fg);
        drect.x = (font_height_game - psurfacetmp->w) / 2;
        drect.y = (psurface->h - psurfacetmp->h) / 2;
        SDL_BlitSurface(psurfacetmp, NULL, psurface, &drect);
        SDL_FreeSurface(psurfacetmp);

	/* Player pieces get a second image for when they're swapped out.
	   The swapped out piece always takes player two's colours. */
	if(piece == PIECE_PLAYER_ONE || piece == PIECE_PLAYER_TWO)
	{
	    fg = pdisplaycolours->foreground[PIECE_PLAYER_TWO];
	    bg = pdisplaycolours->background[PIECE_PLAYER_TWO];

            if(pdisplaycolours->reverse[PIECE_PLAYER_TWO])
            {   
                tg = fg; fg = bg; bg = tg;
            }
            if(fg == -1)
                fg = 0;

            if(bg != -1)
            {
                drect.x = font_height_game;
                drect.y = 0;
                drect.w = font_height_game;
                drect.h = font_height_game;
                SDL_FillRect(psurface, &drect, SDL_MapRGB(psurface->format, bg & 1 ? 255 : 0,  bg & 2 ? 255 : 0, bg & 4 ? 255 : 0));
            }
    	    sprintf(buffer, "%c", pdisplaycolours->character[piece]);
            psurfacetmp = font_render(buffer, fg);
            drect.x = font_height_game + ((font_height_game - psurfacetmp->w) / 2);
            drect.y = (psurface->h - psurfacetmp->h) / 2;
            SDL_BlitSurface(psurfacetmp, NULL, psurface, &drect);
            SDL_FreeSurface(psurfacetmp);

	}

	if(bg == -1 || piece == PIECE_PLAYER_ONE || piece == PIECE_PLAYER_TWO)
	    pgraphics->image[piece][IMAGE_SMALL] = SDL_DisplayFormatAlpha(psurface);
	else
	    pgraphics->image[piece][IMAGE_SMALL] = SDL_DisplayFormat(psurface);
	SDL_FreeSurface(psurface);

    }
}

struct menu* graphics_menu()
{
    DIR *pdir;
    struct dirent *dentry;
    struct menu* pmenu;
    struct menuentry* pentry;
    char directory[FILENAME_MAX];
    char filename[FILENAME_MAX];
    struct graphics* pgraphics;
    int location;

    pmenu = menu_new(gettext("Graphics Schemes"));

    menuentry_new(pmenu, gettext("Quit and return to previous menu"), 'Q', 0);
    menuentry_new(pmenu, "", 0, MENU_SPACE);

    menuentry_new(pmenu, gettext("Current graphics scheme:"), 0, MENU_NOTE);

    if(pdisplaygraphics == NULL)
        menuentry_new(pmenu, gettext("** NONE **"), 0, MENU_NOTE | MENU_RIGHT);
    else if(pdisplaygraphics->title == NULL)
        menuentry_new(pmenu, gettext("[untitled graphics]"), 0, MENU_NOTE | MENU_RIGHT);
    else if(pdisplaygraphics->flags & GRAPHICS_TRANSLATE)
        menuentry_new(pmenu, gettext(pdisplaygraphics->title), 0, MENU_NOTE | MENU_RIGHT);
    else
        menuentry_new(pmenu, pdisplaygraphics->title, 0, MENU_NOTE | MENU_RIGHT);

    /* Global, then user */
    for(location = 1; location >= 0; location --)
    {
        getfilename("graphics", directory, 0, location);

        pdir = opendir(directory);

        if(pdir == NULL)
    	    continue;

        while((dentry = readdir(pdir)) != NULL)
        {   
            if(strcmp(dentry->d_name, ".") == 0)
                continue;
            if(strcmp(dentry->d_name, "..") == 0)
                continue;

            sprintf(filename, "%s%s%s", directory, "/", dentry->d_name);

            if(isfile(filename) && strlen(filename) > 7 && strcmp(filename + strlen(filename) - 7, ".chroma") == 0)
	    {
	        pgraphics = graphics_load(filename, 1);
	        if(pgraphics != NULL)
	        {
                    if(pgraphics->title == NULL)
                        pentry = menuentry_newwithvalue(pmenu, gettext("[untitled graphics]"), 0, MENU_SORT, filename);
                    else if(pgraphics->flags & GRAPHICS_TRANSLATE)
                        pentry = menuentry_newwithvalue(pmenu, gettext(pgraphics->title), 0, MENU_SORT, filename);
                    else
                        pentry = menuentry_newwithvalue(pmenu, pgraphics->title, 0, MENU_SORT, filename);

		    graphics_delete(pgraphics);

		    if(strcmp(options_graphics, filename) == 0)
		        pmenu->entry_selected = pentry;
	        }
	    }
        }

        closedir(pdir);

        menu_unsort(pmenu);

        if(location == 1)
    	    menuentry_new(pmenu, "", 0, MENU_SPACE);
    }

    menu_assignletters(pmenu);

    return pmenu;
}

SDL_Surface *graphics_loadimage(char *filename)
{
    char fullfilename[FILENAME_MAX];
    char directory[FILENAME_MAX];

    getfilename("graphics", directory, 0, LOCATION_SYSTEM);
    sprintf(fullfilename, "%s%s%s", directory, "/", filename);

    return IMG_Load(fullfilename);
}

struct menu* graphics_error(struct menu *pmenu, char *filename, struct parser *pparser, char *error)
{
    char buffer[256];

    if(pmenu == NULL)
    {
	pmenu = menu_new(gettext("Graphics Errors"));

	menuentry_new(pmenu, gettext("Return to previous menu"), 'Q', 0);
	menuentry_new(pmenu, "", 0, MENU_SPACE);
    }

    if(pparser != NULL)
        sprintf(buffer, "%d: %s", pparser->line, error);
    else
	sprintf(buffer, "%s", error);

    fprintf(stderr, gettext("In file '%s':\n"), filename);
    fprintf(stderr, "%s\n", buffer);

    menuentry_new(pmenu, buffer, 0, MENU_SORT);

    return pmenu;
}

int graphics_translatecolour(char *text, int *red, int *green, int *blue, int *alpha)
{
    char buffer[8];

    /* #rrggbb */
    if(strlen(text) == 7 && text[0] == '#')
    {
	strcpy(buffer, "0xff");
	buffer[2] = text[1]; buffer[3] = text[2]; *red = strtol(buffer, NULL, 0);
	buffer[2] = text[3]; buffer[3] = text[4]; *green = strtol(buffer, NULL, 0);
	buffer[2] = text[5]; buffer[3] = text[6]; *blue = strtol(buffer, NULL, 0);
        *alpha = 255;
	return 1;
    }

    /* #rrggbbaa */
    if(strlen(text) == 9 && text[0] == '#')
    {
	strcpy(buffer, "0xff");
	buffer[2] = text[1]; buffer[3] = text[2]; *red = strtol(buffer, NULL, 0);
	buffer[2] = text[3]; buffer[3] = text[4]; *green = strtol(buffer, NULL, 0);
	buffer[2] = text[5]; buffer[3] = text[6]; *blue = strtol(buffer, NULL, 0);
	buffer[2] = text[7]; buffer[3] = text[8]; *alpha = strtol(buffer, NULL, 0);
	return 1;
    }

    /* #rgb */
    if(strlen(text) == 4 && text[0] == '#')
    {
	strcpy(buffer, "0xff");
	buffer[2] = text[1]; buffer[3] = text[1]; *red = strtol(buffer, NULL, 0);
	buffer[2] = text[2]; buffer[3] = text[2]; *green = strtol(buffer, NULL, 0);
	buffer[2] = text[3]; buffer[3] = text[3]; *blue = strtol(buffer, NULL, 0);
        *alpha = 255;
	return 1;
    }

    /* #rgba */
    if(strlen(text) == 5 && text[0] == '#')
    {
	strcpy(buffer, "0xff");
	buffer[2] = text[1]; buffer[3] = text[1]; *red = strtol(buffer, NULL, 0);
	buffer[2] = text[2]; buffer[3] = text[2]; *green = strtol(buffer, NULL, 0);
	buffer[2] = text[3]; buffer[3] = text[3]; *blue = strtol(buffer, NULL, 0);
	buffer[2] = text[4]; buffer[3] = text[4]; *alpha = strtol(buffer, NULL, 0);
	return 1;
    }

    /* Couldn't make sense of it */
    return 0;
}

void graphics_addimage(struct graphics* pgraphics, int piece, int type, SDL_Surface* psurface)
{
    SDL_Surface *psurfacetmp;
    SDL_Rect drect;

    if(psurface == NULL)
	return;

    if(piece >= 0 && piece < PIECE_MAX)
    {
	/* If this is the first image, create a new piece image */
	if(pgraphics->image[piece][type] == NULL)
	{
            if(psurface->flags & SDL_SRCALPHA)
                pgraphics->image[piece][type] = SDL_DisplayFormatAlpha(psurface);
            else
                pgraphics->image[piece][type] = SDL_DisplayFormat(psurface);
	}
	else
	{
	    /* Otherwise, create a larger piece image */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            psurfacetmp = SDL_CreateRGBSurface(screen_flags, pgraphics->image[piece][type]->w + psurface->w, pgraphics->image[piece][type]->h, 32, 0xff000000, 0xff0000, 0xff00, 0xff);
#else
            psurfacetmp = SDL_CreateRGBSurface(screen_flags, pgraphics->image[piece][type]->w + psurface->w, pgraphics->image[piece][type]->h, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
#endif
            if(psurfacetmp == NULL)
                fatal("Out of memory in graphics_addimage()");

	    SDL_SetAlpha(pgraphics->image[piece][type], 0, 0);

	    /* Copy the old piece image into it */
            if(pgraphics->image_flags[piece] & GRAPHICS_KEY)
		/* Copy as is; we'll deal with the keying later */
                SDL_SetAlpha(psurfacetmp, 0, 0);
	    else
                SDL_SetAlpha(psurfacetmp, SDL_SRCALPHA, 255);

	    SDL_BlitSurface(pgraphics->image[piece][type], NULL, psurfacetmp, NULL);

	    /* Copy the new piece image into it */
	    drect.x = pgraphics->image[piece][type]->w;
	    drect.y = 0;
	    drect.w = psurface->w;
	    drect.h = pgraphics->size_y;

            SDL_SetAlpha(psurface, 0, 0);
	    SDL_BlitSurface(psurface, NULL, psurfacetmp, &drect);

	    /* Free the old piece image */
            SDL_FreeSurface(pgraphics->image[piece][type]);

            if(psurfacetmp->flags & SDL_SRCALPHA)
                pgraphics->image[piece][type] = SDL_DisplayFormatAlpha(psurfacetmp);
            else
                pgraphics->image[piece][type]  = SDL_DisplayFormat(psurfacetmp);

	    SDL_FreeSurface(psurfacetmp);

	}
    }
}

int graphics_evaluate(struct graphics *pgraphics, char *text)
{
    int v, e;

    if(strcmp(text, "true") == 0)
	return 1;
    if(strcmp(text, "false") == 0)
	return 0;

    if(strlen(text) > 2)
    {
	if(text[1] == '=')
	{
	    switch(text[0])
	    {
		case 'x':
		    v = pgraphics->size_x;
		    break;
		case 'y':
		    v = pgraphics->size_y;
		    break;
		case 's':
		    v = pgraphics->small_size_x;
		    break;
		case 't':
		    v = pgraphics->small_size_y;
		    break;
		case 'l':
		    v = pgraphics->level;
		    break;
		default:
		    return 0;
	    }
	    e = atoi(text + 2);
	    if(v == e)
		return 1;
	    else
		return 0;
	}
    }

    return 0;
}

int graphics_evaluatesize(char *text, int base)
{
    if(strlen(text) > 0 && text[strlen(text) - 1] == '%')
	return (int)(atof(text) * base / 100);
    else
	return atoi(text);
}

struct graphics* graphics_load(char *filename, int partial)
{
    struct graphics* pgraphics;
    struct graphicssize* psize;
    struct graphicssize* psizetmp;
    struct shadow* pshadow;
    struct shadow* pshadowtmp;
    struct parser* pparser;
    struct menu* pmenu;
    int i, j, k;
    int state;
    int piece;
    int clone;

    int width, height, offset_x, offset_y;
    int size_x, size_y;

    char basepath[FILENAME_MAX];
    char path[FILENAME_MAX];
    char file[FILENAME_MAX];
    char buffer[256];
    char c;

    SDL_Surface *psurface;
    SDL_Surface *psurfacetmp;
    SDL_Rect drect;
    int imagetype;
    int red, green, blue, alpha;

    int tg, fg, bg;
    SDL_Rect rect;
    int x, y, z, w, h;
    int failed;
    int flags;

    int shadow_flags[9] =
    {
	SHADOW_TOP_LEFT,	SHADOW_TOP,		SHADOW_TOP_RIGHT,
	SHADOW_LEFT,		SHADOW_MIDDLE,		SHADOW_RIGHT,
	SHADOW_BOTTOM_LEFT,	SHADOW_BOTTOM,		SHADOW_BOTTOM_RIGHT
    };

    if(filename != NULL && !isfile(filename))
        return NULL;

    pgraphics = (struct graphics*)malloc(sizeof(struct graphics));
    if(pgraphics == NULL)
	fatal(gettext("Out of memory in graphics_load()"));

    /* Initialise graphics structure */
    pgraphics->title = NULL;
    pgraphics->sizes = NULL;
    pgraphics->shadows = NULL;

    pgraphics->size_x = 0;
    pgraphics->size_y = 0;
    pgraphics->small_size_x = 0;
    pgraphics->small_size_y = 0;
    pgraphics->levels = 0;
    pgraphics->flags = 0;

    pgraphics->level = 0;

    for(i = 0; i < PIECE_MAX; i ++)
    {
	for(j = 0; j < IMAGE_MAX; j ++)
	    pgraphics->image[i][j] = NULL;

	pgraphics->image_flags[i] = 0;
	pgraphics->shadow_flags[i] = 0;
	pgraphics->shadow_z[i] = 0;

	for(j = 0; j < 10; j ++)
	{
	    pgraphics->shadow_offset_x[i][j] = 0;
	    pgraphics->shadow_offset_y[i][j] = 0;
	    pgraphics->shadow_start_x[i][j] = 0;
	    pgraphics->shadow_start_y[i][j] = 0;
	    pgraphics->shadow_width[i][j] = 0;
	    pgraphics->shadow_height[i][j] = 0;
	}
    }
    for(i = 0; i < 3; i ++)
    {
	pgraphics->background[i] = 0;
    }

    /* Emergency default graphics scheme */
    if(filename == NULL)
    {
        graphics_createfromfont(pgraphics);
        return pgraphics;
    }

    /* Determine base directory */
    strcpy(basepath, filename);
    for(i = strlen(filename); i >= 0; i --)
    {
	if(basepath[i] == '/')
	{
	    basepath[i] = 0;
	    break;
	}
    }
    strcpy(path, "");

    piece = PIECE_UNKNOWN;
    psurface = NULL;
    pmenu = NULL;
    failed = 0;
    psize = NULL;
    imagetype = 0;
    w = 0; h = 0; z = 0;
    flags = 0;
    clone = PIECE_UNKNOWN;
    x = 0; y = 0;

    /* Parse XML file */
    /*
       <chroma type="graphics">
           <head>
               <title>title</title>
               <sizes>
                   <size x="x" y="y" />
               </sizes>
               </head>
	   <shadows>
	       <shadow x="x" y="y" />
	   </shadows>
           <background colour="#colour" />
	   <if condition="condition">
	   </if>
           <pieces path="path" levels="1">
               <piece name="piece" bevel="[4|16]" mover="yes" animate="yes" level="yes" random="yes" tile="yes">
                   <image colour="#colour" [width="x" height="y"] />
                   <image file="file" key="#colour" />
		   <image type="shadow" file="file" x="x%" y="y" />
		   <image type="small" file="file" />
               </piece>
           </pieces>
       </chroma>
       */
       
    pparser = parser_new(filename);
    if(pparser == NULL)
    {
        graphics_delete(pgraphics);
        return NULL;
    }

    enum{
	GRAPHICSPARSER_BAD,		/* End of bad file */
	GRAPHICSPARSER_END,		/* End of good file */
	GRAPHICSPARSER_OUTSIDE,		/* Outside of <chroma> */
	GRAPHICSPARSER_CHROMA,		/* Inside <chroma> */
	GRAPHICSPARSER_PIECES,		/* Inside <pieces> */
	GRAPHICSPARSER_PIECE,		/* Inside <piece> */
	GRAPHICSPARSER_SIZES,		/* Inside <sizes> */
	GRAPHICSPARSER_SHADOWS,		/* Inside <shadows> */
	GRAPHICSPARSER_IF		/* Inside failed <if> */
    };

    /* Another state machine! */
    state = GRAPHICSPARSER_OUTSIDE;

    while(state != GRAPHICSPARSER_BAD && state != GRAPHICSPARSER_END)
    {
	switch(parser_parse(pparser))
	{
	    case PARSER_END:
		if(state == GRAPHICSPARSER_OUTSIDE)
		    state = GRAPHICSPARSER_BAD;
		else
		    state = GRAPHICSPARSER_END;
		break;

	    case PARSER_ELEMENT_START:
		switch(state)
		{
		    case GRAPHICSPARSER_CHROMA:
			if(parser_match(pparser, 0, "pieces"))
			{
			    state = GRAPHICSPARSER_PIECES;
			    strcpy(path, "");
			}
			if(parser_match(pparser, 0, "sizes"))
			{
			    state = GRAPHICSPARSER_SIZES;
			}
			if(parser_match(pparser, 0, "shadows"))
			{
			    state = GRAPHICSPARSER_SHADOWS;
			}
			break;

		    case GRAPHICSPARSER_PIECES:
			if(parser_match(pparser, 0, "piece"))
			{
			    piece = PIECE_UNKNOWN;
			    clone = PIECE_UNKNOWN;
			}
			break;

		    case GRAPHICSPARSER_PIECE:
			if(parser_match(pparser, 0, "image"))
			{
			    psurface = NULL;
			    imagetype = IMAGE_PIECE;
			    x = 0;
			    y = 0;
			    z = 0;
			    w = pgraphics->size_x;
			    h = pgraphics->size_y;
			}
			break;

		    case GRAPHICSPARSER_SIZES:
			if(parser_match(pparser, 0, "size"))
			{
			    x = -1; y = -1;
                            flags = SIZE_PIECES | SIZE_SMALL;

			}
			break;

		    case GRAPHICSPARSER_SHADOWS:
			if(parser_match(pparser, 0, "shadow"))
			{
			    x = 0; y = 0;
			}
			break;

		    default:
			break;
		}
		break;

	    case PARSER_ELEMENT_END:

                /* </if> */
		if(parser_match(pparser, 0, "if") && failed > 0)
		{
		    failed --;
		    if(failed == 0)
			state -= GRAPHICSPARSER_IF;
		}

		switch(state)
		{
		    case GRAPHICSPARSER_CHROMA:
		        /* If we're only partially loading the file, end parsing at </head> */
                        /* </head> */
        		if(parser_match(pparser, 0, "head"))
			{
			    if(partial)
				state = GRAPHICSPARSER_END;
                            /* If we haven't found a size... */
			    if(!(pgraphics->flags & GRAPHICS_CURSES) && pgraphics->size_x == 0 && pgraphics->size_y == 0)

			    {
				pmenu = graphics_error(pmenu, filename, NULL, gettext("No size specified"));
				/* Make it appear in the menu, but not be loadable */
				if(partial)
                                    state = GRAPHICSPARSER_END;
                                else
                                    state = GRAPHICSPARSER_BAD;
			    }
			}

                        /* </curses> */
		        if(parser_match(pparser, 0, "curses"))
		            pgraphics->flags |= GRAPHICS_CURSES;

			break;

		    case GRAPHICSPARSER_PIECES:
		        if(parser_match(pparser, 0, "pieces"))
		            state = GRAPHICSPARSER_CHROMA;
			break;

		    case GRAPHICSPARSER_PIECE:
		        if(parser_match(pparser, 0, "piece"))
			{
			    if(piece != PIECE_UNKNOWN && pgraphics->image[piece][IMAGE_PIECE] != NULL)
			    {
				/* Key transparency */
				if(pgraphics->image_flags[piece] & GRAPHICS_KEY)
				{
				    SDL_SetAlpha(pgraphics->image[piece][IMAGE_PIECE], SDL_SRCALPHA, 255);
				    SDL_SetColorKey(pgraphics->image[piece][IMAGE_PIECE], SDL_SRCCOLORKEY, SDL_MapRGB(pgraphics->image[piece][IMAGE_PIECE]->format, red, green, blue));

                                    if(pgraphics->image[piece][IMAGE_SMALL] != NULL)
                                    {
				        SDL_SetAlpha(pgraphics->image[piece][IMAGE_SMALL], SDL_SRCALPHA, 255);
				        SDL_SetColorKey(pgraphics->image[piece][IMAGE_SMALL], SDL_SRCCOLORKEY, SDL_MapRGB(pgraphics->image[piece][IMAGE_SMALL]->format, red, green, blue));
                                    }
				}
			    }

			    state = GRAPHICSPARSER_PIECES;
			}
			if(parser_match(pparser, 0, "image") && psurface != NULL)
			{
			    /* If this is the first shadow image for this
			       piece, make a note of its size and offset for
			       later use. */
			    if(imagetype == IMAGE_SHADOW && piece >= 0 && pgraphics->image[piece][IMAGE_SHADOW] == NULL)
			    {
			        pgraphics->shadow_z[piece] = z;
			        pgraphics->shadow_offset_x[piece][9] = x;
			        pgraphics->shadow_offset_y[piece][9] = y;
				pgraphics->shadow_width[piece][9] = psurface->w;
				pgraphics->shadow_height[piece][9] = psurface->h;
			    }
			    graphics_addimage(pgraphics, piece, imagetype, psurface);
			    SDL_FreeSurface(psurface);
			}
			if(parser_match(pparser, 0, "clone") && piece >= 0)
			{
			    if(clone != PIECE_UNKNOWN)
			    {
				if(!(pgraphics->image_flags[clone] & GRAPHICS_CLONE) && clone != piece)
				{
			            pgraphics->image_flags[piece] |= GRAPHICS_CLONE;
				    pgraphics->clone[piece] = clone;
				}
			        else
				    pmenu = graphics_error(pmenu, filename, pparser, gettext("Clone loop"));
			    }
			    else
				pmenu = graphics_error(pmenu, filename, pparser, gettext("Invalid piece for clone"));
			}
			break;

		    case GRAPHICSPARSER_SIZES:
                        /* </sizes>: process available sizes */
			if(parser_match(pparser, 0, "sizes"))
			{
                            /* Move out of <sizes> into <chroma> */
			    state = GRAPHICSPARSER_CHROMA;

                            /* Find an appropriate size for the pieces */
                            /* First, is there an exact match? */
                            pgraphics->size_x = 0;
                            pgraphics->size_y = 0;
                            psize = pgraphics->sizes;
                            while(psize != NULL)
                            {
                                if(psize->x == options_sdl_size_x && psize->y == options_sdl_size_y && psize->flags & SIZE_PIECES)
                                {
                                    pgraphics->size_x = psize->x;
                                    pgraphics->size_y = psize->y;
                                }
                                psize = psize->next;
                            }
                            /* If none matches, choose the largest that will
                               allow the level to be fully displayed */
                            if(pgraphics->size_y == 0)
                            {
                                psize = pgraphics->sizes;
                                while(psize != NULL)
                                {
                                    size_x = 32;
                                    size_y = 24;
                                    if(plevelcurrent != NULL)
                                    {
                                        size_x = plevelcurrent->size_x;
                                        size_y = plevelcurrent->size_y;
#ifdef XOR_COMPATIBILITY
                                        if(plevelcurrent->mode == MODE_XOR && options_xor_display)
                                        {
                                            size_x = 8;
                                            size_y = 8;
                                        }
#endif
                                    }

                                    if(psize->y > pgraphics->size_y && psize->x <= screen_width / size_x && psize->y <= screen_height / size_y && psize->flags & SIZE_PIECES)
                                    {
                                        pgraphics->size_x = psize->x;
                                        pgraphics->size_y = psize->y;
                                    }
                                    psize = psize->next;
                                }
                            }
                            /* If still no match, use the smallest */
                            if(pgraphics->size_y == 0)
                            {
                                pgraphics->size_x = SIZE_HUGE;
                                pgraphics->size_y = SIZE_HUGE;
                                psize = pgraphics->sizes;
                                while(psize != NULL)
                                {
                                    if(psize->y < pgraphics->size_y && psize->flags & SIZE_PIECES)
                                    {
                                        pgraphics->size_x = psize->x;
                                        pgraphics->size_y = psize->y;
                                    }
                                    psize = psize->next;
                                }
                            }
                            if(pgraphics->size_y == SIZE_HUGE)
                            {
                                /* Something failed badly */
                                /* An error will be generated at </head> */
                                pgraphics->size_x = 0;
                                pgraphics->size_y = 0;
                            }

                            /* Find an appropriate size for the small pieces */
                            pgraphics->small_size_x = 0;
                            pgraphics->small_size_y = 0;
                            psize = pgraphics->sizes;
                            while(psize != NULL)
                            {
                                if(psize->y < font_height_game && psize->flags & SIZE_SMALL)
                                {
                                    pgraphics->small_size_x = psize->x;
                                    pgraphics->small_size_y = psize->y;
                                }
                                psize = psize->next;
                            }
                            /* If no size was small enough, use the smallest */
                            if(pgraphics->small_size_y == 0)
                            {
                                pgraphics->small_size_x = SIZE_HUGE;
                                pgraphics->small_size_y = SIZE_HUGE;
                                psize = pgraphics->sizes;
                                while(psize != NULL)
                                {
                                    if(psize->y < pgraphics->small_size_y && psize->flags & SIZE_SMALL)
                                    {
                                        pgraphics->small_size_x = psize->x;
                                        pgraphics->small_size_y = psize->y;
                                    }
                                    psize = psize->next;
                                }
                                if(pgraphics->small_size_y == SIZE_HUGE)
                                {
                                    /* Something failed badly! */
                                    pgraphics->small_size_x = 0;
                                    pgraphics->small_size_y = 0;
                                }
                            }
			}

                        /* </size>: Store this size in the list of sizes */
			if(parser_match(pparser, 0, "size"))
			{
			    if(x != -1 && y != -1)
			    {
			        psize = malloc(sizeof(struct graphicssize));
			        if(psize == NULL)
				    fatal(gettext("Out of memory in graphics_load()"));
				psize->x = x;
				psize->y = y;
                                psize->flags = flags;
				psize->next = NULL;

				if(pgraphics->sizes == NULL)
				    pgraphics->sizes = psize;
				else
				{
				    psizetmp = pgraphics->sizes;
				    while(psizetmp->next != NULL)
					psizetmp = psizetmp->next;
				    psizetmp->next = psize;
				}
			    }
			    else
				pmenu = graphics_error(pmenu, filename, pparser, gettext("Invalid size"));
			}
			break;

		    case GRAPHICSPARSER_SHADOWS:
			if(parser_match(pparser, 0, "shadow"))
			{
			    if(x >= -1 && x <= 1 && y >= -1 && y <= 1)
			    {
			        pshadow = malloc(sizeof(struct shadow));
			        if(pshadow == NULL)
				    fatal(gettext("Out of memory in graphics_load()"));
				pshadow->x = x;
				pshadow->y = y;
				pshadow->next = NULL;

				if(pgraphics->shadows == NULL)
				    pgraphics->shadows = pshadow;
				else
				{
				    pshadowtmp = pgraphics->shadows;
				    while(pshadowtmp->next != NULL)
					pshadowtmp = pshadowtmp->next;
				    pshadowtmp->next = pshadow;
				}
			    }
			    else
				pmenu = graphics_error(pmenu, filename, pparser, gettext("Invalid shadow"));
			}
                        if(parser_match(pparser, 0, "shadows"))
                        {   
                            state = GRAPHICSPARSER_CHROMA;
			}
			break;

		    default:
			break;
		}

		break;

	    case PARSER_CONTENT:
		switch(state)
		{
		    case GRAPHICSPARSER_CHROMA:
		        if(parser_match(pparser, 1, "title"))
			{
			    pgraphics->title = malloc(strlen(parser_text(pparser, 0)) + 1);
			    if(pgraphics->title == NULL)
				fatal(gettext("Out of memory in graphics_load()"));
		            strcpy(pgraphics->title, parser_text(pparser, 0));
			}

			break;

		    default:
			break;
		}
		break;

	    case PARSER_ATTRIBUTE:
		if(parser_match(pparser, 2, "if") && parser_match(pparser, 1, "condition"))
		{
		    if(state >= GRAPHICSPARSER_IF)
			failed ++;
		    else
		    {
			if(!graphics_evaluate(pgraphics, parser_text(pparser, 0)))
			{
			    state += GRAPHICSPARSER_IF;
			    failed ++;
			}
		    }
		}

		switch(state)
		{
		    case GRAPHICSPARSER_OUTSIDE:
		        if(parser_match(pparser, 2, "chroma") && parser_match(pparser, 1, "type"))
		        {
		            if(parser_match(pparser, 0, "graphics"))
		        	state = GRAPHICSPARSER_CHROMA;
		        }
			break;

		    case GRAPHICSPARSER_CHROMA:
		        if(parser_match(pparser, 2, "chroma") && parser_match(pparser, 1, "hidden"))
		        {
		            if(parser_match(pparser, 0, "yes"))
                            {
                                if(partial && !(options_debug & DEBUG_HIDDEN))
                                    state = GRAPHICSPARSER_BAD;
                            }
		        }
		        if(parser_match(pparser, 2, "title") && parser_match(pparser, 1, "translate"))
		        {
		            if(parser_match(pparser, 0, "yes"))
                                pgraphics->flags |= GRAPHICS_TRANSLATE;
		        }
			if(parser_match(pparser, 2, "background") && (parser_match(pparser, 1, "colour") || parser_match(pparser, 1, "color")))
			{
			    if(graphics_translatecolour(parser_text(pparser,0), &red, &green, &blue, &alpha))
			    {
				pgraphics->background[0] = red;
				pgraphics->background[1] = green;
				pgraphics->background[2] = blue;
                                pgraphics->flags |= GRAPHICS_BACKGROUND;

			    }
			    else
				pmenu = graphics_error(pmenu, filename, pparser, gettext("Invalid colour"));
			}

			break;

		    case GRAPHICSPARSER_PIECES:
		        if(parser_match(pparser, 2, "pieces") && parser_match(pparser, 1, "path"))
		           strcpy(path, parser_text(pparser, 0));

		        if(parser_match(pparser, 2, "pieces") && parser_match(pparser, 1, "levels"))
                        {
			    pgraphics->levels = atoi(parser_text(pparser, 0));
                            if(pgraphics->levels > 1)
                            {
                                if(options_graphic_level != 0)
                                    pgraphics->level = options_graphic_level % (pgraphics->levels + 1);
                                else
                                {
                                    if(plevelcurrent != NULL && plevelcurrent->level != 0)
                                        pgraphics->level = plevelcurrent->level;
                                    else
                                        pgraphics->level = 1;
                                }
                            }
                        }

		        if(parser_match(pparser, 2, "piece") && parser_match(pparser, 1, "name"))
			{
			    for(i = 0; i < PIECE_UNKNOWN; i ++)
			    {
				if(strcasecmp(parser_text(pparser, 0), piece_name[i]) == 0)
				    piece = i;
			    }
			    if(piece != PIECE_UNKNOWN)
				state = GRAPHICSPARSER_PIECE;
			    else
				pmenu = graphics_error(pmenu, filename, pparser, gettext("Invalid piece name"));
			}
			break;

		    case GRAPHICSPARSER_PIECE:
			if(parser_match(pparser, 2, "piece") && parser_match(pparser, 1, "bevel"))
			{
			    if(strcmp(parser_text(pparser, 0), "piece") == 0)
			        pgraphics->image_flags[piece] |= GRAPHICS_BEVEL;
			    else if(strcmp(parser_text(pparser, 0), "shadow") == 0)
			        pgraphics->image_flags[piece] |= GRAPHICS_BEVEL_SHADOW;
			    else if(strcmp(parser_text(pparser, 0), "16") == 0)
			        pgraphics->image_flags[piece] |= GRAPHICS_BEVEL16;
			    else
				pmenu = graphics_error(pmenu, filename, pparser, gettext("Invalid bevel type"));
			}
			if(parser_match(pparser, 2, "piece") && parser_match(pparser, 1, "mover"))
			    pgraphics->image_flags[piece] |= GRAPHICS_MOVER;
			if(parser_match(pparser, 2, "piece") && parser_match(pparser, 1, "random"))
			    pgraphics->image_flags[piece] |= GRAPHICS_RANDOM;
			if(parser_match(pparser, 2, "piece") && parser_match(pparser, 1, "animate"))
			    pgraphics->image_flags[piece] |= GRAPHICS_ANIMATE;
			if(parser_match(pparser, 2, "piece") && parser_match(pparser, 1, "level"))
			    pgraphics->image_flags[piece] |= GRAPHICS_LEVEL;
			if(parser_match(pparser, 2, "piece") && parser_match(pparser, 1, "tile"))
			    pgraphics->image_flags[piece] |= GRAPHICS_TILE;
			if(parser_match(pparser, 2, "piece") && parser_match(pparser, 1, "scale"))
			    pgraphics->image_flags[piece] |= GRAPHICS_SCALE;

                        /* <image type="value" */
			if(parser_match(pparser, 2, "image") && parser_match(pparser, 1, "type"))
			{
			    if(parser_match(pparser, 0, "piece"))
				imagetype = IMAGE_PIECE;

			    if(parser_match(pparser, 0, "shadow"))
				imagetype = IMAGE_SHADOW;

			    if(parser_match(pparser, 0, "small"))
                            {
                                w = pgraphics->small_size_x;
                                h = pgraphics->small_size_y;
				imagetype = IMAGE_SMALL;
                            }
			}
                        
                        /* <image x="value" */
			if(parser_match(pparser, 2, "image") && parser_match(pparser, 1, "x"))
			    x = graphics_evaluatesize(parser_text(pparser, 0), pgraphics->size_x);
			if(parser_match(pparser, 2, "image") && parser_match(pparser, 1, "y"))
			    y = graphics_evaluatesize(parser_text(pparser, 0), pgraphics->size_y);
			if(parser_match(pparser, 2, "image") && parser_match(pparser, 1, "z"))
			    z = atoi(parser_text(pparser, 0));
			if(parser_match(pparser, 2, "image") && (parser_match(pparser, 1, "w") || parser_match(pparser, 1, "width")))
			    w = graphics_evaluatesize(parser_text(pparser, 0), pgraphics->size_x);
			if(parser_match(pparser, 2, "image") && (parser_match(pparser, 1, "h") || parser_match(pparser, 1, "height")))
			    h = graphics_evaluatesize(parser_text(pparser, 0), pgraphics->size_y);

			/* Create solid colour image */
			if(parser_match(pparser, 2, "image") && (parser_match(pparser, 1, "colour") || parser_match(pparser, 1, "color")))
			{
			    if(graphics_translatecolour(parser_text(pparser,0), &red, &green, &blue, &alpha))
			    {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                                psurface = SDL_CreateRGBSurface(screen_flags, w, h, 32, 0xff000000, 0xff0000, 0xff00, 0xff);
#else
                                psurface = SDL_CreateRGBSurface(screen_flags, w, h, 32, 0xff, 0xff00, 0xff0000, 0xff000000);

#endif
                                if(alpha == 255)
                                {
                                    SDL_SetAlpha(psurface, 0, 0);
                                    SDL_FillRect(psurface, NULL, SDL_MapRGB(psurface->format, red, green, blue));
                                }
                                else
                                {
                                    SDL_FillRect(psurface, NULL, SDL_MapRGBA(psurface->format, red, green, blue, alpha));
                                }
			    }
			    else
				pmenu = graphics_error(pmenu, filename, pparser, gettext("Invalid colour"));
			}

			/* Load file from image */
			if(parser_match(pparser, 2, "image") && parser_match(pparser, 1, "file"))
			{
			    /* Construct pathname */
			    if(strcmp(path, "") == 0)
			        sprintf(file, "%s/", basepath);
			    else
			        sprintf(file, "%s/%s/", basepath, path);

			    /* Substitute %s as necessary */
			    j = strlen(file);
			    for(i = 0; i <= strlen(parser_text(pparser, 0)); i ++)
			    {
				c = pparser->stack[pparser->depth - 1][i];
				if(c == '%')
				{
				    i ++;
				    c = pparser->stack[pparser->depth - 1][i];
				    file[j] = 0;
				    switch(c)
				    {
					case '%':
					    strcat(file, "%");
					    break;
					case 'x':
					    sprintf(buffer, "%d", imagetype == IMAGE_SMALL ? pgraphics->small_size_x : pgraphics->size_x);
					    strcat(file, buffer);
					    break;
					case 'y':
					    sprintf(buffer, "%d", imagetype == IMAGE_SMALL ? pgraphics->small_size_y : pgraphics->size_y);
					    strcat(file, buffer);
					    break;
					case 'l':
                                            if(pgraphics->levels != 0)
					        sprintf(buffer, "%d", pgraphics->level);
                                            else
					        sprintf(buffer, "%d", 0);
					    strcat(file, buffer);
					    break;
					default:
					    sprintf(buffer, "%%%c", c);
					    strcat(file, buffer);
					    break;
				    }
				    j = strlen(file);
				}
				else
				    file[j ++ ] = c;
			    }

			    /* Load the file */
	                    if(isfile(file))
	                        psurface = IMG_Load(file);
			    else
			    {
				sprintf(buffer, gettext("Invalid filename '%s'"), file);
				pmenu = graphics_error(pmenu, filename, pparser, buffer);
			    }
                            if(psurface != NULL && (psurface->w != w || psurface->h != h) && (pgraphics->image_flags[piece] & GRAPHICS_SCALE))
                            {
                                psurface = graphics_scaleimage(psurface, w, h);

                            }
			}

			/* Key transparency */
			if(parser_match(pparser, 2, "image") && parser_match(pparser, 1, "key"))
			{
			    /* Store values in red, green, blue for use later */
			    if(graphics_translatecolour(parser_text(pparser,0), &red, &green, &blue, &alpha))
			    {
				pgraphics->image_flags[piece] |= GRAPHICS_KEY;
			    }
			    else
				pmenu = graphics_error(pmenu, filename, pparser, gettext("Invalid colour"));
			}

		        if(parser_match(pparser, 2, "clone") && parser_match(pparser, 1, "piece"))
			{
			    for(i = 0; i < PIECE_UNKNOWN; i ++)
			    {
				if(strcasecmp(parser_text(pparser, 0), piece_name[i]) == 0)
				    clone = i;
			    }
			}
			break;

		    case GRAPHICSPARSER_SIZES:
		        if(parser_match(pparser, 2, "size") && parser_match(pparser, 1, "x"))
			    x = atoi(parser_text(pparser, 0));
		        if(parser_match(pparser, 2, "size") && parser_match(pparser, 1, "y"))
			    y = atoi(parser_text(pparser, 0));
		        if(parser_match(pparser, 2, "size") && parser_match(pparser, 1, "pieces"))
                        {
                            if(strcasecmp(parser_text(pparser, 0), "yes") == 0)
                                flags |= SIZE_PIECES;
                            if(strcasecmp(parser_text(pparser, 0), "no") == 0)
                                flags &= ~SIZE_PIECES;
                        }
		        if(parser_match(pparser, 2, "size") && parser_match(pparser, 1, "small"))
                        {
                            if(strcasecmp(parser_text(pparser, 0), "yes") == 0)
                                flags |= SIZE_SMALL;
                            if(strcasecmp(parser_text(pparser, 0), "no") == 0)
                                flags &= ~SIZE_SMALL;
                        }
			break;

		    case GRAPHICSPARSER_SHADOWS:
		        if(parser_match(pparser, 2, "shadow") && parser_match(pparser, 1, "x"))
			    x = atoi(parser_text(pparser, 0));
		        if(parser_match(pparser, 2, "shadow") && parser_match(pparser, 1, "y"))
			    y = atoi(parser_text(pparser, 0));
			break;

		    default:
			break;
		}

		break;

	    case PARSER_ERROR:
		pmenu = graphics_error(pmenu, filename, pparser, parser_text(pparser, 0));
		break;

	    default:
		break;
	}
    }

    parser_delete(pparser);

    /* If we failed to find <chroma type="graphics">, this isn't a valid graphics file */
    if(state == GRAPHICSPARSER_BAD)
    {
	graphics_delete(pgraphics);
	return NULL;
    }

    if(partial)
	return pgraphics;

    /* If curses emulation, create graphics from the font */
    if(pgraphics->flags & GRAPHICS_CURSES)
        graphics_createfromfont(pgraphics);

    /* Next, perform a sanity check on the graphics */

    /* Patch up any pieces without an image with a random one */
    for(i = 0; i < PIECE_MAX; i ++)
    {
	if(pgraphics->image[i][IMAGE_PIECE] == NULL && !(pgraphics->image_flags[i] & GRAPHICS_CLONE))
	{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            psurface = SDL_CreateRGBSurface(screen_flags, pgraphics->size_x, pgraphics->size_y, 32, 0xff000000, 0xff0000, 0xff00, 0xff);
#else
            psurface = SDL_CreateRGBSurface(screen_flags, pgraphics->size_x, pgraphics->size_y, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
#endif

	    fg = pdisplaycolours->foreground[i];
	    bg = pdisplaycolours->background[i];

	    if(bg == -1)
		bg = 0;
	    if(fg == -1)
		fg = 7;

	    if(pdisplaycolours->reverse[i])
	    {
                tg = fg; fg = bg; bg = tg; 
	    }

            SDL_FillRect(psurface, NULL, SDL_MapRGB(psurface->format, bg & 1 ? 255 : 0,  bg & 2 ? 255 : 0, bg & 4 ? 255 : 0));
 	    sprintf(buffer, "%c", pdisplaycolours->character[i]);

            psurfacetmp = font_render(buffer, fg);
            drect.x = (psurface->w - psurfacetmp->w) / 2;
            drect.y = (psurface->h - psurfacetmp->h) / 2;
            SDL_BlitSurface(psurfacetmp, NULL, psurface, &drect);
            SDL_FreeSurface(psurfacetmp);

            pgraphics->image[i][IMAGE_PIECE] = SDL_DisplayFormat(psurface);
	    SDL_FreeSurface(psurface);
	}
    }

    /* Set clones */
    for(i = 0; i < PIECE_MAX; i ++)
    {
	if(pgraphics->image_flags[i] & GRAPHICS_CLONE)
	{
	    x = pgraphics->clone[i];
	    pgraphics->image[i][IMAGE_PIECE] = pgraphics->image[x][IMAGE_PIECE];

	    pgraphics->image[i][IMAGE_SHADOW] = pgraphics->image[x][IMAGE_SHADOW];
    	    pgraphics->shadow_width[i][9] = pgraphics->shadow_width[x][9];
    	    pgraphics->shadow_height[i][9] = pgraphics->shadow_height[x][9];
	    pgraphics->shadow_offset_x[i][9] = pgraphics->shadow_offset_x[x][9];
	    pgraphics->shadow_offset_y[i][9] = pgraphics->shadow_offset_y[x][9];
	}
    }

    /* Bevelling requires five images */
    if(pgraphics->image_flags[PIECE_SPACE] & GRAPHICS_BEVEL)
    {
	if(pgraphics->image[PIECE_SPACE][IMAGE_PIECE]->w < pgraphics->size_x * 5)
	{
	    pgraphics->image_flags[PIECE_SPACE] ^= GRAPHICS_BEVEL;
	    sprintf(buffer, gettext("%s bevelling requires five images"), piece_name[PIECE_SPACE]);
	    pmenu = graphics_error(pmenu, filename, NULL, buffer);
	}
    }
    if(pgraphics->image_flags[PIECE_WALL] & GRAPHICS_BEVEL)
    {
	if(pgraphics->image[PIECE_WALL][IMAGE_PIECE]->w < pgraphics->size_x * 5)
	{
	    pgraphics->image_flags[PIECE_WALL] ^= GRAPHICS_BEVEL;
	    sprintf(buffer, gettext("%s bevelling requires five images"), piece_name[PIECE_WALL]);
	    pmenu = graphics_error(pmenu, filename, NULL, buffer);
	}
    }

    if(pgraphics->image_flags[PIECE_WALL] & GRAPHICS_BEVEL_SHADOW)
    {
	if(pgraphics->image[PIECE_WALL][IMAGE_SHADOW]->w < pgraphics->size_x * 5)
	{
	    pgraphics->image_flags[PIECE_WALL] ^= GRAPHICS_BEVEL_SHADOW;
	    sprintf(buffer, gettext("%s bevelling requires five images"), piece_name[PIECE_WALL]);
	    pmenu = graphics_error(pmenu, filename, NULL, buffer);
	}
    }

    /* Create cursor */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    psurface = SDL_CreateRGBSurface(screen_flags, pgraphics->size_x, pgraphics->size_y, 32, 0xff000000, 0xff0000, 0xff00, 0xff);
#else
    psurface = SDL_CreateRGBSurface(screen_flags, pgraphics->size_x, pgraphics->size_y, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
#endif
    i = pgraphics->size_x / 16;
    if(i < 1)
	i = 1;
    rect.x = i * 1; rect.y = i * 1; rect.w = pgraphics->size_x - i * 2; rect.h = pgraphics->size_y - i * 2;
    SDL_FillRect(psurface, &rect, SDL_MapRGB(psurface->format, 0, 0, 0));
    rect.x = i * 2; rect.y = i * 2; rect.w = pgraphics->size_x - i * 4; rect.h = pgraphics->size_y - i * 4;
    SDL_FillRect(psurface, &rect, SDL_MapRGB(psurface->format, 255, 255, 255));
    rect.x = i * 3; rect.y = i * 3; rect.w = pgraphics->size_x - i * 6; rect.h = pgraphics->size_y - i * 6;
    SDL_FillRect(psurface, &rect, SDL_MapRGB(psurface->format, 0, 0, 0));
    rect.x = i * 4; rect.y = i * 4; rect.w = pgraphics->size_x - i * 8; rect.h = pgraphics->size_y - i * 8;
    SDL_FillRect(psurface, &rect, SDL_MapRGBA(psurface->format, 0, 0, 0, 64));
    pgraphics->image[PIECE_CURSOR][IMAGE_PIECE] = psurface;

    /* Calculate quadrants for shadow images */
    for(i = 0; i < PIECE_MAX; i ++)
    {
	if(pgraphics->image[i][IMAGE_SHADOW] != NULL)
	{
	    if(pgraphics->shadow_z[i] != 0)
		pgraphics->flags |= GRAPHICS_ZORDER;

    	    width = pgraphics->shadow_width[i][9];
    	    height = pgraphics->shadow_height[i][9];
	    offset_x = pgraphics->shadow_offset_x[i][9];
	    offset_y = pgraphics->shadow_offset_y[i][9];

	    for(j = 0; j < 3; j ++)
	    {
    	        for(k = 0; k < 3; k ++)
	        {
    		    x = (j - 1) * pgraphics->size_x - offset_x;
		    y = (k - 1) * pgraphics->size_y - offset_y;

		    /* Does the shadow fall into this square? */
		    if(x > -(pgraphics->size_x) && x < width && y > -(pgraphics->size_y) && y < height)
		    {
		        if(x >= 0)
		        {
			    pgraphics->shadow_start_x[i][j+k*3]= x;
			    pgraphics->shadow_offset_x[i][j+k*3] = 0;

		            if(x + pgraphics->size_x > width)
		    	        pgraphics->shadow_width[i][j+k*3] = width - x;
		            else 
			        pgraphics->shadow_width[i][j+k*3] = pgraphics->size_x;
		        }
		        else
		        {
			    pgraphics->shadow_start_x[i][j+k*3] = 0;
			    pgraphics->shadow_offset_x[i][j+k*3] = -x;

			    pgraphics->shadow_width[i][j+k*3] = pgraphics->size_x + x;

			}

		        if(y >= 0)
		        {
			    pgraphics->shadow_start_y[i][j+k*3]= y;
			    pgraphics->shadow_offset_y[i][j+k*3] = 0;

		            if(y + pgraphics->size_y > height)
			        pgraphics->shadow_height[i][j+k*3] = height - y;
		            else
			        pgraphics->shadow_height[i][j+k*3] = pgraphics->size_y;
		        }
		        else
		        {
			    pgraphics->shadow_start_y[i][j+k*3] = 0;
			    pgraphics->shadow_offset_y[i][j+k*3] = -y;

			    pgraphics->shadow_height[i][j+k*3] = pgraphics->size_y + y;
		        }

		        pgraphics->shadow_flags[i] |= shadow_flags[j+k*3];
		    }
		}
	    }
	}
    }
    
    /* Calcuate flags for shadows */
    pshadow = pgraphics->shadows;
    while(pshadow != NULL)
    {
	pshadow->flag = shadow_flags[(pshadow->x + 1) + 3 * (pshadow->y + 1)];
	pshadow->shadow = (1 + pshadow->x) + 3 * (1 + pshadow->y);
	pshadow->nextordered = pshadow->next;
	pshadow = pshadow->next;
    }

    /* Display any errors that have arisen */
    if(pmenu != NULL)
    {
	menu_process(pmenu);
	menu_delete(pmenu);
    }

    return pgraphics;
}

void graphics_reload()
{
    struct graphics *pgraphicstmp;
    int reload;

    /* Fixed size, so no automatic resizing */
    if(options_graphic_level != pdisplaygraphics->level)
        reload = 1;
    else if(options_sdl_size_x != 0 || options_sdl_size_y != 0)
        reload = 0;
    else
    {
        /* Partially load the graphics again and see if they're different */
        pgraphicstmp = graphics_load(options_graphics, 1);

        reload = 0;
        if(pgraphicstmp == NULL || pdisplaygraphics == NULL ||
		pdisplaygraphics->size_x != pgraphicstmp->size_x ||
                pdisplaygraphics->size_y != pgraphicstmp->size_y ||
                pdisplaygraphics->small_size_x != pgraphicstmp->small_size_x ||
                pdisplaygraphics->small_size_y != pgraphicstmp->small_size_y)
            reload = 1;

	if(pgraphicstmp != NULL)
            graphics_delete(pgraphicstmp);
    }

    if(reload)
        graphics_init();
}

SDL_Surface *graphics_scaleimage(SDL_Surface* psurface, int width, int height)
{
    SDL_Surface* pnew;
    Uint32 *src;
    Uint32 *dst;
    SDL_Color *colour;
    int i, j;
    Uint8 r, g, b, a;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    pnew = SDL_CreateRGBSurface(screen_flags, width, height, 32, 0xff000000, 0xff0000, 0xff00, 0xff);
#else
    pnew = SDL_CreateRGBSurface(screen_flags, width, height, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
#endif

    SDL_SetAlpha(pnew, 0, 0);

    /* This is probably not an optimal way of doing this! */
    for(j = 0; j < height; j ++)
    {
        for(i = 0; i < width; i ++)
        {
            src = psurface->pixels
                + (j * psurface->h / height) * psurface->pitch
                + (i * psurface->w / width) * psurface->format->BytesPerPixel;
            dst = pnew->pixels
                + j * pnew->pitch
                + i * pnew->format->BytesPerPixel;
            if(psurface->format->BitsPerPixel == 8)
            {
                colour = &(psurface->format->palette->colors[(Uint8)*src]);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                *dst = colour->r * 0x1000000 + colour->g * 0x10000 + colour->b * 0x100 + 0xff;
#else
                *dst = colour->r + colour->g * 0x100 + colour->b * 0x10000 + 0xff000000;
#endif
            }
            else
            {
                SDL_GetRGBA(*src, psurface->format, &r, &g, &b, &a);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                *dst = r * 0x1000000 + g * 0x10000 + b * 0x100 + a;
#else
                *dst = r + g * 0x100 + b * 0x10000 + a * 0x10000000;
#endif
            }
        }
    }

    SDL_FreeSurface(psurface);

    return pnew;
}
