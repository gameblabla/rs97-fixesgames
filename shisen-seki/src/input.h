#ifndef _INPUT_H_
#define _INPUT_H_

#include <SDL.h>

#define JOY_DEADZONE		5000

extern int keys[2048];
extern int mouse[2];
extern int mouseMoved;
extern int enableJoystick;

enum KeyNames
{
	KEY_BACK	= SDLK_ESCAPE,
	KEY_START	= SDLK_RETURN,
	KEY_LEFT	= SDLK_LEFT,
	KEY_RIGHT	= SDLK_RIGHT,
	KEY_UP		= SDLK_UP,
	KEY_DOWN	= SDLK_DOWN,
	KEY_OK		= SDLK_LCTRL,
	KEY_CANCEL	= SDLK_LALT,
	KEY_EXTRA	= SDLK_SPACE
};

void input();
void updateMouse();

#endif /* _INPUT_H_ */
