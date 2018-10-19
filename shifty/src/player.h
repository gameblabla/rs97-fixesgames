#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "backend/input.h"
#include "ai.h"
#include "block.h"
#include "board.h"
#include "piece.h"

#define PLAYER_SPEED		30
#define KEY_HOLD_MAX		10
#define COMBO_TIMER			120

typedef struct Stats
{
	unsigned int matchedBlocks;
	unsigned int combo;
	unsigned int linesHorizontal;
	unsigned int linesVertical;
	unsigned int blocks;
	unsigned int pieces;
	unsigned int score;
} Stats;

typedef struct Player {
	int local;
	AI ai;
	int *keyMap[KEY_COUNT];
	unsigned int keyHold;
	unsigned int speed;
	Board board;
	Piece piece;
	Piece pieceNext;
	unsigned int ticks;
	int newPiece;
	int canPenalize;
	Stats stats;
} Player;

extern Player player1;
extern Player player2;

void playerLogic(Player *player);
void playerMove(Player *player);

#endif /* _PLAYER_H_ */
