#ifndef _BLOCK_H_
#define _BLOCK_H_

#include "tileset.h"

#define BLOCK_SIZE			15
#define BLOCK_ANIM_SPEED	4
#define BLOCK_ANIM_MAX		4
#define BLOCK_ANIM_OFFSET	(9*5)

typedef enum BlockType
{
	BLOCK_TYPE_EMPTY,
	BLOCK_TYPE_A,
	BLOCK_TYPE_B,
	BLOCK_TYPE_C,
	BLOCK_TYPE_D,
	BLOCK_TYPE_JUNK,
	BLOCK_TYPE_COUNT
} BlockType;

typedef enum BlockConnection
{
	BLOCK_CON_NONE		= 0,
	BLOCK_CON_TOP		= (1 << 0),
	BLOCK_CON_RIGHT		= (1 << 1),
	BLOCK_CON_BOTTOM	= (1 << 2),
	BLOCK_CON_LEFT		= (1 << 3),
	BLOCK_CON_TOP_LEFT = BLOCK_CON_TOP | BLOCK_CON_LEFT,
	BLOCK_CON_TOP_RIGHT = BLOCK_CON_TOP | BLOCK_CON_RIGHT,
	BLOCK_CON_BOTTOM_LEFT = BLOCK_CON_BOTTOM | BLOCK_CON_LEFT,
	BLOCK_CON_BOTTOM_RIGHT = BLOCK_CON_BOTTOM | BLOCK_CON_RIGHT
} BlockConnection;

typedef struct Block
{
	BlockType type;
	BlockConnection connection;
	int animationCounter;
	int animate;
} Block;

Tileset *blocksTileset;

#endif /* _BLOCK_H_ */
