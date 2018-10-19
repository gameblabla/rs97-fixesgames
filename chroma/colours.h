/*  
    colours.h

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

#define COLOURS_DEFAULT "chroma-standard.chroma"

#define COLOURS_TRANSLATE 1

struct colours
{
    char *title;

    char character[PIECE_MAX];
    int foreground[PIECE_MAX];
    int background[PIECE_MAX];
    int bold[PIECE_MAX];
    int reverse[PIECE_MAX];
    int flags;
};

void colours_init();
struct colours* colours_load(char *filename, int partial);
void colours_delete(struct colours*);
struct menu* colours_menu();


