/*  
    engine.c

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

#ifdef XOR_COMPATIBILITY
int xor_move(struct level* plevel, int move);
int xor_evolve(struct level* plevel);
void xor_focus(struct level* plevel);
extern int options_xor_mode;
#endif
#ifdef ENIGMA_COMPATIBILITY
int enigma_move(struct level* plevel, int move);
int enigma_evolve(struct level* plevel);
extern int options_enigma_mode;
#endif

extern int options_debug;
extern char* piece_name[];

/*               l  u  r  d  n  s  w   */
int move_x[] = {-1, 0, 1, 0, 0, 0, 0};
int move_y[] = {0, -1, 0, 1, 0, 0, 0};

#ifdef ENIGMA_COMPATIBILITY
int enigma_move_order[] = {MOVE_DOWN, MOVE_RIGHT, MOVE_LEFT, MOVE_UP};
#endif
#ifdef XOR_COMPATIBILITY
int xor_teleport_order[] = {MOVE_RIGHT, MOVE_UP, MOVE_LEFT, MOVE_DOWN};
#endif

struct mover* mover_new(struct level* plevel, int x, int y, int d, int piece, int fast);
void mover_consider(struct level* plevel, int x, int y, int d);
struct mover* mover_explode(struct level *plevel, int x, int y, int d, int p);
void explode_sides(struct level* plevel, int x, int y, int p, int d);
int canfall(int p, int into, int d);
int canmove(int p, int into, int d, int fast);
int canexplode(int p, int into, int d, int fast, int mode);
int canbepushed(int p, int into, int d, int mode);

int explosiontype(int p)
{
    switch(p)
    {
	case PIECE_ARROW_RED_LEFT:
	case PIECE_ARROW_RED_RIGHT:
	case PIECE_BOMB_RED_LEFT:
	case PIECE_BOMB_RED_RIGHT:
	    return PIECE_EXPLOSION_NEW_RED_VERTICAL;
	case PIECE_ARROW_RED_UP:
	case PIECE_ARROW_RED_DOWN:
	case PIECE_BOMB_RED_UP:
	case PIECE_BOMB_RED_DOWN:
	    return PIECE_EXPLOSION_NEW_RED_HORIZONTAL;

	case PIECE_ARROW_GREEN_LEFT:
	case PIECE_ARROW_GREEN_RIGHT:
	case PIECE_BOMB_GREEN_LEFT:
	case PIECE_BOMB_GREEN_RIGHT:
	    return PIECE_EXPLOSION_NEW_GREEN_VERTICAL;
	case PIECE_ARROW_GREEN_UP:
	case PIECE_ARROW_GREEN_DOWN:
	case PIECE_BOMB_GREEN_UP:
	case PIECE_BOMB_GREEN_DOWN:
	    return PIECE_EXPLOSION_NEW_GREEN_HORIZONTAL;

	case PIECE_ARROW_BLUE_LEFT:
	case PIECE_ARROW_BLUE_RIGHT:
	case PIECE_BOMB_BLUE_LEFT:
	case PIECE_BOMB_BLUE_RIGHT:
	    return PIECE_EXPLOSION_NEW_BLUE_VERTICAL;
	case PIECE_ARROW_BLUE_UP:
	case PIECE_ARROW_BLUE_DOWN:
	case PIECE_BOMB_BLUE_UP:
	case PIECE_BOMB_BLUE_DOWN:
	    return PIECE_EXPLOSION_NEW_BLUE_HORIZONTAL;

	default:
	    /* This should never happen */
	    return PIECE_GONE;
    }
}

void level_moved(struct level* plevel, int move)
{
    if(move != MOVE_REDO)
        level_addmove(plevel, move);
    else
    {
	if(plevel->move_current != NULL)
	    plevel->move_current = plevel->move_current->next;
	else
	    plevel->move_current = plevel->move_first;
    }

    plevel->moves ++;
    plevel->flags |= LEVELFLAG_MOVES;

    level_storemovers(plevel);
}

int level_move(struct level* plevel, int move)
{
    int x, y;
    int tx, ty;
    int px, py;
    int p;
    int ok;
#ifdef XOR_COMPATIBILITY
    int dx, dy;
    int teleport;
    int td;
    int i;
#endif
    int realmove;
    struct move* pmove;

    realmove = move;

    if(plevel == NULL || plevel->mover_first != NULL)
	return 0;

    if(move == MOVE_REDO)
    {
	if(plevel->move_current == NULL)
	    pmove = plevel->move_first;
	else
	    pmove = plevel->move_current->next;

	if(pmove == NULL)
	    return 0;

	move = pmove->direction;
    }

    if(move == MOVE_SWAP)
    {
	if(plevel->alive[1 - plevel->player])
        {
	    plevel->player = 1 - plevel->player;

            /* Create new movers for the stationary swapped players to allow
               the display to redraw them after the swap. */
            mover_new(plevel, plevel->player_x[plevel->player], plevel->player_y[plevel->player], MOVE_SWAP, PIECE_PLAYER_ONE +  plevel->player, 0);

	    /* Is the first player still alive? */
	    if(plevel->alive[1 - plevel->player])
                mover_new(plevel, plevel->player_x[1 - plevel->player], plevel->player_y[1 - plevel->player], MOVE_SWAPPED, PIECE_PLAYER_ONE + 1 - plevel->player, 0);

            level_moved(plevel, realmove);

            return 1;
	}
	return 0;
    }

    if(plevel->alive[plevel->player] == 0)
	return 0;

#ifdef XOR_COMPATIBILITY
    if(plevel->mode == MODE_XOR && options_xor_mode == 1)
    {
	if(xor_move(plevel, move))
	{
            level_moved(plevel, realmove);
            xor_focus(plevel);
	    return 1;
	}
	return 0;
    }
#endif
#ifdef ENIGMA_COMPATIBILITY
    if(plevel->mode == MODE_ENIGMA && options_enigma_mode == 1)
    {  
        if(enigma_move(plevel, move))
        {  
            level_moved(plevel, realmove);
            return 1;
        }
        return 0;
    }

#endif

    /* Consider where we are moving to */
    x = plevel->player_x[plevel->player] + move_x[move];
    y = plevel->player_y[plevel->player] + move_y[move];

    p = level_piece(plevel, x, y);

    ok = 0;

    /* Can we move into the piece in that direction? */
    switch(p)
    {
	case PIECE_DOOR:
            if(plevel->stars_caught == plevel->stars_total)
    	    {
	        plevel->flags |= LEVELFLAG_EXIT;
	        ok = 1;
	    }
            break;

        case PIECE_STAR:
            plevel->stars_caught ++;
            plevel->flags |= LEVELFLAG_STARS;
            ok = 1;
            break;

#ifdef XOR_COMPATIBILITY
        case PIECE_TELEPORT:
	    /* Only XOR has teleports. We force the issue so as not to break
	       Chroma's rotational symmetry by introducing teleport order. */
	    if(plevel->mode != MODE_XOR)
		break;

            teleport = -1;
            if(x == plevel->teleport_x[0] && y == plevel->teleport_y[0])
		teleport = 0;
	    if(x == plevel->teleport_x[1] && y == plevel->teleport_y[1])
		teleport = 1;
	    if(teleport != -1)
	    {
	        tx = plevel->teleport_x[1 - teleport];
	        ty = plevel->teleport_y[1 - teleport];
		td = move;

		/* Does the other teleport still exist? */
	        if(level_piece(plevel, tx, ty) == PIECE_TELEPORT)
	        {
		    ok = 0;
		    /* Find the first available exit from it */
		    for(i = 0; i < 4; i ++)
		    {
			dx = tx + move_x[xor_teleport_order[i]];
			dy = ty + move_y[xor_teleport_order[i]];
			if(!ok && level_piece(plevel, dx, dy) == PIECE_SPACE)
			{
			    /* Change move to produce the effect of coming
			       out of the teleport */
			    x = dx; y = dy; move = xor_teleport_order[i];
			    ok = 1;
			}
		    }

		    if(ok)
		    {
			/* Visual effects for the player going in one teleport */
			/* Store original player move direction in cosmetic mover */
                        mover_new(plevel, plevel->teleport_x[teleport], plevel->teleport_y[teleport], td, PIECE_TELEPORT, 0);
		        level_setprevious(plevel, plevel->teleport_x[teleport], plevel->teleport_y[teleport], PIECE_PLAYER_ONE + plevel->player);
		        level_setpreviousmoving(plevel, plevel->teleport_x[teleport], plevel->teleport_y[teleport], realmove);
			/* and out of the other teleport */
                        mover_new(plevel, plevel->teleport_x[1 - teleport], plevel->teleport_y[1 - teleport], MOVE_NONE, PIECE_TELEPORT, 0);

			/* Change the viewpoint to that of the other teleport */
                        plevel->view_x[plevel->player] = plevel->view_teleport_x[1 - teleport];
                        plevel->view_y[plevel->player] = plevel->view_teleport_y[1 - teleport];

		    }
	        }
	    }
            break;

        case PIECE_SWITCH:
            plevel->switched = 1 - plevel->switched;
	    plevel->flags |= LEVELFLAG_SWITCH;
            ok = 1;
	    break;

        case PIECE_MAP_TOP_LEFT:
	    plevel->mapped |= MAPPED_TOP_LEFT;
            plevel->flags |= LEVELFLAG_MAP;
            ok = 1;
            break;
        case PIECE_MAP_TOP_RIGHT:
	    plevel->mapped |= MAPPED_TOP_RIGHT;
            plevel->flags |= LEVELFLAG_MAP;
            ok = 1;
            break;
        case PIECE_MAP_BOTTOM_LEFT:
	    plevel->mapped |= MAPPED_BOTTOM_LEFT;
            plevel->flags |= LEVELFLAG_MAP;
            ok = 1;
            break;
        case PIECE_MAP_BOTTOM_RIGHT:
	    plevel->mapped |= MAPPED_BOTTOM_RIGHT;
            plevel->flags |= LEVELFLAG_MAP;
            ok = 1;
            break;

        case PIECE_DOTS_X:
            if(move == MOVE_LEFT || move == MOVE_RIGHT)
		ok = 1;
            break;

        case PIECE_DOTS_Y:
            if(move == MOVE_UP || move == MOVE_DOWN)
		ok = 1;
            break;
#endif

#ifdef ENIGMA_COMPATIBILITY
        case PIECE_DOTS_DOUBLE:
#endif
        case PIECE_DOTS:
        case PIECE_SPACE:
            ok = 1;
            break;
    }

    /* Is there a piece we can push? */
    if(!ok)
    {
	tx = x + move_x[move];
	ty = y + move_y[move];

	if(canbepushed(p, level_piece(plevel, tx, ty), move, plevel->mode))
	{
            mover_new(plevel, tx, ty, move, p, 0);
	    ok = 1;
	}
    }

    if(ok)
    {

	/* Cosmetic mover for storing the player's direction in undo */
	mover_new(plevel, plevel->player_x[plevel->player], plevel->player_y[plevel->player], move, PIECE_GONE, 0);

        mover_new(plevel, x, y, move, PIECE_PLAYER_ONE + plevel->player, 0);

	px = plevel->player_x[plevel->player];
	py = plevel->player_y[plevel->player];

#ifdef XOR_COMPATIBILITY
        /* XOR protects the players move */
        if(plevel->mode == MODE_XOR)
	{
	    /* Blank the player's space first to avoid upsetting undo */
	    level_setpiece(plevel, px, py, PIECE_SPACE);
            mover_new(plevel, px, py, (move + 1) % 4, PIECE_SPACE, 1);
	}
	/* Chroma lets a piece follow in the player's trail */
	else
	{
#endif
	    /* Blank the player's space first to avoid upsetting undo */
	    level_setpiece(plevel, px, py, PIECE_SPACE);
	    mover_consider(plevel, px, py, move % 4);
#ifdef XOR_COMPATIBILITY
	}
#endif

        plevel->player_x[plevel->player] = x;
        plevel->player_y[plevel->player] = y;

        level_moved(plevel, realmove);

#ifdef XOR_COMPATIBILITY
        if(plevel->mode == MODE_XOR)
            xor_focus(plevel);
#endif

        return 1;
    }

    return 0;
}

struct mover* mover_explode(struct level *plevel, int x, int y, int d, int p)
{
    /* Don't explode any of the edge wall */
    if(x == 0 || y == 0 || x == plevel->size_x - 1 || y == plevel->size_y - 1)
        return NULL;

    /* What have we exploded? */
    switch(level_piece(plevel, x, y))
    {
	case PIECE_STAR:
	    plevel->stars_exploded ++;
	    plevel->flags |= LEVELFLAG_STARS;
	    break;

#ifdef XOR_COMPATIBILITY
	case PIECE_SWITCH:
	    plevel->switched = 1 - plevel->switched;
	    plevel->flags |= LEVELFLAG_SWITCH;
	    break;
#endif
    }

    return mover_new(plevel, x, y, d, p, 1);
}

struct mover* mover_new(struct level* plevel, int x, int y, int d, int piece, int fast)
{
    struct mover* pmover;
    int previous;
    int data;

    /* Don't allow two movers in the same space, unless one is exploding */
    if(!isnewexplosion(piece) && level_moving(plevel, x, y) != MOVE_NONE)
	return NULL;

    pmover = (struct mover*)malloc(sizeof(struct mover));
    if(pmover == NULL)
	fatal("Out of memory in mover_new()");

    previous = level_piece(plevel, x, y);

    pmover->x = x;
    pmover->y = y;
    pmover->direction = d;
    pmover->piece = piece;
    pmover->piece_previous = previous;
    pmover->fast = fast;
    pmover->next = NULL;
    pmover->previous = plevel->mover_last;

    if(plevel->mover_first == NULL)
        plevel->mover_first = pmover; 

    if(plevel->mover_last != NULL)
        plevel->mover_last->next = pmover; 

    plevel->mover_last = pmover;

    /* Show pieces collected by players */
    if(piece == PIECE_PLAYER_ONE || piece == PIECE_PLAYER_TWO)
    {
	if((previous < PIECE_MOVERS_FIRST || previous > PIECE_MOVERS_LAST)
                && previous != PIECE_CIRCLE
#ifdef ENIGMA_COMPATIBILITY
                && previous != PIECE_CIRCLE_DOUBLE
#endif
                && previous != PIECE_PLAYER_ONE
                && previous != PIECE_PLAYER_TWO)
	    level_setprevious(plevel, x, y, previous);
    }

    /* Show players squashed by movers */
    if(piece >= PIECE_MOVERS_FIRST && piece <= PIECE_MOVERS_LAST)
    {
	if(previous == PIECE_PLAYER_ONE || previous == PIECE_PLAYER_TWO)
           level_setprevious(plevel, x, y, previous);
    }

    /* Show pieces removed by movers or explosions */
    if(previous == PIECE_DOTS
#ifdef ENIGMA_COMPATIBILITY
            || previous == PIECE_DOTS_DOUBLE
#endif
#ifdef XOR_COMPATIBILITY
            || previous == PIECE_DOTS_X
            || previous == PIECE_DOTS_Y
#endif
            || isexplosion(previous))
        level_setprevious(plevel, x, y, previous);

    /* Show exploded pieces */
    if(isnewexplosion(piece) && !isnewexplosion(previous))
    {
        level_setprevious(plevel, x, y, previous);
        level_setpreviousmoving(plevel, x, y, level_moving(plevel, x, y));
    }

    /* Explosions occur later */
    if(!isnewexplosion(piece) && piece != PIECE_GONE)
    {
        level_setpiece(plevel, x, y, piece);
        level_setmoving(plevel, x, y, d);
    }

    /* Maintain piece graphic */
    if(d != MOVE_NONE)
    {
	data = level_data(plevel, x - move_x[d], y - move_y[d]) & 0xff00;
	data = (level_data(plevel, x, y) & ~0xff00) | data;
	level_setdata(plevel, x, y, data);
    }

    return pmover;
}

struct mover* mover_addtostack(struct level* plevel, int x, int y, int move)
{
    struct mover* pmover;

    pmover = (struct mover*)malloc(sizeof(struct mover));
    if(pmover == NULL)
        fatal("Out of memory in mover_addtostack()");

    pmover->x = x;
    pmover->y = y;
    pmover->direction = move;
    pmover->piece = PIECE_SPACE;
    pmover->fast = 0;
    pmover->next = NULL;

    if(plevel->stack_first == NULL)
        plevel->stack_first = pmover;

    if(plevel->stack_last != NULL)
        plevel->stack_last->next = pmover;

    plevel->stack_last = pmover;

    return pmover;
}

void level_storemovers(struct level* plevel)
{
    struct mover* pmover;
    int previous;

    int count = 0;

    if(plevel->move_current == NULL || plevel->mover_first == NULL)
	return;

    if((options_debug & DEBUG_MOVERS) && plevel->move_current->mover_first == NULL)
	fprintf(stderr, "\n");

    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
	/* If something is moving into an explosion, don't store it as the
	   previous piece for this space; it will have its own mover, and thus
	   will be stored elsewhere. */
	previous = pmover->piece_previous;
	if(isexplosion(pmover->piece) && level_previousmoving(plevel, pmover->x, pmover->y) != MOVE_NONE)
	    previous = PIECE_SPACE;

	mover_newundo(plevel, pmover->x, pmover->y,
		pmover->direction, pmover->piece, previous,
		MOVER_STORE | (pmover->next == NULL ? 0 : MOVER_FAST));

	if(options_debug & DEBUG_MOVERS)
	    fprintf(stderr, "[%d] Storing undo mover at (%d,%d) is %s was %s (direction=%c) (flags=%d)\n", 
		count ++, pmover->x, pmover->y,
		piece_name[pmover->piece], piece_name[previous],
		directiontochar(pmover->direction),
		(pmover->next == NULL ? 0 : MOVER_FAST));

	pmover = pmover->next;
    }
}

int level_evolve(struct level* plevel)
{
    struct mover* poldmovers;
    struct mover* pmover;
    int x, y;
    int i;
    int d;
    int ad;
    int ed;
    int ax, ay;
    int bp, bd;
    int filled;

#ifdef XOR_COMPATIBILITY
    if(plevel->mode == MODE_XOR && options_xor_mode == 1)
    {
	return xor_evolve(plevel);
    }
#endif
#ifdef ENIGMA_COMPATIBILITY
    if(plevel->mode == MODE_ENIGMA && options_enigma_mode == 1)
    {  
        return enigma_evolve(plevel);
    }
#endif

    poldmovers = plevel->mover_first;

    plevel->mover_first = NULL;
    plevel->mover_last = NULL;

    /* Chroma's engine isn't perfect. Pieces that appear to be in continuous
       motion are actually momentarily stationary at the start of every cycle.
       In pathological cases, this can give rise to some counterintuitive
       situations, where the outcome depends on the order of the movers.

       See levels/regression/chroma-regression.chroma for some examples.
    */

    pmover = poldmovers;
    while(pmover != NULL)
    {
        level_setmoving(plevel, pmover->x, pmover->y, MOVE_NONE);
        level_setprevious(plevel, pmover->x, pmover->y, PIECE_SPACE);
        level_setpreviousmoving(plevel, pmover->x, pmover->y, MOVE_NONE);
        level_setdetonator(plevel, pmover->x, pmover->y, PIECE_SPACE);
        level_setdetonatormoving(plevel, pmover->x, pmover->y, MOVE_NONE);
        pmover = pmover->next;
    }

    pmover = poldmovers;
    while(pmover != NULL)
    {

	/* Remove the mover if something has already moved into its space */
	if(level_moving(plevel, pmover->x, pmover->y) != MOVE_NONE
		/* or it isn't what it should be */
		|| level_piece(plevel, pmover->x, pmover->y) != pmover->piece
	  )
	    pmover->piece = PIECE_GONE;

	switch(pmover->piece)
	{
	    case PIECE_SPACE:
	    case PIECE_EXPLOSION_RED_LEFT:
            case PIECE_EXPLOSION_RED_HORIZONTAL:
            case PIECE_EXPLOSION_RED_RIGHT:
            case PIECE_EXPLOSION_RED_TOP:
            case PIECE_EXPLOSION_RED_VERTICAL:
            case PIECE_EXPLOSION_RED_BOTTOM:
            case PIECE_EXPLOSION_GREEN_LEFT:
            case PIECE_EXPLOSION_GREEN_HORIZONTAL:
            case PIECE_EXPLOSION_GREEN_RIGHT:
            case PIECE_EXPLOSION_GREEN_TOP:
            case PIECE_EXPLOSION_GREEN_VERTICAL:
            case PIECE_EXPLOSION_GREEN_BOTTOM:
            case PIECE_EXPLOSION_BLUE_LEFT:
            case PIECE_EXPLOSION_BLUE_HORIZONTAL:
            case PIECE_EXPLOSION_BLUE_RIGHT:
            case PIECE_EXPLOSION_BLUE_TOP:
            case PIECE_EXPLOSION_BLUE_VERTICAL:
            case PIECE_EXPLOSION_BLUE_BOTTOM:
		i = 0;
		filled = 0;

		/* Consider the pieces around the space */
		for(i = 0; i < 4; i ++)
		{
		    if(filled)
			continue;

#ifdef ENIGMA_COMPATIBILITY
		    /* Enigma has a fixed move order */
                    if(plevel->mode == MODE_ENIGMA)
                        d = enigma_move_order[i];
                    else
#endif
			/* Chroma and XOR depend on how the space was emptied */
                        d = (pmover->direction + i) % 4;

                    ad = (d + 2) % 4;
                    ax = pmover->x + move_x[ad];
                    ay = pmover->y + move_y[ad];

		    /* Can the piece move into the space? */
		    if(canfall(level_piece(plevel, ax, ay), PIECE_SPACE, d)
			    /* and that piece isn't already moving */
			    && level_moving(plevel, ax, ay) == MOVE_NONE
		      )
		    {
			x = pmover->x + move_x[d];
			y = pmover->y + move_y[d];

			/* Can the piece from the opposite direction also
			   move into this space? */
			if(canfall(level_piece(plevel, x, y), PIECE_SPACE, ad)
				/* and that piece isn't already moving */
				&& level_moving(plevel, x, y) == MOVE_NONE
				/* If so, can the two explode? */
				&& canexplode(level_piece(plevel, ax, ay), level_piece(plevel, x, y), d, 1, plevel->mode)
				/* (but not for XOR and Enigma) */
				&& plevel->mode == MODE_CHROMA
			  )
			{
			    /* If so, detonate them in the middle */
			    if((level_piece(plevel, x, y) & 4) == 4)
			    {
				/* The first piece is the bomb */
				bp = level_piece(plevel, x, y);
				bd = ad;
                                ed = level_piece(plevel, x, y) & 3;

                                level_setdetonator(plevel, pmover->x, pmover->y, level_piece(plevel, ax, ay));
                                level_setdetonatormoving(plevel, pmover->x, pmover->y, d);
			    }
			    else
			    {
				/* The second piece is the bomb */
				bp = level_piece(plevel, ax, ay);
				bd = d;
                                ed = level_piece(plevel, ax, ay) & 3;

                                level_setdetonator(plevel, pmover->x, pmover->y, level_piece(plevel, x, y));
                                level_setdetonatormoving(plevel, pmover->x, pmover->y, ad);
			    }

			    /* and consider anything following them */
                            mover_consider(plevel, x, y, ad);
                            mover_consider(plevel, ax, ay, d);

			    /* Move the bomb into the space */
                            level_setpiece(plevel, pmover->x, pmover->y, bp);
                            level_setmoving(plevel, pmover->x, pmover->y, bd);

			    /* and explode it */
                            mover_explode(plevel, pmover->x, pmover->y, ed, explosiontype(bp));

			    /* Create the central explosion now, to prevent the
			       piece there being processed as a later mover. */
			    level_setpiece(plevel, pmover->x, pmover->y, explosiontype(bp));

                            explode_sides(plevel, pmover->x, pmover->y, bp, ed);

			    filled = 1;
			    break;
			}

			/* Otherwise, keep the piece moving */
			mover_new(plevel, pmover->x, pmover->y, d, level_piece(plevel, ax, ay), 1);
			/* and see if anything is following in its trail */
			mover_consider(plevel, ax, ay, d);

			filled = 1;
			break;
                    }
		}

		/* If the explosion has not been filled */
		if(isexplosion(pmover->piece) && filled == 0
			/* and nothing else is moving into it */
			&& level_moving(plevel, pmover->x, pmover->y) == MOVE_NONE
		  )
		    /* then turn it into a space */
		    mover_new(plevel, pmover->x, pmover->y, pmover->direction, PIECE_SPACE, 0);

		break;

	    case PIECE_PLAYER_ONE:
	    case PIECE_PLAYER_TWO:
            case PIECE_GONE:
		/* These 'movers' are purely for cosmetic purposes */
                break;

#ifdef XOR_COMPATIBILITY
            case PIECE_TELEPORT:
		/* These 'movers' are purely for cosmetic purposes */
                break;
#endif

            default:
	        /* A pushed arrow still falls in its natural direction */
    	        if(pmover->fast == 0 && pmover->piece >= PIECE_MOVERS_FIRST && pmover->piece <= PIECE_MOVERS_LAST)
                    pmover->direction = pmover->piece % 4; 

		/* Consider the space in front of the mover */
		x = pmover->x + move_x[pmover->direction];
		y = pmover->y + move_y[pmover->direction];

		/* Can the mover move into the space in front of it? */
		if(canmove(pmover->piece, level_piece(plevel, x, y), pmover->direction, pmover->fast)
			/* and that space doesn't already have something
			   moving into it */
			&& (level_moving(plevel, x, y) == MOVE_NONE)
		  )
		{
		    /* If so, keep it moving */
                    mover_new(plevel, x, y, pmover->direction, pmover->piece, 1);
		    /* and see if anything is following in its trail */
		    mover_consider(plevel, pmover->x, pmover->y, pmover->direction);
		    break;
		}

		/* Can the mover explode the piece in front of it? */
                if(canexplode(pmover->piece, level_piece(plevel, x, y), pmover->direction, pmover->fast, plevel->mode)
                        /* and the piece in front isn't moving */
                        && (level_moving(plevel, x, y) == MOVE_NONE
                        /* or it is moving towards us */
                        || (level_moving(plevel, x, y) == ((pmover->direction + 2) % 4)
                            /* (but not for XOR or Enigma) */
                            && plevel->mode==MODE_CHROMA))
                  ) 
		{
                    bp = level_piece(plevel, x, y);   
                    level_setdetonator(plevel, x, y, pmover->piece);
                    level_setdetonatormoving(plevel, x, y, pmover->direction);

                    /* Explosion direction is bomb fall direction */
                    if(bp & 4)
                        ed = bp & 3;
                    else
                        ed = pmover->piece & 3;

		    mover_explode(plevel, x, y, ed, explosiontype(bp));

		    /* Create the central explosion now, to prevent the piece
		       there being processed as a later mover. */
		    level_setpiece(plevel, x, y, explosiontype(bp));

                    mover_consider(plevel, pmover->x, pmover->y, pmover->direction);

		    explode_sides(plevel, x, y, bp, ed);

		    break;
		}
	}

	pmover = pmover->next;
    }

    /* Create the side explosions at the end, rather than during the previous
       loop. This allows multiple explosions to occur in parallel. Centre
       explosions will have already been created earlier on. */
    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
        if(isnewexplosion(pmover->piece))
        {   
	    if(!isnewexplosion(level_piece(plevel, pmover->x, pmover->y)))
	    {
		level_setprevious(plevel, pmover->x, pmover->y, level_piece(plevel, pmover->x, pmover->y));
		level_setpreviousmoving(plevel, pmover->x, pmover->y, level_moving(plevel, pmover->x, pmover->y));
	    }

	    /* Use PIECE_EXPLOSION_NEW to allow detection of overlapping
	       explosions further down. */
            level_setpiece(plevel, pmover->x, pmover->y, pmover->piece);
	    level_setmoving(plevel, pmover->x, pmover->y, pmover->direction);

            pmover->piece += PIECE_EXPLOSION_FIRST - PIECE_EXPLOSION_NEW_FIRST;
	}

        pmover = pmover->next;
    }

    pmover = plevel->mover_first;
    while(pmover != NULL)
    {   
	if(isexplosion(pmover->piece))
	{
	    /* Remove any explosions that overlap other explosions */
	    if(isexplosion(level_piece(plevel, pmover->x, pmover->y)))
		pmover->piece = PIECE_GONE;
	    /* Otherwise, convert new explosions into explosions proper */
	    else
	        level_setpiece(plevel, pmover->x, pmover->y, pmover->piece);
	}
	/* Remove any movers that have exploded, or aren't as they should be */
	if(level_piece(plevel, pmover->x, pmover->y) != pmover->piece)
        {
            pmover->piece = PIECE_GONE;
        }
        pmover = pmover->next;
    }

    /* Is player one still alive? */
    if(level_piece(plevel, plevel->player_x[0], plevel->player_y[0]) != PIECE_PLAYER_ONE)
    {
        plevel->flags |= LEVELFLAG_MOVES;
       	plevel->alive[0] = 0; 
    }

    /* Is player two still alive? */
    if(level_piece(plevel, plevel->player_x[1], plevel->player_y[1]) != PIECE_PLAYER_TWO)
    {
        plevel->flags |= LEVELFLAG_MOVES;
	plevel->alive[1] = 0; 
    }

    /* Free old movers */
    while(poldmovers != NULL)
    {
	pmover = poldmovers;
	poldmovers = poldmovers->next;
	free(pmover);
    }

    return 0;
}

void mover_consider(struct level* plevel, int x, int y, int d)
{
    int tx, ty;
    int ad;

    /* Is there already a mover in this space? If so, don't allow another */
    if(level_moving(plevel, x, y) != MOVE_NONE)
        return;

#ifdef ENIGMA_COMPATIBILITY
    /* Enigma doesn't consider the direction in which a space was emptied */
    if(plevel->mode == MODE_ENIGMA)
    {
        mover_new(plevel, x, y, d, PIECE_SPACE, 1);
        return;
    }
#endif

#ifdef XOR_COMPATIBILITY
    if(plevel->mode == MODE_XOR)
    {
        mover_new(plevel, x, y, d, PIECE_SPACE, 1);
        return;
    }
#endif

    ad = (d + 2) % 4;
    tx = x + move_x[ad];
    ty = y + move_y[ad];

    /* Can a piece follow in the trail of this one? */
    if(canfall(level_piece(plevel, tx, ty), PIECE_SPACE, d))
    {
        /* If it's moving already, just clear this space (1.07) */
        if(level_moving(plevel, tx, ty) != MOVE_NONE)
        {
            mover_new(plevel, x, y, MOVE_NONE, PIECE_SPACE, 0);
            return;
        }

	/* Otherwise, set it moving */
        mover_new(plevel, x, y, d, level_piece(plevel, tx, ty), 1);
	/* and see if there's anything following in its trail */
        mover_consider(plevel, tx, ty, d);
        return;
    }

    mover_new(plevel, x, y, d, PIECE_SPACE, 1);
}

void explode_sides(struct level* plevel, int x, int y, int p, int d)
{
    /* Chroma is subtle. This may be too subtle to have any effect in practice,
       but the principle elsewhere is that things should be rotationally
       symmetric, and this carries through here. */
    if(plevel->mode == MODE_CHROMA)
    {
        switch(p % 4)
        {   
            case 0: /* left */
                mover_explode(plevel, x, y - 1, d, explosiontype(p) - 1);
                mover_explode(plevel, x, y + 1, d, explosiontype(p) + 1);
                break;

            case 1: /* up */
                mover_explode(plevel, x + 1, y, d, explosiontype(p) + 1);
                mover_explode(plevel, x - 1, y, d, explosiontype(p) - 1);
                break;

            case 2: /* right */
                mover_explode(plevel, x, y + 1, d, explosiontype(p) + 1);
                mover_explode(plevel, x, y - 1, d, explosiontype(p) - 1);
                break;

            case 3: /* down */
                mover_explode(plevel, x - 1, y, d, explosiontype(p) - 1);
                mover_explode(plevel, x + 1, y, d, explosiontype(p) + 1);
                break;
	}
    }
    else
    {
        switch(p % 2)
        {
            case 0: /* left / right */
                mover_explode(plevel, x, y - 1, d, explosiontype(p) - 1);
                mover_explode(plevel, x, y + 1, d, explosiontype(p) + 1);
                break;
            case 1: /* up /down */
                mover_explode(plevel, x - 1, y, d, explosiontype(p) - 1);
                mover_explode(plevel, x + 1, y, d, explosiontype(p) + 1);
                break;
        }
    }
}


int canfall(int p, int into, int d)
{
    /* Determine whether a piece can start moving */

    /* Arrows and bombs */
    if(p >= PIECE_MOVERS_FIRST && p<= PIECE_MOVERS_LAST)
    {
	/* can start falling in their natural direction */
	if(d == (p % 4))
        {
	    /* but only into empty space */
	    if(into == PIECE_SPACE)
		return 1;
#ifdef XOR_COMPATIBILITY
	    /* or into directional dots if appropriate */
            if(into == PIECE_DOTS_X && (d == MOVE_LEFT || d == MOVE_RIGHT ))
                return 1;
            if(into == PIECE_DOTS_Y && (d == MOVE_UP || d == MOVE_DOWN ))
                return 1;
#endif
        }
    }

    return 0;
}


int canmove(int p, int into, int d, int fast)
{
    /* Determine whether a piece can continue moving */

    /* Arrows and bombs */
    if(p >= PIECE_MOVERS_FIRST && p<= PIECE_MOVERS_LAST)
    {
	/* can continue moving in their natural direction */
	if(d == (p % 4))
        {
	    /* into empty space */
	    if(into == PIECE_SPACE)
                return 1;
	    /* into dots if they're already moving */
            if(into == PIECE_DOTS && fast)
                return 1;
#ifdef XOR_COMPATIBILITY
	    /* into directional dots if appropriate */
            if(into == PIECE_DOTS_X && (d == MOVE_LEFT || d == MOVE_RIGHT ))
                return 1;
            if(into == PIECE_DOTS_Y && (d == MOVE_UP || d == MOVE_DOWN ))
                return 1;
#endif
	    /* through dying explosions */
	    if(isexplosion(into))
	        return 1; 
	    /* can kill players if already moving */
            if(into == PIECE_PLAYER_ONE && fast)
                return 1;
            if(into == PIECE_PLAYER_TWO && fast)
                return 1;
        }
	return 0;
    }

    /* Circles */
    if(p == PIECE_CIRCLE)
    {
	/* are stopped by everything other than empty space */
	if(into == PIECE_SPACE)
	    return 1;
	/* and dying explosions */
	if(isexplosion(into))
	    return 1;
	return 0;
    }

  return 0;
}

int canbepushed(int p, int into, int d, int mode)
{
    /* Determine whether a piece can be pushed by the player */

    /* Arrows and bombs */
    if(p >= PIECE_MOVERS_FIRST && p<= PIECE_MOVERS_LAST)
    {
	/* can be pushed, but not against their natural direction */
        if(d != ((p + 2) % 4))
        {
	    /* into empty space or through dots */
	    if(into == PIECE_SPACE || into == PIECE_DOTS)
                return 1;
#ifdef XOR_COMPATIBILITY
	    /* through directional dots if appropriate */
            if(into == PIECE_DOTS_X && (d == MOVE_LEFT || d == MOVE_RIGHT))
                return 1;
            if(into == PIECE_DOTS_Y && (d == MOVE_UP || d == MOVE_DOWN))
                return 1;
#endif
        }
        return 0;
    }

    /* Circles can be pushed in any direction */
    if(p == PIECE_CIRCLE
#ifdef ENIGMA_COMPATIBILITY
            || p == PIECE_CIRCLE_DOUBLE
#endif
      )
    {
	/* into empty space */
        if(into == PIECE_SPACE)
            return 1; 
#ifdef XOR_COMPATIBILITY
	/* XOR won't let circles (dolls) pass through dots */
        if(mode == MODE_XOR)
            return 0;
#endif
	/* pushed through dots */
        if(into == PIECE_DOTS)
            return 1;
        return 0;
    }

  return 0;
}

int canexplode(int p, int i, int d, int fast, int mode)
{
    /* Only an already moving arrow or bomb can act as a detonator */
    if(fast == 0)
        return 0; 

    /* Arrows can detonate bombs */
    if(p >= PIECE_ARROW_RED_LEFT && p<= PIECE_ARROW_RED_DOWN &&
       i >= PIECE_BOMB_RED_LEFT && i<= PIECE_BOMB_RED_DOWN)
	return 1;
    if(p >= PIECE_ARROW_GREEN_LEFT && p<= PIECE_ARROW_GREEN_DOWN &&
       i >= PIECE_BOMB_GREEN_LEFT && i<= PIECE_BOMB_GREEN_DOWN)
	return 1;
    if(p >= PIECE_ARROW_BLUE_LEFT && p<= PIECE_ARROW_BLUE_DOWN &&
       i >= PIECE_BOMB_BLUE_LEFT && i<= PIECE_BOMB_BLUE_DOWN)
	return 1;

#ifdef ENIGMA_COMPATIBILITY
    /* Enigma requires a moving arrow to detonate a stationary bomb, and
       does not permit bombs to detonate other bombs */
    if(mode == MODE_ENIGMA)
	return 0;
#endif

    /* Bombs can be detonated by arrows pointing towards them */
    if(p >= PIECE_BOMB_RED_LEFT && p<= PIECE_BOMB_RED_DOWN &&
       i == (PIECE_ARROW_RED_LEFT + ((d + 2) % 4)))
	return 1;
    if(p >= PIECE_BOMB_GREEN_LEFT && p<= PIECE_BOMB_GREEN_DOWN &&
       i == (PIECE_ARROW_GREEN_LEFT + ((d + 2) % 4)))
	return 1;
    if(p >= PIECE_BOMB_BLUE_LEFT && p<= PIECE_BOMB_BLUE_DOWN &&
       i == (PIECE_ARROW_BLUE_LEFT + ((d + 2) % 4)))
	return 1;

    /* Bombs can detonate other bombs */
    if(p >= PIECE_BOMB_RED_LEFT && p<= PIECE_BOMB_RED_DOWN &&
       i >= PIECE_BOMB_RED_LEFT && i<= PIECE_BOMB_RED_DOWN)
	return 1;
    if(p >= PIECE_BOMB_GREEN_LEFT && p<= PIECE_BOMB_GREEN_DOWN &&
       i >= PIECE_BOMB_GREEN_LEFT && i<= PIECE_BOMB_GREEN_DOWN)
	return 1;
    if(p >= PIECE_BOMB_BLUE_LEFT && p<= PIECE_BOMB_BLUE_DOWN &&
       i >= PIECE_BOMB_BLUE_LEFT && i<= PIECE_BOMB_BLUE_DOWN)
	return 1;

    return 0;
}

struct mover* mover_newundo(struct level* plevel, int x, int y, int d, int piece, int previous, int flags)
{
    struct mover* pmover;

    static int count = 0;

    if(plevel->flags & LEVELFLAG_NOUNDO)
        return NULL;

    pmover = (struct mover*)malloc(sizeof(struct mover));
    if(pmover == NULL)
        fatal("Out of memory in mover_newundo()");

    pmover->x = x;
    pmover->y = y;
    pmover->direction = d;
    pmover->piece = piece;
    pmover->piece_previous = previous;
    pmover->next = NULL;
    pmover->previous = plevel->mover_last;

    if(flags & MOVER_FAST)
	pmover->fast = 1;
    else
	pmover->fast = 0;

    if(flags & MOVER_UNDO)
    {
	level_setmoving(plevel, pmover->x, pmover->y, pmover->direction);

	if(options_debug & DEBUG_MOVERS)
	{
	    if(plevel->mover_first == NULL)
	        count = 0;

	    fprintf(stderr, "[%d] Cosmetic mover at (%d,%d) is %s was %s (direction=%c) (flags=%d)\n", 
	  	    count ++, pmover->x, pmover->y,
		    piece_name[pmover->piece], piece_name[pmover->piece_previous],
		    directiontochar(pmover->direction), pmover->fast);
	}

        if(plevel->mover_first == NULL)
            plevel->mover_first = pmover;

        if(plevel->mover_last != NULL)
            plevel->mover_last->next = pmover;

        plevel->mover_last = pmover;

    }

    if(flags & MOVER_STORE)
    {
        pmover->previous = plevel->move_current->mover_last;
        pmover->next = NULL;

        if(plevel->move_current->mover_first == NULL)
            plevel->move_current->mover_first = pmover;
        if(plevel->move_current->mover_last != NULL)
            plevel->move_current->mover_last->next = pmover;
        plevel->move_current->mover_last = pmover;

    }

    return pmover;
}


int level_undo(struct level* plevel)
{
    struct mover* pmover;
    struct mover* ptmp;
    struct mover* pmoverfirst;

    int td;
    int x, y, d;

    int count = 0;

    /* Can't undo if the level has no undo data (eg, a partial save) */
    if(plevel->move_first == NULL || (plevel->move_first->mover_first == NULL && plevel->move_current != plevel->move_first))
        return 0;

    /* Working backwards, undo any changes made to the map by movers in the
       previous step. */
    pmoverfirst = NULL;
    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
	pmoverfirst = pmover;
	pmover = pmover->next;
    }
    pmover = pmoverfirst;
    while(pmover != NULL)
    {
        /* Not setting SPACEs fixes a pathological case without apparently breaking anything (1.07) */
        if(pmover->piece != PIECE_SPACE)
        {
            level_setpiece(plevel, pmover->x, pmover->y, pmover->piece);
	    if(options_debug & DEBUG_MOVERS)
	        fprintf(stderr, "+ level_setpiece(%d, %d, %s)\n", pmover->x, pmover->y, piece_name[pmover->piece]);
        }

	pmover = pmover->previous;
	if(pmover == NULL)
	    break;
    }

    /* Is player one still alive? */
    if(level_piece(plevel, plevel->player_x[0], plevel->player_y[0]) != PIECE_PLAYER_ONE)
       	plevel->alive[0] = 0; 
    /* Is player two still alive? */
    if(level_piece(plevel, plevel->player_x[1], plevel->player_y[1]) != PIECE_PLAYER_TWO)
	plevel->alive[1] = 0; 

    /* Tidy up any movers created in the previous step */
    pmover = plevel->mover_first;
    while(pmover != NULL)
    {
        level_setmoving(plevel, pmover->x, pmover->y, MOVE_NONE);
	level_setprevious(plevel, pmover->x, pmover->y, PIECE_SPACE);
	ptmp = pmover;
        pmover = pmover->next;
	free(ptmp);
    }
    plevel->mover_first = NULL;
    plevel->mover_last = NULL;

    /* Can't undo at very start of level */
    if(plevel->move_current == NULL)
	return 0;

    /* If there is no previous step to undo, remove this move entirely */
    if(plevel->move_current->mover_last == NULL)
    {
	plevel->move_current = plevel->move_current->previous;
	plevel->flags |= LEVELFLAG_MOVES;

	return 0;
    }

    if(options_debug & DEBUG_MOVERS)
        fprintf(stderr, "\n");
    
    /* Start from the last mover for this step. */
    pmover = plevel->move_current->mover_last;

    pmoverfirst = NULL;

    td = MOVE_NONE;

    /* Working backwards, remove these pieces from the map */
    while(pmover != NULL)
    {
	pmoverfirst = pmover;

	level_setpiece(plevel, pmover->x, pmover->y, PIECE_SPACE);

	if(options_debug & DEBUG_MOVERS)
	    fprintf(stderr, "- level_setpiece(%d, %d, %s)\n", pmover->x, pmover->y, piece_name[PIECE_SPACE]);

	/* If the piece is the player, update position and status */
	if(pmover->piece_previous == PIECE_PLAYER_ONE || pmover->piece_previous == PIECE_PLAYER_TWO)
	{
            plevel->player_x[pmover->piece_previous - PIECE_PLAYER_ONE] = pmover->x;
            plevel->player_y[pmover->piece_previous - PIECE_PLAYER_ONE] = pmover->y;

#ifdef XOR_COMPATIBILITY
            if(plevel->mode == MODE_XOR)
            {
                /* If a player is being resurrected in this move, and the
                   other player is alive, undo the automatic swap */
                if(plevel->alive[pmover->piece_previous - PIECE_PLAYER_ONE] == 0 && plevel->alive[plevel->player])
                {
                    /* Cosmetic mover to deactivate other player */
 	            mover_newundo(plevel, plevel->player_x[plevel->player], plevel->player_y[plevel->player], MOVE_SWAPPED, PIECE_PLAYER_ONE + plevel->player, PIECE_SPACE, MOVER_UNDO);
                    plevel->player = 1 - plevel->player; 
                }
            }
#endif

            plevel->alive[pmover->piece_previous - PIECE_PLAYER_ONE] = 1;
	}

#ifdef XOR_COMPATIBILITY
	/* If the piece is a teleport, store the direction of the original move
	   into it for later use. */
	if(pmover->piece == PIECE_TELEPORT)
	    td = pmover->direction;
#endif

	/* until we reach the first mover for this step. */
	pmover = pmover->previous;
	if(pmover != NULL && pmover->fast == 0)
	    break;
    }

    pmover = pmoverfirst;

    /* Now, move forwards through the movers and create cosmetic effects. */
    while(pmover != NULL)
    {

	if(options_debug & DEBUG_MOVERS)
	    fprintf(stderr, "[%d] Undo mover at (%d,%d) is %s was %s (direction=%c) (flags=%d)\n", 
		count++, pmover->x, pmover->y,
		piece_name[pmover->piece], piece_name[pmover->piece_previous],
		directiontochar(pmover->direction), pmover->fast);

	d = pmover->direction;
	x = pmover->x - move_x[d];
	y = pmover->y - move_y[d];

	if(d != MOVE_NONE && d != MOVE_SWAP && d != MOVE_SWAPPED)
	    d = (d + 2) % 4;

	if(isexplosion(pmover->piece))
	{
	    /* Explosions don't move. */
	    d = MOVE_NONE;
	    /* Show dying explosion when undoing new explosion */
	    if(options_debug & DEBUG_MOVERS)
	        fprintf(stderr, "* level_setprevious(%d, %d, %s)\n", pmover->x, pmover->y, piece_name[pmover->piece]);
	    level_setprevious(plevel, pmover->x, pmover->y, pmover->piece);
	}

	/* Do we need to patch up the direction this piece is moving in? */
	/* Is it the player? */
	if((pmover->piece_previous == PIECE_PLAYER_ONE || pmover->piece_previous == PIECE_PLAYER_TWO) && (pmover->piece == PIECE_SPACE || pmover->piece == PIECE_GONE))
	{
	    /* If so, are they moving out of a teleport? Use original direction
	       of move if so. */
	    if(td != MOVE_NONE)
	        d = (td + 2) % 4;
	}
	/* Otherwise, if the previous piece wasn't a move, it must have been a
	   static piece being eaten by a mover, and thus shouldn't move. */
	else if((pmover->piece_previous < PIECE_MOVERS_FIRST || pmover->piece_previous > PIECE_MOVERS_LAST) && pmover->piece_previous != PIECE_CIRCLE
#ifdef ENIGMA_COMPATIBILITY
                && pmover->piece_previous != PIECE_CIRCLE_DOUBLE
#endif
               )
	    d = MOVE_NONE;

	/* Plot a cosmetic mover. */
        if(level_previous(plevel, pmover->x, pmover->y) != PIECE_SPACE)
            d = MOVE_NONE;
        /* but not if there are overlapping explosions */
        if(!(pmover->piece_previous >= PIECE_EXPLOSION_NEW_FIRST && pmover->piece_previous <= PIECE_EXPLOSION_NEW_LAST))
	    mover_newundo(plevel, pmover->x, pmover->y, d, pmover->piece_previous, PIECE_SPACE, MOVER_UNDO);

	pmover = pmover->next;
    }

    pmover = pmoverfirst->previous;

    /* If there is another step, set it up for the next iteration */
    if(pmover != NULL)
    {
        plevel->move_current->mover_last = pmover;
	pmover = pmover->next;
    }
    else
    {
	pmover = plevel->move_current->mover_first;
	plevel->move_current->mover_first = NULL;
	plevel->move_current->mover_last = NULL;

	plevel->moves --;
    }

    /* Remove the movers in the step we've just done */
    if(pmover != NULL)
    {
	while(pmover != NULL)
	{
	    /* Undo any pieces exploded or caught */
	    if(pmover->piece_previous == PIECE_STAR)
	    {
		if(pmover->piece == PIECE_PLAYER_ONE || pmover->piece == PIECE_PLAYER_TWO)
		    plevel->stars_caught --;
		else
		    plevel->stars_exploded --;

		plevel->flags |= LEVELFLAG_STARS;
	    }
#ifdef XOR_COMPATIBILITY
	    if(pmover->piece_previous == PIECE_SWITCH)
	    {
		plevel->switched = 1 - plevel->switched;
		plevel->flags |= LEVELFLAG_SWITCH;
	    }
	    if(pmover->piece_previous == PIECE_MAP_TOP_LEFT)
	    {
		plevel->mapped ^= MAPPED_TOP_LEFT;
		plevel->flags |= LEVELFLAG_MAP;
	    }
	    if(pmover->piece_previous == PIECE_MAP_TOP_RIGHT)
	    {
		plevel->mapped ^= MAPPED_TOP_RIGHT;
		plevel->flags |= LEVELFLAG_MAP;
	    }
	    if(pmover->piece_previous == PIECE_MAP_BOTTOM_LEFT)
	    {
		plevel->mapped ^= MAPPED_BOTTOM_LEFT;
		plevel->flags |= LEVELFLAG_MAP;
	    }
	    if(pmover->piece_previous == PIECE_MAP_BOTTOM_RIGHT)
	    {
		plevel->mapped ^= MAPPED_BOTTOM_RIGHT;
		plevel->flags |= LEVELFLAG_MAP;
	    }
#endif

	    ptmp = pmover;
	    pmover = pmover->next;
	    free(ptmp);
	}
    }

    if(plevel->move_current->mover_last != NULL)
	plevel->move_current->mover_last->next = NULL;

    /* If the move was a swap, revert to the previous player */
    if(plevel->move_current->direction == MOVE_SWAP)
	plevel->player = 1 - plevel->player;

    /* Have we just undone failure? */
    if((plevel->flags & LEVELFLAG_FAILED) && (plevel->alive[0] != 0 || plevel->alive[1] != 0))
    {
	plevel->flags &= ~LEVELFLAG_FAILED;
	plevel->flags |= LEVELFLAG_MOVES;
    }

    /* Have we just undone success? */
    if(plevel->flags & (LEVELFLAG_SOLVED | LEVELFLAG_EXIT))
    {
	plevel->flags &= ~LEVELFLAG_SOLVED;
	plevel->flags &= ~LEVELFLAG_EXIT;
        plevel->flags |= LEVELFLAG_STARS;
    }

#ifdef XOR_COMPATIBILITY
    if(plevel->mode == MODE_XOR)
        xor_focus(plevel);
#endif

    /* If there are no more steps in this move, chroma-curses needs advanced
       warning that the move counter is going to change. */
    if(plevel->move_current->mover_last == NULL)
    {
        plevel->flags |= LEVELFLAG_MOVES;
    }

    return 1;
}
