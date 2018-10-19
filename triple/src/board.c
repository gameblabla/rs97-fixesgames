#include "board.h"

#include <SDL.h>
#include <string.h>
#include "block.h"
#include "game.h"
#include "main.h"
#include "video.h"

int board[BOARD_WIDTH][BOARD_HEIGHT];
int marker[BOARD_WIDTH][BOARD_HEIGHT];
int blockCounter;

int boardX;
int boardY;
int boardVx;
int boardVy;

void boardInit()
{
	memset(board, COLOR_NONE, sizeof(board));
	board[BOARD_WIDTH/2+1][BOARD_HEIGHT/2+1] = COLOR_WHITE;
}

int boardInsertElement(int x, int y, Color color)
{
	if(color == COLOR_ERASER)
	{
		if(board[x][y] == COLOR_NONE)
		{
			return 1;
		}

		if(board[x][y] == COLOR_WHITE)
		{
			// Remove the eraser block, but don't remove the tile.
			return 0;
		}

		explosionHead = explosionNodePrepend(explosionHead);
		explosionHead->explosion.color = board[x][y];
		explosionHead->explosion.timer = EXPLOSION_SPEED;
		explosionHead->explosion.step = 0;
		explosionHead->explosion.x = x * BLOCK_SIZE + boardX;
		explosionHead->explosion.y = y * BLOCK_SIZE + boardY;

		board[x][y] = COLOR_NONE;
		blockCounter--;
		// Remove bricks that don't make connection
		memset(marker, 0, sizeof(marker));
		marker[BOARD_WIDTH/2+1][BOARD_HEIGHT/2+1] = 1;
		boardCheckConnection();

		return 0;
	}

	if(color == COLOR_NONE)
	{
		return 1;
	}

/*	if(x < 0 || y < 0 || x >= BOARD_WIDTH || y >= BOARD_HEIGHT)*/
/*	{*/
/*		return 1;*/
/*	}*/

	if(x < 0)
	{
		x = BOARD_WIDTH + x;
	}
	else if(x >= BOARD_WIDTH)
	{
		x = x - BOARD_WIDTH;
	}
	if(y < 0)
	{
		y = BOARD_HEIGHT + y;
	}
	else if(y >= BOARD_HEIGHT)
	{
		y = y - BOARD_HEIGHT;
	}

	if(board[x][y] != COLOR_NONE)
	{
		return 1;
	}

	board[x][y] = color;
	blockCounter++;

	boardClearColor(color);

	if(blockCounter >= BLOCKS_MAX)
	{
		over = 1;
		overTimer = 60;
		pause = 0;
	}

	return 0;
}

void boardClearAll()
{
	int i;
	int j;

	for(j = 0; j < BOARD_HEIGHT; j++)
	{
		for(i = 0; i < BOARD_WIDTH; i++)
		{
			if(board[i][j] != COLOR_NONE)
			{
				explosionHead = explosionNodePrepend(explosionHead);
				explosionHead->explosion.color = COLOR_WHITE;
				explosionHead->explosion.timer = EXPLOSION_SPEED;
				explosionHead->explosion.step = 0;
				explosionHead->explosion.x = i * BLOCK_SIZE + boardX;
				explosionHead->explosion.y = j * BLOCK_SIZE + boardY;

				board[i][j] = COLOR_NONE;
			}
		}
	}
}

void boardClearColor(Color color)
{
	int i;
	int j;
	int remNum = 0;

	memset(marker, 0, sizeof(marker));

	for(j = 0; j < BOARD_HEIGHT; j++)
	{
		for(i = 0; i < BOARD_WIDTH; i++)
		{
			int count = 0;
			if(board[i][j] == color)
			{
				// N
				if(j > 0 && board[i][j-1] == color)
				{
					count++;
				}
				// E
				if(i < BOARD_WIDTH - 1 && board[i+1][j] == color)
				{
					count++;
				}
				// S
				if(j < BOARD_HEIGHT - 1 && board[i][j+1] == color)
				{
					count++;
				}
				// W
				if(i > 0 && board[i-1][j] == color)
				{
					count++;
				}

				if(count < marker[i][j])
				{
					count = marker[i][j];
				}

				// N
				if(j > 0 && board[i][j-1] == color)
				{
					marker[i][j-1] = count;
				}
				// E
				if(i < BOARD_WIDTH - 1 && board[i+1][j] == color)
				{
					marker[i+1][j] = count;
				}
				// S
				if(j < BOARD_HEIGHT - 1 && board[i][j+1] == color)
				{
					marker[i][j+1] = count;
				}
				// W
				if(i > 0 && board[i-1][j] == color)
				{
					marker[i-1][j] = count;
				}

				marker[i][j] = count;
			}
		}
	}

	for(j = 0; j < BOARD_HEIGHT; j++)
	{
		for(i = 0; i < BOARD_WIDTH; i++)
		{
			if(board[i][j] == color && marker[i][j] > 1)
			{
				explosionHead = explosionNodePrepend(explosionHead);
				explosionHead->explosion.color = board[i][j];
				explosionHead->explosion.timer = EXPLOSION_SPEED;
				explosionHead->explosion.step = 0;
				explosionHead->explosion.x = i * BLOCK_SIZE + boardX;
				explosionHead->explosion.y = j * BLOCK_SIZE + boardY;

				board[i][j] = COLOR_NONE;
				remNum++;
			}
		}
	}

	if(remNum)
	{
		clearedInitial = remNum;
		clearedPoints = clearedInitial;
		//score += 10*remNum;
		blockCounter -= remNum;
		if(blockCounter < 0)
		{
			blockCounter = 0;
		}
	}

	// Remove bricks that don't make connection
	memset(marker, 0, sizeof(marker));
	marker[BOARD_WIDTH/2+1][BOARD_HEIGHT/2+1] = 1;

	memset(clearedColors, 0, sizeof(clearedColors));
	boardCheckConnection();

	score += clearedPoints;
}

void boardCheckConnection()
{
	int i;
	int j;
	int n;
	int repeat = 1;
	int remNum = 0;

	while(repeat)
	{
		repeat = 0;
		for(j = 0; j < BOARD_HEIGHT; j++)
		{
			for(i = 0; i < BOARD_WIDTH; i++)
			{
				if(marker[i][j] == 1)
				{
					// N
					n = (j > 0) ? j-1 : BOARD_HEIGHT - 1;
					
					if(board[i][n])
					//if(j > 0 && board[i][j-1])
					{
						if(!marker[i][n])
						{
							marker[i][n] = 1;
							repeat = 1;
						}
					}
					// E
					n = (i < BOARD_WIDTH - 1) ? i+1 : 0;

					if(board[n][j])
					//if(i < BOARD_WIDTH - 1 && board[i+1][j])
					{
						if(!marker[n][j])
						{
							marker[n][j] = 1;
							repeat = 1;
						}
					}
					// S
					n = (j < BOARD_HEIGHT - 1) ? j+1 : 0;

					if(board[i][n])
					//if(j < BOARD_HEIGHT - 1 && board[i][j+1])
					{
						if(!marker[i][n])
						{
							marker[i][n] = 1;
							repeat = 1;
						}
					}
					// W
					n = (i > 0) ? i-1 : BOARD_WIDTH - 1;

					if(board[n][j])
					//if(i > 0 && board[i-1][j])
					{
						if(!marker[n][j])
						{
							marker[n][j] = 1;
							repeat = 1;
						}
					}

					marker[i][j] = 2;
				}
			}
		}
	}

	for(j = 0; j < BOARD_HEIGHT; j++)
	{
		for(i = 0; i < BOARD_WIDTH; i++)
		{
			if(!marker[i][j] && board[i][j] != COLOR_NONE)
			{
				explosionHead = explosionNodePrepend(explosionHead);
				explosionHead->explosion.color = board[i][j];
				explosionHead->explosion.timer = EXPLOSION_SPEED;
				explosionHead->explosion.step = 0;
				explosionHead->explosion.x = i * BLOCK_SIZE + boardX;
				explosionHead->explosion.y = j * BLOCK_SIZE + boardY;

				if(clearedInitial)
				{
					clearedColors[board[i][j]-2]++;
				}

				board[i][j] = COLOR_NONE;
				remNum++;
			}
		}
	}

	if(remNum)
	{
		if(clearedInitial)
		{
			for(i = 0; i < sizeof(clearedColors)/sizeof(clearedColors[0]); i++)
			{
				if(clearedColors[i])
				{
					clearedPoints *= clearedColors[i];
				}
			}
		}
		//score += 100*remNum;
		blockCounter -= remNum;
		if(blockCounter < 0)
		{
			blockCounter = 0;
		}
	}
}

void boardDraw()
{
	int i;
	int j;

	clearScreen();

	for(j = 0; j < BOARD_HEIGHT; j++)
	{
		for(i = 0; i < BOARD_WIDTH; i++)
		{
			SDL_Rect rOrig = {i*BLOCK_SIZE+boardX, j*BLOCK_SIZE+boardY, BLOCK_SIZE, BLOCK_SIZE};
			SDL_Rect r = rOrig;
			int rcolor;

			switch(board[i][j])
			{
				case COLOR_NONE:
				continue;
				case COLOR_WHITE:
					rcolor = SDL_MapRGB(screen->format, 255, 255, 255);
				break;
				case COLOR_RED:
					rcolor = SDL_MapRGB(screen->format, 255, 0, 0);
				break;
				case COLOR_GREEN:
					rcolor = SDL_MapRGB(screen->format, 0, 100, 0);
				break;
				case COLOR_BLUE:
					rcolor = SDL_MapRGB(screen->format, 0, 0, 255);
				break;
				case COLOR_MAGENTA:
					rcolor = SDL_MapRGB(screen->format, 255, 0, 255);
				break;
				case COLOR_YELLOW:
					rcolor = SDL_MapRGB(screen->format, 255, 255, 0);
				break;
				case COLOR_CYAN:
					rcolor = SDL_MapRGB(screen->format, 0, 200, 200);
				break;


				default:
					rcolor = SDL_MapRGB(screen->format, 0, 0, 0);
				break;
			}

			if((pause || over) && board[i][j] != COLOR_NONE)
			{
				rcolor = SDL_MapRGB(screen->format, 255, 255, 255);
			}
			if(r.x > -BLOCK_SIZE && r.x < SCREEN_W && r.y > -BLOCK_SIZE && r.y < SCREEN_H)
			{
				r.w = BLOCK_SIZE;
				r.h = BLOCK_SIZE;
				SDL_FillRect(screen, &r, rcolor);
			}

			// NOTE: SDL_FillRect() overwrites r.w and r.h values if drawing occurs outside the surface area.
			// Restore the original r values:
			r = rOrig;

			if(r.x < 0 || r.x >= SCREEN_W - BLOCK_SIZE || r.y < 0 || r.y >= SCREEN_H - BLOCK_SIZE)
			{
				if(r.x < 0)
				{
					r.x = SCREEN_W + r.x;
				}
				else if(r.x >= SCREEN_W - BLOCK_SIZE)
				{
					r.x = r.x - SCREEN_W;
				}

				SDL_FillRect(screen, &r, rcolor);

				// NOTE: SDL_FillRect() overwrites r.w and r.h values if drawing occurs outside the surface area.
				// Restore the original r values:
				r = rOrig;

				if(r.y < 0)
				{
					r.y = SCREEN_H + r.y;
				}
				else if(r.y >= SCREEN_H - BLOCK_SIZE)
				{
					r.y = r.y - SCREEN_H;
				}

				SDL_FillRect(screen, &r, rcolor);

				// NOTE: SDL_FillRect() overwrites r.w and r.h values if drawing occurs outside the surface area.
				// Restore the original r values:
				r = rOrig;

				if(r.x < 0)
				{
					r.x = SCREEN_W + r.x;
				}
				else if(r.x >= SCREEN_W - BLOCK_SIZE)
				{
					r.x = r.x - SCREEN_W;
				}

				if(r.y < 0)
				{
					r.y = SCREEN_H + r.y;
				}
				else if(r.y >= SCREEN_H - BLOCK_SIZE)
				{
					r.y = r.y - SCREEN_H;
				}

				SDL_FillRect(screen, &r, rcolor);
			}
		}
	}
}
