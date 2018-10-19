#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <SDL.h>
#include "backend.h"

#define SCREEN_W		320
#define SCREEN_H		240
#define SCREEN_BPP	32
#define FPS				60.0
#define NO_FRAMELIMIT	0

typedef SDL_Surface Image;

typedef struct Rect
{
	int x;
	int y;
	int w;
	int h;
} Rect;

extern int screenScale;
extern int fullscreen;
extern int showFps;
extern unsigned int fps;

int videoInit();
void videoClean();
void updateScale();
void frameCounter();
int frameLimiter();
void flipScreen();
void clearScreen();
Image *loadImage(const char *fileName);
void unloadImage(Image *image);
void drawImage(Image *image, Rect *clip, int x, int y);
void drawDashedLine(int x, int y, unsigned int line, unsigned int length);

#endif /* _VIDEO_H_ */
