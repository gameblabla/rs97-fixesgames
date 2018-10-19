/*  
    sdlfont.c

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

#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <libintl.h>
#include <locale.h>

#include "util.h"

/* If USE_FREETYPE is defined, a pleasing stroked font is used. Alternative
   code is provided to fall back to SDL_ttf, but this is plain by comparison.
   */
#define USE_FREETYPE

#ifdef USE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#else
#include <SDL/SDL_ttf.h>
#endif

void font_init();
void font_set_size(int size);
void font_box(int x, int y, int w, int h, int c);
int font_calculate_width(char *text, int cache);

extern struct SDL_Surface* psurfacelogo;
extern struct SDL_Surface* psurfacelogosmall;
extern int screen_width;

int font_colours[48][3] = {
/* normal */
    {0xcc, 0xcc, 0xcc}, {0x7f, 0x7f, 0x7f}, /* 0 */
    {0xff, 0x99, 0x99}, {0xff, 0x00, 0x00}, /* 1 */
    {0x99, 0xff, 0x99}, {0x00, 0xcc, 0x00}, /* 2 */
    {0xff, 0xff, 0x33}, {0xff, 0xa0, 0x00}, /* 3 */
    {0x99, 0x99, 0xff}, {0x00, 0x00, 0xff}, /* 4 */
    {0xcc, 0x99, 0xff}, {0x80, 0x00, 0xff}, /* 5 */
    {0x66, 0xb3, 0xff}, {0x00, 0x80, 0xff}, /* 6 */
    {0xff, 0xff, 0xff}, {0x7f, 0x7f, 0x7f}, /* 7 */

/* light */
    {0xee, 0xee, 0xee}, {0x7f, 0x7f, 0x7f}, /* 0 */
    {0xff, 0xcc, 0xcc}, {0xff, 0x00, 0x00}, /* 1 */
    {0xcc, 0xff, 0xcc}, {0x00, 0xcc, 0x00}, /* 2 */
    {0xff, 0xff, 0x99}, {0xff, 0xa0, 0x00}, /* 3 */
    {0xcc, 0xcc, 0xff}, {0x00, 0x00, 0xff}, /* 4 */
    {0xe6, 0xcc, 0xff}, {0x80, 0x00, 0xff}, /* 5 */
    {0xb3, 0xd9, 0xff}, {0x00, 0x80, 0xff}, /* 6 */
    {0xff, 0xff, 0xff}, {0x7f, 0x7f, 0x7f}, /* 7 */

/* bold */
    {0xaa, 0xaa, 0xaa}, {0x7f, 0x7f, 0x7f}, /* 0 */
    {0xff, 0x5c, 0x5c}, {0xff, 0x00, 0x00}, /* 1 */
    {0x5c, 0xff, 0x5c}, {0x00, 0xcc, 0x00}, /* 2 */
    {0xff, 0xc0, 0x00}, {0xff, 0xa0, 0x00}, /* 3 */
    {0x5c, 0x5c, 0xff}, {0x00, 0x00, 0xff}, /* 4 */
    {0xae, 0x5c, 0xff}, {0x80, 0x00, 0xff}, /* 5 */
    {0x29, 0x94, 0xff}, {0x00, 0x80, 0xff}, /* 6 */
    {0xff, 0xff, 0xff}, {0x00, 0x00, 0x00}  /* 7 white, black outline */
};

                         /* chroma 1.01 : */
char font_logo_colours[] = "1326454646644";

int font_border = 0;
int font_border_partial = 0;
int font_padding = 0;

extern SDL_Surface* screen_surface;

int font_size_menu = 0;
int font_size_game = 0;
int font_height_menu = 0;
int font_height_game = 0;

#ifdef USE_FREETYPE

/* ************ FreeType version ************ */

FT_Library font_library;
FT_Face font_face;
FT_Stroker font_stroker;
int font_height;
int font_width;
int font_descent;
int font_size;

#define FT_CEIL(X)      (((X + 63) & -64) / 64)

void font_init()
{
    char directory[FILENAME_MAX];
    char filename[FILENAME_MAX];
    FT_Error error;

    getfilename("graphics", directory, 0, LOCATION_SYSTEM);

    sprintf(filename, "%s/font.ttf", directory);

    if((error = FT_Init_FreeType(&font_library)) != 0)
    {
        sprintf(filename, gettext("Unable to initialise freetype: error=%d"), error);
        fatal(filename);
    }
    if((error = FT_New_Face(font_library, filename, 0, &font_face)) != 0)
    {
printf("%s\n", filename);
        sprintf(filename, gettext("Unable to load font: error=%d"), error);
        fatal(filename);
    }

    font_stroker = NULL;
    font_height = 0;
    font_descent = 0;
    font_width = 0;
    font_size = 0;
}

void font_set_size(int size)
{
    FT_Fixed scale;

    if(size == font_size)
        return;

    if(FT_Set_Char_Size(font_face, 0, size * 64, 0, 0))
    {
        fatal(gettext("Unable to set font size"));
    }
    if(font_stroker != NULL)
    {
        FT_Stroker_Done(font_stroker);
    }
    if(FT_Stroker_New(font_library, &font_stroker))
    {
        fatal(gettext("Unable to create stroker"));
    }
    FT_Stroker_Set(font_stroker, size * 2, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

    font_size = size;
    scale = font_face->size->metrics.y_scale;
    font_height = FT_CEIL(FT_MulFix(font_face->ascender, scale)) - FT_CEIL(FT_MulFix(font_face->descender, scale)) + 1;
    font_descent = FT_CEIL(FT_MulFix(font_face->descender, scale));
}

int font_calculate_width(char *text, int cache)
{
    int n, x;
    FT_Glyph glyph;
    Uint16 c;
    FT_UInt glyph_index;

    /* Determine width of text */
    x = 0;
    for(n = 0; n < strlen(text); n ++)
    {
        /* Translate UTF8 to UNICODE */
        c = ((unsigned char *)text)[n];
        if(c >= 0xf0)
        {
            c =  (text[n++] & 0x07) << 18;
            c |= (text[n++] & 0x3f) << 12;
            c |= (text[n++] & 0x3f) << 6;
            c |= text[n] & 0x3f;
        }
        else if(c >= 0xe0)
        {
            c =  (text[n++] & 0x0f) << 12;
            c |= (text[n++] & 0x3f) << 6;
            c |= text[n] & 0x3f;
        }
        else if(c >= 0xc0)
        {
            c =  (text[n++] & 0x1f) << 6;
            c |= text[n] & 0x3f;
        }

        glyph_index = FT_Get_Char_Index( font_face, c );
        FT_Load_Glyph( font_face, glyph_index, FT_LOAD_DEFAULT );
        FT_Get_Glyph( font_face->glyph, &glyph );
        x += (glyph->advance.x) >> 16;
        FT_Done_Glyph(glyph);
    }

    /* Add a little extra padding for the strokes around the edges */
    x += (font_size / 10);
    x += (font_size / 10);


    if(cache)
        font_width = x;

    return x;
}

SDL_Surface* font_render(char *text, int foreground)
{
    int height, width;
    int m, n, x;
    int i, j, p, q;
    struct SDL_Surface* psurface;
    struct SDL_Surface* psurfacetmp = NULL;
    FT_Glyph glyph;
    FT_BitmapGlyph glyph_bitmap;
    int pass;
    Uint32 *dst;
    Uint32 colour;
    Uint16 c;
    FT_UInt glyph_index;
    FT_Bitmap*  bitmap;

    height = font_height;

    /* Sometimes we work the width out in advanced, eg, for centering.
       This optimisation saves us having to calculate it twice. */
    if(font_width != 0)
        width = font_width;
    else
        width = font_calculate_width(text, 1);
    font_width = 0;

    /* Render font in two passes - unstroked, then stroked */
    for(pass = 0; pass < 2; pass ++)
    {

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        psurface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0xff000000, 0xff0000, 0xff00, 0xff);
#else
        psurface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
#endif
        SDL_LockSurface(psurface);

        x = 0;
        for(n = 0, m = 0; n < strlen(text); n ++)
        {
            if(foreground < 0)
            {
                if(m < strlen(font_logo_colours))
                    colour = font_logo_colours[m ++] - 48;
                else
                    colour = -foreground - 1;
            }
            else
                colour = foreground;
                
            colour =
                font_colours[2 * colour + pass][0] +
                font_colours[2 * colour + pass][1] * 0x100 +
                font_colours[2 * colour + pass][2] * 0x10000;

            /* Translate UTF8 to UNICODE */
            c = ((unsigned char *)text)[n];
            if(c >= 0xf0)
            {
                c =  (text[n++] & 0x07) << 18;
                c |= (text[n++] & 0x3f) << 12;
                c |= (text[n++] & 0x3f) << 6;
                c |= text[n] & 0x3f;
            }
            else if(c >= 0xe0)
            {
                c =  (text[n++] & 0x0f) << 12;
                c |= (text[n++] & 0x3f) << 6;
                c |= text[n] & 0x3f;
            }
            else if(c >= 0xc0)
            {
                c =  (text[n++] & 0x1f) << 6;
                c |= text[n] & 0x3f;
            }

            glyph_index = FT_Get_Char_Index( font_face, c );
            FT_Load_Glyph( font_face, glyph_index, FT_LOAD_DEFAULT );
            FT_Get_Glyph( font_face->glyph, &glyph );
            if(pass == 1)
                FT_Glyph_StrokeBorder(&glyph, font_stroker, 0, 1);
            FT_Glyph_To_Bitmap( &glyph, FT_RENDER_MODE_NORMAL, 0, 1 );
            glyph_bitmap=(FT_BitmapGlyph)glyph;
            bitmap = &(glyph_bitmap->bitmap);

            for ( i = x + (font_size / 10) + glyph_bitmap->left, p = 0; p < bitmap->width; i++, p++ )
            {
                for ( j = height - glyph_bitmap->top - 1 + font_descent, q = 0; q < bitmap->rows; j++, q++ )
                {
                    if(i < 0 || i >= width || j < 0 || j >= height)
                        continue;
                    if(bitmap->buffer[q * bitmap->width + p] == 0)
                        continue;
                    dst = psurface->pixels
                        + j * psurface->pitch
                        + i * psurface->format->BytesPerPixel;      
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                    *dst |= bitmap->buffer[q * bitmap->width + p] + colour * 0x100;
#else
                    *dst |= bitmap->buffer[q * bitmap->width + p] * 0x1000000 + colour;
#endif
                }
            }

            x += ((glyph->advance.x) >> 16);
            FT_Done_Glyph(glyph);
        }

        SDL_UnlockSurface(psurface);

        if(pass == 0)
        {
            psurfacetmp = psurface;
        }
    }

    SDL_BlitSurface(psurfacetmp, NULL, psurface, NULL);
    SDL_FreeSurface(psurfacetmp);

    return psurface;
}

#else

/* ************ SDL_TTF version ************ */

TTF_Font* font_face;
int font_height;
int font_width;

void font_init()
{
    TTF_Init();

    font_face = NULL;
}

void font_set_size(int size)
{
    char directory[FILENAME_MAX];
    char filename[FILENAME_MAX];

    getfilename("graphics", directory, 0, LOCATION_SYSTEM);

    sprintf(filename, "%s/font.ttf", directory);

    if(font_face != NULL)
        TTF_CloseFont(font_face);

    font_face = TTF_OpenFont(filename, size);
    if(font_face == NULL)
        fatal(gettext("Unable to open font"));
    font_height = TTF_FontHeight(font_face);
}

int font_calculate_width(char *text, int cache)
{
    TTF_SizeUTF8(font_face, text, &font_width, NULL);
    return font_width;
}

SDL_Surface* font_render(char *text, int foreground)
{
    SDL_Color colour;
    SDL_Rect drect;
    int i, j, w;
    char buffer[256];
    int c;
    int width, height;
    SDL_Surface* psurface;
    SDL_Surface* psurfacetmp;

    if(foreground < 0)
    {
        TTF_SizeUTF8(font_face, text, &width, &height);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        psurface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0xff000000, 0xff0000, 0xff00, 0xff);
#else
        psurface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
#endif
        SDL_SetAlpha(psurface, SDL_SRCALPHA, 0);
        SDL_FillRect(psurface, NULL, SDL_MapRGBA(screen_surface->format, 128, 0, 255, 128));

	w = 0;
	for(i = 0; i < strlen(text); i ++)
	{
	    j = 0;
            buffer[j] = text[i]; j ++;
            while((text[i + j] & 0xc0) == 0x80)
            {
                buffer[j] = text[i + j]; j ++;
            }
	    buffer[j] = 0;
	    i += j - 1;

            if(i < strlen(font_logo_colours))
                c = font_logo_colours[i] - 48;
            else
                c = -foreground - 1;

	    colour.r = font_colours[2 * c + 1][0];
	    colour.g = font_colours[2 * c + 1][1];
	    colour.b = font_colours[2 * c + 1][2];

	    psurfacetmp = TTF_RenderUTF8_Blended(font_face, buffer, colour);
            SDL_SetAlpha(psurfacetmp, 0, 255);

	    drect.x = w;
	    drect.y = 0;
	    drect.w = psurfacetmp->w;
	    drect.h = psurfacetmp->h;

	    SDL_BlitSurface(psurfacetmp, NULL, psurface, &drect);
	    w += psurfacetmp->w;
            SDL_FreeSurface(psurfacetmp);
	}

	return psurface;
    }

    colour.r = font_colours[2 * foreground + 1][0];
    colour.g = font_colours[2 * foreground + 1][1];
    colour.b = font_colours[2 * foreground + 1][2];
    if(text == NULL || strcmp(text, "") == 0)
        return TTF_RenderUTF8_Blended(font_face, " ", colour);
    else
        return TTF_RenderUTF8_Blended(font_face, text, colour);
}

#endif

void font_box(int x, int y, int w, int h, int c)
{
    SDL_Surface *psurface;
    SDL_Rect rect;
    int r, g, b;

    psurface = screen_surface;

    /* If the box is too small, fill it all with the border colour */
    if(w <= font_border * 2 || h <= font_border * 2)
    {

        r = font_colours[2 * c + 1][0];
        g = font_colours[2 * c + 1][1];
        b = font_colours[2 * c + 1][2];

        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
        SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, r, g, b));

        return;
    }

    /* Plot the inside of the box */
    r = font_colours[2 * c][0];
    g = font_colours[2 * c][1];
    b = font_colours[2 * c][2];

    rect.x = x + font_border;
    rect.y = y + font_border;
    rect.w = w - (font_border * 2);
    rect.h = h - (font_border * 2);
    SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, r, g, b));

    r = font_colours[2 * c + 1][0];
    g = font_colours[2 * c + 1][1];
    b = font_colours[2 * c + 1][2];

    /* Plot the outer border in solid border colour */
    if(font_border > 1)
    {
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = (font_border - 1);
        SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, r, g, b));
        rect.x = x;
        rect.y = y + h - (font_border - 1);
        rect.w = w;
        rect.h = (font_border - 1);
        SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, r, g, b));
    
        rect.x = x;
        rect.y = y;
        rect.w = (font_border - 1);
        rect.h = h;
        SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, r, g, b));
        rect.x = x + w - (font_border - 1);
        rect.y = y;
        rect.w = (font_border - 1);
        rect.h = h;
        SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, r, g, b));
    }

    /* Plot a one-pixel inner border to give an anti-aliased effect */
    r = (font_colours[2 * c + 0][0] * (256 - font_border_partial) + font_colours[2 * c + 1][0] * font_border_partial ) / 256;
    g = (font_colours[2 * c + 0][1] * (256 - font_border_partial) + font_colours[2 * c + 1][1] * font_border_partial ) / 256;
    b = (font_colours[2 * c + 0][2] * (256 - font_border_partial) + font_colours[2 * c + 1][2] * font_border_partial ) / 256;

    rect.x = x + (font_border - 1);
    rect.y = y + (font_border - 1);
    rect.w = w - 2 * (font_border - 1);
    rect.h = 1;
    SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, r, g, b));
    rect.x = x + (font_border - 1);
    rect.y = y + h - font_border;
    rect.w = w - 2 * (font_border - 1);
    rect.h = 1;
    SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, r, g, b));
    rect.x = x + (font_border - 1);
    rect.y = y + (font_border - 1);
    rect.w = 1;
    rect.h = h - 2 * (font_border - 1);
    SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, r, g, b));
    rect.x = x + w - font_border;
    rect.y = y + (font_border - 1);
    rect.w = 1;
    rect.h = h - 2 * (font_border - 1);
    SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, r, g, b));
}

void font_resize()
{
    int size;

    size = screen_width / 32;

    font_size_menu = size;
    font_size_game = size * 3 / 4;

    if(psurfacelogo != NULL)
        SDL_FreeSurface(psurfacelogo);
    font_set_size(size * 6);
    psurfacelogo = font_render("chroma", -1);

    if(psurfacelogo != NULL)
        SDL_FreeSurface(psurfacelogosmall);
    font_set_size(size * 3 / 2);
    psurfacelogosmall = font_render("chroma", -1);

    font_set_size(font_size_game);
    font_height_game = font_height;

    font_set_size(font_size_menu);
    font_height_menu = font_height;

    font_border = size / 24;
    font_padding = size / 24;
    font_border_partial = ((256 * size) / 24) - (256 * font_border);

    font_border ++;
    font_padding ++;
}
