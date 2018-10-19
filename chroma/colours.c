/*  
    colours.c

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
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <libintl.h>
#include <locale.h>

#include "chroma.h"
#include "level.h"
#include "util.h"
#include "colours.h"
#include "menu.h"
#include "xmlparser.h"

struct colours* pdisplaycolours = NULL;

extern char options_colours[];
extern int options_debug;
extern char *piece_name[];

void colours_init()
{
    char filename[FILENAME_MAX];
    char directory[FILENAME_MAX];

    if(pdisplaycolours != NULL)
        colours_delete(pdisplaycolours);

    pdisplaycolours = colours_load(options_colours, 0);

    if(pdisplaycolours == NULL)
    {   
        /* Revert to default */
        getfilename("colours", directory, 0, LOCATION_SYSTEM);
        sprintf(filename, "%s%s%s", directory, "/", COLOURS_DEFAULT);
        pdisplaycolours = colours_load(filename, 0);

        /* If we can't even load the default, use an emergency scheme */
        if(pdisplaycolours == NULL)
            pdisplaycolours = colours_load(NULL, 0);
    }
}

struct colours* colours_load(char *filename, int partial)
{
    struct colours* pcolours;
    struct parser* pparser = NULL;
    int state;
    int piece;
    int i;
    int foreground, background, bold, reverse;
    char character;
    char *text;
    int colour;

    if(filename != NULL)
    {
        if(!isfile(filename))
            return NULL;

        pparser = parser_new(filename);
        if(pparser == NULL)
            return NULL;
    }
  
    pcolours = (struct colours*)malloc(sizeof(struct colours));
    pcolours->title = NULL;
    if(pcolours == NULL)
	return NULL;

    /* Sensible defaults */
    if(!partial)
    {
        for(i = 0; i < PIECE_MAX; i ++)
        {
    	    pcolours->character[i] = piecetochar(i);
	    pcolours->foreground[i] = -1;
	    pcolours->background[i] = -1;
	    pcolours->bold[i] = 0;
	    pcolours->reverse[i] = 0;
        }
    }

    /* Emergency default colour scheme */
    if(filename == NULL)
        return pcolours;

    /* Parse XML file */
    /*
       <chroma type="colours">
           <head>
               <title>title</title>
           </head>
           <pieces>
           <piece name="piece" character="c" foreground="colour" background ="colour" bold="yes" reverse = "yes" />
           </pieces>
       </chroma>
    */

    enum{
        COLOURSPARSER_BAD,      /* End of bad file */
        COLOURSPARSER_END,      /* End of good file */
        COLOURSPARSER_OUTSIDE,  /* Outside <chroma> */
        COLOURSPARSER_CHROMA,   /* Inside <chroma> */
        COLOURSPARSER_PIECES    /* Inside <pieces> */
    };

    state = COLOURSPARSER_OUTSIDE;
    piece = PIECE_UNKNOWN;
    foreground = 7; background = 0; bold = 0; reverse = 0; character = ' ';

    while(state != COLOURSPARSER_BAD && state != COLOURSPARSER_END)
    {   
        switch(parser_parse(pparser))
        {   
            case PARSER_END:
                if(state == COLOURSPARSER_OUTSIDE)
                    state = COLOURSPARSER_BAD;
                else
                    state = COLOURSPARSER_END;
                break;

            case PARSER_ELEMENT_START:
                switch(state)
                {   
                    case COLOURSPARSER_CHROMA:
                        if(parser_match(pparser, 0, "pieces"))
                            state = COLOURSPARSER_PIECES; 
                        break;

                    case COLOURSPARSER_PIECES:
                        if(parser_match(pparser, 0, "piece"))
                        {   
                            piece = PIECE_UNKNOWN;
                            foreground = 7; background = -1;
                            bold = 0; reverse = 0;
                            character = ' ';
                        }
                        break;

                    default:
                        break;
                }
                break;

           case PARSER_ELEMENT_END:
                switch(state)
                {   
                    case COLOURSPARSER_CHROMA:                                                          /* If we're only partially loading the file, end parsing at </head> */
                        if(parser_match(pparser, 0, "head"))
                        {   
                            if(partial)
                                state = COLOURSPARSER_END;
                        }
                        break;

                    case COLOURSPARSER_PIECES:
                        if(parser_match(pparser, 0, "pieces"))
                            state = COLOURSPARSER_CHROMA;
                        if(parser_match(pparser, 0, "piece"))
                        {
                            if(piece != PIECE_UNKNOWN)
                            {
                                pcolours->character[piece] = character;
                                pcolours->foreground[piece] = foreground;
                                pcolours->background[piece] = background;
                                pcolours->bold[piece] = bold;
                                pcolours->reverse[piece] = reverse;
                            }
                        }
                        break;

                    default:
                        break;
                }

                break;

            case PARSER_CONTENT:
                switch(state)
                {   
                    case COLOURSPARSER_CHROMA:
                        if(parser_match(pparser, 1, "title"))
                        {   
                            pcolours->title = malloc(strlen(parser_text(pparser, 0)) + 1);
                            if(pcolours->title == NULL)
                                fatal(gettext("Out of memory in colours_load()"));
                            strcpy(pcolours->title, parser_text(pparser, 0));
                        }

                        break;

                    default:
                        break;
                }
                break;

            case PARSER_ATTRIBUTE:
                switch(state)
                {   
                    case COLOURSPARSER_OUTSIDE:                                                         if(parser_match(pparser, 2, "chroma") && parser_match(pparser, 1, "type"))
                        {
                            if(parser_match(pparser, 0, "colours") || parser_match(pparser, 0, "colors"))
                                state = COLOURSPARSER_CHROMA;
                        }
                        break;

                    case COLOURSPARSER_CHROMA:
                        if(parser_match(pparser, 2, "chroma") && parser_match(pparser, 1, "hidden"))
                        {   
                            if(parser_match(pparser, 0, "yes"))
                            {   
                                if(partial && !(options_debug & DEBUG_HIDDEN))
                                    state = COLOURSPARSER_BAD;
                            }
                        }
                        if(parser_match(pparser, 2, "title") && parser_match(pparser, 1, "translate")) 
                        {  
                            if(parser_match(pparser, 0, "yes"))
                                pcolours->flags |= COLOURS_TRANSLATE;
                        }
                        break;

                    case COLOURSPARSER_PIECES:
                        if(parser_match(pparser, 2, "piece") && parser_match(pparser, 1, "name"))
                        {   
                            for(i = 0; i < PIECE_UNKNOWN; i ++)
                            {   
                                if(strcasecmp(parser_text(pparser, 0), piece_name[i]) == 0)
                                    piece = i;
                            }
                        }
                        if(parser_match(pparser, 2, "piece") && parser_match(pparser, 1, "character"))
                        {
                            text = parser_text(pparser, 0);
                            if(text[0] != 0)
                                character = text[0];
                        }
                        if(parser_match(pparser, 2, "piece") && parser_match(pparser, 1, "bold"))
                        {
                            bold = 0;
                            if(strcmp(parser_text(pparser, 0), "yes") == 0)
                                bold = 1;
                        }
                        if(parser_match(pparser, 2, "piece") && parser_match(pparser, 1, "reverse"))
                        {
                            reverse = 0;
                            if(strcmp(parser_text(pparser, 0), "yes") == 0)
                                reverse = 1;
                        }
                        if(parser_match(pparser, 2, "piece") && (parser_match(pparser, 1, "background") || parser_match(pparser, 1, "foreground")))
                        {
                            colour = -1;
                            if(strcmp(parser_text(pparser, 0), "black") == 0)
                                colour = 0;
                            if(strcmp(parser_text(pparser, 0), "red") == 0)
                                colour = 1;
                            if(strcmp(parser_text(pparser, 0), "green") == 0)
                                colour = 2;
                            if(strcmp(parser_text(pparser, 0), "yellow") == 0)
                                colour = 3;
                            if(strcmp(parser_text(pparser, 0), "blue") == 0)
                                colour = 4;
                            if(strcmp(parser_text(pparser, 0), "magenta") == 0)
                                colour = 5;
                            if(strcmp(parser_text(pparser, 0), "cyan") == 0)
                                colour = 6;
                            if(strcmp(parser_text(pparser, 0), "white") == 0)
                                colour = 7;

                            if(parser_match(pparser, 1, "background"))
                                background = colour;
                            if(parser_match(pparser, 1, "foreground") && colour != -1)
                                foreground = colour;
                        }
                        break;

                    default:
                        break;
                }
                break;

            case PARSER_ERROR:
            default:
                break;
        }
    }

    parser_delete(pparser);

    if(state == COLOURSPARSER_BAD)
    {   
        colours_delete(pcolours);
        return NULL;
    }

    return pcolours;
}

void colours_delete(struct colours* pcolours)
{
    if(pcolours->title != NULL)
        free(pcolours->title);

    free(pcolours);
}

struct menu* colours_menu()
{
    DIR *pdir;
    struct dirent *dentry;
    struct menu* pmenu;
    struct menuentry* pentry;
    char directory[FILENAME_MAX];
    char filename[FILENAME_MAX];
    struct colours* pcolours;
    int location;

    pmenu = menu_new(gettext("Colour Schemes"));

    menuentry_new(pmenu, gettext("Quit and return to previous menu"), 'Q', 0);
    menuentry_new(pmenu, "", 0, MENU_SPACE);

    menuentry_new(pmenu, gettext("Current colour scheme:"), 0, MENU_NOTE);

    if(pdisplaycolours == NULL)
        menuentry_new(pmenu, gettext("** NONE **"), 0, MENU_NOTE | MENU_RIGHT);
    else if(pdisplaycolours->title == NULL)
        menuentry_new(pmenu, gettext("[untitled colours]"), 0, MENU_NOTE | MENU_RIGHT);
    else if(pdisplaycolours->flags & COLOURS_TRANSLATE)
        menuentry_new(pmenu, gettext(pdisplaycolours->title), 0, MENU_NOTE | MENU_RIGHT);
    else
        menuentry_new(pmenu, pdisplaycolours->title, 0, MENU_NOTE | MENU_RIGHT);  
    /* Global, then user */
    for(location = 1; location >= 0; location --)
    {
        getfilename("colours", directory, 0, location);

        pdir = opendir(directory);
        if(pdir == NULL)
    	    continue;

        while((dentry = readdir(pdir)) != NULL)
        {   
            if(strcmp(dentry->d_name, ".") == 0)
                continue;
            if(strcmp(dentry->d_name, "..") == 0)
                continue;

            sprintf(filename, "%s%s%s", directory, "/", dentry->d_name);

	    if(isfile(filename) && strlen(filename) > 7 && strcmp(filename + strlen(filename) - 7, ".chroma") == 0)
	    {
	        pcolours = colours_load(filename, 1);
	        if(pcolours != NULL)
	        {
                    if(pcolours->title == NULL)
                        pentry = menuentry_newwithvalue(pmenu, gettext("[untitled colours]"), 0, MENU_SORT, filename);
                    else if(pcolours->flags & COLOURS_TRANSLATE)
                        pentry = menuentry_newwithvalue(pmenu, gettext(pcolours->title), 0, MENU_SORT, filename);
                    else
                        pentry = menuentry_newwithvalue(pmenu, pcolours->title, 0, MENU_SORT, filename);
		    colours_delete(pcolours);

		    if(strcmp(options_colours, filename) == 0)
		        pmenu->entry_selected = pentry;
	        }
	    }
        }

        closedir(pdir);

        menu_unsort(pmenu);

        if(location == 1)
	    menuentry_new(pmenu, "", 0, MENU_SPACE);
    }

    menu_assignletters(pmenu);

    return pmenu;
}
