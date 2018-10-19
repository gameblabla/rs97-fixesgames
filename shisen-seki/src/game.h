#ifndef _GAME_H_
#define _GAME_H_

#define KEY_DELAY	12

typedef enum gameMode
{
	GAME_MODE_UNSET = 0,
	GAME_MODE_CLASSIC,
	GAME_MODE_GRAVITY,
	GAME_MODE_COUNT
} gameMode;

extern int showAnimations;
extern int showStoneRank;
extern int practice;
extern unsigned long gameTime;
extern int continueGame;
extern gameMode newGameMode;
extern gameMode currentGameMode;
extern int gameOver;

void gameUnload();
void gameLoad();
void gamePrepareHiscore();
void gameLogic();
void gameGuiDraw();
void gameDraw();

#endif /* _GAME_H_ */
