/*  
    menu.c

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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <libintl.h>
#include <locale.h>

#include "chroma.h"
#include "menu.h"
#include "level.h"
#include "util.h"
#include "xmlparser.h"

int menu_levelentries(struct menu* pmenu, char *directory, int flags);
void level_play(struct level* plevelgame);

extern struct level* plevelcurrent;

extern int options_debug;
#ifdef XOR_COMPATIBILITY
extern int options_xor_options;
#endif
#ifdef ENIGMA_COMPATIBILITY
extern int options_enigma_options;
#endif

struct menu *menu_new(char *title)
{
    struct menu* pmenu;

    pmenu = (struct menu*)malloc(sizeof(struct menu));

    if(pmenu == NULL)
        fatal(gettext("Out of memory in menu_new()"));

    pmenu->title = (char *)malloc(strlen(title) + 1);

    if(pmenu->title == NULL)
        fatal(gettext("Out of memory in menu_new()"));
     
    strcpy(pmenu->title, title);

    pmenu->entry_first = NULL;
    pmenu->entry_last = NULL;
    pmenu->entry_selected = NULL;
    pmenu->display_first = NULL;
    pmenu->display_last = NULL;
    pmenu->offset = 0;
    pmenu->redraw = MENUREDRAW_ALL;
    pmenu->logo = 0;

    return pmenu;
}

void menu_delete(struct menu *pmenu)
{
    struct menuentry *pentry;
    struct menuentry *pentrytmp;

    pentry = pmenu->entry_first;

    while(pentry != NULL)
    {
	pentrytmp = pentry;
	pentry = pentry->next;

	if(pentrytmp->text != NULL)
	    free(pentrytmp->text);
	if(pentrytmp->text2 != NULL)
	    free(pentrytmp->text2);
	if(pentrytmp->text3 != NULL)
	    free(pentrytmp->text3);
	if(pentrytmp->text4 != NULL)
	    free(pentrytmp->text4);
	if(pentrytmp->value != NULL)
	    free(pentrytmp->value);

	free(pentrytmp);
    }

    if(pmenu->title != NULL)
	free(pmenu->title);

    free(pmenu);
}

void menu_unsort(struct menu *pmenu)
{
    struct menuentry* pentry;

    if(pmenu == NULL)
	return;

    pentry = pmenu->entry_first;
    while(pentry != NULL)
    {
	pentry->flags &= ~(MENU_SORT | MENU_TOPSORT);
	pentry = pentry->next;
    }
}

void menu_assignletters(struct menu *pmenu)
{
    struct menuentry* pentry;
    struct menuentry* pentry_before;
    char c;

    if(pmenu == NULL)
	return;

    c = '1';

    pentry = pmenu->entry_first;
    while(pentry != NULL)
    {
	if(pentry->key == 0 && !(pentry->flags & (MENU_GREY | MENU_NOTE | MENU_SPACE)))
	{
	    pentry_before = pmenu->entry_first;
	    while(pentry_before != NULL && pentry_before != pentry)
	    {
		if(pentry_before->key == c)
		{
		    c ++;

		    /* Valid keys are 1-9 A-P R-Z. Q is deliberately omitted. */
		    if(c == '9' + 1)
			c = 'A';
		    if(c == 'Q')
			c ++;
		    if(c == 'Z' + 1)
			return;

		    pentry_before = pmenu->entry_first;
		}
		else
		    pentry_before = pentry_before ->next;
	    }
	    pentry->key = c;
	}
	pentry = pentry->next;
    }
}

struct menuentry* menuentry_new(struct menu *pmenu, char *text, char key, int flags)
{
    return menuentry_newwithvalue(pmenu, text, key, flags, NULL);
}

struct menuentry* menuentry_newwithvalue(struct menu *pmenu, char *text, char key, int flags, char *value)
{
    struct menuentry* pentry;
    struct menuentry* pentry_before;

    pentry = (struct menuentry*)malloc(sizeof(struct menuentry));

    if(pentry == NULL)
	fatal(gettext("Out of memory in menuentry_newwithvalue()"));
    
    if(flags & MENU_SORT)
    {
	pentry_before = pmenu->entry_first;
	while(pentry_before != NULL)
	{
	    if((pentry_before->flags & MENU_SORT) && (!(pentry_before->flags & MENU_TOPSORT)) && pentry_before->value != NULL)
	    {
		if(strcmp(pentry_before->value, value) > 0)
		    break;
	    }
	    pentry_before = pentry_before->next;
	}
    }
    else if(flags & MENU_TOPSORT)
    {
	pentry_before = pmenu->entry_first;
	while(pentry_before != NULL)
	{
	    if((pentry_before->flags & MENU_TOPSORT) && pentry_before->value != NULL)
	    {
		if(strcmp(pentry_before->value, value) > 0)
		    break;
	    }

	    if(pentry_before->flags & MENU_SORT && !(pentry_before->flags & MENU_TOPSORT))
		break;

	    pentry_before = pentry_before->next;
	}
    }
    else
        pentry_before = NULL;

    /* If pentry_before is NULL, we add to the bottom of the list.
       Otherwise, we add before pentry_before.
     */
    if(pentry_before == NULL)
    {
	if(pmenu->entry_first == NULL)
	    pmenu->entry_first = pentry;

	pentry->previous = pmenu->entry_last;
	pentry->next = NULL;

	if(pmenu->entry_last != NULL)
	    pmenu->entry_last->next = pentry;

	pmenu->entry_last = pentry;
    }
    else
    {
	if(pentry_before == pmenu->entry_first)
	    pmenu->entry_first = pentry;
	pentry->previous = pentry_before->previous;
	pentry->next = pentry_before;
	if(pentry_before->previous != NULL)
	    pentry_before->previous->next = pentry;
	pentry_before->previous = pentry;
    }

    pentry->text = (char *)malloc(strlen(text) + 1);

    if(pentry->text == NULL)
	fatal(gettext("Out of memory in menuentry_newwithvalue()"));

    strcpy(pentry->text, text);

    pentry->redraw = 0;

    pentry->flags = flags;

    pentry->key = key;

    pentry->value = NULL;
    if(value != NULL)
    {
        pentry->value = (char *)malloc(strlen(value) + 1);

        if(pentry->value == NULL)
	    fatal(gettext("Out of memory in menuentry_newwithvalue()"));

        strcpy(pentry->value, value);
    }

    pentry->text2 = NULL;
    pentry->text3 = NULL;
    pentry->text4 = NULL;

    return pentry;
}

void menuentry_value(struct menuentry* pentry, char *value)
{
    if(pentry->value != NULL)
	free(pentry->value);

    pentry->value = (char *)malloc(strlen(value) + 1);

    if(pentry->value == NULL)
	fatal(gettext("Out of memory in menuentry_value()"));

    strcpy(pentry->value, value);
}

void menuentry_text(struct menuentry* pentry, char *text)
{
    if(pentry->text != NULL)
	free(pentry->text);

    pentry->text = (char *)malloc(strlen(text) + 1);

    if(pentry->text == NULL)
	fatal(gettext("Out of memory in menuentry_text()"));

    strcpy(pentry->text, text);
}

void menuentry_extratext(struct menuentry* pentry, char *text2, char *text3, char *text4)
{
    if(pentry->text2 != NULL)
	free(pentry->text2);
    if(pentry->text3 != NULL)
	free(pentry->text3);
    if(pentry->text4 != NULL)
	free(pentry->text4);

    if(text2 != NULL)
    {
        pentry->text2 = (char *)malloc(strlen(text2) + 1);
        if(pentry->text2 == NULL)
	    fatal(gettext("Out of memory in menuentry_extratext()"));

        strcpy(pentry->text2, text2);
    }
    else
	pentry->text2 = NULL;

    if(text3 != NULL)
    {
        pentry->text3 = (char *)malloc(strlen(text3) + 1);
        if(pentry->text3 == NULL)
	    fatal(gettext("Out of memory in menuentry_extratext()"));

        strcpy(pentry->text3, text3);
    }
    else
	pentry->text3 = NULL;

    if(text4 != NULL)
    {
        pentry->text4 = (char *)malloc(strlen(text4) + 1);
        if(pentry->text4 == NULL)
	    fatal(gettext("Out of memory in menuentry_extratext()"));

        strcpy(pentry->text4, text4);
    }
    else
	pentry->text4 = NULL;
}

void menuentry_delete(struct menu* pmenu, struct menuentry* pentry)
{
    struct menuentry* pentry_replace;

    pentry_replace = pentry->next;
    if(pentry_replace == NULL)
	pentry_replace = pentry->previous;

    if(pmenu->entry_first == pentry)
	pmenu->entry_first = pentry_replace;
    if(pmenu->display_first == pentry)
	pmenu->display_first = pentry_replace;
    if(pmenu->entry_last == pentry)
	pmenu->entry_last = pentry_replace;
    if(pmenu->entry_selected == pentry)
	pmenu->entry_selected = pentry_replace;
    if(pentry->previous != NULL)
	pentry->previous->next = pentry->next;
    if(pentry->next != NULL)
	pentry->next->previous = pentry->previous;

    if(pentry->text != NULL)
	free(pentry->text);
    if(pentry->text2 != NULL)
	free(pentry->text2);
    if(pentry->text3 != NULL)
	free(pentry->text3);
    if(pentry->text4 != NULL)
	free(pentry->text4);
    if(pentry->value != NULL)
	free(pentry->value);

    free(pentry);
}

int menu_levelselector(char *directory_levels, char *chosen, char *title, int flags)
{
    struct menu* pmenu;
    struct level* plevel;
    int flags_child;
    int result;
    char directory[FILENAME_MAX];

    pmenu = menu_new(title);

    if(flags & LEVELMENU_RETURN)
        menuentry_new(pmenu, gettext("Return to previous menu"), 'Q', 0);

    if(flags & LEVELMENU_NEW)
	menuentry_new(pmenu, gettext("Save as new position"), 'S', 0);

    if(flags & LEVELMENU_CREATE)
	menuentry_new(pmenu, gettext("Create new level"), 'C', 0);

    if(flags & LEVELMENU_IMPORT)
    {
	getfilename("", directory, 0, LOCATION_DOCUMENTS);
	if(isdirectory(directory))
	    menuentry_newwithvalue(pmenu, gettext("Import a position"), 'I', 0, directory);
    }

    if(flags & LEVELMENU_SAVED)
    {
	getfilename("saved", directory, 0, LOCATION_LOCAL);
	if(isdirectory(directory))
	    menuentry_newwithvalue(pmenu, gettext("Saved Positions"), 'S', MENU_BOLD, directory);
    }

    if(flags & LEVELMENU_SOLVED)
    {
	getfilename("solved", directory, 0, LOCATION_LOCAL);
	if(isdirectory(directory))
	    menuentry_newwithvalue(pmenu, gettext("Solved Positions"), 'O', MENU_BOLD, directory);
    }

    if(flags & LEVELMENU_MAIN)
    {
	getfilename("construct", directory, 0, LOCATION_LOCAL);
	if(isdirectory(directory))
 	    menuentry_newwithvalue(pmenu, gettext("Levels under Construction"), 'C', MENU_BOLD, directory);
    }

    /* Separator only if needed */
    if(pmenu->entry_first != NULL)
        menuentry_new(pmenu, "", 0, MENU_TOPSORT | MENU_SPACE);

    if(flags & LEVELMENU_DELETE)
        menuentry_new(pmenu, gettext("[DELETE] to delete a position"), 0, MENU_NOTE | MENU_CENTRE);

    if(flags & LEVELMENU_MAIN)
    {
        getfilename("levels", directory, 0, LOCATION_SYSTEM);
	if(menu_levelentries(pmenu, directory, flags))
	    menuentry_newwithvalue(pmenu, "", 0, MENU_SORT | MENU_SPACE, "");

	menu_unsort(pmenu);

	/* Separator only if needed */
	if(pmenu->entry_last != NULL && !(pmenu->entry_last->flags & MENU_SPACE))
	    menuentry_newwithvalue(pmenu, "", 0, MENU_SPACE, "");

        getfilename("levels", directory, 0, LOCATION_LOCAL);
	if(menu_levelentries(pmenu, directory, flags))
	    menuentry_newwithvalue(pmenu, "", 0, MENU_SORT | MENU_SPACE, "");
    }
    else
    {
	if(menu_levelentries(pmenu, directory_levels, flags))
	    menuentry_newwithvalue(pmenu, "", 0, MENU_SORT | MENU_SPACE, "");
    }

    menu_assignletters(pmenu);

    while(1)
    {
        result = menu_process(pmenu);

        if(result == MENU_QUIT)
        {
            if(chosen != NULL)
                strcpy(chosen, "");
	    break;
        }

        if(result == MENU_SELECT)
        {
            if(pmenu->entry_selected->key == '+' && flags & LEVELMENU_FILTER)
            {
                result = menu_levelselector(directory_levels, chosen, title, flags & ~LEVELMENU_FILTER);
                if(result == MENU_SELECT)
                    break;
            }
            else if(pmenu->entry_selected->value != NULL)
	    {
		if(isfile(pmenu->entry_selected->value))
	        {
		    if(flags & LEVELMENU_GAME)
		    {
			plevel = level_load(pmenu->entry_selected->value, 0);
			if(plevel != NULL)
			    level_play(plevel);
			/* plevel is deleted by level_play */
		    }
		    else
		    {
                        if(chosen != NULL)
	                    strcpy(chosen, pmenu->entry_selected->value);
	                break;
		    }
	        }
                else if(isdirectory(pmenu->entry_selected->value))
	        {
		    flags_child = flags & (LEVELMENU_NEW | LEVELMENU_DELETE | LEVELMENU_FILENAMES | LEVELMENU_MOVES | LEVELMENU_GAME | LEVELMENU_FILTER);
		    flags_child |= LEVELMENU_RETURN;

                    /* Fix flags for "saved", "solved" and "construct" */
		    if(pmenu->entry_selected->key == 'S')
		        flags_child |= LEVELMENU_MOVES | LEVELMENU_DELETE;
		    if(pmenu->entry_selected->key == 'C')
		        flags_child |= LEVELMENU_MOVES | LEVELMENU_DELETE;
		    if(pmenu->entry_selected->key == 'O')
		        flags_child |= LEVELMENU_MOVES | LEVELMENU_DELETE;

                    result = menu_levelselector(pmenu->entry_selected->value, chosen, pmenu->entry_selected->text, flags_child);
		    if(result == MENU_SELECT)
		        break;

		    result = MENU_NULL;
	        }
	    }
	    else
	    {
                if(chosen != NULL)
	            strcpy(chosen, "");

	        if(pmenu->entry_selected->key == 'Q')
		    result = MENU_QUIT;

	        break;
	    }
        }

        if(result == MENU_DELETE)
        {
	    if(pmenu->entry_selected->value != NULL)
	        unlink(pmenu->entry_selected->value);
	    menuentry_delete(pmenu, pmenu->entry_selected);
        }
    }

    menu_delete(pmenu);

    return result;
}

int menu_levelentries(struct menu* pmenu, char *directory, int flags)
{
    DIR *pdir;
    struct dirent *dentry;
    struct menuentry* pentry;
    struct level* plevel;
    struct stat filestat;
    int flags_child;
    int submenus;
    int subfiles;
    int state;
    int hidden;
    int filtered;

    char text_date[256];
    char text_moves[256];
    char text_stars[256];
    char *text_title;
    char *text_subtitle;

    char filename[FILENAME_MAX];
    char titlefilename[FILENAME_MAX];

    struct parser* pparser;

    submenus = 0;
    subfiles = 0;
    filtered = 0;

    pdir = opendir(directory);

    if(pdir == NULL)
	return 0;

    while((dentry = readdir(pdir)) != NULL)
    {
        if(strcmp(dentry->d_name, ".") == 0)
	    continue;
        if(strcmp(dentry->d_name, "..") == 0)
            continue; 

        if(dentry->d_name[0] == '.')
            continue;

        sprintf(filename, "%s%s%s", directory, "/", dentry->d_name);

        if(isfile(filename) && strlen(filename) > 7 && strcmp(filename + strlen(filename) - 7, ".chroma") == 0)
        {
            plevel = level_load(filename, 1);

            if(plevel != NULL)
            { 
	        flags_child = MENU_SORT;
	        if(flags & (LEVELMENU_MOVES | LEVELMENU_FILENAMES))
    	            flags_child |= MENU_DOUBLE;
	        if(flags & LEVELMENU_DELETE)
	            flags_child |= MENU_DELETABLE;

	        if(plevel->title == NULL || strcmp(plevel->title, "") == 0)
	            pentry = menuentry_newwithvalue(pmenu, gettext("[untitled level]"), 0, flags_child, filename);
	        else
                {
                    if(strncmp(plevel->title, "chroma", 6) == 0)
	                pentry = menuentry_newwithvalue(pmenu, gettext(plevel->title), 0, flags_child, filename);
                    else
	                pentry = menuentry_newwithvalue(pmenu, plevel->title, 0, flags_child, filename);
                }

	        if(plevel->stars_exploded)
	            sprintf(text_stars, gettext("%d lost"), plevel->stars_exploded);
	        else
	            sprintf(text_stars, "%d / %d", plevel->stars_caught, plevel->stars_total);
	        if(plevel->flags & LEVELFLAG_SOLVED && !(plevel->flags & LEVELFLAG_FAILED))
		    sprintf(text_stars, gettext("solved"));

	        if(plevel->flags & LEVELFLAG_FAILED)
		    sprintf(text_moves, gettext("failed"));
	        else
	            sprintf(text_moves, gettext("%d moves"), plevel->moves);

                if(!stat(filename, &filestat))
	        {
                    sprintf(text_date, "%s", ctime(&filestat.st_mtime));
	            if(strlen(text_date) > 0)
	                text_date[strlen(text_date) - 1] = 0;
	        }
	        else
		    strcpy(text_date, "");

	        if(flags & LEVELMENU_FILENAMES)
	            menuentry_extratext(pentry, NULL, text_date, dentry->d_name);
	        else if(flags & LEVELMENU_MOVES)
	            menuentry_extratext(pentry, text_stars, text_date, text_moves);
                else
                {
                    /* Not really needed except for development */
                    if(plevel->flags & LEVELFLAG_SOLVED && !(plevel->flags & LEVELFLAG_FAILED))
                        menuentry_extratext(pentry, gettext("solved"), NULL, NULL);
                }

                if((flags & LEVELMENU_FILTER) && plevelcurrent != NULL)
                {
                    if((plevelcurrent->title != NULL && strcmp(plevelcurrent->title, pentry->text) != 0)
                            || (plevelcurrent->title == NULL && plevel->title != NULL))
                    {
                        filtered ++;
                        menuentry_delete(pmenu, pentry);
                        continue;
                    }
                }

#ifdef XOR_COMPATIBILITY
                if(plevel->mode == MODE_XOR)
                    options_xor_options = 1;
#endif
#ifdef ENIGMA_COMPATIBILITY
                if(plevel->mode == MODE_ENIGMA)
                    options_enigma_options = 1;
#endif
                subfiles ++;
            }

            level_delete(plevel);
        }
        else if(isdirectory(filename))
        {
	    flags_child = MENU_TOPSORT | MENU_BOLD;
	    if(flags & (LEVELMENU_MOVES | LEVELMENU_FILENAMES))
    	        flags_child |= MENU_DOUBLE;

            /* Read the title information from set.chroma, if it exists */
	    snprintf(titlefilename, FILENAME_MAX, "%s/%s", filename, "set.chroma");
            text_title = NULL;
            text_subtitle = NULL;

            hidden = 0;

            pparser = parser_new(titlefilename);

            enum {
                LEVELSETPARSER_END,
                LEVELSETPARSER_OUTSIDE,
                LEVELSETPARSER_CHROMA
            };

            state = LEVELSETPARSER_OUTSIDE;

            while(state != LEVELSETPARSER_END)
            {
                switch(parser_parse(pparser))
                {
                    case PARSER_END:
                        state = LEVELSETPARSER_END;
                        break;

                    case PARSER_ELEMENT_START:
                        break;

                    case PARSER_ELEMENT_END:
                        switch(state)
                        {
                            case LEVELSETPARSER_CHROMA:
                                /* We only read the file's header here */
                                if(parser_match(pparser, 0, "head"))
                                    state = LEVELSETPARSER_END;
                                break;
                        }
                        break;

                    case PARSER_CONTENT:
                        switch(state)
                        {   
                            case LEVELSETPARSER_CHROMA:
                                if(parser_match(pparser, 1, "title"))
                                {   
                                    text_title = malloc(strlen(parser_text(pparser, 0)) + 1);
                                    if(text_title == NULL)
                                        fatal(gettext("Out of memory in menu_levelentries()")); 
                                    strcpy(text_title, parser_text(pparser, 0));
                                }
                                if(parser_match(pparser, 1, "subtitle"))
                                {   
                                    text_subtitle = malloc(strlen(parser_text(pparser, 0)) + 1);
                                    if(text_title == NULL)
                                        fatal(gettext("Out of memory in menu_levelentries()")); 
                                    strcpy(text_subtitle, parser_text(pparser, 0));
                                }
                                break;
                        }
                        break;

                    case PARSER_ATTRIBUTE:
                        switch(state)
                        {   
                            case LEVELSETPARSER_OUTSIDE:
                                if(parser_match(pparser, 2, "chroma") && parser_match(pparser, 1, "type"))
                                {   
                                    if(parser_match(pparser, 0, "set"))
                                        state = LEVELSETPARSER_CHROMA;
                                }
                                break;

                            case LEVELSETPARSER_CHROMA:
                                if(parser_match(pparser, 2, "chroma") && parser_match(pparser, 1, "hidden"))
                                {   
                                    if(parser_match(pparser, 0, "yes"))
                                        hidden = 1;
                                }
                                break;
                        }
                        break;

                    case PARSER_ERROR:
                        state = LEVELSETPARSER_END;
                        break;
                }
            }

            parser_delete(pparser);

            if(!hidden || options_debug & DEBUG_HIDDEN)
            {
                if(text_title != NULL)
                {
                    if(strncmp(text_title, "chroma", 6) == 0)
    	                pentry = menuentry_newwithvalue(pmenu, gettext(text_title), 0, flags_child | (text_subtitle != NULL ? MENU_DOUBLE : 0), filename);
                    else
    	                pentry = menuentry_newwithvalue(pmenu, text_title, 0, flags_child | (text_subtitle != NULL ? MENU_DOUBLE : 0), filename);

                }
                else
	            pentry = menuentry_newwithvalue(pmenu, dentry->d_name, 0, flags_child, filename);

	        if(flags & LEVELMENU_FILENAMES)
 	            menuentry_extratext(pentry, NULL, NULL, dentry->d_name);
	        else
                {
                    if(text_title != NULL && strncmp(text_title, "chroma", 6) == 0)
 	                menuentry_extratext(pentry, NULL, NULL, gettext(text_subtitle));
                    else
 	                menuentry_extratext(pentry, NULL, NULL, text_subtitle);
                }
                if(text_title != NULL)
                    free(text_title);
                if(text_subtitle != NULL)
                    free(text_subtitle);

	        submenus ++;
            }
        }
    }

    closedir(pdir);

    if(filtered != 0)
    {
        if(submenus != 0 || subfiles != 0)
            menuentry_new(pmenu, "", 0, MENU_SPACE);
        pentry = menuentry_new(pmenu, gettext("Show positions for other levels"), '+', 0);
        sprintf(text_date, filtered == 1 ? gettext("(%d other)") : gettext("(%d others)"), filtered);
        menuentry_extratext(pentry, text_date, NULL, NULL);
    }

    if(submenus !=0 && subfiles != 0)
        return 1;
    else
        return 0;

}

