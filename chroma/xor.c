/*  
    xor.c

    An (almost) exact reverse engineering of the original BBC XOR engine.
    Rather than the usual list of movers, it maintains a separate stack of
    spaces to be examined. In each round, we examine the first space on this
    stack and generate movers accordingly; in doing so, we may add additional
    spaces to the stack. The movers are cosmetic, and are removed at the end of
    the round.

    See levels/regression/xor-regression.chroma for further details of what
    it gets right versus other engines. The memory addresses are those of the
    equivalent 6502 code in the original version.


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

extern int move_x[];
extern int move_y[];

extern int options_xor_mode;

#define PIECE_XOR_MAGUS	        PIECE_PLAYER_ONE
#define PIECE_XOR_QUAESTOR	PIECE_PLAYER_TWO
#define PIECE_XOR_DOTS	        PIECE_DOTS_X
#define PIECE_XOR_WAVES	        PIECE_DOTS_Y
#define PIECE_XOR_CHICKEN	PIECE_ARROW_RED_LEFT
#define PIECE_XOR_V_BOMB	PIECE_BOMB_RED_LEFT
#define PIECE_XOR_FISH	        PIECE_ARROW_RED_DOWN
#define PIECE_XOR_H_BOMB	PIECE_BOMB_RED_DOWN
#define PIECE_XOR_DOLL	        PIECE_CIRCLE
#define PIECE_XOR_MASK	        PIECE_STAR

void xor_focus(struct level* plevel);

/* 0x192e - 0x1941 */
/* mover_addtostack */

/* 0x1a4f - 0x1a75 */
void xor_considerdownmover(struct level* plevel, int x, int y)
{
    int p; 

    p = level_piece(plevel, x, y - 1);
    if(p == PIECE_XOR_FISH || p == PIECE_XOR_H_BOMB)
        mover_addtostack(plevel, x, y - 1, 0);
}

/* 0x1a76 - 0x1a98 */
void xor_considerleftmover(struct level* plevel, int x, int y)
{
    int p;

    p = level_piece(plevel, x + 1, y);
    if(p == PIECE_XOR_CHICKEN || p == PIECE_XOR_V_BOMB)
        mover_addtostack(plevel, x + 1, y, 0);
}

/* 0x1f39 - 0x202b */
int xor_teleport(struct level* plevel, int px, int py, int *dx, int *dy, int *move)
{
    int teleport;
    int tx, ty;
    int moveout = MOVE_NONE;

    teleport = -1;
    if(px == plevel->teleport_x[0] && py == plevel->teleport_y[0])
	teleport = 0;
    if(px == plevel->teleport_x[1] && py == plevel->teleport_y[1])
	teleport = 1;
    if(teleport == -1)
	return 0;

    tx = plevel->teleport_x[1 - teleport];
    ty = plevel->teleport_y[1 - teleport];

    /* If the other teleport has been destroyed, turn this one into a wall */
    if(level_piece(plevel, tx, ty) != PIECE_TELEPORT)
    {
        mover_new(plevel, plevel->teleport_x[teleport], plevel->teleport_y[teleport], MOVE_NONE, PIECE_WALL, 0);
	return 0;
    }

    if(level_piece(plevel, tx + 1, ty) == PIECE_SPACE)
    {
	*dx = tx + 1;
	*dy = ty;
	moveout = MOVE_RIGHT;
    }
    else if(level_piece(plevel, tx, ty - 1) == PIECE_SPACE)
    {
	*dx = tx;
	*dy = ty - 1;
	moveout = MOVE_UP;
    }
    else if(level_piece(plevel, tx - 1, ty) == PIECE_SPACE)
    {
	*dx = tx - 1;
	*dy = ty;
	moveout = MOVE_LEFT;
    }
    else if(level_piece(plevel, tx, ty + 1) == PIECE_SPACE)
    {
	*dx = tx;
	*dy = ty + 1;
	moveout = MOVE_DOWN;
    }

    if(moveout != MOVE_NONE)
    {

	/* Visual effects for the player moving in to the teleport */
	/* Store original player move direction in cosmetic mover */
        mover_new(plevel, px, py, *move, PIECE_TELEPORT, 0);
        level_setprevious(plevel, px, py, PIECE_PLAYER_ONE + plevel->player);
        level_setpreviousmoving(plevel, px, py, *move);

	/* Visual effects for the player moving out of the teleport */
        mover_new(plevel, tx, ty, MOVE_NONE, PIECE_TELEPORT, 0);

	/* Change move to produce the effect of coming out of the teleport */
        *move = moveout;

        plevel->view_x[plevel->player] = plevel->view_teleport_x[1 - teleport];
        plevel->view_y[plevel->player] = plevel->view_teleport_y[1 - teleport];

	return 1;
    }

    return 0;
}

/* 0x1f0e - 0x1f38 */
void xor_explode(struct level* plevel, int x, int y, int type, int previous)
{
    int p;

    p = level_piece(plevel, x, y);

    if(p == PIECE_SWITCH)
    {
        plevel->switched = 1 - plevel->switched;
        plevel->flags |= LEVELFLAG_SWITCH;
    }
    if(p == PIECE_XOR_MASK)
    {
	plevel->stars_exploded ++;
	plevel->flags |= LEVELFLAG_STARS;
    }

    /* Don't explode edge walls */
    if(x == 0 || y == 0 || x == plevel->size_x - 1 || y == plevel->size_y - 1)
        return;

    if(previous != MOVE_NONE)
    {
        mover_new(plevel, x, y, previous, PIECE_GONE, 0);
        level_setpiece(plevel, x, y, PIECE_SPACE);
    }
    mover_new(plevel, x, y, MOVE_NONE, type, 0);
    if(previous == MOVE_NONE)
        level_setprevious(plevel, x, y, p);
}

int xor_move(struct level* plevel, int move)
{
    int p;
    int px, py;
    int dx, dy;
    int ox, oy;
    int into;
    int bp;

    if(plevel->alive[plevel->player] == 0)
	return 0;

    px = plevel->player_x[plevel->player];
    py = plevel->player_y[plevel->player];

    /* 0x141e - 0x14df */
    if(move == MOVE_LEFT)
    {
        dx = px - 1;
        dy = py;

        ox = px - 2;
        oy = py;

	p = level_piece(plevel, dx, dy);
	if(p == PIECE_WALL || p == PIECE_XOR_MAGUS || p == PIECE_XOR_QUAESTOR)
	    return 0;
	if(p == PIECE_DOOR)
	{
            if(plevel->stars_caught == plevel->stars_total)
	        plevel->flags |= LEVELFLAG_EXIT;
	    else
		return 0;
	}
	if(p == PIECE_XOR_WAVES || p == PIECE_XOR_CHICKEN || p == PIECE_XOR_V_BOMB)
	    return 0;
	if(p == PIECE_XOR_FISH || p == PIECE_XOR_H_BOMB)
	{
	    into = level_piece(plevel, ox, oy);
	    if(into == PIECE_SPACE || into == PIECE_XOR_DOTS)
	    {
	        mover_new(plevel, ox, oy, MOVE_LEFT, p, 0);
		bp = level_piece(plevel, ox, oy + 1);
		if(bp == PIECE_SPACE || bp == PIECE_XOR_WAVES)
		    mover_addtostack(plevel, ox, oy, 0);
	    }
	    else
		return 0;
	}
	if(p == PIECE_TELEPORT)
	{
	    if(!xor_teleport(plevel, px - 1, py, &dx, &dy, &move))
		return 0;
	}
	if(p == PIECE_XOR_MASK)
	{
            plevel->stars_caught ++;
            plevel->flags |= LEVELFLAG_STARS;
	}
	if(p == PIECE_SWITCH)
	{
            plevel->switched = 1 - plevel->switched;
	    plevel->flags |= LEVELFLAG_SWITCH;
	}
	if(p == PIECE_MAP_TOP_LEFT)
	{
	    plevel->mapped |= MAPPED_TOP_LEFT;
	    plevel->flags |= LEVELFLAG_MAP;
	}
	if(p == PIECE_MAP_TOP_RIGHT)
	{
	    plevel->mapped |= MAPPED_TOP_RIGHT;
	    plevel->flags |= LEVELFLAG_MAP;
	}
	if(p == PIECE_MAP_BOTTOM_LEFT)
	{
	    plevel->mapped |= MAPPED_BOTTOM_LEFT;
	    plevel->flags |= LEVELFLAG_MAP;
	}
	if(p == PIECE_MAP_BOTTOM_RIGHT)
	{
	    plevel->mapped |= MAPPED_BOTTOM_RIGHT;
	    plevel->flags |= LEVELFLAG_MAP;
	}
	if(p == PIECE_XOR_DOLL)
	{
	    into = level_piece(plevel, ox, oy);
	    if(into == PIECE_SPACE)
	    {
	        mover_new(plevel, ox, oy, MOVE_LEFT, p, 0);
		if(level_piece(plevel, ox - 1, oy) == PIECE_SPACE)
		    mover_addtostack(plevel, ox, oy, MOVE_LEFT);
	    }
	    else
		return 0;
	}

	plevel->player_x[plevel->player] = dx;
	plevel->player_y[plevel->player] = dy;

	mover_new(plevel, dx, dy, move, PIECE_XOR_MAGUS + plevel->player, 0);
	mover_new(plevel, px, py, move, PIECE_SPACE, 0);

	if(plevel->flags & LEVELFLAG_EXIT)
	    return 1;

	xor_considerdownmover(plevel, px, py);
	xor_considerleftmover(plevel, px, py);

	return 1;
    }

    /* 0x14e8 - 0x15a2 */
    if(move == MOVE_RIGHT)
    {
        dx = px + 1;
        dy = py;

        ox = px + 2;
        oy = py;

	p = level_piece(plevel, dx, dy);
	if(p == PIECE_WALL || p == PIECE_XOR_MAGUS || p == PIECE_XOR_QUAESTOR)
	    return 0;
	if(p == PIECE_DOOR)
	{
            if(plevel->stars_caught == plevel->stars_total)
	        plevel->flags |= LEVELFLAG_EXIT;
	    else
		return 0;
	}
	if(p == PIECE_XOR_WAVES || p == PIECE_XOR_CHICKEN || p == PIECE_XOR_V_BOMB)
	    return 0;
	if(p == PIECE_XOR_FISH || p == PIECE_XOR_H_BOMB)
	{
	    into = level_piece(plevel, ox, oy);
	    if(into == PIECE_SPACE || into == PIECE_XOR_DOTS)
	    {
	        mover_new(plevel, ox, oy, MOVE_RIGHT, p, 0);
		bp = level_piece(plevel, ox, oy + 1);
		if(bp == PIECE_SPACE || bp == PIECE_XOR_WAVES)
		    mover_addtostack(plevel, ox, oy, 0);
	    }
	    else
		return 0;
	}
	if(p == PIECE_TELEPORT)
	{
	    if(!xor_teleport(plevel, px + 1, py, &dx, &dy, &move))
		return 0;
	}
	if(p == PIECE_XOR_MASK)
	{
            plevel->stars_caught ++;
            plevel->flags |= LEVELFLAG_STARS;
	}
	if(p == PIECE_SWITCH)
	{
            plevel->switched = 1 - plevel->switched;
	    plevel->flags |= LEVELFLAG_SWITCH;
	}
	if(p == PIECE_MAP_TOP_LEFT)
	{
	    plevel->mapped |= MAPPED_TOP_LEFT;
	    plevel->flags |= LEVELFLAG_MAP;
	}
	if(p == PIECE_MAP_TOP_RIGHT)
	{
	    plevel->mapped |= MAPPED_TOP_RIGHT;
	    plevel->flags |= LEVELFLAG_MAP;
	}
	if(p == PIECE_MAP_BOTTOM_LEFT)
	{
	    plevel->mapped |= MAPPED_BOTTOM_LEFT;
	    plevel->flags |= LEVELFLAG_MAP;
	}
	if(p == PIECE_MAP_BOTTOM_RIGHT)
	{
	    plevel->mapped |= MAPPED_BOTTOM_RIGHT;
	    plevel->flags |= LEVELFLAG_MAP;
	}
	if(p == PIECE_XOR_DOLL)
	{
	    into = level_piece(plevel, ox, oy);
	    if(into == PIECE_SPACE)
	    {
	        mover_new(plevel, ox, oy, MOVE_RIGHT, p, 0);
		if(level_piece(plevel, ox + 1, oy) == PIECE_SPACE)
		    mover_addtostack(plevel, ox, oy, MOVE_RIGHT);
	    }
	    else
		return 0;
	}

	plevel->player_x[plevel->player] = dx;
	plevel->player_y[plevel->player] = dy;

	mover_new(plevel, dx, dy, move, PIECE_XOR_MAGUS + plevel->player, 0);
	mover_new(plevel, px, py, move, PIECE_SPACE, 0);

	if(plevel->flags & LEVELFLAG_EXIT)
	    return 1;

	xor_considerdownmover(plevel, px, py);

	return 1;
    }

    /* 0x15a5 - 0x1666 */
    if(move == MOVE_UP)
    {
        dx = px;
        dy = py - 1;

        ox = px;
        oy = py - 2;

	p = level_piece(plevel, dx, dy);
	if(p == PIECE_WALL || p == PIECE_XOR_MAGUS || p == PIECE_XOR_QUAESTOR)
	    return 0;
	if(p == PIECE_DOOR)
	{
            if(plevel->stars_caught == plevel->stars_total)
	        plevel->flags |= LEVELFLAG_EXIT;
	    else
		return 0;
	}
	if(p == PIECE_XOR_DOTS || p == PIECE_XOR_FISH || p == PIECE_XOR_H_BOMB)
	    return 0;
	if(p == PIECE_XOR_CHICKEN || p == PIECE_XOR_V_BOMB)
	{
	    into = level_piece(plevel, ox, oy);
	    if(into == PIECE_SPACE || into == PIECE_XOR_WAVES)
	    {
	        mover_new(plevel, ox, oy, MOVE_UP, p, 0);
		bp = level_piece(plevel, ox - 1, oy);
		if(bp == PIECE_SPACE || bp == PIECE_XOR_DOTS)
		    mover_addtostack(plevel, ox, oy, 0);
	    }
	    else
		return 0;
	}
	if(p == PIECE_TELEPORT)
	{
	    if(!xor_teleport(plevel, px, py - 1, &dx, &dy, &move))
		return 0;
	}
	if(p == PIECE_XOR_MASK)
	{
            plevel->stars_caught ++;
            plevel->flags |= LEVELFLAG_STARS;
	}
	if(p == PIECE_SWITCH)
	{
            plevel->switched = 1 - plevel->switched;
	    plevel->flags |= LEVELFLAG_SWITCH;
	}
	if(p == PIECE_MAP_TOP_LEFT)
	{
	    plevel->mapped |= MAPPED_TOP_LEFT;
	    plevel->flags |= LEVELFLAG_MAP;
	}
	if(p == PIECE_MAP_TOP_RIGHT)
	{
	    plevel->mapped |= MAPPED_TOP_RIGHT;
	    plevel->flags |= LEVELFLAG_MAP;
	}
	if(p == PIECE_MAP_BOTTOM_LEFT)
	{
	    plevel->mapped |= MAPPED_BOTTOM_LEFT;
	    plevel->flags |= LEVELFLAG_MAP;
	}
	if(p == PIECE_MAP_BOTTOM_RIGHT)
	{
	    plevel->mapped |= MAPPED_BOTTOM_RIGHT;
	    plevel->flags |= LEVELFLAG_MAP;
	}
	if(p == PIECE_XOR_DOLL)
	{
	    into = level_piece(plevel, ox, oy);
	    if(into == PIECE_SPACE)
	    {
	        mover_new(plevel, ox, oy, MOVE_UP, p, 0);
		if(level_piece(plevel, ox, oy - 1) == PIECE_SPACE)
		    mover_addtostack(plevel, ox, oy, MOVE_UP);
	    }
	    else
		return 0;
	}

	plevel->player_x[plevel->player] = dx;
	plevel->player_y[plevel->player] = dy;

	mover_new(plevel, dx, dy, move, PIECE_XOR_MAGUS + plevel->player, 0);
	mover_new(plevel, px, py, move, PIECE_SPACE, 0);

	if(plevel->flags & LEVELFLAG_EXIT)
	    return 1;

	xor_considerleftmover(plevel, px, py);

	return 1;
    }

    /* 0x1669 - 0x1729 */
    if(move == MOVE_DOWN)
    {
        dx = px;
        dy = py + 1;

        ox = px;
        oy = py + 2;

	p = level_piece(plevel, dx, dy);
	if(p == PIECE_WALL || p == PIECE_XOR_MAGUS || p == PIECE_XOR_QUAESTOR)
	    return 0;
	if(p == PIECE_DOOR)
	{
            if(plevel->stars_caught == plevel->stars_total)
	        plevel->flags |= LEVELFLAG_EXIT;
	    else
		return 0;
	}
	if(p == PIECE_XOR_DOTS || p == PIECE_XOR_FISH || p == PIECE_XOR_H_BOMB)
	    return 0;
	if(p == PIECE_XOR_CHICKEN || p == PIECE_XOR_V_BOMB)
	{
	    into = level_piece(plevel, ox, oy);
	    if(into == PIECE_SPACE || into == PIECE_XOR_WAVES)
	    {
	        mover_new(plevel, ox, oy, MOVE_DOWN, p, 0);
		bp = level_piece(plevel, ox - 1, oy);
		if(bp == PIECE_SPACE || bp == PIECE_XOR_DOTS)
		    mover_addtostack(plevel, ox, oy, 0);
	    }
	    else
		return 0;
	}
	if(p == PIECE_TELEPORT)
	{
	    if(!xor_teleport(plevel, px, py + 1, &dx, &dy, &move))
		return 0;
	}
	if(p == PIECE_XOR_MASK)
	{
            plevel->stars_caught ++;
            plevel->flags |= LEVELFLAG_STARS;
	}
	if(p == PIECE_SWITCH)
	{
            plevel->switched = 1 - plevel->switched;
	    plevel->flags |= LEVELFLAG_SWITCH;
	}
	if(p == PIECE_MAP_TOP_LEFT)
	{
	    plevel->mapped |= MAPPED_TOP_LEFT;
	    plevel->flags |= LEVELFLAG_MAP;
	}
	if(p == PIECE_MAP_TOP_RIGHT)
	{
	    plevel->mapped |= MAPPED_TOP_RIGHT;
	    plevel->flags |= LEVELFLAG_MAP;
	}
	if(p == PIECE_MAP_BOTTOM_LEFT)
	{
	    plevel->mapped |= MAPPED_BOTTOM_LEFT;
	    plevel->flags |= LEVELFLAG_MAP;
	}
	if(p == PIECE_MAP_BOTTOM_RIGHT)
	{
	    plevel->mapped |= MAPPED_BOTTOM_RIGHT;
	    plevel->flags |= LEVELFLAG_MAP;
	}
	if(p == PIECE_XOR_DOLL)
	{
	    into = level_piece(plevel, ox, oy);
	    if(into == PIECE_SPACE)
	    {
	        mover_new(plevel, ox, oy, MOVE_DOWN, p, 0);
		if(level_piece(plevel, ox, oy + 1) == PIECE_SPACE)
		    mover_addtostack(plevel, ox, oy, MOVE_DOWN);
	    }
	    else
		return 0;
	}

	plevel->player_x[plevel->player] = dx;
	plevel->player_y[plevel->player] = dy;

	mover_new(plevel, dx, dy, move, PIECE_XOR_MAGUS + plevel->player, 0);
	mover_new(plevel, px, py, move, PIECE_SPACE, 0);

	if(plevel->flags & LEVELFLAG_EXIT)
	    return 1;

	xor_considerleftmover(plevel, px, py);
	xor_considerdownmover(plevel, px, py);

	return 1;
    }

    return 0;
}

int xor_evolve(struct level* plevel)
{
    struct mover* poldmovers;
    struct mover* pmover;
    struct mover* ptmp;

    int mp;
    int into;
    int np;

    int d;

    int px, py;

    poldmovers = plevel->mover_first;

    plevel->mover_first = NULL;
    plevel->mover_last = NULL;

    pmover = poldmovers;
    while(pmover != NULL)
    {
	if(isexplosion(pmover->piece))
	    mover_new(plevel, pmover->x, pmover->y, MOVE_NONE, PIECE_SPACE, 0);

        level_setmoving(plevel, pmover->x, pmover->y, MOVE_NONE);
        level_setprevious(plevel, pmover->x, pmover->y, PIECE_SPACE);
        level_setpreviousmoving(plevel, pmover->x, pmover->y, MOVE_NONE);
        level_setdetonator(plevel, pmover->x, pmover->y, PIECE_SPACE);
        level_setdetonatormoving(plevel, pmover->x, pmover->y, MOVE_NONE);

	if(isexplosion(pmover->piece))
            level_setprevious(plevel, pmover->x, pmover->y, pmover->piece);

	ptmp = pmover->next;
	free(pmover);
	pmover = ptmp;
    }

    while(plevel->mover_first == NULL)
    {

    pmover = plevel->stack_first;

    if(pmover == NULL)
	return 0;

    px = pmover->x;
    py = pmover->y;

    mp = level_piece(plevel, px, py);

    /* 0x1988 - 0x19df */
    if(mp == PIECE_XOR_FISH || mp == PIECE_XOR_H_BOMB)
    {
	into = level_piece(plevel, px, py + 1);
	if(into == PIECE_SPACE || into == PIECE_XOR_WAVES || into == PIECE_XOR_MAGUS || into == PIECE_XOR_QUAESTOR)
	{
	    xor_considerdownmover(plevel, px, py);
	    xor_considerleftmover(plevel, px, py);

	    np = level_piece(plevel, px, py + 2);
	    if(np == PIECE_SPACE || np == PIECE_XOR_WAVES || np == PIECE_XOR_MAGUS || np == PIECE_XOR_QUAESTOR || np == PIECE_XOR_V_BOMB || np == PIECE_XOR_H_BOMB)
	        mover_addtostack(plevel, px, py + 1, MOVE_NONE);

	    mover_new(plevel, px, py + 1, MOVE_DOWN, mp, 0);
	    mover_new(plevel, px, py, MOVE_DOWN, PIECE_SPACE, 0);

	}
	/* 0x1ae9 - 0x1b54 */
	if(into == PIECE_XOR_H_BOMB)
	{
	    /* BBC XOR does *not* considerleftmover(px, py) */
	    xor_considerdownmover(plevel, px, py);
            xor_considerdownmover(plevel, px - 1, py + 1);
            xor_considerdownmover(plevel, px + 1, py + 1);
            xor_considerleftmover(plevel, px + 1, py + 1);

	    mover_new(plevel, px, py, MOVE_DOWN, PIECE_SPACE, 0);
	    level_setdetonator(plevel, px, py + 1, mp);
	    level_setdetonatormoving(plevel, px, py + 1, MOVE_DOWN);

	    xor_explode(plevel, px - 1, py + 1, PIECE_EXPLOSION_RED_LEFT, MOVE_NONE);
	    xor_explode(plevel, px, py + 1, PIECE_EXPLOSION_RED_HORIZONTAL, MOVE_NONE);
	    xor_explode(plevel, px + 1, py + 1, PIECE_EXPLOSION_RED_RIGHT, MOVE_NONE);
	}

	/* 0x1b57 - 0x1baf */
	if(into == PIECE_XOR_V_BOMB)
	{
	    xor_considerdownmover(plevel, px, py);
	    xor_considerleftmover(plevel, px, py);
	    xor_considerleftmover(plevel, px, py + 1);
	    xor_considerleftmover(plevel, px, py + 2);

	    level_setdetonator(plevel, px, py + 1, mp);
	    level_setdetonatormoving(plevel, px, py + 1, MOVE_DOWN);

	    xor_explode(plevel, px, py + 0, PIECE_EXPLOSION_RED_TOP, MOVE_DOWN);
	    xor_explode(plevel, px, py + 1, PIECE_EXPLOSION_RED_VERTICAL, MOVE_NONE);
	    xor_explode(plevel, px, py + 2, PIECE_EXPLOSION_RED_BOTTOM, MOVE_NONE);
	}
    }

    /* 0x1bb2 - 0x1c05 */
    if(mp == PIECE_XOR_CHICKEN || mp == PIECE_XOR_V_BOMB)
    {
	into = level_piece(plevel, px - 1, py);
	if(into == PIECE_SPACE || into == PIECE_XOR_DOTS || into == PIECE_XOR_MAGUS || into == PIECE_XOR_QUAESTOR)
	{
	    xor_considerleftmover(plevel, px, py);
	    xor_considerdownmover(plevel, px, py);

	    np = level_piece(plevel, px - 2, py);
	    if(np == PIECE_SPACE || np == PIECE_XOR_DOTS || np == PIECE_XOR_MAGUS || np == PIECE_XOR_QUAESTOR || np == PIECE_XOR_V_BOMB || np == PIECE_XOR_H_BOMB)
	        mover_addtostack(plevel, px - 1, py, MOVE_NONE);

	    mover_new(plevel, px - 1, py, MOVE_LEFT, mp, 0);
	    mover_new(plevel, px, py, MOVE_LEFT, PIECE_SPACE, 0);

	}

	/* 0x1c08 - 0x1c4e */
	if(into == PIECE_XOR_H_BOMB)
	{
	    xor_considerleftmover(plevel, px, py);
	    xor_considerdownmover(plevel, px, py);
	    xor_considerdownmover(plevel, px - 1, py);
	    xor_considerdownmover(plevel, px - 2, py);

	    level_setdetonator(plevel, px - 1, py, mp);
	    level_setdetonatormoving(plevel, px - 1, py, MOVE_LEFT);

	    xor_explode(plevel, px - 2, py, PIECE_EXPLOSION_RED_LEFT, MOVE_NONE);
	    xor_explode(plevel, px - 1, py, PIECE_EXPLOSION_RED_HORIZONTAL, MOVE_NONE);
	    xor_explode(plevel, px, py, PIECE_EXPLOSION_RED_RIGHT, MOVE_LEFT);
	}

	/* 0x1c51 - 0x1cb4 */
	if(into == PIECE_XOR_V_BOMB)
	{
	    xor_considerleftmover(plevel, px, py);
	    xor_considerdownmover(plevel, px, py);
	    xor_considerleftmover(plevel, px - 1, py + 1);
            xor_considerleftmover(plevel, px - 1, py - 1);
            xor_considerdownmover(plevel, px - 1, py - 1);

	    mover_new(plevel, px, py, MOVE_LEFT, PIECE_SPACE, 0);
	    level_setdetonator(plevel, px - 1, py, mp);
	    level_setdetonatormoving(plevel, px - 1, py, MOVE_LEFT);

	    xor_explode(plevel, px - 1, py - 1, PIECE_EXPLOSION_RED_TOP, MOVE_NONE);
	    xor_explode(plevel, px - 1, py, PIECE_EXPLOSION_RED_VERTICAL, MOVE_NONE);
	    xor_explode(plevel, px - 1, py + 1, PIECE_EXPLOSION_RED_BOTTOM, MOVE_NONE);
	}
    }

    /* 0x209e - 0x20f4 */
    if(mp == PIECE_XOR_DOLL)
    {
	d = pmover->direction;

	into = level_piece(plevel, px + move_x[d], py + move_y[d]);
	if(into == PIECE_SPACE)
	{
	    np = level_piece(plevel, px + 2 * move_x[d], py + 2 * move_y[d]);
	    if(np == PIECE_SPACE)
		mover_addtostack(plevel, px + move_x[d], py + move_y[d], d);

	    mover_new(plevel, px + move_x[d], py + move_y[d], d, PIECE_XOR_DOLL, 0);
	    mover_new(plevel, px, py, d, PIECE_SPACE, 0);
	}
    }

    if(plevel->stack_last == plevel->stack_first)
	plevel->stack_last = NULL;
    plevel->stack_first = plevel->stack_first->next;
    free(pmover);

    }

    /* Is player one still alive? */
    if(level_piece(plevel, plevel->player_x[0], plevel->player_y[0]) != PIECE_PLAYER_ONE)
        plevel->alive[0] = 0;

    /* Is player two still alive? */
    if(level_piece(plevel, plevel->player_x[1], plevel->player_y[1]) != PIECE_PLAYER_TWO)
        plevel->alive[1] = 0; 

    /* If a player has died, swap to the other player at the end of the move */
    if(plevel->alive[plevel->player] == 0 && plevel->stack_first == NULL)
    {
        if(plevel->alive[1 - plevel->player])
        {
            plevel->player = 1 - plevel->player;

            /* Cosmetic mover for the newly swapped in player */
            mover_new(plevel, plevel->player_x[plevel->player], plevel->player_y[plevel->player], MOVE_SWAP, PIECE_PLAYER_ONE +  plevel->player, 0);
        }

        /* Force a check for whether a redraw is needed */
        return 1;
    }

    return 0;
}

void xor_focus(struct level* plevel)
{
    if(plevel->player_x[plevel->player] <= plevel->view_x[plevel->player])
        plevel->view_x[plevel->player] = plevel->player_x[plevel->player] - 1;
    if(plevel->player_x[plevel->player] >= plevel->view_x[plevel->player] + 7)
        plevel->view_x[plevel->player] = plevel->player_x[plevel->player] - 6;
    if(plevel->player_y[plevel->player] <= plevel->view_y[plevel->player])
        plevel->view_y[plevel->player] = plevel->player_y[plevel->player] - 1;
    if(plevel->player_y[plevel->player] >= plevel->view_y[plevel->player] + 7)
        plevel->view_y[plevel->player] = plevel->player_y[plevel->player] - 6;
}

#endif

