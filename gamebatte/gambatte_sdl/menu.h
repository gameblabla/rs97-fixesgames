#ifndef _MENU_H
#define _MENU_H

#include <gambatte.h>
#include "src/blitterwrapper.h"

void menu_set_screen(SDL_Surface *set_screen);
int init_menu();
void main_menu(gambatte::GB *gambatte, BlitterWrapper *blitter);
void show_fps(SDL_Surface *surface, int fps);


#endif
