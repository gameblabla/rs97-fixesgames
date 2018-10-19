#ifndef _PIECE_H_
#define _PIECE_H_

#include "block.h"
#include "board.h"

#define PIECE_SIZE					2

#define PIECE_NEXT_X_P1				115
#define PIECE_NEXT_Y_P1				41
#define PIECE_NEXT_X_P2				174
#define PIECE_NEXT_Y_P2				41

#define PIECE_RANDOM_JUNK_CHANCE	10

typedef struct Piece
{
	Block blocks[PIECE_SIZE][PIECE_SIZE];
	int x;
	int y;
	int draw;
	int drawShadow;
} Piece;

void pieceNew(Piece *piece);
void pieceCenter(Piece *piece);
void pieceRotate(Piece *piece, Board *board, int clockwise);
int pieceCollision(Piece *piece, Board *board);
void piecePenalty(Piece *piece, int penalty);
void pieceDraw(Piece *piece, int x, int y);

#endif /* _PIECE_H_ */
