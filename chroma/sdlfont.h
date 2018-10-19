/*  
    sdlfont.h

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

/* UTF8 arrow symbols */
#define ARROW_UP "\xe2\x96\xb2"
#define ARROW_DOWN "\xe2\x96\xbc"
#define ARROW_LEFT "\xe2\x97\x80"
#define ARROW_RIGHT "\xe2\x96\xb6"

#define COLOUR_BLACK    0
#define COLOUR_RED      1
#define COLOUR_GREEN    2
#define COLOUR_YELLOW   3
#define COLOUR_BLUE     4
#define COLOUR_MAGENTA  5
#define COLOUR_CYAN     6
#define COLOUR_WHITE    7

#define COLOUR_LIGHT    8
#define COLOUR_BOLD     16

void font_init();
void font_resize();
void font_set_size(int size);
int font_calculate_width(char *text, int cache);
SDL_Surface* font_render(char *text, int foreground);
void font_box(int x, int y, int w, int h, int c);

