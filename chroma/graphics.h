/*  
    graphics.h

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

#define GRAPHICS_DEFAULT	"chroma-zen.chroma"

/* Flags for pieces */
#define GRAPHICS_BEVEL		1
#define GRAPHICS_BEVEL_SHADOW	2
#define GRAPHICS_BEVEL16	4
#define GRAPHICS_RANDOM		8
#define GRAPHICS_KEY		16
#define GRAPHICS_MOVER		32
#define GRAPHICS_TILE		64
#define GRAPHICS_ANIMATE	128
#define GRAPHICS_LEVEL		256
#define GRAPHICS_CLONE		512
#define GRAPHICS_SCALE          1024

/* Flags for set */
#define GRAPHICS_CURSES		1
#define GRAPHICS_ZORDER		2
#define GRAPHICS_BACKGROUND     4
#define GRAPHICS_TRANSLATE      8

#define BEVEL_BASE              0x10000
#define BEVEL_L                (BEVEL_BASE * 1)
#define BEVEL_R                (BEVEL_BASE * 2)
#define BEVEL_U                (BEVEL_BASE * 4)
#define BEVEL_D                (BEVEL_BASE * 8)
#define BEVEL_TL               (BEVEL_BASE * 16)
#define BEVEL_TR               (BEVEL_BASE * 32)
#define BEVEL_BL               (BEVEL_BASE * 64)
#define BEVEL_BR               (BEVEL_BASE * 128)
#define BEVEL_ALL              (BEVEL_BASE * 255)

#define SHADOW_BASE            0x1000000
#define SHADOW_TOP_LEFT		1
#define SHADOW_TOP		2
#define SHADOW_TOP_RIGHT	4
#define SHADOW_LEFT		8
#define SHADOW_MIDDLE		16
#define SHADOW_RIGHT		32
#define SHADOW_BOTTOM_LEFT	64
#define SHADOW_BOTTOM		128
#define SHADOW_BOTTOM_RIGHT	256

#define IMAGE_PIECE	0
#define IMAGE_SHADOW	1
#define IMAGE_SMALL     2
#define IMAGE_MAX       3

#define SIZE_PIECES     1
#define SIZE_SMALL      2

struct graphicssize
{
    int x;
    int y;
    int flags;
    struct graphicssize *next;
};

struct shadow
{
    int x;
    int y;
    int z;
    int p;
    int flag;
    int shadow;
    struct shadow *next;
    struct shadow *nextordered;
    struct shadow *previousordered;
};


struct graphics
{
    int size_x;
    int size_y;
    int small_size_x;
    int small_size_y;
    int flags;
    char *title;
    int background[3];
    int level;
    int levels;

    int image_flags[PIECE_MAX];
    SDL_Surface *image[PIECE_MAX][3];

    int clone[PIECE_MAX];
    int shadow_z[PIECE_MAX];
    int shadow_offset_x[PIECE_MAX][10];
    int shadow_offset_y[PIECE_MAX][10];
    int shadow_start_x[PIECE_MAX][10];
    int shadow_start_y[PIECE_MAX][10];
    int shadow_width[PIECE_MAX][10];
    int shadow_height[PIECE_MAX][10];
    int shadow_flags[PIECE_MAX];

    struct graphicssize *sizes;
    struct shadow *shadows;

};

void graphics_init();
struct graphics* graphics_load(char *filename, int partial);
void graphics_delete(struct graphics*);
struct menu* graphics_menu();
SDL_Surface *graphics_loadimage(char *filename);
void graphics_reload();

