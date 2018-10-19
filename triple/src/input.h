#ifndef _INPUT_H_
#define _INPUT_H_

#include <SDL.h>

#define JOY_DEADZONE		5000

void input();

extern int keys[2048];

enum KeyNames
{
	KEY_BACK	= SDLK_ESCAPE,
	KEY_START	= SDLK_RETURN,
	KEY_OK		= SDLK_LCTRL,
	KEY_LEFT	= SDLK_LEFT,
	KEY_RIGHT	= SDLK_RIGHT,
	KEY_UP		= SDLK_UP,
	KEY_DOWN	= SDLK_DOWN
};

#endif /* _INPUT_H_ */
