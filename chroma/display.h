/*  
    display.h

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

enum { DISPLAY_NONE, DISPLAY_CURSES, DISPLAY_SDL };

void display_init();
void display_quit();
void display_play(struct level*, struct level*);
void display_edit(struct level*);
int display_type();
void display_options();

void display_menuentry(struct menuentry *pentry, int y, int selected);
void display_menu(struct menu *pmenu, int all);
void display_options_save();
void display_options_load();
void display_hide();
