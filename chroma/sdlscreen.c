/*  
    sdlscreen.c

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
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <libintl.h>
#include <locale.h>

#include "chroma.h"
#include "sdlfont.h"
#include "util.h"
#include "level.h"
#include "graphics.h"

extern void flip_it(void);
SDL_Surface *ScreenSurface;
SDL_Surface *screen_surface;

int screen_width;
int screen_height;
int screen_fullscreen;
int screen_bpp;
int screen_flags = SDL_SWSURFACE;

extern struct graphics* pdisplaygraphics;

void screen_size(int width, int height, int fullscreen)
{
    int flags;
    int i, j;
    SDL_Rect **modes;

    flags = screen_flags | SDL_RESIZABLE;

    if(fullscreen)
	flags |= SDL_FULLSCREEN;

    if(width == 0 && height == 0)
    {
        for(j = 0; j < 2; j ++)
        {
            modes = SDL_ListModes(NULL, flags | (j == 1 ? SDL_FULLSCREEN : 0));
            
            /* Are there any modes available? */
            if(modes == (SDL_Rect **)0)
            {
                if(j == 1)
                    fatal(gettext("No resolutions available"));
            }
            /* Is resolution unrestricted? If not, examine list of modes */
            else if(modes != (SDL_Rect **)-1)
            {
                for(i = 0; modes[i]; i ++)
                {
                    /* and choose the largest one available, avoiding rotated options */
                    if(modes[i]->w >= modes[i]->h && modes[i]->w >= width && modes[i]->h >= height)
                    {
                        width = modes[i]->w;
                        height = modes[i]->h;
                    }
                }
            }
        }
        /* Fallback to defaults if we didn't find anything */
        if(width == 0 && height == 0)
        {
            width = 640;
            height = 480;
        }
    }

		width = 320;
		height = 240;
    //screen_surface = SDL_SetVideoMode(width, height, 0, flags);
		ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
    screen_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 16, 0,0,0,0);

    SDL_WM_SetCaption("chroma", NULL);
    SDL_ShowCursor(0);

    screen_width = width;
    screen_height = height;
    screen_fullscreen = fullscreen;

    if(screen_surface == NULL)
    {
        /* Emergency fallback */
        if(width != 640 && height != 480)
        {
            screen_size(640, 480, 0);
        }
        else
	    fatal(gettext("Unable to create screen in screen_size"));
    }
}

void screen_clear(int r, int g, int b)
{
    SDL_Rect drect;

    drect.x = 0;
    drect.y = 0;
    drect.w = screen_width;
    drect.h = screen_height;

    SDL_FillRect(screen_surface, &drect, SDL_MapRGB(screen_surface->format, r, g, b));

    //SDL_UpdateRects(screen_surface, 1, &drect);
		flip_it();
}

void screen_resize(int screen_width, int screen_height, int screen_fullscreen)
{
    screen_size(screen_width, screen_height, screen_fullscreen);
    font_resize();
    graphics_reload();
}

void screen_resizeevent(SDL_Event *event)
{
    screen_width = ((SDL_ResizeEvent *)event)->w;
    screen_height = ((SDL_ResizeEvent *)event)->h;

    screen_resize(screen_width, screen_height, screen_fullscreen);
}

void screen_redraw(int x, int y, int w, int h)
{
    SDL_Rect drect;

    drect.x = x;
    drect.y = y;
    drect.w = w;
    drect.h = h;

    if(drect.x < 0)
        drect.x = 0;
    if(drect.x >= screen_width)
        return;
    if(drect.y >= screen_height)
        return;
    if(drect.y < 0)
        drect.y = 0;
    if(drect.x + drect.w > screen_width)
        drect.w = screen_width - drect.x;
    if(drect.y + drect.h > screen_height)
        drect.h = screen_height - drect.y;

    //SDL_UpdateRects(screen_surface, 1, &drect);
		flip_it();
}

void screen_plotimage(SDL_Surface *psurface, int x, int y)
{
    SDL_Rect drect;

    if(psurface == NULL)
	return;

    drect.x = x;
    drect.y = y;
    drect.w = psurface->w;
    drect.h = psurface->h;

    SDL_BlitSurface(psurface, NULL, screen_surface, &drect);
    screen_redraw(x, y, psurface->w, psurface->h);
}

void screen_cursor(int cursor)
{/*
    if(!screen_fullscreen)
    {
	SDL_ShowCursor(SDL_ENABLE);
	return;
    }
    SDL_ShowCursor(cursor ? SDL_ENABLE : SDL_DISABLE);*/
}
