#ifndef _TITLE_H_
#define _TITLE_H_

#include <SDL.h>

SDL_Surface *titleBackgroundIMG;

void titleUnload();
void titleLoad();
void titleLogic();
void titleDraw();

#endif /* _TITLE_H_ */
