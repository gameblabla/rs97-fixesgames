/*  
    cursesmenudisplay.c

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
#include <ctype.h>
#include <string.h>

#ifdef CHROMA_CURSES_HEADER
#include CHROMA_CURSES_HEADER
#else
#ifdef __WIN32__
#include <curses.h>
#else
#include <ncurses.h>
#endif
#endif

#include "chroma.h"
#include "menu.h"
#include "actions.h"
#include "level.h"
#include "display.h"
#include "util.h"

#define MAX_WIDTH 65

extern int display_size_x, display_size_y;
extern int actions[KEY_MAX];

extern short colourpair_menu;
extern short colourpair_menugrey;
extern short colourpair_red;
extern short colourpair_green;
extern short colourpair_yellow;
extern short colourpair_blue;
extern short colourpair_cyan;
extern short colourpair_magenta;
extern short colourpair_cyan;
extern short colourpair_white;

int menu_offset;
int menu_width;
int menu_height_notes;
int menu_height_entries;
int menu_y_min;
int menu_y_max;
int menu_y_logo_top;
int menu_y_logo_bottom;
int menu_scroll_y_min;
int menu_scroll_y_max;
int menu_scroll_top;
int menu_scroll_bottom;

int menu_entryheight(struct menuentry *pentry)
{
    if(pentry->flags & MENU_INVISIBLE)
        return 0;
    if(pentry->flags & MENU_DOUBLE)
        return 2;
    return 1;
}

void menu_displayentry(struct menu *pmenu, struct menuentry *pentry, int y, int selected)
{
    char buffer[MAX_WIDTH + 1];
    int x, i;

    if(menu_width < 1)
        return;

    if(pentry->flags & MENU_INVISIBLE)
        return;

    if(pentry->flags & (MENU_GREY | MENU_NOTE | MENU_TEXT))
        selected = 0;

    /* Plot first line, if visible */
    if((y >= menu_y_min && y < menu_y_max) || (pentry->flags & MENU_NOTE))
    {
        /* Plot key press */
        attron(COLOR_PAIR(colourpair_white));
        if(pentry->key != 0 && pentry->key != MENU_KEY_ANY)
        {   
            mvprintw(y, menu_offset - 4, "[ ] ");
            attron(A_BOLD);
            mvprintw(y, menu_offset - 3, "%c", pentry->key);
            attroff(A_BOLD);
        }
        else
            mvprintw(y, menu_offset - 4, "    ");
        attroff(COLOR_PAIR(colourpair_white));

        /* Determine colour */
        if(pentry->flags & (MENU_GREY | MENU_SPACE))
            attron(COLOR_PAIR(colourpair_menugrey));
        else if(pentry->flags & MENU_NOTE)
        {
            if(pmenu->logo == 0)
                attron(COLOR_PAIR(colourpair_menugrey));
        }
        else if(pentry->flags & MENU_TEXT)
            attron(COLOR_PAIR(colourpair_white));
        else
            attron(COLOR_PAIR(colourpair_menu));
        if(selected)
            attron(A_REVERSE);

        /* Blank line */
        move(y, menu_offset);
        for(i = 0; i < menu_width; i ++)
            addch(' ');

        /* Plot right hand side text */
        if(pentry->text2 != NULL)
        {   
	    utf8strncpy(buffer, pentry->text2, menu_width - 2);
            x = menu_offset + menu_width - 1 - utf8strlen(buffer);
            mvprintw(y, x, buffer);
        }

        /* Plot main text */
        if(pentry->text != NULL)
        {
	    utf8strncpy(buffer, pentry->text, menu_width - 2);

            x = menu_offset + 1;
            if(pentry->flags & MENU_RIGHT)
                x = menu_offset + menu_width - utf8strlen(buffer);
            if(pentry->flags & MENU_CENTRE)
                x = menu_offset + ((menu_width - utf8strlen(buffer))/2);

            if(pentry->flags & MENU_BOLD)
                attron(A_BOLD);
            mvprintw(y, x, buffer);
            if(pentry->flags & MENU_BOLD)
                attroff(A_BOLD);
        }

        if(selected)
            attroff(A_REVERSE);
        if(pentry->flags & (MENU_GREY | MENU_NOTE | MENU_SPACE))
            attroff(COLOR_PAIR(colourpair_menugrey));
        else if(pentry->flags & MENU_TEXT)
            attroff(COLOR_PAIR(colourpair_white));
        else
            attroff(COLOR_PAIR(colourpair_menu));
    }

    if(!(pentry->flags & MENU_DOUBLE))
        return;

    y ++;

    /* Plot second line, if visible */
    if((y >= menu_y_min && y < menu_y_max) || (pentry->flags & MENU_NOTE))
    {
        /* Blank key press area */
        attron(COLOR_PAIR(colourpair_white));
        mvprintw(y, menu_offset - 4, "    ");
        attroff(COLOR_PAIR(colourpair_white));

        /* Determine colour */
        if(pentry->flags & (MENU_GREY | MENU_NOTE | MENU_SPACE))
            attron(COLOR_PAIR(colourpair_menugrey));
        else
            attron(COLOR_PAIR(colourpair_menu));
        if(selected)
            attron(A_REVERSE);

        /* Blank line */
        move(y, menu_offset);
        for(i = 0; i < menu_width; i ++)
            addch(' ');

        /* Plot right hand side text */
        if(pentry->text4 != NULL)
        {   
	    utf8strncpy(buffer, pentry->text4, menu_width - 2);

            x = menu_offset + menu_width - 1 - utf8strlen(buffer);
            if(pentry->flags & MENU_EDITING)
                attron(COLOR_PAIR(colourpair_white));
            mvprintw(y, x, buffer);
            if(pentry->flags & MENU_EDITING)
                attroff(COLOR_PAIR(colourpair_white));
        }

        /* Plot left hand side text */
        if(pentry->text3 != NULL)
        {   
	    utf8strncpy(buffer, pentry->text3, menu_width - 2);

            x = menu_offset + 1;
            mvprintw(y, x, buffer);
        }

        if(selected)
            attroff(A_REVERSE);
        if(pentry->flags & (MENU_GREY | MENU_SPACE))
            attroff(COLOR_PAIR(colourpair_menugrey));
        else if(pentry->flags & MENU_NOTE)
        {
            if(pmenu->logo == 0)
                attroff(COLOR_PAIR(colourpair_menugrey));
        }
        else
            attroff(COLOR_PAIR(colourpair_menu));
    }
}

void menu_display(struct menu *pmenu, int redraw)
{
    struct menuentry *pentry;
    int x, y, selected;
    char title[] = "chroma";
    char buffer[256];
    int i;
    int state;
    int y_editing;
    short cp;
    int j;

                /* 012345678901234567890123456789012345678901 */
    char logo[] = "       8                                  "
                  "       8                                  "
                  ".oPYo. 8oPYo. oPYo. .oPYo. ooYoYo. .oPYo. "
                  "8    ' 8    8 8  `' 8    8 8' 8  8 .oooo8 "
                  "8    . 8    8 8     8    8 8  8  8 8    8 "
                  "`YooP' 8    8 8     `YooP' 8  8  8 `YooP8 ";

#define LOGO_HEIGHT 7

    /* Calculate various widths */
    menu_width = display_size_x - 2;
    if(menu_width > MAX_WIDTH)
        menu_width = MAX_WIDTH;

    menu_offset = (display_size_x - menu_width) / 2;
    if(menu_offset < 0)
        menu_offset = 0;

    menu_width -= 5;
    menu_offset += 4;

    /* Calculate various heights */
    menu_height_notes = 0;
    menu_height_entries = 0;

    pentry = pmenu->entry_first;
    while(pentry != NULL)
    {   
        if(pentry->flags & MENU_NOTE)
            menu_height_notes += menu_entryheight(pentry);
        else
            menu_height_entries += menu_entryheight(pentry);
        pentry = pentry->next;
    }

    /* Add an extra line to pad the notes out, if there are any */
    if(menu_height_notes != 0)
        menu_height_notes += 1;

    if(pmenu->logo)
    {   
        menu_y_logo_top = (display_size_y - menu_height_entries - LOGO_HEIGHT) / 2;
        if(menu_y_logo_top < 0)
            menu_y_logo_top = 0;
        menu_y_logo_bottom = menu_y_logo_top + LOGO_HEIGHT;
        menu_y_min = display_size_y - menu_height_entries - 1;
        if(menu_y_min < menu_y_logo_bottom)
            menu_y_min = menu_y_logo_bottom;
        menu_y_max = display_size_y - 1;
    }
    else
    {   
        menu_y_min = 4;
        menu_y_max = display_size_y - menu_height_notes - 1;
        if(pmenu->title != NULL)
            menu_y_min += 2;
    }

    /* If a full redraw, clear the screen and plot the title */
    if(redraw == MENUREDRAW_ALL)
    { 
	clear();
	curs_set(0); 

        if(pmenu->logo)
        {
            x = (display_size_x - 42) / 2;
            y = menu_y_logo_top;
            cp = colourpair_red;

            move(y, x);

            for(i = 0; i < strlen(logo); i ++)
            {
                addch(logo[i] | A_BOLD | COLOR_PAIR(cp));
                switch(i % 42)
                {
                    case 6:
                        cp = colourpair_yellow;
                        break;
                    case 13:
                        cp = colourpair_green;
                        break;
                    case 18:
                        cp = colourpair_cyan;
                        break;
                    case 25:
                        cp = colourpair_blue;
                        break;
                    case 33:
                        cp = colourpair_magenta;
                        break;
                    case 41:
                        y ++;
                        move(y, x);
                        cp = colourpair_red;
                        break;
                }
            }

            y ++;

        }
        else
        {
            /* Display game title */
            x = (display_size_x / 2) - strlen(title);

            attron(A_BOLD);
            for(i = 0; i < strlen(title); i ++)
            {   
                if(i == 0) attron(COLOR_PAIR(colourpair_red));
                if(i == 1) attron(COLOR_PAIR(colourpair_yellow));
                if(i == 2) attron(COLOR_PAIR(colourpair_green));
                if(i == 3) attron(COLOR_PAIR(colourpair_cyan));
                if(i == 4) attron(COLOR_PAIR(colourpair_blue));
                if(i == 5) attron(COLOR_PAIR(colourpair_magenta));

                sprintf(buffer, "%c", title[i]);
                mvprintw(2, x, buffer);
                x +=2;
            }
    	    attroff(COLOR_PAIR(colourpair_red));
            attroff(A_BOLD);

            /* Display menu title */
            y = 4;
            x = (display_size_x / 2) - utf8strlen(pmenu->title);

            attron(A_BOLD);
    	    attroff(COLOR_PAIR(colourpair_white));
            /* If the title is too long, plot normally */
            if(x < 0)
            {
                x = (display_size_x - utf8strlen(pmenu->title)) / 2;
                mvprintw(y, x, pmenu->title);
            }
            else
            {
                /* Otherwise, spread it out */
                for(i = 0; i < strlen(pmenu->title); i ++)
                {
		    j = 0;
		    buffer[j] = pmenu->title[i]; j ++;
		    while((pmenu->title[i + j] & 0xc0) == 0x80)
		    {
			buffer[j] = pmenu->title[i + j]; j ++;
		    }
		    buffer[j] = 0;
                    mvprintw(y, x, buffer);
		    x += 2;  i += j - 1;
                }
            }
            attroff(A_BOLD);
        }
    }

    /* Keep scroll bar within reasonable limits */
    if(pmenu->offset > (menu_height_entries - (menu_y_max - menu_y_min)))
        pmenu->offset = menu_height_entries - (menu_y_max - menu_y_min);
    if(pmenu->offset < 0)
        pmenu->offset = 0;

    /* Display scrollbar, if needed */
    if(menu_height_entries > (menu_y_max - menu_y_min) || pmenu->offset != 0)
    {
        if(redraw >= MENUREDRAW_ENTRIES)
        {
            menu_scroll_y_min = menu_y_min + 1;
            menu_scroll_y_max = menu_y_max - 1;
            menu_scroll_top = menu_scroll_y_min + ((menu_scroll_y_max - menu_scroll_y_min) * pmenu->offset / menu_height_entries);
            menu_scroll_bottom = menu_scroll_top + ((menu_scroll_y_max - menu_scroll_y_min) * (1 + menu_y_max - menu_y_min) / menu_height_entries);

            attron(COLOR_PAIR(colourpair_menugrey));
            x = menu_offset + menu_width + 1;
            mvaddch(menu_y_min, x, '^');
            mvaddch(menu_y_max - 1, x, 'v');

            attron(COLOR_PAIR(colourpair_white));
            for(y = menu_y_min + 1; y < menu_y_max - 1; y ++)
            {
                /* >= and <= to guarantee at least one character scrollbar */
                if(y >= menu_scroll_top && y <= menu_scroll_bottom)
                {
                attron(A_REVERSE);
                mvaddch(y, x, '|');
                attroff(A_REVERSE);
                }
                else
                mvaddch(y, x, '.');
            }
        }
    }

    /* Display notes */
    if((menu_height_notes != 0) && redraw >= MENUREDRAW_ENTRIES)
    {
        if(pmenu->logo)
            y = menu_y_logo_bottom;
        else
            y = menu_y_max + 1;

        pentry = pmenu->entry_first;
        while(pentry != NULL)
        {
            if(pentry->flags & MENU_NOTE)
            {
                menu_displayentry(pmenu, pentry, y, 0);
                y += menu_entryheight(pentry);
            }
            pentry = pentry->next;
        }
    }

    /* Select an entry if the selected one is not on screen */
    if(pmenu->entry_selected == NULL)
        pmenu->entry_selected = pmenu->entry_first;
    y = menu_y_min - pmenu->offset;
    pentry = pmenu->entry_first;
    pmenu->display_first = NULL;
    pmenu->display_last = NULL;
    state = 1;
    while(pentry != NULL)
    {
        /* Don't count notes */
        if(pentry->flags & MENU_NOTE)
        {
            pentry = pentry->next;
            continue;
        }
        /* Is the entry off the top of the screen? */
        if(y < (menu_y_min - menu_entryheight(pentry)))
        {
            if(pentry == pmenu->entry_selected)
                state = -1;
            y += menu_entryheight(pentry);
            pentry = pentry->next;
            continue;
        }
        /* Stop processing once we hit the bottom of the screen */
        if(y > menu_y_max)
        {
            pentry = NULL;
            continue;
        }
        if(pentry == pmenu->entry_selected)
            state = 0;
        
        if(pmenu->display_first == NULL)
            pmenu->display_first = pentry->next;
        pmenu->display_last = pentry->previous;

        y += menu_entryheight(pentry);
        pentry = pentry->next;
    }
    if(state == -1)
        pmenu->entry_selected = pmenu->display_first;
    if(state == 1)
        pmenu->entry_selected = pmenu->display_last;
    if(state != 0 && redraw < MENUREDRAW_ENTRIES)
        redraw = MENUREDRAW_ENTRIES;

    /* Display entries */
    pentry = pmenu->entry_first;
    y = menu_y_min - pmenu->offset;
    y_editing = 0;
    while(pentry != NULL)
    {
        /* Don't display notes */
        if(pentry->flags & MENU_NOTE)
        {
            pentry = pentry->next;
            continue;
        }
        /* Don't display entries off the top of the screen */
        if(y < (menu_y_min - menu_entryheight(pentry)))
        {
            y += menu_entryheight(pentry);
            pentry = pentry->next;
            continue;
        }
        /* Stop processing once we hit the bottom of the screen */
        if(y > menu_y_max)
        {
            pentry = NULL;
            continue;
        }
        selected = (pmenu->entry_selected == pentry) ? 1 : 0;

        if(redraw >= MENUREDRAW_ENTRIES ||
                (redraw >= MENUREDRAW_CHANGED && pentry->redraw))
            menu_displayentry(pmenu, pentry, y, selected);
        if(pentry->flags & MENU_EDITING)
            y_editing = y + 1;


        pentry->redraw = 0;
        y += menu_entryheight(pentry);
        pentry = pentry->next;
    }

    /* Display cursor if we're editing a text field */
    if(y_editing)
    {
	curs_set(1); 
        move(y_editing, menu_offset + menu_width - 2);
    }
    else
	curs_set(0); 

    /* Redraw the screen */
    refresh();
}

int menu_process(struct menu* pmenu)
{
    int ok;
    int redraw;
    int c;
    struct menuentry* pentry;
    char *buffer;

    redraw = pmenu->redraw;

    ok = 0;
    while(!ok)
    {   
        if(redraw != MENUREDRAW_NONE)
        {   
            menu_display(pmenu, redraw);
            redraw = MENUREDRAW_NONE;
        }

        c = getch();

        /* Are we editing a text field? */
        if(pmenu->entry_selected != NULL && pmenu->entry_selected->flags & MENU_EDITING)
        {  
            switch(c)
            {  
                case KEY_RESIZE:
                    getmaxyx(stdscr, display_size_y, display_size_x);
                    redraw = MENUREDRAW_ALL;
                    break;

                case '\n':
                case '\r':
                case 27:
                case KEY_UP:
                case KEY_DOWN:
                case KEY_PPAGE:
                case KEY_NPAGE:
                    pmenu->entry_selected->flags -= MENU_EDITING;
                    pmenu->entry_selected->redraw = 1;
                    redraw = MENUREDRAW_CHANGED;
                    break;

                case KEY_BACKSPACE:
                case KEY_DC:
                case 8:
                    if(strlen(pmenu->entry_selected->text4) == 0)
                        break;

                    buffer = malloc(strlen(pmenu->entry_selected->text4) + 1);
                    if(buffer != NULL)
                    {  
                        strcpy(buffer, pmenu->entry_selected->text4);

                        /* not particularly efficient, but does the job of deleting one UTF8 character */
                        while(strlen(buffer) > 0 && ((buffer[strlen(buffer) - 1] & 0xc0) == 0x80))
                            buffer[strlen(buffer) - 1] = 0;
                        buffer[strlen(buffer) - 1] = 0;
                    }
                    menuentry_extratext(pmenu->entry_selected, NULL, NULL, buffer);
                    free(buffer);

                    pmenu->entry_selected->redraw = 1;
                    redraw = MENUREDRAW_CHANGED;
                    break;

                default:
                    if(c > 31 && c != 127)
                    {  
                        buffer = malloc(strlen(pmenu->entry_selected->text4) + 2);
                        if(buffer != NULL)
                        {  
                            strcpy(buffer, pmenu->entry_selected->text4);
                            buffer[strlen(buffer) + 1] = 0;
                            buffer[strlen(buffer)] = c;
                        }
                        menuentry_extratext(pmenu->entry_selected, NULL, NULL, buffer);
                        free(buffer);

                        pmenu->entry_selected->redraw = 1;
                        redraw = MENUREDRAW_CHANGED;
                        break;
                    }
                    break;
            }
            continue;
        }

        /* Not a text field, but a regular entry */
        /* First, check if the key corresponds to an entry */
        if(c >= 0 && c < 127)
            c = toupper(c);
        pentry = pmenu->entry_first;
        while(pentry != NULL)
        {  
            if(pentry->key == c)
            {   
                if(!(pentry->flags & (MENU_GREY | MENU_INVISIBLE | MENU_SPACE)))
                {
                    if(pmenu->entry_selected != NULL)
                        pmenu->entry_selected->redraw = 1;
                    pmenu->entry_selected = pentry;
                    pentry->redraw = 1;

                    if(pmenu->entry_selected->flags & MENU_EDITABLE)
                    {
                        pmenu->entry_selected->flags |= MENU_EDITING;
                        pmenu->entry_selected->redraw = 1;
                        redraw = MENUREDRAW_CHANGED;
                    }
                    else
                        ok = MENU_SELECT;
                    break;
                }
            }
            pentry = pentry->next;
        }

        if(ok == MENU_SELECT)
            continue;

        if(c < 0)
            continue;

        /* Otherwise, see if it corresponds to an action */
        switch(actions[c])
        {   
            case ACTION_UP:
                pentry = pmenu->entry_selected;
                while(pentry != NULL)
                {   
                    if(pentry == pmenu->display_first || pentry->flags & MENU_TEXT)
                        redraw = MENUREDRAW_ENTRIES;

                    pentry = pentry->previous;

                    if(pentry == NULL)
                        break;
                    if(redraw != MENUREDRAW_NONE)
                        pmenu->offset -= menu_entryheight(pentry);
                    if(!(pentry->flags & (MENU_GREY | MENU_NOTE | MENU_SPACE)))
                        break;
                }
                if(pentry != NULL)
                {
                    pentry->redraw = 1;
                    pmenu->entry_selected->redraw = 1;
                    pmenu->entry_selected = pentry;
                    if(redraw == MENUREDRAW_NONE)
                        redraw = MENUREDRAW_CHANGED;
                }
                break;

            case ACTION_DOWN:
                pentry = pmenu->entry_selected;
                while(pentry != NULL)
                {
                    if(pentry == pmenu->display_last || pentry->flags & MENU_TEXT)
                        redraw = MENUREDRAW_ENTRIES;

                    pentry = pentry->next;

                    if(pentry == NULL)
                        break;
                    if(redraw != MENUREDRAW_NONE)
                        pmenu->offset += menu_entryheight(pentry);
                    if(!(pentry->flags & (MENU_GREY | MENU_NOTE | MENU_SPACE)))
                        break;
                }
                if(pentry != NULL)
                {
                    pentry->redraw = 1;
                    pmenu->entry_selected->redraw = 1;
                    pmenu->entry_selected = pentry;
                    if(redraw == MENUREDRAW_NONE)
                        redraw = MENUREDRAW_CHANGED;
                }
                break;

            case ACTION_LEFT:
                if(pmenu->entry_selected != NULL && pmenu->entry_selected->flags & MENU_SCROLLABLE)
                    ok = MENU_SCROLLLEFT;
                break;

            case ACTION_RIGHT:
                if(pmenu->entry_selected != NULL && pmenu->entry_selected->flags & MENU_SCROLLABLE)
                    ok = MENU_SCROLLRIGHT;
                break;

            case ACTION_PAGE_UP:
                pmenu->offset -= (menu_y_max - menu_y_min);
                redraw = MENUREDRAW_ENTRIES;
                break;

            case ACTION_PAGE_DOWN:
                pmenu->offset += (menu_y_max - menu_y_min);
                redraw = MENUREDRAW_ENTRIES;
                break;

            case ACTION_ENTER:
                if(pmenu->entry_selected != NULL)
                {  
                   if(pmenu->entry_selected->flags & MENU_EDITABLE)
                   {   
                       pmenu->entry_selected->flags |= MENU_EDITING;
                       pmenu->entry_selected->redraw = 1;
                       redraw = MENUREDRAW_CHANGED;
                   }
                   else
                       ok = MENU_SELECT;
                }
                break;

            case ACTION_DELETE:
                if(pmenu->entry_selected != NULL && pmenu->entry_selected->flags & MENU_DELETABLE)
                    ok = MENU_DELETE;
                break;

            case ACTION_QUIT:
                ok = MENU_QUIT;
                break;

            case ACTION_REDRAW:
                getmaxyx(stdscr, display_size_y, display_size_x);
                redraw = MENUREDRAW_ALL;
                break;

            case ACTION_HIDE:
                display_hide();
                redraw = MENUREDRAW_ALL;
                break;

            default:
                /* See if there is an "any key" entry */
                pentry = pmenu->entry_first;
                while(pentry != NULL)
                {    
                    if(pentry->key == MENU_KEY_ANY)
                    {   
                        if(!(pentry->flags & (MENU_GREY | MENU_INVISIBLE | MENU_SPACE)))
                        {   
                            if(pmenu->entry_selected != NULL)
                                pmenu->entry_selected->redraw = 1;
                            pmenu->entry_selected = pentry;
                            pentry->redraw = 1;

                            ok = MENU_SELECT;
                            break;
                        }
                    }
                    pentry = pentry->next;
                }
                break;
        }
    }

    /* Redraw the whole menu when we next process it */
    pmenu->redraw = MENUREDRAW_ALL;

    if(ok == MENU_SELECT)
    {   
        if(pmenu->entry_selected->flags & MENU_SCROLLABLE)
            ok = MENU_SCROLLRIGHT;
    }
    /* unless this is a scrollable entry */
    if(ok == MENU_SCROLLLEFT || ok == MENU_SCROLLRIGHT)
    {   
        pmenu->redraw = MENUREDRAW_CHANGED;
        pmenu->entry_selected->redraw = 1;
    }

    return ok;
}

int menu_addfile(struct menu *pmenu, char *filename)
{
    FILE *file;
    char word[256], buffer[4096];
    char c;
    int i;
    int ok, wok;

    file = fopen(filename, "r");
    if(file == NULL)
	return 0;

    ok = 0;
    strcpy(buffer, "");
    while(!ok)
    {
	wok = 0; i = 0;
	while(!wok)
	{
	    c = fgetc(file);
	    if(c == 0 || c == 13 || c == 10 || c == 32 || c == 9 || feof(file) || i == 254)
		wok = 1;
	    else
	        word[i++] = c;
	}
	word[i] = 0;

        /* We assume menu_width is defined by the time we get here - a safe
           assumption provided this isn't the first menu we see. */
	if(utf8strlen(buffer) + utf8strlen(word) < menu_width - 2 && !(strcmp(word, "") == 0 && (c == 10 || c == 13)) && !(strncmp(word, "===", 3) == 0))
	{
	    if(buffer[0] != 0 && word[0] != 0)
	        strcat(buffer, " ");
	    strcat(buffer, word);
	}
	else
	{
	    menuentry_new(pmenu, buffer, 0, MENU_TEXT);
	    if(strcmp(word, "") == 0 && (c == 10 || c ==13))
		menuentry_new(pmenu, "", 0, MENU_TEXT);
            if(strncmp(word, "===", 3) == 0)
            {
                strcpy(buffer, "");
                pmenu->entry_last->flags |= MENU_GREY;
            }
            else
	        strcpy(buffer, word);
	}

	if(feof(file))
	    ok = 1;
    }

    menuentry_new(pmenu, buffer, 0, MENU_TEXT);

    fclose(file);

    return 1;
}
