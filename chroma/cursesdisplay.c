/*  
    cursesdisplay.c

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
#include <string.h>
#include <libintl.h>
#include <locale.h>

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
#include "level.h"
#include "display.h"
#include "colours.h"
#include "actions.h"
#include "util.h"
#include "xmlparser.h"

char options_colours[FILENAME_MAX] = COLOURS_DEFAULT;
int options_curses_delay = 1;
int options_curses_replay_delay = 1;
int options_debug = 0;
#ifdef XOR_COMPATIBILITY
int options_xor_options = 0;
int options_xor_mode = 1;
int options_xor_display = 0;
#endif
#ifdef ENIGMA_COMPATIBILITY
int options_enigma_options = 0;
int options_enigma_mode = 1;
#endif


extern struct colours* pdisplaycolours;
extern int *editor_piece_maps[];
extern char *action_name[];
extern char *action_shortname[];

/* Translation table for colours.
   This is necessary as some versions of curses interchange red and blue.
 */
short colourtrans[] = {COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW, COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE};

int dp_attr[256], dp_col[256];
char dp_char[256];

int actions[KEY_MAX];

int display_size_x, display_size_y;
int display_offset_x, display_offset_y;
int display_focus_x, display_focus_y;
int display_start_x, display_start_y;
int display_end_x, display_end_y;
int display_border = 7;

void display_piece(struct level* plevel, int piece);

int display_colourpairs = 0;
short display_cpfore[64];
short display_cpback[64];

short colourpair_red;
short colourpair_green;
short colourpair_yellow;
short colourpair_blue;
short colourpair_cyan;
short colourpair_magenta;
short colourpair_cyan;
short colourpair_white;
short colourpair_menu;
short colourpair_menugrey;

char *display_keyname(int i);
void display_addkeytomenu(struct menu* pmenu, int action, char *text);
void display_keys();
void display_debug();
void display_initcolours();

void display_options_othergames();

short display_newcolourpair(short foreground, short background)
{
    short i;

    for(i = 1; i <= display_colourpairs; i ++)
    {
	if(foreground == display_cpfore[i] && background == display_cpback[i])
	    return i;
    }
    display_colourpairs ++;
    display_cpfore[display_colourpairs] = foreground;
    display_cpback[display_colourpairs] = background;

    init_pair(display_colourpairs, foreground, background);

    return display_colourpairs;
}

void display_init()
{
    setlocale(LC_CTYPE, "");
    atexit(display_quit);
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();

    getmaxyx(stdscr, display_size_y, display_size_x);

    colourpair_red = display_newcolourpair(COLOR_RED, COLOR_BLACK);
    colourpair_green = display_newcolourpair(COLOR_GREEN, COLOR_BLACK);
    colourpair_yellow = display_newcolourpair(COLOR_YELLOW, COLOR_BLACK);
    colourpair_blue = display_newcolourpair(COLOR_BLUE, COLOR_BLACK);
    colourpair_magenta = display_newcolourpair(COLOR_MAGENTA, COLOR_BLACK);
    colourpair_cyan = display_newcolourpair(COLOR_CYAN, COLOR_BLACK);
    colourpair_white = display_newcolourpair(COLOR_WHITE, COLOR_BLACK);
    colourpair_menu = display_newcolourpair(COLOR_CYAN, COLOR_BLUE);
    colourpair_menugrey = display_newcolourpair(COLOR_CYAN, COLOR_BLACK);

    display_options_load();
    colours_init();
    display_initcolours();
}

void display_initcolours()
{
  int i;
  short fg, bg;

#ifdef PDCURSES
  short tg;
#endif

  for(i = 0; i < PIECE_MAX; i ++)
  {
      fg = pdisplaycolours->foreground[i];
      if(fg < 0 || fg > 7)
	  fg = 7;

      bg = pdisplaycolours->background[i];
      if(bg < 0 || bg > 7)
	  bg = 0;

#ifdef PDCURSES
      /* PDCurses doesn't handle reverse colours well; we swap them manually */
      if(pdisplaycolours->reverse[i])
      {
	  tg = fg; fg = bg; bg = tg;
      }
#endif

      dp_attr[i] = COLOR_PAIR(display_newcolourpair(colourtrans[fg], colourtrans[bg]));

      if(pdisplaycolours->bold[i])
	  dp_attr[i] |= A_BOLD;

#ifndef PDCURSES
      if(pdisplaycolours->reverse[i])
	  dp_attr[i] |= A_REVERSE;
#endif

  }

}

void display_quit()
{
    clear();
    refresh();
    endwin();
}

void display_hide()
{
    clear();
    refresh();
    getch();
}

void display_piece(struct level* plevel, int piece)
{
    int p;

    if(piece < 0 || piece >= PIECE_MAX)
	return;

    /* Use the colours of PIECE_PLAYER_ONE for the active player,
       and the colours of PIECE_PLAYER_TWO for the inactive one. */
    p = piece;
    if(p == PIECE_PLAYER_ONE || p == PIECE_PLAYER_TWO)
    {
	if(plevel->player != 2)
	{
	    if(plevel->player != (p & 1))
		p = PIECE_PLAYER_TWO;
	    else
		p = PIECE_PLAYER_ONE;
	}
	else
	    p = PIECE_PLAYER_ONE;
    }

    addch(pdisplaycolours->character[piece] | dp_attr[p]);
}

void display_moves(struct level* plevel, struct level* plevelreplay)
{
    static int length = 0;
    int i;
    char buffer[256];
    int moves, moves2;

    moves = 0;
    if(plevel->move_current != NULL)
    {
        /* If move_current->mover_first == NULL, we've actually undone all of
           the current move, and are just about to move back to the previous
           one; we treat it as the previous one for counting purposes */
        if(plevel->move_current->mover_first != NULL)
            moves = plevel->move_current->count;
        else
            moves = plevel->move_current->count - 1;
    }

    moves2 = -1;
    if(plevelreplay != NULL)
    {   
        moves2 = 0;
        if(plevelreplay->move_last != NULL)
            moves2 = plevelreplay->move_last->count;
    }
    /* Similarly, move_current->mover_first == NULL complicates things here */
    else if(plevel->move_current != plevel->move_last
            || (plevel->move_current != NULL && plevel->move_current->mover_first == NULL))
    {   
        if(plevel->move_last != NULL)
            moves2 = plevel->move_last->count;
    }

    if(moves2 != -1)
        sprintf(buffer, "%s%d/%d",
                plevel->flags & LEVELFLAG_PAUSED ? gettext("paused ") :
                plevelreplay != NULL ? gettext("replay ") : "",
                moves, moves2);
    else
        sprintf(buffer, "%s%d",
                plevel->flags & LEVELFLAG_PAUSED ? gettext("paused ") : "",
                moves);

    if(plevel->flags & LEVELFLAG_FAILED)
        sprintf(buffer, gettext("failed"));

    attron(COLOR_PAIR(colourpair_cyan));

    /* Blank previous display only if necessary */
    if(utf8strlen(buffer) < length)
    {
        for(i = 0; i < length; i ++)
	    mvprintw(display_size_y - 1, display_size_x - 2 - length + i, " "); 
    }

    length = utf8strlen(buffer);

    mvprintw(display_size_y - 1, display_size_x - utf8strlen(buffer) - 2, "%s", buffer);
    attroff(COLOR_PAIR(colourpair_cyan));

    move(display_size_y - 1, display_size_x - 1);
    display_piece(plevel, PIECE_PLAYER_ONE + plevel->player);
    refresh();
}

void display_stars(struct level* plevel)
{
    static int length = 0;
    char buffer[256];
    int i;

    sprintf(buffer, "%d/%d", plevel->stars_caught, plevel->stars_total);

    if(plevel->stars_exploded != 0)
        sprintf(buffer, gettext("%d lost"), plevel->stars_exploded);

    if(plevel->flags & LEVELFLAG_SOLVED && !(plevel->flags & LEVELFLAG_FAILED))
        sprintf(buffer, gettext("solved"));

    attron(COLOR_PAIR(colourpair_yellow));

    /* Blank previous display only if necessary */
    if(utf8strlen(buffer) < length)
    {   
        for(i = 0; i < length; i ++)
	    mvprintw(display_size_y - 1, i + 2, " ");
    }

    length = utf8strlen(buffer);

    mvprintw(display_size_y - 1, 2, "%s", buffer);
    attroff(COLOR_PAIR(colourpair_yellow));

    move(display_size_y - 1, 0);
    display_piece(plevel, PIECE_STAR);
}

int display_focus(struct level* plevel)
{
    int px, py;
    int ox, oy;
#ifdef XOR_COMPATIBILITY
    int redraw;
#endif

    getmaxyx(stdscr, display_size_y, display_size_x);

#ifdef XOR_COMPATIBILITY
    if(plevel->mode == MODE_XOR && options_xor_display)
    {  
        if(display_start_x != plevel->view_x[plevel->player] || display_start_y != plevel->view_y[plevel->player])
            redraw = 1;
        else
            redraw = 0;

        display_start_x = plevel->view_x[plevel->player];
        display_start_y = plevel->view_y[plevel->player];
        display_end_x = display_start_x + 8;
        display_end_y = display_start_y + 8;

        return redraw;
    }
#endif

    ox = display_start_x;
    oy = display_start_y;
    px = plevel->player_x[plevel->player];
    py = plevel->player_y[plevel->player];

    if(plevel->size_x < display_size_x)
    {
        display_start_x = 0;
        display_end_x = plevel->size_x;
    }
    else
    {
        if(px < display_start_x + display_border)
           display_start_x = px - display_border;
        if(px >= display_start_x + display_size_x - display_border)
            display_start_x = px - display_size_x + display_border;
        if(display_start_x < 0)
            display_start_x = 0;
        if(display_start_x + display_size_x > plevel->size_x)
            display_start_x = plevel->size_x - display_size_x;
        display_end_x = display_start_x + display_size_x;

    }
    if(plevel->size_y < display_size_y - 1)
    {
        display_start_y = 0;
        display_end_y = plevel->size_y;
    }
    else
    {
        if(py < display_start_y + display_border)
           display_start_y = py - display_border;
        if(py >= display_start_y + display_size_y - 1 - display_border)
            display_start_y = py - display_size_y + 1 + display_border;
        if(display_start_y < 0)
            display_start_y = 0;
        if(display_start_y + display_size_y - 1 > plevel->size_y)
            display_start_y = plevel->size_y - display_size_y + 1;
        display_end_y = display_start_y + display_size_y - 1;

    }

    if(ox != display_start_x || oy != display_start_y)
        return 1;
    else
        return 0;
}

void display_level(struct level* plevel)
{
    int x, y;
    int p;

    clear();

    getmaxyx(stdscr, display_size_y, display_size_x);

    if(display_start_x < 0)
        display_start_x = 0;
    if(display_start_y < 0)
        display_start_y = 0;

    if(display_end_x > plevel->size_x)
        display_end_x = plevel->size_x;
    if(display_end_y > plevel->size_y)
        display_end_y = plevel->size_y;

    display_offset_x = (display_size_x - (display_end_x - display_start_x))/2;
    display_offset_y = (display_size_y - (display_end_y - display_start_y))/2;

    for(y = display_start_y; y < display_end_y; y++)
    {
        move(y + display_offset_y - display_start_y, display_offset_x);
        for(x = display_start_x; x < display_end_x; x++)
        {
            p = level_piece(plevel, x, y);
#ifdef XOR_COMPATIBILITY
            if(plevel->switched && (p == PIECE_SPACE || p == PIECE_WALL))
                p = PIECE_DARKNESS;
#endif
            display_piece(plevel, p);
        }
    }
}

void display_play(struct level* plevel, struct level* plevelreplay)
{
    int key;
    int quit;
    struct mover* pmover;
    int redraw;
    int x, y;
    int p;
    int playermove;
    int delay;
    int fast;
    int pass;
    int c;
    short cp;
    char font_logo_colours[] = "1326454646644";
    char buffer[256];

    quit = 0;
    redraw = 1;
    fast = 0;

    while(!quit)
    {
	redraw += display_focus(plevel);

	if(redraw)
	{
	    display_level(plevel);

	    if(plevel->title != NULL)
	    {
                y = display_size_y - 1;
                x = (display_size_x - utf8strlen(plevel->title) - (plevel->flags & LEVELFLAG_TESTING ? utf8strlen(gettext("testing: ")) : 0) ) / 2;
                if(x < 0)
                    x = 0;
                move(y, x);

                if(plevel->flags & LEVELFLAG_TESTING)
                {
                    attron(COLOR_PAIR(colourpair_cyan));
                    printw(gettext("testing: "));
                    attroff(COLOR_PAIR(colourpair_cyan));
                }
                if((strncmp(gettext(plevel->title), "chroma", 6) == 0))
                {
                    strcpy(buffer, gettext(plevel->title));

                    for(x = 0; x < strlen(buffer); x ++)
                    {
                        cp = colourpair_white;
                        if(x < strlen(font_logo_colours))
                            c = font_logo_colours[x] - '0';
                        else
			{
    			    printw(buffer + x);
			    x = strlen(buffer);
			    break;
			}
                        switch(c)
                        {
                            case 1:
                                cp = colourpair_red;
                                break;
                            case 2:
                                cp = colourpair_green;
                                break;
                            case 3:
                                cp = colourpair_yellow;
                                break;
                            case 4:
                                cp = colourpair_blue;
                                break;
                            case 5:
                                cp = colourpair_magenta;
                                break;
                            case 6:
                                cp = colourpair_cyan;
                                break;
                            default:
                                cp = colourpair_white;
                                break;
                        }
                        addch((*(buffer + x)) | COLOR_PAIR(cp));
                    }
                }
                else
                    printw("%s", plevel->title);
	    }

            display_moves(plevel, plevelreplay);
            display_stars(plevel);

            refresh();
	    curs_set(0);
	    redraw = 0;
	}

	/* If there are movers, plot and then evolve them */
        if(plevel->mover_first != NULL && !(plevel->flags & LEVELFLAG_PAUSED))
        {
	    /* Plot movers in two passes - first spaces, then non-spaces.
	       This is counter-intuitive, but makes undoing the player work. */
	    for(pass = 0; pass < 2; pass ++)
	    {
                pmover = plevel->mover_first;
                while(pmover != NULL)
                {
		    if((pass == 0 && pmover->piece != PIECE_SPACE) ||
		       (pass == 1 && pmover->piece == PIECE_SPACE))
		    {
			pmover = pmover->next;
			continue;
		    }
		    x = pmover->x;
		    y = pmover->y;;
		    if(x >= display_start_x && x < display_end_x && y>= display_start_y && y < display_end_y)
		    {
                        move(display_offset_y - display_start_y + y, display_offset_x - display_start_x + x);
                        p = pmover->piece;
#ifdef XOR_COMPATIBILITY
		        if(plevel->switched && (p == PIECE_SPACE || p == PIECE_WALL))
			    p = PIECE_DARKNESS;
#endif
		        if(p != PIECE_GONE)
                            display_piece(plevel, p);
		    }
                    pmover = pmover->next;
		}
            }

	    /* Debug movers */
	    if(options_debug & DEBUG_ORDER)
	    {
		/* Display the movers */
		pmover = plevel->mover_first;
		y = 0;
		while(pmover != NULL && y < display_size_y - 1)
		{
		    if(pmover->piece != PIECE_GONE)
		    {
		        move(y ++, 0);
		        display_piece(plevel, pmover->piece);
		        printw(" %2d,%2d ", pmover->x, pmover->y);

		    }
		    pmover = pmover->next;
		}
		while(y < display_size_y - 1)
		    mvprintw(y++, 0, "        ");

		/* Display the stack if our game engine uses it */
		if(0
#ifdef XOR_COMPATIBILITY
			|| (plevel->mode == MODE_XOR && options_xor_mode)
#endif
#ifdef ENIGMA_COMPATIBILITY
			|| (plevel->mode == MODE_ENIGMA && options_enigma_mode)
#endif
		  )
		{
		    pmover = plevel->stack_first;
		    y = 0;
		    while(pmover != NULL && y < display_size_y - 1)
		    {
		        if(pmover->piece != PIECE_GONE)
		        {
		            move(y ++, display_size_x - 8);
		            display_piece(plevel, pmover->piece);
		            printw(" %2d,%2d ", pmover->x, pmover->y);

		        }
		        pmover = pmover->next;
		    }
		    while(y < display_size_y - 1)
		        mvprintw(y++, display_size_x - 8, "        ");
		}
	    }

            refresh();

	    /* Evolve movers */
	    if(!(plevel->flags & LEVELFLAG_UNDO))
	    {
                if(level_evolve(plevel))
                    redraw += display_focus(plevel);
		level_storemovers(plevel);
	    }
	    else
	    {
                if(level_undo(plevel))
                    plevel->flags |= LEVELFLAG_UNDO;
                else
                    plevel->flags &= ~LEVELFLAG_UNDO;
	    }
	}

        /* Determine which delay to use */
	delay = options_curses_delay;
	if(plevelreplay != NULL)
	{
	   if(plevel->mover_first == NULL && plevelreplay->move_current != NULL)
	      delay = options_curses_replay_delay;

	    if(fast)
		delay = 0;

	    if(plevelreplay->flags & LEVELFLAG_UNDO)
	    {
		if(plevel->move_current == NULL && plevel->mover_first == NULL)
                {
                    if(options_curses_replay_delay != 0)
		        delay = options_curses_replay_delay;
                    else
                        delay = 1;
                }
	    }
            else
	    {
		if(plevel->mover_first == NULL && plevelreplay->move_current == NULL)
		    delay = -1;
	    }
	}
	else
	{
	    if(fast)
		delay = 0;

	    if(plevel->mover_first == NULL)
            {
		delay = -1;
	        fast = 0;
            }
	}

	if(delay > 0)
	    halfdelay(delay);

	if(delay != 0)
        {
	    key = getch();
            if(key < 0 || key >= KEY_MAX)
                key = 0;
            if(key >= 'a' && key <='z')
                key -= 32;
        }
	else
	    key = 0;

	if(delay > 0)
	    cbreak();

	playermove = MOVE_NONE;

	switch(actions[key])
	{
            case ACTION_REDRAW:
		redraw = 1;
		break;

            case ACTION_HIDE:
                display_hide();
                redraw = 1;
                break;

            case ACTION_QUIT:
		quit = 1;
		break;

            case ACTION_FAST:
		fast = 1 - fast;
                break;

            case ACTION_LEFT:
                 if(plevelreplay != NULL)
                 {
                     plevelreplay->flags |= LEVELFLAG_UNDO;
                     plevelreplay->flags &= ~LEVELFLAG_PAUSED;
                 }
                 else
                     playermove = MOVE_LEFT;
		break;

	    case ACTION_RIGHT:
                if(plevelreplay != NULL)
                {   
                    plevelreplay->flags &= ~LEVELFLAG_UNDO;
                    plevelreplay->flags &= ~LEVELFLAG_PAUSED;
                }
                else
                    playermove = MOVE_RIGHT;
		break;

	    case ACTION_UP:
                if(plevelreplay != NULL)
                    plevelreplay->flags |= LEVELFLAG_PAUSED;
                else
                    playermove = MOVE_UP;
                break;

	    case ACTION_DOWN:
                if(plevelreplay != NULL)
                    plevelreplay->flags |= LEVELFLAG_PAUSED;
                else
                    playermove = MOVE_DOWN;
                break;  

            case ACTION_PAUSE:
                if(plevelreplay != NULL)
                {
                   if(plevelreplay->flags & LEVELFLAG_PAUSED)
                       plevelreplay->flags &= ~LEVELFLAG_PAUSED;
                   else
                       plevelreplay->flags |= LEVELFLAG_PAUSED;
                }
                else if(plevel->mover_first != NULL)
                {
                   if(plevel->flags & LEVELFLAG_PAUSED)
                       plevel->flags &= ~LEVELFLAG_PAUSED;
                   else
                       plevel->flags |= LEVELFLAG_PAUSED;
                   plevel->flags |= LEVELFLAG_MOVES;
                }
                break;

            case ACTION_SWAP:
                if(plevelreplay == NULL)
		    playermove = MOVE_SWAP;
		break;

            case ACTION_UNDO:
		if(plevelreplay == NULL)
		{
		    if(plevel->mover_first == NULL && !(plevel->flags & LEVELFLAG_UNDO))
		    {
		        if(level_undo(plevel))
			    plevel->flags |= LEVELFLAG_UNDO;
		        else
			    plevel->flags &= ~LEVELFLAG_UNDO;
			playermove = MOVE_NONE;
		    }
		}
		break;

            case ACTION_REDO:
		playermove = MOVE_REDO;
		break;

            default:
                break;
	}

	/* Are we replaying the level? */
	if(plevelreplay != NULL)
        {
            /* Is it time for another move? */
            if(plevel->mover_first == NULL && !(plevelreplay->flags & LEVELFLAG_PAUSED))
            {
                /* Moving backwards through replay */
                if(plevelreplay->flags & LEVELFLAG_UNDO)
                {
                    if(level_undo(plevel))
                    {
                        plevel->flags |= LEVELFLAG_UNDO;
                        if(plevelreplay->move_current != NULL)
                            plevelreplay->move_current = plevelreplay->move_current->previous;
                        else
                            plevelreplay->move_current = plevelreplay->move_last;
                    }
                    else
                        plevel->flags &= ~LEVELFLAG_UNDO;
                }
                /* Moving forwards through replay */
                else
                {
                    if(plevelreplay->move_current != NULL)
                    {
                        playermove = plevelreplay->move_current->direction;
                        plevelreplay->move_current = plevelreplay->move_current->next;
                    }
                }   
            }   
        } 

	/* Can't move if we've failed or solved the level */
	if(plevel->flags & (LEVELFLAG_FAILED | LEVELFLAG_SOLVED))
	    playermove = MOVE_NONE;

	/* If we can move, make the move */
	if(playermove != MOVE_NONE && plevel->mover_first == NULL)
	    level_move(plevel, playermove);

	/* Display things changed by the move */
        if(plevel->flags & LEVELFLAG_MOVES)
        {
	    display_moves(plevel, plevelreplay);
            plevel->flags ^= LEVELFLAG_MOVES;
        }

        if(plevel->flags & LEVELFLAG_STARS)
        { 
	    display_stars(plevel);
            plevel->flags ^= LEVELFLAG_STARS;
        }

        if(plevel->flags & LEVELFLAG_SWITCH)
        {
	    redraw = 1;
            plevel->flags ^= LEVELFLAG_SWITCH;
        }

#ifdef XOR_COMPATIBILITY
	if(plevel->flags & LEVELFLAG_MAP)
	{
            /* No sensible way to handle this in curses */
	    plevel->flags ^= LEVELFLAG_MAP;
	}
#endif

        if(!(plevel->flags & LEVELFLAG_SOLVED) && plevel->flags & LEVELFLAG_EXIT)
        {
            plevel->flags |= LEVELFLAG_SOLVED;
	    display_stars(plevel);
        }

        if(!(plevel->flags & LEVELFLAG_FAILED) && plevel->alive[0] == 0 && plevel->alive[1] ==0)
        {
            plevel->flags |= LEVELFLAG_FAILED;
	    display_moves(plevel, plevelreplay);
        }
    }
}

void display_edit(struct level* plevel)
{
    int key;
    int quit;
    static int editor_piece = PIECE_SPACE;
    int redraw, moved, pmoved;
    int i;
    int player;
    int piece_count;

    redraw = 1;
    moved = 1;
    pmoved = 1;

    /* Store player */
    player = plevel->player;
    plevel->player = 2;

    piece_count = 0;
    while(editor_piece_maps[plevel->mode][piece_count] != PIECE_GONE)
        piece_count ++;

    if(editor_piece > piece_count)
        editor_piece = 0;

    quit = 0;
    while(!quit)
    {
        redraw += display_focus(plevel);
	
	if(redraw)
	{
	    redraw = 0;

            display_level(plevel);

            for(i = 0; i < piece_count; i ++)
            {
        	move(display_size_y - 1, 1 + i * 2);
        	display_piece(plevel, editor_piece_maps[plevel->mode][i]);
            }

            move(display_size_y - 1, display_size_x - 4);
            printw("[ ]");
	    
	    curs_set(1);

	    pmoved = 1;
	}

	if(pmoved)
	{
	    pmoved = 0;

	    move(display_size_y - 1, editor_piece * 2);
	    printw(">");
	    move(display_size_y - 1, 2 + editor_piece * 2);
	    printw("<");
	    move(display_size_y - 1, display_size_x - 3);
	    display_piece(plevel, editor_piece_maps[plevel->mode][editor_piece]);
	    moved = 1;
	}

	if(moved)
	{
	    moved = 0;

            move(display_offset_y - display_start_y + plevel->player_y[2], display_offset_x - display_start_x + plevel->player_x[2]);
	    refresh();
	}
	
	key = getch();
        if(key < 0 || key >= KEY_MAX)
            key = 0;
        if(key >= 'a' && key <='z')
            key -= 32;

	switch(actions[key])
	{
            case ACTION_REDRAW:
		redraw = 1;
		break;

            case ACTION_HIDE:
                display_hide();
                redraw = 1;
                break;

            case ACTION_QUIT:
		quit = 1;
		break;

	    case ACTION_LEFT:
		if(plevel->player_x[2] > 0)
		{
		    plevel->player_x[2] --; moved = 1;
		}
		break;

	    case ACTION_RIGHT:
		if(plevel->player_x[2] < plevel->size_x - 1)
		{
		    plevel->player_x[2] ++; moved = 1;
		}
		break;

	    case ACTION_UP:
		if(plevel->player_y[2] > 0)
		{
		    plevel->player_y[2] --; moved = 1;
		}
		break;

	    case ACTION_DOWN:
		if(plevel->player_y[2] < plevel->size_y -1)
		{
		    plevel->player_y[2] ++; moved = 1;
		}
		break;

            case ACTION_SWAP:
		level_setpiece(plevel, plevel->player_x[2], plevel->player_y[2], editor_piece_maps[plevel->mode][editor_piece]);
  	        display_piece(plevel, editor_piece_maps[plevel->mode][editor_piece]);
		moved = 1;
		break;

            case ACTION_PIECE_LEFT:
                move(display_size_y - 1, editor_piece * 2);
                printw(" ");
                move(display_size_y - 1, 2 + editor_piece * 2);
                printw(" ");
		editor_piece --;
		if(editor_piece < 0)
		    editor_piece = piece_count - 1;
		pmoved = 1;
		break;

            case ACTION_PIECE_RIGHT:
                move(display_size_y - 1, editor_piece * 2);
                printw(" ");
                move(display_size_y - 1, 2 + editor_piece * 2);
                printw(" ");
		editor_piece ++;
		if(editor_piece >= piece_count)
		    editor_piece = 0;
		pmoved = 1;
		break;
	}
    }

    /* Restore real player */
    plevel->player = player;
}

int display_type()
{
    return DISPLAY_CURSES;
}

void display_options()
{
    struct menu* pmenu;
    struct menu* pcoloursmenu;
    struct menuentry* pentrycolours;
    struct menuentry* pentryspeed;
    struct menuentry* pentryreplayspeed;
    char buffer[256];
    int ok;
    int result;

    pmenu = menu_new(gettext("Display Options"));

    menuentry_new(pmenu, gettext("Return to previous menu"), 'Q', 0);
    menuentry_new(pmenu, "", 0, MENU_SPACE);

    menuentry_new(pmenu, gettext("Save Options"), 'S', 0);
    menuentry_new(pmenu, "", 0, MENU_SPACE);

    pentrycolours = menuentry_new(pmenu, gettext("Colour Scheme"), 'C', 0);
    menuentry_new(pmenu, "", 0, MENU_SPACE);

    pentryspeed = menuentry_new(pmenu, gettext("Move Speed"), 'M', MENU_SCROLLABLE);
    pentryreplayspeed = menuentry_new(pmenu, gettext("Replay Speed"), 'R', MENU_SCROLLABLE);
    menuentry_new(pmenu, "", 0, MENU_SPACE);

    menuentry_new(pmenu, gettext("Change Keys"), 'K', 0);

    /* XOR and Enigma options are only visible once an appropriate level has
     * been seen so as not to confuse those simply playing Chroma levels */
    if(0
#ifdef XOR_COMPATIBILITY
            || options_xor_options
#endif
#ifdef ENIGMA_COMPATIBILITY
            || options_enigma_options
#endif
      )
    {
        menuentry_new(pmenu, "", 0, MENU_SPACE);
        menuentry_new(pmenu, gettext("Other Games Options"), 'X', 0);
    }

    if(options_debug & DEBUG_MENU)
    {   
        menuentry_new(pmenu, "", 0, MENU_SPACE);
        menuentry_new(pmenu, gettext("Debug Options"), 'D', 0);
    }

    ok = 0;
    while(!ok)
    {
        if(pdisplaycolours == NULL)
            menuentry_extratext(pentrycolours, gettext("** NONE **"), NULL, NULL);
        else if(pdisplaycolours->title == NULL)
            menuentry_extratext(pentrycolours, gettext("[untitled colours]"), NULL, NULL);
        else if(pdisplaycolours->flags & COLOURS_TRANSLATE)
            menuentry_extratext(pentrycolours, gettext(pdisplaycolours->title), NULL, NULL);
        else
            menuentry_extratext(pentrycolours, pdisplaycolours->title, NULL, NULL);

	switch(options_curses_delay)
	{
	    case -1:
		sprintf(buffer, gettext("after a key is pressed"));
		break;
	    case 0:
		sprintf(buffer, gettext("instantaneous"));
		break;
	    default:
		sprintf(buffer, gettext("%d00 milliseconds"), options_curses_delay);
		break;
	}
	menuentry_extratext(pentryspeed, buffer, NULL, NULL);

	switch(options_curses_replay_delay)
	{
	    case -1:
		sprintf(buffer, gettext("after a key is pressed"));
		break;
	    case 0:
		sprintf(buffer, gettext("instantaneous"));
		break;
	    default:
		sprintf(buffer, gettext("%d00 milliseconds"), options_curses_replay_delay);
		break;
	}
	menuentry_extratext(pentryreplayspeed, buffer, NULL, NULL);

        result = menu_process(pmenu);
        if(result == MENU_QUIT)
            ok = 1;

        if(result == MENU_SELECT && pmenu->entry_selected != NULL)
        {   
            switch(pmenu->entry_selected->key)
            {   
                case 'Q':
                    ok = 1;
                    break;

                case 'C':
                    pcoloursmenu = colours_menu();
                    if(menu_process(pcoloursmenu) == MENU_SELECT)
                    {   
                        if(pcoloursmenu->entry_selected != NULL && pcoloursmenu->entry_selected->value != NULL)
                        {   
                            strcpy(options_colours, pcoloursmenu->entry_selected->value);
                            colours_init();
                            display_initcolours();
                        }
                    }
                    menu_delete(pcoloursmenu);
                    break;

                case 'S':
                    display_options_save();
                    ok = 1;
                    break;

                case 'K':
                    display_keys();
                    break;

                case 'X':
                    display_options_othergames();
                    break;

                case 'D':
                    display_debug();
                    break;
            }
        }

        if(result == MENU_SCROLLLEFT && pmenu->entry_selected != NULL)
        {   
            switch(pmenu->entry_selected->key)
            {   
		case 'M':
		    options_curses_delay --;
		    if(options_curses_delay < -1)
			options_curses_delay = 10;
#ifdef PDCURSES
		    if(options_curses_delay > 0)
		        options_curses_delay = 0;
#endif
		    break;
		case 'R':
		    options_curses_replay_delay --;
		    if(options_curses_replay_delay < -1)
			options_curses_replay_delay = 10;

#ifdef PDCURSES
		    if(options_curses_replay_delay > 0)
		        options_curses_replay_delay = 0;
#endif
		    break;
	    }
        }

        if(result == MENU_SCROLLRIGHT && pmenu->entry_selected != NULL)
        {   
            switch(pmenu->entry_selected->key)
            {   
		case 'M':
		    options_curses_delay ++;
		    if(options_curses_delay > 10)
			options_curses_delay = -1;
#ifdef PDCURSES
		    if(options_curses_delay > 0)
		        options_curses_delay = 0;
#endif
		    break;
                case 'R':
                    options_curses_replay_delay ++;
                    if(options_curses_replay_delay > 10)
                        options_curses_replay_delay = -1;
#ifdef PDCURSES
		    if(options_curses_replay_delay > 0)
		        options_curses_replay_delay = 0;
#endif
                    break;
	    }
        }

    }

    menu_delete(pmenu);
}

int display_keyfixed(int i)
{
    if(i == 0 || i == KEY_RESIZE || i == 27 || i == 'Q' || i == '\n' || i == '\r' || i == KEY_UP || i == KEY_DOWN || i == KEY_LEFT || i == KEY_RIGHT)
        return 1;

    return 0;
}

char *display_keyname(int i)
{
    static char buffer[4];

    if(i == '\t')
        return "TAB";
    if(i == '\n')
        return "ENTER";
    if(i == 27)
        return "ESCAPE";
    if(i == 32)
        return "SPACE";
    if(i == KEY_DC)
        return "DELETE";
    if(i == KEY_IC)
        return "INSERT";

    if(keyname(i) == NULL)
        return "UNKNOWN";

    if(strcmp(keyname(i), "NO KEY NAME") == 0)
    {
        if(i >= 0 && i < 32)
        {
            sprintf(buffer, "^%c", i + '@');
            return buffer;
        }
        if(i > 32 && i < 127)
        {
            sprintf(buffer, "%c", i);
            return buffer;
        }

        return "UNKNOWN";
    }

    if(strncmp(keyname(i), "KEY_", 4) == 0)
        return (char *)(keyname(i) + 4);

    return (char *)keyname(i);
}

void display_addkeytomenu(struct menu* pmenu, int action, char *text)
{
    struct menuentry *pentry;
    char buffer[256];
    int i;

    sprintf(buffer, "%d", action);
    pentry = menuentry_newwithvalue(pmenu, text, 0, MENU_DOUBLE, buffer);

    strcpy(buffer, "");
    for(i = 0; i < KEY_MAX; i ++)
    {
        if(actions[i] == action && i != KEY_RESIZE)
        {
            if(strlen(buffer) != 0)
                strcat(buffer,", ");
            strcat(buffer, "[");
            strcat(buffer, display_keyname(i));
            strcat(buffer, "]");
        }
    }

    if(strcmp(buffer, "") == 0)
        strcpy(buffer, "(none)");

    menuentry_extratext(pentry, NULL, NULL, buffer);

}

void display_keys()
{
    struct menu *pmenu;
    struct menu *psubmenu;
    struct menuentry *pentry;
    int action;
    int result;
    int redraw;
    int ok;
    int subok;
    char buffer[256];
    int i;
    int key;

    ok = 0;
    while(!ok)
    {
        pmenu = menu_new(gettext("Keys"));

        menuentry_new(pmenu, gettext("Quit and return to previous menu"), 'Q', 0);
        menuentry_new(pmenu, "", 0, MENU_SPACE);

        display_addkeytomenu(pmenu, ACTION_LEFT, gettext(action_name[ACTION_LEFT]));
        display_addkeytomenu(pmenu, ACTION_RIGHT, gettext(action_name[ACTION_RIGHT]));
        display_addkeytomenu(pmenu, ACTION_UP, gettext(action_name[ACTION_UP]));
        display_addkeytomenu(pmenu, ACTION_DOWN, gettext(action_name[ACTION_DOWN]));
        display_addkeytomenu(pmenu, ACTION_SWAP, gettext(action_name[ACTION_SWAP]));
        display_addkeytomenu(pmenu, ACTION_UNDO, gettext(action_name[ACTION_UNDO]));
        display_addkeytomenu(pmenu, ACTION_REDO, gettext(action_name[ACTION_REDO]));
        display_addkeytomenu(pmenu, ACTION_FAST, gettext(action_name[ACTION_FAST]));
        display_addkeytomenu(pmenu, ACTION_PAUSE, gettext(action_name[ACTION_PAUSE])); 
        display_addkeytomenu(pmenu, ACTION_QUIT, gettext(action_name[ACTION_QUIT]));
        display_addkeytomenu(pmenu, ACTION_REDRAW, gettext(action_name[ACTION_REDRAW]));
        display_addkeytomenu(pmenu, ACTION_HIDE, gettext(action_name[ACTION_HIDE]));
        display_addkeytomenu(pmenu, ACTION_PIECE_LEFT, gettext(action_name[ACTION_PIECE_LEFT]));
        display_addkeytomenu(pmenu, ACTION_PIECE_RIGHT, gettext(action_name[ACTION_PIECE_RIGHT]));

        menu_assignletters(pmenu);

        result = menu_process(pmenu);

        if(result == MENU_QUIT)
            ok = 1;

        if(result == MENU_SELECT)
        {   
            if(pmenu->entry_selected->key == 'Q')
                ok = 1;
            else if(pmenu->entry_selected->value != NULL)
            {
                redraw = MENUREDRAW_ALL;
                subok = 0;
                while(!subok)
                {
                    action = atoi(pmenu->entry_selected->value);

                    sprintf(buffer, gettext("Set keys for '%s'"), gettext(action_name[action]));
                    psubmenu = menu_new(buffer);

                    menuentry_new(psubmenu, gettext("Quit and return to previous menu"), 'Q', 0);
                    menuentry_new(psubmenu, "", 0, MENU_SPACE);

                    for(i = 0; i < KEY_MAX; i ++)
                    {   
                        if(actions[i] == action && i != KEY_RESIZE)
                        {   
                            sprintf(buffer, "[%s]", display_keyname(i));
                            pentry = menuentry_new(psubmenu, buffer, 0, MENU_GREY);
                            if(display_keyfixed(i))
                                menuentry_extratext(pentry, gettext("(fixed)"), NULL, NULL);
                        }
                    }
                    menuentry_new(psubmenu, "", 0, MENU_SPACE);

                    menuentry_new(psubmenu, gettext("Press a key to add or remove it from this list."), 0, MENU_NOTE | MENU_CENTRE);

                    menu_display(psubmenu, redraw);
                    redraw = MENUREDRAW_ENTRIES;
                    menu_delete(psubmenu);

                    key = getch();
                    if(key == KEY_RESIZE)
                    {
                        getmaxyx(stdscr, display_size_y, display_size_x);
                        redraw = MENUREDRAW_ALL;
                    }

                    if(key >= 'a' && key <='z')
                        key -=32;

                    if(key == 27 || key == 'q' || key == 'Q' || key== '\n')
                        subok = 1;
                    else if(!display_keyfixed(key))
                    {
                        if(actions[key] == action)
                            actions[key] = ACTION_NONE;
                        else
                            actions[key] = action;
                    }
                }
            }
        }

        menu_delete(pmenu);
    }
}

void display_debug()
{
    struct menu* pmenu;
    struct menuentry* pentrymovers;
    struct menuentry* pentryhidden;
    int ok;
    int result;

    pmenu = menu_new(gettext("Debug Options"));

    menuentry_new(pmenu, gettext("Return to previous menu"), 'Q', 0);

    menuentry_new(pmenu, "", 0, MENU_SPACE);

    pentrymovers = menuentry_new(pmenu, gettext("Display order of movers"), 'O', MENU_SCROLLABLE);
    pentryhidden = menuentry_new(pmenu, gettext("Show hidden items"), 'H', MENU_SCROLLABLE);

    ok = 0;
    while(!ok)
    {
        menuentry_extratext(pentrymovers, options_debug & DEBUG_ORDER ? gettext("yes") : gettext("no"), NULL, NULL);
        menuentry_extratext(pentryhidden, options_debug & DEBUG_HIDDEN ? gettext("yes") : gettext("no"), NULL, NULL);

        result = menu_process(pmenu);
        if(result == MENU_QUIT)
            ok = 1;

        if((result == MENU_SELECT || result == MENU_SCROLLLEFT || result == MENU_SCROLLRIGHT) && pmenu->entry_selected != NULL)
        {   
            switch(pmenu->entry_selected->key)
            {   
                case 'Q':
                    ok = 1;
                    break;

                case 'O':
                    options_debug ^= DEBUG_ORDER;
                    break;

                case 'H':
                    options_debug ^= DEBUG_HIDDEN;
                    break;
            }

            pmenu->redraw = MENUREDRAW_CHANGED;
            pmenu->entry_selected->redraw = 1;
        }

    }

    menu_delete(pmenu);
}

void display_options_othergames()
{
    struct menu* pmenu;
#ifdef XOR_COMPATIBILITY
    struct menuentry* pentryxormode;
    struct menuentry* pentryxordisplay;
#endif
#ifdef ENIGMA_COMPATIBILITY
    struct menuentry* pentryenigmamode;
#endif

    int ok;
    int result;

    pmenu = menu_new(gettext("Other Games Options"));

    menuentry_new(pmenu, gettext("Return to previous menu"), 'Q', 0);

    menuentry_new(pmenu, "", 0, MENU_SPACE);
 
#ifdef XOR_COMPATIBILITY 
    pentryxormode = menuentry_new(pmenu, gettext("XOR Engine"), 'X', options_xor_options ? 0 : MENU_INVISIBLE | MENU_GREY);
    pentryxordisplay = menuentry_new(pmenu, gettext("XOR Display"), 'D', options_xor_options ? 0 : MENU_INVISIBLE | MENU_GREY);
    if(options_xor_options)
        menuentry_new(pmenu, "", 0, MENU_SPACE);
#endif

#ifdef ENIGMA_COMPATIBILITY
    pentryenigmamode = menuentry_new(pmenu, gettext("Enigma Engine"), 'E', options_enigma_options ? 0 : MENU_INVISIBLE | MENU_GREY);
#endif

    ok = 0;
    while(!ok)
    {
#ifdef XOR_COMPATIBILITY
        menuentry_extratext(pentryxormode, options_xor_mode ? gettext("exact") : gettext("approximate"), NULL, NULL);
        menuentry_extratext(pentryxordisplay, options_xor_display ? gettext("partial") : gettext("full"), NULL, NULL);
#endif
#ifdef ENIGMA_COMPATIBILITY
        menuentry_extratext(pentryenigmamode, options_enigma_mode ? gettext("exact") : gettext("approximate"), NULL, NULL);
#endif

        result = menu_process(pmenu);
        if(result == MENU_QUIT)
            ok = 1;

        if(result == MENU_SELECT && pmenu->entry_selected != NULL)
        {   
            switch(pmenu->entry_selected->key)
            {   
                case 'Q':
                    ok = 1;
                    break;

#ifdef XOR_COMPATIBILITY
                case 'X':
                    options_xor_mode = 1 - options_xor_mode;
                    break;

                case 'D':
                    options_xor_display = 1 - options_xor_display;
                    break;
#endif

#ifdef ENIGMA_COMPATIBILITY
                case 'E':
                    options_enigma_mode = 1 - options_enigma_mode;
                    break;
#endif
            }

            pmenu->redraw = MENUREDRAW_CHANGED;
            pmenu->entry_selected->redraw = 1;
        }

    }

    menu_delete(pmenu);
}

void display_options_save()
{
    FILE *file;
    char filename[FILENAME_MAX];
    int i;

    getfilename("curses.chroma", filename, 1, 0);

    file = fopen(filename, "w");
    if(file == NULL)
    {
        warning("Unable to save options");
        return; 
    }

    fprintf(file, "<!-- Chroma curses options \n"
                  "     This file is automatically generated. -->\n"
                  "\n"
                  "<chroma type=\"options\">\n");

    fprintf(file, "    <colour scheme=\"%s\" />\n", options_colours);

    if(options_curses_delay == -1)
        fprintf(file, "    <move speed=\"key\" />\n");
    else
        fprintf(file, "    <move speed=\"%d\" />\n", options_curses_delay * 100);

    if(options_curses_replay_delay == -1)
        fprintf(file, "    <replay speed=\"key\" />\n");
    else
        fprintf(file, "    <replay speed=\"%d\" />\n", options_curses_replay_delay * 100);

#ifdef XOR_COMPATIBILITY
    if(options_xor_options)
        fprintf(file, "    <xor mode=\"%s\" display=\"%s\" />\n", options_xor_mode ? "exact" : "approximate", options_xor_display ? "partial" : "full");
#endif
#ifdef ENIGMA_COMPATIBILITY
    if(options_enigma_options)
        fprintf(file, "    <enigma mode=\"%s\" />\n", options_enigma_mode ? "exact" : "approximate");
#endif

    fprintf(file, "    <!-- Set <debug menu=\"yes\" /> to change debug options within Chroma -->\n");
    fprintf(file, "    <debug ");
    fprintf(file, "menu=\"%s\" ", options_debug & DEBUG_MENU ? "yes" : "no");
    fprintf(file, "order=\"%s\" ", options_debug & DEBUG_ORDER ? "yes" : "no");
    fprintf(file, "hidden=\"%s\" ", options_debug & DEBUG_HIDDEN ? "yes" : "no");
    fprintf(file,"/>\n");

    fprintf(file, "    <keys>\n");

    for(i = 0; i < KEY_MAX; i ++)
    {
        if(actions[i] != ACTION_NONE && i != KEY_RESIZE)
            fprintf(file, "        <key name=\"%s\" action=\"%s\" />\n", display_keyname(i), action_shortname[actions[i]]);
    }

    fprintf(file, "    </keys>\n");

    fprintf(file, "</chroma>\n");

    fclose(file);
}

void display_options_load()
{
    struct parser* pparser;
    char filename[FILENAME_MAX];
    int state;
    int i;
    int key, action;

    /* Sensible defaults */
#ifdef PDCURSES
    /* halfdelay() is broken in PDCurses */
    options_curses_delay = 0;
    options_curses_replay_delay = 0;
#else
    options_curses_delay = 1;
    options_curses_replay_delay = 1;
#endif
#ifdef XOR_COMPATIBILITY
    options_xor_options = 0;
    options_xor_mode = 1;
    options_xor_display = 0;
#endif
#ifdef ENIGMA_COMPATIBILITY
    options_enigma_options = 0;
    options_enigma_mode = 1;
#endif
    options_debug = 0;

    getfilename("colours", filename, 0, 1);
    sprintf(options_colours, "%s/%s", filename, COLOURS_DEFAULT);

    getfilename("curses.chroma", filename, 0, 0);

    for(i = 0; i < KEY_MAX; i ++)
    {
        actions[i] = ACTION_NONE;
    }

    /* Fixed keys */
    actions[KEY_RESIZE] = ACTION_REDRAW;
    actions[KEY_UP] = ACTION_UP;
    actions[KEY_DOWN] = ACTION_DOWN;
    actions[KEY_LEFT] = ACTION_LEFT;
    actions[KEY_RIGHT] = ACTION_RIGHT;
    actions['\r'] = ACTION_SWAP;
    actions['\n'] = ACTION_SWAP;
    actions['Q'] = ACTION_QUIT;
    actions[27] = ACTION_QUIT;

    /* Sensible default keys */
    if(!isfile(filename))
    {
        actions[12] = ACTION_REDRAW;
        actions[' '] = ACTION_SWAP;
        actions['F'] = ACTION_FAST;
        actions[KEY_BACKSPACE] = ACTION_UNDO;
        actions[KEY_DC] = ACTION_UNDO;
        actions['U'] = ACTION_UNDO;
        actions[KEY_IC] = ACTION_REDO;
        actions['Y'] = ACTION_REDO;
        actions['Z'] = ACTION_PIECE_LEFT;
        actions['X'] = ACTION_PIECE_RIGHT;
        actions[KEY_PPAGE] = ACTION_PIECE_LEFT;;
        actions[KEY_NPAGE] = ACTION_PIECE_RIGHT;
        actions['P'] = ACTION_PAUSE;
        return;
    }

    /* Parse XML file */
    /*
       <chroma type="options">
           <colour scheme="filename" />
           <move speed="speed" />
           <replay speed="speed" />
           <xor mode="mode" />
           <debug menu="yes/no" movers="yes/no" />
           <keys>
               <key name="name" action="action" />
           </keys>
       </chroma>
    */

    pparser = parser_new(filename);

    enum {
        OPTIONSPARSER_END,       /* End of file */
        OPTIONSPARSER_OUTSIDE,   /* Outside of <chroma> */
        OPTIONSPARSER_CHROMA,    /* Inside <chroma> */
        OPTIONSPARSER_KEYS       /* Inside <keys> */
    };

    state = OPTIONSPARSER_OUTSIDE;
    key = 0;
    action = 0;

    while(state != OPTIONSPARSER_END)
    {
        switch(parser_parse(pparser))
        {
            case PARSER_END:
                state = OPTIONSPARSER_END;
                break;

            case PARSER_ELEMENT_START:
                switch(state)
                {
                    case OPTIONSPARSER_CHROMA:
                        if(parser_match(pparser, 0, "keys"))
                            state = OPTIONSPARSER_KEYS;
                        break;

                    case OPTIONSPARSER_KEYS:
                        if(parser_match(pparser, 0, "key"))
                        {
                            key = 0;
                            action = ACTION_NONE;
                        }
                        break;

                    default:
                        break;
                }
                break;

            case PARSER_ELEMENT_END:
                switch(state)
                {
                    case OPTIONSPARSER_KEYS:
                        if(parser_match(pparser, 0, "keys"))
                        {
                            state = OPTIONSPARSER_CHROMA;
                        }
                        if(parser_match(pparser, 0, "key"))
                        {
                            if(key != 0 && !display_keyfixed(key))
                                actions[key] = action;
                        }
                        break;

                    default:
                        break;
                }
                break;

            case PARSER_CONTENT:
                break;

            case PARSER_ATTRIBUTE:
                switch(state)
                {
                    case OPTIONSPARSER_OUTSIDE:
                        if(parser_match(pparser, 2, "chroma") && parser_match(pparser, 1, "type"))
                        {   
                            if(parser_match(pparser, 0, "options"))
                                state = OPTIONSPARSER_CHROMA;
                        }
                        break;

                    case OPTIONSPARSER_CHROMA:
                        if(parser_match(pparser, 2, "colour") && parser_match(pparser, 1, "scheme"))
                        {
                            strncpy(options_colours, parser_text(pparser, 0), FILENAME_MAX);
                        }
                        if(parser_match(pparser, 2, "move") && parser_match(pparser, 1, "speed"))
                        {
                            if(parser_match(pparser, 0, "key"))
                                options_curses_delay = -1;
                            else
                                options_curses_delay = atoi(parser_text(pparser, 0)) / 100;
#ifdef PDCURSES
			    if(options_curses_delay > 0)
			        options_curses_delay = 0;
#endif
                        }
                        if(parser_match(pparser, 2, "replay") && parser_match(pparser, 1, "speed"))
                        {
                            if(parser_match(pparser, 0, "key"))
                                options_curses_replay_delay = -1;
                            else
                                options_curses_replay_delay = atoi(parser_text(pparser, 0)) / 100;
#ifdef PDCURSES
			    if(options_curses_replay_delay > 0)
			        options_curses_replay_delay = 0;
#endif
                        }
#ifdef XOR_COMPATIBILITY
                        if(parser_match(pparser, 2, "xor") && parser_match(pparser, 1, "mode"))
                        {
                            options_xor_options = 1;

                            if(parser_match(pparser, 0, "approximate"))
                                options_xor_mode = 0;
                            if(parser_match(pparser, 0, "exact"))
                                options_xor_mode = 1;
                        }
                        if(parser_match(pparser, 2, "xor") && parser_match(pparser, 1, "display"))
                        {  
                            options_xor_options = 1;

                            if(parser_match(pparser, 0, "full"))
                                options_xor_display = 0;
                            if(parser_match(pparser, 0, "partial"))
                                options_xor_display = 1;
                        }
#endif
#ifdef ENIGMA_COMPATIBILITY
                        if(parser_match(pparser, 2, "enigma") && parser_match(pparser, 1, "mode"))
                        {   
                            options_enigma_options = 1;

                            if(parser_match(pparser, 0, "approximate"))
                                options_enigma_mode = 0;
                            if(parser_match(pparser, 0, "exact"))
                                options_enigma_mode = 1;
                        }
#endif
                        if(parser_match(pparser, 2, "debug") && parser_match(pparser, 1, "menu"))
                        {
                            if(parser_match(pparser, 0, "yes"))
                                options_debug |= DEBUG_MENU;
                        }
                        if(parser_match(pparser, 2, "debug") && parser_match(pparser, 1, "order"))
                        {
                            if(parser_match(pparser, 0, "yes"))
                                options_debug |= DEBUG_ORDER;
                        }
                        if(parser_match(pparser, 2, "debug") && parser_match(pparser, 1, "hidden"))
                        {
                            if(parser_match(pparser, 0, "yes"))
                                options_debug |= DEBUG_HIDDEN;
                        }
                        break;

                    case OPTIONSPARSER_KEYS:
                        if(parser_match(pparser, 2, "key") &&  parser_match(pparser, 1, "name"))
                        {
                            for(i = 0; i < KEY_MAX; i ++)
                            {
                                if(parser_match(pparser, 0, display_keyname(i)))
                                {
                                    key = i;
                                    i = KEY_MAX;
                                }
                            }
                        }
                        if(parser_match(pparser, 2, "key") &&  parser_match(pparser, 1, "action"))
                        {
                            for(i = ACTION_KEY_MIN; i < ACTION_KEY_MAX; i ++)
                            {
                                if(parser_match(pparser, 0, action_shortname[i]))
                                {
                                    action = i;
                                    i = ACTION_KEY_MAX;
                                }
                            }
                        }
                        break;
                }
                break;

            case PARSER_ERROR:
                state = OPTIONSPARSER_END;
                break;
        }
    }

    parser_delete(pparser);
}
