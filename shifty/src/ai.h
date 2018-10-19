#ifndef _AI_H_
#define _AI_H_

typedef enum AIMode
{
	AI_IDLE,
	AI_ANALYZE,
	AI_MOVE
} AIMode;

typedef struct AITarget
{
	int x;
	int y;
	int rotation;
} AITarget;

typedef struct AIMove
{
	AITarget target;
	unsigned int combo;
	unsigned int matchedBlocks;
	unsigned int height;
} AIMove;

typedef struct AI
{
	unsigned int enabled;
	AIMode mode;
	AITarget target;
} AI;

struct Player;

void aiMove(struct Player *player);

#endif /* _AI_H_ */
