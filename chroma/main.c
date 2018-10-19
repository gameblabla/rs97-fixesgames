/*  
    main.c

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

#ifdef __WIN32__
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <libintl.h>
#include <locale.h>

#include "chroma.h"
#include "level.h"
#include "menu.h"
#include "display.h"
#include "colours.h"
#include "util.h"

void editor();
void help();
void export(struct level*, int);

extern int options_xor_mode;
extern int options_xor_display;
struct level* plevelcurrent = NULL;

#ifdef __WIN32__
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    main(0, NULL);
}
#endif

int main(int argc, char **argv)
{
    struct menu* pmenu;
    struct menuentry* pentryplay;
    int result;
    int ok;
    char directory[FILENAME_MAX];

    getfilename("locale", directory, 0, LOCATION_SYSTEM);
    setlocale(LC_ALL, "");
    bindtextdomain("chroma", directory);
    textdomain("chroma");

    display_init();

    pmenu = menu_new(gettext("Chroma"));
    pmenu->logo = 1;

    pentryplay = menuentry_new(pmenu, gettext("Press any key to play"), MENU_KEY_ANY, MENU_CENTRE);
    menuentry_new(pmenu, "", 0, MENU_SPACE);
    menuentry_new(pmenu, gettext("How to Play"), 'H', 0);
    menuentry_new(pmenu, gettext("Editor"), 'E', 0);
    menuentry_new(pmenu, gettext("Display Options"), 'D', 0);
    menuentry_new(pmenu, gettext("Quit"), 'Q', 0);

    ok = 0;
    while(!ok)
    {
        pmenu->entry_selected = pentryplay;
        result = menu_process(pmenu);
        switch(result)
        {
            case MENU_SELECT:
                switch(pmenu->entry_selected->key)
                {
                    case MENU_KEY_ANY:
                        while(menu_levelselector(NULL, NULL, gettext("Choose a level"), LEVELMENU_MAIN | LEVELMENU_SAVED | LEVELMENU_SOLVED | LEVELMENU_GAME) != MENU_QUIT);
                        break;

		    case 'H':
			help();
			break;

                    case 'E':
                        editor();
                        break;

                    case 'D':
                        display_options();
                        break;

                    case 'Q':
                        ok = 1;
                        break;
                }
                break;

            case MENU_QUIT:
                ok = 1;
                break;

            default:
                break;
        }
    }

    exit(0);
}

void level_play(struct level* plevelgame)
{
    struct menu* pmenu;

    struct menuentry* pentry;
    struct level* plevelrevert;
    struct level* plevelload;
    struct level *plevelreplay;

    int result;
    int quit;
    int ok;

    char filename[FILENAME_MAX];
    char directory[FILENAME_MAX];
    char menutitle[256];

    plevelrevert = level_copy(plevelgame);
    plevelreplay = NULL;

    plevelcurrent = plevelgame;

    quit = 0;

    while(!quit)
    {
	display_play(plevelgame, plevelreplay);

        /* Has the replay finished? */
	if(plevelreplay != NULL && plevelreplay->move_current == NULL)
	{
	    level_delete(plevelreplay);
	    plevelreplay = NULL;
	}

	ok = 0;
	while(!ok)
	{
	    pmenu = menu_new(gettext("Game Options"));

    	    menuentry_new(pmenu, gettext("Return to Level"), 'Q', 0);
	    menuentry_new(pmenu, gettext("Abort Level and Return to Selection"), 'A', 0);

	    if(plevelrevert->moves != plevelgame->moves)
	        menuentry_new(pmenu, gettext("Revert to Original Position"), 'V', 0);
	    else
	    {
	        pentry = menuentry_new(pmenu, gettext("Revert to Original Position"), 'V', MENU_GREY);
	        menuentry_extratext(pentry, gettext("(only after changes)"), NULL, NULL);
	    }

	    menuentry_new(pmenu, "", 0, MENU_SPACE);

	    menuentry_new(pmenu, gettext("Load Position"), 'L', 0);

	    if(plevelgame->mover_first == NULL)
            {

	        menuentry_new(pmenu, gettext("Save Position"), 'S', 0);

                if(plevelgame->flags & LEVELFLAG_SOLVED)
	            menuentry_new(pmenu, gettext("Export Solution"), 'E', 0);
            }
	    else
	    {
	        pentry = menuentry_new(pmenu, gettext("Save Position"), 'S', MENU_GREY);
	        menuentry_extratext(pentry, gettext("(not while pieces in motion)"), NULL, NULL);
                if(plevelgame->flags & LEVELFLAG_SOLVED)
                {
	            pentry = menuentry_new(pmenu, gettext("Export Solution"), 'E', MENU_GREY);
	            menuentry_extratext(pentry, gettext("(not while pieces in motion)"), NULL, NULL);
                }
	    }

	    menuentry_new(pmenu, "", 0, MENU_SPACE);

	    if(plevelreplay == NULL)
	    {
	        if(plevelgame->moves == 0)
	        {
	            menuentry_new(pmenu, gettext("Replay Saved Position"), 'R', 0);
                    menuentry_new(pmenu, "", 0, MENU_SPACE);
	        }
                else
                {
	            pentry = menuentry_new(pmenu, gettext("Replay Saved Position"), 'R', MENU_GREY);
		    menuentry_extratext(pentry, gettext("(only at start of a level)"), NULL, NULL);

                    menuentry_new(pmenu, "", 0, MENU_SPACE);
                }
	    }
	    else
	    {
	        menuentry_new(pmenu, gettext("Stop Replaying Position"), 'T', 0);
                menuentry_new(pmenu, "", 0, MENU_SPACE);
	    }

	    menuentry_new(pmenu, gettext("Display Options"), 'D', 0);

	    menuentry_new(pmenu, gettext("Currently playing:"), 0, MENU_NOTE);
	    if(plevelgame->title == NULL || strcmp(plevelgame->title, "") == 0)
	        menuentry_new(pmenu, gettext("[untitled level]"), 0, MENU_NOTE | MENU_RIGHT);
	    else
	        menuentry_new(pmenu, gettext(plevelgame->title), 0, MENU_NOTE | MENU_RIGHT);

	    result = menu_process(pmenu);

	    if(result == MENU_QUIT)
	        ok = 1;

	    if(result == MENU_SELECT && pmenu->entry_selected != NULL)
	    {
	        switch(pmenu->entry_selected->key)
	        {
		    case 'Q':
		        ok =1;
		        break;

		    case 'A':
		        ok = 1; quit = 1;
		        break;

		    case 'S':
                        /* Are there pieces still in motion? */
                        if(plevelgame->mover_first != NULL)
                            break;

		        if(plevelgame->flags & LEVELFLAG_SOLVED && !(plevelgame->flags & LEVELFLAG_FAILED))
		        {
		            getfilename("solved", directory, 1, LOCATION_LOCAL);
			    strcpy(menutitle, gettext("Save Solution"));
		        }
		        else
		        {
		            getfilename("saved", directory, 1, LOCATION_LOCAL);
			    strcpy(menutitle, gettext("Save Position"));
		        }

		        if(menu_levelselector(directory, filename, menutitle, LEVELMENU_RETURN | LEVELMENU_NEW | LEVELMENU_DELETE | LEVELMENU_MOVES | LEVELMENU_FILTER) == MENU_SELECT)
		        {
		            if(strcmp(filename, "") == 0)
                                sprintf(filename, "%s/%x.chroma", directory, (int)time(NULL));
		            level_save(plevelgame, filename, 0);

			    level_delete(plevelrevert);
			    plevelrevert = level_copy(plevelgame);
			    ok = 1;
		        }
		        break;

		    case 'L':
		        getfilename("saved", directory, 0, LOCATION_LOCAL);
		        if(menu_levelselector(directory, filename, gettext("Load Position"), LEVELMENU_RETURN | LEVELMENU_DELETE | LEVELMENU_MOVES | LEVELMENU_FILTER) == MENU_SELECT)
		        {
			    plevelload = level_load(filename, 0);
			    if(plevelload != NULL)
			    {
                                if(plevelreplay != NULL)
                                {
                                    level_delete(plevelreplay);
                                    plevelreplay = NULL;
                                }
			        level_delete(plevelgame);
			        level_delete(plevelrevert);

			        plevelrevert = plevelload;
			        plevelgame = level_copy(plevelrevert);
                                plevelcurrent = plevelgame;
			        ok = 1;
			    }
		        }
		        break;

		    case 'E':
                        /* Are there pieces still in motion? */
                        if(plevelgame->mover_first != NULL)
                            break;
                        export(plevelgame, 1);
                        break;

		    case 'V':
		        if(plevelreplay != NULL)
		        {
			    level_delete(plevelreplay);
			    plevelreplay = NULL;
		        }
		        level_delete(plevelgame);
		        plevelgame = level_copy(plevelrevert);
                        plevelcurrent = plevelgame;
		        ok = 1;
		        break;

		    case 'R':
		        getfilename("saved", directory, 0, LOCATION_LOCAL);
		        if(menu_levelselector(directory, filename, gettext("Replay Saved Position"), LEVELMENU_RETURN | LEVELMENU_DELETE | LEVELMENU_MOVES | LEVELMENU_SOLVED | LEVELMENU_FILTER) == MENU_SELECT)
		        {
			    plevelreplay = level_load(filename, 0);
			    plevelreplay->move_current = plevelreplay->move_first;
			    ok = 1;
		        }
		        break;

		    case 'T':
		        level_delete(plevelreplay);
		        plevelreplay = NULL;
		        break;

		    case 'D':
		        display_options();
		        break;
	        }
	    }
	}

	menu_delete(pmenu);
    }

    if(plevelrevert != NULL)
	level_delete(plevelrevert);
    if(plevelreplay != NULL)
	level_delete(plevelreplay);
    if(plevelgame != NULL)
	level_delete(plevelgame);

    plevelcurrent = NULL;
}

void help()
{
    struct menu* pmenu;
    char directory[FILENAME_MAX];
    char filename[FILENAME_MAX];
    char buffer[FILENAME_MAX];
    char *locale;
    int ok;
    int i;

    pmenu = menu_new(gettext("How to Play"));

    menuentry_new(pmenu, gettext("Quit and return to previous menu"), 'Q', 0);
    menuentry_new(pmenu, gettext("Use [UP] and [DOWN] to scroll"), 0, MENU_NOTE | MENU_CENTRE);
    menuentry_new(pmenu, "", 0, MENU_TEXT);

    getfilename("help/README", directory, 0, LOCATION_SYSTEM);

    locale = setlocale(LC_MESSAGES, NULL);

    /* Given a locale of, say, en_GB.UTF-8, try help/README.en_GB.UTF-8, then
       help/README.en_GB and finally help/README.en before defaulting to just
       help/README - if we can't get even that, generate an error. */
    ok = 0;
    if(locale != NULL)
    {
	i = 0;
        while(locale[i] != 0 && locale[i] != '/')
	{
	    buffer[i] = locale[i]; i ++;
	}
	buffer[i] = 0;
	sprintf(filename, "%s.%s", directory, buffer);

        if(menu_addfile(pmenu, filename))
	    ok = 1;
    }
    else
	ok = 2;
    if(ok == 0)
    {
	i = 0;
        while(locale[i] != 0 && locale[i] != '/' && locale[i] != '.')
	{
	    buffer[i] = locale[i]; i ++;
	}
	buffer[i] = 0;
	sprintf(filename, "%s.%s", directory, buffer);

        if(menu_addfile(pmenu, filename))
	    ok = 1;
    }
    if(ok == 0)
    {
	i = 0;
        while(locale[i] != 0 && locale[i] != '/' && locale[i] != '.' && locale[i] != '_')
	{
	    buffer[i] = locale[i]; i ++;
	}
	buffer[i] = 0;
	sprintf(filename, "%s.%s", directory, buffer);

        if(menu_addfile(pmenu, filename))
	    ok = 1;
    }
    if(ok != 1)
    {
        strcpy(filename, directory);
        if(!menu_addfile(pmenu, filename))
	    menuentry_new(pmenu, "Unable to read help file!", 0, MENU_GREY | MENU_CENTRE);
    }

    menu_process(pmenu);

}

void export(struct level* plevel, int solution)
{
    struct menu *pmenu;
    struct menuentry *pentry_filename;
    char filename[FILENAME_MAX];
    int ok, result;
    int i, j;

    if(!solution)
        pmenu = menu_new(gettext("Export Level"));
    else
        pmenu = menu_new(gettext("Export Solution"));

    menuentry_new(pmenu, gettext("Quit and return to previous menu"), 'Q', 0);
    menuentry_new(pmenu, "", 0, MENU_SPACE);

    pentry_filename = menuentry_new(pmenu, gettext("Filename"), 'I', MENU_DOUBLE | MENU_EDITABLE);
    if(solution)
    {
        if(plevel->title != NULL && sscanf(plevel->title, "chroma %d.%d", &i, &j) == 2)
            sprintf(filename, "solution-%d-%02d", i, j);
        else
            strcpy(filename, "solution");
    }
    else
    {
        if(plevel->title != NULL)
            strcpy(filename, plevel->title);
        else
            strcpy(filename, "level");
    }

    menuentry_extratext(pentry_filename, NULL, NULL, filename);

    if(!solution)
        menuentry_new(pmenu, gettext("Export level"), 'E', 0);
    else
        menuentry_new(pmenu, gettext("Export solution"), 'E', 0);

#ifdef __WIN32__
    if(!solution)
        menuentry_new(pmenu, gettext("Exported levels are saved in \"My Documents\""), 0, MENU_NOTE | MENU_CENTRE);
    else
        menuentry_new(pmenu, gettext("Exported solutions are saved in \"My Documents\""), 0, MENU_NOTE | MENU_CENTRE);
#else
    if(!solution)
        menuentry_new(pmenu, gettext("Exported levels are saved in your home directory"), 0, MENU_NOTE | MENU_CENTRE);
    else
        menuentry_new(pmenu, gettext("Exported solutions are saved in your home directory"), 0, MENU_NOTE | MENU_CENTRE);
#endif

    ok = 0;
    while(!ok)
    {
        result = menu_process(pmenu);

        if(result == MENU_QUIT)
            ok = 1;

        if(result == MENU_SELECT && pmenu->entry_selected != NULL)
        {  
            switch(pmenu->entry_selected->key)
            {  
                case 'E':
                    getfilename("", filename, 0, LOCATION_DOCUMENTS);
                    i = 0;
                    j = strlen(filename);
                    while(pentry_filename->text4[i] != 0)
                    {
                        if(pentry_filename->text4[i] != '/' && pentry_filename->text4[i] != '\\')
                            filename[j ++] = pentry_filename->text4[i ++];
                    }
                    filename[j ++] = 0;
                    if(!(strlen(filename) > 7 && strcmp(filename + strlen(filename) - 7, ".chroma") == 0))
                        strcat(filename, ".chroma");

                    level_save(plevel, filename, 1);
#ifndef __WIN32__
                    if(solution)
                        chmod(filename, 00600);
#endif
                    
                    ok = 1;
                    break;

                case 'Q':
                    ok = 1;
                    break;
            }
        }
    }

    menu_delete(pmenu);
}
