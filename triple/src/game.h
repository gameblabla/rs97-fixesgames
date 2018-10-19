#ifndef _GAME_H_
#define _GAME_H_

#include "block.h"

#define POINTS_INFO_TIMER	120

extern int pause;
extern int over;
extern int overTimer;
extern int score;
extern int clearedInitial;
extern int clearedPoints;
extern int clearedColors[COLOR_NUM - 2];
extern unsigned int gameTime;
extern char pointsStr[30];
extern int pointsStrTimer;

void gameUnload();
void gameLoad();
void gameLogic();
void gameDraw();

#endif /* _GAME_H_ */
