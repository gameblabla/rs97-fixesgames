/*  
    level.h

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

#define MOVE_LEFT	        0
#define MOVE_UP		        1
#define MOVE_RIGHT	        2
#define MOVE_DOWN	        3
#define MOVE_NONE	        4
#define MOVE_SWAP	        5
#define MOVE_SWAPPED	        6
#define MOVE_UNKNOWN	        7
#define MOVE_REDO	        8

#define MOVER_FAST	        1
#define MOVER_UNDO	        2
#define MOVER_STORE	        4

#define LEVELFLAG_MOVES		1
#define LEVELFLAG_STARS	        2
#define LEVELFLAG_SWITCH	4
#define LEVELFLAG_EXIT		8
#define LEVELFLAG_SOLVED	16
#define LEVELFLAG_FAILED	32
#define LEVELFLAG_PAUSED	64
#define LEVELFLAG_UNDO		128
#define LEVELFLAG_TESTING       256
#define LEVELFLAG_MAP		512
#define LEVELFLAG_NOUNDO        1024

#ifdef XOR_COMPATIBILITY
#define MAPPED_TOP_LEFT		1
#define MAPPED_TOP_RIGHT	2
#define MAPPED_BOTTOM_LEFT	4
#define MAPPED_BOTTOM_RIGHT	8
#endif

enum {
    PIECE_SPACE = 0,
    PIECE_WALL = 1,
    PIECE_PLAYER_ONE = 2,
    PIECE_PLAYER_TWO = 3,
    PIECE_STAR = 4,
    PIECE_DOTS = 5,
    PIECE_DOOR = 6,
    PIECE_CIRCLE = 7,
    PIECE_ARROW_RED_LEFT = 8,           /* PIECE_ARROW_RED_LEFT % 4 must = 0 */
    PIECE_ARROW_RED_UP = 9,
    PIECE_ARROW_RED_RIGHT = 10,
    PIECE_ARROW_RED_DOWN = 11,
    PIECE_BOMB_RED_LEFT = 12,
    PIECE_BOMB_RED_UP = 13,
    PIECE_BOMB_RED_RIGHT = 14,
    PIECE_BOMB_RED_DOWN = 15,
    PIECE_ARROW_GREEN_LEFT = 16,
    PIECE_ARROW_GREEN_UP = 17,
    PIECE_ARROW_GREEN_RIGHT = 18,
    PIECE_ARROW_GREEN_DOWN = 19,
    PIECE_BOMB_GREEN_LEFT = 20,
    PIECE_BOMB_GREEN_UP = 21,
    PIECE_BOMB_GREEN_RIGHT = 22,
    PIECE_BOMB_GREEN_DOWN = 23,
    PIECE_ARROW_BLUE_LEFT = 24,
    PIECE_ARROW_BLUE_UP = 25,
    PIECE_ARROW_BLUE_RIGHT = 26,
    PIECE_ARROW_BLUE_DOWN = 27,
    PIECE_BOMB_BLUE_LEFT = 28,
    PIECE_BOMB_BLUE_UP = 29,
    PIECE_BOMB_BLUE_RIGHT = 30,
    PIECE_BOMB_BLUE_DOWN = 31,

#ifdef ENIGMA_COMPATIBILITY
    PIECE_CIRCLE_DOUBLE,
    PIECE_DOTS_DOUBLE,
#endif

#ifdef XOR_COMPATIBILITY
    PIECE_DOTS_X,
    PIECE_DOTS_Y,
    PIECE_SWITCH,
    PIECE_TELEPORT,
    PIECE_MAP_TOP_LEFT,
    PIECE_MAP_TOP_RIGHT,
    PIECE_MAP_BOTTOM_LEFT,
    PIECE_MAP_BOTTOM_RIGHT,
    PIECE_DARKNESS,
#endif

    PIECE_EXPLOSION_RED_LEFT,
    PIECE_EXPLOSION_RED_HORIZONTAL,
    PIECE_EXPLOSION_RED_RIGHT,
    PIECE_EXPLOSION_RED_TOP,
    PIECE_EXPLOSION_RED_VERTICAL,
    PIECE_EXPLOSION_RED_BOTTOM,
    PIECE_EXPLOSION_GREEN_LEFT,
    PIECE_EXPLOSION_GREEN_HORIZONTAL,
    PIECE_EXPLOSION_GREEN_RIGHT,
    PIECE_EXPLOSION_GREEN_TOP,
    PIECE_EXPLOSION_GREEN_VERTICAL,
    PIECE_EXPLOSION_GREEN_BOTTOM,
    PIECE_EXPLOSION_BLUE_LEFT,
    PIECE_EXPLOSION_BLUE_HORIZONTAL,
    PIECE_EXPLOSION_BLUE_RIGHT,
    PIECE_EXPLOSION_BLUE_TOP,
    PIECE_EXPLOSION_BLUE_VERTICAL,
    PIECE_EXPLOSION_BLUE_BOTTOM,

    PIECE_EXPLOSION_NEW_RED_LEFT,
    PIECE_EXPLOSION_NEW_RED_HORIZONTAL,
    PIECE_EXPLOSION_NEW_RED_RIGHT,
    PIECE_EXPLOSION_NEW_RED_TOP,
    PIECE_EXPLOSION_NEW_RED_VERTICAL,
    PIECE_EXPLOSION_NEW_RED_BOTTOM,
    PIECE_EXPLOSION_NEW_GREEN_LEFT,
    PIECE_EXPLOSION_NEW_GREEN_HORIZONTAL,
    PIECE_EXPLOSION_NEW_GREEN_RIGHT,
    PIECE_EXPLOSION_NEW_GREEN_TOP,
    PIECE_EXPLOSION_NEW_GREEN_VERTICAL,
    PIECE_EXPLOSION_NEW_GREEN_BOTTOM,
    PIECE_EXPLOSION_NEW_BLUE_LEFT,
    PIECE_EXPLOSION_NEW_BLUE_HORIZONTAL,
    PIECE_EXPLOSION_NEW_BLUE_RIGHT,
    PIECE_EXPLOSION_NEW_BLUE_TOP,
    PIECE_EXPLOSION_NEW_BLUE_VERTICAL,
    PIECE_EXPLOSION_NEW_BLUE_BOTTOM,

    PIECE_CURSOR,
    PIECE_GONE,
    PIECE_UNKNOWN
};

#define PIECE_EXPLOSION_FIRST	PIECE_EXPLOSION_RED_LEFT
#define PIECE_EXPLOSION_LAST	PIECE_EXPLOSION_BLUE_BOTTOM
#define PIECE_EXPLOSION_NEW_FIRST	PIECE_EXPLOSION_NEW_RED_LEFT
#define PIECE_EXPLOSION_NEW_LAST	PIECE_EXPLOSION_NEW_BLUE_BOTTOM
#define PIECE_EXPLOSION_NEW_OFFSET	(PIECE_EXPLOSION_NEW_FIRST - PIECE_EXPLOSION_FIRST)
#define PIECE_MOVERS_FIRST	PIECE_ARROW_RED_LEFT
#define PIECE_MOVERS_LAST	PIECE_BOMB_BLUE_DOWN
#define PIECE_MAX		PIECE_GONE

#define isexplosion(x) (x >= PIECE_EXPLOSION_FIRST && x<= PIECE_EXPLOSION_LAST)
#define isnewexplosion(x) (x >= PIECE_EXPLOSION_NEW_FIRST && x <= PIECE_EXPLOSION_NEW_LAST)

enum {
    MODE_CHROMA,
#ifdef XOR_COMPATIBILITY
    MODE_XOR,
#endif
#ifdef ENIGMA_COMPATIBILITY
    MODE_ENIGMA,
#endif
    MODE_MAX
};

struct mover
{
    int x;
    int y;
    int direction;
    int piece;
    int piece_previous;
    int fast;
    struct mover* next;
    struct mover* previous;
};

struct move
{
    int count;
    int direction;
    struct move* previous;
    struct move* next;
    struct mover* mover_first;
    struct mover* mover_last;
};

struct level
{
    int size_x;
    int size_y;

    char *data_pieces;
    char *data_moving;
    char *data_previous;
    char *data_previousmoving;
    char *data_detonator;
    char *data_detonatormoving;
    unsigned int *data_data;

    struct move* move_first;
    struct move* move_last;
    struct move* move_current;

    struct mover* mover_first;
    struct mover* mover_last;

    struct mover* stack_first;
    struct mover* stack_last;

    int player;
    int player_x[3];
    int player_y[3];
    int view_x[3];
    int view_y[3];
    int alive[2];
   
    int moves;

    int stars_caught;
    int stars_exploded;
    int stars_total;

    char *title;

    int flags;

#ifdef XOR_COMPATIBILITY
    int switched;
    int mapped;
#endif

    int mode;
    int level;

    int teleport_x[2];
    int teleport_y[2];
    int view_teleport_x[2];
    int view_teleport_y[2];
};

/* level.c */
char piecetochar(int piece);
int chartopiece(char c);
char directiontochar(int direction);
int chartodirection(char c);

struct level* level_new();
struct level* level_load(char *filename, int partial);
int level_save(struct level* plevel, char *filename, int partial);
struct level* level_copy(struct level*);
void level_addmove(struct level*, int);
void level_fix(struct level*);
struct level* level_create(int, int);
void level_delete(struct level* plevel);

char level_piece(struct level*, int, int);
void level_setpiece(struct level*, int, int, char);
char level_moving(struct level*, int, int);
void level_setmoving(struct level*, int, int, char);
char level_previous(struct level*, int, int);
void level_setprevious(struct level*, int, int, char);
char level_detonator(struct level*, int, int);
void level_setdetonator(struct level*, int, int, char);
char level_detonatormoving(struct level*, int, int);
void level_setdetonatormoving(struct level*, int, int, char);
char level_previousmoving(struct level*, int, int);
void level_setpreviousmoving(struct level*, int, int, char);
unsigned int level_data(struct level*, int, int);
void level_setdata(struct level*, int, int, unsigned int);
void level_settitle(struct level* plevel, char *title);

/* engine.c */
int level_move(struct level*, int);
int level_evolve(struct level*);
int canfall(int p, int into, int d);
struct mover* mover_new(struct level* plevel, int x, int y, int d, int piece, int fast);
struct mover* mover_addtostack(struct level* plevel, int x, int y, int move);
void level_storemovers(struct level*);
int level_undo(struct level*);
struct mover* mover_newundo(struct level* plevel, int x, int y, int d, int piece, int previous, int flags);
