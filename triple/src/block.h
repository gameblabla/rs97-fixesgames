#ifndef _BLOCK_H_
#define _BLOCK_H_

#define BLOCK_SIZE		16
#define EXPLOSION_SPEED		30
#define BLOCK_BASE_SPEED	1.6
#define BLOCK_COUNTER_LIMIT	105

typedef enum Color
{
	COLOR_ERASER = -1,
	COLOR_NONE = 0,
	COLOR_WHITE,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_MAGENTA,
	COLOR_YELLOW,
	COLOR_CYAN,
	COLOR_NUM	// Stores the number of colors
} Color;

typedef struct BlocksPos
{
	int x;
	int y;
	int ax;
	int ay;
} BlocksPos;

typedef struct Block
{
	Color color;
	float x;
	float y;
	float vx;
	float vy;
} Block;

typedef struct BlockNode
{
	struct BlockNode *next;
	Block block;
} BlockNode;

typedef struct Explosion
{
	Color color;
	float x;
	float y;
	int timer;
	int step;
} Explosion;

typedef struct ExplosionNode
{
	struct ExplosionNode *next;
	Explosion explosion;
} ExplosionNode;

extern BlockNode *blockHead;
extern ExplosionNode *explosionHead;
extern int blockSpawnCounter;
extern int blockSpawnCounterLimit;
extern float blockSpeed;

BlockNode *blockNodePrepend(BlockNode *head);
BlockNode *blockNodeDelete(BlockNode *head, BlockNode *toDelNode);
BlockNode *blockNodeDeleteAll(BlockNode *head);
ExplosionNode *explosionNodePrepend(ExplosionNode *head);
ExplosionNode *explosionNodeDelete(ExplosionNode *head, ExplosionNode *toDelNode);
ExplosionNode *explosionNodeDeleteAll(ExplosionNode *head);
void blockCheckCollision(Block *block);
void blockDrawExplosions();
void blockDrawBlocks();

#endif /* _BLOCK_H_ */
