/*  
    sdldisplay.c

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
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <libintl.h>
#include <locale.h>

#include "chroma.h"
#include "menu.h"
#include "level.h"
#include "display.h"
#include "graphics.h"
#include "colours.h"
#include "sdlfont.h"
#include "sdlscreen.h"
#include "util.h"
#include "actions.h"
#include "xmlparser.h"

#define MOUSE_TIMEOUT_CLICK 200
#define MOUSE_TIMEOUT_MOVE 1000

char options_colours[FILENAME_MAX] = COLOURS_DEFAULT;
char options_graphics[FILENAME_MAX] = GRAPHICS_DEFAULT;
int options_graphic_level = 0;
int options_sdl_fullscreen = 1;
int options_sdl_width = 0;
int options_sdl_height = 0;
int options_sdl_delay = 100;
int options_sdl_player_delay = 200;
int options_sdl_replay_delay = 200;
int options_sdl_undo_delay = 200;
int options_sdl_size_x = 0;
int options_sdl_size_y = 0;
int options_debug = 0;
#ifdef XOR_COMPATIBILITY
int options_xor_options = 0;
int options_xor_mode = 1;
int options_xor_display = 0;

int xor_map_scale = 4;
int xor_map_x_offset = 0;
int xor_map_y_offset = 0;
#endif

#ifdef ENIGMA_COMPATIBILITY
int options_enigma_options = 0;
int options_enigma_mode = 1;
#endif

int actions[SDLK_LAST];
int actions_mouse[3][MOUSE_BUTTONS_MAX];

int display_offset_x;
int display_offset_y;
int display_offset_pixels_x = 0;
int display_offset_pixels_y = 0;
int display_start_x;
int display_start_y;
int display_end_x;
int display_end_y;
int display_pieces_x;
int display_pieces_y;
int display_focus_x;
int display_focus_y;
int display_bar_pixels = 0;
float display_animation;
int display_animation_x;
int display_animation_y;
int display_border_x = 3;
int display_border_y = 3;

struct SDL_Surface* psurfacelogo = NULL;
struct SDL_Surface* psurfacelogosmall = NULL;

extern int font_height;

extern SDL_Surface *screen_surface;

extern int screen_width;
extern int screen_height;
extern int screen_fullscreen;

extern int font_height;
extern int font_width;
extern int font_border;
extern int font_padding;

extern int font_size_game;

extern char *piece_name[];
extern char *action_name[];
extern char *action_shortname[];

extern int move_x[];
extern int move_y[];

extern struct graphics* pdisplaygraphics;
extern struct colours* pdisplaycolours;

extern char options_graphics[];
extern char options_colours[];

extern int *editor_piece_maps[];

void display_movers(struct level* plevel, int redraw);
void displayshadowed_movers(struct level* plevel, int redraw);

void displayshadowed_level(struct level* plevel);

void display_clip(struct level* plevel, int clip);
void display_screensizemenu();

void display_initactions();
char *display_keyname(SDLKey key);
void display_addkeytomenu(struct menu* pmenu, int action, char *text);
void display_options_keys();
void display_options_mouse();
void display_options_size();
void display_options_debug();

#ifdef XOR_COMPATIBILITY
void xor_focus(struct level* plevel);
#endif

void display_options_othergames();

#define SCREENREDRAW_LEVEL	1
#define SCREENREDRAW_BAR	2
#define SCREENREDRAW_ALL	(SCREENREDRAW_BAR | SCREENREDRAW_LEVEL)

extern SDL_Surface *ScreenSurface;
void flip_it(void)
{
#if 1
	if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
	int x, y;
	uint32_t *s = screen_surface->pixels;
	uint32_t *d = ScreenSurface->pixels;
	for(uint8_t y2 = 0; y2 < 240; y2++, s += 160, d += 320) 
		memmove(d, s, 1280); // double-line fix by pingflood, 2018
	if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
#else
	printf("%d %d\n", screen_surface->w, screen_surface->h);
	SDL_Surface *p = SDL_ConvertSurface(screen_surface, ScreenSurface->format, 0);
	SDL_SoftStretch(p, NULL, ScreenSurface, NULL);
	SDL_FreeSurface(p);
#endif
	SDL_Flip(ScreenSurface);
}

void display_init()
{
    char buffer[256];
    struct SDL_Surface* psurfaceicon;

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        snprintf(buffer, 256, gettext("Unable to initalise SDL: %s"), SDL_GetError());
        fatal(buffer);
    }

    display_options_load();

    atexit(display_quit);

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
    SDL_EnableUNICODE(1);

    psurfaceicon = graphics_loadimage("icon.png");
    if(psurfaceicon != NULL)
        SDL_WM_SetIcon(psurfaceicon, NULL);

    screen_size(options_sdl_width, options_sdl_height, options_sdl_fullscreen);
    screen_clear(255, 255, 255);

    font_init();
    font_resize();
    colours_init();
    graphics_init();

}

void display_quit()
{
    SDL_Quit();
}

void display_piece(struct level* plevel, int p, int x, int y, int d)
{
    SDL_Surface *image;

    SDL_Rect srect;
    SDL_Rect drect;
    int alpha;
    int px, py;
    int b;
    int bimage[4];
    int i;
    int bsizex, bsizey, boffset;
    int op;

    int xstart = 0, xend = 0, xsize = 0, xpos = 0;

    if(p < PIECE_SPACE || p >= PIECE_MAX)
        return;

#ifdef XOR_COMPATIBILITY
    if(plevel->switched && (p == PIECE_WALL || p == PIECE_SPACE))
        p = PIECE_DARKNESS;
#endif

    op = p;

    px = x * pdisplaygraphics->size_x + display_offset_pixels_x; 
    py = y * pdisplaygraphics->size_y + display_offset_pixels_y; 
    if(d != MOVE_NONE
            && p != PIECE_SPACE && !isexplosion(p)
#ifdef XOR_COMPATIBILITY
            && p != PIECE_TELEPORT
#endif
            ) 
    { 
        if(d == MOVE_LEFT)
            px = px - display_animation_x;
        if(d == MOVE_RIGHT)
            px = px + display_animation_x;
        if(d == MOVE_UP)
            py = py - display_animation_y;
        if(d == MOVE_DOWN)
            py = py + display_animation_y;
    } 

    if(isexplosion(p) && !(pdisplaygraphics->image_flags[p] & GRAPHICS_KEY))
    {
        alpha = 255 * (1 - display_animation);
        SDL_SetAlpha(pdisplaygraphics->image[p][IMAGE_PIECE], SDL_SRCALPHA, alpha);
    }
    if(isnewexplosion(p))
    {
        p += PIECE_EXPLOSION_FIRST - PIECE_EXPLOSION_NEW_FIRST;
        if(!(pdisplaygraphics->image_flags[p] & GRAPHICS_KEY))
        {
            alpha = 255 * display_animation;
            SDL_SetAlpha(pdisplaygraphics->image[p][IMAGE_PIECE], SDL_SRCALPHA, alpha);
        }
    }

    image = pdisplaygraphics->image[p][IMAGE_PIECE];

    srect.x = 0;
    srect.y = 0;
    srect.w = pdisplaygraphics->size_x;
    srect.h = pdisplaygraphics->size_y;

    if(image->w > pdisplaygraphics->size_x)
    {
        xstart = 0;
        xend = image->w / pdisplaygraphics->size_x;
        xsize = 1;
        xpos = 0;

        if(pdisplaygraphics->image_flags[p] & GRAPHICS_BEVEL)
            xend -= 4;

        if(pdisplaygraphics->image_flags[p] & GRAPHICS_BEVEL16)
            xsize = 16;

        if(pdisplaygraphics->image_flags[p] & GRAPHICS_MOVER)
        {
            xsize = 5;

            if(d == MOVE_LEFT)
                xpos += 1;
            if(d == MOVE_UP)
                xpos += 2;
            if(d == MOVE_RIGHT)
                xpos += 3;
            if(d == MOVE_DOWN)
                xpos += 4;
        }

        /* If we're plotting the players */
        if(p == PIECE_PLAYER_ONE || p == PIECE_PLAYER_TWO)
        {
            /* and there's an image for the swapped player */
            if(xend > xstart + xsize)
            {
                /* then use it if the player is swapped out */
                if(plevel->player != (p & 1) && plevel->player != 2)
                    xpos += xsize;
            }
        }

        if(pdisplaygraphics->image_flags[p] & GRAPHICS_BEVEL16)
        {
            i = 15;
                b = level_data(plevel, x, y) & BEVEL_ALL;

            if(b & BEVEL_U)
                i -= 1;
            if(b & BEVEL_R)
                i -= 2;
            if(b & BEVEL_D)
                i -= 4;
            if(b & BEVEL_L)
                i -= 8;

            xpos += i;
        }

        if(pdisplaygraphics->image_flags[p] & GRAPHICS_ANIMATE)
        {
            b = (xend - xstart) / xsize;

            if(!isexplosion(p))
                b = b * display_animation;
            else
                  b = b * ((display_animation + (isnewexplosion(op) ? 0 : 1)) * 0.5);

            xpos += b * xsize;
        }
        else if(pdisplaygraphics->image_flags[p] & GRAPHICS_RANDOM)
        {
            b = (xend - xstart) / xsize;

            if(p == PIECE_SPACE)
                b = (level_data(plevel, x, y) & 0xff) % b;
            else
                b = ((level_data(plevel, x, y) & 0xff00) / 0x100) % b;

            xpos += b * xsize;
        }
        else if(pdisplaygraphics->image_flags[p] & GRAPHICS_TILE)
        {
            b = x % ((xend - xstart) / xsize);
            if(b < 0)
                b += (xend - xstart) / xsize;
            xpos += b * xsize;

            b = y % (image->h / pdisplaygraphics->size_y);
            if(b < 0)
                b += image->h / pdisplaygraphics->size_y;
            srect.y = b * pdisplaygraphics->size_y;
        }

        srect.x = (xstart + xpos) * pdisplaygraphics->size_x;
    }

    drect.x = px; 
    drect.y = py;
    drect.w = pdisplaygraphics->size_x;
    drect.h = pdisplaygraphics->size_y;

    /* Plot piece */
    SDL_BlitSurface(image, &srect, screen_surface, &drect);

    /* Plot bevelling */
    if(pdisplaygraphics->image_flags[p] & GRAPHICS_BEVEL)
    {
        b = level_data(plevel, x, y) & BEVEL_ALL;
        if(b != 0)
        {
            bsizex = pdisplaygraphics->size_x / 2;
            bsizey = pdisplaygraphics->size_y / 2;
            boffset = (xend - 1) * pdisplaygraphics->size_x;

            for(i = 0; i < 4; i ++)
                bimage[i] = 0;

            if(b & BEVEL_L)
            {
                if(b & BEVEL_U)
                    bimage[0] = 3 * pdisplaygraphics->size_x;
                else
                    bimage[0] = 1 * pdisplaygraphics->size_x;

                if(b & BEVEL_D)
                    bimage[2] = 3 * pdisplaygraphics->size_x;
                else
                    bimage[2] = 1 * pdisplaygraphics->size_x;
            }
            else
            {
                if(b & BEVEL_U)
                    bimage[0] = 2 * pdisplaygraphics->size_x;
                if(b & BEVEL_D)
                    bimage[2] = 2 * pdisplaygraphics->size_x;
            }

            if(b & BEVEL_R)
            {
                if(b & BEVEL_U)
                    bimage[1] = 3 * pdisplaygraphics->size_x;
                else
                    bimage[1] = 1 * pdisplaygraphics->size_x;

                if(b & BEVEL_D)
                    bimage[3] = 3 * pdisplaygraphics->size_x;
                else
                    bimage[3] = 1 * pdisplaygraphics->size_x;
            }
            else
            {
                if(b & BEVEL_U)
                    bimage[1] = 2 * pdisplaygraphics->size_x;
                if(b & BEVEL_D)
                    bimage[3] = 2 * pdisplaygraphics->size_x;
            }

            if(b & BEVEL_TL)
                bimage[0] = 4 * pdisplaygraphics->size_x;
            if(b & BEVEL_TR)
                bimage[1] = 4 * pdisplaygraphics->size_x;
            if(b & BEVEL_BL)
                bimage[2] = 4 * pdisplaygraphics->size_x;
            if(b & BEVEL_BR)
                bimage[3] = 4 * pdisplaygraphics->size_x;

            for(i = 0; i < 4; i ++)
            {
                if(bimage[i] != 0)
                {
                    srect.x = boffset + bimage[i] + ((i & 1) ? bsizex : 0);
                    srect.y = (i & 2) ? bsizey : 0;
                    srect.w = bsizex;
                    srect.h = bsizey;

                    drect.x = px + ((i & 1) ? bsizex : 0);
                    drect.y = py + ((i & 2) ? bsizey : 0);
                    drect.w = bsizex;
                    drect.h = bsizey;

                    SDL_BlitSurface(image, &srect, screen_surface, &drect);
                }
            }
        }
    }
}

void display_redrawpiece(int p, int x, int y, int d)
{
    int dx, dy;

    dx = x * pdisplaygraphics->size_x + display_offset_pixels_x;
    dy = y * pdisplaygraphics->size_y + display_offset_pixels_y;
    if(d != MOVE_NONE && p != PIECE_SPACE && !isexplosion(p))
    {
        if(d == MOVE_LEFT) { dx = dx - display_animation_x; }
        if(d == MOVE_RIGHT) { dx = dx + display_animation_x; }
        if(d == MOVE_UP) { dy = dy - display_animation_y; }
        if(d == MOVE_DOWN) { dy = dy + display_animation_y; }
    }

    screen_redraw(dx, dy, pdisplaygraphics->size_x, pdisplaygraphics->size_y);
}

void display_pieceabsolute(int p, int x, int y, int redraw)
{
    SDL_Rect srect;
    SDL_Rect drect;

    srect.x = 0;
    srect.y = 0;
    srect.w = pdisplaygraphics->size_x;
    srect.h = pdisplaygraphics->size_y;

    drect.x = x;
    drect.y = y;
    drect.w = pdisplaygraphics->size_x;
    drect.h = pdisplaygraphics->size_y;

    if(p != PIECE_SPACE && p!= PIECE_CURSOR)
        SDL_BlitSurface(pdisplaygraphics->image[PIECE_SPACE][IMAGE_PIECE], &srect, screen_surface, &drect);

    SDL_BlitSurface(pdisplaygraphics->image[p][IMAGE_PIECE], &srect, screen_surface, &drect);

    if(redraw)
        screen_redraw(x, y, pdisplaygraphics->size_x, pdisplaygraphics->size_y);
}

int display_focus(struct level* plevel, int refocus)
{
    int ox, oy;
    int px, py;

    int maxx, maxy;

#ifdef XOR_COMPATIBILITY
    if(plevel->mode == MODE_XOR && options_xor_display)
    {
        ox = display_offset_pixels_x;
        oy = display_offset_pixels_y;

        display_start_x = plevel->view_x[plevel->player];
        display_start_y = plevel->view_y[plevel->player];

        display_end_x = display_start_x + 8;
        display_end_y = display_start_y + 8;

        display_offset_pixels_x = (screen_width - pdisplaygraphics->size_x * 8) / 2;
        display_offset_pixels_y = (screen_height - display_bar_pixels - pdisplaygraphics->size_y * 8) / 2;

        display_offset_pixels_x -= display_start_x * pdisplaygraphics->size_x;
        display_offset_pixels_y -= display_start_y * pdisplaygraphics->size_y;

        if(display_offset_pixels_x != ox || display_offset_pixels_y != oy)
            return 1;
        else
            return 0;
    }
#endif

    px = plevel->player_x[plevel->player] * pdisplaygraphics->size_x;
    py = plevel->player_y[plevel->player] * pdisplaygraphics->size_y;
    ox = display_offset_pixels_x;
    oy = display_offset_pixels_y;

    display_border_x = pdisplaygraphics->size_x * 3;
    display_border_y = pdisplaygraphics->size_y * 3;

    maxx = (plevel->size_x * pdisplaygraphics->size_x - screen_width);
    maxy = (plevel->size_y * pdisplaygraphics->size_y - screen_height + display_bar_pixels);

    if((plevel->size_x - 1) * pdisplaygraphics->size_x < screen_width)
    {
        display_offset_pixels_x = (screen_width - (plevel->size_x * pdisplaygraphics->size_x)) / 2;
    }
    else
    {
        if(refocus)
        {
            if(px < -(display_offset_pixels_x - display_border_x))
                display_offset_pixels_x = -(px - display_border_x);
            if(px >= -(display_offset_pixels_x - screen_width + display_border_x + pdisplaygraphics->size_x))
                display_offset_pixels_x = -(px - screen_width + display_border_x + pdisplaygraphics->size_x);
        }
        
        if(display_offset_pixels_x > 0)
            display_offset_pixels_x = 0;
        if(display_offset_pixels_x < -maxx)
            display_offset_pixels_x = -maxx;

    }

    if((plevel->size_y - 1) * pdisplaygraphics->size_y < screen_height)
    {
        display_offset_pixels_y = (screen_height - display_bar_pixels - (plevel->size_y * pdisplaygraphics->size_y)) / 2;
    }
    else
    {
        if(refocus)
        {
            if(py < -(display_offset_pixels_y - display_border_y))
                display_offset_pixels_y = -(py - display_border_y);
            if(py >= -(display_offset_pixels_y - screen_height + display_bar_pixels + display_border_y + pdisplaygraphics->size_y))
                display_offset_pixels_y = -(py - screen_height + display_bar_pixels + display_border_y + pdisplaygraphics->size_y);
        }

        if(display_offset_pixels_y > 0)
            display_offset_pixels_y = 0;
        if(display_offset_pixels_y < -maxy)
            display_offset_pixels_y = -maxy;
    }

    /* Calculate start and end points */
    display_start_x = -display_offset_pixels_x / pdisplaygraphics->size_x;
    display_end_x = (-display_offset_pixels_x + screen_width + pdisplaygraphics->size_x - 1) / pdisplaygraphics->size_x;
    if(display_offset_pixels_x > 0)
        display_start_x --;

    display_start_y = -display_offset_pixels_y / pdisplaygraphics->size_y;
    display_end_y = (-display_offset_pixels_y + screen_height + pdisplaygraphics->size_y - display_bar_pixels - 1) / pdisplaygraphics->size_y;
    if(display_offset_pixels_y > 0)
        display_start_y --;

    if(pdisplaygraphics->flags & GRAPHICS_BACKGROUND)
    {
        if(display_start_x < 0)
            display_start_x = 0;
        if(display_end_x > plevel->size_x)
            display_end_x = plevel->size_x;
        if(display_start_y < 0)
            display_start_y = 0;
        if(display_end_y > plevel->size_y)
            display_end_y = plevel->size_y;
    }

    if(display_offset_pixels_x != ox || display_offset_pixels_y != oy)
        return 1;
    else
        return 0;
}

#ifdef XOR_COMPATIBILITY
void display_map_piece(struct level* plevel, int p, int x, int y, int redraw)
{
    SDL_Rect rect;

    rect.x = xor_map_x_offset + x * xor_map_scale;
    rect.y = xor_map_x_offset + y * xor_map_scale;
    rect.w = xor_map_scale;
    rect.h = xor_map_scale;

    if(plevel->player != 2)
    {
    if(x < plevel->size_x / 2 && y < plevel->size_y / 2 && !(plevel->mapped & MAPPED_TOP_LEFT))
	p = PIECE_UNKNOWN;
    if(x >= plevel->size_x / 2 && y < plevel->size_y / 2 && !(plevel->mapped & MAPPED_TOP_RIGHT))
	p = PIECE_UNKNOWN;
    if(x < plevel->size_x / 2 && y >= plevel->size_y / 2 && !(plevel->mapped & MAPPED_BOTTOM_LEFT))
	p = PIECE_UNKNOWN;
    if(x >= plevel->size_x / 2 && y >= plevel->size_y / 2 && !(plevel->mapped & MAPPED_BOTTOM_RIGHT))
	p = PIECE_UNKNOWN;
    }

    switch(p)
    {
	case PIECE_UNKNOWN:
	    SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, 0x00, 0x00, 0x00));
	    break;

	case PIECE_WALL:
	    SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, 0x7f, 0x7f, 0x7f));
	    break;

	case PIECE_STAR:
	    SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, 0xff, 0xa0, 0x00));
	    if(xor_map_scale > 2)
	    {
		rect.x ++;
		rect.y ++;
		rect.w -= 2;
		rect.h -= 2;
	        SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, 0xff, 0xff, 0x33));
	    }
	    break;

	case PIECE_DOOR:
	    SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, 0x00, 0x80, 0xff));
	    if(xor_map_scale > 2)
	    {
		rect.x ++;
		rect.y ++;
		rect.w -= 2;
		rect.h -= 2;
	        SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, 0x66, 0xb3, 0xff));
	    }
	    break;

	default:
	    SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, 0xff, 0xff, 0xff));
	    break;
    }

    if(redraw)
    {

    rect.x = xor_map_x_offset + x * xor_map_scale;
    rect.y = xor_map_x_offset + y * xor_map_scale;
    rect.w = xor_map_scale;
    rect.h = xor_map_scale;
        screen_redraw(rect.x, rect.y, rect.w, rect.h);
	}
}

void display_map(struct level* plevel)
{
    int i, j;

    xor_map_scale = screen_width / 256;
    if(xor_map_scale < 1)
        xor_map_scale = 1;

    display_clip(plevel, 0);

    for(j = 0; j < plevel->size_y; j ++)
    {
	for(i = 0; i < plevel->size_y; i ++)
	{
	    display_map_piece(plevel, level_piece(plevel, i, j), i, j, 0);
	}
    }

    /* Redraw map */
    screen_redraw(xor_map_x_offset, xor_map_y_offset, plevel->size_x * xor_map_scale, plevel->size_y * xor_map_scale);
}
#endif

void display_level(struct level* plevel, int redraw)
{
    int x, y;
    int p;
    SDL_Rect rect;


    display_focus(plevel, 0);

   if(redraw == SCREENREDRAW_ALL && (
	  (pdisplaygraphics->flags & GRAPHICS_BACKGROUND)
#ifdef XOR_COMPATIBILITY
	   || (plevel->mode == MODE_XOR && options_xor_display)
#endif
	  ))
    { 
        rect.x = 0;
        rect.y = 0;
        rect.w = screen_width;
        rect.h = screen_height - display_bar_pixels;

#ifdef XOR_COMPATIBILITY
        if(plevel->switched)
            SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, 0, 0, 0));
        else
#endif
            SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, pdisplaygraphics->background[0], pdisplaygraphics->background[1], pdisplaygraphics->background[2]));
    }

    if(pdisplaygraphics->shadows != NULL)
    {
        displayshadowed_level(plevel);
#ifdef XOR_COMPATIBILITY
    if(plevel->mode == MODE_XOR && options_xor_display)
        display_map(plevel);
#endif
        return;
    }

    display_clip(plevel, 1);

    for(y = display_start_y; y < display_end_y; y ++)
    {
        for(x = display_start_x; x < display_end_x; x ++)
        {
            p = level_piece(plevel, x, y);

            /* Moving pieces will be redrawn shortly */
            if(level_moving(plevel, x, y) != MOVE_NONE)
                p = level_previous(plevel, x, y); 

#ifdef XOR_COMPATIBILITY
            if(plevel->switched && (p == PIECE_WALL || p == PIECE_SPACE))
                p = PIECE_DARKNESS;
#endif

            /* If the piece is transparent, plot a background */
            if(pdisplaygraphics->image[p][IMAGE_PIECE]->flags & SDL_SRCALPHA)
            {
#ifdef XOR_COMPATIBILITY
                if(plevel->switched)
                    display_piece(plevel, PIECE_DARKNESS, x, y, MOVE_NONE);
                else
#endif
                    display_piece(plevel, PIECE_SPACE, x, y, MOVE_NONE);
            }

            /* Plot the piece itself */
            display_piece(plevel, p, x, y, MOVE_NONE);
        }
    }

    display_clip(plevel, 0);

    if(plevel->mover_first != NULL)
        display_movers(plevel, 0);

#ifdef XOR_COMPATIBILITY
    if(plevel->mode == MODE_XOR && options_xor_display)
        display_map(plevel);
#endif

    screen_redraw(0, 0, screen_width, screen_height);

}

void display_title(struct level* plevel)
{
    SDL_Surface *psurface;
    SDL_Surface *psurfacetest;
    SDL_Rect drect;
    int w;

    if(options_debug & DEBUG_SPEED)
        return;

    if(plevel->title != NULL)
    {
        if((strncmp(gettext(plevel->title), "chroma", 6) == 0))
            psurface = font_render(gettext(plevel->title), -8);
        else
            psurface = font_render(plevel->title, COLOUR_WHITE);

        if(plevel->flags & LEVELFLAG_TESTING)
        {
            psurfacetest = font_render(gettext("testing: "), COLOUR_CYAN);
            w = psurfacetest->w;

            drect.x = (screen_width - psurface->w - w) / 2;
            drect.y = screen_height - font_height;;
            drect.w = psurfacetest->w;;
            drect.h = psurfacetest->h;
            SDL_FillRect(screen_surface, &drect, SDL_MapRGB(screen_surface->format, 0, 0, 0));
            SDL_BlitSurface(psurfacetest, NULL, screen_surface, &drect);
            //SDL_UpdateRects(screen_surface, 1, &drect);
						flip_it();
            SDL_FreeSurface(psurfacetest);
        }
        else
            w = 0;

        drect.x = ((screen_width - psurface->w - w) / 2) + w;
        drect.y = screen_height - font_height;
        drect.w = psurface->w;
        drect.h = psurface->h;
        SDL_FillRect(screen_surface, &drect, SDL_MapRGB(screen_surface->format, 0, 0, 0));
        SDL_BlitSurface(psurface, NULL, screen_surface, &drect);
        //SDL_UpdateRects(screen_surface, 1, &drect);
				flip_it();
        SDL_FreeSurface(psurface);
    }
}

void display_moves(struct level* plevel, struct level* plevelreplay)
{
    static int length = 0;
    char buffer[256];
    SDL_Surface *psurface;
    SDL_Surface *pimage;
    SDL_Rect srect, drect;
    int w;
    int moves, moves2;

    moves = 0;
    if(plevel->move_current != NULL)
        moves = plevel->move_current->count;

    moves2 = -1;
    if(plevelreplay != NULL)
    {
        moves2 = 0;
        if(plevelreplay->move_last != NULL)
            moves2 = plevelreplay->move_last->count;
    }
    else if(plevel->move_current != plevel->move_last)
    {
        if(plevel->move_last != NULL)
            moves2 = plevel->move_last->count;
    }

    if(moves2 != -1)
        sprintf(buffer, "%s%d/%d",
                plevel->flags & LEVELFLAG_PAUSED ? gettext("paused ") :
                plevelreplay != NULL ? gettext("replay ") : "",
                moves, moves2);
    else
        sprintf(buffer, "%s%d",
                plevel->flags & LEVELFLAG_PAUSED ? gettext("paused ") : "",
                moves);

    if(plevel->flags & LEVELFLAG_FAILED)
        sprintf(buffer, gettext("failed"));

    psurface = font_render(buffer, COLOUR_CYAN);

    pimage = pdisplaygraphics->image[PIECE_PLAYER_ONE + plevel->player][IMAGE_SMALL];
    if(pimage != NULL)
        w = pdisplaygraphics->small_size_x;
    else
        w = 0;

    drect.w = length > (psurface->w + w) ? length : (psurface->w + w);
    drect.h = psurface->h;
    drect.x = screen_width - drect.w;
    drect.y = screen_height - font_height;
    SDL_FillRect(screen_surface, &drect, SDL_MapRGB(screen_surface->format, 0, 0, 0));
    drect.x = screen_width - w - psurface->w;
    SDL_BlitSurface(psurface, NULL, screen_surface, &drect);

    if(pimage != NULL)
    {
        srect.x = 0;
        srect.y = 0;
        srect.w = pdisplaygraphics->small_size_x;
        srect.h = pdisplaygraphics->small_size_y;

        /* If there is a second small image, use it for a dead player */
        if(plevel->alive[plevel->player] == 0 && pimage->w > pdisplaygraphics->small_size_x)
            srect.x += pdisplaygraphics->small_size_x;

        drect.x = screen_width - w;
        if(pimage->h < font_height)
            drect.y += (font_height - pimage->h) / 2;

        SDL_BlitSurface(pimage, &srect, screen_surface, &drect);
    }

    drect.w = (length > psurface->w ? length : psurface->w) + w;
    drect.h = psurface->h;
    drect.x = screen_width - drect.w;
    drect.y = screen_height - font_height;
    //SDL_UpdateRects(screen_surface, 1, &drect);
		flip_it();
    length = psurface->w + w;
    SDL_FreeSurface(psurface);
}

void display_stars(struct level* plevel)
{
    static int length = 0;
    char buffer[256];

    SDL_Surface *psurface;
    SDL_Rect srect, drect;
    SDL_Surface *pimage;
    int w;
    int p;

    sprintf(buffer, "%d/%d", plevel->stars_caught, plevel->stars_total);

    if(plevel->stars_exploded != 0)
        sprintf(buffer, gettext("%d lost"), plevel->stars_exploded);

    if(plevel->flags & LEVELFLAG_SOLVED && !(plevel->flags & LEVELFLAG_FAILED))
        sprintf(buffer, gettext("solved"));

    psurface = font_render(buffer, COLOUR_YELLOW);

    /* If solved, and there is a small door, use that */
    if(plevel->flags & LEVELFLAG_SOLVED && !(plevel->flags & LEVELFLAG_FAILED) && pdisplaygraphics->image[PIECE_DOOR][IMAGE_SMALL] != NULL)
        p = PIECE_DOOR;
    /* otherwise use a small star */
    else
        p = PIECE_STAR;

    pimage = pdisplaygraphics->image[p][IMAGE_SMALL];

    if(pimage != NULL)
        w = pdisplaygraphics->small_size_x;
    else
        w = 0;

    drect.w = length > psurface->w ? length : psurface->w + w;
    drect.h = psurface->h;
    drect.x = 0;
    drect.y = screen_height - font_height;

    SDL_FillRect(screen_surface, &drect, SDL_MapRGB(screen_surface->format, 0, 0, 0));
    drect.x = w;
    SDL_BlitSurface(psurface, NULL, screen_surface, &drect);

    if(pimage != NULL)
    {
        srect.x = 0;
        srect.y = 0;
        srect.w = pdisplaygraphics->small_size_x;
        srect.h = pdisplaygraphics->small_size_y;

        drect.x = 0;
        if(pimage->h < font_height)
            drect.y += (font_height - pimage->h) / 2;

        SDL_BlitSurface(pimage, NULL, screen_surface, &drect);
    }

    drect.w = (length > psurface->w ? length : psurface->w) + w;
    drect.h = psurface->h;
    drect.x = 0;
    drect.y = screen_height - font_height;
    //SDL_UpdateRects(screen_surface, 1, &drect);
		flip_it();
    length = psurface->w + w;
    SDL_FreeSurface(psurface);
}

int display_bevelsquare(struct level* plevel, int x, int y)
{
    int bevel;

    bevel = 0;

    if(level_piece(plevel, x, y) == PIECE_WALL)
    {
        if(level_piece(plevel, x - 1, y) != PIECE_WALL)
            bevel |= BEVEL_L;
        if(level_piece(plevel, x + 1, y) != PIECE_WALL)
            bevel |= BEVEL_R;
        if(level_piece(plevel, x, y - 1) != PIECE_WALL)
            bevel |= BEVEL_U;
        if(level_piece(plevel, x, y + 1) != PIECE_WALL)
            bevel |= BEVEL_D;

        if(((bevel & (BEVEL_L | BEVEL_U)) == 0) && level_piece(plevel, x - 1, y - 1) != PIECE_WALL)
            bevel |= BEVEL_TL;
        if(((bevel & (BEVEL_R | BEVEL_U)) == 0) && level_piece(plevel, x + 1, y - 1) != PIECE_WALL)
            bevel |= BEVEL_TR;
        if(((bevel & (BEVEL_L | BEVEL_D)) == 0) && level_piece(plevel, x - 1, y + 1) != PIECE_WALL)
            bevel |= BEVEL_BL;
        if(((bevel & (BEVEL_R | BEVEL_D)) == 0) && level_piece(plevel, x + 1, y + 1) != PIECE_WALL)
            bevel |= BEVEL_BR;
    }
    else
    {
        if(level_piece(plevel, x - 1, y) == PIECE_WALL)
            bevel |= BEVEL_L;
        if(level_piece(plevel, x + 1, y) == PIECE_WALL)
            bevel |= BEVEL_R;
        if(level_piece(plevel, x, y - 1) == PIECE_WALL)
            bevel |= BEVEL_U;
        if(level_piece(plevel, x, y + 1) == PIECE_WALL)
            bevel |= BEVEL_D;

        if(((bevel & (BEVEL_L | BEVEL_U)) == 0) && level_piece(plevel, x - 1, y - 1) == PIECE_WALL)
            bevel |= BEVEL_TL;
        if(((bevel & (BEVEL_R | BEVEL_U)) == 0) && level_piece(plevel, x + 1, y - 1) == PIECE_WALL)
            bevel |= BEVEL_TR;
        if(((bevel & (BEVEL_L | BEVEL_D)) == 0) && level_piece(plevel, x - 1, y + 1) == PIECE_WALL)
            bevel |= BEVEL_BL;
        if(((bevel & (BEVEL_R | BEVEL_D)) == 0) && level_piece(plevel, x + 1, y + 1) == PIECE_WALL)
            bevel |= BEVEL_BR;
    }

    return bevel;
}

void display_bevellevel(struct level* plevel)
{
    int x, y;
    int bevel;

    for(x = 0; x < plevel->size_x; x ++)
    {
        for(y = 0; y < plevel->size_y; y ++)
        {
            bevel = level_data(plevel, x, y) & ~BEVEL_ALL;
            bevel = bevel | display_bevelsquare(plevel, x, y);
            level_setdata(plevel, x, y, bevel);
        }
    }
}


void display_movers(struct level* plevel, int redraw)
{
    struct mover* pmover;
    int x, y, p, pm;
    int i, j;
    char buffer[16];
    int bevel;
    struct SDL_Surface *psurface;
    SDL_Rect srect, drect;

    if(pdisplaygraphics->shadows != NULL)
    {
        displayshadowed_movers(plevel, redraw);
        return;
    }

    display_clip(plevel, 1);

    display_animation_x = - pdisplaygraphics->size_x + (int)((float) pdisplaygraphics->size_x * display_animation);
    display_animation_y = - pdisplaygraphics->size_y + (int)((float) pdisplaygraphics->size_y * display_animation);

    /* First, plot spaces for all moving pieces */
    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
#ifdef XOR_COMPATIBILITY
        if(plevel->switched)
            display_piece(plevel, PIECE_DARKNESS, pmover->x, pmover->y, MOVE_NONE);
        else
#endif
            display_piece(plevel, PIECE_SPACE, pmover->x, pmover->y, MOVE_NONE);

        pmover = pmover->next;
    }

    /* Plot moving piece */
    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
        x = pmover->x;
        y = pmover->y;

        if(isexplosion(pmover->piece))
        {
            /* Plot any piece destroyed by the explosion, or the bomb itself */
            p = level_previous(plevel, x, y);
            pm = level_previousmoving(plevel, x, y);
            if(p != PIECE_SPACE)
                display_piece(plevel, p, x, y, pm);

            /* Plot the detonator */
            p = level_detonator(plevel, x, y);
            pm = level_detonatormoving(plevel, x, y);
            if(p != PIECE_SPACE)
                display_piece(plevel, p, x, y, pm);
        }
        /* Spaces have already been covered */
        else if(pmover->piece != PIECE_SPACE && pmover->piece != PIECE_GONE)
        {
            if(display_animation < 1)
            {
                /* Pieces being collected, earth being eaten */
                p = level_previous(plevel, x, y);
                pm = level_previousmoving(plevel, x, y);
                if((p != PIECE_SPACE && !isexplosion(p) && pm == MOVE_NONE)
#ifdef XOR_COMPATIBILITY
                        || pmover->piece == PIECE_TELEPORT
#endif
                  )
                    display_piece(plevel, p, x, y, pm);
            }

            display_piece(plevel, pmover->piece, x, y, pmover->direction);
        }

        pmover = pmover->next;
    }

    /* Plot explosions */
    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
        x = pmover->x;
        y = pmover->y;
        /* Plot growing explosion */
        if(isexplosion(pmover->piece))
            display_piece(plevel, pmover->piece + PIECE_EXPLOSION_NEW_FIRST - PIECE_EXPLOSION_FIRST, x, y, MOVE_NONE);

        /* Plot dying explosion */
        p = level_previous(plevel, x, y);
        if(isexplosion(p) && display_animation < 1)
            display_piece(plevel, p, x, y, MOVE_NONE);

        pmover = pmover->next;
    }

    /* Plot order of movers if debugging (but not if editing) */
    if(options_debug & DEBUG_ORDER && display_animation < 1 && plevel->player != 2)
    {
        pmover = plevel->mover_first;
        i = 0;
        while(pmover != NULL)
        {
            if(pmover->piece != PIECE_SPACE && pmover->piece != PIECE_GONE)
            {
                pm = pmover->direction;
                if(isexplosion(pmover->piece) || isnewexplosion(pmover->piece))
                    pm = MOVE_NONE;
    
                x = pmover->x * pdisplaygraphics->size_x + display_offset_pixels_x + ((-1 + display_animation) * move_x[pm] * pdisplaygraphics->size_x);
                y = pmover->y * pdisplaygraphics->size_y + display_offset_pixels_y + ((-1 + display_animation) * move_y[pm] * pdisplaygraphics->size_y);
    
                sprintf(buffer, "%X", i++);
                switch(pmover->direction)
                {
                    case MOVE_UP:
                        strcat(buffer, ARROW_UP);
                        break;
                    case MOVE_DOWN:
                        strcat(buffer, ARROW_DOWN);
                        break;
                    case MOVE_LEFT:
                        strcat(buffer, ARROW_LEFT);
                        break;
                    case MOVE_RIGHT:
                        strcat(buffer, ARROW_RIGHT);
                        break;
                    default:
                        break;
                }
                psurface = font_render(buffer, COLOUR_WHITE | COLOUR_BOLD);
                srect.w = psurface->w > pdisplaygraphics->size_x ? pdisplaygraphics->size_x : psurface->w;
                srect.h = psurface->h > pdisplaygraphics->size_y ? pdisplaygraphics->size_y : psurface->h;
                srect.x = psurface->w - srect.w;
                srect.y = 0;
                drect.x = x + pdisplaygraphics->size_x - srect.w;
                drect.y = y;
                SDL_BlitSurface(psurface, &srect, screen_surface, &drect);
                SDL_FreeSurface(psurface);
            }
            else
                i ++;

            pmover = pmover->next;
        }
    }

    if(redraw == 0)
        return;

    /* Redraw screen */
    pmover = plevel->mover_first;

    while(pmover != NULL)
    {
        x = pmover->x;
        y = pmover->y;
        display_redrawpiece(pmover->piece, x, y, pmover->direction);

        if(isexplosion(pmover->piece))
        {
            p = level_previous(plevel, x, y);
            pm = level_previousmoving(plevel, x, y);
            if(pm != MOVE_NONE && p != PIECE_SPACE)
                display_redrawpiece(p, x, y, pm);

            p = level_detonator(plevel, x, y);
            pm = level_detonatormoving(plevel, x, y);
            if(pm != MOVE_NONE && p != PIECE_SPACE)
                display_redrawpiece(p, x, y, pm);
        }

        p = level_previous(plevel, x, y);
        if(isexplosion(p))
            display_redrawpiece(p, x, y, MOVE_NONE);

        pmover = pmover->next;
    }

    /* At the peak of the explosion, rebevel any walls that have been
       destroyed. When undoing, rebevel any walls that have been recreated. */
    if(display_animation == 0 || display_animation == 1)
    {
        /* When undoing, we have to create the wall prior to rebevelling, as it
           wouldn't otherwise exist until after the end of the animation. */
        pmover = plevel->mover_first;
        while(pmover != NULL)
        {
            if(pmover->piece == PIECE_WALL)
                level_setpiece(plevel, pmover->x, pmover->y, pmover->piece);
            pmover = pmover->next;
        }

        pmover = plevel->mover_first;
        while(pmover != NULL)
        {
            x = pmover->x;
            y = pmover->y;
            if(pmover->piece == PIECE_WALL ||
                (isexplosion(pmover->piece) && display_animation == 1))
            {
                for(i = -1; i < 2; i ++)
                {
                    for(j = - 1; j < 2; j ++)
                    {
                        bevel = display_bevelsquare(plevel, x + i, y + j);
                        if(bevel != (level_data(plevel, x + i, y + j) & BEVEL_ALL))
                        {
                            level_setdata(plevel, x + i, y + j, bevel | (level_data(plevel, x + i, y + j) & ~BEVEL_ALL));
                            p = level_piece(plevel, x + i, y + j);
                            if(p == PIECE_WALL)
                            {
#ifdef XOR_COMPATIBILITY
                                if(plevel->switched)
                                    display_piece(plevel, PIECE_DARKNESS, x + i, y + j, MOVE_NONE);
                                else
#endif
                                    display_piece(plevel, PIECE_WALL, x + i, y + j, MOVE_NONE);
                            }
                            else
                            {  
#ifdef XOR_COMPATIBILITY
                                if(plevel->switched)
                                    display_piece(plevel, PIECE_DARKNESS, x + i, y + j, MOVE_NONE);
                                else
#endif
                                    display_piece(plevel, PIECE_SPACE, x + i, y + j, MOVE_NONE);
                                /* Moving pieces will be replotted when they next move. */
                                if(p != PIECE_SPACE && level_moving(plevel, x + i, y + j) == MOVE_NONE)
                                    display_piece(plevel, p, x + i, y + j, MOVE_NONE);
                            }
                            display_redrawpiece(p, x + i, y + j, MOVE_NONE);
                        }
                    }
                }
            }
            pmover = pmover->next;
        }
    }

    display_clip(plevel, 0);
}

void display_play(struct level* plevel, struct level* plevelreplay)
{
    SDL_Event event;
    SDL_Surface *psurface;
    SDL_Rect drect;
    int quit;
    int redraw;
    int playermove;
    struct mover* pmover;
    Uint32 basetime;
    Uint32 nowtime;
    Uint32 pausetime = 0;
    int delay, delayold;
    int keymod;
    int frames = 0;
    int events;
    char buffer[256];
    int action;
    int fast;

    int mouse_x = 0;
    int mouse_y = 0;
    int mouse_destination_x = 0;
    int mouse_destination_y = 0;
    int mouse_button = 0;
    int mouse_time;

    int swap = 0;

    graphics_reload();

    display_bevellevel(plevel);

    /* Force full redraw */
    redraw = SCREENREDRAW_ALL;

    playermove = MOVE_NONE;
    action = ACTION_NONE;
    delayold = options_sdl_delay;

    mouse_time = SDL_GetTicks();

    /* Force all animation to end */
    basetime = SDL_GetTicks() - delayold;

    keymod = 0;
    fast = 0;

    plevel->flags &= ~LEVELFLAG_PAUSED;

    font_set_size(font_size_game);
    display_bar_pixels = font_height;
    display_focus(plevel, 1);

    quit = 0;
    while(!quit)
    {
	//////// Screen redraw ////////

        if(redraw & SCREENREDRAW_ALL)
        {
	    font_set_size(font_size_game);
            display_bar_pixels = font_height;
        }
        if(redraw & SCREENREDRAW_BAR)
        {
            /* Clear bar */
            drect.x = 0;
            drect.y = screen_height - display_bar_pixels;
            drect.w = screen_width;
            drect.h = display_bar_pixels;
            SDL_FillRect(screen_surface, &drect, SDL_MapRGB(screen_surface->format, 0, 0, 0));

            display_title(plevel);
            display_stars(plevel);
            display_moves(plevel, plevelreplay);
        }
        if(redraw & SCREENREDRAW_LEVEL)
            display_level(plevel, redraw);

        redraw = 0;
	
	//////// Delay calculation ////////

        /* Calculate what the delay should be, defaulting to the Move Speed. */
        delay = options_sdl_delay;
        /* If we're replaying, use the Replay Speed */
        if(plevelreplay != NULL && plevelreplay->moves != -1)
            delay = options_sdl_replay_delay;
        else
        {
	    /* Otherwise */
            pmover = plevel->mover_first;
            while(pmover != NULL)
            {
                /* Use the Player Speed if the player is still moving */
                if(pmover->piece == PIECE_PLAYER_ONE || pmover->piece == PIECE_PLAYER_TWO)
                    delay = options_sdl_player_delay;
                /* unless there's a piece following in their trail */
                else if(pmover->piece != PIECE_SPACE && pmover->fast == 1)
                    delay = options_sdl_delay;

                pmover = pmover->next;
            }
        }
        /* If we're undoing, use the Undo Speed */
        if(plevel->flags & LEVELFLAG_UNDO)
            delay = options_sdl_undo_delay;
        /* If SHIFT is pressed, speed things up. */
        if(keymod & 1)
	    delay = delay / 10;
        /* If CTRL is pressed, slow things down. */
        if(keymod & 2)
            delay = delay * 4;
        /* If the delay has changed, preserve our position in the animation */
        if(delay != delayold)
        {
            nowtime = SDL_GetTicks();
            if((plevel->flags & LEVELFLAG_PAUSED))
                nowtime = pausetime;
            if(delayold != 0)
                basetime = nowtime - (((nowtime - basetime) * delay) / delayold );
            delayold = delay;
        }
        if(fast && !(plevel->flags & LEVELFLAG_PAUSED))
            basetime = 0;

	//////// Movers ////////

        /* If there are movers, plot and then evolve them */
        if(plevel->mover_first != NULL)
        {
            nowtime = SDL_GetTicks();
            if((plevel->flags & LEVELFLAG_PAUSED))
                nowtime = pausetime;
            display_animation = (float)(nowtime - basetime) / delay;
            if(display_animation > 1)
                display_animation = 1;
            frames ++;
            display_movers(plevel, 1);
        }

        if(plevel->mover_first != NULL)
        {
            /* Is it time for the next stage of this move? */
            if(nowtime > basetime + delay && !(plevel->flags & LEVELFLAG_PAUSED))
            {
#ifdef XOR_COMPATIBILITY
		if(plevel->mode == MODE_XOR && options_xor_display)
		{
		    pmover = plevel->mover_first;
		    while(pmover != NULL)
		    {
			display_map_piece(plevel, pmover->piece, pmover->x, pmover->y, 1);
		        pmover = pmover->next;
		    }
		}
#endif

                /* Evolve movers */
                if(!(plevel->flags & LEVELFLAG_UNDO))
                {
                    if(level_evolve(plevel))
                        redraw |= display_focus(plevel, 1) * SCREENREDRAW_LEVEL;
                    level_storemovers(plevel);

                    if(options_debug & DEBUG_SPEED)
                    {
                        sprintf(buffer, "    %4dfps (%d frames / %d ms)    ", 1000 * frames / (nowtime - basetime), frames, nowtime - basetime);
                        psurface = font_render(buffer, COLOUR_WHITE);
                        drect.x = (screen_width - psurface->w) / 2;
                        drect.y = screen_height - font_height;
                        drect.w = psurface->w;
                        drect.h = psurface->h;
                        SDL_FillRect(screen_surface, &drect, SDL_MapRGB(screen_surface->format, 0, 0, 0));
                        SDL_BlitSurface(psurface, NULL, screen_surface, &drect);
                        //SDL_UpdateRects(screen_surface, 1, &drect);
												flip_it();
                        SDL_FreeSurface(psurface);

                        printf("%s\n", buffer);
                    }
                    frames = 0;
                }
                else
                {
                    if(level_undo(plevel))
                        plevel->flags |= LEVELFLAG_UNDO;
                    else
                        plevel->flags &= ~LEVELFLAG_UNDO;

                        /* Refocus in case we've moved offscreen */
                        redraw |= display_focus(plevel, 1) * SCREENREDRAW_LEVEL;
                }
                
                basetime = SDL_GetTicks();

                /* Reset animation in case we redraw before it is next calculated */
                display_animation = 0;
            }
        }

	//////// Events ////////

	/* Hide the mouse if not used whilst in full screen mode */
	if(SDL_GetTicks() > mouse_time + MOUSE_TIMEOUT_MOVE)
	    screen_cursor(0);

        /* Poll if there are movers, otherwise wait so as to reduce load */
        if(plevel->mover_first != NULL || SDL_GetTicks() < basetime + delay)
            events = SDL_PollEvent(&event);
        else
            events = SDL_WaitEvent(&event);

        while(events)
        {
            switch(event.type)
            {
                case SDL_KEYDOWN:
    
                    switch(actions[event.key.keysym.sym])
                    {   
    		        case ACTION_FASTER:
                            keymod |= 1;
                            break;
    
    		        case ACTION_SLOWER:
                            keymod |= 2;
                            break;
    
    		        case ACTION_QUIT:
                            quit = 1;
                            break;
    
    		        case ACTION_FAST:
			    fast = 1;
                            break;
    
    		        case ACTION_PAUSE:
                            if(plevel->flags & LEVELFLAG_PAUSED)
                            {
                                plevel->flags &= ~LEVELFLAG_PAUSED;
                                basetime = SDL_GetTicks() - (pausetime - basetime);
                            }
                            else
                            {
                                plevel->flags |= LEVELFLAG_PAUSED;
                                pausetime = SDL_GetTicks();
                            }
                            redraw |= SCREENREDRAW_BAR;
                            break;
    
    		        case ACTION_REDRAW:
                            redraw |= SCREENREDRAW_ALL;
                            break;
    
    		        case ACTION_HIDE:
    			    SDL_WM_IconifyWindow();
    			    break;
    
    		        case ACTION_UNDO:
                            if(plevelreplay != NULL)
                            {
                                plevelreplay->flags |= LEVELFLAG_UNDO;
                                plevelreplay->flags &= ~LEVELFLAG_PAUSED;
                            }
			    else
				action = ACTION_UNDO;
                            break;
    
    
        		case ACTION_REDO:
                            if(plevelreplay!= NULL)
                            {
                                plevelreplay->flags &= ~LEVELFLAG_UNDO;
                                plevelreplay->flags &= ~LEVELFLAG_PAUSED;
                            }
			    else
				action = ACTION_REDO;
                            break;
    
                        case ACTION_LEFT:
                            if(plevelreplay != NULL)
                            {
                                plevelreplay->flags |= LEVELFLAG_UNDO;
                                plevelreplay->flags &= ~LEVELFLAG_PAUSED;
                            }
                            else
                                action = ACTION_LEFT;
                            break;
    
                        case ACTION_RIGHT:
                            if(plevelreplay != NULL)
                            {
                                plevelreplay->flags &= ~LEVELFLAG_UNDO;
                                plevelreplay->flags &= ~LEVELFLAG_PAUSED;
                            }
                            else
                                action = ACTION_RIGHT;
                            break;
    
                        case ACTION_UP:
                            if(plevelreplay != NULL)
                                plevelreplay->flags |= LEVELFLAG_PAUSED;
                            else
                                action = ACTION_UP;
                            break;
    
                        case ACTION_DOWN:
                            if(plevelreplay != NULL)
                                plevelreplay->flags |= LEVELFLAG_PAUSED;
                            else
                                action = ACTION_DOWN;
                            break;
    
    		        case ACTION_SWAP:
                            action = ACTION_SWAP;
                            break;
    
                        default:
                            break;
                    }
                    break;
    
                case SDL_KEYUP:
                    switch(actions[event.key.keysym.sym])
                    {   
                        case ACTION_SWAP:
                            swap = 0;
                        case ACTION_UP:
                        case ACTION_DOWN:
                        case ACTION_LEFT:
                        case ACTION_RIGHT:
			case ACTION_UNDO:
			case ACTION_REDO:
                            action = ACTION_NONE;
                            break;
    
       		        case ACTION_FASTER:
                            keymod &= ~1;
                            break;
    
    		        case ACTION_SLOWER:
                            keymod &= ~2;
                            break;

    		        case ACTION_FAST:
                            fast = 0;
                            break;
    
                        default:
                            break;
                    }
                    break;
    
                case SDL_QUIT:
                    exit(0);
    
                case SDL_VIDEORESIZE:
                    screen_resizeevent(&event);
                    redraw = SCREENREDRAW_ALL;
                    break;

                case SDL_ACTIVEEVENT:
                    if((event.active.state & SDL_APPACTIVE) && event.active.gain == 1)
                        redraw = SCREENREDRAW_ALL;
                    break;
    
                case SDL_MOUSEBUTTONDOWN:
                    mouse_x = event.button.x;
                    mouse_y = event.button.y;
                    mouse_button = event.button.button;
                    mouse_time = SDL_GetTicks();
		    screen_cursor(1);

                    if(mouse_button > 0 && mouse_button < MOUSE_BUTTONS_MAX)
                        action = actions_mouse[ACTIONS_GAME][mouse_button];

                    if(action == ACTION_HIDE)
                    {
                        SDL_WM_IconifyWindow();
                        action = ACTION_NONE;
                    }
                    if(action == ACTION_QUIT)
                        quit = 1;
                    break;

                case SDL_MOUSEBUTTONUP:
                    if(action == ACTION_MOUSE_CLICK ||
                            (action == ACTION_MOUSE_DRAG_OR_CLICK && (SDL_GetTicks() < mouse_time + MOUSE_TIMEOUT_CLICK)))
                    {
                        mouse_destination_x = (mouse_x - display_offset_pixels_x) / pdisplaygraphics->size_x;
                        mouse_destination_y = (mouse_y - display_offset_pixels_y) / pdisplaygraphics->size_y;
                        if(mouse_destination_x == plevel->player_x[plevel->player])
                        {
                            if(mouse_destination_y < plevel->player_y[plevel->player])
                                action = ACTION_UP;
                            if(mouse_destination_y > plevel->player_y[plevel->player])
                                action = ACTION_DOWN;
                        }
                        if(mouse_destination_y == plevel->player_y[plevel->player])
                        {
                            if(mouse_destination_x < plevel->player_x[plevel->player])
                                action = ACTION_LEFT;
                            if(mouse_destination_x > plevel->player_x[plevel->player])
                                action = ACTION_RIGHT;
                        }
                        if(mouse_destination_x == plevel->player_x[1 - plevel->player] && mouse_destination_y == plevel->player_y[1 - plevel->player])
                            action = ACTION_SWAP;
                    }
                    /* Buttons 4 and 5 can't be held down */
                    else if(mouse_button != 4 && mouse_button != 5)
                    {
                        mouse_button = 0;
                        action = ACTION_NONE;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    /* Are we dragging? */
                    if(action == ACTION_MOUSE_DRAG || action == ACTION_MOUSE_DRAG_OR_CLICK)
                    {
                        display_offset_pixels_y -= (mouse_y - event.motion.y);
                        display_offset_pixels_x -= (mouse_x - event.motion.x);

			mouse_x = event.motion.x;
			mouse_y = event.motion.y;

                        redraw |= SCREENREDRAW_LEVEL;
                    }
		    else
		    {
			mouse_time = SDL_GetTicks();
			screen_cursor(1);
		    }
                    break;

                default:
                    break;
            }
            events = SDL_PollEvent(&event);
        }

	//////// Actions ////////

        /* Are we replaying the level? */
        if(plevelreplay != NULL)
        {
            /* Prevent the user from moving during the replay */
            playermove = MOVE_NONE;

            /* Is it time for another move? */
            if(plevel->mover_first == NULL && !(plevelreplay->flags & LEVELFLAG_PAUSED))
            {
                /* Moving backwards through replay */
                if(plevelreplay->flags & LEVELFLAG_UNDO)
                {
                    if(level_undo(plevel))
                    {
                        plevel->flags |= LEVELFLAG_UNDO;
                        if(plevelreplay->move_current != NULL)
                            plevelreplay->move_current = plevelreplay->move_current->previous;
                        else
                            plevelreplay->move_current = plevelreplay->move_last;
                    }
                    else
                        plevel->flags &= ~LEVELFLAG_UNDO;
                }
                /* Moving forwards through replay */
                else
                {
                    if(plevelreplay->move_current != NULL)
                    {
                        playermove = plevelreplay->move_current->direction;
                        plevelreplay->move_current = plevelreplay->move_current->next;
                    }
                }
            }
        }
	/* otherwise, see what action the user has asked for */
	else
	{
	    playermove = MOVE_NONE;

	    switch(action)
	    {
		case ACTION_LEFT:
		    playermove = MOVE_LEFT;
		    break;
		case ACTION_RIGHT:
		    playermove = MOVE_RIGHT;
		    break;
		case ACTION_UP:
		    playermove = MOVE_UP;
		    break;
		case ACTION_DOWN:
		    playermove = MOVE_DOWN;
		    break;
		case ACTION_SWAP:
                    /* Swap only once per keypress */
                    if(plevel->mover_first == NULL && !(plevel->flags & LEVELFLAG_PAUSED) && !swap)
                    {
  		        playermove = MOVE_SWAP;
                        swap = 1;
                    }
		    break;
		case ACTION_UNDO:
		    if(plevel->mover_first == NULL && !(plevel->flags & LEVELFLAG_UNDO))
		    {
			if(level_undo(plevel))
			    plevel->flags |= LEVELFLAG_UNDO;
			else
			    plevel->flags &= ~LEVELFLAG_UNDO;
			playermove = MOVE_NONE;
			basetime = SDL_GetTicks();

                        /* Refocus in case we've moved offscreen */
                        redraw |= display_focus(plevel, 1) * SCREENREDRAW_LEVEL;
		    }
		    break;
		case ACTION_REDO:
		    playermove = MOVE_REDO;
		    break;
		default:
		    break;
	    }
	}

        if(mouse_button == 4 || mouse_button == 5)
        {
            action = ACTION_NONE;
            mouse_button = 0;
        }

        /* Can't move if we've failed or solved the level */
        if(plevel->flags & (LEVELFLAG_FAILED | LEVELFLAG_SOLVED))
            playermove = MOVE_NONE;

        /* If we can move, make the move */
        if(playermove != MOVE_NONE && plevel->mover_first == NULL && !(plevel->flags & LEVELFLAG_PAUSED))
        {
	    level_move(plevel, playermove);
            basetime = SDL_GetTicks();
            redraw |= display_focus(plevel, 1) * SCREENREDRAW_LEVEL;

            if(mouse_destination_x != 0 || mouse_destination_y != 0)
            {
		/* Have we reached our destination, or been blocked? */
                if((plevel->player_x[plevel->player] == mouse_destination_x && plevel->player_y[plevel->player] == mouse_destination_y) || plevel->mover_first == NULL)
		{
                    action = ACTION_NONE;
		    mouse_destination_x = 0;
		    mouse_destination_y = 0;
		}
            }
        }

	//////// Display changes ////////

        if(plevel->flags & LEVELFLAG_MOVES)
        {
            display_moves(plevel, plevelreplay);
            plevel->flags &= ~LEVELFLAG_MOVES;
        }

        if(plevel->flags & LEVELFLAG_STARS)
        {
            display_stars(plevel);
            plevel->flags &= ~LEVELFLAG_STARS;
        }

#ifdef XOR_COMPATIBILITY
        if(plevel->flags & LEVELFLAG_SWITCH)
        {
            redraw |= SCREENREDRAW_ALL; 
            plevel->flags &= ~LEVELFLAG_SWITCH;
        }

	if(plevel->flags & LEVELFLAG_MAP)
	{
	    plevel->flags &= ~LEVELFLAG_MAP;
	    if(plevel->mode == MODE_XOR && options_xor_display)
	        display_map(plevel);
	}
#endif


        if(!(plevel->flags & LEVELFLAG_SOLVED) && plevel->flags & LEVELFLAG_EXIT)
        {
            redraw |= SCREENREDRAW_BAR;
            plevel->flags |= LEVELFLAG_SOLVED;
        }

        if(!(plevel->flags & LEVELFLAG_FAILED) && plevel->alive[0] == 0 && plevel->alive[1] ==0)
        {
            redraw |= SCREENREDRAW_BAR;
            plevel->flags |= LEVELFLAG_FAILED;
        }
    }

    screen_cursor(1);
}

void display_edit(struct level* plevel)
{
    int quit;

    struct mover* pmover;
    struct mover* pmovertmp;

    static int editor_piece = 0;
    int redraw = 0, predraw = 0, moved = 0, pmoved = 0;
    int i, j;
    int player;

    int ex, ey, ep;
    int x, y;
    int bevel;
    int piece_start = 0;
    int piece_end = 0;
    int piece_width = 0;
    int piece_count = 0;
    int p;
    int action;
    int effect;

    SDL_Event event;
    SDL_Rect drect;
    SDL_Surface *psurface;

    int mouse_x, mouse_y;
    int dx, dy;

    int mouse_button;
    int mouse_time;

    font_set_size(font_size_game);
    graphics_reload();

    display_bevellevel(plevel);

    piece_count = 0;
    while(editor_piece_maps[plevel->mode][piece_count] != PIECE_GONE)
        piece_count ++;

    if(editor_piece > piece_count)
        editor_piece = 0;

    /* The editor uses player 2, so store the player value */
    player = plevel->player;

    ex = plevel->player_x[2];
    ey = plevel->player_y[2];
    ep = editor_piece;

    mouse_x = 0;
    mouse_y = 0;
    mouse_button = 0;
    mouse_time = SDL_GetTicks();
    action = 0;
    effect = 0;

    /* Force a complete redraw */
    redraw = 2;

    display_focus(plevel, 1);

    quit = 0;
    while(!quit)
    {
        plevel->player_x[2] = plevel->player_x[2];
        plevel->player_y[2] = plevel->player_y[2];
        plevel->player = 2;

#ifdef XOR_COMPATIBILITY
        if(plevel->mode == MODE_XOR)
            xor_focus(plevel);
#endif

        /* Clear screen, and calculate size of bar */
        if(redraw == 2)
        {
            screen_clear(pdisplaygraphics->background[0], pdisplaygraphics->background[1], pdisplaygraphics->background[2]);

            piece_width = (screen_width - pdisplaygraphics->size_x - 2) / pdisplaygraphics->size_x;

            display_bar_pixels = pdisplaygraphics->size_y + 1;

            if(piece_width < piece_count)
            {
                if(display_bar_pixels < font_height)
                    display_bar_pixels = font_height;
                piece_width = (screen_width - pdisplaygraphics->size_x - 2) / pdisplaygraphics->size_x;
            }
            predraw = 2;
        }

        if(moved == 1)
            redraw |= display_focus(plevel, 1);
        
        /* Draw level */
        if(redraw)
        {
            display_level(plevel, SCREENREDRAW_ALL);

            redraw = 0;
            moved = 1;
        }

        /* Recalculate piece bar */
        if(pmoved || predraw)
        {
            if(piece_width >= piece_count)
            {
                piece_start =0;
                piece_end = piece_count;
            }
            else
            {
                while(editor_piece < piece_start + 3 && piece_start > 0)
                {
                    piece_start --;
                    piece_end = piece_start + piece_width;

                    predraw = 1;
                }

                while(editor_piece >= piece_start + piece_width - 3 && piece_end < piece_count)
                {
                    piece_start ++;
                    piece_end = piece_start + piece_width;

                    predraw = 1;
                }

                piece_end = piece_start + piece_width;
                if(piece_end > piece_count)
                    piece_end = piece_count;
            }
        }

        /* Redraw all of piece bar */
        if(predraw)
        {
            drect.x = 0;
            drect.w = screen_width;
            drect.y = screen_height - display_bar_pixels;
            drect.h = display_bar_pixels;
            SDL_FillRect(screen_surface, &drect, SDL_MapRGB(screen_surface->format, 0, 0, 0));

            drect.x = 0;
            drect.w = screen_width - pdisplaygraphics->size_x - 2;
            drect.y = screen_height - pdisplaygraphics->size_y;
            drect.h = pdisplaygraphics->size_y;
            SDL_SetClipRect(screen_surface, &drect);
            for(i = piece_start; i < piece_end; i ++)
            {
                display_pieceabsolute(editor_piece_maps[plevel->mode][i], (i - piece_start) * pdisplaygraphics->size_x, screen_height - pdisplaygraphics->size_y, 0);
            }
            if(piece_start != 0)
            {
                psurface = font_render(ARROW_LEFT, COLOUR_WHITE | COLOUR_BOLD);
                drect.x = 0;
                drect.y = screen_height - (pdisplaygraphics->size_y + font_height) / 2;
                SDL_BlitSurface(psurface, NULL, screen_surface, &drect);
                SDL_FreeSurface(psurface);
            }
            if(piece_end != piece_count)
            {
                psurface = font_render(ARROW_RIGHT, COLOUR_WHITE | COLOUR_BOLD);
                drect.x = piece_width * pdisplaygraphics->size_x - 2 - psurface->w;
                drect.y = screen_height - (pdisplaygraphics->size_y + font_height) / 2;
                SDL_BlitSurface(psurface, NULL, screen_surface, &drect);
                SDL_FreeSurface(psurface);
            }
            SDL_SetClipRect(screen_surface, NULL);

            drect.x = 0;
            drect.w = screen_width;
            drect.y = screen_height - pdisplaygraphics->size_y;
            drect.h = pdisplaygraphics->size_y;
            //SDL_UpdateRects(screen_surface, 1, &drect);
						flip_it();

            pmoved = 1;
        }

        /* Redraw piece bar cursor */
        if(pmoved)
        {
            pmoved = 0;

            /* Remove cursor from previous piece */
            if(!predraw)
                display_pieceabsolute(editor_piece_maps[plevel->mode][ep], (ep - piece_start) * pdisplaygraphics->size_x, screen_height - pdisplaygraphics->size_y, 1);

            /* Plot cursor */
            display_pieceabsolute(PIECE_CURSOR, (editor_piece - piece_start) * pdisplaygraphics->size_x, screen_height - pdisplaygraphics->size_y, predraw ? 0 : 1);
            
            /* Plot current piece in far right corner */
            display_pieceabsolute(editor_piece_maps[plevel->mode][editor_piece], screen_width - pdisplaygraphics->size_x, screen_height - pdisplaygraphics->size_y, predraw ? 0 : 1);
        }

        if(predraw)
        {
            drect.x = 0;
            drect.w = screen_width;
            drect.y = screen_height - display_bar_pixels;
            drect.h = display_bar_pixels;
            //SDL_UpdateRects(screen_surface, 1, &drect);
						flip_it();
            predraw = 0;
        }

        /* Redraw level cursor */
        if(moved)
        {
            moved = 0;

            /* Create cosmetic movers */
            level_setprevious(plevel, plevel->player_x[2], plevel->player_y[2], level_piece(plevel, plevel->player_x[2], plevel->player_y[2]));
            if(pdisplaygraphics->shadows != NULL)
                mover_newundo(plevel, ex, ey, MOVE_NONE, PIECE_SPACE, PIECE_SPACE, MOVER_UNDO); 
            else
                mover_newundo(plevel, ex, ey, MOVE_NONE, level_piece(plevel, ex, ey), PIECE_SPACE, MOVER_UNDO);
            mover_newundo(plevel, plevel->player_x[2], plevel->player_y[2], MOVE_NONE, PIECE_CURSOR, PIECE_SPACE, MOVER_UNDO);
            display_animation = 0.5;
            display_movers(plevel, 1);
            level_setmoving(plevel, ex, ey, MOVE_NONE);
            level_setmoving(plevel, plevel->player_x[2], plevel->player_y[2], MOVE_NONE);
            level_setprevious(plevel, plevel->player_x[2], plevel->player_y[2], PIECE_SPACE);

            /* Delete cosmetic movers */
            pmover = plevel->mover_first;
            while(pmover != NULL)
            {   
#ifdef XOR_COMPATIBILITY
                /* Update map if present */
                if(options_xor_display && pmover->piece != PIECE_CURSOR)
		    display_map_piece(plevel, pmover->piece, pmover->x, pmover->y, 1);
#endif
                pmovertmp = pmover;
                pmover = pmover->next;
                free(pmovertmp);
            }
            plevel->mover_first = NULL;
            plevel->mover_last = NULL;
        }

        SDL_WaitEvent(&event);

        /* Hide the mouse if not used whilst in full screen mode */
        if(SDL_GetTicks() > mouse_time + MOUSE_TIMEOUT_MOVE)
            screen_cursor(0);

        /* Store previous cursor location for redrawing damaged areas */
        ex = plevel->player_x[2];
        ey = plevel->player_y[2];
        ep = editor_piece;

        switch(event.type)
        {
            case SDL_KEYDOWN:
                switch(actions[event.key.keysym.sym])
		{
		    case ACTION_QUIT:
                        quit = 1;
                        break;

		    case ACTION_HIDE:
		       	SDL_WM_IconifyWindow();
			break;
        
                    case ACTION_LEFT:
                        if(plevel->player_x[2] > 0)
                        {
                            plevel->player_x[2] --; moved = 1;
                        }
                        break;

                    case ACTION_RIGHT:
                        if(plevel->player_x[2] < plevel->size_x - 1)
                        {
                            plevel->player_x[2] ++; moved = 1;
                        }
                        break;

                    case ACTION_UP:
                        if(plevel->player_y[2] > 0)
                        {
                            plevel->player_y[2] --; moved = 1;
                        }
                        break;

                    case ACTION_DOWN:
                        if(plevel->player_y[2] < plevel->size_y -1)
                        {
                            plevel->player_y[2] ++; moved = 1;
                        }
                        break;

		    case ACTION_PIECE_LEFT:
                        effect = ACTION_PIECE_LEFT;
                        break;

		    case ACTION_PIECE_RIGHT:
                        effect = ACTION_PIECE_RIGHT;
                        break;

		    case ACTION_SWAP:
                        effect = ACTION_SWAP;
                        moved = 1;
                        break;

                    default:
                        break;
                }
            break;


                case SDL_MOUSEBUTTONDOWN:
                    mouse_x = event.button.x;
                    mouse_y = event.button.y;
                    mouse_button = event.button.button;
                    mouse_time = SDL_GetTicks();
                    screen_cursor(1);

                    if(mouse_button > 0 && mouse_button < MOUSE_BUTTONS_MAX)
                        action = actions_mouse[ACTIONS_EDIT][mouse_button];

                    switch(action)
                    {
                        case ACTION_HIDE:
                            SDL_WM_IconifyWindow();
                            action = ACTION_NONE;
                            break;

                        case ACTION_QUIT:
                            quit = 1;
                            break;

                        case ACTION_PIECE_LEFT:
                            effect = ACTION_PIECE_LEFT;
                            break;

                        case ACTION_PIECE_RIGHT:
                            effect = ACTION_PIECE_RIGHT;
                            break;

                        default:
                            break;
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if(action == ACTION_MOUSE_CLICK ||
                            (action == ACTION_MOUSE_DRAG_OR_CLICK && ((SDL_GetTicks() < mouse_time + MOUSE_TIMEOUT_CLICK) || (mouse_y > (screen_height - display_bar_pixels)))))
                    { 
                        /* Have they clicked within the piece bar? */
                        if(mouse_y > (screen_height - display_bar_pixels))
                        {
                            editor_piece = piece_start + ((mouse_x) / pdisplaygraphics->size_x);
                            pmoved = 1;
                        }
                        else
                        {
                            plevel->player_x[2] = (mouse_x - display_offset_pixels_x) / pdisplaygraphics->size_x;
                            plevel->player_y[2] = (mouse_y - display_offset_pixels_y) / pdisplaygraphics->size_y;
                            moved = 2;
                            effect = ACTION_SWAP;
                        }
                    }

                    /* Buttons 4 and 5 can't be held down */
                    if(mouse_button != 4 && mouse_button != 5)
                    {  
                        mouse_button = 0;
                        action = ACTION_NONE;
                    }
                    break;

                case SDL_MOUSEMOTION:
                    mouse_time = SDL_GetTicks();
                    screen_cursor(1);

                    if(action == ACTION_MOUSE_DRAG || action == ACTION_MOUSE_DRAG_OR_CLICK)
                    {  
                        dx = mouse_x - event.motion.x;
                        dy = mouse_y - event.motion.y;

                        mouse_x = event.motion.x;
                        mouse_y = event.motion.y;

                        display_offset_pixels_y -= dy;
                        display_offset_pixels_x -= dx;

                        redraw |= 1;
                    }
                    else if(action == ACTION_MOUSE_CLICK)
                    { 

                        mouse_x = event.motion.x;
                        mouse_y = event.motion.y;
                        /* Have they clicked within the piece bar? */
                        if(mouse_y > (screen_height - display_bar_pixels))
                        {
                            editor_piece = piece_start + ((mouse_x) / pdisplaygraphics->size_x);
                            pmoved = 1;
                        }
                        else
                        {
                            plevel->player_x[2] = (mouse_x - display_offset_pixels_x) / pdisplaygraphics->size_x;
                            plevel->player_y[2] = (mouse_y - display_offset_pixels_y) / pdisplaygraphics->size_y;
                            moved = 2;
                            effect = ACTION_SWAP;
                        }
                    }
                    break;

            case SDL_QUIT:
                exit(0);

            case SDL_VIDEORESIZE:
                screen_resizeevent(&event);
                redraw = 2;
                break;

            case SDL_ACTIVEEVENT:
                if((event.active.state & SDL_APPACTIVE) && event.active.gain == 1)
                    redraw = 2;
                break;
        }

        switch(effect)
        {
	    case ACTION_PIECE_LEFT:
                editor_piece --;
                if(editor_piece < 0)
                    editor_piece = piece_count - 1;
                pmoved = 1;
                break;

            case ACTION_PIECE_RIGHT:
                editor_piece ++;
                if(editor_piece >= piece_count)
                    editor_piece = 0;
                pmoved = 1;
                break;

            case ACTION_SWAP:
                bevel = 0;
                if(editor_piece_maps[plevel->mode][editor_piece] == PIECE_WALL || level_piece(plevel, plevel->player_x[2], plevel->player_y[2]) == PIECE_WALL)
                    bevel = 1;
                /* Don't allow the edges to be changed */
                if(!(plevel->player_x[2] < 1 || plevel->player_x[2] > plevel->size_x - 2 || plevel->player_y[2] < 1 || plevel->player_y[2] > plevel->size_y - 2))
                    level_setpiece(plevel, plevel->player_x[2], plevel->player_y[2], editor_piece_maps[plevel->mode][editor_piece]);

                /* Rebevel if necessary */
                if(bevel == 1)
                {
                    x = plevel->player_x[2]; y = plevel->player_y[2];
                    for(i = -1; i < 2; i ++)
                    {   
                        for(j = - 1; j < 2; j ++)
                        {   
                                bevel = display_bevelsquare(plevel, x + i, y + j);
                                if(bevel != (level_data(plevel, x + i, y + j) & BEVEL_ALL))
                                {   
                                    level_setdata(plevel, x + i, y + j, bevel | (level_data(plevel, x + i, y + j) & ~BEVEL_ALL));
                                    p = level_piece(plevel, x + i, y + j);
                                    /* Redraw changed piece */
                                    /* The mover will get deleted when next redrawn */
                                    if(pdisplaygraphics->shadows != NULL)
                                        mover_newundo(plevel, x + i, y + j, MOVE_NONE, PIECE_SPACE, PIECE_SPACE, MOVER_UNDO); 
                                    else
                                        mover_newundo(plevel, x + i, y + j, MOVE_NONE, level_piece(plevel, x + i, y + j), PIECE_SPACE, MOVER_UNDO);
                                }
                            }
                        }
                    }
                break;

            default:
                break;
        }
        effect = ACTION_NONE;
    }

    /* Restore real player value */
    plevel->player = player;
    screen_cursor(1);
}

int display_type()
{
    return DISPLAY_SDL;
}

int scale_delay(int delay, int change)
{
    int tmp;
    int magnitude;

    if(delay < 1)
        delay = 1;

    if(change == 1)
    {
        tmp = delay;

        while(tmp >= 100)
            tmp = tmp / 10;

        magnitude = delay / tmp;
        
        if(tmp < 20)
            tmp = tmp + 1;
        else if(tmp < 50)
            tmp = tmp + 2;
        else
            tmp = tmp + 5;

        delay = tmp * magnitude;

        if(delay > 1000)
            delay = 1;
    }

    if(change == -1)
    {
        tmp = delay;

        while(tmp > 100)
            tmp = tmp / 10;

        magnitude = delay / tmp;

        if(tmp > 50)
            tmp = tmp - 5;
        else if(tmp > 20)
            tmp = tmp - 2;
        else
            tmp = tmp - 1;

        delay = tmp * magnitude;

        if(delay < 1)
            delay = 1000;
    }

    return delay;
}

void display_options()
{
    struct menu* pmenu;
    struct menu* pgraphicsmenu;
    struct menu* pcoloursmenu;
    struct menuentry* pentryscreensize;
    struct menuentry* pentryfullscreen;
    struct menuentry* pentrygraphics;
    struct menuentry* pentrysize;
    struct menuentry* pentrylevel;
    struct menuentry* pentrycolours;
    struct menuentry* pentryspeed;
    struct menuentry* pentryfirstspeed;
    struct menuentry* pentryreplayspeed;
    struct menuentry* pentryundospeed;
    struct menuentry* pentry;
    int result;
    int ok;

    char buffer[4096];

    pmenu = menu_new(gettext("Display Options"));

    pentry = menuentry_new(pmenu, gettext("Return to previous menu"), 'Q', 0);
    menuentry_new(pmenu, gettext("Save Options"), 'S', 0);

    menuentry_new(pmenu, "", 0, MENU_SPACE);

    pentrygraphics = menuentry_new(pmenu, gettext("Graphics Scheme"), 'G', 0);
    pentrysize = menuentry_new(pmenu, gettext("Graphics Size"), 'I', 0);
    pentrylevel = menuentry_new(pmenu, gettext("Graphics Level"), 'L', 0);
    pentrycolours = menuentry_new(pmenu, gettext("Colour Scheme"), 'C', 0);

    menuentry_new(pmenu, "", 0, MENU_SPACE);

    pentryscreensize = menuentry_new(pmenu, gettext("Screen size"), 'Z', 0);
    pentryfullscreen = menuentry_new(pmenu, gettext("Fullscreen"), 'F', 0);

    menuentry_new(pmenu, "", 0, MENU_SPACE);

    pentryfirstspeed = menuentry_new(pmenu, gettext("Player Speed"), 'P', MENU_SCROLLABLE);
    pentryspeed = menuentry_new(pmenu, gettext("Move Speed"), 'M', MENU_SCROLLABLE);
    pentryreplayspeed = menuentry_new(pmenu, gettext("Replay Speed"), 'R', MENU_SCROLLABLE);
    pentryundospeed = menuentry_new(pmenu, gettext("Undo Speed"), 'U', MENU_SCROLLABLE);

    menuentry_new(pmenu, "", 0, MENU_SPACE);

    menuentry_new(pmenu, gettext("Change Keys"), 'K', 0);
    menuentry_new(pmenu, gettext("Change Mouse"), 'O', 0);

    /* XOR and Enigma options are only visible once an appropriate level has
     * been seen so as not to confuse those simply playing Chroma levels */
    if(0
#ifdef XOR_COMPATIBILITY
	    || options_xor_options
#endif
#ifdef ENIGMA_COMPATIBILITY
	    || options_enigma_options
#endif
      )
    {
        menuentry_new(pmenu, "", 0, MENU_SPACE);
        menuentry_new(pmenu, gettext("Other Games Options"), 'X', 0);
    }

    if(options_debug & DEBUG_MENU)
    {
        menuentry_new(pmenu, "", 0, MENU_SPACE);
        menuentry_new(pmenu, gettext("Debug Options"), 'D', 0);
    }

    ok = 0;
    while(!ok)
    {
        if(options_sdl_width == 0 && options_sdl_height == 0)
            sprintf(buffer, gettext("Auto (%d x %d)"), screen_width, screen_height);
        else
            sprintf(buffer, "%d x %d", screen_width, screen_height);
        menuentry_extratext(pentryscreensize, buffer, NULL, NULL);

        menuentry_extratext(pentryfullscreen, screen_fullscreen ? gettext("Yes") : gettext("No"), NULL, NULL);

        sprintf(buffer, gettext("%d milliseconds"), options_sdl_delay);
        menuentry_extratext(pentryspeed, buffer, NULL, NULL);

        sprintf(buffer, gettext("%d milliseconds"), options_sdl_player_delay);
        menuentry_extratext(pentryfirstspeed, buffer, NULL, NULL);

        sprintf(buffer, gettext("%d milliseconds"), options_sdl_replay_delay);
        menuentry_extratext(pentryreplayspeed, buffer, NULL, NULL);

        sprintf(buffer, gettext("%d milliseconds"), options_sdl_undo_delay);
        menuentry_extratext(pentryundospeed, buffer, NULL, NULL);

        if(pdisplaygraphics != NULL)
        {
            if(options_sdl_size_x != pdisplaygraphics->size_x || options_sdl_size_y != pdisplaygraphics->size_y)
                sprintf(buffer, gettext("Auto (%d x %d)"), pdisplaygraphics->size_x, pdisplaygraphics->size_y);

            else
                sprintf(buffer, "%d x %d", pdisplaygraphics->size_x, pdisplaygraphics->size_y);

            menuentry_extratext(pentrysize, buffer, NULL, NULL);

            menuentry_extratext(pentrygraphics, pdisplaygraphics->title != NULL ? pdisplaygraphics->title : gettext("[untitled graphics]"), NULL, NULL);

            if(pdisplaygraphics->title == NULL)
                menuentry_extratext(pentrygraphics, gettext("[untitled graphics]"), NULL, NULL);
            else if(pdisplaygraphics->flags & GRAPHICS_TRANSLATE)
                menuentry_extratext(pentrygraphics, gettext(pdisplaygraphics->title), NULL, NULL);
            else
                menuentry_extratext(pentrygraphics, pdisplaygraphics->title, NULL, NULL);

            if(options_graphic_level != 0)
                sprintf(buffer, "%d / %d", options_graphic_level, pdisplaygraphics->levels);
            else
                sprintf(buffer, gettext("Auto (%d)"), pdisplaygraphics->levels);

            menuentry_extratext(pentrylevel, buffer, NULL, NULL);
        }
        else
            menuentry_extratext(pentrygraphics, gettext("** NONE **"), NULL, NULL);

        if(pdisplaygraphics != NULL && (pdisplaygraphics->flags & GRAPHICS_CURSES))
            pentrycolours->flags = 0;
        else
            pentrycolours->flags = MENU_INVISIBLE | MENU_GREY;

        if(pdisplaygraphics != NULL && pdisplaygraphics->sizes != NULL)
            pentrysize->flags = 0;
        else
            pentrysize->flags = MENU_INVISIBLE | MENU_GREY;

        if(pdisplaygraphics != NULL && pdisplaygraphics->levels > 0)
            pentrylevel->flags = MENU_SCROLLABLE;
        else
            pentrylevel->flags = MENU_INVISIBLE | MENU_GREY;

        if(pdisplaycolours == NULL)
            menuentry_extratext(pentrycolours, gettext("** NONE **"), NULL, NULL);
        else if(pdisplaycolours->title == NULL)
            menuentry_extratext(pentrycolours, gettext("[untitled colours]"), NULL, NULL);
        else if(pdisplaycolours->flags & COLOURS_TRANSLATE)
            menuentry_extratext(pentrycolours, gettext(pdisplaycolours->title), NULL, NULL);
        else
            menuentry_extratext(pentrycolours, pdisplaycolours->title, NULL, NULL);

        result = menu_process(pmenu);
        if(result == MENU_QUIT)
            ok = 1;

        if(result == MENU_SELECT && pmenu->entry_selected != NULL)
        {
            switch(pmenu->entry_selected->key)
            {
                case 'Q':
                    ok = 1;
                    break;

                case 'Z':
                    display_screensizemenu();
                    break;

                case 'F':
                    screen_fullscreen = 1 - screen_fullscreen;
                    screen_resize(screen_width, screen_height, screen_fullscreen);
                    options_sdl_fullscreen = screen_fullscreen;
                    break;

                case 'G':
                    pgraphicsmenu = graphics_menu();
                    if(menu_process(pgraphicsmenu) == MENU_SELECT)
                    {
                        if(pgraphicsmenu->entry_selected != NULL && pgraphicsmenu->entry_selected->value != NULL)
                        {
                            strcpy(options_graphics, pgraphicsmenu->entry_selected->value);
                            graphics_init();
                        }
                    }
                    menu_delete(pgraphicsmenu);
                    break;

                case 'C':
                    pcoloursmenu = colours_menu();
                    if(menu_process(pcoloursmenu) == MENU_SELECT)
                    {
                        if(pcoloursmenu->entry_selected != NULL && pcoloursmenu->entry_selected->value != NULL)
                        {
                            strcpy(options_colours, pcoloursmenu->entry_selected->value);
                            colours_init();
                            graphics_init();
                        }
                    }
                    menu_delete(pcoloursmenu);
                    break;

                case 'I':
                    display_options_size();
                    break;

                case 'S':
                    display_options_save();
                    ok = 1;
                    break;

                case 'K':
                    display_options_keys();
                    break;

                case 'O':
                    display_options_mouse();
                    break;

                case 'X':
                    display_options_othergames();
                    break;

                case 'D':
                    display_options_debug();
                    break;
            }
        }

        if(result == MENU_SCROLLLEFT && pmenu->entry_selected != NULL)
        {
            switch(pmenu->entry_selected->key)
            {
                case 'M':
                    options_sdl_delay = scale_delay(options_sdl_delay, -1);
                    break;
                case 'P':
                    options_sdl_player_delay = scale_delay(options_sdl_player_delay, -1);
                    break;
                case 'R':
                    options_sdl_replay_delay = scale_delay(options_sdl_replay_delay, -1);
                    break;
                case 'U':
                    options_sdl_undo_delay = scale_delay(options_sdl_undo_delay, -1);
                    break;
                case 'L':
                    options_graphic_level --;
                    if(options_graphic_level < 0)
                        options_graphic_level = pdisplaygraphics->levels;
                    graphics_reload();
                    break;
            }
        }

        if(result == MENU_SCROLLRIGHT && pmenu->entry_selected != NULL)
        {
            switch(pmenu->entry_selected->key)
            {   
                case 'M':
                    options_sdl_delay = scale_delay(options_sdl_delay, 1);
                    break;
                case 'P':
                    options_sdl_player_delay = scale_delay(options_sdl_player_delay, 1);
                    break;
                case 'R':
                    options_sdl_replay_delay = scale_delay(options_sdl_replay_delay, 1);
                    break;
                case 'U':
                    options_sdl_undo_delay = scale_delay(options_sdl_undo_delay, 1);
                    break;
                case 'L':
                    options_graphic_level ++;
                    if(options_graphic_level > pdisplaygraphics->levels)
                        options_graphic_level = 0;
                    graphics_reload();
                    break;
            }
        }


    }

    menu_delete(pmenu);
}

void display_clip(struct level* plevel, int clip)
{
    SDL_Rect crect;

    if(clip)
    {
        crect.x = 0;
        crect.y = 0;
        crect.w = screen_width;
        crect.h = screen_height - display_bar_pixels;

#ifdef XOR_COMPATIBILITY
        if(plevel->mode == MODE_XOR && options_xor_display)
        {
            crect.w = pdisplaygraphics->size_x * 8;
            crect.h = pdisplaygraphics->size_y * 8;
            crect.x = (screen_width - crect.w) / 2;
            crect.y = (screen_height - display_bar_pixels - crect.h) / 2;

            /* Ensure bar is always visible */
            if(crect.y + crect.h > screen_height - display_bar_pixels)
                crect.h = screen_height - display_bar_pixels - crect.y;
        }
#endif

        SDL_SetClipRect(screen_surface, &crect);
    }
    else
    {
        SDL_SetClipRect(screen_surface, NULL);
    }
}


void display_screensizemenu()
{
    struct menu* pmenu;
    struct menuentry *pentry;

    int sizes[7][2] = { {320, 240}, {640, 480}, {800, 600}, {1024, 768}, {1280, 1024}, {1600, 1200}, {0, 0} };
    int i;
    char buffer[256], tmp[256];
    int custom;
    SDL_Rect **modes;
    int w, h;
    int ok;

    custom = 1;

    pmenu = menu_new(gettext("Screen Size"));

    menuentry_new(pmenu, gettext("Quit and return to previous menu"), 'Q', 0);
    menuentry_new(pmenu, "", 0, MENU_SPACE);

    pentry = menuentry_new(pmenu, gettext("Automatic sizing"), 0, 0);
    menuentry_extratext(pentry, NULL, "0", "0");
    if(options_sdl_width == 0 && options_sdl_height == 0)
    {
        pmenu->entry_selected = pentry;
        custom = 0;
    }

    /* First, add modes that we know the screen can do */
    modes = SDL_ListModes(NULL, SDL_FULLSCREEN);
    if(modes != (SDL_Rect **)0 && modes != (SDL_Rect **)-1)
    {
        for(i = 0; modes[i]; i ++)
        {
            w = modes[i]->w; h = modes[i]->h;
            sprintf(buffer, "%d x %d", w, h);

            pentry = pmenu->entry_first; ok = 1;
            while(pentry != NULL)
            {  
                if(pentry->text != NULL && strcmp(pentry->text, buffer) == 0)
                {  
                    ok = 0;
                }
                pentry = pentry->next;
            }

            if(ok)
            {
                sprintf(tmp, "%04dx%04d", w, h);
                pentry = menuentry_newwithvalue(pmenu, buffer, 0, MENU_SORT, tmp);
                sprintf(buffer, "%d", w);
                sprintf(tmp, "%d", h);
                menuentry_extratext(pentry, NULL, buffer, tmp);
                if(options_sdl_width == w && options_sdl_height == h)
                    pmenu->entry_selected = pentry;
                if(screen_width == w && screen_height == h)
                    custom = 0;
            }
        }
    }

    /* Then add any of the default modes that haven't been already added */
    i = 0;
    while(sizes[i][0] != 0)
    {
        sprintf(buffer, "%d x %d", sizes[i][0], sizes[i][1]);
        pentry = pmenu->entry_first; ok = 1;
        while(pentry != NULL)
        {
            if(pentry->text != NULL && strcmp(pentry->text, buffer) == 0)
            {
                ok = 0;
            }
            pentry = pentry->next;
        }

        if(ok)
        {
            sprintf(tmp, "%04dx%04d", sizes[i][0], sizes[i][1]);
            pentry = menuentry_newwithvalue(pmenu, buffer, 0, MENU_SORT, tmp);
            sprintf(buffer, "%d", sizes[i][0]);
            sprintf(tmp, "%d", sizes[i][1]);
            menuentry_extratext(pentry, NULL, buffer, tmp);
            if(options_sdl_width == sizes[i][0] && options_sdl_height == sizes[i][1])
                pmenu->entry_selected = pentry;
            if(screen_width == sizes[i][0] && screen_height == sizes[i][1])
                custom = 0;
        }

        i ++;
    }
    if(custom)
    {
        sprintf(buffer, gettext("Custom (%d x %d)"), screen_width, screen_height);
        pentry = menuentry_new(pmenu, buffer, 0, 0);
        sprintf(buffer, "%d", screen_width);
        sprintf(buffer + 16, "%d", screen_height);
        menuentry_extratext(pentry, NULL, buffer, buffer + 16);
        pmenu->entry_selected = pentry;
    }

    pentry = menuentry_new(pmenu, gettext("Current screen size:"), 0, MENU_NOTE);
    sprintf(buffer, gettext("%d x %d (%d bits per pixel)"), screen_width, screen_height, screen_surface->format->BitsPerPixel);
    pentry = menuentry_new(pmenu, buffer, 0, MENU_NOTE | MENU_RIGHT);

    menu_assignletters(pmenu);

    if(menu_process(pmenu) == MENU_SELECT)
    {
        if(pmenu->entry_selected->text3 != NULL)
        {
            options_sdl_width = atoi(pmenu->entry_selected->text3);
            options_sdl_height = atoi(pmenu->entry_selected->text4);
            screen_resize(options_sdl_width, options_sdl_height, screen_fullscreen);
        }
    }

    menu_delete(pmenu);
}

int display_keyfixed(SDLKey key)
{
    if(key == SDLK_ESCAPE || key == SDLK_q || key == SDLK_RETURN || key == SDLK_UP || key == SDLK_DOWN || key == SDLK_LEFT || key == SDLK_RIGHT)
	return 1;

    return 0;
}

char *display_keyname(SDLKey key)
{
    return SDL_GetKeyName(key);
}

void display_addkeytomenu(struct menu* pmenu, int action, char *text)
{
    struct menuentry *pentry;
    char buffer[256];
    SDLKey key;

    sprintf(buffer, "%d", action);
    pentry = menuentry_newwithvalue(pmenu, text, 0, MENU_DOUBLE, buffer);

    strcpy(buffer, "");
    for(key = SDLK_FIRST; key < SDLK_LAST; key ++)
    {
        if(actions[key] == action)
        {
            if(strlen(buffer) != 0)
                strcat(buffer,", ");
            strcat(buffer, "[");
            strcat(buffer, display_keyname(key));
            strcat(buffer, "]");
        }
    }

    if(strcmp(buffer, "") == 0)
        strcpy(buffer, gettext("(none)"));

    menuentry_extratext(pentry, NULL, NULL, buffer);
}

void display_options_keys()
{
    struct menu *pmenu;
    struct menu *psubmenu;
    struct menuentry *pentry;
    int redraw;
    int action;
    int result;
    int ok;
    int subok;
    char buffer[256];
    SDLKey key;
    SDL_Event event;

    ok = 0;
    while(!ok)
    {
        pmenu = menu_new(gettext("Keys"));

        menuentry_new(pmenu, gettext("Quit and return to previous menu"), 'Q', 0);
        menuentry_new(pmenu, "", 0, MENU_SPACE);

        display_addkeytomenu(pmenu, ACTION_LEFT, gettext(action_name[ACTION_LEFT]));
        display_addkeytomenu(pmenu, ACTION_RIGHT, gettext(action_name[ACTION_RIGHT]));
        display_addkeytomenu(pmenu, ACTION_UP, gettext(action_name[ACTION_UP]));
        display_addkeytomenu(pmenu, ACTION_DOWN, gettext(action_name[ACTION_DOWN]));
        display_addkeytomenu(pmenu, ACTION_SWAP, gettext(action_name[ACTION_SWAP]));
        display_addkeytomenu(pmenu, ACTION_UNDO, gettext(action_name[ACTION_UNDO]));
        display_addkeytomenu(pmenu, ACTION_REDO, gettext(action_name[ACTION_REDO]));
        display_addkeytomenu(pmenu, ACTION_FAST, gettext(action_name[ACTION_FAST]));
        display_addkeytomenu(pmenu, ACTION_FASTER, gettext(action_name[ACTION_FASTER]));
        display_addkeytomenu(pmenu, ACTION_SLOWER, gettext(action_name[ACTION_SLOWER]));
        display_addkeytomenu(pmenu, ACTION_PAUSE, gettext(action_name[ACTION_PAUSE]));
        display_addkeytomenu(pmenu, ACTION_QUIT, gettext(action_name[ACTION_QUIT]));
        display_addkeytomenu(pmenu, ACTION_REDRAW, gettext(action_name[ACTION_REDRAW]));
        display_addkeytomenu(pmenu, ACTION_HIDE, gettext(action_name[ACTION_HIDE]));
        display_addkeytomenu(pmenu, ACTION_PIECE_LEFT, gettext(action_name[ACTION_PIECE_LEFT]));
        display_addkeytomenu(pmenu, ACTION_PIECE_RIGHT, gettext(action_name[ACTION_PIECE_RIGHT]));

        menu_assignletters(pmenu);

        result = menu_process(pmenu);

        if(result == MENU_QUIT)
            ok = 1;

        if(result == MENU_SELECT)
        {   
            if(pmenu->entry_selected->key == 'Q')
                ok = 1;
            else if(pmenu->entry_selected->value != NULL)
            {
                subok = 0;
                redraw = MENUREDRAW_ALL;
                while(!subok)
                {
                    action = atoi(pmenu->entry_selected->value);

                    sprintf(buffer, gettext("Set keys for '%s'"), gettext(action_name[action]));
                    psubmenu = menu_new(buffer);

                    menuentry_new(psubmenu, gettext("Quit and return to previous menu"), 'Q', 0);
                    menuentry_new(psubmenu, "", 0, MENU_SPACE);

                    for(key = SDLK_FIRST; key < SDLK_LAST; key ++)
                    {   
                        if(actions[key] == action)
                        {   
                            sprintf(buffer, "[%s]", display_keyname(key));
                            pentry = menuentry_new(psubmenu, buffer, 0, MENU_GREY);
                            if(display_keyfixed(key))
                                menuentry_extratext(pentry, gettext("(fixed)"), NULL, NULL);
                        }
                    }

                    menuentry_new(psubmenu, gettext("Press a key to add or remove it from this list."), 0, MENU_NOTE | MENU_CENTRE);

                    menu_display(psubmenu, redraw);
                    redraw = MENUREDRAW_ENTRIES;

                    menu_delete(psubmenu);

		    subok = 0;
		    while(!subok)
		    {
			SDL_WaitEvent(&event);

			switch(event.type)
			{
			    case SDL_KEYDOWN:
				key = event.key.keysym.sym;
				if(key == SDLK_q || key == SDLK_ESCAPE || key == 51)
				    subok = 1;
				else if(!display_keyfixed(key))
				{
                                    if(actions[key] == action)
                                        actions[key] = ACTION_NONE;
                                    else
                                        actions[key] = action;

				    subok = 2;
				}
				break;

			    case SDL_QUIT:
				exit(0);

			    case SDL_VIDEORESIZE:
				screen_resizeevent(&event);
				subok = 2;
                                redraw = MENUREDRAW_ALL;
				break;

                            case SDL_ACTIVEEVENT:
                                if((event.active.state & SDL_APPACTIVE) && event.active.gain == 1)
                                    redraw = MENUREDRAW_ALL;
                                break;
			}
		    }
		    if(subok == 2)
			subok = 0;
                }
            }
        }

        menu_delete(pmenu);
    }
}

void display_options_mouse()
{
    struct menu *pmenu;
    struct menu *psubmenu;
    struct menuentry *pentry;
    int result;
    int ok;
    int subok;
    int i, j, k;
    int button, where;
    char buffer[256];

    char *locations[] = {"Game", "Editor", "Menu"};

    int actions_game[] = {
        ACTION_NONE,
        ACTION_MOUSE_CLICK,
        ACTION_MOUSE_DRAG,
        ACTION_MOUSE_DRAG_OR_CLICK,
        ACTION_SWAP,
        ACTION_UNDO,
        ACTION_REDO,
        ACTION_HIDE,
        ACTION_QUIT,
        ACTION_MAX
    };

    int actions_edit[] = {
        ACTION_NONE,
        ACTION_MOUSE_CLICK,
        ACTION_MOUSE_DRAG,
        ACTION_MOUSE_DRAG_OR_CLICK,
        ACTION_PIECE_LEFT,
        ACTION_PIECE_RIGHT,
        ACTION_HIDE,
        ACTION_QUIT,
        ACTION_MAX
    };

    int actions_menu[] = {
        ACTION_NONE,
        ACTION_UP,
        ACTION_DOWN,
        ACTION_PAGE_UP,
        ACTION_PAGE_DOWN,
        ACTION_HIDE,
        ACTION_MOUSE_CLICK,
        ACTION_QUIT,
        ACTION_MAX
    };

    int *actions_available[] = {
        actions_game,
        actions_edit,
        actions_menu,
    };

    char *actions_names_game[] = {
        "Do nothing",
        "Click to move player",
        "Drag to scroll screen",
        "Click to move player, drag to scroll screen",
        "Click to swap player",
        "Undo move",
        "Redo move",
        "Hide screen",
        "Return to previous menu",
        ""
    };

    char *actions_names_edit[] = {
        "Do nothing",
        "Click to set piece",
        "Drag to scroll screen",
        "Click to set piece, drag to scroll screen",
        "Piece left",
        "Piece right",
        "Hide screen",
        "Return to previous menu",
        ""
    };

    char *actions_names_menu[] = {
        "Do nothing",
        "Move up",
        "Move down",
        "Page up",
        "Page down",
        "Hide screen",
        "Select entry",
        "Return to previous menu",
        ""
    };

    char **actions_names[] = {
        actions_names_game,
        actions_names_edit,
        actions_names_menu,
    };

    ok = 0;
    pmenu = menu_new(gettext("Mouse"));

    menuentry_new(pmenu, gettext("Quit and return to previous menu"), 'Q', 0);

    for(i = 0; i < 3; i ++)
    {
        menuentry_new(pmenu, "", 0, MENU_SPACE);
        menuentry_new(pmenu, locations[i], 0, MENU_GREY);
        for(j = 1; j < MOUSE_BUTTONS_MAX; j ++)
        {
            sprintf(buffer, gettext("Button %d"), j);
            pentry = menuentry_new(pmenu, buffer, 0, 0);

            sprintf(buffer, "%d%d", i, j);
            menuentry_value(pentry, buffer);

            /* Should be overwritten shortly - this is a fallback if not */
            sprintf(buffer, "? %s", gettext(action_name[actions_mouse[i][j]]));
            menuentry_extratext(pentry, buffer, NULL, NULL);

            /* Perhaps not the neatest way of doing this here, but saves
               having to have redundant actions merely to give them different
               names. */
            k = 0;
            while(actions_available[i][k] != ACTION_MAX)
            {
                if(actions_available[i][k] == actions_mouse[i][j])
                    menuentry_extratext(pentry, gettext(actions_names[i][k]), NULL, NULL);
                k ++;
            }
        }
    }

    menu_assignletters(pmenu);

    ok  = 0;
    while(!ok)
    {
        result = menu_process(pmenu);

        if(result == MENU_QUIT)
            ok = 1;

        if(result == MENU_SELECT)
        {   
            if(pmenu->entry_selected->key == 'Q')
                ok = 1;
            else if(pmenu->entry_selected->value != NULL)
            {
                where = pmenu->entry_selected->value[0] - '0';
                button = pmenu->entry_selected->value[1] - '0';
                sprintf(buffer, gettext("Set action for button %d in %s"), button, locations[where]);
                psubmenu = menu_new(buffer);

                menuentry_new(psubmenu, gettext("Quit and return to previous menu"), 'Q', 0);
                menuentry_new(psubmenu, "", 0, MENU_SPACE);

                i = 0;
                while(actions_available[where][i] != ACTION_MAX)
                {
                    pentry = menuentry_new(psubmenu, gettext(actions_names[where][i]), 0, 0);
                    sprintf(buffer, "%d", i);
                    menuentry_value(pentry, buffer);
                    if(actions_mouse[where][button] == actions_available[where][i])
                    {
                        psubmenu->entry_selected = pentry;

                        menuentry_new(psubmenu, gettext("Current action is:"), 0, MENU_NOTE);
                        menuentry_new(psubmenu, gettext(actions_names[where][i]), 0, MENU_NOTE | MENU_RIGHT);
                    }

                    i ++;
                }

                menu_assignletters(psubmenu);

                subok = 0;
                while(!subok)
                {
                    result = menu_process(psubmenu);

                    if(result == MENU_QUIT)
                        subok = 1;

                    if(result == MENU_SELECT)
                    {
                        if(psubmenu->entry_selected->key == 'Q')
                            subok = 1;
                        else if(psubmenu->entry_selected->value != NULL)
                        {
                            actions_mouse[where][button] = actions_available[where][atoi(psubmenu->entry_selected->value)];
                            menuentry_extratext(pmenu->entry_selected, gettext(actions_names[where][atoi(psubmenu->entry_selected->value)]), NULL, NULL);
                            subok = 1;
                        }
                    }

                }
                menu_delete(psubmenu);
            }
        }

    }

    menu_delete(pmenu);
}

void display_options_size()
{
    struct menu* pmenu;
    struct menuentry* pentry;
    struct graphicssize* psize;
    char buffer[4096];

    pmenu = menu_new(gettext("Graphics Size"));

    menuentry_new(pmenu, gettext("Quit and return to previous menu"), 'Q', 0);
    menuentry_new(pmenu, "", 0, MENU_SPACE);

    menuentry_new(pmenu, gettext("Current graphics size:"), 0, MENU_NOTE);
    if(pdisplaygraphics != NULL)
    {
        if(options_sdl_size_x != pdisplaygraphics->size_x || options_sdl_size_y != pdisplaygraphics->size_y)
            sprintf(buffer, gettext("Automatic (%d x %d)"), pdisplaygraphics->size_x, pdisplaygraphics->size_y);
        else
            sprintf(buffer, "%d x %d", pdisplaygraphics->size_x, pdisplaygraphics->size_y);
        menuentry_new(pmenu, buffer, 0, MENU_NOTE | MENU_RIGHT);
    }
    else
        menuentry_new(pmenu, gettext("** NONE **"), 0, MENU_NOTE | MENU_RIGHT);

    pentry = menuentry_new(pmenu, gettext("Automatic sizing"), 0, 0);
    menuentry_extratext(pentry, NULL, "0", "0");
    if(options_sdl_size_x != pdisplaygraphics->size_x || options_sdl_size_y != pdisplaygraphics->size_y)
        pmenu->entry_selected = pentry;

    psize = pdisplaygraphics->sizes;
    while(psize != NULL)
    {
        if(psize->flags & SIZE_PIECES)
        {
            sprintf(buffer, "%d x %d", psize->x, psize->y);
            pentry = menuentry_new(pmenu, buffer, 0, 0);
            if(psize->x == options_sdl_size_x && psize->y == options_sdl_size_y)
                pmenu->entry_selected = pentry;
            sprintf(buffer, "%d", psize->x);
            sprintf(buffer + 16, "%d", psize->y);
            menuentry_extratext(pentry, NULL, buffer, buffer + 16);
        }
        psize = psize->next;
    }
    menu_assignletters(pmenu);

    if(menu_process(pmenu) == MENU_SELECT)
    {
        if(pmenu->entry_selected->text3 != NULL)
        {
            options_sdl_size_x = atoi(pmenu->entry_selected->text3);
            options_sdl_size_y = atoi(pmenu->entry_selected->text4);

            graphics_init();
        }
    }

    menu_delete(pmenu);
}

void display_options_debug()
{
    struct menu* pmenu;
    struct menuentry* pentryorder;
    struct menuentry* pentrymovers;
    struct menuentry* pentryspeed;
    struct menuentry* pentryhidden;

    int ok;
    int result;

    pmenu = menu_new(gettext("Debug Options"));

    menuentry_new(pmenu, gettext("Return to previous menu"), 'Q', 0);

    menuentry_new(pmenu, "", 0, MENU_SPACE);
  
    pentryorder = menuentry_new(pmenu, gettext("Display order of movers"), 'O', 0);
    pentrymovers = menuentry_new(pmenu, gettext("List movers on stderr"), 'M', 0);
    pentryspeed = menuentry_new(pmenu, gettext("Show frames per second"), 'F', 0);
    pentryhidden = menuentry_new(pmenu, gettext("Show hidden items"), 'H', 0);

    ok = 0;
    while(!ok)
    {
        menuentry_extratext(pentryorder, options_debug & DEBUG_ORDER ? gettext("yes") : gettext("no"), NULL, NULL);
        menuentry_extratext(pentrymovers, options_debug & DEBUG_MOVERS ? gettext("yes") : gettext("no"), NULL, NULL);
        menuentry_extratext(pentryspeed, options_debug & DEBUG_SPEED ? gettext("yes") : gettext("no"), NULL, NULL);
        menuentry_extratext(pentryhidden, options_debug & DEBUG_HIDDEN ? gettext("yes") : gettext("no"), NULL, NULL);

        result = menu_process(pmenu);
        if(result == MENU_QUIT)
            ok = 1;

        if(result == MENU_SELECT && pmenu->entry_selected != NULL)
        {   
            switch(pmenu->entry_selected->key)
            {   
                case 'Q':
                    ok = 1;
                    break;

                case 'O':
                    options_debug ^= DEBUG_ORDER;
                    break;

                case 'M':
                    options_debug ^= DEBUG_MOVERS;
                    break;

                case 'F':
                    options_debug ^= DEBUG_SPEED;
                    break;

                case 'H':
                    options_debug ^= DEBUG_HIDDEN;
                    break;
            }

            pmenu->redraw = MENUREDRAW_CHANGED;
            pmenu->entry_selected->redraw = 1;
        }

    }

    menu_delete(pmenu);
}

void display_options_othergames()
{
    struct menu* pmenu;
#ifdef XOR_COMPATIBILITY
    struct menuentry* pentryxormode;
    struct menuentry* pentryxordisplay;
#endif
#ifdef ENIGMA_COMPATIBILITY
    struct menuentry* pentryenigmamode;
#endif

    int ok;
    int result;

    pmenu = menu_new(gettext("Other Games Options"));

    menuentry_new(pmenu, gettext("Return to previous menu"), 'Q', 0);

    menuentry_new(pmenu, "", 0, MENU_SPACE);
 
#ifdef XOR_COMPATIBILITY 
    pentryxormode = menuentry_new(pmenu, gettext("XOR Engine"), 'X', options_xor_options ? 0 : MENU_INVISIBLE | MENU_GREY);
    pentryxordisplay = menuentry_new(pmenu, gettext("XOR Display"), 'D', options_xor_options ? 0 : MENU_INVISIBLE | MENU_GREY);
    if(options_xor_options)
        menuentry_new(pmenu, "", 0, MENU_SPACE);
#endif

#ifdef ENIGMA_COMPATIBILITY
    pentryenigmamode = menuentry_new(pmenu, gettext("Enigma Engine"), 'E', options_enigma_options ? 0 : MENU_INVISIBLE | MENU_GREY);
#endif

    ok = 0;
    while(!ok)
    {
#ifdef XOR_COMPATIBILITY
        menuentry_extratext(pentryxormode, options_xor_mode ? gettext("exact") : gettext("approximate"), NULL, NULL);
        menuentry_extratext(pentryxordisplay, options_xor_display ? gettext("partial") : gettext("full"), NULL, NULL);
#endif
#ifdef ENIGMA_COMPATIBILITY
        menuentry_extratext(pentryenigmamode, options_enigma_mode ? gettext("exact") : gettext("approximate"), NULL, NULL);
#endif

        result = menu_process(pmenu);
        if(result == MENU_QUIT)
            ok = 1;

        if(result == MENU_SELECT && pmenu->entry_selected != NULL)
        {   
            switch(pmenu->entry_selected->key)
            {   
                case 'Q':
                    ok = 1;
                    break;

#ifdef XOR_COMPATIBILITY
                case 'X':
                    options_xor_mode = 1 - options_xor_mode;
                    break;

                case 'D':
                    options_xor_display = 1 - options_xor_display;
                    break;
#endif

#ifdef ENIGMA_COMPATIBILITY
                case 'E':
                    options_enigma_mode = 1 - options_enigma_mode;
                    break;
#endif
            }

            pmenu->redraw = MENUREDRAW_CHANGED;
            pmenu->entry_selected->redraw = 1;
        }

    }

    menu_delete(pmenu);
}

void display_options_save()
{
    FILE *file;
    char filename[FILENAME_MAX];
    SDLKey key;
    int i, j;
    char *locations[] = {"game", "editor", "menu"};

    getfilename("sdl.chroma", filename, 1, LOCATION_LOCAL);

    file = fopen(filename, "w");
    if(file == NULL)
    {   
        warning("Unable to save options");
        return;
    }

    fprintf(file, "<!-- Chroma SDL options \n"
                  "     This file is automatically generated. -->\n"
                  "\n"
                  "<chroma type=\"options\">\n");

    fprintf(file, "    <screen ");
    if(options_sdl_width == 0 && options_sdl_height == 0)
	fprintf(file, "width=\"auto\" height=\"auto\" ");
    else
	fprintf(file, "width=\"%d\" height=\"%d\" ", options_sdl_width, options_sdl_height);
    fprintf(file, "fullscreen=\"%s\" />\n", options_sdl_fullscreen == 1 ? "yes" : "no");

    fprintf(file, "    <graphics scheme=\"%s\" ", options_graphics);
    if(options_graphic_level != 0)
        fprintf(file, "level=\"%d\" ", options_graphic_level);
    if(options_sdl_size_x == 0)
	fprintf(file, "width=\"auto\" height=\"auto\" />\n");
    else
	fprintf(file, "width=\"%d\" height=\"%d\" />\n", options_sdl_size_x, options_sdl_size_y);

    fprintf(file, "    <colour scheme=\"%s\" />\n", options_colours);

    fprintf(file, "    <move speed=\"%d\" />\n", options_sdl_delay);
    fprintf(file, "    <player speed=\"%d\" />\n", options_sdl_player_delay);
    fprintf(file, "    <replay speed=\"%d\" />\n", options_sdl_replay_delay);
    fprintf(file, "    <undo speed=\"%d\" />\n", options_sdl_undo_delay);

#ifdef XOR_COMPATIBILITY
    if(options_xor_options)
        fprintf(file, "    <xor mode=\"%s\" display=\"%s\" />\n", options_xor_mode ? "exact" : "approximate", options_xor_display ? "partial" : "full");
#endif
#ifdef ENIGMA_COMPATIBILITY
    if(options_enigma_options)
        fprintf(file, "    <enigma mode=\"%s\" />\n", options_enigma_mode ? "exact" : "approximate");
#endif

    fprintf(file, "    <!-- Set <debug menu=\"yes\" /> to change debug options within Chroma -->\n");
    fprintf(file, "    <debug ");
    fprintf(file, "menu=\"%s\" ", options_debug & DEBUG_MENU ? "yes" : "no");
    fprintf(file, "order=\"%s\" ", options_debug & DEBUG_ORDER ? "yes" : "no");
    fprintf(file, "speed=\"%s\" ", options_debug & DEBUG_SPEED ? "yes" : "no");
    fprintf(file, "movers=\"%s\" ", options_debug & DEBUG_MOVERS ? "yes" : "no");
    fprintf(file, "hidden=\"%s\" ", options_debug & DEBUG_HIDDEN ? "yes" : "no");
    fprintf(file, "/>\n");

    fprintf(file, "    <keys>\n");

    for(key = SDLK_FIRST; key < SDLK_LAST; key ++)
    {
        if(actions[key] != ACTION_NONE)
            fprintf(file, "        <key name=\"%s\" action=\"%s\" />\n", display_keyname(key), action_shortname[actions[key]]);
    }

    fprintf(file, "    </keys>\n");

    for(i = 0; i < 3; i ++)
    {
        fprintf(file, "    <mouse location=\"%s\">\n", locations[i]);
        for(j = 1; j < MOUSE_BUTTONS_MAX; j ++)
        {
            fprintf(file, "        <button type=\"%d\" action=\"%s\" />\n", j, action_shortname[actions_mouse[i][j]]);
        }
        fprintf(file, "    </mouse>\n");
    }

    fprintf(file, "</chroma>\n");

    fclose(file);
}

void display_options_load()
{
    struct parser* pparser;
    char filename[FILENAME_MAX];
    int state;
    SDLKey k;
    int i, key, action, location, button;

    /* Sensible defaults */
    options_sdl_fullscreen = 1;
    options_sdl_width = 0;
    options_sdl_height = 0;
    options_sdl_delay = 100;
    options_sdl_player_delay = 200;
    options_sdl_replay_delay = 200;
    options_sdl_undo_delay = 200;
    options_sdl_size_x = 0;
    options_sdl_size_y = 0;
    options_graphic_level = 0;
#ifdef XOR_COMPATIBILITY
    options_xor_options = 0;
    options_xor_mode = 1;
    options_xor_display = 0;
#endif
#ifdef ENIGMA_COMPATIBILITY
    options_enigma_options = 0;
    options_enigma_mode = 1;
#endif
    options_debug = 0;

    getfilename("colours", filename, 0, LOCATION_SYSTEM);
    sprintf(options_colours, "%s/%s", filename, COLOURS_DEFAULT);
    getfilename("graphics", filename, 0, LOCATION_SYSTEM);
    sprintf(options_graphics, "%s/%s", filename, GRAPHICS_DEFAULT);

    getfilename("sdl.chroma", filename, 0, LOCATION_LOCAL);

    for(k = SDLK_FIRST; k < SDLK_LAST; k ++)
    {
        actions[k] = ACTION_NONE;
    }

    for(action = 0; action < MOUSE_BUTTONS_MAX; action ++)
    {
        actions_mouse[ACTIONS_GAME][action] = ACTION_NONE;
        actions_mouse[ACTIONS_MENU][action] = ACTION_NONE;
        actions_mouse[ACTIONS_EDIT][action] = ACTION_NONE;
    }

    /* Fixed keys */
    actions[SDLK_UP] = ACTION_UP;
    actions[SDLK_DOWN] = ACTION_DOWN;
    actions[SDLK_LEFT] = ACTION_LEFT;
    actions[SDLK_RIGHT] = ACTION_RIGHT;
    actions[SDLK_RETURN] = ACTION_SWAP;
    actions[SDLK_q] = ACTION_QUIT;
    actions[SDLK_ESCAPE] = ACTION_QUIT;
    actions[51] = ACTION_QUIT;

    actions_mouse[ACTIONS_GAME][1] = ACTION_MOUSE_DRAG_OR_CLICK;
    actions_mouse[ACTIONS_GAME][2] = ACTION_SWAP;
    actions_mouse[ACTIONS_GAME][3] = ACTION_UNDO;
    actions_mouse[ACTIONS_GAME][4] = ACTION_UNDO;
    actions_mouse[ACTIONS_GAME][5] = ACTION_REDO;

    actions_mouse[ACTIONS_MENU][1] = ACTION_MOUSE_CLICK;
    actions_mouse[ACTIONS_MENU][2] = ACTION_MOUSE_CLICK;
    actions_mouse[ACTIONS_MENU][3] = ACTION_QUIT;
    actions_mouse[ACTIONS_MENU][4] = ACTION_PAGE_UP;
    actions_mouse[ACTIONS_MENU][5] = ACTION_PAGE_DOWN;

    actions_mouse[ACTIONS_EDIT][1] = ACTION_MOUSE_CLICK;
    actions_mouse[ACTIONS_EDIT][2] = ACTION_MOUSE_DRAG;
    actions_mouse[ACTIONS_EDIT][3] = ACTION_QUIT;
    actions_mouse[ACTIONS_EDIT][4] = ACTION_PIECE_LEFT;
    actions_mouse[ACTIONS_EDIT][5] = ACTION_PIECE_RIGHT;

    /* Sensible default keys */
    if(!isfile(filename))
    {
        actions[SDLK_SPACE] = ACTION_SWAP;
        actions[SDLK_f] = ACTION_FAST;
        actions[SDLK_BACKSPACE] = ACTION_UNDO;
        actions[SDLK_DELETE] = ACTION_UNDO;
        actions[SDLK_u] = ACTION_UNDO;
        actions[SDLK_INSERT] = ACTION_REDO;
        actions[SDLK_y] = ACTION_REDO;
        actions[SDLK_p] = ACTION_PAUSE;
        actions[SDLK_LSHIFT] = ACTION_FASTER;
        actions[SDLK_RSHIFT] = ACTION_FASTER;
        actions[SDLK_LCTRL] = ACTION_SLOWER;
        actions[SDLK_RCTRL] = ACTION_SLOWER;
        actions[SDLK_z] = ACTION_PIECE_LEFT;
        actions[SDLK_x] = ACTION_PIECE_RIGHT;
        actions[SDLK_PAGEUP] = ACTION_PIECE_LEFT;;
        actions[SDLK_PAGEDOWN] = ACTION_PIECE_RIGHT;
        actions[SDLK_KP8] = ACTION_UP;
        actions[SDLK_KP2] = ACTION_DOWN;
        actions[SDLK_KP4] = ACTION_LEFT;
        actions[SDLK_KP6] = ACTION_RIGHT;
        actions[SDLK_KP_ENTER] = ACTION_SWAP;
        actions[SDLK_KP_MINUS] = ACTION_UNDO;
        actions[SDLK_KP_PLUS] = ACTION_REDO;
        actions[SDLK_KP_DIVIDE] = ACTION_PIECE_LEFT;
        actions[SDLK_KP_MULTIPLY] = ACTION_PIECE_RIGHT;

        return;
    }

    /* Parse XML file */
    /*
       <chroma type="options">
           <colour scheme="filename" />
           <move speed="speed" />
           <replay speed="speed" />
           <xor mode="mode" />
           <debug movers="yes/no" />
           <keys>
               <key name="name" action="action" />
           </keys>
       </chroma>
    */

    pparser = parser_new(filename);

    enum {
        OPTIONSPARSER_END,       /* End of file */
        OPTIONSPARSER_OUTSIDE,   /* Outside of <chroma> */
        OPTIONSPARSER_CHROMA,    /* Inside <chroma> */
        OPTIONSPARSER_KEYS,      /* Inside <keys> */
        OPTIONSPARSER_MOUSE      /* Inside <mouse> */
    };

    state = OPTIONSPARSER_OUTSIDE;
    key = 0;
    action = 0;
    location = -1;
    button = -1;

    while(state != OPTIONSPARSER_END)
    {
        switch(parser_parse(pparser))
        {
            case PARSER_END:
                state = OPTIONSPARSER_END;
                break;

            case PARSER_ELEMENT_START:
                switch(state)
                {
                    case OPTIONSPARSER_CHROMA:
                        if(parser_match(pparser, 0, "keys"))
                            state = OPTIONSPARSER_KEYS;
                        if(parser_match(pparser, 0, "mouse"))
                        {
                            location = -1;
                            state = OPTIONSPARSER_MOUSE;
                        }
                        break;

                    case OPTIONSPARSER_KEYS:
                        if(parser_match(pparser, 0, "key"))
                        {
                            key = 0;
                            action = ACTION_NONE;
                        }
                        break;

                    case OPTIONSPARSER_MOUSE:
                        if(parser_match(pparser, 0, "button"))
                        {
                            button = -1;
                            action = ACTION_NONE;
                        }
                        break;
                }
                break;

            case PARSER_ELEMENT_END:
                switch(state)
                {
                    case OPTIONSPARSER_KEYS:
                        if(parser_match(pparser, 0, "keys"))
                            state = OPTIONSPARSER_CHROMA;
                        if(parser_match(pparser, 0, "key"))
                        {
                            if(key != 0 && !display_keyfixed(key))
                                actions[key] = action;
                        }
                        break;

                    case OPTIONSPARSER_MOUSE:
                        if(parser_match(pparser, 0, "mouse"))
                            state = OPTIONSPARSER_CHROMA;
                        if(parser_match(pparser, 0, "button"))
                        {
                            if(location != -1 && button != -1)
                                actions_mouse[location][button] = action;
                        }
                        break;
                }
                break;

            case PARSER_CONTENT:
                break;

            case PARSER_ATTRIBUTE:
                switch(state)
                {
                    case OPTIONSPARSER_OUTSIDE:
                        if(parser_match(pparser, 2, "chroma") && parser_match(pparser, 1, "type"))
                        {   
                            if(parser_match(pparser, 0, "options"))
                                state = OPTIONSPARSER_CHROMA;
                        }
                        break;

                    case OPTIONSPARSER_CHROMA:
                        if(parser_match(pparser, 2, "screen") && parser_match(pparser, 1, "width"))
                        {
			    options_sdl_width = atoi(parser_text(pparser, 0));
                        }
                        if(parser_match(pparser, 2, "screen") && parser_match(pparser, 1, "height"))
                        {
			    options_sdl_height = atoi(parser_text(pparser, 0));
                        }
			if(parser_match(pparser, 2, "screen") && parser_match(pparser, 1, "fullscreen"))
			{
			    if(parser_match(pparser, 0, "yes"))
				options_sdl_fullscreen = 1;
			    if(parser_match(pparser, 0, "no"))
				options_sdl_fullscreen = 0;
			}
                        if(parser_match(pparser, 2, "colour") && parser_match(pparser, 1, "scheme"))
                        {
                            strncpy(options_colours, parser_text(pparser, 0), FILENAME_MAX);
                        }
                        if(parser_match(pparser, 2, "graphics") && parser_match(pparser, 1, "scheme"))
                        {
                            strncpy(options_graphics, parser_text(pparser, 0), FILENAME_MAX);
                        }
                        if(parser_match(pparser, 2, "graphics") && parser_match(pparser, 1, "width"))
                        {
			    options_sdl_size_x = atoi(parser_text(pparser, 0));
                        }
                        if(parser_match(pparser, 2, "graphics") && parser_match(pparser, 1, "height"))
                        {
			    options_sdl_size_y = atoi(parser_text(pparser, 0));
                        }
                        if(parser_match(pparser, 2, "graphics") && parser_match(pparser, 1, "level"))
                        {
			    options_graphic_level = atoi(parser_text(pparser, 0));
                        }
                        if(parser_match(pparser, 2, "move") && parser_match(pparser, 1, "speed"))
                        {
                            options_sdl_delay = atoi(parser_text(pparser, 0));
                        }
                        if(parser_match(pparser, 2, "player") && parser_match(pparser, 1, "speed"))
                        {
                            options_sdl_player_delay = atoi(parser_text(pparser, 0));
                        }
                        if(parser_match(pparser, 2, "replay") && parser_match(pparser, 1, "speed"))
                        {
                            options_sdl_replay_delay = atoi(parser_text(pparser, 0));
                        }
                        if(parser_match(pparser, 2, "undo") && parser_match(pparser, 1, "speed"))
                        {
                            options_sdl_undo_delay = atoi(parser_text(pparser, 0));
                        }
#ifdef XOR_COMPATIBILITY
                        if(parser_match(pparser, 2, "xor") && parser_match(pparser, 1, "mode"))
                        {
                            options_xor_options = 1;

                            if(parser_match(pparser, 0, "approximate"))
                                options_xor_mode = 0;
                            if(parser_match(pparser, 0, "exact"))
                                options_xor_mode = 1;
                        }
                        if(parser_match(pparser, 2, "xor") && parser_match(pparser, 1, "display"))
                        {
                            options_xor_options = 1;

                            if(parser_match(pparser, 0, "full"))
                                options_xor_display = 0;
                            if(parser_match(pparser, 0, "partial"))
                                options_xor_display = 1;
                        }
#endif
#ifdef ENIGMA_COMPATIBILITY
                        if(parser_match(pparser, 2, "enigma") && parser_match(pparser, 1, "mode"))
                        {
                            options_enigma_options = 1;

                            if(parser_match(pparser, 0, "approximate"))
                                options_enigma_mode = 0;
                            if(parser_match(pparser, 0, "exact"))
                                options_enigma_mode = 1;
                        }
#endif
                        if(parser_match(pparser, 2, "debug") && parser_match(pparser, 1, "menu"))
                        {
                            if(parser_match(pparser, 0, "yes"))
                                options_debug |= DEBUG_MENU;
                        }
                        if(parser_match(pparser, 2, "debug") && parser_match(pparser, 1, "order"))
                        {
                            if(parser_match(pparser, 0, "yes"))
                                options_debug |= DEBUG_ORDER;
                        }
                        if(parser_match(pparser, 2, "debug") && parser_match(pparser, 1, "speed"))
                        {
                            if(parser_match(pparser, 0, "yes"))
                                options_debug |= DEBUG_SPEED;
                        }
                        if(parser_match(pparser, 2, "debug") && parser_match(pparser, 1, "movers"))
                        {
                            if(parser_match(pparser, 0, "yes"))
                                options_debug |= DEBUG_MOVERS;
                        }
                        if(parser_match(pparser, 2, "debug") && parser_match(pparser, 1, "hidden"))
                        {
                            if(parser_match(pparser, 0, "yes"))
                                options_debug |= DEBUG_HIDDEN;
                        }
                        break;

                    case OPTIONSPARSER_KEYS:
                        if(parser_match(pparser, 2, "key") &&  parser_match(pparser, 1, "name"))
                        {
                            for(i = SDLK_FIRST; i < SDLK_LAST; i ++)
                            {
                                if(parser_match(pparser, 0, display_keyname(i)))
                                {
                                    key = i;
                                    i = SDLK_LAST;
                                }
                            }
                        }
                        if(parser_match(pparser, 2, "key") &&  parser_match(pparser, 1, "action"))
                        {
                            for(i = ACTION_KEY_MIN; i < ACTION_KEY_MAX; i ++)
                            {
                                if(parser_match(pparser, 0, action_shortname[i]))
                                {
                                    action = i;
                                    i = ACTION_KEY_MAX;
                                }
                            }
                        }
                        break;

                    case OPTIONSPARSER_MOUSE:
                        if(parser_match(pparser, 2, "button") &&  parser_match(pparser, 1, "type"))
                        {
                            button = atoi(parser_text(pparser, 0));
                            if(button < 1 || button >= MOUSE_BUTTONS_MAX)
                                button = -1;
                        }
                        if(parser_match(pparser, 2, "mouse") &&  parser_match(pparser, 1, "location"))
                        {
                            if(parser_match(pparser, 0, "game"))
                                location = ACTIONS_GAME;
                            if(parser_match(pparser, 0, "menu"))
                                location = ACTIONS_MENU;
                            if(parser_match(pparser, 0, "editor"))
                                location = ACTIONS_EDIT;
                        }
                        if(parser_match(pparser, 2, "button") &&  parser_match(pparser, 1, "action"))
                        {
                            for(i = 0; i < ACTION_MAX; i ++)
                            {
                                if(parser_match(pparser, 0, action_shortname[i]))
                                {
                                    action = i;
                                    i = ACTION_MAX;
                                }
                            }
                        }
                        break;
                }
                break;

            case PARSER_ERROR:
                state = OPTIONSPARSER_END;
                break;
        }
    }

    parser_delete(pparser);
}
