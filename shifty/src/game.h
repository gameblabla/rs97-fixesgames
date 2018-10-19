#ifndef _GAME_H_
#define _GAME_H_

#define GRAVITY_SPEED	15

typedef enum GameMode
{
	GAME_MODE_SOLO,
	GAME_MODE_LOCAL,
	GAME_MODE_NETWORKED
} GameMode;

typedef enum GameState
{
	GAME_STATE_OVER,
	GAME_STATE_PAUSED,
	GAME_STATE_PLAYING
} GameState;

typedef struct GameModifier
{
	int shiftEvent;
	int junkBlocks;
	int debugInfo;
} GameModifier;

typedef struct Game
{
	GameMode mode;
	GameState state;
	GameModifier modifiers;
	int ticks;
} Game;

extern Game game;

void gameLoad();
void gameUnload();
void gameLogic();
void gameDrawing();

#endif /* _GAME_H_ */
