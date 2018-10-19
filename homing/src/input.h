#ifndef _INPUT_H_
#define _INPUT_H_

#include <SDL.h>

#define JOY_DEADZONE		5000

enum KeyNames
{
	KEY_D_SLOMO	= SDLK_0,
	KEY_D_SPAWN_ENEMY = SDLK_a,
	KEY_BACK	= SDLK_ESCAPE,
	KEY_START	= SDLK_RETURN,
	KEY_LEFT	= SDLK_LEFT,
	KEY_RIGHT	= SDLK_RIGHT,
	KEY_UP		= SDLK_UP,
	KEY_DOWN	= SDLK_DOWN,
	KEY_OK		= SDLK_LCTRL
};

typedef enum JoystickMode
{
	JOY_MODE_DISABLED = 0,
	JOY_MODE_DIGITAL,
	JOY_MODE_ANALOG
} JoystickMode;

typedef struct JoystickData
{
	int x;
	int y;
	int inDeadzoneX;
	int inDeadzoneY;
} JoystickData;

extern int keys[2048];
extern SDL_Joystick *joyDevice;
extern int joyNum;
extern JoystickMode joyMode;
extern JoystickData joyData;
extern int joyDeadzone;

void input();

#endif /* _INPUT_H_ */
