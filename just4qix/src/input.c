/* maps platform control events (SDL) to generic controller
*/

#include <SDL/SDL.h>
#include "misc.h"
#include "input.h"


extern GameState;
bool CLEFT;
bool CRIGHT;
bool CDOWN;
bool CUP;
bool CFIRE;
bool CFILL;
bool CQUIT;

SDL_Event Event;

void KeyDown(SDL_keysym* keysym)
{
	int key = keysym->sym;
	switch (key) 	{
	case SDLK_ESCAPE:
		CQUIT = true;
		break;
	case SDLK_UP:
		CUP = true;
		break;
	case SDLK_DOWN:
		CDOWN = true;
		break;
	case SDLK_LEFT:
		CLEFT = true;
		break;
	case SDLK_RIGHT:
		CRIGHT = true;
		break;
	case SDLK_SPACE:
		CFIRE = true;
		break;
	case SDLK_LCTRL:
		CFIRE = true;
		break;
	}
}

void KeyUp(SDL_keysym* keysym)
{
	int key = keysym->sym;
	switch (key) 	{
	case SDLK_ESCAPE:
		CQUIT = false;
		break;
	case SDLK_UP:
		CUP = false;
		break;
	case SDLK_DOWN:
		CDOWN = false;
		break;
	case SDLK_LEFT:
		CLEFT = false;
		break;
	case SDLK_RIGHT:
		CRIGHT = false;
		break;
	case SDLK_SPACE:
		CFIRE = false;
		break;
	case SDLK_LCTRL:
		CFIRE =false;
		break;
	}
	
}



void Events(SDL_Event* Event){
	SDL_keysym keysym;
	switch (Event->type) {
	case SDL_QUIT:
		CQUIT = true;
		break;
	case SDL_KEYDOWN:
		keysym = Event->key.keysym;
		KeyDown(&keysym);
		break;
	case SDL_KEYUP:
		keysym = Event->key.keysym;
		KeyUp(&keysym);
		break;
	}
	
}

void GetInput(){
	while(SDL_PollEvent(&Event)) {
		Events(&Event);
	}
}