#include "game.h"

#include <math.h>
#include <stdlib.h>
#include "debug.h"
#include "fileio.h"
#include "font.h"
#include "helpers.h"
#include "input.h"
#include "objects.h"
#include "states.h"
#include "video.h"

listElement *objListHead;
object *playerObj;
tileset marker;
int weaponMode;
int gameTicks;
uint16_t gameTime;
uint16_t bestTime;
int cooldownTime;
int playerLastAngle;
int playerPenaltyTimer;
int gameOverTimer;
int scoreBlinkingDraw;
int scoreBlinkingTimer;
int gamePaused;
int gamePausedTimer;

void gameUnload()
{
	objListHead = listElementDeleteAll(objListHead, objectItemDelete);
	objTemplateListHead = listElementDeleteAll(objTemplateListHead, objectTemplateItemDelete);
	tilesetUnload(&marker);

	if (gameTime > bestTime)
	{
		bestTime = gameTime;
		storeHiscore();
	}

	gameTime = 0;
	gameOverTimer = 0;
	playerPenaltyTimer = 0;
}

void gameLoad()
{
	object newObj;

	tilesetLoad(&marker, "data/gfx/marker.bmp", 7, 7, 3, 3);

	objectLoad(&newObj, OBJ_PLAYER);
	newObj.x = 0;
	newObj.y = 0;

	objListHead = listElementPrepend(objListHead);
	objListHead->item = malloc(sizeof(object));
	memcpy(objListHead->item, &newObj, sizeof(object));

	playerObj = objListHead->item;

	gamePaused = 0;
	gameTime = 0;
}

void gameLogic()
{
	listElement *curNode;

	if (gamePaused)
	{
		++gamePausedTimer;
	}
	else
	{
		++gameTicks;
	}

	if (gameOverTimer)
	{
		if (!--gameOverTimer)
		{
			gameTime = 0;

			playerPenaltyTimer = 0;
			playerLastAngle = playerObj->angle;
		}

		if (!--scoreBlinkingTimer)
		{
			scoreBlinkingDraw = !scoreBlinkingDraw;
			scoreBlinkingTimer = SCORE_BLINKING_INTERVAL;
		}
	}

	if (!gameOverTimer && !gamePaused)
		++gameTime;

	/* Remove flagged objects */
	objListHead = listElementDeleteMatching(objListHead, objectItemDelete, objectItemDisposedMatch);

#if defined(DEBUG)
	if (keys[KEY_D_SLOMO])
	{
		keys[KEY_D_SLOMO] = 0;
		debugSlowMotion = !debugSlowMotion;
	}

	if (keys[KEY_D_SPAWN_ENEMY])
	{
		int angle = rand() % SINE_STEPS;
		int radius = 400 + (rand() % 100);
		object newObj;

		keys[KEY_D_SPAWN_ENEMY] = 0;

		objectLoad(&newObj, OBJ_MISSILE_RED);

		newObj.x = FMODF(playerObj->x + radius * sineTable[angle], LEVEL_W);
		newObj.y = FMODF(playerObj->y + radius * sineTable[(angle+90)%SINE_STEPS], LEVEL_H);
		newObj.angle = angle;

		objListHead = listElementPrepend(objListHead);
		objListHead->item = malloc(sizeof(object));
		memcpy(objListHead->item, &newObj, sizeof(object));
	}
#endif

	if (keys[KEY_BACK] && !gamePaused)
	{
		keys[KEY_BACK] = 0;
		programStateNew = STATE_TITLE;
	}
	if (keys[KEY_START] && !gameOverTimer)
	{
		keys[KEY_START] = 0;

		if (!gamePaused || gamePausedTimer >= PAUSE_RESUME_TIME)
		{
			gamePaused = !gamePaused;
			gamePausedTimer = 0;
		}
	}
	if (keys[KEY_LEFT] && !gamePaused && joyData.inDeadzoneX && joyData.inDeadzoneY)
	{
		playerObj->angle = MOD(playerObj->angle + PLAYER_ROTATION, SINE_STEPS);

	}
	if (keys[KEY_RIGHT] && !gamePaused && joyData.inDeadzoneX && joyData.inDeadzoneY)
	{
		playerObj->angle = MOD(playerObj->angle - PLAYER_ROTATION, SINE_STEPS);
	}
/*	if (keys[KEY_UP])*/
/*	{*/
/*	}*/
/*	if (keys[KEY_DOWN])*/
/*	{*/
/*	}*/

	if (!gameOverTimer && !gamePaused)
	{
		if (!(gameTime % (60*30)))
		{
			int i;
			int num = 4;

			for (i = 0; i < num; ++i)
			{
				int angle = 45+90*i;
				int radius = 350;
				object newObj;

				objectLoad(&newObj, OBJ_MISSILE_YELLOW);

				newObj.x = FMODF(playerObj->x + radius * sineTable[angle], LEVEL_W);
				newObj.y = FMODF(playerObj->y + radius * sineTable[(angle+90)%SINE_STEPS], LEVEL_H);
				newObj.angle = angle;

				objListHead = listElementPrepend(objListHead);
				objListHead->item = malloc(sizeof(object));
				memcpy(objListHead->item, &newObj, sizeof(object));
			}
		}
		else
		{
			if (!((gameTime+GAME_OVER_TIME) % (60*6)))
			{
				int i;
				int max = 1 + (gameTime/1800);
				int num = (rand() % max) + 1;

				for (i = 0; i < num; ++i)
				{
					int angle = rand() % SINE_STEPS;
					int radius = 400 + (rand() % 100);
					object newObj;

					objectLoad(&newObj, OBJ_MISSILE_RED);

					newObj.x = FMODF(playerObj->x + radius * sineTable[angle], LEVEL_W);
					newObj.y = FMODF(playerObj->y + radius * sineTable[(angle+90)%SINE_STEPS], LEVEL_H);
					newObj.angle = angle;

					objListHead = listElementPrepend(objListHead);
					objListHead->item = malloc(sizeof(object));
					memcpy(objListHead->item, &newObj, sizeof(object));
				}
			}
			if (!(gameTime % (30*23)))
			{
				int i;
				int num = (rand() % 2) + 1;

				for (i = 0; i < num; ++i)
				{
					int angle = rand() % SINE_STEPS;
					int radius = 400 + (rand() % 100);
					object newObj;

					objectLoad(&newObj, OBJ_MISSILE_BLUE);

					newObj.x = FMODF(playerObj->x + radius * sineTable[angle], LEVEL_W);
					newObj.y = FMODF(playerObj->y + radius * sineTable[(angle+90)%SINE_STEPS], LEVEL_H);
					newObj.angle = angle;

					objListHead = listElementPrepend(objListHead);
					objListHead->item = malloc(sizeof(object));
					memcpy(objListHead->item, &newObj, sizeof(object));
				}
			}
		}
	}

	curNode = objListHead;

	while(curNode && !gamePaused)
	{
		objectLogic((object *)curNode->item);

		curNode = curNode->next;
	}

	/* Collision check. */
	curNode = objListHead;
	while(curNode && !gamePaused)
	{
		listElement *curNode2 = objListHead;
		object *curObj = (object *)curNode->item;

		if (curObj->dispose)
		{
			curNode = curNode->next;
			continue;
		}

		while(curNode2)
		{
			object *curObj2 = (object *)curNode2->item;

			if (curObj == curObj2 || curObj2->dispose)
			{
				curNode2 = curNode2->next;
				continue;
			}

			if (!(curObj->type == OBJ_CLOUD || curObj->type == OBJ_SMOKE || curObj2->type == OBJ_CLOUD || curObj2->type == OBJ_SMOKE))
			{
				if (objectCollisionCheck(curObj, curObj2))
				{
					int i;

					for (i = 0; i < 5; ++i)
					{
						object newObj;

						curObj->dispose = curObj->type == OBJ_PLAYER ? 0 : 1;
						curObj2->dispose = curObj2->type == OBJ_PLAYER ? 0 : 1;

						curObj->smoking = curObj->type == OBJ_PLAYER ? 30 : 0;
						curObj2->smoking = curObj2->type == OBJ_PLAYER ? 30 : 0;

						if (!gameOverTimer && (curObj->type == OBJ_PLAYER || curObj2->type == OBJ_PLAYER))
						{
							gameOverTimer = GAME_OVER_TIME;
							scoreBlinkingDraw = 1;
							scoreBlinkingTimer = SCORE_BLINKING_INTERVAL;

							if (gameTime > bestTime)
							{
								bestTime = gameTime;
								storeHiscore();
							}
						}

						objectLoad(&newObj, OBJ_SMOKE);
						newObj.x = (curObj->x + curObj->w/2 - newObj.w/2) - 8 + (rand()%16);
						newObj.y = (curObj->y + curObj->h/2 - newObj.h/2) - 8 + (rand()%16);

						objListHead = listElementPrepend(objListHead);
						objListHead->item = malloc(sizeof(object));
						memcpy(objListHead->item, &newObj, sizeof(object));
					}
				}
			}

			curNode2 = curNode2->next;
		}

		curNode = curNode->next;
	}
}

void gameDraw()
{
#if defined(DEBUG)
	char debugStr[50];
#endif
	char timerStr[20];
	listElement *curNode;
	int i;

	drawBackground(screen, gameOverTimer ? getColor(128, 0, 0) : getColor(0, 0, 128));

#if defined(DEBUG)
	{
		SDL_Rect r;

		r.x = LEVEL_W - MOD((int)(playerObj->x + playerObj->w/2 - SCREEN_W/2), LEVEL_W) - 1;
		r.y = LEVEL_H - MOD((int)(playerObj->y + playerObj->h/2 - SCREEN_H/2), LEVEL_H) - 1;
		r.w = 2;
		r.h = 2;

		SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 255, 0, 0));
	}
#endif

	for (i = 0; i < SCREEN_W + 16; i += 16)
	{
		int j;
		for (j = 0; j < SCREEN_H + 16; j += 16)
		{
			int x = (int)playerObj->x % 16;
			int y = (int)playerObj->y % 16;

			drawPoint(screen, i - x, j - y, gameOverTimer ? getColor(224, 0, 0) : getColor(0, 0, 224));
		}
	}

	curNode = objListHead;

	while(curNode)
	{
		if (!gamePaused || curNode->item == playerObj)
			objectDraw((object *)curNode->item);

		curNode = curNode->next;
	}

	sprintf(timerStr, "%d'%02d\"%02d\n", (bestTime > gameTime ? bestTime : gameTime)/60/60, (bestTime > gameTime ? bestTime : gameTime)/60%60, (bestTime > gameTime ? bestTime : gameTime)%60*1000/600);

	if (bestTime > gameTime && (!gameOverTimer || scoreBlinkingDraw))
	{
		char timerStr2[10];

		sprintf(timerStr2, "%d'%02d\"%02d", gameTime/60/60, gameTime/60%60, gameTime%60*1000/600);
		strcat(timerStr, timerStr2);
	}

	if (!gameOverTimer || bestTime > gameTime || scoreBlinkingDraw)
		dTextCentered(&gameFont, timerStr, gameFont.h, ALPHA_OPAQUE, SHADOW_DROP);

#if defined(DEBUG)
	sprintf(debugStr, "Obj: %u\n(%03d,%03d)", listLength(objListHead), (int)playerObj->x, (int)playerObj->y);
	dText(&gameFont, debugStr, gameFont.tracking + 1, SCREEN_H - (gameFont.h + gameFont.leading) * 2 - 1, ALPHA_OPAQUE, SHADOW_DROP);
#endif

	if (!bestTime && gameTime < 60*4)
	{
		dTextCentered(&gameFont, "MISSION:", SCREEN_H/2 + 30, ALPHA_OPAQUE, SHADOW_DROP);
		dTextCentered(&gameFont, "Avoid missiles.", SCREEN_H/2 + 30 + (gameFont.h + gameFont.leading), ALPHA_OPAQUE, SHADOW_DROP);
		dTextCentered(&gameFont, "Stay alive.", SCREEN_H/2 + 30 + (gameFont.h + gameFont.leading) * 2, ALPHA_OPAQUE, SHADOW_DROP);
	}
	if (gameOverTimer)
	{
		dTextCentered(&gameFont, "BOOM!", SCREEN_H/2 + 30, ALPHA_OPAQUE, SHADOW_DROP);
		dTextCentered(&gameFont, "Try again.", SCREEN_H/2 + 30 + (gameFont.h + gameFont.leading), ALPHA_OPAQUE, SHADOW_DROP);
	}

	if (gamePaused)
	{
		dTextCentered(&gameFont, "PAUSED", SCREEN_H/2 + 30, ALPHA_OPAQUE, SHADOW_DROP);

		if (gamePausedTimer >= PAUSE_RESUME_TIME)
			dTextCentered(&gameFont, "Press START to resume", SCREEN_H/2 + 30 + (gameFont.h + gameFont.leading) * 2, ALPHA_OPAQUE, SHADOW_DROP);
	}
}
