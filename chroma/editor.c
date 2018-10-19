/*  
    editor.c

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
#include <string.h>
#include <time.h>
#include <libintl.h>
#include <locale.h>

#include "chroma.h"
#include "level.h"
#include "menu.h"
#include "display.h"
#include "util.h"

void export(struct level*, int);

int editor_verify(struct level*);
struct level* editor_options(struct level* plevel);

extern int move_x[];
extern int move_y[];
extern char* piece_name[];
extern struct level* plevelcurrent;
extern int options_debug;

#ifdef XOR_COMPATIBILITY
extern int options_xor_options;
#endif
#ifdef ENIGMA_COMPATIBILITY
extern int options_enigma_options;
#endif

int editor_pieces_chroma[] = {
	PIECE_SPACE,
	PIECE_WALL,
	PIECE_PLAYER_ONE,
	PIECE_PLAYER_TWO,
	PIECE_DOTS,
	PIECE_ARROW_RED_LEFT,
	PIECE_ARROW_RED_UP,
	PIECE_ARROW_RED_RIGHT,
	PIECE_ARROW_RED_DOWN,
	PIECE_BOMB_RED_LEFT,
	PIECE_BOMB_RED_UP,
	PIECE_BOMB_RED_RIGHT,
	PIECE_BOMB_RED_DOWN,
	PIECE_ARROW_GREEN_LEFT,
	PIECE_ARROW_GREEN_UP,
	PIECE_ARROW_GREEN_RIGHT,
	PIECE_ARROW_GREEN_DOWN,
	PIECE_BOMB_GREEN_LEFT,
	PIECE_BOMB_GREEN_UP,
	PIECE_BOMB_GREEN_RIGHT,
	PIECE_BOMB_GREEN_DOWN,
	PIECE_ARROW_BLUE_LEFT,
	PIECE_ARROW_BLUE_UP,
	PIECE_ARROW_BLUE_RIGHT,
	PIECE_ARROW_BLUE_DOWN,
	PIECE_BOMB_BLUE_LEFT,
	PIECE_BOMB_BLUE_UP,
	PIECE_BOMB_BLUE_RIGHT,
	PIECE_BOMB_BLUE_DOWN,
	PIECE_CIRCLE,
	PIECE_STAR,
	PIECE_DOOR,
	PIECE_GONE
};

#ifdef XOR_COMPATIBILITY
int editor_pieces_xor[] = {
	PIECE_SPACE,
	PIECE_WALL,
	PIECE_PLAYER_ONE,
	PIECE_PLAYER_TWO,
	PIECE_DOTS_X,
	PIECE_DOTS_Y,
	PIECE_ARROW_RED_DOWN,
	PIECE_ARROW_RED_LEFT,
	PIECE_BOMB_RED_DOWN,
	PIECE_BOMB_RED_LEFT,
	PIECE_STAR,
	PIECE_DOOR,
	PIECE_CIRCLE,
	PIECE_TELEPORT,
	PIECE_SWITCH,
	PIECE_MAP_TOP_LEFT,
	PIECE_MAP_TOP_RIGHT,
	PIECE_MAP_BOTTOM_LEFT,
	PIECE_MAP_BOTTOM_RIGHT,
	PIECE_GONE
};
#endif

#ifdef ENIGMA_COMPATIBILITY
int editor_pieces_enigma[] = {
	PIECE_SPACE,
	PIECE_WALL,
	PIECE_PLAYER_ONE,
	PIECE_DOTS,
	PIECE_DOTS_DOUBLE,
	PIECE_ARROW_RED_LEFT,
	PIECE_ARROW_RED_UP,
	PIECE_ARROW_RED_RIGHT,
	PIECE_ARROW_RED_DOWN,
	PIECE_BOMB_RED_LEFT,
	PIECE_BOMB_RED_UP,
	PIECE_BOMB_RED_RIGHT,
	PIECE_BOMB_RED_DOWN,
	PIECE_STAR,
	PIECE_DOOR,
	PIECE_CIRCLE,
	PIECE_CIRCLE_DOUBLE,
	PIECE_GONE
};
#endif

int *editor_piece_maps[] =
{
    editor_pieces_chroma,
#ifdef XOR_COMPATIBILITY
    editor_pieces_xor,
#endif
#ifdef ENIGMA_COMPATIBILITY
    editor_pieces_enigma,
#endif
    NULL
};

void editor()
{
    char filename[FILENAME_MAX];
    char directory[FILENAME_MAX];
    struct level* plevelload;
    struct level* pleveltest;
    struct menu* pmenu;
    struct menu* pmenuconfirm;
    struct menuentry* pentryno;
    int ok;
    int result;

    plevelcurrent = NULL;

    ok = 0;
    while(!ok)
    {
	pmenu = menu_new(gettext("Editing Menu"));

	if(plevelcurrent == NULL)
            menuentry_new(pmenu, gettext("Return to Main Menu"), 'Q', 0);
	else
	{
	    menuentry_new(pmenu, gettext("Return to Editor"), 'Q', 0);
	    menuentry_new(pmenu, gettext("Abort Editor and Return to Main Menu"), 'A', 0);
	}
	menuentry_new(pmenu, "", 0, MENU_SPACE);

	menuentry_new(pmenu, gettext("Load Level"), 'L', 0);
	menuentry_new(pmenu, gettext("Save Level"), 'S', plevelcurrent == NULL ? MENU_GREY : 0);
	menuentry_new(pmenu, gettext("Export Level"), 'E', plevelcurrent == NULL ? MENU_GREY : 0);
        menuentry_new(pmenu, gettext("Test Level"), 'T', plevelcurrent == NULL ? MENU_GREY : 0);
        menuentry_new(pmenu, gettext("Verify Level"), 'V', plevelcurrent == NULL ? MENU_GREY : 0);

	menuentry_new(pmenu, "", 0, MENU_SPACE);

	menuentry_new(pmenu, gettext("Level Options"), 'O', plevelcurrent == NULL ? MENU_GREY : 0);
	menuentry_new(pmenu, "", 0, MENU_SPACE);

	menuentry_new(pmenu, gettext("Display Options"), 'D', 0);

	menuentry_new(pmenu, gettext("Currently editing:"), 0, MENU_NOTE);

	if(plevelcurrent != NULL)
	{
	    if(plevelcurrent->title == NULL || strcmp(plevelcurrent->title, "") == 0)
  	        menuentry_new(pmenu, gettext("[untitled level]"), 0, MENU_NOTE | MENU_RIGHT);
	    else
  	        menuentry_new(pmenu, gettext(plevelcurrent->title), 0, MENU_NOTE | MENU_RIGHT);
	}
	else
  	    menuentry_new(pmenu, gettext("** NO LEVEL LOADED **"), 0, MENU_NOTE | MENU_RIGHT);

	result = menu_process(pmenu);

	if(result == MENU_SELECT && pmenu->entry_selected != NULL)
	{
	    switch(pmenu->entry_selected->key)
	    {
		case 'A':
		    ok = 1;
		    break;

		case 'S':
		    getfilename("construct", directory, 1, 0);
		    if(menu_levelselector(directory, filename, gettext("Save Level"), LEVELMENU_RETURN | LEVELMENU_NEW | LEVELMENU_DELETE | LEVELMENU_FILENAMES) == MENU_SELECT)
		    {

			if(strcmp(filename, "") == 0)
			{
			    /* Create new file */
			    sprintf(filename, "%s/%x.chroma", directory, (int)time(NULL));
			    level_save(plevelcurrent, filename, 1);
			}
			else
			{
			    /* Check file we're trying to save over */
			    pleveltest = level_load(filename, 1);

			    if(pleveltest != NULL && pleveltest->title != NULL)
			    {
				if(plevelcurrent->title == NULL || strcmp(plevelcurrent->title, pleveltest->title) != 0)
				{
	                            pmenuconfirm = menu_new(gettext("Confirm Save"));

	                            menuentry_new(pmenuconfirm, gettext("Do you wish to overwrite:"), 0, MENU_GREY);

	                            if(pleveltest->title == NULL || strcmp(pleveltest->title, "") == 0)
  	                                menuentry_new(pmenuconfirm, gettext("[untitled level]"), 0, MENU_GREY | MENU_RIGHT);
	                            else
  	                                menuentry_new(pmenuconfirm, pleveltest->title, 0, MENU_GREY | MENU_RIGHT);
	                            menuentry_new(pmenuconfirm, gettext("with:"), 0, MENU_GREY);

	                            if(plevelcurrent->title == NULL || strcmp(plevelcurrent->title, "") == 0)
  	                                menuentry_new(pmenuconfirm, gettext("[untitled level]"), 0, MENU_GREY | MENU_RIGHT);
	                            else
  	                                menuentry_new(pmenuconfirm, plevelcurrent->title, 0, MENU_GREY | MENU_RIGHT);

	                            menuentry_new(pmenuconfirm, "", 0, MENU_SPACE);

				    pentryno = menuentry_new(pmenuconfirm, gettext("No"), 'N', 0);
				    menuentry_new(pmenuconfirm, gettext("Yes"), 'Y', 0);

				    pmenuconfirm->entry_selected = pentryno;

				    result = menu_process(pmenuconfirm);
				    if(result == MENU_SELECT && pmenuconfirm->entry_selected != NULL && pmenuconfirm->entry_selected->key == 'Y')
			                level_save(plevelcurrent, filename, 1);

				    menu_delete(pmenuconfirm);

				}
				else
			            level_save(plevelcurrent, filename, 1);

			    }
			    else
			        level_save(plevelcurrent, filename, 1);

			    level_delete(pleveltest);
			}
		    }
		    break;

		case 'L':
                    getfilename("construct", directory, 1, 0);
		    if(menu_levelselector(directory, filename, gettext("Load Level"), LEVELMENU_RETURN | LEVELMENU_CREATE | LEVELMENU_DELETE | LEVELMENU_FILENAMES | LEVELMENU_IMPORT) == MENU_SELECT)
		    {
			/* Create new level */
			if(strcmp(filename, "") == 0)
			    plevelload = level_create(32, 23);
			/* Load existing level */
			else
			    plevelload = level_load(filename, 0);

			if(plevelload != NULL)
			{
			    level_delete(plevelcurrent);

			    plevelcurrent = plevelload;

			    /* Set editor position */
			    plevelcurrent->player_x[2] = 0;
			    plevelcurrent->player_y[2] = 0;
			}
		    }
		    break;

                case 'E':
                    export(plevelcurrent, 0);
                    break;

		case 'T':
		    pleveltest = level_copy(plevelcurrent);
		    level_fix(pleveltest);
                    pleveltest->flags |= LEVELFLAG_TESTING;
		    display_play(pleveltest, NULL);
		    level_delete(pleveltest);
		    break;

		case 'O':
		    plevelcurrent = editor_options(plevelcurrent);
		    break;

		case 'D':
		    display_options();
		    break;

		case 'V':
		    if(editor_verify(plevelcurrent))
			result = MENU_QUIT;
		    break;

		case 'Q':
		    result = MENU_QUIT;
		    break;

	    }
	}

	if(result == MENU_QUIT)
	{
	    if(plevelcurrent != NULL)
	        display_edit(plevelcurrent);
	    else
		ok = 1;
	}

	menu_delete(pmenu);

    }

    if(plevelcurrent != NULL)
	level_delete(plevelcurrent);
    plevelcurrent = NULL;
}

int editor_verify(struct level* plevel)
{
    struct menu* pmenu;
    struct menuentry* pentry;
    int result;
    int ok;
    int x, y;
    int p, p2;
    int d;
    int errors;
    int i;
    int count_playerone, count_playertwo, count_exit, count_teleport;
    int count_map_tl, count_map_tr, count_map_bl, count_map_br;
    int entries;
    char buffer[4096];
    int piece_ok[PIECE_MAX];

    pmenu = menu_new(gettext("Verify Level"));

    menuentry_new(pmenu, gettext("Return to Previous Menu"), 'Q', 0);

    menuentry_new(pmenu, "", 0, MENU_SPACE);

    errors = 0;
    count_playerone = 0;
    count_playertwo = 0;
    count_exit = 0;
    count_teleport = 0;
    count_map_tl = 0;
    count_map_tr = 0;
    count_map_bl = 0;
    count_map_br = 0;

    for(i = 0; i < PIECE_MAX; i ++)
	piece_ok[i] = 0;

    i = 0;
    while(editor_piece_maps[plevel->mode][i] != PIECE_GONE)
    {
	piece_ok[editor_piece_maps[plevel->mode][i]] = 1;
	i ++;
    }

    for(y = 0; y < plevel->size_y; y ++)
    {
	for(x = 0; x < plevel->size_x; x ++)
	{
	    p = level_piece(plevel, x, y);

	    if(p == PIECE_PLAYER_ONE)
		count_playerone ++;
	    if(p == PIECE_PLAYER_TWO)
		count_playertwo ++;
	    if(p == PIECE_DOOR)
		count_exit ++;
#ifdef XOR_COMPATIBILITY
	    if(p == PIECE_TELEPORT)
		count_teleport ++;
	    if(p == PIECE_MAP_TOP_LEFT)
		count_map_tl ++;
	    if(p == PIECE_MAP_TOP_RIGHT)
		count_map_tr ++;
	    if(p == PIECE_MAP_BOTTOM_LEFT)
		count_map_bl ++;
	    if(p == PIECE_MAP_BOTTOM_RIGHT)
		count_map_br ++;
#endif

	    if(!piece_ok[p])
	    {
		sprintf(buffer, gettext("Invalid piece %s at (%d,%d)"), gettext(piece_name[p]), x, y);
		pentry = menuentry_new(pmenu, buffer, 0, 0);
		sprintf(buffer, "%d %d", x, y);
		menuentry_value(pentry, buffer);
		errors ++;
	    }

	    if(p >= PIECE_MOVERS_FIRST && p <= PIECE_MOVERS_LAST)
	    {
		d = p % 4;
		p2 = level_piece(plevel, x + move_x[d], y + move_y[d]);

		if(canfall(p, p2, d))
		{
		    sprintf(buffer, gettext("Unsupported %s resting on %s at (%d,%d)"), gettext(piece_name[p]), gettext(piece_name[p2]), x, y);
	            pentry = menuentry_new(pmenu, buffer, 0, 0);
		    sprintf(buffer, "%d %d", x, y);
		    menuentry_value(pentry, buffer);
		    errors ++;
		}
	    }
	}
    }

    if(count_playerone == 0)
    {
	sprintf(buffer, gettext("No %s"), gettext(piece_name[PIECE_PLAYER_ONE]));
	pentry = menuentry_new(pmenu, buffer, 0, 0);
	errors ++;
    }

    if(count_playertwo == 0)
    {
#ifdef ENIGMA_COMPATIBILITY
        if(plevel->mode != MODE_ENIGMA)
        {
#endif
	    sprintf(buffer, gettext("No %s"), gettext(piece_name[PIECE_PLAYER_TWO]));
	    pentry = menuentry_new(pmenu, buffer, 0, 0);
	    errors ++;
#ifdef ENIGMA_COMPATIBILITY
        }
#endif
    }

    if(count_exit == 0)
    {
	sprintf(buffer, gettext("No %s"), gettext(piece_name[PIECE_DOOR]));
	pentry = menuentry_new(pmenu, buffer, 0, 0);
	errors ++;
    }

    if(count_playerone > 1)
    {
	sprintf(buffer, gettext("Too many %s"), gettext(piece_name[PIECE_PLAYER_ONE]));
	pentry = menuentry_new(pmenu, buffer, 0, 0);
	errors ++;
    }

    if(count_playertwo > 1)
    {
	sprintf(buffer, gettext("Too many %s"), gettext(piece_name[PIECE_PLAYER_TWO]));
	pentry = menuentry_new(pmenu, buffer, 0, 0);
	errors ++;
    }

#ifdef XOR_COMPATIBILITY
    if(plevel->mode == MODE_XOR)
    {
	if(count_teleport != 0 && count_teleport != 2)
	{
	    sprintf(buffer, gettext("Too many %s"), gettext(piece_name[PIECE_TELEPORT]));
	    pentry = menuentry_new(pmenu, buffer, 0, 0);
	    errors ++;
	}

	if(count_map_tl != 1)
	{
            if(count_map_tl == 0)
                sprintf(buffer, gettext("No %s"), gettext(piece_name[PIECE_MAP_TOP_LEFT]));
            else
                sprintf(buffer, gettext("Too many %s"), gettext(piece_name[PIECE_MAP_TOP_LEFT]));
	    pentry = menuentry_new(pmenu, buffer, 0, 0);
	    errors ++;
	}

	if(count_map_tr != 1)
	{
            if(count_map_tr == 0)
                sprintf(buffer, gettext("No %s"), gettext(piece_name[PIECE_MAP_TOP_RIGHT]));
            else
                sprintf(buffer, gettext("Too many %s"), gettext(piece_name[PIECE_MAP_TOP_RIGHT]));
	    pentry = menuentry_new(pmenu, buffer, 0, 0);
	    errors ++;
	}

	if(count_map_bl != 1)
	{
            if(count_map_bl == 0)
                sprintf(buffer, gettext("No %s"), gettext(piece_name[PIECE_MAP_BOTTOM_LEFT]));
            else
                sprintf(buffer, gettext("Too many %s"), gettext(piece_name[PIECE_MAP_BOTTOM_LEFT]));
	    pentry = menuentry_new(pmenu, buffer, 0, 0);
	    errors ++;
	}

	if(count_map_br != 1)
	{
            if(count_map_br == 0)
                sprintf(buffer, gettext("No %s"), gettext(piece_name[PIECE_MAP_BOTTOM_RIGHT]));
            else
                sprintf(buffer, gettext("Too many %s"), gettext(piece_name[PIECE_MAP_BOTTOM_RIGHT]));
	    pentry = menuentry_new(pmenu, buffer, 0, 0);
	    errors ++;
	}
    }
#endif

    entries = 1;
    pentry = pmenu->entry_first;
    while(pentry != NULL && entries < 11)
    {   
        if(pentry->key == 0 && !(pentry->flags & (MENU_GREY | MENU_NOTE | MENU_SPACE)))
        {   
            pentry->key = '0' + (entries % 10);
            entries ++;
        }

        pentry = pentry->next;
    }

    if(errors == 1)
        sprintf(buffer, gettext("%d error found"), errors);
    else
        sprintf(buffer, gettext("%d errors found"), errors);
    menuentry_new(pmenu, buffer, 0, MENU_NOTE | MENU_CENTRE); 

    ok = 0;
    while(!ok)
    {
        result = menu_process(pmenu);
	
	if(result == MENU_SELECT && pmenu->entry_selected != NULL)
	{
	    if(pmenu->entry_selected->value != NULL)
	    {
		i = sscanf(pmenu->entry_selected->value, "%d %d", &x, &y);
		if(i == 2)
		{
		    plevel->player_x[2] = x;
		    plevel->player_y[2] = y;
		    ok = 2;
		}

	    }
	    else
		ok = 1;
	}

	if(result == MENU_QUIT)
	    ok = 1;
    }

    menu_delete(pmenu);

    if(ok == 2)
	return 1;

    return 0;
}

struct level* editor_options(struct level* plevel)
{
    struct level* plevel_new;
    struct menu* pmenu;
    struct menuentry* pentry_mode;
    struct menuentry* pentry_left;
    struct menuentry* pentry_right;
    struct menuentry* pentry_top;
    struct menuentry* pentry_bottom;
    struct menuentry* pentry_width;
    struct menuentry* pentry_height;
    struct menuentry* pentry_rotate;
    struct menuentry* pentry_title;
    int result;
    int padding_left;
    int padding_right;
    int padding_top;
    int padding_bottom;
    int width, height;
    int delta_left, delta_right, delta_top, delta_bottom;
    int x, y;
    int ok;
    int rotate = 0;
    int p, d;
    char buffer[256];

    pmenu = menu_new(gettext("Level Options"));

    padding_left = 0; ok = 1;
    for(x = 1; x <= plevel->size_x - 2; x ++)
    {
	for(y = 1; y <= plevel->size_y - 2; y ++)
	{
	    if(level_piece(plevel, x, y) != PIECE_SPACE)
		ok = 0;
	}
	if(ok == 1)
	    padding_left ++;
	else
	    break;
    }

    padding_right = 0; ok = 1;
    for(x = plevel->size_x -2 ; x >= 1; x --)
    {
	for(y = 1; y <= plevel->size_y - 2; y ++)
	{
	    if(level_piece(plevel, x, y) != PIECE_SPACE)
		ok = 0;
	}
	if(ok == 1)
	    padding_right ++;
	else
	    break;
    }

    padding_top = 0; ok = 1;
    for(y = 1; y <= plevel->size_y - 2; y ++)
    {
	for(x = 1; x <= plevel->size_x - 2; x ++)
	{
	    if(level_piece(plevel, x, y) != PIECE_SPACE)
		ok = 0;
	}
	if(ok == 1)
	    padding_top ++;
	else
	    break;
    }

    padding_bottom = 0; ok = 1;
    for(y = plevel->size_y -2 ; y >= 1; y --)
    {
	for(x = 1; x <= plevel->size_x - 2; x ++)
	{
	    if(level_piece(plevel, x, y) != PIECE_SPACE)
		ok = 0;
	}
	if(ok == 1)
	    padding_bottom ++;
	else
	    break;
    }

    width = plevel->size_x;
    height = plevel->size_y;
    
    delta_left = 0;
    delta_right = 0;
    delta_top = 0;
    delta_bottom = 0;

    /* Create menu entries */
    menuentry_new(pmenu, gettext("Return to previous menu"), 'Q', 0);
    menuentry_new(pmenu, "", 0, MENU_SPACE);

    pentry_title = menuentry_new(pmenu, gettext("Title"), 'I', MENU_DOUBLE | MENU_EDITABLE);
    if(plevel->title != NULL)
        menuentry_extratext(pentry_title, NULL, NULL, plevel->title);
    else
        menuentry_extratext(pentry_title, NULL, NULL, "");

    menuentry_new(pmenu, "", 0, MENU_SPACE);

    if(MODE_CHROMA == MODE_MAX - 1)
        pentry_mode = NULL;
    else
    {
        pentry_mode = menuentry_new(pmenu, gettext("Level Mode"), 'M', MENU_SCROLLABLE);
        menuentry_new(pmenu, "", 0, MENU_SPACE);
    }

    pentry_width = menuentry_new(pmenu, gettext("Level width"), 0, MENU_GREY);
    pentry_left = menuentry_new(pmenu, gettext("Padding on left edge"), 'L', MENU_SCROLLABLE);
    pentry_right = menuentry_new(pmenu, gettext("Padding on right edge"), 'R', MENU_SCROLLABLE);
    menuentry_new(pmenu, "", 0, MENU_SPACE);

    pentry_height = menuentry_new(pmenu, gettext("Level height"), 0, MENU_GREY);
    pentry_top = menuentry_new(pmenu, gettext("Padding on top edge"), 'T', MENU_SCROLLABLE);
    pentry_bottom = menuentry_new(pmenu, gettext("Padding on bottom edge"), 'B', MENU_SCROLLABLE);
    menuentry_new(pmenu, "", 0, MENU_SPACE);

    pentry_rotate = menuentry_new(pmenu, gettext("Rotate"), 'O', MENU_SCROLLABLE);
    menuentry_new(pmenu, "", 0, MENU_SPACE);

    /* Menu loop */
    ok = 0;
    while(ok == 0)
    {
        if(plevel->mode == MODE_CHROMA)
            pentry_rotate->flags &= ~MENU_GREY;
        else
            pentry_rotate->flags |= MENU_GREY;

	/* Fill in values for menu entries */
        switch(plevel->mode)
        {
            case MODE_CHROMA:
                if(pentry_mode != NULL)
                    menuentry_extratext(pentry_mode, gettext("Chroma"), NULL, NULL);
                if(width != 32)
		    menuentry_text(pentry_width, gettext("Level width (Chroma standard is 32)"));
                else
		    menuentry_text(pentry_width, gettext("Level width"));

                if(height != 23)
                    menuentry_text(pentry_height, gettext("Level height (Chroma standard is 23)"));
                else
                    menuentry_text(pentry_height, gettext("Level height"));
                break;

#ifdef XOR_COMPATIBILITY
	    case MODE_XOR:
                menuentry_extratext(pentry_mode, gettext("XOR"), NULL, NULL);

                if(width != 32)
		    menuentry_text(pentry_width, gettext("Level width (XOR standard is 32)"));
                else
		    menuentry_text(pentry_width, gettext("Level width"));

                if(height != 32)
                    menuentry_text(pentry_height, gettext("Level height (XOR standard is 32)"));
                else
                    menuentry_text(pentry_height, gettext("Level height"));

                break;
#endif

#ifdef ENIGMA_COMPATIBILITY
	    case MODE_ENIGMA:
                menuentry_extratext(pentry_mode, gettext("Enigma"), NULL, NULL);

                if(width != 36)
		    menuentry_text(pentry_width, gettext("Level width (Enigma standard is 36)"));
                else
		    menuentry_text(pentry_width, gettext("Level width"));

                if(height != 20)
                    menuentry_text(pentry_height, gettext("Level height (Enigma standard is 20)"));
                else
                    menuentry_text(pentry_height, gettext("Level height"));

                break;
#endif

        }

        sprintf(buffer, "%d", width);
        menuentry_extratext(pentry_width, buffer, NULL, NULL);

        sprintf(buffer, "%d", padding_left);
        menuentry_extratext(pentry_left, buffer, NULL, NULL);

        sprintf(buffer, "%d", padding_right);
        menuentry_extratext(pentry_right, buffer, NULL, NULL);

        sprintf(buffer, "%d", height);
        menuentry_extratext(pentry_height, buffer, NULL, NULL);

        sprintf(buffer, "%d", padding_top);
        menuentry_extratext(pentry_top, buffer, NULL, NULL);

        sprintf(buffer, "%d", padding_bottom);
        menuentry_extratext(pentry_bottom, buffer, NULL, NULL);

        if(plevel->mode == MODE_CHROMA)
        {
            sprintf(buffer, gettext("%d degrees"), rotate * 90);
            menuentry_extratext(pentry_rotate, buffer, NULL, NULL);
        }
        else
            menuentry_extratext(pentry_rotate, gettext("(only for Chroma levels)"), NULL, NULL);

	/* Display menu */
        result = menu_process(pmenu);

	/* Process results */
        if(result == MENU_QUIT)
            ok = 1;

        if(result == MENU_SELECT && pmenu->entry_selected != NULL)
        {   
	    switch(pmenu->entry_selected->key)
	    {   
		case 'Q':
		    ok = 1;
		    break;
	    }
	}

        if(result == MENU_SCROLLLEFT && pmenu->entry_selected != NULL)
        {   
            switch(pmenu->entry_selected->key)
            {   
		case 'M':
		    plevel->mode --;
		    if(plevel->mode < 0)
			plevel->mode = MODE_MAX - 1;
#ifdef XOR_COMPATIBILITY
		    if(plevel->mode == MODE_XOR)
		        options_xor_options = 1;
#endif
#ifdef ENIGMA_COMPATIBILITY
		    if(plevel->mode == MODE_ENIGMA)
		        options_enigma_options = 1;
#endif
		    pentry_height->redraw = 1;
		    pentry_width->redraw = 1;
		    pentry_mode->redraw = 1;
                    pentry_rotate->redraw = 1;
		    break;
		    
		case 'L':
		    if(width > 2)
		    {
		        delta_left --;
		        padding_left --;
		        width --;

		        pentry_left->redraw = 1;
		        pentry_width->redraw = 1;
		    }
		    break;

		case 'R':
		    if(width > 2)
		    {
		        delta_right --;
		        padding_right --;
		        width --;

		        pentry_right->redraw = 1;
		        pentry_width->redraw = 1;
		    }
		    break;

		case 'T':
		    if(height > 2)
		    {
		        delta_top --;
		        padding_top --;
		        height --;

		        pentry_top->redraw = 1;
		        pentry_height->redraw = 1;
		    }
		    break;

		case 'B':
		    if(height > 2)
		    {
		        delta_bottom --;
		        padding_bottom --;
		        height --;

		        pentry_bottom->redraw = 1;
		        pentry_height->redraw = 1;
		    }
		    break;

                case 'O':
                    rotate -= 1;
                    if(rotate == -1)
                        rotate = 3;
                    break;

	    }

	}

        if(result == MENU_SCROLLRIGHT && pmenu->entry_selected != NULL)
        {   
            switch(pmenu->entry_selected->key)
            {   
		case 'M':
		    plevel->mode ++;
		    if(plevel->mode == MODE_MAX)
			plevel->mode = 0;
#ifdef XOR_COMPATIBILITY
		    if(plevel->mode == MODE_XOR)
		        options_xor_options = 1;
#endif
#ifdef ENIGMA_COMPATIBILITY
		    if(plevel->mode == MODE_ENIGMA)
		        options_enigma_options = 1;
#endif
		    pentry_height->redraw = 1;
		    pentry_width->redraw = 1;
		    pentry_mode->redraw = 1;
                    pentry_rotate->redraw = 1;
		    break;
		    
		case 'L':
		    if(width < 256)
		    {
		        delta_left ++;
		        padding_left ++;
		        width ++;

		        pentry_left->redraw = 1;
		        pentry_width->redraw = 1;
		    }
		    break;

		case 'R':
		    if(width < 256)
		    {
		        delta_right ++;
		        padding_right ++;
		        width ++;

		        pentry_right->redraw = 1;
		        pentry_width->redraw = 1;
		    }
		    break;

		case 'T':
		    if(height < 256)
		    {
		        delta_top ++;
		        padding_top ++;
		        height ++;

		        pentry_top->redraw = 1;
		        pentry_height->redraw = 1;
		    }
		    break;

		case 'B':
		    if(height < 256)
		    {
		        delta_bottom ++;
		        padding_bottom ++;
		        height ++;

		        pentry_bottom->redraw = 1;
		        pentry_height->redraw = 1;
		    }
		    break;

                case 'O':
                    rotate += 1;
                    if(rotate == 4)
                        rotate = 0;
                    break;
	    }
	}
    }

    /* Set new title */
    if(pentry_title->text4 != NULL)
    {
        level_settitle(plevel, pentry_title->text4);
        /* Easier than editing the configuration file! */
	if(strcmp(pentry_title->text4, "debug") == 0)
	    options_debug |= DEBUG_MENU;
    }

    /* Resize the level if necessary */
    if(delta_left != 0 || delta_right != 0 || delta_top != 0 || delta_bottom != 0 )
    {

	/* Create a new level */
	plevel_new = level_create(width, height);

	/* Copy pieces into it */
	for(x = 1; x <= plevel->size_x - 2; x ++)
	{
	    for(y = 1; y <= plevel->size_y - 2; y ++)
	    {
		level_setpiece(plevel_new, x + delta_left, y + delta_top, level_piece(plevel, x, y));
	    }
	}

	/* Force borders */
	for(x = 0; x<= plevel_new->size_x - 1; x ++)
	{
	    level_setpiece(plevel_new, x, 0, PIECE_WALL);
	    level_setpiece(plevel_new, x, plevel_new->size_y - 1, PIECE_WALL);
	}
	for(y = 0; y<= plevel_new->size_y - 1; y ++)
	{
	    level_setpiece(plevel_new, 0, y, PIECE_WALL);
	    level_setpiece(plevel_new, plevel_new->size_x - 1, y, PIECE_WALL);
	}

        /* If the original padding on any edge was zero, add a wall */
        if(padding_left - delta_left == 0)
        {
            for(y = 1; y <= plevel->size_y - 2; y ++)
            {
                level_setpiece(plevel_new, padding_left, y, PIECE_WALL);
            }
        }
        if(padding_right - delta_right == 0)
        {
            for(y = 1; y <= plevel->size_y - 2; y ++)
            {
                level_setpiece(plevel_new, plevel_new->size_x - 1 - padding_right, y, PIECE_WALL);
            }
        }
        if(padding_top - delta_top == 0)
        {
            for(x = 1; x <= plevel->size_x - 2; x ++)
            {
                level_setpiece(plevel_new, x, padding_top, PIECE_WALL);
            }
        }
        if(padding_bottom - delta_bottom == 0)
        {
            for(x = 1; x <= plevel->size_x - 2; x ++)
            {
                level_setpiece(plevel_new, x, plevel_new->size_y - 1 - padding_bottom, PIECE_WALL);
            }
        }

	/* Copy level options */
	plevel_new->mode = plevel->mode;
	level_settitle(plevel_new, plevel->title);

	/* Delete old level */
	level_delete(plevel);
	plevel = plevel_new;
    }

    /* Rotate the level if necessary */
    if(rotate != 0)
    {
        if(rotate == 2)
        {
            width = plevel->size_x;
            height = plevel->size_y;
        }
        else
        {
            width = plevel->size_y;
            height = plevel->size_x;
        }

        /* Create a new level */
        plevel_new = level_create(width, height);

	/* Copy pieces into it */
	for(x = 1; x <= plevel_new->size_x - 2; x ++)
	{
	    for(y = 1; y <= plevel_new->size_y - 2; y ++)
	    {
                switch(rotate)
                {
                    case 1:
                        p = level_piece(plevel, y, plevel->size_y - 1 - x);
                        break;
                    case 2:
                        p = level_piece(plevel, plevel->size_x - 1 - x, plevel->size_y - 1 - y);
                        break;
                    case 3:
                        p = level_piece(plevel, plevel->size_x - 1 - y, x);
                        break;
                    default:
                        p = level_piece(plevel, x, y);
                        break;
                }
                if(p >= PIECE_MOVERS_FIRST && p <= PIECE_MOVERS_LAST)
                {
                    d = p % 4;
                    p = p - d + ((d + rotate) % 4);
                }
		level_setpiece(plevel_new, x, y, p);
	    }
	}

	/* Force borders */
	for(x = 0; x<= plevel_new->size_x - 1; x ++)
	{
	    level_setpiece(plevel_new, x, 0, PIECE_WALL);
	    level_setpiece(plevel_new, x, plevel_new->size_y - 1, PIECE_WALL);
	}
	for(y = 0; y<= plevel_new->size_y - 1; y ++)
	{
	    level_setpiece(plevel_new, 0, y, PIECE_WALL);
	    level_setpiece(plevel_new, plevel_new->size_x - 1, y, PIECE_WALL);
	}

	/* Copy level options */
	plevel_new->mode = plevel->mode;

	level_settitle(plevel_new, plevel->title);

	/* Delete old level */
	level_delete(plevel);
	plevel = plevel_new;
    }

    return plevel;
}

