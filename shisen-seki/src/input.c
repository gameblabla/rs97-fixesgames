#include "input.h"

#include <SDL.h>
#include "main.h"

SDL_Event event;
int keys[2048];
int mouse[2];
int mouseMoved;
int enableJoystick = 1;
int joyCanMoveX = 1;
int joyCanMoveY = 1;

void input()
{
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				quit = 1;
			break;
			case SDL_KEYDOWN:			// Button press
				keys[event.key.keysym.sym] = 1;
			break;
			case SDL_KEYUP:				// Button release
				keys[event.key.keysym.sym] = 0;
			break;
			case SDL_JOYAXISMOTION:			// Analog joystick movement
				if (!enableJoystick)
				{
					break;
				}

				switch (event.jaxis.axis)
				{
					case 0:		// axis 0 (left-right)
						if (event.jaxis.value < -JOY_DEADZONE)
						{
							if (joyCanMoveX)
							{
								joyCanMoveX = 0;
								// left movement
								keys[KEY_LEFT] = 1;
								keys[KEY_RIGHT] = 0;
							}
						}
						else if (event.jaxis.value > JOY_DEADZONE)
						{
							if (joyCanMoveX)
							{
								joyCanMoveX = 0;
								// right movement
								keys[KEY_LEFT] = 0;
								keys[KEY_RIGHT] = 1;
							}
						}
						else
						{
							joyCanMoveX = 1;
							keys[KEY_LEFT] = 0;
							keys[KEY_RIGHT] = 0;
						}
					break;
					case 1:		// axis 1 (up-down)
						if (event.jaxis.value < -JOY_DEADZONE)
						{
							if (joyCanMoveY)
							{
								joyCanMoveY = 0;
								// up movement
								keys[KEY_UP] = 1;
								keys[KEY_DOWN] = 0;
							}
						}
						else if (event.jaxis.value > JOY_DEADZONE)
						{
							if (joyCanMoveY)
							{
								joyCanMoveY = 0;
								// down movement
								keys[KEY_UP] = 0;
								keys[KEY_DOWN] = 1;
							}
						}
						else
						{
							joyCanMoveY = 1;
							keys[KEY_UP] = 0;
							keys[KEY_DOWN] = 0;
						}
					break;

					default:
					break;
				}
			break;
			case SDL_MOUSEBUTTONDOWN:
				switch (event.button.button)
				{
					case SDL_BUTTON_LEFT:
						keys[KEY_OK] = 1;
					break;
					case SDL_BUTTON_MIDDLE:
						keys[KEY_BACK] = 1;
					break;
					case SDL_BUTTON_RIGHT:
						keys[KEY_CANCEL] = 1;
					break;

					default:
					break;
				}
			break;
			case SDL_MOUSEBUTTONUP:
				switch (event.button.button)
				{
					case SDL_BUTTON_LEFT:
						keys[KEY_OK] = 0;
					break;
					case SDL_BUTTON_MIDDLE:
						keys[KEY_BACK] = 0;
					break;
					case SDL_BUTTON_RIGHT:
						keys[KEY_CANCEL] = 0;
					break;

					default:
					break;
				}
			break;

			default:
			break;
		}
	}

	updateMouse();
}

void updateMouse()
{
	int x;
	int y;

	SDL_GetMouseState(&x, &y);

	if (x != mouse[0] || y != mouse[1])
	{
		mouseMoved = 1;
		mouse[0] = x;
		mouse[1] = y;

		SDL_ShowCursor(SDL_ENABLE);
	}
	else
	{
		mouseMoved = 0;
	}
}
