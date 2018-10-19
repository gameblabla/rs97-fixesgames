#include "input.h"

#include <SDL.h>
#include "main.h"

SDL_Event event;
int keys[2048];

void input()
{
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_QUIT:
				quit = 1;
			break;
			case SDL_KEYDOWN:			// Button press
				keys[event.key.keysym.sym] = 1;
				if(event.key.keysym.sym == 51){
					keys[SDLK_ESCAPE] = 1;
				}
			break;
			case SDL_KEYUP:				// Button release
				keys[event.key.keysym.sym] = 0;
				if(event.key.keysym.sym == 51){
					keys[SDLK_ESCAPE] = 0;
				}
			break;
			case SDL_JOYAXISMOTION:			// Analog joystick movement
				switch(event.jaxis.axis)
				{
					case 0:		// axis 0 (left-right)
						if(event.jaxis.value < -JOY_DEADZONE)
						{
							// left movement
							keys[SDLK_LEFT] = 1;
							keys[SDLK_RIGHT] = 0;
						}
						else if(event.jaxis.value > JOY_DEADZONE)
						{
							// right movement
							keys[SDLK_LEFT] = 0;
							keys[SDLK_RIGHT] = 1;
						}
						else
						{
							keys[SDLK_LEFT] = 0;
							keys[SDLK_RIGHT] = 0;
						}
					break;
					case 1:		// axis 1 (up-down)
						if(event.jaxis.value < -JOY_DEADZONE)
						{
							// up movement
							keys[SDLK_UP] = 1;
							keys[SDLK_DOWN] = 0;
						}
						else if(event.jaxis.value > JOY_DEADZONE)
						{
							// down movement
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

			default:
			break;
		}
	}
}
