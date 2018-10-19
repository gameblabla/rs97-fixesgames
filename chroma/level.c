/*  
    level.c

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
#include <string.h>
#include <ctype.h>

#include "chroma.h"
#include "level.h"
#include "util.h"

extern char *piece_name[];

char piecetochar(int piece)
{
    switch(piece)
    {
        case PIECE_SPACE:
	    return ' ';
        case PIECE_WALL:
	    return '%';
        case PIECE_PLAYER_ONE:
	    return '1';
        case PIECE_PLAYER_TWO:
	    return '2';
        case PIECE_DOTS:
	    return '.';
        case PIECE_ARROW_RED_LEFT:
	    return 'a'; 
        case PIECE_ARROW_RED_UP:
	    return 'b'; 
        case PIECE_ARROW_RED_RIGHT:
	    return 'c'; 
        case PIECE_ARROW_RED_DOWN:
	    return 'd'; 
        case PIECE_BOMB_RED_LEFT:
	    return 'A'; 
        case PIECE_BOMB_RED_UP:
	    return 'B'; 
        case PIECE_BOMB_RED_RIGHT:
	    return 'C'; 
        case PIECE_BOMB_RED_DOWN:
	    return 'D'; 
        case PIECE_ARROW_GREEN_LEFT:
	    return 'e'; 
        case PIECE_ARROW_GREEN_UP:
	    return 'f'; 
        case PIECE_ARROW_GREEN_RIGHT:
	    return 'g'; 
        case PIECE_ARROW_GREEN_DOWN:
	    return 'h'; 
        case PIECE_BOMB_GREEN_LEFT:
	    return 'E'; 
        case PIECE_BOMB_GREEN_UP:
	    return 'F'; 
        case PIECE_BOMB_GREEN_RIGHT:
	    return 'G'; 
        case PIECE_BOMB_GREEN_DOWN:
	    return 'H'; 
        case PIECE_ARROW_BLUE_LEFT:
	    return 'i'; 
        case PIECE_ARROW_BLUE_UP:
	    return 'j'; 
        case PIECE_ARROW_BLUE_RIGHT:
	    return 'k'; 
        case PIECE_ARROW_BLUE_DOWN:
	    return 'l'; 
        case PIECE_BOMB_BLUE_LEFT:
	    return 'I'; 
        case PIECE_BOMB_BLUE_UP:
	    return 'J'; 
        case PIECE_BOMB_BLUE_RIGHT:
	    return 'K'; 
        case PIECE_BOMB_BLUE_DOWN:
	    return 'L'; 
        case PIECE_CIRCLE:
	    return 'o'; 
        case PIECE_STAR:
	    return '*'; 
        case PIECE_DOOR:
	    return '/'; 
#ifdef ENIGMA_COMPATIBILITY
        case PIECE_DOTS_DOUBLE:
	    return ':'; 
        case PIECE_CIRCLE_DOUBLE:
	    return '8'; 
#endif
#ifdef XOR_COMPATIBILITY
        case PIECE_DOTS_X:
	    return '-'; 
        case PIECE_DOTS_Y:
	    return '|'; 
        case PIECE_SWITCH:
	    return 'S'; 
        case PIECE_TELEPORT:
	    return 'T'; 
        case PIECE_MAP_TOP_LEFT:
	    return 'M'; 
        case PIECE_MAP_TOP_RIGHT:
	    return 'm'; 
        case PIECE_MAP_BOTTOM_LEFT:
	    return 'N'; 
        case PIECE_MAP_BOTTOM_RIGHT:
	    return 'n'; 
#endif
	case PIECE_GONE:
	    return '!';
	case PIECE_EXPLOSION_RED_LEFT:
            return 'p';
	case PIECE_EXPLOSION_RED_HORIZONTAL:
            return 'q';
	case PIECE_EXPLOSION_RED_RIGHT:
            return 'r';
	case PIECE_EXPLOSION_RED_TOP:
            return 'P';
	case PIECE_EXPLOSION_RED_VERTICAL:
            return 'Q';
	case PIECE_EXPLOSION_RED_BOTTOM:
            return 'R';
	case PIECE_EXPLOSION_GREEN_LEFT:
            return 'u';
	case PIECE_EXPLOSION_GREEN_HORIZONTAL:
            return 'v';
	case PIECE_EXPLOSION_GREEN_RIGHT:
            return 'w';
	case PIECE_EXPLOSION_GREEN_TOP:
            return 'U';
	case PIECE_EXPLOSION_GREEN_VERTICAL:
            return 'V';
	case PIECE_EXPLOSION_GREEN_BOTTOM:
            return 'W';
	case PIECE_EXPLOSION_BLUE_LEFT:
            return 'x';
	case PIECE_EXPLOSION_BLUE_HORIZONTAL:
            return 'y';
	case PIECE_EXPLOSION_BLUE_RIGHT:
            return 'z';
	case PIECE_EXPLOSION_BLUE_TOP:
            return 'X';
	case PIECE_EXPLOSION_BLUE_VERTICAL:
            return 'Y';
	case PIECE_EXPLOSION_BLUE_BOTTOM:
            return 'Z';

	default:
	    return '?';
    }
}

int chartopiece(char c)
{
    switch(c)
    {
        case ' ':
	    return PIECE_SPACE; 
        case '%':
	    return PIECE_WALL; 
        case '1':
	    return PIECE_PLAYER_ONE; 
        case '2':
	    return PIECE_PLAYER_TWO; 
        case '.':
	    return PIECE_DOTS; 
        case 'a':
	    return PIECE_ARROW_RED_LEFT; 
        case 'b':
	    return PIECE_ARROW_RED_UP; 
        case 'c':
	    return PIECE_ARROW_RED_RIGHT; 
        case 'd':
	    return PIECE_ARROW_RED_DOWN; 
        case 'A':
	    return PIECE_BOMB_RED_LEFT; 
        case 'B':
	    return PIECE_BOMB_RED_UP; 
        case 'C':
	    return PIECE_BOMB_RED_RIGHT; 
        case 'D':
	    return PIECE_BOMB_RED_DOWN; 
        case 'e':
	    return PIECE_ARROW_GREEN_LEFT; 
        case 'f':
	    return PIECE_ARROW_GREEN_UP; 
        case 'g':
	    return PIECE_ARROW_GREEN_RIGHT; 
        case 'h':
	    return PIECE_ARROW_GREEN_DOWN; 
        case 'E':
	    return PIECE_BOMB_GREEN_LEFT; 
        case 'F':
	    return PIECE_BOMB_GREEN_UP; 
        case 'G':
	    return PIECE_BOMB_GREEN_RIGHT; 
        case 'H':
	    return PIECE_BOMB_GREEN_DOWN; 
        case 'i':
	    return PIECE_ARROW_BLUE_LEFT; 
        case 'j':
	    return PIECE_ARROW_BLUE_UP; 
        case 'k':
	    return PIECE_ARROW_BLUE_RIGHT; 
        case 'l':
	    return PIECE_ARROW_BLUE_DOWN; 
        case 'I':
	    return PIECE_BOMB_BLUE_LEFT; 
        case 'J':
	    return PIECE_BOMB_BLUE_UP; 
        case 'K':
	    return PIECE_BOMB_BLUE_RIGHT; 
        case 'L':
	    return PIECE_BOMB_BLUE_DOWN; 
        case 'o':
	    return PIECE_CIRCLE; 
        case '*':
	    return PIECE_STAR; 
        case '/':
	    return PIECE_DOOR; 
#ifdef ENIGMA_COMPATIBILITY
        case '8':
	    return PIECE_CIRCLE_DOUBLE; 
        case ':':
	    return PIECE_DOTS_DOUBLE; 
#endif
#ifdef XOR_COMPATIBILITY
        case '-':
	    return PIECE_DOTS_X; 
        case '|':
	    return PIECE_DOTS_Y; 
        case 'S':
	    return PIECE_SWITCH; 
        case 'T':
	    return PIECE_TELEPORT; 
        case 'M':
	    return PIECE_MAP_TOP_LEFT; 
        case 'm':
	    return PIECE_MAP_TOP_RIGHT; 
        case 'N':
	    return PIECE_MAP_BOTTOM_LEFT; 
        case 'n':
	    return PIECE_MAP_BOTTOM_RIGHT; 
#endif
	case '!':
	    return PIECE_GONE;
        case 'p':
            return PIECE_EXPLOSION_RED_LEFT;
        case 'q':
	    return PIECE_EXPLOSION_RED_HORIZONTAL;
        case 'r':
	    return PIECE_EXPLOSION_RED_RIGHT;
        case 'P':
	    return PIECE_EXPLOSION_RED_TOP;
        case 'Q':
	    return PIECE_EXPLOSION_RED_VERTICAL;
        case 'R':
	    return PIECE_EXPLOSION_RED_BOTTOM;
        case 'u':
	    return PIECE_EXPLOSION_GREEN_LEFT;
        case 'v':
	    return PIECE_EXPLOSION_GREEN_HORIZONTAL;
        case 'w':
	    return PIECE_EXPLOSION_GREEN_RIGHT;
        case 'U':
	    return PIECE_EXPLOSION_GREEN_TOP;
        case 'V':
	    return PIECE_EXPLOSION_GREEN_VERTICAL;
        case 'W':
	    return PIECE_EXPLOSION_GREEN_BOTTOM;
        case 'x':
	    return PIECE_EXPLOSION_BLUE_LEFT;
        case 'y':
	    return PIECE_EXPLOSION_BLUE_HORIZONTAL;
        case 'z':
	    return PIECE_EXPLOSION_BLUE_RIGHT;
        case 'X':
	    return PIECE_EXPLOSION_BLUE_TOP;
        case 'Y':
    	    return PIECE_EXPLOSION_BLUE_VERTICAL;
        case 'Z':
	    return PIECE_EXPLOSION_BLUE_BOTTOM;
        default:
	    return PIECE_UNKNOWN;
    }
}

char directiontochar(int direction)
{
    switch(direction)
    {
	case MOVE_LEFT:
	    return 'l';
	case MOVE_UP:
	    return 'u';
	case MOVE_RIGHT:
	    return 'r';
	case MOVE_DOWN:
	    return 'd';
	case MOVE_SWAP:
	    return 's';
	case MOVE_SWAPPED:
	    return 'w';
	case MOVE_NONE:
	    return 'n';
	default:
	    return '?';
    }
}

int chartodirection(char c)
{
    switch(tolower(c))
    {
	case 'l':
	    return MOVE_LEFT;
	case 'u':
	    return MOVE_UP;
	case 'r':
	    return MOVE_RIGHT;
	case 'd':
	    return MOVE_DOWN;
	case 's':
	    return MOVE_SWAP;
	case 'w':
	    return MOVE_SWAPPED;
	case 'n':
	    return MOVE_NONE;
	default:
	    return MOVE_UNKNOWN;
    }
}

struct level* level_new()
{
    struct level* plevel;
    int i;

    plevel = (struct level*)malloc(sizeof(struct level));

    if(plevel == NULL)
	fatal("Out of memory in level_new()");

    plevel->size_x = 0;
    plevel->size_y = 0;

    plevel->player = 0;

    plevel->data_pieces = NULL;
    plevel->data_moving = NULL;
    plevel->data_previous = NULL;
    plevel->data_previousmoving = NULL;
    plevel->data_detonator = NULL;
    plevel->data_detonatormoving = NULL;
    plevel->data_data = NULL;

    plevel->move_first = NULL;
    plevel->move_last = NULL;
    plevel->move_current = NULL;

    plevel->mover_first = NULL;
    plevel->mover_last = NULL;

    plevel->stack_first = NULL;
    plevel->stack_last = NULL;

    plevel->stars_caught = 0;
    plevel->stars_exploded = 0;
    plevel->stars_total = 0;

    plevel->moves = 0;

    plevel->flags = 0;

#ifdef XOR_COMPATIBILITY
    plevel->switched = 0;
    plevel->mapped = 0;
#endif

    plevel->mode = MODE_CHROMA;

    plevel->level = 0;

    plevel->title = NULL;

    for(i = 0; i < 2; i++)
    {
	plevel->alive[i] = 0;
        plevel->teleport_x[0] = -1;
        plevel->teleport_y[0] = -1;
        plevel->view_teleport_x[0] = 0;
        plevel->view_teleport_y[0] = 0;
    }

    for(i = 0; i < 3; i++)
    {
        plevel->player_x[i] = 0;
        plevel->player_y[i] = 0;
        plevel->view_x[i] = 0;
        plevel->view_y[i] = 0;
    }

    return plevel;
}

struct level* level_create(int size_x, int size_y)
{
    struct level *pnew;
    int x, y;

    pnew = level_new();

    pnew->size_x = size_x;
    pnew->size_y = size_y;
    
    pnew->data_pieces = malloc(sizeof(char) * pnew->size_x * pnew->size_y);
    pnew->data_moving = malloc(sizeof(char) * pnew->size_x * pnew->size_y);
    pnew->data_previous = malloc(sizeof(char) * pnew->size_x * pnew->size_y);
    pnew->data_previousmoving = malloc(sizeof(char) * pnew->size_x * pnew->size_y);  
    pnew->data_detonator = malloc(sizeof(char) * pnew->size_x * pnew->size_y);
    pnew->data_detonatormoving = malloc(sizeof(char) * pnew->size_x * pnew->size_y);
    pnew->data_data = (unsigned int *)malloc(sizeof(unsigned int) * pnew->size_x * pnew->size_y);

    if(pnew->data_pieces == NULL || pnew->data_moving == NULL ||
	    pnew->data_previous == NULL || pnew->data_previousmoving == NULL ||
	    pnew->data_detonator == NULL || pnew->data_detonatormoving == NULL ||
	    pnew->data_data == NULL )
	fatal("Out of memory in level_create()");

    for(y = 0; y < pnew->size_y; y++)
    {
        for(x = 0; x < pnew->size_x; x++)
        { 
            level_setpiece(pnew, x, y, PIECE_WALL);
            level_setmoving(pnew, x, y, MOVE_NONE);
            level_setprevious(pnew, x, y, PIECE_SPACE);
            level_setpreviousmoving(pnew, x, y, MOVE_NONE);
            level_setdetonator(pnew, x, y, PIECE_SPACE);
            level_setdetonatormoving(pnew, x, y, MOVE_NONE);
            level_setdata(pnew, x, y, 0);
        }
    }

    for(y = 1; y < pnew->size_y - 1; y++)
    {
        for(x = 1; x < pnew->size_x - 1; x++)
        { 
            level_setpiece(pnew, x, y, PIECE_SPACE);
        }
    }

    return pnew;
}

void level_delete(struct level* plevel)
{
    struct mover *pmover;
    struct mover *ptmp;
    struct move *pmove;
    struct move *pmovetmp;

    if(plevel == NULL)
	return;
    
    if(plevel->title != NULL)
	free(plevel->title);

    if(plevel->data_pieces != NULL)
	free(plevel->data_pieces);
    if(plevel->data_moving != NULL)
	free(plevel->data_moving);
    if(plevel->data_previous != NULL)
	free(plevel->data_previous);
    if(plevel->data_previousmoving != NULL)
	free(plevel->data_previousmoving);
    if(plevel->data_detonator != NULL)
	free(plevel->data_detonator);
    if(plevel->data_detonatormoving != NULL)
	free(plevel->data_detonatormoving);
    if(plevel->data_data != NULL)
	free(plevel->data_data);

    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
	ptmp = pmover;
	pmover = pmover->next;
	free(ptmp);
    }

    pmover = plevel->stack_first;
    while(pmover != NULL)
    {
	ptmp = pmover;
	pmover = pmover->next;
	free(ptmp);
    }

    pmove = plevel->move_first;
    while(pmove != NULL)
    {
	pmover = pmove->mover_first;
	while(pmover != NULL)
	{
	    ptmp = pmover;
	    pmover = pmover->next;
	    free(ptmp);
	}

	pmovetmp = pmove;
	pmove = pmove->next;
	free(pmovetmp);
    }

    free(plevel);
}

char level_piece(struct level* plevel, int x, int y)
{ 
    if(plevel == NULL || plevel->data_pieces == NULL ||
	    x < 0 || x >= plevel->size_x || y < 0 || y >= plevel->size_y)
	return PIECE_WALL;

    return *(plevel->data_pieces + x + y * plevel->size_x);
} 
  
void level_setpiece(struct level* plevel, int x, int y, char piece)
{ 
    if(plevel == NULL || plevel->data_pieces == NULL ||
	    x < 0 || x >= plevel->size_x || y < 0 || y >= plevel->size_y)
	return;

    *(plevel->data_pieces + x + y * plevel->size_x) = piece;
}

char level_moving(struct level* plevel, int x, int y)
{
    if(plevel == NULL || plevel->data_moving == NULL ||
	    x < 0 || x >= plevel->size_x || y < 0 || y >= plevel->size_y)
	return MOVE_NONE;
    
    return *(plevel->data_moving + x + y * plevel->size_x);
}

void level_setmoving(struct level* plevel, int x, int y, char moving)
{
    if(plevel == NULL || plevel->data_moving == NULL ||
	    x < 0 || x >= plevel->size_x || y < 0 || y >= plevel->size_y)
	return;

    *(plevel->data_moving + x + y * plevel->size_x) = moving;
}

char level_previous(struct level* plevel, int x, int y)
{
    if(plevel == NULL || plevel->data_previous == NULL ||
	    x < 0 || x >= plevel->size_x || y < 0 || y >= plevel->size_y)
        return PIECE_WALL;
    
    return *(plevel->data_previous + x + y * plevel->size_x);
}

void level_setprevious(struct level* plevel, int x, int y, char previous)
{
    if(plevel == NULL || plevel->data_previous == NULL ||
	    x < 0 || x >= plevel->size_x || y < 0 || y >= plevel->size_y)
	return;

    *(plevel->data_previous + x + y * plevel->size_x) = previous;
}

char level_previousmoving(struct level* plevel, int x, int y)
{
    if(plevel == NULL || plevel->data_previousmoving == NULL ||
	    x < 0 || x >= plevel->size_x || y < 0 || y >= plevel->size_y)
	return MOVE_NONE;
    
    return *(plevel->data_previousmoving + x + y * plevel->size_x);
}

void level_setpreviousmoving(struct level* plevel, int x, int y, char previousmoving)
{
    if(plevel == NULL || plevel->data_previousmoving == NULL ||
	    x < 0 || x >= plevel->size_x || y < 0 || y >= plevel->size_y)
	return;

    *(plevel->data_previousmoving + x + y * plevel->size_x) = previousmoving;
}

char level_detonator(struct level* plevel, int x, int y)
{
    if(plevel == NULL || plevel->data_detonator == NULL ||
	    x < 0 || x >= plevel->size_x || y < 0 || y >= plevel->size_y)
	return PIECE_SPACE;
    
    return *(plevel->data_detonator + x + y * plevel->size_x);
}

void level_setdetonator(struct level* plevel, int x, int y, char detonator)
{
    if(plevel == NULL || plevel->data_detonator == NULL ||
	    x < 0 || x >= plevel->size_x || y < 0 || y >= plevel->size_y)
	return;

    *(plevel->data_detonator + x + y * plevel->size_x) = detonator;
}

char level_detonatormoving(struct level* plevel, int x, int y)
{
    if(plevel == NULL || plevel->data_detonatormoving == NULL ||
	    x < 0 || x >= plevel->size_x || y < 0 || y >= plevel->size_y)
	return MOVE_NONE;

    return *(plevel->data_detonatormoving + x + y * plevel->size_x);
}

void level_setdetonatormoving(struct level* plevel, int x, int y, char moving)
{
    if(plevel == NULL || plevel->data_detonator == NULL ||
	    x < 0 || x >= plevel->size_x || y < 0 || y >= plevel->size_y)
	return;

    *(plevel->data_detonatormoving + x + y * plevel->size_x) = moving;
}

unsigned int level_data(struct level* plevel, int x, int y)
{
    if(plevel == NULL || plevel->data_pieces == NULL ||
	    x < 0 || x >= plevel->size_x || y < 0 || y >= plevel->size_y)
	return 0;

    return *(plevel->data_data + x + y * plevel->size_x);
}

void level_setdata(struct level* plevel, int x, int y, unsigned int data)
{
    if(plevel == NULL || plevel->data_pieces == NULL ||
	    x < 0 || x >= plevel->size_x || y < 0 || y >= plevel->size_y)
	return;

    *(plevel->data_data + x + y * plevel->size_x) = data;
}

struct level* level_copy(struct level* pold)
{
    struct level* pnew;
    struct move* pmove;
    struct mover* pmover;
    int x, y;
    int i;

    pnew = level_new();

    pnew->size_x = pold->size_x;
    pnew->size_y = pold->size_y;

    pnew->player = pold->player;

    pnew->data_pieces = malloc(sizeof(char) * pnew->size_x * pnew->size_y);
    pnew->data_moving = malloc(sizeof(char) * pnew->size_x * pnew->size_y);
    pnew->data_previous = malloc(sizeof(char) * pnew->size_x * pnew->size_y);
    pnew->data_previousmoving = malloc(sizeof(char) * pnew->size_x * pnew->size_y);
    pnew->data_detonator = malloc(sizeof(char) * pnew->size_x * pnew->size_y);
    pnew->data_detonatormoving = malloc(sizeof(char) * pnew->size_x * pnew->size_y);
    pnew->data_data = (unsigned int *)malloc(sizeof(unsigned int) * pnew->size_x * pnew->size_y);

    if(pnew->data_pieces == NULL || pnew->data_moving == NULL ||
	    pnew->data_previous == NULL || pnew->data_previousmoving == NULL ||
	    pnew->data_detonator == NULL || pnew->data_detonatormoving == NULL ||
	    pnew->data_data == NULL )
	fatal("Out of memory in level_copy");

    pnew->mover_first = NULL;
    pnew->mover_last = NULL;

    pnew->stack_first = NULL;
    pnew->stack_last = NULL;

    pnew->stars_caught = pold->stars_caught;
    pnew->stars_exploded = pold->stars_exploded;
    pnew->stars_total = pold->stars_total;

    pnew->moves = pold->moves;
  
    pnew->flags = pold->flags;

#ifdef XOR_COMPATIBILITY
    pnew->switched = pold->switched;
    pnew->mapped = pold->mapped;
#endif

    pnew->mode = pold->mode;

    pnew->level = pold->level;

    level_settitle(pnew, pold->title);

    for(i = 0; i < 2; i ++)
    {
        pnew->alive[i] = pold->alive[i];
        pnew->teleport_x[i] = pold->teleport_x[i];
        pnew->teleport_y[i] = pold->teleport_y[i];
        pnew->view_teleport_x[i] = pold->view_teleport_x[i];
        pnew->view_teleport_y[i] = pold->view_teleport_y[i];
        pnew->player_x[i] = pold->player_x[i];
        pnew->player_y[i] = pold->player_y[i];
        pnew->view_x[i] = pold->view_x[i];
        pnew->view_y[i] = pold->view_y[i];
    }

    for(y = 0; y < pnew->size_y; y++)
    {
        for(x = 0; x < pnew->size_x; x++)
	{
	    level_setpiece(pnew, x, y, level_piece(pold, x, y));
	    level_setmoving(pnew, x, y, MOVE_NONE);
	    level_setprevious(pnew, x, y, PIECE_SPACE);
            level_setpreviousmoving(pnew, x, y, MOVE_NONE);
	    level_setdetonator(pnew, x, y, PIECE_SPACE);
	    level_setdetonatormoving(pnew, x, y, MOVE_NONE);
	    level_setdata(pnew, x, y, level_data(pold, x, y));
        }
    }

    /* Copy moves and undo data */
    pmove = pold->move_first;
    while(pmove != NULL)
    {
	level_addmove(pnew, pmove->direction);

	pnew->move_current = pnew->move_last;

	pmover = pmove->mover_first;
	while(pmover != NULL)
	{
	    mover_newundo(pnew, pmover->x, pmover->y, pmover->direction, pmover->piece, pmover->piece_previous, MOVER_STORE | (pmover->fast ? MOVER_FAST : 0));
	    pmover = pmover->next;
	}
	pmove = pmove->next;
    }

    pnew->move_current = pnew->move_last;

    return pnew;
}

struct level* level_load(char *filename, int partial)
{
    struct level* plevel;
    FILE *level;
    char buffer[4096];
    int state;
    int x, y, z;
#ifdef XOR_COMPATIBILITY
    int w;
#endif
    int i;
    char c;
    int piece, previous, direction;
    int teleport;
    int move;
    int loop;

    if(!isfile(filename))
        return NULL;

    level = fopen(filename, "r");
    if(level == NULL)
        return NULL;

    plevel = level_new();

    state = 0;
    while(!feof(level))
    {
        file_readline(level, buffer, 4096);

        /* Ignore comments and blank lines */
        if(buffer[0] == '#' || buffer[0] == 0)
	    continue;

	/* and everything before the "chroma level" line */
        if(strncmp(buffer, "chroma level", 12) == 0)
	    state = 1;
  
        if(state == 1)
        {
            if(strncmp(buffer, "mode: ", 6) == 0)
            {
                plevel->mode = MODE_MAX;
#ifdef XOR_COMPATIBILITY
	        if(strncmp(buffer, "mode: xor", 9) == 0)
		    plevel->mode = MODE_XOR;
#endif
#ifdef ENIGMA_COMPATIBILITY
	        if(strncmp(buffer, "mode: enigma", 12) == 0)
		    plevel->mode = MODE_ENIGMA;
#endif
                /* Unrecognised mode */
                if(plevel->mode == MODE_MAX)
                {
                    level_delete(plevel);
                    return NULL;
                }
            }
	    
	    /* Level data comes after level options */
            if(strncmp(buffer, "data:", 5) == 0)
	    {
		state = 2;
		break;
	    }

	    /* Read level size */
            if(strncmp(buffer, "size:", 5) == 0)
            {
                i = sscanf(buffer, "size: %d %d", &x, &y);
                if(i == 2)
                {
		    plevel->size_x = x;
		    plevel->size_y = y; 
		}
            }

            /* Read star totals */
            if(strncmp(buffer,"stars:",6)==0)
            {
                i = sscanf(buffer, "stars: %d %d %d", &x, &y, &z);
                if(i == 3)
                {
		    plevel->stars_caught = x;
		    plevel->stars_exploded = y;
		    plevel->stars_total = z; 
		}
            }

            /* Read move total */
            if(strncmp(buffer,"moves:",6)==0)
            {
                i = sscanf(buffer, "moves: %d", &x);
                if(i == 1)
                    plevel->moves = x; 
            }

            /* Read player */
            if(strncmp(buffer,"player:",7)==0)
            {
                i = sscanf(buffer, "player: %d", &x);
                if(i == 1 && (x == 1 || x == 2))
                    plevel->player = x - 1; 
            }

            /* Read level number */
            if(strncmp(buffer,"level:",6)==0)
            {
                i = sscanf(buffer, "level: %d", &x);
                if(i == 1)
		    plevel->level = x;
            }

            /* Read title */
            if(strncmp(buffer,"title: ",7)==0)
		level_settitle(plevel, buffer + 7);

  	    if(strcmp(buffer, "solved: 1") == 0)
  	        plevel->flags |= LEVELFLAG_SOLVED;

	    if(strcmp(buffer, "failed: 1") == 0)
	        plevel->flags |= LEVELFLAG_FAILED;

#ifdef XOR_COMPATIBILITY
            /* Read switched */
            if(strncmp(buffer,"switched:",9)==0)
            {
                i = sscanf(buffer, "switched: %d", &x);
                if(i == 1)
                    plevel->switched = x; 
            }
            /* Read viewpoints */
            if(strncmp(buffer,"view1: ",7)==0)
            {
                i = sscanf(buffer, "view1: %d %d", &x, &y);
                if(i == 2)
                {
                    plevel->view_x[0] = x;
                    plevel->view_y[0] = y;
                }
            }
            if(strncmp(buffer,"view2: ",7)==0)
            {
                i = sscanf(buffer, "view2: %d %d", &x, &y);
                if(i == 2)
                {
                    plevel->view_x[1] = x;
                    plevel->view_y[1] = y;
                }
            }
            if(strncmp(buffer,"viewteleport1: ",15)==0)
            {
                i = sscanf(buffer, "viewteleport1: %d %d (%d %d)", &x, &y, &z, &w);
                if(i == 4)
                {
                    plevel->teleport_x[0] = z;
                    plevel->teleport_y[0] = w;
                    plevel->view_teleport_x[0] = x;
                    plevel->view_teleport_y[0] = y;
                    teleport ++;

                }
            }
            if(strncmp(buffer,"viewteleport2: ",15)==0)
            {
                i = sscanf(buffer, "viewteleport2: %d %d (%d %d)", &x, &y, &z, &w);
                if(i == 4)
                {
                    plevel->teleport_x[1] = z;
                    plevel->teleport_y[1] = w;
                    plevel->view_teleport_x[1] = x;
                    plevel->view_teleport_y[1] = y;
                    teleport ++;
                }
            }
	    if(strncmp(buffer, "mapped: ", 8) == 0)
	    {
		if(strstr(buffer, "top_left") != NULL)
		    plevel->mapped |= MAPPED_TOP_LEFT;
		if(strstr(buffer, "top_right") != NULL)
		    plevel->mapped |= MAPPED_TOP_RIGHT;
		if(strstr(buffer, "bottom_left") != NULL)
		    plevel->mapped |= MAPPED_BOTTOM_LEFT;
		if(strstr(buffer, "bottom_right") != NULL)
		    plevel->mapped |= MAPPED_BOTTOM_RIGHT;
	    }
#endif
        }
    }

    /* If the file ended before the level data started, it's no good */
    if(state != 2)
    {
        fclose(level);
        level_delete(plevel);
        return NULL;
    }

    /* Don't load level or move data if we're indexing for a menu */
    if(partial)
    {
        fclose(level);
        return plevel;
    }

    plevel->data_pieces = malloc(sizeof(char) * plevel->size_x * plevel->size_y);
    plevel->data_moving = malloc(sizeof(char) * plevel->size_x * plevel->size_y);
    plevel->data_previous = malloc(sizeof(char) * plevel->size_x * plevel->size_y);
    plevel->data_previousmoving = malloc(sizeof(char) * plevel->size_x * plevel->size_y);
    plevel->data_detonator = malloc(sizeof(char) * plevel->size_x * plevel->size_y);
    plevel->data_detonatormoving = malloc(sizeof(char) * plevel->size_x * plevel->size_y);
    plevel->data_data = malloc(sizeof(int) * plevel->size_x * plevel->size_y);

    if(plevel->data_pieces == NULL || plevel->data_moving == NULL ||
            plevel->data_previous == NULL || plevel->data_previousmoving == NULL ||
            plevel->data_detonator == NULL || plevel->data_detonatormoving == NULL ||
            plevel->data_data == NULL )
        fatal("Out of memory in level_load");

    /* Seed random number generator based on level title */
    x = 7;
    if(plevel->title != NULL)
    {
        for(i = 0; i < strlen(plevel->title); i ++)
	    x = x ^ (plevel->title[i] << (i % 24));
    }
    srand(x);

    for(y = 0; y < plevel->size_y; y++)
    {
        for(x = 0; x < plevel->size_x; x++)
        {
            level_setpiece(plevel, x, y, PIECE_WALL);
            level_setmoving(plevel, x, y, MOVE_NONE);
  	    level_setprevious(plevel, x, y, PIECE_SPACE);
            level_setpreviousmoving(plevel, x, y, MOVE_NONE);
  	    level_setdetonator(plevel, x, y, PIECE_SPACE);
  	    level_setdetonatormoving(plevel, x, y, MOVE_NONE);
            level_setdata(plevel, x, y, rand() % 0xffff);
        }
    }

    /* Recalculate stars_total - use the saved value in partial loads only */
    plevel->stars_total = plevel->stars_caught + plevel->stars_exploded;

    teleport = 0;

    x = 0; y = 0;
    while(!feof(level) && y <= plevel->size_y)
    {
        c = fgetc(level);
        if(feof(level))
	    break;
        if(c == 10 || c == 13 || c == 0 || c == -1)
	    continue;

	piece = chartopiece(c);
      
        switch(piece)
        {
	    case PIECE_PLAYER_ONE:
		plevel->player_x[0] = x;
	        plevel->player_y[0] = y;
	        plevel->alive[0] = 1;
	        break;

	    case PIECE_PLAYER_TWO:
	        plevel->player_x[1] = x;
	        plevel->player_y[1] = y;
	        plevel->alive[1] = 1;
	        break;

            case PIECE_STAR:
                plevel->stars_total ++;
                break;

#ifdef XOR_COMPATIBILITY
            case PIECE_TELEPORT:
	        if(teleport < 2)
	        {
	            plevel->teleport_x[teleport] = x;
	            plevel->teleport_y[teleport] = y;
	            teleport ++;
	        }
	        break;
#endif
            case PIECE_UNKNOWN:
                piece = PIECE_SPACE;
                break;

	    default:
	        break;
	}

        level_setpiece(plevel, x, y, piece);

        x++;
        if(x == plevel->size_x)
        {
	    x = 0;
	    y ++;
	}
        if(y == plevel->size_y)
	    break;
    }

    /* Search for move data */
    state = 0;
    while(!feof(level))
    {
        file_readline(level, buffer, 4096);
        if(strncmp(buffer,"movedata:",9)==0)
	{
	    state = 1;
	    break;
	}
    }
    /* and if we find it, read it */
    if(state == 1)
    {
	i = 0;

        while(i < plevel->moves && !feof(level))
        {
	    move = chartodirection(fgetc(level));
	    if(move != MOVE_NONE && move != MOVE_UNKNOWN)
	    {
		level_addmove(plevel, move);
		i ++;
	    }
        }
    }

    /* Search for undo data */
    state = 0;
    while(!feof(level))
    {
        file_readline(level, buffer, 4096);
        if(strncmp(buffer,"undodata:",9)==0)
	{
	    state = 1;
	    break;
	}
    }
    /* and if we find it, read it */
    if(state == 1)
    {
	plevel->move_current = plevel->move_first;

	state = 0;

	x = 0;
	y = 0;
	piece = PIECE_UNKNOWN;
	previous = PIECE_UNKNOWN;
	direction = MOVE_UNKNOWN;

	while(plevel->move_current != NULL && !feof(level))
	{
	    c = fgetc(level);

	    if(c == 13 || c == 10)
		continue;

	    /* Undo data is read by means of a state machine. */
	    /* [x]:[y][direction][piece][previous piece][state of next move] */
	    loop = 1;
	    while(loop)
	    {
		loop = 0;
 	        switch(state)
	        {
	    	    /* Read x */
		    case 0:
		        if(c >= '0' && c <='9')
		        {
			    x = x * 10 + (c - '0');
		        }
		        else
		        {
		    	    state = 1; loop = 1;
		        }
		        break;

		    /* Read : */
		    case 1:
		        if(c == ':')
		    	    state = 2;
		        break;

		    /* Read y */
		    case 2:
		        if(c >= '0' && c <='9')
		        {
		    	    y = y * 10 + (c - '0');
		        }
		        else
		        {
			    state = 3; loop = 1;
		        }
		        break;

		    /* Read direction */
		    case 3:
		        if(chartodirection(c) != MOVE_UNKNOWN)
		        {
			    direction = chartodirection(c);
			    state = 4;
		        }
		        break;

		    /* Read piece */
		    case 4:
		        if(chartopiece(c) != PIECE_UNKNOWN)
		        {
			    piece = chartopiece(c);
			    state = 5;
		        }
		        break;

		    /* Read previous piece */
		    case 5:
		        if(chartopiece(c) != PIECE_UNKNOWN)
		        {
			    previous = chartopiece(c);
			    state = 6;
		        }
		        break;

		    /* Read state of next move */
		    case 6:
		        if(c == ',' || c == ';' || c == '.')
		        {
			    /* Add mover to current move */
			    mover_newundo(plevel, x, y, direction, piece, previous, MOVER_STORE | (c == ',' ? MOVER_FAST : 0));
			    /* Next move if this is the last mover for this one */
			    if(c == '.')
				plevel->move_current = plevel->move_current->next;

			    /* Reset state machine for next mover */
			    state = 0;
			    x = 0;
			    y = 0;
			    piece = PIECE_UNKNOWN;
			    previous = PIECE_UNKNOWN;
			    direction = MOVE_UNKNOWN;
		        }
		        break;

		    default:
		        break;
		}
	    }
	}

	plevel->move_current = plevel->move_last;
    }

    fclose(level);
    
    return plevel;
}

int level_save(struct level* plevel, char *filename, int partial)
{
    FILE *level;
    char c;
    int x, y;
    int i, j;
    char buffer[256];
    struct move* pmove;
    struct mover* pmover;

    level = fopen(filename, "w");
    if(level == NULL)
	return errno;

    fprintf(level, "chroma level\n\n");

#ifdef XOR_COMPATIBILITY
    if(plevel->mode == MODE_XOR)
        fprintf(level, "mode: xor\n\n");
#endif
#ifdef ENIGMA_COMPATIBILITY
    if(plevel->mode == MODE_ENIGMA)
        fprintf(level, "mode: enigma\n\n");
#endif

    if(plevel->title != NULL && strcmp(plevel->title, "") != 0)
	fprintf(level, "title: %s\n", plevel->title);
    if(plevel->level != 0)
        fprintf(level, "level: %d\n", plevel->level);
    fprintf(level, "size: %d %d\n", plevel->size_x, plevel->size_y);

    if(!partial || plevel->flags & LEVELFLAG_SOLVED)
    {
	fprintf(level, "player: %d\n", plevel->player + 1);
	fprintf(level, "moves: %d\n", plevel->moves);
	fprintf(level, "stars: %d %d %d\n", plevel->stars_caught, plevel->stars_exploded, plevel->stars_total);
	if(plevel->flags & LEVELFLAG_SOLVED)
	    fprintf(level, "solved: 1\n");
	if(plevel->flags & LEVELFLAG_FAILED)
	    fprintf(level, "failed: 1\n");
    }

#ifdef XOR_COMPATIBILITY
    if(plevel->mode == MODE_XOR)
    {
	if(plevel->switched)
	    fprintf(level, "switched: %d\n", plevel->switched);
        fprintf(level, "view1: %d %d\n", plevel->view_x[0], plevel->view_y[0]);
        fprintf(level, "view2: %d %d\n", plevel->view_x[1], plevel->view_y[1]);
        if(plevel->teleport_x[0] != -1)
        {
            fprintf(level, "viewteleport1: %d %d (%d %d)\n", plevel->view_teleport_x[0], plevel->view_teleport_y[0], plevel->teleport_x[0], plevel->teleport_y[0]);
            fprintf(level, "viewteleport2: %d %d (%d %d)\n", plevel->view_teleport_x[1], plevel->view_teleport_y[1], plevel->teleport_x[1], plevel->teleport_y[1]);
        }
	if(plevel->mapped)
	{
	    fprintf(level, "mapped:");
	    if(plevel->mapped & MAPPED_TOP_LEFT)
		fprintf(level, " top_left");
	    if(plevel->mapped & MAPPED_TOP_RIGHT)
		fprintf(level, " top_right");
	    if(plevel->mapped & MAPPED_BOTTOM_LEFT)
		fprintf(level, " bottom_left");
	    if(plevel->mapped & MAPPED_BOTTOM_RIGHT)
		fprintf(level, " bottom_right");
	    fprintf(level, "\n");
	}

    }
#endif

    fprintf(level, "\ndata:\n");

    for(y = 0; y < plevel->size_y; y ++)
    {
	for(x = 0; x < plevel->size_x; x ++)
	{
	    fputc(piecetochar(level_piece(plevel, x, y)), level);
	}
	fputc('\n', level);
    }
    fputc('\n', level);

    if(plevel->move_first != NULL && plevel->moves != 0)
    {
	fprintf(level, "movedata:\n");

	i = 0;
	pmove = plevel->move_first;
	while(pmove != NULL && i < plevel->moves)
	{
	    fputc(directiontochar(pmove->direction), level);
	    i ++;

	    if(i % 78 == 77 && pmove->next != NULL)
		fputc('\n', level);

	    pmove = pmove->next;
	}

	fprintf(level, "\n\n");

        if(!partial)
        {
	    fprintf(level, "undodata:\n");

    	    i = 0; j = 0;
	    pmove = plevel->move_first;
	    while(pmove != NULL && j < plevel->moves)
	    {
	        pmover = pmove->mover_first;
	        while(pmover != NULL)
	        {
		    c = ',';
		    if(pmover->fast == 0)
		        c = ';';
		    if(pmover->next == NULL)
		        c = '.';

		    sprintf(buffer, "%02d:%02d%c%c%c%c", pmover->x, pmover->y, directiontochar(pmover->direction), piecetochar(pmover->piece), piecetochar(pmover->piece_previous), c);
		    if(i + strlen(buffer) > 78)
		    {
		        fprintf(level, "\n");
		        i = 0;
		    }

		    fprintf(level, "%s", buffer);
		    i += strlen(buffer);
		    pmover = pmover->next;
	        }
                j ++;
	        pmove = pmove->next;
	    }

	    fprintf(level, "\n\n");
        }
    }

    fclose(level);

    return 0;
}

void level_addmove(struct level* plevel, int move)
{
    struct move* pmove;
    struct move* ptmp;
    struct mover* pmover;
    struct mover* pmovertmp;

    /* If we are making a move after undoing some moves */
    if(plevel->move_current != plevel->move_last)
    {

	/* Find the first undone move */
	if(plevel->move_current != NULL)
	    pmove = plevel->move_current->next;
	else
	    pmove = plevel->move_first;

	/* Delete all moves that follow it */
	while(pmove != NULL)
	{
	    ptmp = pmove->next;

	    /* Delete movers associated with deleted move */
	    pmover = pmove->mover_first;
	    while(pmover != NULL)
	    {
		pmovertmp = pmover->next;
		free(pmover);
		pmover = pmovertmp;
	    }

	    free(pmove);
	    pmove = ptmp;
	}

	/* Fix up this move so that it appears to be the last */
	if(plevel->move_current != NULL)
	    plevel->move_current->next = NULL;
	else
	    plevel->move_first = NULL;

	plevel->move_last = plevel->move_current;
    }

    /* Create the new move */
    pmove = (struct move*)malloc(sizeof(struct move));
    if(pmove == NULL)
	fatal("Out of memory in level_addmove()");

    pmove->direction = move;
    pmove->previous = plevel->move_last;
    pmove->next = NULL;
    pmove->mover_first = NULL;
    pmove->mover_last = NULL;

    if(plevel->move_first == NULL)
	plevel->move_first = pmove;

    if(plevel->move_last != NULL)
    {
	plevel->move_last->next = pmove;
	pmove->count = plevel->move_last->count + 1;
    }
    else
	pmove->count = 1;

    plevel->move_last = pmove;
    plevel->move_current = pmove;

}

void level_fix(struct level *plevel)
{
    int i, j;
    int teleport;

    for(i = 0; i < 2; i++)
    {
	plevel->alive[i] = 0;
	plevel->teleport_x[i] = -1;
	plevel->teleport_y[i] = -1;
    }

    plevel->stars_caught = 0;
    plevel->stars_exploded = 0;
    plevel->stars_total = 0;

    teleport = 0;

    for(j = 0; j < plevel->size_y; j ++)
    {
	for(i = 0; i < plevel->size_x; i ++)
	{
	    switch(level_piece(plevel, i, j))
	    {
		case PIECE_PLAYER_ONE:
		    plevel->alive[0] = 1;
		    plevel->player_x[0] = i;
		    plevel->player_y[0] = j;
		    break;

		case PIECE_PLAYER_TWO:
		    plevel->alive[1] = 1;
		    plevel->player_x[1] = i;
		    plevel->player_y[1] = j;
		    break;

		case PIECE_STAR:
		    plevel->stars_total ++;
		    break;

#ifdef XOR_COMPATIBILITY
                case PIECE_TELEPORT:
                    if(teleport < 2)
                    {
                        plevel->teleport_x[teleport] = i;
                        plevel->teleport_y[teleport] = j;
                        teleport ++;
                    }
                    break;
#endif
	    }
	}
    }
}

void level_settitle(struct level* plevel, char *title)
{
    if(plevel->title != NULL)
	free(plevel->title);

    if(title == NULL)
    {
	plevel->title = NULL;
	return;
    }

    plevel->title = malloc(strlen(title) + 1);

    if(plevel->title != NULL)
	strcpy(plevel->title, title);
}

