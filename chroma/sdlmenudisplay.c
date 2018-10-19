/*  
    sdlmenudisplay.c

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

#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "chroma.h"
#include "menu.h"
#include "sdlfont.h"
#include "sdlscreen.h"
#include "actions.h"

extern void flip_it(void);
extern int screen_width;

extern int display_size_x;
extern int display_size_y;

extern struct SDL_Surface* psurfacelogo;
extern struct SDL_Surface* psurfacelogosmall;

extern SDL_Surface *screen_surface;
extern int screen_width;
extern int screen_height;

extern int font_height;
extern int font_width;
extern int font_border;
extern int font_padding;
extern int font_size_menu;

extern int actions[SDLK_LAST];
extern int actions_mouse[3][MOUSE_BUTTONS_MAX];

int menu_offset;
int menu_width;
int menu_height_notes;
int menu_height_entries;
int menu_y_logo_top;
int menu_y_logo_bottom;
int menu_x_min;
int menu_x_max;
int menu_y_min;
int menu_y_max;
int menu_scroll_x_min;
int menu_scroll_x_max;
int menu_scroll_y_min;
int menu_scroll_y_max;
int menu_scroll_top;
int menu_scroll_bottom;

int menu_entryheight(struct menuentry *pentry)
{
    if(pentry == NULL)
        return 0;
    if(pentry->flags & MENU_INVISIBLE)
        return 0;
    if(pentry->flags & MENU_DOUBLE)
        return font_height * 2 + font_padding + font_border * 2;
    return font_height + font_padding + font_border * 2;
}

void menu_displayentry(struct menu *pmenu, struct menuentry *pentry, int offset, int selected)
{
    int colour;
    SDL_Surface *psurface;
    SDL_Rect srect, drect;
    char buffer[256];
    int x, y, w, h;

    if(pentry->flags & MENU_INVISIBLE)
        return;

    h = font_height * ((pentry->flags & MENU_DOUBLE) ? 2 : 1);

    if((pentry->flags & MENU_SPACE) == MENU_SPACE)
        return;

    colour = COLOUR_BLUE;
    if(selected)
        colour = COLOUR_CYAN;
    if(pentry->flags & MENU_GREY)
        colour = COLOUR_BLACK;
    if(pentry->flags & MENU_NOTE)
        colour = COLOUR_BLACK;
    if(pentry->flags & MENU_TEXT)
        colour = COLOUR_BLACK;

    /* Key box */
    if(pentry->key != 0 && pentry->key != MENU_KEY_ANY)
    {
        x = menu_offset;
        y = offset;

        font_box(x, y, font_height + font_border * 2, font_height + font_border * 2, colour | COLOUR_LIGHT);
        sprintf(buffer, "%c", pentry->key);
        psurface = font_render(buffer, colour);
        drect.x = x + font_border + ((font_height - psurface->w) / 2);
        drect.y = y + font_border;
        SDL_BlitSurface(psurface, NULL, screen_surface, &drect);
        SDL_FreeSurface(psurface);
    }

    /* Text box */
    x = menu_offset + font_height + font_padding + font_border * 2;
    y = offset;

    if(!((pentry->flags & MENU_NOTE) || ((pentry->flags & (MENU_GREY | MENU_TEXT)) == MENU_TEXT)))
    {
        if(pentry->key != MENU_KEY_ANY)
            font_box(x, y, menu_width, h + font_border * 2, colour | COLOUR_LIGHT);
        else
            font_box(menu_offset, y, menu_width + font_height + font_padding + font_border * 2, h + font_border * 2, colour | COLOUR_LIGHT);
    }

    if(pentry->text != NULL)
    {
        w = menu_width - font_border * 2 - font_height / 4;
        psurface = font_render(pentry->text, colour | ((pentry->flags & MENU_BOLD) ? COLOUR_BOLD : 0));
        srect.x = 0;
        srect.y = 0;
        srect.h = psurface->h;
        srect.w = psurface->w < w ? psurface->w : w;
        drect.x = x + font_border + font_height / 4;
        drect.y = y + font_border;

        if(pentry->flags & MENU_RIGHT)
            drect.x = x + menu_width - font_border - font_height / 4 - srect.w;
        if(pentry->flags & MENU_CENTRE)
            drect.x = x + font_border + (menu_width - srect.w) / 2;

        SDL_BlitSurface(psurface, &srect, screen_surface, &drect);
        SDL_FreeSurface(psurface);
    }

    if(pentry->text2 != NULL)
    {
        w = menu_width - font_border * 2 - font_height / 4;
        psurface = font_render(pentry->text2, colour);
        srect.x = 0;
        srect.y = 0;
        srect.h = psurface->h;
        srect.w = psurface->w < w ? psurface->w : w;
        drect.x = x + menu_width - font_border - font_height / 4 - srect.w;
        drect.y = y + font_border;
        SDL_BlitSurface(psurface, &srect, screen_surface, &drect);
        SDL_FreeSurface(psurface);
    }

    if(!(pentry->flags & MENU_DOUBLE))
        return;

    if(pentry->text3 != NULL)
    {
        w = menu_width - font_border * 2 - font_height / 4;
        psurface = font_render(pentry->text3, colour);
        srect.x = 0;
        srect.y = 0;
        srect.h = psurface->h;
        srect.w = psurface->w < w ? psurface->w : w;
        drect.x = x + font_border + font_height / 4;
        drect.y = y + font_border + font_height;
        SDL_BlitSurface(psurface, &srect, screen_surface, &drect);
        SDL_FreeSurface(psurface);
    }

    if(pentry->text4 != NULL)
    {
        w = menu_width - font_border * 2 - font_height / 4;

        psurface = font_render(pentry->text4, colour | ((pentry->flags & MENU_EDITING) ? COLOUR_LIGHT : 0));
        srect.x = 0;
        srect.y = 0;
        srect.h = psurface->h;
        srect.w = psurface->w;
        if(psurface->w > w)
        {
            srect.w = w;
            if(pentry->flags & MENU_EDITING)
                srect.x = psurface->w - srect.w;
        }
        drect.x = x + menu_width - font_border - font_height / 4 - srect.w;
        drect.y = y + font_border + font_height;

        if(pentry->flags & MENU_EDITING)
        {
            w = srect.w + font_height / 2 + font_border * 2;
            if(w > menu_width)
                w = menu_width;

            font_box(x + menu_width - w, y + font_border + font_height, w, font_height + font_border, colour);
        }

        SDL_BlitSurface(psurface, &srect, screen_surface, &drect);
        SDL_FreeSurface(psurface);
    }
}

void menu_display(struct menu *pmenu, int redraw)
{
    struct menuentry *pentry;
    SDL_Rect drect;
    SDL_Surface *psurface;
    int x, y, w, selected;
    int state;

    /* Calculate various widths */
    menu_offset = font_height + font_border * 2;
    menu_width = screen_width - menu_offset * 2 - font_height - font_border * 2 - font_padding;

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
        menu_height_notes += font_height;

    if(pmenu->logo)
    {
        menu_y_logo_top = (screen_height - menu_height_entries - font_height - menu_height_notes - psurfacelogo->h) / 2;
        if(menu_y_logo_top < 0)
            menu_y_logo_top = 0;
        menu_y_logo_bottom = menu_y_logo_top + psurfacelogo->h;
        menu_y_min = screen_height - menu_height_entries - font_height;
        if(menu_y_min < menu_y_logo_bottom)
            menu_y_min = menu_y_logo_bottom;
        menu_y_max = screen_height - font_height;
    }
    else
    {
        menu_y_min = psurfacelogosmall->h * 1.5;
        menu_y_max = screen_height - menu_height_notes;
        if(pmenu->title != NULL)
            menu_y_min += font_height * 1.5;
    }

    /* If a full redraw, clear the screen and plot the title */
    if(redraw == MENUREDRAW_ALL)
    {  
        SDL_FillRect(screen_surface, NULL, SDL_MapRGB(screen_surface->format, 255, 255, 255));

        if(pmenu->logo)
        {
            /* Display large logo */
            drect.x = (screen_width - psurfacelogo->w) / 2;
            drect.y = menu_y_logo_top;
            SDL_BlitSurface(psurfacelogo, NULL, screen_surface, &drect);
        }
        else
        {
            /* Display small logo*/
            drect.x = (screen_width - psurfacelogosmall->w) / 2;
            drect.y = psurfacelogosmall->h / 4;
            SDL_BlitSurface(psurfacelogosmall, NULL, screen_surface, &drect);

            /* Display menu title */
            if(pmenu->title != NULL)
            {
                psurface = font_render(pmenu->title, COLOUR_BLACK);
                drect.x = (screen_width - psurface->w) / 2;
                drect.y = psurfacelogosmall->h * 1.5;
                SDL_BlitSurface(psurface, NULL, screen_surface, &drect);
                SDL_FreeSurface(psurface);
            }
        }
    }

    /* Keep scroll bar within reasonable limits */
    if(pmenu->offset > (menu_height_entries - (menu_y_max - menu_y_min)))
        pmenu->offset = menu_height_entries - (menu_y_max - menu_y_min);
    if(pmenu->offset < 0)
        pmenu->offset = 0;
    
    /* Display notes */
    if((menu_height_notes != 0) && redraw >= MENUREDRAW_ENTRIES)
    {
        if(pmenu->logo)
            y = menu_y_logo_bottom;
        else
            y = menu_y_max + font_height / 2;;

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

    /* Clip to just the entries area */
    drect.x = 0;
    drect.y = menu_y_min;
    drect.w = screen_width;
    drect.h = menu_y_max - menu_y_min;
    SDL_SetClipRect(screen_surface, &drect);

    /* Wipe entries area if we have not previously done so */
    if(redraw == MENUREDRAW_ENTRIES)
        SDL_FillRect(screen_surface, &drect, SDL_MapRGB(screen_surface->format, 255, 255, 255));

    /* Display scroll bar, if needed */
    if(menu_height_entries > (menu_y_max - menu_y_min) || pmenu->offset != 0)
    {
        if(redraw >= MENUREDRAW_ENTRIES)
        {
            x = menu_offset + menu_width;
            x += font_height + font_padding + font_border * 3;

            /* Up arrow */
            psurface = font_render(ARROW_UP, COLOUR_BLACK);
            w = psurface->w;
            x += (font_height - w) / 2;
            drect.x = x;
            drect.y = menu_y_min - (font_height * 5 / 32);
            SDL_BlitSurface(psurface, NULL, screen_surface, &drect);
            SDL_FreeSurface(psurface);

            /* Down arrow */
            psurface = font_render(ARROW_DOWN, COLOUR_BLACK);
            drect.x = x;
            drect.y = menu_y_max - font_height;
            SDL_BlitSurface(psurface, NULL, screen_surface, &drect);
            SDL_FreeSurface(psurface);
    
            menu_scroll_x_min = x;
            menu_scroll_x_max = x + w;

            /* Outer box */
            /* (the positions are slightly adjusted as the arrows aren't centered) */
            menu_scroll_y_min = menu_y_min + font_height - (font_height * 5 / 32);
            menu_scroll_y_max = menu_y_max - font_height + (font_height * 5 / 32);
            if(menu_scroll_y_max > menu_scroll_y_min)
                font_box(x, menu_scroll_y_min, w, menu_scroll_y_max - menu_scroll_y_min, COLOUR_BLACK | COLOUR_LIGHT);

            /* Inner box */
            menu_scroll_top = menu_scroll_y_min + ((menu_scroll_y_max - menu_scroll_y_min) * pmenu->offset / menu_height_entries);
            menu_scroll_bottom = menu_scroll_top + ((menu_scroll_y_max - menu_scroll_y_min) * (1 + menu_y_max - menu_y_min) / menu_height_entries);
            if(menu_scroll_top < menu_scroll_y_min)
                menu_scroll_top = menu_scroll_y_min;
            if(menu_scroll_bottom > menu_scroll_y_max)
                menu_scroll_bottom = menu_scroll_y_max;
            if(menu_scroll_bottom > menu_scroll_top)
                font_box(x, menu_scroll_top, w, menu_scroll_bottom - menu_scroll_top, COLOUR_BLACK);
        }
    }
    else
    {
        menu_scroll_x_min = 0;
        menu_scroll_x_max = 0;
        menu_scroll_y_min = 0;
        menu_scroll_y_max = 0;
    }

    /* Calculate widths */
    menu_x_min = menu_offset;
    menu_x_max = menu_offset + menu_width + font_height + font_padding + font_border * 2;

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
                (redraw >= MENUREDRAW_CHANGED && pentry->redraw&& !(pentry->flags & MENU_TEXT)))
            menu_displayentry(pmenu, pentry, y, selected);

        pentry->redraw = 0;
        y += menu_entryheight(pentry);
        pentry = pentry->next;
    }

    /* Release clip */
    SDL_SetClipRect(screen_surface, NULL);

    if(redraw >= MENUREDRAW_ALL){
        //SDL_UpdateRect(screen_surface, 0, 0, 0, 0);
		}
    else{
        //SDL_UpdateRect(screen_surface, 0,  menu_y_min, screen_width, menu_y_max - menu_y_min);
		}
		flip_it();
}

int menu_process(struct menu *pmenu)
{
    SDL_Event event;
    struct menuentry *pentry;
    int redraw;
    int ok;
    char c;
    char *buffer;
    int y, h;
    int mx, my;
    int scroll_speed;
    int scroll_start;
    int first = 1;
    int action;
    int i;
    Uint16 uc;

    font_set_size(font_size_menu);

    redraw = pmenu->redraw; 

    ok = 0;
    scroll_speed = 0;
    scroll_start = 0;

    while(!ok)
    {
        if(redraw != MENUREDRAW_NONE)
        {
            menu_display(pmenu, redraw);
            redraw = MENUREDRAW_NONE;
        }

        if(scroll_speed || scroll_start)
            SDL_PollEvent(&event);
        else
            SDL_WaitEvent(&event);

        /* If an arrow button is pressed, scroll the scroll bar */
        if(scroll_speed)
        {
            pmenu->offset += scroll_speed;
            redraw = MENUREDRAW_ENTRIES;
        }

        action = ACTION_NONE;
        switch(event.type)
        {
            case SDL_KEYDOWN:
                /* Are we editing a text field? */
                if(pmenu->entry_selected != NULL && pmenu->entry_selected->flags & MENU_EDITING)
                {
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_RETURN:
                        case SDLK_ESCAPE:
                        case SDLK_UP:
                        case SDLK_DOWN:
                        case SDLK_PAGEUP:
                        case SDLK_PAGEDOWN:
                            pmenu->entry_selected->flags -= MENU_EDITING;
                            pmenu->entry_selected->redraw = 1;
                            redraw = MENUREDRAW_CHANGED;
                            break;

                        case SDLK_DELETE:
                        case SDLK_BACKSPACE:
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
			    if(event.key.keysym.unicode > 31 && event.key.keysym.unicode != 127)
			    {
			        uc = event.key.keysym.unicode;

                                buffer = malloc(strlen(pmenu->entry_selected->text4) + 4);
                                if(buffer != NULL)
                                {   
                                    strcpy(buffer, pmenu->entry_selected->text4);
				    i = strlen(buffer);

				    if(uc < 0x80)
				    {
                                        buffer[i] = uc;
                                        buffer[i + 1] = 0;
				    }
                                    else if(uc < 0x800)
                                    {
                                        buffer[i] = 0xc0 | (uc >> 6);
                                        buffer[i + 1] = 0x80 | (uc & 0x3f);
                                        buffer[i + 2] = 0;
				    }
                                    else if(uc < 0x10000)
                                    {
                                        buffer[i] = 0xe0 | (uc >> 12);
                                        buffer[i + 1] = 0x80 | ((uc >> 6) & 0x3f);
                                        buffer[i + 2] = 0x80 | (uc & 0x3f);
                                        buffer[i + 3] = 0;
				    }


                                }
                                menuentry_extratext(pmenu->entry_selected, NULL, NULL, buffer);
                                free(buffer);

                                pmenu->entry_selected->redraw = 1;
                                redraw = MENUREDRAW_CHANGED;
                                break;
			    }
			    break;
                    }
                    break;
                }

                /* Not a text field, but a regular entry */

                /* First, check if the key corresponds to an entry */
                c = toupper(event.key.keysym.unicode);
                pentry = pmenu->entry_first;
                while(pentry != NULL)
                {
                    if(pentry->key == c && c != 0)
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
                    break;

                /* Otherwise, see if it corresponds to an action */
                action = actions[event.key.keysym.sym];
                if(action == ACTION_UP ||
                        action == ACTION_DOWN ||
                        action == ACTION_PAGE_UP ||
                        action == ACTION_PAGE_DOWN ||
                        action == ACTION_HIDE ||
			action == ACTION_ENTER)
                    break;

                /* Finally, see if there is an "any key" entry. */
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

            case SDL_QUIT:
                exit(0);

            case SDL_VIDEORESIZE:
                screen_resizeevent(&event);
                redraw = MENUREDRAW_ALL;
                ok = MENU_RESIZE;
                break;

            case SDL_ACTIVEEVENT:
                if((event.active.state & SDL_APPACTIVE) && event.active.gain == 1)
                    redraw = MENUREDRAW_ALL;
                break;

            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
                if(event.type == SDL_MOUSEMOTION)
                {
                    /* Ignoring the initial mouse position avoids an
                       unfortunate case where the mouse is over "Quit" on the
                       title screen, but the player presses SPACE, believing
                       it to be "any key" to play :( */
                    if(first)
                    {
                        first = 0;
                        break;
                    }
                    
                    mx = event.motion.x;
                    my = event.motion.y;
                }
                else
                {
                    mx = event.button.x;
                    my = event.button.y;

                    if(event.button.button < 1 || event.button.button > MOUSE_BUTTONS_MAX)
                        break;

                    action = ACTION_MAX;

                    switch(actions_mouse[ACTIONS_MENU][event.button.button])
                    {
                        case ACTION_NONE:
                            action = ACTION_NONE;
                            break;

                        case ACTION_QUIT:
                            action = ACTION_QUIT;
                            break;

                        case ACTION_UP:
                            action = ACTION_UP;
                            break;
                            
                        case ACTION_DOWN:
                            action = ACTION_DOWN;
                            break;

                        case ACTION_PAGE_UP:
                            action = ACTION_PAGE_UP;
                            break;

                        case ACTION_PAGE_DOWN:
                            action = ACTION_PAGE_DOWN;
                            break;

                        case ACTION_HIDE:
                            action = ACTION_HIDE;
                            break;
                    }

                    if(action != ACTION_MAX)
                        break;
                    else
                        action = ACTION_NONE;
                }

                /* Are we already moving the scroll bar? */
                if(scroll_start != 0)
                {
                    /* Only move it in quanta for improved responsiveness */
                    if(abs(scroll_start - my) > (font_height / 4))
                    {
                        pmenu->offset += (my - scroll_start) * menu_height_entries / (menu_scroll_y_max - menu_scroll_y_min);
                        redraw = MENUREDRAW_ENTRIES;
                        scroll_start = my;
                    }
                    break;
                }

                /* Are we in one of the entries? */
                if(mx >= menu_x_min && mx < menu_x_max && my >= menu_y_min && my < menu_y_max)
                {
                    pentry = pmenu->entry_first;
                    y = menu_y_min - pmenu->offset;
                    while(pentry != NULL)
                    {
                        if(pentry->flags & MENU_NOTE)
                        {
                            pentry = pentry->next;
                            continue;
                        }

                        h = menu_entryheight(pentry);

                        if(my >= y && my < (y + h))
                        {
                            if(!(pentry->flags & (MENU_GREY | MENU_SPACE)))
                            {
                                if(pmenu->entry_selected != pentry)
                                {
                                    if(pmenu->entry_selected != NULL)
                                    {
                                        if(pmenu->entry_selected->flags & MENU_EDITING)
                                            pmenu->entry_selected->flags -= MENU_EDITING;
                                        
                                        pmenu->entry_selected->redraw = 1;
                                    }
                                    pmenu->entry_selected = pentry;
                                    pentry->redraw = 1;
                                    redraw = MENUREDRAW_CHANGED;
                                }

                                if(event.type == SDL_MOUSEBUTTONDOWN)
                                {
                                    if(pmenu->entry_selected->flags & MENU_EDITABLE)
                                    {
                                        if(pmenu->entry_selected->flags & MENU_EDITING)
                                            pmenu->entry_selected->flags -= MENU_EDITING;
                                        else
                                            pmenu->entry_selected->flags |= MENU_EDITING;
                                        pmenu->entry_selected->redraw = 1;
                                        redraw = MENUREDRAW_CHANGED;
                                    }
                                    else
                                        ok = MENU_SELECT;
                                }
                            }
                        }
    
                        y += h;
                        pentry = pentry->next;
                    }
                }
                /* Are we in the scroll bar? */
                if(mx >= menu_scroll_x_min && mx < menu_scroll_x_max)
                {
                    /* In the bar itself */
                    if(my >= menu_scroll_y_min && my < menu_scroll_y_max)
                    {
                        /* Above the inner bar */
                        if(my < menu_scroll_top)
                        {
                            if(event.type == SDL_MOUSEBUTTONDOWN)
                            {
                                pmenu->offset -= (menu_y_max - menu_y_min);
                                redraw = MENUREDRAW_ENTRIES;
                            }
                        }
                        /* Below the inner bar */
                        else if(my >= menu_scroll_bottom)
                        {
                            if(event.type == SDL_MOUSEBUTTONDOWN)
                            {
                                pmenu->offset += (menu_y_max - menu_y_min);
                                redraw = MENUREDRAW_ENTRIES;
                            }
                        }
                        /* In the inner bar */
                        else
                        {
                            if(event.type == SDL_MOUSEBUTTONDOWN)
                                scroll_start = my;
                        }
                    }
                    /* In the up arrow */
                    else if(my >= (menu_scroll_y_min - font_height) && my < menu_scroll_y_min)
                    {
                        if(event.type == SDL_MOUSEBUTTONDOWN)
                            scroll_speed = -font_height;
                    }
                    /* In the down arrow */
                    else if(my >= menu_scroll_y_max && my < (menu_scroll_y_max + font_height))
                    {
                        if(event.type == SDL_MOUSEBUTTONDOWN)
                            scroll_speed = font_height;
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                scroll_speed = 0;
                scroll_start = 0;
                break;

            default:
                break;
        }

        if(action != ACTION_NONE)
        {
                switch(action)
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
                        pmenu->offset -= (menu_y_max - menu_y_min - menu_entryheight(pmenu->display_first));
                        redraw = MENUREDRAW_ENTRIES;
                        break;

                    case ACTION_PAGE_DOWN:
                        pmenu->offset += (menu_y_max - menu_y_min - menu_entryheight(pmenu->display_last));
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
                        redraw = MENUREDRAW_ALL;
                        break;

                    case ACTION_HIDE:
                        SDL_WM_IconifyWindow();
                        break;

                    default:
                        break;
                }

                action = ACTION_NONE;
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
    int width;
    int bw, ww, sw;

    font_set_size(font_size_menu);

    width = screen_width * 0.85;

    file = fopen(filename, "r");
    if(file == NULL)
	return 0;

    /* font_calculate_width is slow; we try to avoid calling it too often! */
    sw = font_calculate_width(" ", 0) / 2;

    ok = 0;
    strcpy(buffer, ""); bw = 0;
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

	ww = font_calculate_width(word, 0);
	if(strncmp(word, "===", 3) == 0)
	    ww = 10000;

	if((bw + ww < width) && !(strcmp(word, "") == 0 && (c == 10 || c == 13)))
	{
	    if(buffer[0] != 0 && word[0] != 0)
	        strcat(buffer, " ");
	    strcat(buffer, word);
            bw += ww + sw;
	}
	else
	{
	    menuentry_new(pmenu, buffer, 0, MENU_TEXT);
	    if(strcmp(word, "") == 0 && (c == 10 || c ==13))
		menuentry_new(pmenu, "", 0, MENU_TEXT);
	    if(strncmp(word, "===", 3) == 0)
	    {
		strcpy(buffer, ""); bw = 0;
		pmenu->entry_last->flags |= MENU_GREY;
	    }
	    else
	    {
	        strcpy(buffer, word); bw = ww;
	    }
	}

	if(feof(file))
	    ok = 1;
    }

    menuentry_new(pmenu, buffer, 0, MENU_TEXT);

    fclose(file);

    return 1;
}
