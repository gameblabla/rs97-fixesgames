/*  
    menu.h

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

#define MENU_GREY	        1
#define MENU_NOTE	        2
#define MENU_RIGHT	        4
#define MENU_CENTRE	        8
#define MENU_DOUBLE	        16
#define MENU_SORT	        32
#define MENU_TOPSORT	        64
#define MENU_DELETABLE	        128
#define MENU_BOLD	        256
#define MENU_INVISIBLE	        512
#define MENU_SCROLLABLE	        1024
#define MENU_EDITABLE	        2048
#define MENU_EDITING	        4096
#define MENU_SPACE              8192
#define MENU_TEXT		16384

#define MENU_NULL	        0
#define MENU_QUIT	        1
#define MENU_SELECT	        2
#define MENU_DELETE	        3
#define MENU_SCROLLLEFT	        4
#define MENU_SCROLLRIGHT	5
#define MENU_RESIZE	        6

#define LEVELMENU_RETURN	1
#define LEVELMENU_NEW		2
#define LEVELMENU_FILENAMES	4
#define LEVELMENU_DELETE	8
#define LEVELMENU_MAIN		16
#define LEVELMENU_MOVES		32
#define LEVELMENU_SAVED		64
#define LEVELMENU_SOLVED	128
#define LEVELMENU_CREATE	256
#define LEVELMENU_GAME		512
#define LEVELMENU_IMPORT        1024
#define LEVELMENU_FILTER        2048

#define MENU_KEY_ANY            1

enum {
    MENUREDRAW_NONE,
    MENUREDRAW_CHANGED,
    MENUREDRAW_ENTRIES,
    MENUREDRAW_ALL,
};

struct menuentry
{
    struct menuentry *next;
    struct menuentry *previous;
    char *text;
    char *text2;
    char *text3;
    char *text4;
    char *value;
    int redraw;
    int flags;
    char key;
};

struct menu
{
    char *title;
    struct menuentry *entry_first;
    struct menuentry *entry_last;
    struct menuentry *entry_selected;
    struct menuentry *display_first;
    struct menuentry *display_last;
    int offset;
    int redraw;
    int logo;
};

/* menu.c */
struct menu *menu_new(char *);
struct menuentry *menuentry_new(struct menu *, char *, char, int);
struct menuentry *menuentry_newwithvalue(struct menu *, char *, char, int, char *);
void menu_delete(struct menu *);
void menu_unsort(struct menu *);
void menu_assignletters(struct menu *);
void menuentry_text(struct menuentry* pentry, char *text);
void menuentry_value(struct menuentry* pentry, char *value);
void menuentry_extratext(struct menuentry* pentry, char *text2, char *text3, char *text4);
int menu_levelselector(char *directory, char *chosen, char *title, int flags);
int menu_colourselector(char *directory, char *chosen, char *title, int flags);

/* $DISPLAYmenudisplay.c */
void menu_display(struct menu *, int);
int menu_process(struct menu *);
int menu_addfile(struct menu *, char *);
