#include "objects.h"

#include <math.h>
#include "game.h"
#include "helpers.h"
#include "input.h"
#include "tileset.h"
#include "video.h"

listElement *objTemplateListHead;

object *objectTemplate(objectType type)
{
	object newObj;
	listElement *curNode = objTemplateListHead;

	while(curNode)
	{
		object *curObj = (object *)curNode->item;

		if (curObj->type == type)
			return curObj;

		curNode = curNode->next;
	}

	memset(&newObj, 0, sizeof(object));
	newObj.type = type;
	newObj.tiles = malloc(sizeof(tileset));
	memset(newObj.tiles, 0, sizeof(tileset));

	switch (newObj.type)
	{
		case OBJ_PLAYER:
			tilesetLoad(newObj.tiles, "data/gfx/player.bmp", 16, 16, 8, 32);
			newObj.w = 16;
			newObj.h = 16;
			newObj.hitboxW = 10;
			newObj.hitboxH = 10;
		break;
		case OBJ_MISSILE_RED:
			tilesetLoad(newObj.tiles, "data/gfx/missileRed.bmp", 8, 8, 8, 16);
			newObj.w = 8;
			newObj.h = 8;
			newObj.hitboxW = 6;
			newObj.hitboxH = 6;
			newObj.ttl = 60*8;

			newObj.turnSpeed = 2.5f;
			newObj.turnStep = 9;
		break;
		case OBJ_MISSILE_BLUE:
			tilesetLoad(newObj.tiles, "data/gfx/missileBlue.bmp", 8, 8, 8, 16);
			newObj.w = 8;
			newObj.h = 8;
			newObj.hitboxW = 6;
			newObj.hitboxH = 6;
			newObj.ttl = 60*8;

			newObj.turnSpeed = 2.5f;
			newObj.turnStep = 3;
		break;
		case OBJ_MISSILE_YELLOW:
			tilesetLoad(newObj.tiles, "data/gfx/missileYellow.bmp", 8, 8, 8, 16);
			newObj.w = 8;
			newObj.h = 8;
			newObj.hitboxW = 6;
			newObj.hitboxH = 6;
			newObj.ttl = 60*8;

			newObj.turnSpeed = 2.5f;
			newObj.turnStep = 6;
		break;
		case OBJ_CLOUD:
			tilesetLoad(newObj.tiles, "data/gfx/cloud.bmp", 3, 3, 1, 1);
			newObj.w = 3;
			newObj.h = 3;
			newObj.hitboxW = 0;
			newObj.hitboxH = 0;
			newObj.ttl = 60*2;
		break;
		case OBJ_SMOKE:
			tilesetLoad(newObj.tiles, "data/gfx/smoke.bmp", 5, 5, 1, 1);
			newObj.w = 5;
			newObj.h = 5;
			newObj.hitboxW = 0;
			newObj.hitboxH = 0;
			newObj.ttl = 60*2;
		break;

		default:
			free(newObj.tiles);
		break;
	}

	objTemplateListHead = listElementPrepend(objTemplateListHead);
	objTemplateListHead->item = malloc(sizeof(object));
	memcpy(objTemplateListHead->item, &newObj, sizeof(object));

	return objTemplateListHead->item;
}

void objectLoad(object *obj, objectType type)
{
	if (!obj)
		return;

	memcpy(obj, objectTemplate(type), sizeof(object));

	if (obj->type == OBJ_PLAYER)
		playerLastAngle = obj->angle;
}

void objectItemDelete(void *item)
{
	object *obj = (object *)item;

	free(obj);
}

void objectTemplateItemDelete(void *item)
{
	object *obj = (object *)item;

	tilesetUnload(obj->tiles);
	free(obj->tiles);
	obj->tiles = NULL;
	free(obj);
}

int objectItemDisposedMatch(void *item)
{
	object *obj = (object *)item;

	if (!obj)
		return 0;

	return obj->dispose ? 1 : 0;
}

int objectCollisionCheck(object *obj, object *obj2)
{
	int x1;
	int y1;
	int x2;
	int y2;

	if (!obj || !obj2)
		return 0;

	x1 = obj->x + obj->w/2 - obj->hitboxW/2;
	y1 = obj->y + obj->h/2 - obj->hitboxH/2;
	x2 = obj2->x + obj2->w/2 - obj2->hitboxW/2;
	y2 = obj2->y + obj2->h/2 - obj2->hitboxH/2;

	if (x1 + obj->hitboxW-1 >= x2 &&
	    x1 <= x2 + obj2->hitboxW-1 &&
	    y1 + obj->hitboxH-1 >= y2 &&
	    y1 <= y2 + obj2->hitboxH-1)
	{
		return 1;
	}

	return 0;	
}

void objectLogic(object *obj)
{
	if (!obj)
		return;

	obj->x += obj->vx;
	obj->y += obj->vy;

	if (obj->x < 0)
		obj->x = LEVEL_W + obj->x;
	if (obj->x > LEVEL_W)
		obj->x = obj->x - LEVEL_W;
	if (obj->y < 0)
		obj->y = LEVEL_H + obj->y;
	if (obj->y > LEVEL_H)
		obj->y = obj->y - LEVEL_H;

	if (obj->type != OBJ_PLAYER && --obj->ttl <= 0)
	{
		obj->dispose = 1;
	}

	if (obj->type == OBJ_PLAYER)
	{
		int penaltyRange = 70;
		int inPenaltyRange = 0;

		if (joyMode == JOY_MODE_ANALOG && !(joyData.inDeadzoneX && joyData.inDeadzoneY))
		{
			int step = PLAYER_ROTATION;
			int desiredAngle;
			int A;
			int B;

			desiredAngle = atan2(joyData.y, joyData.x) * 180/PI;
			desiredAngle = MOD(desiredAngle, 360);
			desiredAngle = (desiredAngle + 90) % SINE_STEPS;
			desiredAngle = (desiredAngle - 359) * -1;

			A = obj->angle;
			B = desiredAngle;

			if (B - A >= 180)
			{
				obj->angle -= step;
			}
			else
			{
				if (B - A <= -180)
				{
					obj->angle += step;
				}
				else
				{
					obj->angle += ((B > A) ? step : 0) + ((B < A) ? -step : 0);
				}
			}

			if (abs(desiredAngle - obj->angle) < step)
				obj->angle = desiredAngle;

			obj->angle = MOD(obj->angle, SINE_STEPS);
		}

		obj->vx = -PLAYER_SPEED * sineTable[obj->angle];
		obj->vy = -PLAYER_SPEED * sineTable[(obj->angle+90)%SINE_STEPS];

		if (obj->smoking > 0)
		{
			--obj->smoking;

			if (!(gameTicks %3))
			{
				object newObj;

				objectLoad(&newObj, OBJ_SMOKE);
				newObj.x = (obj->x + obj->w/2 - newObj.w/2) - 3 + (rand()%6);
				newObj.y = (obj->y + obj->h/2 - newObj.h/2) - 3 + (rand()%6);

				objListHead = listElementPrepend(objListHead);
				objListHead->item = malloc(sizeof(object));
				memcpy(objListHead->item, &newObj, sizeof(object));
			}
		}

		if ((playerLastAngle - penaltyRange < 0) || (playerLastAngle + penaltyRange > SINE_STEPS))
		{
			if ((obj->angle > MOD(playerLastAngle - penaltyRange, SINE_STEPS)) || (obj->angle < MOD(playerLastAngle + penaltyRange, SINE_STEPS)))
				inPenaltyRange = 1;
		}
		else
		{
			if ((obj->angle > playerLastAngle - penaltyRange) && (obj->angle < playerLastAngle + penaltyRange))
				inPenaltyRange = 1;
		}

		if (inPenaltyRange)
		{
			if (!gameOverTimer && (++playerPenaltyTimer > PLAYER_PENALTY_TIME))
			{
				int radius = 400 + (rand() % 100);
				int angle = (obj->angle + 180) % SINE_STEPS;
				object newObj;

				playerPenaltyTimer = 0;
				playerLastAngle = obj->angle;

				objectLoad(&newObj, OBJ_MISSILE_BLUE);

				newObj.x = FMODF(obj->x + radius * sineTable[angle], LEVEL_W);
				newObj.y = FMODF(obj->y + radius * sineTable[(angle+90)%SINE_STEPS], LEVEL_H);
				newObj.angle = angle;

				objListHead = listElementPrepend(objListHead);
				objListHead->item = malloc(sizeof(object));
				memcpy(objListHead->item, &newObj, sizeof(object));
			}
		}
		else
		{
			playerLastAngle = obj->angle;
			playerPenaltyTimer = 0;
		}
	}
	else if (obj->type == OBJ_MISSILE_RED || obj->type == OBJ_MISSILE_BLUE || obj->type == OBJ_MISSILE_YELLOW)
	{
		if (playerObj)
		{
			int step = obj->turnStep;
			int desiredAngle;
			int A;
			int B;

			float x = (obj->x + obj->w/2) - (playerObj->x + playerObj->w/2);
			float y = (obj->y + obj->h/2) - (playerObj->y + playerObj->h/2);

			if (fabsf(x) > LEVEL_W/2)
			{
				x = FMODF(x - LEVEL_W, LEVEL_W);

				if (x > LEVEL_W/2)
					x = (x - LEVEL_W);
			}
			if (fabsf(y) > LEVEL_H/2)
			{
				y = FMODF(y - LEVEL_H, LEVEL_H);

				if (y > LEVEL_H/2)
					y = (y - LEVEL_H);

			}

			desiredAngle = atan2(y, x) * 180/PI;
			desiredAngle = MOD(desiredAngle, 360);
			desiredAngle = MOD(desiredAngle - 90, 360);
			desiredAngle = (desiredAngle - 359) * -1;

			A = obj->angle;
			B = desiredAngle;

			if (obj->type == OBJ_MISSILE_RED || obj->type == OBJ_MISSILE_YELLOW)
			{

				if (B - A >= 180)
				{
					if (obj->turnAngle > -step)
						--obj->turnAngle;
				}
				else
				{
					if (B - A <= -180)
					{
						if (obj->turnAngle < step)
							++obj->turnAngle;
					}
					else
					{
						if (B > A)
						{
						if (obj->turnAngle < step)
							++obj->turnAngle;
						}
						else if (B < A)
						{
							if (obj->turnAngle > -step)
								--obj->turnAngle;
						}
					}
				}

				obj->angle += obj->turnAngle/3;
			}
			else
			{
				if (B - A >= 180)
				{
					obj->angle -= step;
				}
				else
				{
					if (B - A <= -180)
					{
						obj->angle += step;
					}
					else
					{
						obj->angle += ((B > A) ? step : 0) + ((B < A) ? -step : 0);
					}
				}
			}

			obj->angle = MOD(obj->angle, SINE_STEPS);
		}

		obj->vx = -obj->turnSpeed * sineTable[obj->angle];
		obj->vy = -obj->turnSpeed * sineTable[(obj->angle+90)%SINE_STEPS];

		if (!(gameTicks % 3))
		{
			object newObj;

			objectLoad(&newObj, OBJ_CLOUD);
			newObj.x = obj->x + obj->w/2 - newObj.w/2;
			newObj.y = obj->y + obj->h/2 - newObj.h/2;

			objListHead = listElementPrepend(objListHead);
			objListHead->item = malloc(sizeof(object));
			memcpy(objListHead->item, &newObj, sizeof(object));
		}

		if (obj->dispose)
		{
			object newObj;

			objectLoad(&newObj, OBJ_SMOKE);
			newObj.x = obj->x + obj->w/2 - newObj.w/2;
			newObj.y = obj->y + obj->h/2 - newObj.h/2;

			objListHead = listElementPrepend(objListHead);
			objListHead->item = malloc(sizeof(object));
			memcpy(objListHead->item, &newObj, sizeof(object));
		}
	}
}

void objectDraw(object *obj)
{
	point camera;
	point objRel;

	if (!obj)
		return;

	camera.x = MOD((int)floor(playerObj->x + playerObj->w/2 - SCREEN_W/2), LEVEL_W);
	camera.y = MOD((int)floor(playerObj->y + playerObj->h/2 - SCREEN_H/2), LEVEL_H);

	objRel.x = MOD((int)floor(obj->x - (playerObj->x + playerObj->w/2 - SCREEN_W/2)), LEVEL_W);
	objRel.y = MOD((int)floor(obj->y - (playerObj->y + playerObj->h/2 - SCREEN_H/2)), LEVEL_H);

	if ((objRel.x + obj->w - 1 >= 0) && (objRel.x < SCREEN_W) && (objRel.y + obj->h - 1 >= 0) && (objRel.y < SCREEN_H))
	{
		int angle;
		int divisor;

		if (obj->type == OBJ_PLAYER)
			divisor = 11;
		else if (obj->type == OBJ_MISSILE_RED || obj->type == OBJ_MISSILE_BLUE || obj->type == OBJ_MISSILE_YELLOW)
			divisor = 22;
		else
			divisor = 1;

		angle = (360 - obj->angle)/divisor;

		if (obj->type == OBJ_CLOUD || obj->type == OBJ_SMOKE)
			SDL_SetAlpha(obj->tiles->image, SDL_SRCALPHA, obj->ttl > 60 ? 255 : 256/60 * obj->ttl);
		else if (obj->type == OBJ_MISSILE_RED || obj->type == OBJ_MISSILE_BLUE || obj->type == OBJ_MISSILE_YELLOW)
			SDL_SetAlpha(obj->tiles->image, SDL_SRCALPHA, obj->ttl > 30 ? 255 : 256/30 * obj->ttl);

		drawImage(obj->tiles->image, &obj->tiles->clip[angle%obj->tiles->length], screen, MOD((int)obj->x - camera.x, LEVEL_W), MOD((int)obj->y - camera.y, LEVEL_H));
	}
	else if (obj->type == OBJ_MISSILE_RED || obj->type == OBJ_MISSILE_BLUE || obj->type == OBJ_MISSILE_YELLOW)
	{
		int x = (objRel.x + obj->w - 1) > LEVEL_W/2 ? 0 : (objRel.x >= SCREEN_W ? SCREEN_W - 7 : objRel.x);
		int y = (objRel.y + obj->h - 1) > LEVEL_H/2 ? 0 : (objRel.y >= SCREEN_H ? SCREEN_H - 7 : objRel.y);

		drawImage(marker.image, &marker.clip[obj->type - OBJ_MISSILE_RED], screen, x, y);
	}
}
