#include "board.h"
#include "backend/video.h"
#include "block.h"
#include "font.h"
#include "game.h"
#include "player.h"

void boardInit(Board *board, int x, int y, struct Stats *stats)
{
	if (!board)
		return;

	memset(board, 0, sizeof(Board));

	board->animate = 1;
	board->x = x;
	board->y = y;
	board->shift.count = SHIFT_TIME;
	board->stats = stats;
}

void boardApplyGravity(Board *board)
{
	int y;

	if (!board)
		return;

	board->canMove = 1;
	board->canClear = 1;

	for (y = BOARD_H - 2; y >= 0; --y)
	{
		int x;

		for (x = 0; x < BOARD_W; ++x)
		{
			if (board->blocks[y][x].type != BLOCK_TYPE_EMPTY && board->blocks[y+1][x].type == BLOCK_TYPE_EMPTY)
			{
				if (board->blocks[y][x].connection & BLOCK_CON_TOP)
				{
					if (board->blocks[y-1][x].connection & BLOCK_CON_LEFT)
					{
						if (board->blocks[y][x-1].type != BLOCK_TYPE_EMPTY)
							continue;
					}

					if (board->blocks[y-1][x].connection & BLOCK_CON_RIGHT)
					{
						if (board->blocks[y][x+1].type != BLOCK_TYPE_EMPTY)
							continue;
					}
				}

				if (board->blocks[y][x].connection & BLOCK_CON_LEFT)
				{
					if (board->blocks[y+1][x-1].type == BLOCK_TYPE_EMPTY)
					{
						board->blocks[y+1][x-1] = board->blocks[y][x-1];

						memset(&board->blocks[y][x-1], 0, sizeof(Block));
					}
					else
					{
						continue;
					}
				}

				if (board->blocks[y][x].connection & BLOCK_CON_RIGHT)
				{
					if (board->blocks[y+1][x+1].type == BLOCK_TYPE_EMPTY)
					{
						board->blocks[y+1][x+1] = board->blocks[y][x+1];

						memset(&board->blocks[y][x+1], 0, sizeof(Block));
					}
					else
					{
						continue;
					}
				}

				board->blocks[y+1][x] = board->blocks[y][x];

				memset(&board->blocks[y][x], 0, sizeof(Block));

				board->canMove = 0;
				board->canClear = 0;
			}
		}
	}
}

void boardClearBlocks(Board *board)
{
	int y;
	int x;
	int performShift = game.modifiers.shiftEvent;

	if (!board)
		return;

	/* Horizontally. */
	for (y = 0; y < BOARD_H; ++y)
	{
		int x;

		for (x = 0; x < BOARD_W; ++x)
		{
			int n;
			int count = 0;
			BlockType match = board->blocks[y][x].type;

			if (match == BLOCK_TYPE_EMPTY || match == BLOCK_TYPE_JUNK)
				continue;

			for (n = x; n < BOARD_W; ++n)
			{
				if (board->blocks[y][n].type != match)
					break;

				++count;
			}

			if (count >= LINE_LEN)
			{
				int offX = x;

				if (x + count < BOARD_W && board->blocks[y][x+count].type == BLOCK_TYPE_JUNK)
					++count;

				if (x > 0 && board->blocks[y][x-1].type == BLOCK_TYPE_JUNK)
				{
					++count;
					--offX;
				}

				for (n = 0; n < count; ++n)
				{
					memcpy(&board->blocksRem[y][offX+n], &board->blocks[y][offX+n], sizeof(Block));

					if (y > 0)
						board->blocks[y-1][offX+n].connection &= ~BLOCK_CON_BOTTOM;

					if (y < BOARD_H - 1)
						board->blocks[y+1][offX+n].connection &= ~BLOCK_CON_TOP;
				}

				if (offX > 0)
					board->blocks[y][offX-1].connection &= ~BLOCK_CON_RIGHT;

				if (offX + count < BOARD_W)
					board->blocks[y][offX+count].connection &= ~BLOCK_CON_LEFT;

				x += count;

				if (board->stats)
				{
					++board->stats->linesHorizontal;
					board->stats->combo += count;
				}
				board->canMove = 0;
			}
		}
	}

	/* Vertically. */
	for (x = 0; x < BOARD_W; ++x)
	{
		int y;

		for (y = 0; y < BOARD_H; ++y)
		{
			int n;
			int count = 0;
			BlockType match = board->blocks[y][x].type;

			if (match == BLOCK_TYPE_EMPTY || match == BLOCK_TYPE_JUNK)
				continue;

			for (n = y; n < BOARD_H; ++n)
			{
				if (board->blocks[n][x].type != match)
					break;

				++count;
			}

			if (count >= LINE_LEN)
			{
				int offY = y;

				if (y + count < BOARD_H && board->blocks[y+count][x].type == BLOCK_TYPE_JUNK)
					++count;

				if (y > 0 && board->blocks[y-1][x].type == BLOCK_TYPE_JUNK)
				{
					++count;
					--offY;
				}

				for (n = 0; n < count; ++n)
				{
					memcpy(&board->blocksRem[offY+n][x], &board->blocks[offY+n][x], sizeof(Block));

					if (x > 0)
						board->blocks[offY+n][x-1].connection &= ~BLOCK_CON_RIGHT;

					if (x < BOARD_W - 1)
						board->blocks[offY+n][x+1].connection &= ~BLOCK_CON_LEFT;
				}

				if (offY > 0)
					board->blocks[offY-1][x].connection &= ~BLOCK_CON_BOTTOM;

				if (offY + count < BOARD_H)
					board->blocks[offY+count][x].connection &= ~BLOCK_CON_TOP;

				y += count;

				if (board->stats)
				{
					++board->stats->linesVertical;
					board->stats->combo += count;
				}

				board->canMove = 0;
			}
		}
	}

	for (y = 0; y < BOARD_H; ++y)
	{
		for (x = 0; x < BOARD_W; ++x)
		{
			if (board->blocksRem[y][x].type != BLOCK_TYPE_EMPTY)
			{
				if (board->animate)
				{
					board->blocks[y][x].animate = 1;
				}
				else
				{
					memset(&board->blocks[y][x], 0, sizeof(Block));
					if (board->stats)
						--board->stats->blocks;
				}

				performShift = 0;
			}
		}
	}

	if (performShift && board->shift.available && board->shift.ticks >= board->shift.count)
		boardShift(board);

	boardUpdateHeight(board);

	if (!board->comboTimer && board->stats->combo >= COMBO_MIN)
	{
		board->comboTimer = COMBO_TIMER;
		board->comboHeight = board->currentHeight;
	}
}

void boardShift(Board *board)
{
	int x;

	if (!board)
		return;

	for (x = 0; x < BOARD_W; ++x)
	{
		board->blocks[BOARD_H-2][x].connection &= ~BLOCK_CON_BOTTOM;

		memcpy(&board->blocksRem[BOARD_H-1][x], &board->blocks[BOARD_H-1][x], sizeof(Block));
	}

	boardClearBlocks(board);

	board->shift.ticks = 0;
	++board->shift.count;
	board->shift.available = 0;

	board->canMove = 0;
}

void boardCalculateMatchingBlocks(Board *board)
{
	int j;
	int i;
	int matches = 0;

	/* Horizontally. */
	for (j = 0; j < BOARD_H; ++j)
	{
		int i;

		for (i = 0; i < BOARD_W; ++i)
		{
			BlockType match = board->blocks[j][i].type;
			int junkPile = 0;
			int accessible = 0;
			int nEmptyBlocks = 0;
			int count = 0;
			int n;

			if (match == BLOCK_TYPE_EMPTY || match == BLOCK_TYPE_JUNK)
				continue;

			if (i > 0 && board->blocks[j][i-1].type == BLOCK_TYPE_EMPTY)
				accessible = 1;

			if (i > 0 && board->blocks[j][i-1].type == BLOCK_TYPE_JUNK)
			{
				accessible = 1;
				junkPile = 1;
			}

			for (n = i; n < BOARD_W; ++n)
			{
				if (board->blocks[j][n].type == BLOCK_TYPE_EMPTY)
				{
					accessible = 1;

					if (j > 0 && board->blocks[j-1][n].type != BLOCK_TYPE_EMPTY)
						break;
					if (j < BOARD_H-2 && board->blocks[j+3][n].type == BLOCK_TYPE_EMPTY)
						break;
					
					++nEmptyBlocks;

					if (nEmptyBlocks > 2)
						break;

					continue;
				}

				if (board->blocks[j][n].type == BLOCK_TYPE_JUNK)
				{
					accessible = junkPile ? 0 : 1;
					break;
				}

				if (board->blocks[j][n].type != match)
				{
					accessible = junkPile ? 0 : accessible;
					break;
				}

				++count;
			}

			if (accessible && (count + junkPile) > 1)
				matches += count + junkPile;

			i += count;
		}
	}

	/* Vertically. */
	for (i = 0; i < BOARD_W; ++i)
	{
		int j;

		for (j = 0; j < BOARD_H; ++j)
		{
			BlockType match = board->blocks[j][i].type;
			int junkPile = 0;
			int accessible = 0;
			int count = 0;
			int n;

			if (match == BLOCK_TYPE_EMPTY || match == BLOCK_TYPE_JUNK)
				continue;

			if (j > 0 && board->blocks[j-1][i].type == BLOCK_TYPE_EMPTY)
				accessible = 1;

			if (j > 0 && board->blocks[j-1][i].type == BLOCK_TYPE_JUNK)
			{
				accessible = 1;
				junkPile = 1;
			}

			if (!accessible)
				continue;

			for (n = j; n < BOARD_W; ++n)
			{
				if (board->blocks[n][i].type == BLOCK_TYPE_EMPTY)
				{
					accessible = 1;
					break;
				}

				if (board->blocks[n][i].type == BLOCK_TYPE_JUNK)
				{
					accessible = junkPile ? 0 : 1;
					break;
				}

				if (board->blocks[n][i].type != match)
				{
					accessible = junkPile ? 0 : accessible;
					break;
				}


				++count;
			}

			if (accessible && (count + junkPile) > 1)
				matches += count + junkPile;

			j += count;
		}
	}

	board->stats->matchedBlocks += matches;
}

void boardCalculateScore(Board *board)
{
	if (!board)
		return;

	board->stats->score += board->stats->combo * 100 * (board->stats->combo >= COMBO_MIN ? board->stats->combo : 1);

	if (!board->shift.ticks)
		board->stats->score += (board->shift.count-SHIFT_TIME) * 1000;
}

int boardAnimate(Board *board)
{
	int animInProgress = 0;
	int j;

	if (!board->animate)
		return 0;

	for (j = 0; j < BOARD_H; ++j)
	{
		int i;

		for (i = 0; i < BOARD_W; ++i)
		{
			if (board->blocks[j][i].animate)
			{
				animInProgress = 1;

				if (++board->blocks[j][i].animationCounter > BLOCK_ANIM_MAX * BLOCK_ANIM_SPEED)
				{
						memset(&board->blocks[j][i], 0, sizeof(Block));
						if (board->stats)
							--board->stats->blocks;
				}
			}
		}
	}

	return animInProgress;
}

static int boardCalculateHeight(const Board *board)
{
	int j;

	if (!board)
		return 0;

	for (j = 0; j < BOARD_H; ++j)
	{
		int i;

		for (i = 0; i < BOARD_W; ++i)
		{
			if (board->blocks[j][i].type != BLOCK_TYPE_EMPTY)
				return BOARD_H-j;
		}
	}

	return 0;
}

void boardUpdateHeight(Board *board)
{
	if (!board)
		return;

	board->currentHeight = boardCalculateHeight(board);

	if (board->currentHeight > board->topHeight)
		board->topHeight = board->currentHeight;
}

void boardDraw(Board *board)
{
	int j;

	if (!board || !blocksTileset)
		return;

	if (!board->over)
		drawDashedLine(board->x, board->y + (BOARD_H - board->topHeight) * BLOCK_SIZE - 1, 1, BOARD_W * BLOCK_SIZE);

	for (j = 0; j < BOARD_H; ++j)
	{
		int i;

		for (i = 0; i < BOARD_W; ++i)
		{
			if (board->blocks[j][i].animate)
			{
				int offset = board->blocks[j][i].animationCounter / BLOCK_ANIM_SPEED;

				drawImage(blocksTileset->image, &blocksTileset->clip[BLOCK_ANIM_OFFSET + offset], board->x + BLOCK_SIZE * i, board->y + BLOCK_SIZE * j);
			}
			else
			{
				int blockOffset = board->blocks[j][i].type - 1;
				int conOffset;

				if (board->blocks[j][i].type == BLOCK_TYPE_EMPTY)
					continue;

				switch (board->blocks[j][i].connection)
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

				drawImage(blocksTileset->image, &blocksTileset->clip[blockOffset], board->x + BLOCK_SIZE * i, board->y + BLOCK_SIZE * j);
			}
		}
	}

	if (board->comboTimer)
	{
		char comboTxt[50];
		int y = (BOARD_H - board->comboHeight - 2);

		y = board->y + (y < 0 ? 0 : y) * BLOCK_SIZE;
		snprintf(comboTxt, 50, "Combo %dx", board->stats->combo);

		dTextCenteredAtOffset(fontDefault, SHADOW_DROP, comboTxt, y, board->x + (BOARD_W * BLOCK_SIZE)/2);
	}
}
