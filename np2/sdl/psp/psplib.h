#ifndef PSPLIB_H

#define PSPLIB_H

#include <SDL.h>

void plPrint(int x, int y, int color, char *s);
void plClrChars(int x, int y, int w);
void plPrintErr(char *s);
void plSetSurf(SDL_Surface *ps);

void plClear(void);

#endif
