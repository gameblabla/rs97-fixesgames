#include "ai.h"
#include "board.h"
#include "piece.h"
#include "player.h"

static void aiAnalyze(struct Player *player)
{
	int rotation;
	Piece piece = player->piece;

	AIMove comboMove;
	AIMove matchedBlocksMove;
	AIMove heightMove;

	memset(&comboMove, 0, sizeof(AIMove));
	memset(&matchedBlocksMove, 0, sizeof(AIMove));
	memset(&heightMove, 0, sizeof(AIMove));

	comboMove.height = BOARD_H;
	matchedBlocksMove.height = BOARD_H;
	heightMove.height = BOARD_H;

	if (!player->board.canMove)
		return;

	for (rotation = 0; rotation < 4; ++rotation, pieceRotate(&piece, NULL, 1))
	{
		for (piece.x = 0; piece.x < BOARD_W-1; ++piece.x, piece.y = 0)
		{
			Board board = player->board;
			Stats stats;
			unsigned int matchedBlocks;
			int m;

			memset(&stats, 0, sizeof(Stats));
			board.stats = &stats;
			board.animate = 0;

			while (piece.y < BOARD_H-1 && !pieceCollision(&piece, &board))
			{
				++piece.y;
			}

			--piece.y;

			boardCalculateMatchingBlocks(&board);
			matchedBlocks = board.stats->matchedBlocks;

			board.stats->matchedBlocks = 0;

			for (m = 0; m < PIECE_SIZE; ++m)
			{
				int n;

				for (n = 0; n < PIECE_SIZE; ++n)
				{
					if (piece.blocks[m][n].type != BLOCK_TYPE_EMPTY)
						board.blocks[piece.y + m][piece.x + n] = piece.blocks[m][n];
				}
			}

			board.canMove = 0;
			board.canClear = 0;

			while (!board.canMove)
			{
				memset(&board.blocksRem, 0, sizeof(Block)*BOARD_W*BOARD_H);
				
				boardApplyGravity(&board);

				if (board.canClear)
				{
					boardClearBlocks(&board);
				}
			}

			boardCalculateMatchingBlocks(&board);

			matchedBlocks = (board.stats->matchedBlocks > matchedBlocks) ? board.stats->matchedBlocks - matchedBlocks : 0;

/*
comboCheck:
*/
			if (board.stats->combo >= comboMove.combo)
			{
				if (board.stats->combo == comboMove.combo && (piece.y < (int)comboMove.height || (piece.y == (int)comboMove.height && rand() % 2)))
					goto matchCheck;

				comboMove.target.x = piece.x;
				comboMove.target.y = piece.y;
				comboMove.target.rotation = rotation;
				comboMove.combo = board.stats->combo;
				comboMove.matchedBlocks = matchedBlocks;
				comboMove.height = BOARD_H-1 - piece.y;
			}

matchCheck:
			if (matchedBlocks >= matchedBlocksMove.matchedBlocks && BOARD_H-1 - piece.y < BOARD_H-2)
			{
				if (matchedBlocks == matchedBlocksMove.matchedBlocks && (board.currentHeight > matchedBlocksMove.height || (board.currentHeight == matchedBlocksMove.height && rand() % 2)))
					goto heightCheck;

				matchedBlocksMove.target.x = piece.x;
				matchedBlocksMove.target.y = piece.y;
				matchedBlocksMove.target.rotation = rotation;
				matchedBlocksMove.combo = board.stats->combo;
				matchedBlocksMove.matchedBlocks = matchedBlocks;
				matchedBlocksMove.height = BOARD_H-1 - piece.y;
			}

heightCheck:
			if (BOARD_H-1 - piece.y <= (int)heightMove.height)
			{
				if (BOARD_H-1 - piece.y == (int)heightMove.height && rand() % 2)
					continue;

				heightMove.target.x = piece.x;
				heightMove.target.y = piece.y;
				heightMove.target.rotation = rotation;
				heightMove.combo = board.stats->combo;
				heightMove.matchedBlocks = matchedBlocks;
				heightMove.height = BOARD_H-1 - piece.y;
			}
		}
	}

	if (matchedBlocksMove.matchedBlocks && matchedBlocksMove.matchedBlocks + matchedBlocksMove.combo >= comboMove.combo && (!comboMove.combo || matchedBlocksMove.height < comboMove.height || matchedBlocksMove.height - comboMove.height < 4))
	{
		player->ai.target = matchedBlocksMove.target;
	}
	else if (comboMove.combo)
	{
		player->ai.target = comboMove.target;
	}
	else
	{
		player->ai.target = heightMove.target;
	}

	player->ai.mode = AI_MOVE;
}

void aiMove(struct Player *player) /* aiLogic. */ 
{
	switch (player->ai.mode)
	{
		case AI_MOVE:
			*player->keyMap[KEY_LEFT] = 0;
			*player->keyMap[KEY_RIGHT] = 0;
			*player->keyMap[KEY_DOWN] = 0;
			*player->keyMap[KEY_DROP] = 0;
			*player->keyMap[KEY_ROTATE_LEFT] = 0;
			*player->keyMap[KEY_ROTATE_RIGHT] = 0;

			if (player->ai.target.rotation)
			{
				--player->ai.target.rotation;
				*player->keyMap[KEY_ROTATE_RIGHT] = 1;
			}
		
			if (player->ai.target.x < player->piece.x)
			{
				*player->keyMap[KEY_LEFT] = 1;
			}
			else if (player->ai.target.x > player->piece.x)
			{
				*player->keyMap[KEY_RIGHT] = 1;
			}
			else
			{
				*player->keyMap[KEY_DOWN] = 0;
				*player->keyMap[KEY_DROP] = 0;
			}
 		break;

		case AI_ANALYZE:
			aiAnalyze(player);
		break;

		default:
		break;
	}
}
