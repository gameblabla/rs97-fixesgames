/*  
    actions.h

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

enum
{
    ACTION_NONE,
    ACTION_QUIT,
    ACTION_REDRAW,
    ACTION_FAST,
    ACTION_LEFT,
    ACTION_RIGHT,
    ACTION_UP,
    ACTION_DOWN,
    ACTION_SWAP,
    ACTION_UNDO,
    ACTION_REDO,
    ACTION_PIECE_LEFT,
    ACTION_PIECE_RIGHT,
    ACTION_HIDE,
    ACTION_PAUSE,
    ACTION_FASTER,
    ACTION_SLOWER,

    ACTION_MOUSE_CLICK,
    ACTION_MOUSE_DRAG,
    ACTION_MOUSE_DRAG_OR_CLICK,

    ACTION_MAX
};

#define ACTION_PAGE_UP          ACTION_PIECE_LEFT
#define ACTION_PAGE_DOWN        ACTION_PIECE_RIGHT
#define ACTION_ENTER            ACTION_SWAP
#define ACTION_DELETE           ACTION_UNDO

#define ACTION_KEY_MIN          ACTION_NONE
#define ACTION_KEY_MAX          ACTION_SLOWER + 1
