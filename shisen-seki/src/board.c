#include "board.h"

#include <stdlib.h>
#include "audio.h"
#include "fileio.h"
#include "font.h"
#include "game.h"
#include "stone.h"
#include "tileset.h"
#include "title.h"
#include "video.h"

algorithm currentAlgorithm = ALGO_REVERSE;
tileset stonesTileset;
stone **stones;
stone stoneA;
stone stoneB;
line lineC;
int stonesLeft;
int crossing;
int cursorX = 1;
int cursorY = 1;
int generating;
int checkingMoves;
int fadeOutTimer;

void boardSetAlpha(int alpha)
{
	int i;
	int j;

	if (!stones)
	{
		return;
	}

	for (i = 0; i < BOARD_W; ++i)
	{
		for (j = 0; j < BOARD_H; ++j)
		{
			stones[i][j].alpha = alpha % 256;
		}
	}
}

void boardFadeOutSelectedStones()
{
	if (fadeOutTimer > 0)
	{
		--fadeOutTimer;

		stones[stoneA.x][stoneA.y].alpha -= 255/FADE_DELAY;
		stones[stoneB.x][stoneB.y].alpha -= 255/FADE_DELAY;

		if (stones[stoneA.x][stoneA.y].alpha < 0)
			stones[stoneA.x][stoneA.y].alpha = 0;
		if (stones[stoneB.x][stoneB.y].alpha < 0)
			stones[stoneB.x][stoneB.y].alpha = 0;

		if (fadeOutTimer <= 0)
		{
			boardRemoveSelectedStones();
		}
	}
}

void boardRemoveSelectedStones()
{
	line tmpLineC;

	stones[stoneA.x][stoneA.y].alpha = 255;
	stones[stoneB.x][stoneB.y].alpha = 255;
	stones[stoneA.x][stoneA.y].type = STONE_EMPTY;
	stones[stoneB.x][stoneB.y].type = STONE_EMPTY;

	tmpLineC = lineC;
	stonesLeft -= 2;

	if (!generating && !checkingMoves && currentGameMode == GAME_MODE_GRAVITY)
	{
		boardApplyGravity();
	}

	if (!boardCheckAvailableMoves())
	{
		gameOver = 1;

		if (!stonesLeft && !practice)
		{
			gamePrepareHiscore();
		}
	}

	crossing = 0;
	lineC = tmpLineC;
}

int boardStoneSurrounded(stone *st)
{
	int n;
	int e;
	int s;
	int w;

	if (!st)
	{
		return -1;
	}

	// Check board boundaries.
	n = (st->y <= 2) ? 1 : stones[st->x][st->y-1].type;
	e = (st->x >= BOARD_W - 2) ? 1 : stones[st->x+1][st->y].type;
	s = (st->y >= BOARD_H - 2) ? 1 : stones[st->x][st->y+1].type;
	w = (st->x <= 2) ? 1 : stones[st->x-1][st->y].type;

	return (n && e && s && w) ? 1 : 0;
}

int boardCheckAvailableMoves()
{
	int moveExists = 0;
	int i;
	int j;
	int k;
	int l;

	if (!stones)
	{
		return 0;
	}

	checkingMoves = 1;

	if (practice)
	{
		boardSetAlpha(128);
	}

	for (i = 1; i < BOARD_W - 1; ++i)
	{
		for (j = 1; j < BOARD_H - 1; ++j)
		{
			int typeA;
			stone stoneA;

			typeA = stones[i][j].type;

			if (typeA == STONE_EMPTY)
			{
				continue;
			}

			stoneA.x = i;
			stoneA.y = j;
			stoneA.type = typeA;

			for (k = 1; k < BOARD_W - 1; ++k)
			{
				for (l = 1; l < BOARD_H - 1; ++l)
				{
					int typeB;
					stone stoneB;

					typeB = stones[k][l].type;

					if (!stoneCheckMatchingTypes(typeA, typeB))
					{
						continue;
					}

					stoneB.x = k;
					stoneB.y = l;
					stoneB.type = typeB;

					if (boardCheckConnection(&stoneA, &stoneB))
					{
						stones[i][j].type = typeA;
						stones[k][l].type = typeB;

						stones[i][j].alpha = 255;
						stones[k][l].alpha = 255;

						moveExists = 1;

						if (!practice)
						{
							checkingMoves = 0;
							return 1;
						}
					}
				}
			}
		}
	}

	checkingMoves = 0;

	crossing = moveExists;
	return moveExists;
}

void boardApplyGravity()
{
	int i;
	int j;

	if (!stones)
	{
		return;
	}

	for (i = 1; i < BOARD_W - 1; ++i)
	{
		for (j = BOARD_H - 3; j > 0; --j)
		{
			if (stones[i][j].type != STONE_EMPTY)
			{
				int k;
				stone lastStone;
				lastStone.x = i;
				lastStone.y = j;
				lastStone.type = stones[i][j].type;

				for (k = j+1; k < BOARD_H - 1; ++k)
				{
					if (stones[i][k].type != STONE_EMPTY)
					{
						break;
					}

					stones[i][k].type = lastStone.type;
					stones[lastStone.x][lastStone.y].type = STONE_EMPTY;
					lastStone.y = k;
				}
			}
		}
	}
}

void boardGenerate()
{
	int pool[STONE_COUNT-1];
	int i;
	int j;

	if (!stones)
	{
		return;
	}

	generating = 1;

	memset(pool, 0, sizeof(pool));

	switch (currentAlgorithm)
	{
		case ALGO_RANDOM:
			for (i = 0; i < BOARD_W; ++i)
			{
				for (j = 0; j < BOARD_H; ++j)
				{
					if (i == 0 || j == 0 || i == BOARD_W - 1 || j == BOARD_H - 1)
					{
						stones[i][j].type = STONE_EMPTY;
					}
					else
					{
						int draw = 1;

						while (draw)
						{
							int type = 1 + rand() % (STONE_COUNT - 1);

							if (pool[type - 1] < stoneAmountOfStonesOfType(type))
							{
								++pool[type - 1];
								draw = 0;

								stones[i][j].type = type;
							}
						}
					}
				}
			}
		break;

		case ALGO_REVERSE:
			for (stonesLeft = 144; stonesLeft > 0; stonesLeft-=2)
			{
				int typeA;
				int typeB;
				int match = 0;
				int x;
				int y;

				while (!match)
				{
					typeA = 1 + rand() % (STONE_COUNT - 1);

					if (pool[typeA - 1] < stoneAmountOfStonesOfType(typeA))
					{
						match = 1;
						++pool[typeA - 1];

						if (stoneAmountOfStonesOfType(typeA) == 1)
						{
							typeB = STONE_EMPTY;
							while (typeB == STONE_EMPTY || pool[typeB - 1] >= stoneAmountOfStonesOfType(typeB) || !stoneCheckMatchingTypes(typeA, typeB))
							{
								typeB = 1 + rand() % (STONE_COUNT - 1);
							}
						}
						else
						{
							typeB = typeA;
						}

						++pool[typeB - 1];
					}
				}

				match = 0;

				while (!match)
				{
					x = 1 + rand() % (BOARD_W - 2);
					y = 1 + rand() % (BOARD_H - 2);

					if (stones[x][y].type == STONE_EMPTY)
					{
						stoneA.type = typeA;
						stones[x][y].type = stoneA.type;
						stoneA.x = x;
						stoneA.y = y;

						match = 1;
					}
				}

				match = 0;

				while (!match)
				{
					x = 1 + rand() % (BOARD_W - 2);
					y = 1 + rand() % (BOARD_H - 2);

					if (stones[x][y].type == STONE_EMPTY)
					{
						stoneA.type = typeA;
						stoneB.type = typeB;
						stones[x][y].type = stoneB.type;
						stoneB.x = x;
						stoneB.y = y;

						match = boardCheckConnection(&stoneA, &stoneB);

						if (match)
						{
							stones[stoneA.x][stoneA.y].type = typeA;
							stones[stoneB.x][stoneB.y].type = typeB;
						}
						else
						{
							if (boardStoneSurrounded(&stoneA))
							{
								match = 1;
							}
							else
							{
								stones[stoneB.x][stoneB.y].type = STONE_EMPTY;
							}
						}
					}
				}
			}
		break;

		default:
		break;
	}

	stoneA.type = STONE_EMPTY;
	stoneB.type = STONE_EMPTY;

	stonesLeft = 144;
	gameTime = 0;

	currentGameMode = newGameMode;

	generating = 0;
}

void boardSelectStone(int x, int y)
{
	crossing = 0;

	if (stones[x][y].type == STONE_EMPTY)
	{
		return;
	}

	if (stoneA.type == STONE_EMPTY)
	{
		stoneA.x = x;
		stoneA.y = y;
		stoneA.type = stones[x][y].type;
	}
	else if (x != stoneA.x || y != stoneA.y)
	{
		stoneB.x = x;
		stoneB.y = y;
		stoneB.type = stones[x][y].type;

		if (!boardCheckConnection(&stoneA, &stoneB))
		{
			stoneA = stoneB;
			stoneB.type = STONE_EMPTY;
		}
	}
}

int boardCheckHorizontalIntersection(line *A, line *B, stone *stoneA, stone *stoneB)
{
	int i;
	int j = 0;

	int maxJ;
	int dir;
	int posY;

	int x1 = A->x1 < B->x1 ? A->x1 : B->x1;
	int x2 = A->x2 > B->x2 ? A->x2 : B->x2;
	int y1 = A->y1 > B->y1 ? A->y1 : B->y1;
	int y2 = A->y2 < B->y2 ? A->y2 : B->y2;

	int stoneY1;
	int stoneY2;

	if (x1 == x2)
	{
		return 0;
	}

	if (!((A->y1 >= B->y1 && A->y1 <= B->y2) || (A->y2 >= B->y1 && A->y2 <= B->y2) || (B->y1 >= A->y1 && B->y1 <= A->y2) || (B->y2 >= A->y1 && B->y2 <= A->y2)))
	{
		return 0;
	}

	stoneY1 = (stoneA->y > stoneB->y) ? stoneB->y : stoneA->y;
	stoneY2 = (stoneA->y > stoneB->y) ? stoneA->y : stoneB->y;

	if (stoneY1 < y1)
	{
		stoneY1 = y1;
	}
	if (stoneY2 > y2)
	{
		stoneY2 = y2;
	}

	maxJ = y2 - y1;
	// Locate the center position between stones and set it as initial Y field.
	posY = stoneY1 + (stoneY2 - stoneY1)/2;
	dir = 1;

	/* Y position is calculated by starting in the center field and progressing to outer-most fields by reversing the check direction every turn.
	 Example #1:
	 [5] <- y1
	 [3]
	 [1] <- starting position
	 [2]
	 [4] <- y2

	 Example #2:
	 [5] <- y1
	 [4]
	 [3]
	 [1] <- starting position
	 [2] <- y2
	*/

	while (j <= maxJ)
	{
		for (i = x1; i <= x2; ++i)
		{
			if (stones[i][posY].type > STONE_EMPTY)
			{
				break;
			}
		}

		if (i > x2)
		{
			lineC.x1 = x1;
			lineC.x2 = x2;
			lineC.y1 = posY;
			lineC.y2 = posY;

			return 1;
		}

		++j;
		// If the next non-checked field is outside check boundaries, jump to the nearest field instead.
		if ((posY + j * dir) > y2)
		{
			--posY;
		}
		else if ((posY + j * dir) < y1)
		{
			++posY;
		}
		else // Next non-checked field is within boundaries.
		{
			posY += j * dir;	// Jump to the next non-checked inner-most field.
			dir = -dir;		// Reverse direction of checking for the next turn.
		}
	}

	return 0;
}

int boardCheckVerticalIntersection(line *A, line *B, stone *stoneA, stone *stoneB)
{
	int i = 0;
	int j;

	int maxI;
	int dir;
	int posX;

	int x1 = A->x1 > B->x1 ? A->x1 : B->x1;
	int x2 = A->x2 < B->x2 ? A->x2 : B->x2;
	int y1 = A->y1 < B->y1 ? A->y1 : B->y1;
	int y2 = A->y2 > B->y2 ? A->y2 : B->y2;

	int stoneX1;
	int stoneX2;

	if (y1 == y2)
	{
		return 0;
	}

	if (!((A->x1 >= B->x1 && A->x1 <= B->x2) || (A->x2 >= B->x1 && A->x2 <= B->x2) || (B->x1 >= A->x1 && B->x1 <= A->x2) || (B->x2 >= A->x1 && B->x2 <= A->x2)))
	{
		return 0;
	}

	stoneX1 = (stoneA->x > stoneB->x) ? stoneB->x : stoneA->x;
	stoneX2 = (stoneA->x > stoneB->x) ? stoneA->x : stoneB->x;

	if (stoneX1 < x1)
	{
		stoneX1 = x1;
	}
	if (stoneX2 > x2)
	{
		stoneX2 = x2;
	}

	maxI = x2 - x1;
	posX = stoneX1 + (stoneX2 - stoneX1)/2;
	dir = 1;

	/* See boardCheckHorizontalIntersection function comments for implementation details of below algorithm. */
	while (i <= maxI)
	{
		for (j = y1; j <= y2; ++j)
		{
			if (stones[posX][j].type > STONE_EMPTY)
			{
				break;
			}
		}

		if (j > y2)
		{
			lineC.x1 = posX;
			lineC.x2 = posX;
			lineC.y1 = y1;
			lineC.y2 = y2;

			return 1;
		}

		++i;

		if ((posX + i * dir) > x2)
		{
			--posX;
		}
		else if ((posX + i * dir) < x1)
		{
			++posX;
		}
		else
		{
			posX += i * dir;
			dir = -dir;
		}
	}

	return 0;
}

int boardCheckConnection(stone *A, stone *B)
{
	line lineA;
	line lineB;

	int i;
	int j;

	if (A == NULL || B == NULL || A->type == STONE_EMPTY || B->type == STONE_EMPTY)
	{
		return 0;
	}

	if (!stoneCheckMatchingTypes(A->type, B->type))
	{
		return 0;
	}

	stones[A->x][A->y].type = STONE_EMPTY;
	stones[B->x][B->y].type = STONE_EMPTY;

	// Horizontal field of view.
	lineA.x1 = A->x;
	lineA.y1 = A->y;
	lineA.x2 = A->x;
	lineA.y2 = A->y;

	for (i = A->x, j = A->y; i >= 0; --i)
	{
		if (stones[i][j].type != STONE_EMPTY)
		{
			break;
		}

		lineA.x1 = i;
	}
	for (i = A->x, j = A->y; i < BOARD_W; ++i)
	{
		if (stones[i][j].type != STONE_EMPTY)
		{
			break;
		}

		lineA.x2 = i;
	}

	lineB.x1 = B->x;
	lineB.y1 = B->y;
	lineB.x2 = B->x;
	lineB.y2 = B->y;

	for (i = B->x, j = B->y; i >= 0; --i)
	{
		if (stones[i][j].type != STONE_EMPTY)
		{
			break;
		}

		lineB.x1 = i;
	}
	for (i = B->x, j = B->y; i < BOARD_W; ++i)
	{
		if (stones[i][j].type != STONE_EMPTY)
		{
			break;
		}

		lineB.x2 = i;
	}

	crossing = boardCheckVerticalIntersection(&lineA, &lineB, A, B);

	if (!crossing)
	{
		// Vertical field of view.
		lineA.x1 = A->x;
		lineA.y1 = A->y;
		lineA.x2 = A->x;
		lineA.y2 = A->y;

		for (i = A->x, j = A->y; j >= 0; --j)
		{
			if (stones[i][j].type != STONE_EMPTY)
			{
				break;
			}

			lineA.y1 = j;
		}
		for (i = A->x, j = A->y; j < BOARD_H; ++j)
		{
			if (stones[i][j].type != STONE_EMPTY)
			{
				break;
			}

			lineA.y2 = j;
		}

		lineB.x1 = B->x;
		lineB.y1 = B->y;
		lineB.x2 = B->x;
		lineB.y2 = B->y;

		for (i = B->x, j = B->y; j >= 0; --j)
		{
			if (stones[i][j].type != STONE_EMPTY)
			{
				break;
			}

			lineB.y1 = j;
		}
		for (i = B->x, j = B->y; j < BOARD_H; ++j)
		{
			if (stones[i][j].type != STONE_EMPTY)
			{
				break;
			}

			lineB.y2 = j;
		}

		crossing = boardCheckHorizontalIntersection(&lineA, &lineB, A, B);
	}

	if (crossing)
	{
		if (!generating && !checkingMoves)
		{
			fadeOutTimer = showAnimations ? FADE_DELAY : 1;
			stones[A->x][A->y].type = A->type;
			stones[B->x][B->y].type = B->type;

			playSfx(clearSfx);
		}

		A->type = STONE_EMPTY;
		B->type = STONE_EMPTY;

		return 1;
	}
	else
	{
		stones[A->x][A->y].type = A->type;
		stones[B->x][B->y].type = B->type;
	}

	return 0;
}

void boardDrawConnection()
{
	int rcolor = SDL_MapRGB(screen->format, 0, 0, 255);
	line lineA;
	line lineB;

	int x1;
	int y1;
	int x2;
	int y2;

	if (lineC.y1 == lineC.y2) // lineC is a horizontal line.
	{
		lineA.x1 = stoneA.x;
		lineA.x2 = stoneA.x;
		lineA.y1 = (stoneA.y < lineC.y1 ? stoneA.y : lineC.y1);
		lineA.y2 = (stoneA.y > lineC.y2 ? stoneA.y : lineC.y2);

		lineB.x1 = stoneB.x;
		lineB.x2 = stoneB.x;
		lineB.y1 = (stoneB.y < lineC.y1 ? stoneB.y : lineC.y1);
		lineB.y2 = (stoneB.y > lineC.y2 ? stoneB.y : lineC.y2);
	}
	else if (lineC.x1 == lineC.x2) // lineC is a vertical line.
	{
		lineA.x1 = (stoneA.x < lineC.x1 ? stoneA.x : lineC.x1);
		lineA.x2 = (stoneA.x > lineC.x2 ? stoneA.x : lineC.x2);
		lineA.y1 = stoneA.y;
		lineA.y2 = stoneA.y;

		lineB.x1 = (stoneB.x < lineC.x1 ? stoneB.x : lineC.x1);
		lineB.x2 = (stoneB.x > lineC.x2 ? stoneB.x : lineC.x2);
		lineB.y1 = stoneB.y;
		lineB.y2 = stoneB.y;
	}
	else
	{
		return;
	}

	// Line A.
	x1 = lineA.x1 * STONE_W - 1 + STONE_W/2 - lineA.x1;
	y1 = lineA.y1 * STONE_H + STONE_H/2 - lineA.y1;
	x2 = (lineA.x2 - lineA.x1) * STONE_W + 1 - (lineA.x2 - lineA.x1);
	y2 = (lineA.y2 - lineA.y1) * STONE_H + 1 - (lineA.y2 - lineA.y1);

	// Drawing offset at the top of the screen.
	if (y1 < STONE_H - STONE_H/3)
	{
		y2 -= (STONE_H - STONE_H/3) - y1;
		y1 = STONE_H - STONE_H/3;
	}
	// Drawing offset at the bottom of the screen.
	else if ((y1 + y2) > (BOARD_H - 1) * STONE_H - STONE_H/3)
	{
		y2 -= STONE_H/3;
	}

	x1 += BOARD_OFFSET_X;
	y1 += BOARD_OFFSET_Y;
	drawRectangle(screen, x1, y1, x2, y2, rcolor);

	// Line B.
	x1 = lineB.x1 * STONE_W - 1 + STONE_W/2 - lineB.x1;
	y1 = lineB.y1 * STONE_H + STONE_H/2 - lineB.y1;
	x2 = (lineB.x2 - lineB.x1) * STONE_W + 1 - (lineB.x2 - lineB.x1);
	y2 = (lineB.y2 - lineB.y1) * STONE_H + 1 - (lineB.y2 - lineB.y1);

	// Drawing offset at the top of the screen.
	if (y1 < STONE_H - STONE_H/3)
	{
		y2 -= (STONE_H - STONE_H/3) - y1;
		y1 = STONE_H - STONE_H/3;
	}
	// Drawing offset at the bottom of the screen.
	else if ((y1 + y2) > (BOARD_H - 1) * STONE_H - STONE_H/3)
	{
		y2 -= STONE_H/3;
	}

	x1 += BOARD_OFFSET_X;
	y1 += BOARD_OFFSET_Y;
	drawRectangle(screen, x1, y1, x2, y2, rcolor);

	// Line C.
	x1 = lineC.x1 * STONE_W - 1 + STONE_W/2 - lineC.x1;
	y1 = lineC.y1 * STONE_H + STONE_H/2 - lineC.y1;
	x2 = (lineC.x2 - lineC.x1) * STONE_W + 1 - (lineC.x2 - lineC.x1);
	y2 = (lineC.y2 - lineC.y1) * STONE_H + 1 - (lineC.y2 - lineC.y1);

	// Drawing offset at the top of the screen.
	if (y1 < STONE_H - STONE_H/3)
	{
		y1 = STONE_H - STONE_H/3;
	}
	// Drawing offset at the bottom of the screen.
	else if ((y1 + y2) > (BOARD_H - 1) * STONE_H - STONE_H/3)
	{
		y1 -= STONE_H/3;
	}

	x1 += BOARD_OFFSET_X;
	y1 += BOARD_OFFSET_Y;
	drawRectangle(screen, x1, y1, x2, y2, rcolor);
}

void boardLoad()
{
	int i;

	if (!boardBackgroundIMG)
	{
		boardBackgroundIMG = loadImage("data/gfx/background.bmp");
	}

	tilesetLoad(&stonesTileset, "data/gfx/stones.bmp", STONE_W, STONE_H, 9, STONE_COUNT);

	stones = malloc(sizeof(stone*) * BOARD_W);
	if (!stones)
	{
		return;
	}
	for (i = 0; i < BOARD_W; ++i)
	{
		stones[i] = malloc(sizeof(stone) * BOARD_H);
		if (!stones[i])
		{
			return;
		}

		memset(stones[i], 0, sizeof(stone) * BOARD_H);
	}

	boardSetAlpha(255);

	gameOver = 0;

	if (continueGame)
	{
		stonesLeft = 0;
		getBoard(0);
		if (!boardCheckAvailableMoves())
		{
			gameOver = 1;

			if (!stonesLeft)
			{
				gameOver = 0;
				boardGenerate();
				cursorX = 1;
				cursorY = 1;

				if (practice)
				{
					boardCheckAvailableMoves();
				}
			}
		}
	}
	else
	{
		boardGenerate();
		cursorX = 1;
		cursorY = 1;

		if (practice)
		{
			boardCheckAvailableMoves();
		}
	}

	crossing = 0;
}

void boardUnload()
{
	SDL_FreeSurface(boardBackgroundIMG);
	boardBackgroundIMG = NULL;

	tilesetUnload(&stonesTileset);

	if (stones)
	{
		int i;

		for (i = 0; i < BOARD_W; ++i)
		{
			if (stones[i])
			{
				free(stones[i]);
			}
		}

		free(stones);
		stones = NULL;
	}
}

void boardDraw()
{
	int i;
	int j;
	int x;
	int y;

	drawImage(boardBackgroundIMG, NULL, screen, 0, 0);

	for (i = 0, x = 0; x < BOARD_W; i+=STONE_W, ++x)
	{
		for (j = 0, y = 0; y < BOARD_H; j+=STONE_H, ++y)
		{
			if (stones[x][y].type > 0)
			{
				SDL_SetAlpha(stonesTileset.image, SDL_SRCALPHA, stones[x][y].alpha);
				drawImage(stonesTileset.image, &stonesTileset.clip[stones[x][y].type - 1], screen, BOARD_OFFSET_X + i - x, BOARD_OFFSET_Y + j - y);
				if (showStoneRank && stones[x][y].alpha == 255)
				{
					dText(&gameFontRegular, (char *)stoneRankText(stones[x][y].type), BOARD_OFFSET_X + i - x + 1, BOARD_OFFSET_Y + j - y + 1, SHADOW_OUTLINE);
				}
			}
		}
	}

	if (crossing)
	{
		boardDrawConnection();
	}

	if (!gameOver && !fadeOutTimer)
	{
		int rcolor = SDL_MapRGB(screen->format, 0, 0, 255);

		drawRectangle(screen, BOARD_OFFSET_X + cursorX * STONE_W - 1 - cursorX, BOARD_OFFSET_Y + cursorY * STONE_H - 1 - cursorY, STONE_W + 2, STONE_H + 2, rcolor);
		drawRectangle(screen, BOARD_OFFSET_X + cursorX * STONE_W - cursorX, BOARD_OFFSET_Y + cursorY * STONE_H - cursorY, STONE_W, STONE_H, rcolor);
		if (stoneA.type != STONE_EMPTY)
		{
			drawRectangle(screen, BOARD_OFFSET_X + stoneA.x * STONE_W - 1 - stoneA.x, BOARD_OFFSET_Y + stoneA.y * STONE_H - 1 - stoneA.y, STONE_W + 2, STONE_H + 2, rcolor);
			drawRectangle(screen, BOARD_OFFSET_X + stoneA.x * STONE_W - stoneA.x, BOARD_OFFSET_Y + stoneA.y * STONE_H - stoneA.y, STONE_W, STONE_H, rcolor);
		}
		if (stoneB.type != STONE_EMPTY)
		{
			drawRectangle(screen, BOARD_OFFSET_X + stoneB.x * STONE_W - 1 - stoneB.x, BOARD_OFFSET_Y + stoneB.y * STONE_H - 1 - stoneB.y, STONE_W + 2, STONE_H + 2, rcolor);
			drawRectangle(screen, BOARD_OFFSET_X + stoneB.x * STONE_W - stoneB.x, BOARD_OFFSET_Y + stoneB.y * STONE_H - stoneB.y, STONE_W, STONE_H, rcolor);
		}
	}
}
