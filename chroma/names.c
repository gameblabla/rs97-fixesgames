/*  
    names.c

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

#include "chroma.h"

char* piece_name[] = {
    "SPACE",
    "WALL",
    "PLAYER_ONE",
    "PLAYER_TWO",
    "STAR", 
    "DOTS",
    "DOOR",
    "CIRCLE",
    "ARROW_RED_LEFT",
    "ARROW_RED_UP",
    "ARROW_RED_RIGHT",
    "ARROW_RED_DOWN",
    "BOMB_RED_LEFT",
    "BOMB_RED_UP",
    "BOMB_RED_RIGHT",
    "BOMB_RED_DOWN",
    "ARROW_GREEN_LEFT",
    "ARROW_GREEN_UP",
    "ARROW_GREEN_RIGHT",
    "ARROW_GREEN_DOWN",
    "BOMB_GREEN_LEFT",
    "BOMB_GREEN_UP",
    "BOMB_GREEN_RIGHT",
    "BOMB_GREEN_DOWN",
    "ARROW_BLUE_LEFT",
    "ARROW_BLUE_UP",
    "ARROW_BLUE_RIGHT",
    "ARROW_BLUE_DOWN",
    "BOMB_BLUE_LEFT",
    "BOMB_BLUE_UP",
    "BOMB_BLUE_RIGHT",
    "BOMB_BLUE_DOWN",

#ifdef ENIGMA_COMPATIBILITY
    "CIRCLE_DOUBLE",
    "DOTS_DOUBLE",
#endif

#ifdef XOR_COMPATIBILITY
    "DOTS_X",
    "DOTS_Y",
    "SWITCH",
    "TELEPORT",
    "MAP_TOP_LEFT",
    "MAP_TOP_RIGHT",
    "MAP_BOTTOM_LEFT",
    "MAP_BOTTOM_RIGHT",
    "DARKNESS", 
#endif

    "EXPLOSION_RED_LEFT",
    "EXPLOSION_RED_HORIZONTAL",
    "EXPLOSION_RED_RIGHT",
    "EXPLOSION_RED_TOP",
    "EXPLOSION_RED_VERTICAL",
    "EXPLOSION_RED_BOTTOM",
    "EXPLOSION_GREEN_LEFT",
    "EXPLOSION_GREEN_HORIZONTAL",
    "EXPLOSION_GREEN_RIGHT",
    "EXPLOSION_GREEN_TOP",
    "EXPLOSION_GREEN_VERTICAL",
    "EXPLOSION_GREEN_BOTTOM",
    "EXPLOSION_BLUE_LEFT",
    "EXPLOSION_BLUE_HORIZONTAL",
    "EXPLOSION_BLUE_RIGHT",
    "EXPLOSION_BLUE_TOP",
    "EXPLOSION_BLUE_VERTICAL",
    "EXPLOSION_BLUE_BOTTOM",
    "EXPLOSION_NEW_RED_LEFT",
    "EXPLOSION_NEW_RED_HORIZONTAL",
    "EXPLOSION_NEW_RED_RIGHT",
    "EXPLOSION_NEW_RED_TOP",
    "EXPLOSION_NEW_RED_VERTICAL",
    "EXPLOSION_NEW_RED_BOTTOM",
    "EXPLOSION_NEW_GREEN_LEFT",
    "EXPLOSION_NEW_GREEN_HORIZONTAL",
    "EXPLOSION_NEW_GREEN_RIGHT",
    "EXPLOSION_NEW_GREEN_TOP",
    "EXPLOSION_NEW_GREEN_VERTICAL",
    "EXPLOSION_NEW_GREEN_BOTTOM",
    "EXPLOSION_NEW_BLUE_LEFT",
    "EXPLOSION_NEW_BLUE_HORIZONTAL",
    "EXPLOSION_NEW_BLUE_RIGHT",
    "EXPLOSION_NEW_BLUE_TOP",
    "EXPLOSION_NEW_BLUE_VERTICAL",
    "EXPLOSION_NEW_BLUE_BOTTOM",
    "CURSOR",
    "GONE",
    "" };

char *action_name[] = {
    "Do nothing",
    "Quit",
    "Redraw screen",
    "Fast",
    "Move left",
    "Move right",
    "Move up",
    "Move down",
    "Swap players / Select",
    "Undo move / Delete",
    "Redo move",
    "Piece left / Page up",
    "Piece right / Page down",
    "Hide screen",
    "Pause",
    "Faster",
    "Slower",

    "Mouse click",
    "Mouse drag",
    "Mouse drag or click"
};

char *action_shortname[] = {
    "none",
    "quit",
    "redraw",
    "fast",
    "left",
    "right",
    "up",
    "down",
    "swap",
    "undo",
    "redo",
    "piece-left",
    "piece-right",
    "hide",
    "pause",
    "faster",
    "slower",

    "mouse-click",
    "mouse-drag",
    "mouse-drag-or-click",
};

