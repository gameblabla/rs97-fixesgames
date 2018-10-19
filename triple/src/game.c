#include "game.h"

#include <stdlib.h>
#include "board.h"
#include "block.h"
#include "font.h"
#include "hiscore.h"
#include "input.h"
#include "main.h"
#include "states.h"
#include "video.h"

int pause;
int over;
int overTimer;
int score;
int clearedInitial;
int clearedPoints;
int clearedColors[COLOR_NUM - 2];
unsigned int gameTime;
char pointsStr[30] = "";
int pointsStrTimer;
int allowedColors;
int eraserSpawned;

void gameUnload()
{
	hiscoreUpdate(score);
}

void gameLoad()
{
	topHiscore = hiscore[0];
	blockSpawnCounter = BLOCK_COUNTER_LIMIT;
	blockSpawnCounterLimit = BLOCK_COUNTER_LIMIT;
	gameTime = 0;
	pause = 0;
	over = 0;
	allowedColors = 2;
	eraserSpawned = 0;
	blockSpeed = BLOCK_BASE_SPEED;
	blockHead = NULL;
	explosionHead = NULL;
	score = 0;
	clearedInitial = 0;
	clearedPoints = 0;
	blockCounter = 1;
	pointsStrTimer = 0;
	boardX = SCREEN_W/2 - (BOARD_WIDTH/2+1)*BLOCK_SIZE - BLOCK_SIZE/2;
	boardY = SCREEN_H/2 - (BOARD_HEIGHT/2+1)*BLOCK_SIZE - BLOCK_SIZE/2;
	boardInit();
}

void gameLogic()
{
	boardVx = 0;
	boardVy = 0;

	if(over && overTimer)
	{
		overTimer--;

		if(!overTimer)
		{
			boardClearAll();
		}
	}

	// Once the board is clear, move to Hi-Score state.
	if(over && !overTimer && !explosionHead)
	{
		programStateNew = STATE_HISCORE;

		if(blockHead)
		{
			blockHead = blockNodeDeleteAll(blockHead);
		}
	}

	if(!over && !pause)
	{
		gameTime++;

		if(!(gameTime % 300)) // Every 5 seconds increase the amount of blocks spawned and the speed of blocks.
		{
			if(--blockSpawnCounterLimit < 30)
			{
				blockSpawnCounterLimit = 30;
			}

			blockSpeed += 0.02;

			if(blockSpeed > BLOCK_BASE_SPEED*2.0)
			{
				blockSpeed = BLOCK_BASE_SPEED*2.0;
			}
		}
		if(!(gameTime % (1200 * (allowedColors - 1) * (allowedColors - 1)/2))) // Every (20 * [colors in game]*[colors in game]/2) seconds add a new color.
		{
			if(++allowedColors > COLOR_NUM - 2)
			{
				allowedColors = COLOR_NUM - 2;
			}
		}
	}

	if(!over && keys[KEY_BACK])
	{
		keys[KEY_BACK] = 0;
		over = 1;
		overTimer = 1;
	}

	if(!over && keys[KEY_START])
	{
		keys[KEY_START] = 0;

		pause = !pause;
	}

	if(!over && !pause)
	{
		if(keys[KEY_LEFT])
		{
			boardVx-=BOARD_SPEED;
		}
		else if(keys[KEY_RIGHT])
		{
			boardVx+=BOARD_SPEED;
		}
		if(keys[KEY_UP])
		{
			boardVy-=BOARD_SPEED;
		}
		else if(keys[KEY_DOWN])
		{
			boardVy+=BOARD_SPEED;
		}
	}

	if(!pause && !over)
	{
		BlockNode *blockCur = blockHead;

		while(blockCur)
		{
			BlockNode *toDelete = NULL;

			if(blockCur->block.x < -BLOCK_SIZE || blockCur->block.x > SCREEN_W || blockCur->block.y < -BLOCK_SIZE || blockCur->block.y > SCREEN_H)
			{
				toDelete = blockCur;
			}
			else
			{
				blockCur->block.x += blockCur->block.vx;
				blockCur->block.y += blockCur->block.vy;
			}
			blockCur = blockCur->next;

			if(toDelete)
			{
				if(toDelete->block.color == COLOR_ERASER)
				{
					eraserSpawned = 0;
				}

				blockHead = blockNodeDelete(blockHead, toDelete);
			}
		}

		boardX += boardVx;
		boardY += boardVy;

		if(boardX < -(BOARD_WIDTH/2+1) * BLOCK_SIZE)
		{
			boardX = -(BOARD_WIDTH/2+1) * BLOCK_SIZE;
		}
		if(boardY < -(BOARD_HEIGHT/2+1) * BLOCK_SIZE)
		{
			boardY = -(BOARD_HEIGHT/2+1) * BLOCK_SIZE;
		}
		if(boardX > SCREEN_W -(BOARD_WIDTH/2+1) * BLOCK_SIZE - BLOCK_SIZE)
		{
			boardX = SCREEN_W - (BOARD_WIDTH/2+1) * BLOCK_SIZE - BLOCK_SIZE;
		}
		if(boardY > SCREEN_H -(BOARD_HEIGHT/2+1) * BLOCK_SIZE - BLOCK_SIZE)
		{
			boardY = SCREEN_H - (BOARD_HEIGHT/2+1) * BLOCK_SIZE - BLOCK_SIZE;
		}
	}

	if(!pause && !over)
	{
		if(blockSpawnCounter >= blockSpawnCounterLimit)
		{
			blockSpawnCounter = 0;
			blockHead = blockNodePrepend(blockHead);

			switch(rand() % 4)
			{
				case 0: // N
					blockHead->block.x = rand() % (SCREEN_W - BLOCK_SIZE);
					blockHead->block.y = 0 - BLOCK_SIZE;
					blockHead->block.vx = 0;
					blockHead->block.vy = blockSpeed;
				break;
				case 1: // E
					blockHead->block.x = SCREEN_W;
					blockHead->block.y = rand() % (SCREEN_H - BLOCK_SIZE);
					blockHead->block.vx = -blockSpeed;
					blockHead->block.vy = 0;
				break;
				case 2: // S
					blockHead->block.x = rand() % (SCREEN_W - BLOCK_SIZE);
					blockHead->block.y = SCREEN_H;
					blockHead->block.vx = 0;
					blockHead->block.vy = -blockSpeed;
				break;
				case 3: // W
					blockHead->block.x = 0 - BLOCK_SIZE;
					blockHead->block.y = rand() % (SCREEN_H - BLOCK_SIZE);
					blockHead->block.vx = blockSpeed;
					blockHead->block.vy = 0;
				break;

				default:
				break;
			}

			blockHead->block.color = rand() % allowedColors + 2;

			if(!eraserSpawned && !(rand() % 10)) // Attempt to generate eraser block.
			{
				eraserSpawned = 1;
				blockHead->block.color = COLOR_ERASER;
			}
		}
		else
		{
			blockSpawnCounter++;
		}
	}

	if(!pause && !over)
	{
		BlockNode *blockCur = blockHead;

		while(blockCur)
		{
			blockCheckCollision(&blockCur->block);
			blockCur = blockCur->next;
		}

		if(score > hiscore[0])
		{
			topHiscore = score;
		}
	}
}

void gameDraw()
{
	char blocksStr[20] = "";
	char scoreStr[20] = "";

	blinkTimersTick();

	// Draw assets.
	boardDraw();
	blockDrawExplosions();
	blockDrawBlocks();

	// Draw gui elements.
	sprintf(blocksStr, "w:%2d/%2d", blockCounter, BLOCKS_MAX);

	if(over && score >= topHiscore && blinkTimer <= 10)
	{
		sprintf(scoreStr, "H:        S:%07d", score);
	}
	else
	{
		sprintf(scoreStr, "H:%07d S:%07d", topHiscore, score);
	}
	dText(blocksStr, 1, 1, (!over ? SDL_MapRGB(screen->format, 255, 255, 0) : SDL_MapRGB(screen->format, 255, 0, 0)));
	dText(scoreStr, SCREEN_W - 170, 1, SDL_MapRGB(screen->format, 255, 255, 0));

	if(!over && clearedPoints)
	{
		int i;
		int len = 0;
		int maxlen;
		pointsStr[0] = '\0';
		maxlen = sizeof(pointsStr);

		for(i = 0; i < sizeof(clearedColors)/sizeof(clearedColors[0]); ++i)
		{
			if(clearedColors[i])
			{
				len = sprintf(pointsStr, "[+%d] ", clearedPoints);
				break;
			}
		}

		if(!len)
		{
			len = sprintf(pointsStr, "+%d", clearedPoints);
		}
		else if(len <= maxlen)
		{
			len += sprintf(pointsStr + len, "%d", clearedInitial);

			for(i = 0; i < sizeof(clearedColors)/sizeof(clearedColors[0]); ++i)
			{
				if(len > maxlen)
				{
					break;
				}
				if(clearedColors[i])
				{
					len += sprintf(pointsStr + len, "x%d", clearedColors[i]);
				}
			}
		}

		clearedPoints = 0;
		clearedInitial = 0;

		pointsStrTimer = POINTS_INFO_TIMER;
	}

	if(pointsStrTimer)
	{
		dTextCentered(pointsStr, SCREEN_H - 11, SDL_MapRGB(screen->format, 255, 255, 0));
		pointsStrTimer--;
	}

	if(pause && blinkTimerSlow < 20)
	{
		dText("PAUSE", 63+18, 1, SDL_MapRGB(screen->format, 255, 255, 0));
	}

	if(gameTime < 360)
	{
		dTextCentered("\
		JOIN 3 OF r g b m y c\n\
		      IN CHAINS\n\n\
		      e CLEANS\n\
		", 40, SDL_MapRGB(screen->format, 255, 255, 0));
	}
}
