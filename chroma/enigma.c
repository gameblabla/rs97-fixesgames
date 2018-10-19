/*  
    enigma.c

    A reverse engineering of the original Enigma game engine. This maintains
    the usual list of movers, and a separate stack of spaces to be considered.
    In each round, we examine this stack to generate fresh movers, then examine
    those movers to determine whether newly exposed spaces should be added to
    the stack, and also to generate further movers for the next round.

    See levels/regression/enigma-regression.chroma for some subtleties this
    catches that aren't handled correctly by the Chroma game engine. Such
    situations don't seem to occur in the original Enigma levels, however.


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

#include "chroma.h"
#include "level.h"
#include "util.h"

#ifdef ENIGMA_COMPATIBILITY

extern int move_x[];
extern int move_y[];
extern int enigma_move_order[];

int enigma_move(struct level* plevel, int move)
{
    int px, py;
    int dx, dy;
    int p;
    int into;

    if(plevel->alive[plevel->player] == 0)
        return 0;
    if(move == MOVE_SWAP)
	return 0;

    px = plevel->player_x[plevel->player];
    py = plevel->player_y[plevel->player];

    dx = px + move_x[move];
    dy = py + move_y[move];

    /* Can we make the move? */
    p = level_piece(plevel, dx, dy);
    switch(p)
    {
	/* Pieces that can be collected */
	case PIECE_SPACE:
	case PIECE_DOTS:
	case PIECE_DOTS_DOUBLE:
	    break;
	case PIECE_STAR:
            plevel->stars_caught ++;
            plevel->flags |= LEVELFLAG_STARS;
	    break;
	case PIECE_DOOR:
	    if(plevel->stars_caught == plevel->stars_total)
	        plevel->flags |= LEVELFLAG_EXIT;
	    else
		return 0;
	    break;
	/* Pieces that can be pushed */
	case PIECE_ARROW_RED_LEFT:
	case PIECE_ARROW_RED_RIGHT:
	case PIECE_ARROW_RED_UP:
	case PIECE_ARROW_RED_DOWN:
	case PIECE_BOMB_RED_LEFT:
	case PIECE_BOMB_RED_RIGHT:
	case PIECE_BOMB_RED_UP:
	case PIECE_BOMB_RED_DOWN:
	    /* Can't push against gravity */
	    if(((level_piece(plevel, dx, dy) + 2) % 4) == move)
		return 0;
            /* fallthrough */
	case PIECE_CIRCLE:
	case PIECE_CIRCLE_DOUBLE:
	    /* Can't push into other pieces */
	    into = level_piece(plevel, dx + move_x[move], dy + move_y[move]);
	    if(into != PIECE_SPACE && into != PIECE_DOTS)
		return 0;
	    mover_new(plevel, dx + move_x[move], dy + move_y[move], move, p, 0);
	    break;

        /* Can't move */
	default:
	    return 0;
    }

    mover_new(plevel, dx, dy, move, PIECE_PLAYER_ONE + plevel->player, 0);
    mover_new(plevel, px, py, move, PIECE_SPACE, 0);
    level_setmoving(plevel, px, py, MOVE_NONE);
    plevel->player_x[plevel->player] = dx;
    plevel->player_y[plevel->player] = dy;
    mover_addtostack(plevel, px, py, move);

    return 1;
}

int enigma_evolve(struct level* plevel)
{
    struct mover* pmover;
    struct mover* ptmp;

    int into;

    int d;
    int i, p, ep;

    int px, py;
    int dx, dy;

    int ok;

    ok = 0;
    while(!ok)
    {
        /* Examine the stack, and generate movers from it */
        pmover = plevel->stack_first;
        while(pmover != NULL)
        {
	    /* Can anything fall into this space? */
	    for(i = 0; i < 4; i ++)
    	    {
	        d = enigma_move_order[i];
	        px = pmover->x - move_x[d];
	        py = pmover->y - move_y[d];
	        p = level_piece(plevel, px, py);
	        if(p >= PIECE_ARROW_RED_LEFT && p <= PIECE_BOMB_RED_DOWN && (p % 4 == d))
	        {
                    if(level_moving(plevel, px, py) == MOVE_NONE)
		    {
			mover_new(plevel, px, py, d, p, 1);
			i = 4;
		    }
	        }
	    }
	    ptmp = pmover;
	    pmover = pmover->next;
	    free(ptmp);
        }
        plevel->stack_first = NULL;
        plevel->stack_last = NULL;

	/* Examine the movers, adding new movers to a separate list */
        pmover = plevel->mover_first;
        plevel->mover_first = NULL;
        plevel->mover_last = NULL;
        while(pmover != NULL)
        {  
            level_setmoving(plevel, pmover->x, pmover->y, MOVE_NONE);
            level_setprevious(plevel, pmover->x, pmover->y, PIECE_SPACE);
            level_setpreviousmoving(plevel, pmover->x, pmover->y, MOVE_NONE);
            level_setdetonator(plevel, pmover->x, pmover->y, PIECE_SPACE);
            level_setdetonatormoving(plevel, pmover->x, pmover->y, MOVE_NONE);

    	    p = pmover->piece;
	    if(p == PIECE_EXPLOSION_RED_HORIZONTAL || p == PIECE_EXPLOSION_RED_VERTICAL)
	    {
	        mover_new(plevel, pmover->x, pmover->y, MOVE_NONE, PIECE_SPACE, 0);
	        mover_addtostack(plevel, pmover->x, pmover->y, MOVE_NONE);
	        if(p == PIECE_EXPLOSION_RED_HORIZONTAL)
	        {
		    if(pmover->x - 1 > 0)
		    {
	                mover_new(plevel, pmover->x - 1, pmover->y, MOVE_NONE, PIECE_SPACE, 0);
	                mover_addtostack(plevel, pmover->x - 1, pmover->y, MOVE_NONE);
		    }
		    if(pmover->x + 1 < plevel->size_x - 1)
		    {
	                mover_new(plevel, pmover->x + 1, pmover->y, MOVE_NONE, PIECE_SPACE, 0);
	                mover_addtostack(plevel, pmover->x + 1, pmover->y, MOVE_NONE);
		    }
	        }
                else
	        {
		    if(pmover->y - 1 > 0)
		    {
	                mover_new(plevel, pmover->x, pmover->y - 1, MOVE_NONE, PIECE_SPACE, 0);
	                mover_addtostack(plevel, pmover->x, pmover->y - 1, MOVE_NONE);
		    }
		    if(pmover->y + 1 < plevel->size_y - 1)
		    {
	                mover_new(plevel, pmover->x, pmover->y + 1, MOVE_NONE, PIECE_SPACE, 0);
	                mover_addtostack(plevel, pmover->x, pmover->y + 1, MOVE_NONE);
		    }
	        }
	    }
	    if((p >= PIECE_ARROW_RED_LEFT && p <= PIECE_BOMB_RED_DOWN) || p == PIECE_CIRCLE)
	    {
		if(p == PIECE_CIRCLE)
		    d = pmover->direction;
		else
	            d = p % 4;
	        dx = pmover->x + move_x[d];
	        dy = pmover->y + move_y[d];

	        into = level_piece(plevel, dx, dy);
		/* Can it detonate something? */
	        if(p >= PIECE_ARROW_RED_LEFT && p <= PIECE_ARROW_RED_DOWN && into >= PIECE_BOMB_RED_LEFT && into <= PIECE_BOMB_RED_DOWN && pmover->fast && level_moving(plevel, dx, dy) == MOVE_NONE)
	        {
		    /* Add the central explosion to the stack */
		    mover_new(plevel, pmover->x, pmover->y, d, PIECE_SPACE, 0);
		    level_setprevious(plevel, dx, dy, into);
		    level_setdetonator(plevel, dx, dy, p);
		    level_setdetonatormoving(plevel, dx, dy, d);
		    mover_addtostack(plevel, pmover->x, pmover->y, MOVE_NONE);

		    /* Generate cosmetic side explosions */
		    if(into % 2)
	   	    {
                        if(dx - 1 > 0)
			{
                            ep = level_piece(plevel, dx - 1, dy);
                            if(ep == PIECE_STAR)
                            {
                                plevel->stars_exploded ++;
                                plevel->flags |= LEVELFLAG_STARS;
                            }
                            level_setmoving(plevel, dx - 1, dy, MOVE_NONE);
		            mover_new(plevel, dx - 1, dy, level_moving(plevel, dx - 1, dy), PIECE_EXPLOSION_RED_LEFT, 1);
			    level_setprevious(plevel, dx - 1, dy, ep);
			}
 		        if(dx + 1 < plevel->size_x - 1)
			{
			    ep = level_piece(plevel, dx + 1, dy);
			    if(ep == PIECE_STAR)
			    {
                                plevel->stars_exploded ++;
                                plevel->flags |= LEVELFLAG_STARS;
			    }
                            level_setmoving(plevel, dx + 1, dy, MOVE_NONE);
		            mover_new(plevel, dx + 1, dy, level_moving(plevel, dx + 1, dy), PIECE_EXPLOSION_RED_RIGHT, 0);
			    level_setprevious(plevel, dx + 1, dy, ep);
			}
		        mover_new(plevel, dx, dy, MOVE_NONE, PIECE_EXPLOSION_RED_HORIZONTAL, 0);
		    }
		    else
		    {
                        if(dy - 1 > 0)
                        {
                            ep = level_piece(plevel, dx, dy - 1);
                            if(ep == PIECE_STAR)
                            {
                                plevel->stars_exploded ++;
                                plevel->flags |= LEVELFLAG_STARS;
                            }
                            level_setmoving(plevel, dx, dy - 1, MOVE_NONE);
		            mover_new(plevel, dx, dy - 1, level_moving(plevel, dx, dy - 1), PIECE_EXPLOSION_RED_TOP, 0);
                            level_setprevious(plevel, dx, dy - 1, ep);
                        }
 		        if(dy + 1 < plevel->size_y - 1)
                        {
                            ep = level_piece(plevel, dx, dy + 1);
                            if(ep == PIECE_STAR)
                            {
                                plevel->stars_exploded ++;
                                plevel->flags |= LEVELFLAG_STARS;
                            }
                            level_setmoving(plevel, dx, dy + 1, MOVE_NONE);
		            mover_new(plevel, dx, dy + 1, level_moving(plevel, dx, dy + 1), PIECE_EXPLOSION_RED_BOTTOM, 0);
                            level_setprevious(plevel, dx, dy + 1, ep);
                        }
		        mover_new(plevel, dx, dy, MOVE_NONE, PIECE_EXPLOSION_RED_VERTICAL, 0);
		    }
	        }
		/* Can it keep moving? */
	        else if(into == PIECE_SPACE || ((into == PIECE_DOTS || into == PIECE_PLAYER_ONE) && p != PIECE_CIRCLE && pmover->fast == 1))
	        {
		    mover_new(plevel, dx, dy, d, p, 1);
		    mover_new(plevel, pmover->x, pmover->y, d, PIECE_SPACE, 0);
		    level_setmoving(plevel, pmover->x, pmover->y, MOVE_NONE);
		    mover_addtostack(plevel, pmover->x, pmover->y, MOVE_NONE);
	        }
	    }

            ptmp = pmover->next;
            free(pmover);
            pmover = ptmp;
        }

        if(plevel->mover_first != NULL || plevel->stack_first == NULL)
	    ok = 1;
    }

    /* Is player one still alive? */
    if(level_piece(plevel, plevel->player_x[0], plevel->player_y[0]) != PIECE_PLAYER_ONE)
	plevel->alive[0] = 0;

    return 0;
}

#endif 
