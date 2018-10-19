#ifndef _OBJECTS_H_
#define _OBJECTS_H_

#include "helpers.h"
#include "tileset.h"

extern listElement *objTemplateListHead;

typedef enum objectType
{
	OBJ_UNSET,
	OBJ_PLAYER,
	OBJ_MISSILE_RED,
	OBJ_MISSILE_BLUE,
	OBJ_MISSILE_YELLOW,
	OBJ_CLOUD,
	OBJ_SMOKE
} objectType;

typedef struct point
{
	int x;
	int y;
} point;

typedef struct object
{
	objectType type;
	tileset *tiles;
	int w;
	int h;
	float x;
	float y;
	float vx;
	float vy;
	int angle;
	int turnAngle;
	float turnSpeed;
	int turnStep;
	int dispose;
	int hitboxW;
	int hitboxH;
	int ttl;
	int smoking;
} object;

object *objectTemplate(objectType type);
void objectLoad(object *obj, objectType type);
void objectItemDelete(void *item);
void objectTemplateItemDelete(void *item);
int objectItemDisposedMatch(void *item);
int objectCollisionCheck(object *obj, object *obj2);
void objectLogic(object *obj);
void objectDraw(object *obj);

#endif /* _OBJECTS_H_ */
