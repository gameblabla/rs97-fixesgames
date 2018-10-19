#include "piece.h"
#include <stdlib.h>
#include "backend/video.h"
#include "block.h"
#include "board.h"
#include "game.h"

void pieceNew(Piece *piece)
{
	int rotate;

	if (!piece)
		return;

	memset(piece, 0, sizeof(Piece));

	piece->draw = 1;
	piece->x = 0;
	piece->y = 0;
	piece->blocks[0][0].type = (rand() % (BLOCK_TYPE_COUNT - 2)) + 1;
	piece->blocks[0][1].type = BLOCK_TYPE_EMPTY;
	piece->blocks[1][0].type = (rand() % (BLOCK_TYPE_COUNT - 2)) + 1;
	piece->blocks[1][1].type = (rand() % (BLOCK_TYPE_COUNT - 2)) + 1;

	/* Center block. */
	piece->blocks[1][0].connection = ((rand() % 2) ? BLOCK_CON_TOP : BLOCK_CON_NONE) | ((rand() % 2) ? BLOCK_CON_RIGHT : BLOCK_CON_NONE);

	if (piece->blocks[1][0].connection & BLOCK_CON_TOP)
		piece->blocks[0][0].connection = BLOCK_CON_BOTTOM;

	if (piece->blocks[1][0].connection & BLOCK_CON_RIGHT)
		piece->blocks[1][1].connection = BLOCK_CON_LEFT;

	rotate = rand() % 4;

	while (rotate--)
	{
		pieceRotate(piece, NULL, 1);
	}

	/* Generate random junk blocks in training mode. */
	if (game.mode == GAME_MODE_SOLO && game.modifiers.junkBlocks && !(rand() % PIECE_RANDOM_JUNK_CHANCE))
	{
		piecePenalty(piece, (rand() % 3) + 1);
	}
}

void pieceCenter(Piece *piece)
{
	if (!piece)
		return;

	piece->x = BOARD_W/2;

	/* Re-center the piece if facing left. */
	if (piece->blocks[0][0].type == BLOCK_TYPE_EMPTY || piece->blocks[1][0].type == BLOCK_TYPE_EMPTY)
		--piece->x;	
}

void pieceRotate(Piece *piece, Board *board, int clockwise)
{
	Piece copy;

	if (!piece)
		return;

	if (board)
	{
		int y;

		for (y = 0; y < PIECE_SIZE; ++y)
		{
			int x;

			for (x = 0; x < PIECE_SIZE; ++x)
			{
				if (piece->blocks[y][x].type == BLOCK_TYPE_EMPTY && board->blocks[piece->y + y][piece->x + x].type != BLOCK_TYPE_EMPTY)
					return;
			}
		}
	}

	memcpy(&copy, piece, sizeof(Piece));

	if (clockwise)
	{
		piece->blocks[0][0].type = copy.blocks[1][0].type;
		piece->blocks[0][1].type = copy.blocks[0][0].type;
		piece->blocks[1][0].type = copy.blocks[1][1].type;
		piece->blocks[1][1].type = copy.blocks[0][1].type;

		piece->blocks[0][0].connection = copy.blocks[1][0].connection << 1;
		piece->blocks[0][1].connection = copy.blocks[0][0].connection << 1;
		piece->blocks[1][0].connection = copy.blocks[1][1].connection << 1;
		piece->blocks[1][1].connection = copy.blocks[0][1].connection << 1;

		piece->blocks[0][0].connection |= (piece->blocks[0][0].connection & (1 << 4)) >> 4;
		piece->blocks[0][1].connection |= (piece->blocks[0][1].connection & (1 << 4)) >> 4;
		piece->blocks[1][0].connection |= (piece->blocks[1][0].connection & (1 << 4)) >> 4;
		piece->blocks[1][1].connection |= (piece->blocks[1][1].connection & (1 << 4)) >> 4;

		piece->blocks[0][0].connection &= 0xf;
		piece->blocks[0][1].connection &= 0xf;
		piece->blocks[1][0].connection &= 0xf;
		piece->blocks[1][1].connection &= 0xf;
	}
	else
	{
		piece->blocks[0][0].type = copy.blocks[0][1].type;
		piece->blocks[0][1].type = copy.blocks[1][1].type;
		piece->blocks[1][0].type = copy.blocks[0][0].type;
		piece->blocks[1][1].type = copy.blocks[1][0].type;

		piece->blocks[0][0].connection = copy.blocks[0][1].connection;
		piece->blocks[0][1].connection = copy.blocks[1][1].connection;
		piece->blocks[1][0].connection = copy.blocks[0][0].connection;
		piece->blocks[1][1].connection = copy.blocks[1][0].connection;

		piece->blocks[0][0].connection |= (piece->blocks[0][0].connection & 1) << 4;
		piece->blocks[0][1].connection |= (piece->blocks[0][1].connection & 1) << 4;
		piece->blocks[1][0].connection |= (piece->blocks[1][0].connection & 1) << 4;
		piece->blocks[1][1].connection |= (piece->blocks[1][1].connection & 1) << 4;

		piece->blocks[0][0].connection = (piece->blocks[0][0].connection >> 1) & 0xf;
		piece->blocks[0][1].connection = (piece->blocks[0][1].connection >> 1) & 0xf;
		piece->blocks[1][0].connection = (piece->blocks[1][0].connection >> 1) & 0xf;
		piece->blocks[1][1].connection = (piece->blocks[1][1].connection >> 1) & 0xf;
	}
}

int pieceCollision(Piece *piece, Board *board)
{
	int j;

	if (!piece || !board)
		return -1;

	for (j = 0; j < PIECE_SIZE; ++j)
	{
		int i;

		for (i = 0; i < PIECE_SIZE; ++i)
		{
			if (piece->blocks[j][i].type == BLOCK_TYPE_EMPTY || board->blocks[piece->y + j][piece->x + i].type == BLOCK_TYPE_EMPTY)
				continue;

			return 1;
		}
	}

	return 0;
}

void piecePenalty(Piece *piece, int penalty)
{
	int j;
	int slots = 0;

	if (!piece)
		return;

	for (j = 0; j < PIECE_SIZE; ++j)
	{
		int i;

		for (i = 0; i < PIECE_SIZE; ++i)
		{
			if (piece->blocks[j][i].type != BLOCK_TYPE_EMPTY && piece->blocks[j][i].type != BLOCK_TYPE_JUNK)
				++slots;
		}
	}

	if (penalty > slots)
		penalty = slots;

	while (penalty)
	{
		int slot = rand() % (PIECE_SIZE * PIECE_SIZE);
		int y = slot/PIECE_SIZE;
		int x = slot%PIECE_SIZE;

		if (piece->blocks[y][x].type == BLOCK_TYPE_EMPTY || piece->blocks[y][x].type == BLOCK_TYPE_JUNK)
			continue;

		piece->blocks[y][x].type = BLOCK_TYPE_JUNK;
		--penalty;
	}
}

void pieceDraw(Piece *piece, int x, int y)
{
	int j;

	if (!piece || !blocksTileset)
		return;

	if (!piece->draw)
		return;

	for (j = 0; j < PIECE_SIZE; ++j)
	{
		int i;

		for (i = 0; i < PIECE_SIZE; ++i)
		{
			int blockOffset = piece->blocks[j][i].type - 1;
			int conOffset;

			if (piece->blocks[j][i].type == BLOCK_TYPE_EMPTY)
				continue;

			switch (piece->blocks[j][i].connection)
			{
				case BLOCK_CON_TOP:
					conOffset = 1;
				break;
				case BLOCK_CON_BOTTOM:
					conOffset = 2;
				break;
				case BLOCK_CON_LEFT:
					conOffset = 3;
				break;
				case BLOCK_CON_RIGHT:
					conOffset = 4;
				break;
				case BLOCK_CON_TOP_RIGHT:
					conOffset = 5;
				break;
				case BLOCK_CON_TOP_LEFT:
					conOffset = 6;
				break;
				case BLOCK_CON_BOTTOM_RIGHT:
					conOffset = 7;
				break;
				case BLOCK_CON_BOTTOM_LEFT:
					conOffset = 8;
				break;

				default:
					conOffset = 0;
				break;
			}

			blockOffset += conOffset * 5;

			drawImage(blocksTileset->image, &blocksTileset->clip[blockOffset], x + BLOCK_SIZE * piece->x + BLOCK_SIZE * i, y + BLOCK_SIZE * piece->y + BLOCK_SIZE * j);
		}
	}

	if (piece->drawShadow)
		drawDashedLine(x + piece->x * BLOCK_SIZE, BOARD_OFFSET_Y + BOARD_H * BLOCK_SIZE + 3, 1, PIECE_SIZE * BLOCK_SIZE);
}
