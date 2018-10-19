/*  
    sdlshadowdisplay.c

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

#include "chroma.h"
#include "menu.h"
#include "level.h"
#include "display.h"
#include "graphics.h"
#include "colours.h"
#include "sdlfont.h"
#include "sdlscreen.h"
#include "util.h"

extern SDL_Surface *screen_surface;

extern int screen_width;
extern int screen_height;
extern int screen_fullscreen;

extern int options_sdl_width;
extern int options_sdl_height;
extern int options_sdl_fullscreen;
extern int options_sdl_size_x;
extern int options_sdl_size_y;
extern int options_sdl_delay;
extern int options_sdl_player_delay;
extern int options_sdl_replay_delay;
extern int options_sdl_undo_delay;
extern int options_sdl_mouse;
extern int options_graphic_level;
extern int options_debug;
extern char options_graphics[];
extern char options_colours[];

extern char *piece_name[];

extern int move_x[];
extern int move_y[];

extern int display_offset_x;
extern int display_offset_y;
extern int display_offset_pixels_x;
extern int display_offset_pixels_y;
extern int display_start_x;
extern int display_start_y;
extern int display_end_x;
extern int display_end_y;
extern int display_pieces_x;
extern int display_pieces_y;
extern int display_focus_x;
extern int display_focus_y;
extern int display_bar_pixels;
extern int display_border_x;
extern int display_border_y;

extern float display_animation;
extern int display_animation_x;
extern int display_animation_y;

extern struct graphics* pdisplaygraphics;

void display_clip(struct level* plevel, int clip);
int display_bevelsquare(struct level* plevel, int x, int y);

void displayshadowed_level(struct level* plevel);
void displayshadowed_movers(struct level* plevel, int redraw);

static void displayshadowed_piece(struct level* plevel, int p, int x, int y, int d)
{
    SDL_Surface *pimage;

    SDL_Rect srect;
    SDL_Rect drect;
    int px, py;
    int alpha = 0;
    int bimage[4];
    int b;
    int bsizex, bsizey;
    int boffset = 0;
    int i;
    int xend;

#ifdef XOR_COMPATIBILITY
    if(p == PIECE_WALL && plevel->switched)
        return;
#endif

    px = x * pdisplaygraphics->size_x + display_offset_pixels_x; 
    py = y * pdisplaygraphics->size_y + display_offset_pixels_y; 

    if(d != MOVE_NONE)
    { 
        px += move_x[d] * display_animation_x;
        py += move_y[d] * display_animation_y;
    }

    if(isexplosion(p))
        alpha = 255 * (1 - display_animation);
    if(isnewexplosion(p))
    {
        alpha = 255 * display_animation;
        p += PIECE_EXPLOSION_FIRST - PIECE_EXPLOSION_NEW_FIRST;
    }

    pimage = pdisplaygraphics->image[p][IMAGE_PIECE];

    if(isexplosion(p))
        SDL_SetAlpha(pimage, SDL_SRCALPHA, alpha);

    srect.x = 0;
    srect.y = 0;
    srect.w = pdisplaygraphics->size_x;
    srect.h = pdisplaygraphics->size_y;

    drect.x = px;
    drect.y = py;
    drect.w = pdisplaygraphics->size_x;
    drect.h = pdisplaygraphics->size_y;

    if(pimage->w > pdisplaygraphics->size_x)
    {
        if(p == PIECE_PLAYER_ONE || p == PIECE_PLAYER_TWO)
        {
            if(plevel->player != (p & 1) && plevel->player != 2)
                srect.x = pdisplaygraphics->size_x;
        }
        /* Is the piece tiled? */
        if(pdisplaygraphics->image_flags[p] & GRAPHICS_TILE)
        {
            xend = pimage->w / pdisplaygraphics->size_x;
            if(pdisplaygraphics->image_flags[p] & GRAPHICS_BEVEL)
                xend -= 4;

            b = x % xend;
            if(b < 0)
                b += xend;
            srect.x += pdisplaygraphics->size_x * b;

            b = y % (pimage->h / pdisplaygraphics->size_y);
            if(b < 0)
                b += pimage->h / pdisplaygraphics->size_y;
            srect.y += pdisplaygraphics->size_y * b;
        }
    }

    /* Plot piece */
    SDL_BlitSurface(pimage, &srect, screen_surface, &drect);

    /* Plot bevelling */
    if(pdisplaygraphics->image_flags[p] & GRAPHICS_BEVEL)
    {
        xend = pimage->w / pdisplaygraphics->size_x;
        xend -=4;

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

                    SDL_BlitSurface(pimage, &srect, screen_surface, &drect);
                }
            }
        }
    }
}

static void displayshadowed_pieceshadow(struct level* plevel, int p, int x, int y, int d)
{
    SDL_Surface *pimage;

    SDL_Rect srect;
    SDL_Rect drect;
    int px, py;
    int alpha = 0;

    if(isexplosion(p))
        alpha = 255 * (1 - display_animation);
    if(isnewexplosion(p))
    {
        alpha = 255 * display_animation;
        p += PIECE_EXPLOSION_FIRST - PIECE_EXPLOSION_NEW_FIRST;
    }

    pimage = pdisplaygraphics->image[p][IMAGE_SHADOW];
    if(pimage == NULL)
        return;

    if(isexplosion(p))
        SDL_SetAlpha(pimage, SDL_SRCALPHA, alpha);

    px = x * pdisplaygraphics->size_x + display_offset_pixels_x; 
    py = y * pdisplaygraphics->size_y + display_offset_pixels_y; 
    if(d != MOVE_NONE)
    { 
        px += move_x[d] * display_animation_x;
        py += move_y[d] * display_animation_y;
    }

    srect.x = 0;
    srect.y = 0;
    srect.w = pdisplaygraphics->shadow_width[p][9];
    srect.h = pdisplaygraphics->shadow_height[p][9];

    drect.x = px + pdisplaygraphics->shadow_offset_x[p][9];
    drect.y = py + pdisplaygraphics->shadow_offset_y[p][9];
    drect.w = srect.w ;
    drect.h = srect.h;

    if(pimage->w > pdisplaygraphics->shadow_width[p][9])
    {
        if(p == PIECE_PLAYER_ONE || p == PIECE_PLAYER_TWO)
        {
            if(plevel->player != (p & 1) && plevel->player != 2)
                srect.x = pdisplaygraphics->shadow_width[p][9];
        }
    }

    /* Plot piece */
    SDL_BlitSurface(pimage, &srect, screen_surface, &drect);
}

static void displayshadowed_piecebase(struct level* plevel, int x, int y)
{
    int p;
    SDL_Surface *pimage;
    struct shadow *pshadow;
    struct shadow *pshadowstart;
    struct shadow *pshadowtmp;
    struct shadow *pshadowlast;
    int px, py;
    int z;
    int ok;
    int b, bp;
    int xend;

    SDL_Rect srect;
    SDL_Rect drect;
    SDL_Rect bsrect, bdrect;
    int alpha;

    p = level_piece(plevel, x, y);

    if(level_moving(plevel, x, y) != MOVE_NONE)
    {
        if(display_animation >= 1)
            p = PIECE_SPACE;
        else
            p = level_previous(plevel, x, y);
    }

#ifdef XOR_COMPATIBILITY
    if(plevel->switched && (p == PIECE_WALL || p == PIECE_SPACE))
        p = PIECE_DARKNESS;
#endif

    /* If the piece isn't transparent, nothing needs to be plotted */
    if(p != PIECE_SPACE
#ifdef XOR_COMPATIBILITY
            &&  p != PIECE_DARKNESS
#endif
            && !(pdisplaygraphics->image[p][IMAGE_PIECE]->flags & SDL_SRCALPHA))
        return;

#ifdef XOR_COMPATIBILITY
    if(plevel->switched)
        bp = PIECE_DARKNESS;
    else
#endif
        bp = PIECE_SPACE;

    pimage = pdisplaygraphics->image[bp][IMAGE_PIECE];

    px = x * pdisplaygraphics->size_x + display_offset_pixels_x; 
    py = y * pdisplaygraphics->size_y + display_offset_pixels_y; 

    srect.x = 0;
    srect.y = 0;
    srect.w = pdisplaygraphics->size_x;
    srect.h = pdisplaygraphics->size_y;

    drect.x = px;
    drect.y = py;
    drect.w = pdisplaygraphics->size_x;
    drect.h = pdisplaygraphics->size_y;

    /* Is the base tiled? */
    if(pdisplaygraphics->image_flags[bp] & GRAPHICS_TILE)
    {
        xend = pimage->w / pdisplaygraphics->size_x;
        if(pdisplaygraphics->image_flags[bp] & GRAPHICS_BEVEL)
            xend -= 4;
        srect.x += pdisplaygraphics->size_x * (x % xend);
        srect.y += pdisplaygraphics->size_y * (y % (pimage->h / pdisplaygraphics->size_y));
    }

    /* Plot the base */
    SDL_BlitSurface(pimage, &srect, screen_surface, &drect);

    /* Do we need to order the shadows prior to plotting? */
    if(pdisplaygraphics->flags & GRAPHICS_ZORDER)
    {
        pshadowstart = NULL;
        pshadowlast = NULL;

        pshadow = pdisplaygraphics->shadows;
        while(pshadow != NULL)
        {
            /* Determine which piece to consider the shadow of */
            p = level_piece(plevel, x - pshadow->x, y - pshadow->y);
            if(level_moving(plevel, x - pshadow->x, y - pshadow->y) != MOVE_NONE)
            {
                if(display_animation >= 1)
                    p = PIECE_SPACE;
                else
                {
                    if(level_previousmoving(plevel, x - pshadow->x, y - pshadow->y) == MOVE_NONE)
                        p = level_previous(plevel, x - pshadow->x, y - pshadow->y);
                    else
                        p = PIECE_SPACE;
                }
            }
#ifdef XOR_COMPATIBILITY
            if(p == PIECE_WALL && plevel->switched)
                p = PIECE_DARKNESS;
#endif
            /* Does it have a shadow? */
            if(pdisplaygraphics->image[p][IMAGE_SHADOW] != NULL && (pdisplaygraphics->shadow_flags[p] & pshadow->flag))
            {
                z = pdisplaygraphics->shadow_z[p];
                pshadow->z = pdisplaygraphics->shadow_z[p];
                pshadow->p = p;

                /* Put it in the ordered list */
                pshadowtmp = pshadowstart;
                while(pshadowtmp != NULL)
                {
                    if(pshadowtmp->z > z)
                        break;

                    pshadowtmp = pshadowtmp->nextordered;
                }
                if(pshadowstart == NULL)
                    pshadowstart = pshadow;

                /* It goes on the end of the list */
                if(pshadowtmp == NULL)
                {
                    pshadow->nextordered = NULL;
                    pshadow->previousordered = pshadowlast;
                    if(pshadowlast != NULL)
                        pshadowlast->nextordered = pshadow;
                    pshadowlast = pshadow;
                }
                /* It goes before pshadowtmp */
                else
                {
                    pshadow->nextordered = pshadowtmp;
                    pshadow->previousordered = pshadowtmp->previousordered;
                    if(pshadowtmp->previousordered != NULL)
                        pshadowtmp->previousordered->nextordered = pshadow;
                    else
                        pshadowstart = pshadow;
                    pshadowtmp->previousordered = pshadow;
                }
            }
            pshadow = pshadow->next;
        }

        /* Plot shadows with precalculated values */
        pshadow = pshadowstart;
        while(pshadow != NULL)
        {
            p = pshadow->p;
            pimage = pdisplaygraphics->image[p][IMAGE_SHADOW];

            if(isexplosion(p))
            {
                alpha = 255 * (1 - display_animation);
                SDL_SetAlpha(pdisplaygraphics->image[p][IMAGE_SHADOW], SDL_SRCALPHA, alpha);
            }
            if(isnewexplosion(p))
            {
                p += PIECE_EXPLOSION_FIRST - PIECE_EXPLOSION_NEW_FIRST;
                alpha = 255 * display_animation;
                SDL_SetAlpha(pdisplaygraphics->image[p][IMAGE_SHADOW], SDL_SRCALPHA, alpha);
            }

            srect.x = pdisplaygraphics->shadow_start_x[p][pshadow->shadow];
            srect.y = pdisplaygraphics->shadow_start_y[p][pshadow->shadow];
            srect.w = pdisplaygraphics->shadow_width[p][pshadow->shadow];
            srect.h = pdisplaygraphics->shadow_height[p][pshadow->shadow];

            drect.x = px + pdisplaygraphics->shadow_offset_x[p][pshadow->shadow];
            drect.y = py + pdisplaygraphics->shadow_offset_y[p][pshadow->shadow];
            drect.w = pdisplaygraphics->shadow_width[p][pshadow->shadow];
            drect.h = pdisplaygraphics->shadow_height[p][pshadow->shadow];


            /* Are there multiple images? */
            if(pimage->w > pdisplaygraphics->shadow_width[p][9])
            {   
                /* Choose swapped player if necessary */
                if(p == PIECE_PLAYER_ONE || p == PIECE_PLAYER_TWO)
                {   
                    if(plevel->player != (p & 1) && plevel->player != 2)
                        srect.x += pdisplaygraphics->shadow_width[p][9];
                }
            }

            /* Plot shadow */
            SDL_BlitSurface(pimage, &srect, screen_surface, &drect);

            /* Bevel shadow */
            if(pdisplaygraphics->image_flags[p] & GRAPHICS_BEVEL_SHADOW)
            {
                    b = level_data(plevel, x - pshadow->x, y - pshadow->y) & BEVEL_ALL;
                    /* Top left quadrant */
                    if(b & (BEVEL_L | BEVEL_U | BEVEL_TL))
                    {
                        bsrect.x = 0;
                        bsrect.y = 0;
                        bsrect.w = pdisplaygraphics->shadow_width[p][9] / 2;
                        bsrect.h = pdisplaygraphics->shadow_height[p][9] / 2;
                        bdrect.x = drect.x;
                        bdrect.y = drect.y;

                        ok = 1;
                        if(bsrect.x < srect.x)
                        {
                            if(bsrect.w > (srect.x - bsrect.x))
                            {
                                bsrect.w -= (srect.x - bsrect.x);
                                bdrect.x -= bsrect.x;
                                bsrect.x = srect.x;
                            }
                            else
                                ok = 0;
                        }
                        if(bsrect.y < srect.y)
                        {
                            if(bsrect.h > (srect.y - bsrect.y))
                            {
                                bsrect.h -= (srect.y - bsrect.y);
                                bdrect.y -= bsrect.y;
                                bsrect.y = srect.y;
                            }
                            else
                                ok = 0;
                        }
                        if(bdrect.x + bsrect.w > drect.x + drect.w)
                        {
                            if(bsrect.w > ((bdrect.x + bsrect.w) - (drect.x + drect.w)))
                                bsrect.w -= ((bdrect.x + bsrect.w) - (drect.x + drect.w));
                            else
                                ok = 0;
                        }
                        if(bdrect.y + bsrect.h > drect.y + drect.h)
                        {
                            if(bsrect.h > ((bdrect.y + bsrect.h) - (drect.y + drect.h)))
                                bsrect.h -= ((bdrect.y + bsrect.h) - (drect.y + drect.h));
                            else
                                ok = 0;
                        }
                        if(b & BEVEL_TL)
                            bsrect.x += pdisplaygraphics->shadow_width[p][9] * 4;
                        else
                        {
                            if(b & BEVEL_U)
                                bsrect.x += pdisplaygraphics->shadow_width[p][9] * 2;
                            if(b & BEVEL_L)
                                bsrect.x += pdisplaygraphics->shadow_width[p][9];
                        }

                        if(ok)
                            SDL_BlitSurface(pimage, &bsrect, screen_surface, &bdrect);
                    }

                    /* Top right quadrant */
                    if(b & (BEVEL_R | BEVEL_U | BEVEL_TR))
                    {
                        bsrect.x = pdisplaygraphics->shadow_width[p][9] / 2;
                        bsrect.y = 0;
                        bsrect.w = pdisplaygraphics->shadow_width[p][9] / 2;
                        bsrect.h = pdisplaygraphics->shadow_height[p][9] / 2;
                        bdrect.x = drect.x + pdisplaygraphics->size_x / 2;
                        bdrect.y = drect.y;

                        ok = 1;
                        if(bsrect.x < srect.x)
                        {
                            if(bsrect.w > (srect.x - bsrect.x))
                            {
                                bsrect.w -= (srect.x - bsrect.x);
                                bdrect.x -= bsrect.x;
                                bsrect.x = srect.x;
                            }
                            else
                                ok = 0;
                        }
                        if(bsrect.y < srect.y)
                        {
                            if(bsrect.h > (srect.y - bsrect.y))
                            {
                                bsrect.h -= (srect.y - bsrect.y);
                                bdrect.y -= bsrect.y;
                                bsrect.y = srect.y;
                            }
                            else
                                ok = 0;
                        }
                        if(bdrect.x + bsrect.w > drect.x + drect.w)
                        {
                            if(bsrect.w > ((bdrect.x + bsrect.w) - (drect.x + drect.w)))
                                bsrect.w -= ((bdrect.x + bsrect.w) - (drect.x + drect.w));
                            else
                                ok = 0;
                        }
                        if(bdrect.y + bsrect.h > drect.y + drect.h)
                        {
                            if(bsrect.h > ((bdrect.y + bsrect.h) - (drect.y + drect.h)))
                                bsrect.h -= ((bdrect.y + bsrect.h) - (drect.y + drect.h));
                            else
                                ok = 0;
                        }
                        if(b & BEVEL_TR)
                            bsrect.x += pdisplaygraphics->shadow_width[p][9] * 4;
                        else
                        {
                            if(b & BEVEL_U)
                                bsrect.x += pdisplaygraphics->shadow_width[p][9] * 2;
                            if(b & BEVEL_R)
                                bsrect.x += pdisplaygraphics->shadow_width[p][9];
                        }
                        if(ok)
                            SDL_BlitSurface(pimage, &bsrect, screen_surface, &bdrect);
                    }

                    /* Bottom left quadrant */
                    if(b & (BEVEL_L | BEVEL_D | BEVEL_BL))
                    {
                        bsrect.x = 0;
                        bsrect.y = pdisplaygraphics->shadow_height[p][9] / 2;;
                        bsrect.w = pdisplaygraphics->shadow_width[p][9] / 2;
                        bsrect.h = pdisplaygraphics->shadow_height[p][9] / 2;
                        bdrect.x = drect.x;
                        bdrect.y = drect.y + pdisplaygraphics->size_y / 2;

                        ok = 1;
                        if(bsrect.x < srect.x)
                        {
                            if(bsrect.w > (srect.x - bsrect.x))
                            {
                                bsrect.w -= (srect.x - bsrect.x);
                                bdrect.x -= bsrect.x;
                                bsrect.x = srect.x;
                            }
                            else
                                ok = 0;
                        }
                        if(bsrect.y < srect.y)
                        {
                            if(bsrect.h > (srect.y - bsrect.y))
                            {
                                bsrect.h -= (srect.y - bsrect.y);
                                bdrect.y -= bsrect.y;
                                bsrect.y = srect.y;
                            }
                            else
                                ok = 0;
                        }
                        if(bdrect.x + bsrect.w > drect.x + drect.w)
                        {
                            if(bsrect.w > ((bdrect.x + bsrect.w) - (drect.x + drect.w)))
                                bsrect.w -= ((bdrect.x + bsrect.w) - (drect.x + drect.w));
                            else
                                ok = 0;
                        }
                        if(bdrect.y + bsrect.h > drect.y + drect.h)
                        {
                            if(bsrect.h > ((bdrect.y + bsrect.h) - (drect.y + drect.h)))
                                bsrect.h -= ((bdrect.y + bsrect.h) - (drect.y + drect.h));
                            else
                                ok = 0;
                        }
                        if(b & BEVEL_BL)
                            bsrect.x += pdisplaygraphics->shadow_width[p][9] * 4;
                        else
                        {
                            if(b & BEVEL_D)
                                bsrect.x += pdisplaygraphics->shadow_width[p][9] * 2;
                            if(b & BEVEL_L)
                                bsrect.x += pdisplaygraphics->shadow_width[p][9];
                        }
                        if(ok)
                            SDL_BlitSurface(pimage, &bsrect, screen_surface, &bdrect);
                    }

                    /* Bottom right quadrant */
                    if(b & (BEVEL_R | BEVEL_D | BEVEL_BR))
                    {
                        bsrect.x = pdisplaygraphics->shadow_width[p][9] / 2;
                        bsrect.y = pdisplaygraphics->shadow_height[p][9] / 2;
                        bsrect.w = pdisplaygraphics->shadow_width[p][9] / 2;
                        bsrect.h = pdisplaygraphics->shadow_height[p][9] / 2;
                        bdrect.x = drect.x + pdisplaygraphics->size_x / 2;
                        bdrect.y = drect.y + pdisplaygraphics->size_y / 2;

                        ok = 1;
                        if(bsrect.x < srect.x)
                        {
                            if(bsrect.w > (srect.x - bsrect.x))
                            {
                                bsrect.w -= (srect.x - bsrect.x);
                                bdrect.x -= bsrect.x;
                                bsrect.x = srect.x;
                            }
                            else
                                ok = 0;
                        }
                        if(bsrect.y < srect.y)
                        {
                            if(bsrect.h > (srect.y - bsrect.y))
                            {
                                bsrect.h -= (srect.y - bsrect.y);
                                bdrect.y -= bsrect.y;
                                bsrect.y = srect.y;
                            }
                            else
                                ok = 0;
                        }
                        if(bdrect.x + bsrect.w > drect.x + drect.w)
                        {
                            if(bsrect.w > ((bdrect.x + bsrect.w) - (drect.x + drect.w)))
                                bsrect.w -= ((bdrect.x + bsrect.w) - (drect.x + drect.w));
                            else
                                ok = 0;
                        }
                        if(bdrect.y + bsrect.h > drect.y + drect.h)
                        {
                            if(bsrect.h > ((bdrect.y + bsrect.h) - (drect.y + drect.h)))
                                bsrect.h -= ((bdrect.y + bsrect.h) - (drect.y + drect.h));
                            else
                                ok = 0;
                        }
                        if(b & BEVEL_BR)
                            bsrect.x += pdisplaygraphics->shadow_width[p][9] * 4;
                        else
                        {
                            if(b & BEVEL_D)
                                bsrect.x += pdisplaygraphics->shadow_width[p][9] * 2;
                            if(b & BEVEL_R)
                                bsrect.x += pdisplaygraphics->shadow_width[p][9];
                        }
                        if(ok)
                            SDL_BlitSurface(pimage, &bsrect, screen_surface, &bdrect);
                    }
            }

            pshadow = pshadow->nextordered;
        }
    }
    else
    {
        /* Plot shadows in the order specified in the graphics file */
        pshadow = pdisplaygraphics->shadows;
        while(pshadow != NULL)
        {
            /* Determine which piece to consider the shadow of */
            p = level_piece(plevel, x - pshadow->x, y - pshadow->y);
            if(level_moving(plevel, x - pshadow->x, y - pshadow->y) != MOVE_NONE)
            {
                if(display_animation >= 1)
                    p = PIECE_SPACE;
                else
                {
                    if(level_previousmoving(plevel, x - pshadow->x, y - pshadow->y) == MOVE_NONE)
                        p = level_previous(plevel, x - pshadow->x, y - pshadow->y);
                    else
                        p = PIECE_SPACE;
                }
            }
            /* Does it have a shadow? */
            if(pdisplaygraphics->image[p][IMAGE_SHADOW] != NULL && (pdisplaygraphics->shadow_flags[p] & pshadow->flag))
            {
                pimage = pdisplaygraphics->image[p][IMAGE_SHADOW];
    
                if(isexplosion(p))
                {
                    alpha = 255 * (1 - display_animation);
                    SDL_SetAlpha(pdisplaygraphics->image[p][IMAGE_SHADOW], SDL_SRCALPHA, alpha);
                }
                if(isnewexplosion(p))
                {
                    p += PIECE_EXPLOSION_FIRST - PIECE_EXPLOSION_NEW_FIRST;
                    alpha = 255 * display_animation;
                    SDL_SetAlpha(pdisplaygraphics->image[p][IMAGE_SHADOW], SDL_SRCALPHA, alpha);
                }

                srect.x = pdisplaygraphics->shadow_start_x[p][pshadow->shadow];
                srect.y = pdisplaygraphics->shadow_start_y[p][pshadow->shadow];
                srect.w = pdisplaygraphics->shadow_width[p][pshadow->shadow];
                srect.h = pdisplaygraphics->shadow_height[p][pshadow->shadow];
    
                drect.x = px + pdisplaygraphics->shadow_offset_x[p][pshadow->shadow];
                drect.y = py + pdisplaygraphics->shadow_offset_y[p][pshadow->shadow];
                drect.w = pdisplaygraphics->shadow_width[p][pshadow->shadow];
                drect.h = pdisplaygraphics->shadow_height[p][pshadow->shadow];

                /* Are there multiple images? */
                if(pimage->w > pdisplaygraphics->shadow_width[p][9])
                {   
                    /* Choose swapped player if necessary */
                    if(p == PIECE_PLAYER_ONE || p == PIECE_PLAYER_TWO)
                    {   
                        if(plevel->player != (p & 1) && plevel->player != 2)
                            srect.x += pdisplaygraphics->shadow_width[p][9];
                    } 
                }

                /* Plot shadow */
                SDL_BlitSurface(pimage, &srect, screen_surface, &drect);

                /* Bevel shadow */
                if(pdisplaygraphics->image_flags[p] & GRAPHICS_BEVEL_SHADOW)
                {
                        b = level_data(plevel, x - pshadow->x, y - pshadow->y) & BEVEL_ALL;
                        /* Top left quadrant */
                        if(b & (BEVEL_L | BEVEL_U | BEVEL_TL))
                        {
                            bsrect.x = 0;
                            bsrect.y = 0;
                            bsrect.w = pdisplaygraphics->shadow_width[p][9] / 2;
                            bsrect.h = pdisplaygraphics->shadow_height[p][9] / 2;
                            bdrect.x = drect.x;
                            bdrect.y = drect.y;

                            ok = 1;
                            if(bsrect.x < srect.x)
                            {
                                if(bsrect.w > (srect.x - bsrect.x))
                                {
                                    bsrect.w -= (srect.x - bsrect.x);
                                    bdrect.x -= bsrect.x;
                                    bsrect.x = srect.x;
                                }
                                else
                                    ok = 0;
                            }
                            if(bsrect.y < srect.y)
                            {
                                if(bsrect.h > (srect.y - bsrect.y))
                                {
                                    bsrect.h -= (srect.y - bsrect.y);
                                    bdrect.y -= bsrect.y;
                                    bsrect.y = srect.y;
                                }
                                else
                                    ok = 0;
                            }
                            if(bdrect.x + bsrect.w > drect.x + drect.w)
                            {
                                if(bsrect.w > ((bdrect.x + bsrect.w) - (drect.x + drect.w)))
                                    bsrect.w -= ((bdrect.x + bsrect.w) - (drect.x + drect.w));
                                else
                                    ok = 0;
                            }
                            if(bdrect.y + bsrect.h > drect.y + drect.h)
                            {
                                if(bsrect.h > ((bdrect.y + bsrect.h) - (drect.y + drect.h)))
                                    bsrect.h -= ((bdrect.y + bsrect.h) - (drect.y + drect.h));
                                else
                                    ok = 0;
                            }
                            if(b & BEVEL_TL)
                                bsrect.x += pdisplaygraphics->shadow_width[p][9] * 4;
                            else
                            {
                                if(b & BEVEL_U)
                                    bsrect.x += pdisplaygraphics->shadow_width[p][9] * 2;
                                if(b & BEVEL_L)
                                    bsrect.x += pdisplaygraphics->shadow_width[p][9];
                            }

                            if(ok)
                                SDL_BlitSurface(pimage, &bsrect, screen_surface, &bdrect);
                        }

                        /* Top right quadrant */
                        if(b & (BEVEL_R | BEVEL_U | BEVEL_TR))
                        {
                            bsrect.x = pdisplaygraphics->shadow_width[p][9] / 2;
                            bsrect.y = 0;
                            bsrect.w = pdisplaygraphics->shadow_width[p][9] / 2;
                            bsrect.h = pdisplaygraphics->shadow_height[p][9] / 2;
                            bdrect.x = drect.x + pdisplaygraphics->size_x / 2;
                            bdrect.y = drect.y;
    
                            ok = 1;
                            if(bsrect.x < srect.x)
                            {
                                if(bsrect.w > (srect.x - bsrect.x))
                                {
                                    bsrect.w -= (srect.x - bsrect.x);
                                    bdrect.x -= bsrect.x;
                                    bsrect.x = srect.x;
                                }
                                else
                                    ok = 0;
                            }
                            if(bsrect.y < srect.y)
                            {
                                if(bsrect.h > (srect.y - bsrect.y))
                                {
                                    bsrect.h -= (srect.y - bsrect.y);
                                    bdrect.y -= bsrect.y;
                                    bsrect.y = srect.y;
                                }
                                else
                                    ok = 0;
                            }
                            if(bdrect.x + bsrect.w > drect.x + drect.w)
                            {
                                if(bsrect.w > ((bdrect.x + bsrect.w) - (drect.x + drect.w)))
                                    bsrect.w -= ((bdrect.x + bsrect.w) - (drect.x + drect.w));
                                else
                                    ok = 0;
                            }
                            if(bdrect.y + bsrect.h > drect.y + drect.h)
                            {
                                if(bsrect.h > ((bdrect.y + bsrect.h) - (drect.y + drect.h)))
                                    bsrect.h -= ((bdrect.y + bsrect.h) - (drect.y + drect.h));
                                else
                                    ok = 0;
                            }
                            if(b & BEVEL_TR)
                                bsrect.x += pdisplaygraphics->shadow_width[p][9] * 4;
                            else
                            {
                                if(b & BEVEL_U)
                                    bsrect.x += pdisplaygraphics->shadow_width[p][9] * 2;
                                if(b & BEVEL_R)
                                    bsrect.x += pdisplaygraphics->shadow_width[p][9];
                            }
                            if(ok)
                                SDL_BlitSurface(pimage, &bsrect, screen_surface, &bdrect);
                        }

                        /* Bottom left quadrant */
                        if(b & (BEVEL_L | BEVEL_D | BEVEL_BL))
                        {
                            bsrect.x = 0;
                            bsrect.y = pdisplaygraphics->shadow_height[p][9] / 2;;
                            bsrect.w = pdisplaygraphics->shadow_width[p][9] / 2;
                            bsrect.h = pdisplaygraphics->shadow_height[p][9] / 2;
                            bdrect.x = drect.x;
                            bdrect.y = drect.y + pdisplaygraphics->size_y / 2;

                            ok = 1;
                            if(bsrect.x < srect.x)
                            {
                                if(bsrect.w > (srect.x - bsrect.x))
                                {
                                    bsrect.w -= (srect.x - bsrect.x);
                                    bdrect.x -= bsrect.x;
                                    bsrect.x = srect.x;
                                }
                                else
                                    ok = 0;
                            }
                            if(bsrect.y < srect.y)
                            {
                                if(bsrect.h > (srect.y - bsrect.y))
                                {
                                    bsrect.h -= (srect.y - bsrect.y);
                                    bdrect.y -= bsrect.y;
                                    bsrect.y = srect.y;
                                }
                                else
                                    ok = 0;
                            }
                            if(bdrect.x + bsrect.w > drect.x + drect.w)
                            {
                                if(bsrect.w > ((bdrect.x + bsrect.w) - (drect.x + drect.w)))
                                    bsrect.w -= ((bdrect.x + bsrect.w) - (drect.x + drect.w));
                                else
                                    ok = 0;
                            }
                            if(bdrect.y + bsrect.h > drect.y + drect.h)
                            {
                                if(bsrect.h > ((bdrect.y + bsrect.h) - (drect.y + drect.h)))
                                    bsrect.h -= ((bdrect.y + bsrect.h) - (drect.y + drect.h));
                                else
                                    ok = 0;
                            }
                            if(b & BEVEL_BL)
                                bsrect.x += pdisplaygraphics->shadow_width[p][9] * 4;
                            else
                            {
                                if(b & BEVEL_D)
                                    bsrect.x += pdisplaygraphics->shadow_width[p][9] * 2;
                                if(b & BEVEL_L)
                                    bsrect.x += pdisplaygraphics->shadow_width[p][9];
                            }
                            if(ok)
                                SDL_BlitSurface(pimage, &bsrect, screen_surface, &bdrect);
                        }

                        /* Bottom right quadrant */
                        if(b & (BEVEL_R | BEVEL_D | BEVEL_BR))
                        {
                            bsrect.x = pdisplaygraphics->shadow_width[p][9] / 2;
                            bsrect.y = pdisplaygraphics->shadow_height[p][9] / 2;
                            bsrect.w = pdisplaygraphics->shadow_width[p][9] / 2;
                            bsrect.h = pdisplaygraphics->shadow_height[p][9] / 2;
                            bdrect.x = drect.x + pdisplaygraphics->size_x / 2;
                            bdrect.y = drect.y + pdisplaygraphics->size_y / 2;

                            ok = 1;
                            if(bsrect.x < srect.x)
                            {
                                if(bsrect.w > (srect.x - bsrect.x))
                                {
                                    bsrect.w -= (srect.x - bsrect.x);
                                    bdrect.x -= bsrect.x;
                                    bsrect.x = srect.x;
                                }
                                else
                                    ok = 0;
                            }
                            if(bsrect.y < srect.y)
                            {
                                if(bsrect.h > (srect.y - bsrect.y))
                                {
                                    bsrect.h -= (srect.y - bsrect.y);
                                    bdrect.y -= bsrect.y;
                                    bsrect.y = srect.y;
                                }
                                else
                                    ok = 0;
                            }
                            if(bdrect.x + bsrect.w > drect.x + drect.w)
                            {
                                if(bsrect.w > ((bdrect.x + bsrect.w) - (drect.x + drect.w)))
                                    bsrect.w -= ((bdrect.x + bsrect.w) - (drect.x + drect.w));
                                else
                                    ok = 0;
                            }
                            if(bdrect.y + bsrect.h > drect.y + drect.h)
                            {
                                if(bsrect.h > ((bdrect.y + bsrect.h) - (drect.y + drect.h)))
                                    bsrect.h -= ((bdrect.y + bsrect.h) - (drect.y + drect.h));
                                else
                                    ok = 0;
                            }
                            if(b & BEVEL_BR)
                                bsrect.x += pdisplaygraphics->shadow_width[p][9] * 4;
                            else
                            {
                                if(b & BEVEL_D)
                                    bsrect.x += pdisplaygraphics->shadow_width[p][9] * 2;
                                if(b & BEVEL_R)
                                    bsrect.x += pdisplaygraphics->shadow_width[p][9];
                            }
                            if(ok)
                                SDL_BlitSurface(pimage, &bsrect, screen_surface, &bdrect);
                    }
                }

            }
            pshadow = pshadow->next;
        }
    }
}

static void displayshadowed_redrawpiece(int p, int x, int y, int d)
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

void displayshadowed_level(struct level* plevel)
{
    int x, y;
    int p;

    display_clip(plevel, 1);

    /* Plot base */
    for(y = display_start_y; y < display_end_y; y ++)
    {
        for(x = display_start_x; x < display_end_x; x ++)
        {
            displayshadowed_piecebase(plevel, x, y);

            p = level_piece(plevel, x, y);

            /* Plot the piece itself */
            if(p != PIECE_SPACE)
                 displayshadowed_piece(plevel, p, x, y, MOVE_NONE);
        }
    }

    if(plevel->mover_first != NULL)
        displayshadowed_movers(plevel, 0);

    display_clip(plevel, 0);

    screen_redraw(0, 0, screen_width, screen_height);
}

static int displayshadowed_count(struct level* plevel, int x, int y, int delta)
{
    unsigned int d;

    d = level_data(plevel, x, y);
    level_setdata(plevel, x, y, d + (delta * SHADOW_BASE));

    return d & (0x7f * SHADOW_BASE);
}


void displayshadowed_movers(struct level* plevel, int redraw)
{
    struct shadow* pshadow;
    struct mover* pmover;
    int x, y, p, pm;
    int i, j;
    char buffer[16];
    int d;
    int bevel;
    int bevelold;
    SDL_Surface *psurface;
    SDL_Rect srect, drect;

    p = PIECE_SPACE;

    display_animation_x = - pdisplaygraphics->size_x + (int)((float) pdisplaygraphics->size_x * display_animation);
    display_animation_y = - pdisplaygraphics->size_y + (int)((float) pdisplaygraphics->size_y * display_animation);

    display_clip(plevel, 1);

    /* Stage one: plot base for pieces that need rebevelling */
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
                        bevelold = (level_data(plevel, x + i, y + j) & BEVEL_ALL);
                        /* Because this happens only once per move cycle, we
                           are lazy and don't bother to count whether this base
                           has already been plotted */
                        if(bevel != bevelold);
                            displayshadowed_piecebase(plevel, pmover->x + i, pmover->y + j);
                    }
                }
            }
            pmover = pmover->next;
        }
    }

    /* Stage two: plot shadows for stationary squares affected by movers */
    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
        if(pmover->piece != PIECE_GONE)
        {
            /* This is overkill, but it's easier just to plot everything that
               could be affected than to calculate what is actually affected.
               */
            pshadow = pdisplaygraphics->shadows;
            while(pshadow != NULL)
            {
                if(displayshadowed_count(plevel, pmover->x + pshadow->x, pmover->y + pshadow->y, 1) == 0)
                    displayshadowed_piecebase(plevel, pmover->x + pshadow->x, pmover->y + pshadow->y);
                pshadow = pshadow->next;
            }
            if(displayshadowed_count(plevel, pmover->x, pmover->y, 1) == 0)
                displayshadowed_piecebase(plevel, pmover->x, pmover->y);
        }
        
        pmover = pmover->next;
    }
    /* then reset the counts */
    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
        if(pmover->piece != PIECE_GONE)
        {
            pshadow = pdisplaygraphics->shadows;
            while(pshadow != NULL)
            {
                displayshadowed_count(plevel, pmover->x + pshadow->x, pmover->y + pshadow->y, -1);
                pshadow = pshadow->next;
            }
            displayshadowed_count(plevel, pmover->x, pmover->y, -1);
        }
        
        pmover = pmover->next;
    }

    /* Stage three: plot shadows for movers */
    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
        d = pmover->direction;
        x = pmover->x;
        y = pmover->y;

        if(isexplosion(pmover->piece))
        {
            /* If the previous piece, that is, the piece destroyed in the
               explosion, is stationary, we don't need to plot a shadow for it,
               as that is handled in stage one. If it is moving, however, it
               needs a moving shadow, which we do have to plot here. */
            p = level_previous(plevel, x, y);
            pm = level_previousmoving(plevel, x, y);
            if(p != PIECE_SPACE && pm != MOVE_NONE)
                displayshadowed_pieceshadow(plevel, p, x, y, pm);

            /* Plot shadow for the detonator */
            p = level_detonator(plevel, x, y);
            pm = level_detonatormoving(plevel, x, y);
            if(p != PIECE_SPACE)
                displayshadowed_pieceshadow(plevel, p, x, y, pm);

        }
        /* Spaces and walls were handled in stage one */
        else if(pmover->piece != PIECE_SPACE && pmover->piece != PIECE_WALL && pmover->piece != PIECE_GONE)
        {
            /* We don't need to plot the shadow for the previous piece
               as that is handled in stage one */

            /* Plot shadow for mover */
#ifdef XOR_COMPATIBILITY
            if(pmover->piece == PIECE_TELEPORT)
                d = MOVE_NONE;
#endif
            displayshadowed_pieceshadow(plevel, pmover->piece, pmover->x, pmover->y, d);
        }
        pmover = pmover->next;
    }

    /* Stage four: plot shadows for explosions */
    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
        x = pmover->x;
        y = pmover->y;
        /* Plot growing explosion */
        if(isexplosion(pmover->piece))
            displayshadowed_pieceshadow(plevel, pmover->piece + PIECE_EXPLOSION_NEW_FIRST - PIECE_EXPLOSION_FIRST, x, y, MOVE_NONE);

        /* Plot dying explosion */
        p = level_previous(plevel, x, y);
        if(isexplosion(p) && display_animation < 1)
            displayshadowed_pieceshadow(plevel, p, x, y, MOVE_NONE);

        pmover = pmover->next;
    }

    /* Stage five: plot pieces for stationary squares affected by movers.
       We need to be careful not to plot the same piece twice, so we keep
       count and only plot on the first occurrence. */
    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
        for(i = -1; i < 2; i ++)
        {
            for(j = -1; j < 2; j ++)
            {
                if(displayshadowed_count(plevel, pmover->x + i, pmover->y + j, 1) == 0)
                {
                    p = level_piece(plevel, pmover->x + i, pmover->y + j);
                    pm = level_moving(plevel, pmover->x + i, pmover->y + j);
                    if(p != PIECE_SPACE && p != PIECE_GONE && pm == MOVE_NONE)
                        displayshadowed_piece(plevel, p, pmover->x + i, pmover->y + j, MOVE_NONE);
                }
            }
        }
        pmover = pmover->next;
    }
    /* then reset the counts */
    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
        for(i = -1; i < 2; i ++)
        {
            for(j = -1; j < 2; j ++)
            {
                displayshadowed_count(plevel, pmover->x + i, pmover->y + j, -1);
            }
        }
        pmover = pmover->next;
    }

    /* Stage six: plot pieces for movers */
    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
        d = pmover->direction;
        x = pmover->x;
        y = pmover->y;

        if(isexplosion(pmover->piece))
        {
            /* Plot any piece destroyed by the explosion, or the bomb itself */
            p = level_previous(plevel, x, y);
            pm = level_previousmoving(plevel, x, y);
            if(p != PIECE_SPACE)
                displayshadowed_piece(plevel, p, x, y, pm);

            /* Plot the detonator */
            p = level_detonator(plevel, x, y);
            pm = level_detonatormoving(plevel, x, y);
            if(p != PIECE_SPACE)
                displayshadowed_piece(plevel, p, x, y, pm);

        }
        /* Spaces were handled in stage one */
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
                    displayshadowed_piece(plevel, p, x, y, pm);
            }

            /* Plot the piece itself */
#ifdef XOR_COMPATIBILITY
            if(pmover->piece == PIECE_TELEPORT)
                d = MOVE_NONE;
#endif
            displayshadowed_piece(plevel, pmover->piece, pmover->x, pmover->y, d);
        }
        pmover = pmover->next;
    }

    /* Stage seven: plot pieces that need rebevelling */
    if(display_animation == 0 || display_animation == 1)
    {
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
                        bevelold = (level_data(plevel, x + i, y + j) & BEVEL_ALL);
                        if(bevel != bevelold);
                        {
                            /* Here we are not lazy, to avoid issues with
                               transparent graphics being plotted twice */
                            if(displayshadowed_count(plevel, pmover->x + i, pmover->y + j, 1) == 0)
                            {
                                level_setdata(plevel, x + i, y + j, bevel | (level_data(plevel, x + i, y + j) & ~BEVEL_ALL));
                                p = level_piece(plevel, x + i, y + j);
                                if(p == PIECE_WALL)
                                    displayshadowed_piece(plevel, p, pmover->x + i, pmover->y + j, MOVE_NONE);
                                level_setdata(plevel, x + i, y + j, bevelold | (level_data(plevel, x + i, y + j) & ~BEVEL_ALL));
                            }
                        }
                    }
                }
            }
            pmover = pmover->next;
        }
        /* and reset counts */
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
                        bevelold = (level_data(plevel, x + i, y + j) & BEVEL_ALL);
                        if(bevel != bevelold);
                            displayshadowed_count(plevel, pmover->x + i, pmover->y + j, -1);
                    }
                }
            }
            pmover = pmover->next;
        }
    }

    /* Stage eight: plot pieces for explosions */
    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
        x = pmover->x;
        y = pmover->y;
        /* Plot growing explosion */
        if(isexplosion(pmover->piece))
            displayshadowed_piece(plevel, pmover->piece + PIECE_EXPLOSION_NEW_FIRST - PIECE_EXPLOSION_FIRST, x, y, MOVE_NONE);

        /* Plot dying explosion */
        p = level_previous(plevel, x, y);
        if(isexplosion(p) && display_animation < 1)
            displayshadowed_piece(plevel, p, x, y, MOVE_NONE);

        pmover = pmover->next;
    }

    /* Stage nine: plot order of movers if debugging (but not in editor) */
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
            pmover = pmover->next;
        }
    }

    display_clip(plevel, 0);

    if(redraw == 0)
        return;

    display_clip(plevel, 1);

    /* Stage ten: redraw modified squares */
    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
        if(pmover->piece != PIECE_GONE)
        {
            pshadow = pdisplaygraphics->shadows;
            while(pshadow != NULL)
            {
                if(displayshadowed_count(plevel, pmover->x + pshadow->x, pmover->y + pshadow->y, 1) == 0)
                    displayshadowed_redrawpiece(level_piece(plevel, pmover->x + pshadow->x, pmover->y + pshadow->y), pmover->x + pshadow->x, pmover->y + pshadow->y, MOVE_NONE);
                pshadow = pshadow->next;
            }
            if(displayshadowed_count(plevel, pmover->x, pmover->y, 1) == 0)
                displayshadowed_redrawpiece(level_piece(plevel, pmover->x, pmover->y), pmover->x, pmover->y, MOVE_NONE);
        }
        pmover = pmover->next;
    }
    /* then reset the counts */
    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
        if(pmover->piece != PIECE_GONE)
        {
            pshadow = pdisplaygraphics->shadows;
            while(pshadow != NULL)
            {
                displayshadowed_count(plevel, pmover->x + pshadow->x, pmover->y + pshadow->y, -1);
                pshadow = pshadow->next;
            }
            displayshadowed_count(plevel, pmover->x, pmover->y, -1);
        }
        pmover = pmover->next;
    }

    /* Stage eleven: redraw pieces that need rebevelling */
    if(display_animation == 0 || display_animation == 1)
    {
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
                        bevelold = (level_data(plevel, x + i, y + j) & BEVEL_ALL);
                        if(bevel != bevelold);
                        {
                            /* Again, we are lazy and do not keep count of
                               whether this square has already been redrawn. */
                            level_setdata(plevel, x + i, y + j, bevel | (level_data(plevel, x + i, y + j) & ~BEVEL_ALL));
                            displayshadowed_redrawpiece(level_piece(plevel, pmover->x + i, pmover->y + j), pmover->x + i, pmover->y + j, MOVE_NONE);
                        }
                    }
                }
            }
            pmover = pmover->next;
        }
    }

    display_clip(plevel, 0);
}
