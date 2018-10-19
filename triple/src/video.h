#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <SDL.h>

#define SCREEN_W		320
#define SCREEN_H		240
#define SCREEN_BPP		16
#define FPS			60

extern int quit;
extern SDL_Surface *screen;
extern int blinkTimer;
extern int blinkTimerSlow;

int initSDL();
void deinitSDL();
int frameLimiter();
void blinkTimersTick();
void clearScreen();

#endif /* _VIDEO_H_ */
