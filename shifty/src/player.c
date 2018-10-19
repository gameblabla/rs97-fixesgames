#include "player.h"
#include "board.h"
#include "piece.h"
#include "game.h"

Player player1;
Player player2;

void playerLogic(Player *player)
{
	if (player->board.over)
		return;

	if (!player->local)
		return;

	++player->ticks;

	if (player->newPiece)
	{
		Piece tmpPiece = player->pieceNext;
		pieceCenter(&tmpPiece);

		if (player->board.canMove || !pieceCollision(&tmpPiece, &player->board))
		{
			player->newPiece = 0;
			player->ai.mode = AI_ANALYZE;

			player->canPenalize = game.modifiers.junkBlocks;
			player->piece = player->pieceNext;
			player->piece.drawShadow = 1;

			pieceCenter(&player->piece);
			++player->stats.pieces;
			pieceNew(&player->pieceNext);

			if (pieceCollision(&player->piece, &player->board) > 0)
			{
				player->board.over = 1;
			}
		}
		else
		{
			player->piece.draw = 0;
		}
	}

	if (player->board.canMove)
	{
		if (player->board.comboTimer)
			--player->board.comboTimer;
	}

	playerMove(player);

	if (!player->board.canMove)
	{
		if (boardAnimate(&player->board))
			return;

		/* Temporary. */
		if (player->ticks % GRAVITY_SPEED)
			return;
		
		memset(&player->board.blocksRem, 0, sizeof(Block)*BOARD_W*BOARD_H);

		boardApplyGravity(&player->board);

		if (player->board.canClear)
		{
			boardClearBlocks(&player->board);
		}

		if (player->board.canMove)
		{
			boardCalculateScore(&player->board);

			++player->board.shift.ticks;
		}
	}

	if (player->board.canMove && player->canPenalize && player->stats.combo >= COMBO_MIN)
	{
		int penalty = player->stats.combo / COMBO_MIN;

		player->canPenalize = 0;

		if (game.mode == GAME_MODE_LOCAL || game.mode == GAME_MODE_NETWORKED)
		{
			piecePenalty(player == &player1 ? &player2.pieceNext : &player1.pieceNext, penalty);
		}
	}
}

void playerMove(Player *player)
{
	int y;
	int newX = 0;
	int collision = 0;
	int dropPiece = 0;
	Piece *piece = &player->piece;
	Board *board = &player->board;

	if (!piece || !board)
		return;

	if (player->ai.enabled)
		aiMove(player);

	if (board->canMove && *player->keyMap[KEY_DROP])
	{
		*player->keyMap[KEY_DROP] = 0;

		player->ticks = 0;
		dropPiece = 1;
	}

	if (board->canMove && *player->keyMap[KEY_DOWN])
	{
		player->ticks = 0;
	}

	if (*player->keyMap[KEY_ROTATE_LEFT])
	{
		*player->keyMap[KEY_ROTATE_LEFT] = 0;

		pieceRotate(&player->piece, &player->board, 0);
	}
	else if (*player->keyMap[KEY_ROTATE_RIGHT])
	{
		*player->keyMap[KEY_ROTATE_RIGHT] = 0;

		pieceRotate(&player->piece, &player->board, 1);
	}

	if (*player->keyMap[KEY_LEFT])
	{
		if (!player->keyHold || ++player->keyHold >= KEY_HOLD_MAX)
		{
			if (player->keyHold)
			{
				player->keyHold = 0;
			}

			++player->keyHold;
			newX = -1;
		}
	}
	else if (*player->keyMap[KEY_RIGHT])
	{
		if (!player->keyHold || ++player->keyHold >= KEY_HOLD_MAX)
		{
			if (player->keyHold)
			{
				player->keyHold = 0;
			}

			++player->keyHold;
			newX = 1;
		}
	}
	else
	{
		player->keyHold = 0;
	}

	if ((piece->x + newX >= 0) && (piece->x + newX <= BOARD_W - 2))
	{
		piece->x += newX;

		if (pieceCollision(piece, board))
		{
			piece->x -= newX;
		}
	}

	/* Temporary. */
	if (!board->canMove || player->ticks % player->speed)
		return;

	do
	{
		for (y = 0; y < PIECE_SIZE; ++y)
		{
			int x;

			for (x = 0; x < PIECE_SIZE; ++x)
			{
				if (piece->blocks[y][x].type == BLOCK_TYPE_EMPTY)
					continue;

				if ((board->blocks[piece->y + y + 1][piece->x + x].type != BLOCK_TYPE_EMPTY) || (piece->y + y + 1 >= BOARD_H))
				{
					for (y = 0; y < PIECE_SIZE; ++y)
					{
						for (x = 0; x < PIECE_SIZE; ++x)
						{
							if (piece->blocks[y][x].type != BLOCK_TYPE_EMPTY)
								board->blocks[piece->y + y][piece->x + x] = piece->blocks[y][x];
						}
					}

					player->newPiece = 1;
					collision = 1;

					player->stats.blocks += 3;

					/* Temporary. */
					player->stats.combo = 0;
					player->board.comboTimer = 0;

					board->canMove = 0;
					board->shift.available = 1;
				}
			}
		}

		if (collision)
		{
			dropPiece = 0;
			*player->keyMap[KEY_DOWN] = 0;
		}
		else
		{
			++piece->y;
		}
	}
	while (dropPiece);
}
