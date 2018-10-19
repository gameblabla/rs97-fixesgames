#include "input.h"

#include <SDL.h>
#include "states.h"

SDL_Event event;
int keys[2048];
SDL_Joystick *joyDevice;
int joyNum;
JoystickMode joyMode = JOY_MODE_ANALOG;
JoystickData joyData = {0, 0, 1, 1};
int joyDeadzone = JOY_DEADZONE;

void input()
{
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_QUIT:
				quit = 1;
			break;
			case SDL_KEYDOWN:			/* Key press. */
				keys[event.key.keysym.sym] = 1;
				if(event.key.keysym.sym == 51){
					keys[SDLK_ESCAPE] = 1;
				}
			break;
			case SDL_KEYUP:				/* Key release. */
				keys[event.key.keysym.sym] = 0;
				if(event.key.keysym.sym == 51){
					keys[SDLK_ESCAPE] = 0;
				}
			break;
			case SDL_JOYHATMOTION:			/* Joystick hat movement. */
				keys[SDLK_UP] = event.jhat.value & SDL_HAT_UP;
				keys[SDLK_DOWN] = event.jhat.value & SDL_HAT_DOWN;
				keys[SDLK_LEFT] = event.jhat.value & SDL_HAT_LEFT;
				keys[SDLK_RIGHT] = event.jhat.value & SDL_HAT_RIGHT;
			break;
			case SDL_JOYAXISMOTION:			/* Analog joystick movement. */
				switch (joyMode)
				{
					case JOY_MODE_DISABLED:
					break;

					case JOY_MODE_DIGITAL:
						switch(event.jaxis.axis)
						{
							case 0:		/* Axis 0 (left-right). */
								if(event.jaxis.value < -joyDeadzone)
								{
									/* Left movement. */
									keys[SDLK_LEFT] = 1;
									keys[SDLK_RIGHT] = 0;
								}
								else if(event.jaxis.value > joyDeadzone)
								{
									/* Right movement. */
									keys[SDLK_LEFT] = 0;
									keys[SDLK_RIGHT] = 1;
								}
								else
								{
									keys[SDLK_LEFT] = 0;
									keys[SDLK_RIGHT] = 0;
								}
							break;
							case 1:		/* Axis 1 (up-down). */
								if(event.jaxis.value < -joyDeadzone)
								{
									/* Up movement. */
									keys[SDLK_UP] = 1;
									keys[SDLK_DOWN] = 0;
								}
								else if(event.jaxis.value > joyDeadzone)
								{
									/* Down movement. */
									keys[SDLK_UP] = 0;
									keys[SDLK_DOWN] = 1;
								}
								else
								{
									keys[SDLK_UP] = 0;
									keys[SDLK_DOWN] = 0;
								}
							break;

							default:
							break;
						}
					break;
					case JOY_MODE_ANALOG:
						switch(event.jaxis.axis)
						{
							case 0:
								joyData.x = event.jaxis.value;
								joyData.inDeadzoneX = (event.jaxis.value > -joyDeadzone && event.jaxis.value < joyDeadzone) ? 1 : 0;
							break;
							case 1:
								joyData.y = event.jaxis.value;
								joyData.inDeadzoneY = (event.jaxis.value > -joyDeadzone && event.jaxis.value < joyDeadzone) ? 1 : 0;
							break;

							default:
							break;
						}
					break;

					default:
					break;
				}
			break;
			case SDL_JOYBUTTONDOWN:			/* Joystick button press. */
				switch (event.jbutton.button)
				{
					case 0:
						keys[KEY_OK] = 1;
					break;
					case 1:
						keys[KEY_BACK] = 1;
					break;

					default:
					break;
				}
			break;
			case SDL_JOYBUTTONUP:			/* Joystick button release. */
				switch (event.jbutton.button)
				{
					case 0:
						keys[KEY_OK] = 0;
					break;
					case 1:
						keys[KEY_BACK] = 0;
					break;

					default:
					break;
				}
			break;

			default:
			break;
		}
	}
}
