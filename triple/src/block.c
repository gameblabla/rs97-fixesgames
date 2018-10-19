#include "block.h"

#include <math.h>
#include <stdlib.h>
#include <SDL.h>
#include "board.h"
#include "game.h"
#include "main.h"
#include "video.h"

int blockSpawnCounter;
int blockSpawnCounterLimit;
float blockSpeed;
BlockNode *blockHead;
ExplosionNode *explosionHead;

BlockNode *blockNodePrepend(BlockNode *head)
{
	BlockNode *newNode = malloc(sizeof(BlockNode));
	if(!newNode)
	{
		return head;
	}

	newNode->next = head;
	return newNode;

}

BlockNode *blockNodeDelete(BlockNode *head, BlockNode *toDelNode)
{
	BlockNode *prevNode = NULL;
	BlockNode *curNode = head;

	while(curNode)
	{
		if(curNode == toDelNode)
		{
			if(!prevNode)
			{
				head = curNode->next;
			}
			else
			{
				prevNode->next = curNode->next;
			}

			free(curNode);
			return head;
		}
		prevNode = curNode;
		curNode = curNode->next;
	}

	return head;
}

BlockNode *blockNodeDeleteAll(BlockNode *head)
{
	BlockNode *toDelNode;
	BlockNode *curNode = head;

	while(curNode)
	{
		toDelNode = curNode;
		curNode = curNode->next;
		free(toDelNode);
	}

	return NULL;
}

ExplosionNode *explosionNodePrepend(ExplosionNode *head)
{
	ExplosionNode *newNode = malloc(sizeof(ExplosionNode));
	if(!newNode)
	{
		return head;
	}

	newNode->next = head;
	return newNode;

}

ExplosionNode *explosionNodeDelete(ExplosionNode *head, ExplosionNode *toDelNode)
{
	ExplosionNode *prevNode = NULL;
	ExplosionNode *curNode = head;

	while(curNode)
	{
		if(curNode == toDelNode)
		{
			if(!prevNode)
			{
				head = curNode->next;
			}
			else
			{
				prevNode->next = curNode->next;
			}

			free(curNode);
			return head;
		}
		prevNode = curNode;
		curNode = curNode->next;
	}

	return head;
}

ExplosionNode *explosionNodeDeleteAll(ExplosionNode *head)
{
	ExplosionNode *toDelNode;
	ExplosionNode *curNode = head;

	while(curNode)
	{
		toDelNode = curNode;
		curNode = curNode->next;
		free(toDelNode);
	}

	return NULL;
}

void blockCheckCollision(Block *block)
{
	int ax;
	int ay;
	int aw = BLOCK_SIZE;
	int ah = BLOCK_SIZE;
	int bx;
	int by;
	int bw = BLOCK_SIZE;
	int bh = BLOCK_SIZE;
	int lenx;
	int leny;
	int i;
	int j;
	int n;
	BlocksPos order[4];

	ax = (int)block->x - boardX;
	ay = (int)block->y - boardY;

	for(j = 0, n = 0; j < 2; j++)
	{
		for(i = 0; i < 2; i++)
		{
			order[n].x = floor((float)ax/BLOCK_SIZE)+i;
			order[n].y = floor((float)ay/BLOCK_SIZE)+j;
			order[n].ax = ax;
			order[n].ay = ay;

			if(order[n].x < 0)
			{
				order[n].x = ((int)block->x + i*BLOCK_SIZE + (SCREEN_W - boardX - 1))/BLOCK_SIZE;
				order[n].ax = (order[n].x - i) * BLOCK_SIZE;
				order[n].ax = order[n].ax + (((int)block->x + i*BLOCK_SIZE + (SCREEN_W - boardX - 1))%BLOCK_SIZE);
			}
			else if(order[n].x >= BOARD_WIDTH)
			{
				order[n].x = ((int)block->x + i*BLOCK_SIZE - (SCREEN_W + boardX - 1))/BLOCK_SIZE;
				order[n].ax = (order[n].x - i) * BLOCK_SIZE;
				order[n].ax = order[n].ax + (((int)block->x + i*BLOCK_SIZE + (SCREEN_W - boardX - 1))%BLOCK_SIZE);
			}

			if(order[n].y < 0)
			{
				order[n].y = ((int)block->y + j*BLOCK_SIZE + (SCREEN_H - boardY - 1))/BLOCK_SIZE;
				order[n].ay = (order[n].y - j) * BLOCK_SIZE;
				order[n].ay = order[n].ay + (((int)block->y + j*BLOCK_SIZE + (SCREEN_H - boardY - 1))%BLOCK_SIZE);
			}
			else if(order[n].y >= BOARD_HEIGHT)
			{
				order[n].y = ((int)block->y + j*BLOCK_SIZE - (SCREEN_H + boardY - 1))/BLOCK_SIZE;
				order[n].ay = (order[n].y - j) * BLOCK_SIZE;
				order[n].ay = order[n].ay + (((int)block->y + j*BLOCK_SIZE + (SCREEN_H - boardY - 1))%BLOCK_SIZE);
			}

			n++;
		}
	}

	for(i = 0; i < 3; i++)
	{
		n = 3;
		while(n > 0)
		{
			int distX = abs(((order[n-1].x*BLOCK_SIZE)+BLOCK_SIZE/2 - 1) - (order[n-1].ax + BLOCK_SIZE/2 - 1));
			int distY = abs(((order[n-1].y*BLOCK_SIZE)+BLOCK_SIZE/2 - 1) - (order[n-1].ay + BLOCK_SIZE/2 - 1));

			float len = sqrt(distX*distX+distY*distY);
			float len2;

			distX = abs(((order[n].x*BLOCK_SIZE)+BLOCK_SIZE/2 - 1) - (order[n].ax + BLOCK_SIZE/2 - 1));
			distY = abs(((order[n].y*BLOCK_SIZE)+BLOCK_SIZE/2 - 1) - (order[n].ay + BLOCK_SIZE/2 - 1));

			len2 = sqrt(distX*distX+distY*distY);

			if(len2 < len)
			{
				BlocksPos tmp;

				tmp.x = order[n-1].x;
				tmp.y = order[n-1].y;
				tmp.ax = order[n-1].ax;
				tmp.ay = order[n-1].ay;

				order[n-1].x = order[n].x;
				order[n-1].y = order[n].y;
				order[n-1].ax = order[n].ax;
				order[n-1].ay = order[n].ay;
				order[n].x = tmp.x;
				order[n].y = tmp.y;
				order[n].ax = tmp.ax;
				order[n].ay = tmp.ay;
			}
			n--;
		}
	}

	for(n = 0; n < 4; n++)
	{
		if(order[n].x < 0 || order[n].x >= BOARD_WIDTH || order[n].y < 0 || order[n].y >= BOARD_HEIGHT)
		{
			continue;
		}

		if(board[order[n].x][order[n].y] == COLOR_NONE)
		{
			continue;
		}
		else if(block->x <= -BLOCK_SIZE || block->x >= SCREEN_W || block->y <= -BLOCK_SIZE || block->y >= SCREEN_H)
		{
			if(board[ax/BLOCK_SIZE][ay/BLOCK_SIZE]) // If collision with not-empty tile happens outside the screen boundaries.
			{
				// Put the block outside the screen boundaries, so it will be deleted in the next iteration.
				block->x = -BLOCK_SIZE-1;
				block->y = -BLOCK_SIZE-1;
				block->vx = 0;
				block->vy = 0;

				// Spawn another block immediately.
				blockSpawnCounter = blockSpawnCounterLimit;

				return;
			}
		}

		bx = order[n].x*BLOCK_SIZE;
		by = order[n].y*BLOCK_SIZE;

		lenx = abs((order[n].ax+aw)-(bx+bw));
		leny = abs((order[n].ay+ah)-(by+bh));

		if(lenx >= BLOCK_SIZE && leny >= BLOCK_SIZE)
		{
			return;
		}

		// Eraser block
		if(block->color == COLOR_ERASER)
		{
			if(boardInsertElement(order[n].x, order[n].y, block->color))
				continue;
		}
		else
		{
			// Collision on left-right
			if(lenx >= leny)
			{
				if(order[n].ax > bx)
				{
					if(boardInsertElement(bx/BLOCK_SIZE+1, by/BLOCK_SIZE, block->color))
					continue;
				}
				else
				{
					if(boardInsertElement(bx/BLOCK_SIZE-1, by/BLOCK_SIZE, block->color))
					continue;
				}
			}
			else // Collision on top-down
			{
				if(order[n].ay > by)
				{
					if(boardInsertElement(bx/BLOCK_SIZE, by/BLOCK_SIZE+1, block->color))
						continue;
				}
				else
				{
					if(boardInsertElement(bx/BLOCK_SIZE, by/BLOCK_SIZE-1, block->color))
						continue;
				}
			}
		}

		// Put the block outside the screen boundaries, so it will be deleted in the next iteration.
		block->x = -BLOCK_SIZE-1;
		block->y = -BLOCK_SIZE-1;
		block->vx = 0;
		block->vy = 0;

		return;
	}
}

void blockDrawExplosions()
{
		ExplosionNode *explosionCur = explosionHead;

		while(explosionCur)
		{
			int i;
			int rcolor;
			int step = explosionCur->explosion.step;
			SDL_Rect r;
			SDL_Rect *rOrig;
			SDL_Rect left = {(int)explosionCur->explosion.x + step, (int)explosionCur->explosion.y + step, 1, BLOCK_SIZE - step - step};
			SDL_Rect right = {(int)explosionCur->explosion.x + BLOCK_SIZE - 1 - step, (int)explosionCur->explosion.y + step, 1, BLOCK_SIZE - step - step};
			SDL_Rect up = {(int)explosionCur->explosion.x + step, (int)explosionCur->explosion.y + step, BLOCK_SIZE - step - step, 1};
			SDL_Rect down = {(int)explosionCur->explosion.x + step, (int)explosionCur->explosion.y + BLOCK_SIZE - 1 - step, BLOCK_SIZE - step - step, 1};

			if(explosionCur->explosion.step >= 8)
			{
				explosionHead = explosionNodeDelete(explosionHead, explosionCur);
				break;
			}

			switch(explosionCur->explosion.color)
			{
				case COLOR_ERASER:
					if(blinkTimer > 10)
					{
						rcolor = SDL_MapRGB(screen->format, 255, 255, 255);
					}
					else
					{
						rcolor = SDL_MapRGB(screen->format, 0, 0, 0);
					}
				break;
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
					rcolor = SDL_MapRGB(screen->format, 255, 255, 255);
				break;
			}

			for(i = 0; i < 4; ++i)
			{
				switch(i)
				{
					case 0:
						rOrig = &left;
					break;
					case 1:
						rOrig = &right;
					break;
					case 2:
						rOrig = &up;
					break;
					case 3:
						rOrig = &down;
					break;

					default:
					break;
				}

				r = *rOrig;

				if(pause)
				{
					rcolor = SDL_MapRGB(screen->format, 255, 255, 255);
				}

				if(r.x > -BLOCK_SIZE && r.x < SCREEN_W && r.y > -BLOCK_SIZE && r.y < SCREEN_H)
				{
					SDL_FillRect(screen, &r, rcolor);
				}

				// NOTE: SDL_FillRect() overwrites r.w and r.h values if drawing occurs outside the surface area.
				// Restore the original r values:
				r = *rOrig;

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
					r = *rOrig;

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
					r = *rOrig;

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

			if(!(explosionCur->explosion.timer % (EXPLOSION_SPEED/8)))
			{
				explosionCur->explosion.step++;
			}
			explosionCur->explosion.timer--;

			explosionCur = explosionCur->next;
		}
}

void blockDrawBlocks()
{
		BlockNode *blockCur = blockHead;

		while(blockCur)
		{
			int rcolor;
			SDL_Rect r = {(int)blockCur->block.x, (int)blockCur->block.y, BLOCK_SIZE, BLOCK_SIZE};

			switch(blockCur->block.color)
			{
				case COLOR_ERASER:
					if(blinkTimer > 10)
					{
						rcolor = SDL_MapRGB(screen->format, 255, 255, 255);
					}
					else
					{
						rcolor = SDL_MapRGB(screen->format, 0, 0, 0);
					}
				break;
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
					rcolor = SDL_MapRGB(screen->format, 255, 255, 255);
				break;
			}

			if(pause)
			{
				SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 255, 255, 255));
			}
			else
			{
				SDL_FillRect(screen, &r, rcolor);
			}
			blockCur = blockCur->next;
		}
}
