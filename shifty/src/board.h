#ifndef _BOARD_H_
#define _BOARD_H_

#include "block.h"

#define BOARD_W				7
#define BOARD_H				15

#define BOARD_OFFSET_X_P1	4
#define BOARD_OFFSET_X_P2	210
#define BOARD_OFFSET_Y		5

#define LINE_LEN			3

#define SHIFT_TIME			15

#define COMBO_MIN			5

struct Stats;

typedef struct Shift
{
	int available;
	unsigned int ticks;
	unsigned int count;
} Shift;

typedef struct Board
{
	Block blocks[BOARD_H][BOARD_W];
	Block blocksRem[BOARD_H][BOARD_W];
	int animate;
	int x;
	int y;
	unsigned int currentHeight;
	unsigned int topHeight;
	int over;
	int canMove;
	int canClear;
	Shift shift;
	struct Stats *stats;

	/* Display. */
	int comboHeight;
	int comboTimer;
} Board;

void boardInit(Board *board, int x, int y, struct Stats *stats);
void boardApplyGravity(Board *board);
void boardClearBlocks(Board *board);
void boardShift(Board *board);
void boardCalculateMatchingBlocks(Board *board);
void boardCalculateScore(Board *board);
int boardAnimate(Board *board);
void boardUpdateHeight(Board *board);
void boardDraw(Board *board);

#endif /* _BOARD_H_ */
