#include "wiz.h"

#include "SDL.h"

#include <cstdio>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <fcntl.h>
#include <unistd.h>

int volume = 40;
int volume_direction;
 
void WIZ_AdjustVolume( int direction )
{
	if( direction != VOLUME_NOCHG )
	{
		if( volume <= 10 )
		{
			if( direction == VOLUME_UP )   volume += VOLUME_CHANGE_RATE/2;
			if( direction == VOLUME_DOWN ) volume -= VOLUME_CHANGE_RATE/2;
		}
		else
		{
			if( direction == VOLUME_UP )   volume += VOLUME_CHANGE_RATE;
			if( direction == VOLUME_DOWN ) volume -= VOLUME_CHANGE_RATE;
		}

		if( volume < VOLUME_MIN ) volume = VOLUME_MIN;
		if( volume > VOLUME_MAX ) volume = VOLUME_MAX;

		printf( "Volume Change: %i\n", volume );
	}
	unsigned long soundDev = open("/dev/mixer", O_RDWR);
	if(soundDev)
	{
		int vol = ((volume << 8) | volume);
		ioctl(soundDev, SOUND_MIXER_WRITE_PCM, &vol);
		close(soundDev);
	}
}

void WIZ_KeyboardEmulation( int button, int value )
{
	SDL_Event fakeevent1;
	SDL_Event fakeevent2;

	//printf( "Button %d Value %d\n", button, value );

	if( value == 1 ) {
	      fakeevent1.type			= SDL_KEYDOWN;
	      fakeevent1.key.state		= SDL_PRESSED;
	      fakeevent1.key.type		= SDL_KEYDOWN;
	      fakeevent1.key.keysym.mod		= KMOD_NONE;

	      fakeevent2.type			= SDL_KEYDOWN;
	      fakeevent2.key.state		= SDL_PRESSED;
	      fakeevent2.key.type		= SDL_KEYDOWN;
	      fakeevent2.key.keysym.mod		= KMOD_NONE;
	}
	else {
	      fakeevent1.type			= SDL_KEYUP;
	      fakeevent1.key.state		= SDL_RELEASED;
	      fakeevent1.key.type		= SDL_KEYUP;
	      fakeevent1.key.keysym.mod		= KMOD_NONE;

	      fakeevent2.type			= SDL_KEYUP;
	      fakeevent2.key.state		= SDL_RELEASED;
	      fakeevent2.key.type		= SDL_KEYUP;
	      fakeevent2.key.keysym.mod		= KMOD_NONE;
	}

	fakeevent1.key.keysym.sym = SDLK_UNKNOWN;
	fakeevent2.key.keysym.sym = SDLK_UNKNOWN;
	switch(button)
	{
		case GP2X_BUTTON_LEFT:
			fakeevent1.key.keysym.sym = SDLK_LEFT;
			break;
		case GP2X_BUTTON_RIGHT:
			fakeevent1.key.keysym.sym = SDLK_RIGHT;
			break;
		case GP2X_BUTTON_UP:
			fakeevent1.key.keysym.sym = SDLK_UP;
			break;
		case GP2X_BUTTON_DOWN:
			fakeevent1.key.keysym.sym = SDLK_DOWN;
			break;
		case GP2X_BUTTON_UPLEFT:
			fakeevent1.key.keysym.sym = SDLK_UP;
			fakeevent2.key.keysym.sym = SDLK_LEFT;
			break;
		case GP2X_BUTTON_UPRIGHT:
			fakeevent1.key.keysym.sym = SDLK_UP;
			fakeevent2.key.keysym.sym = SDLK_RIGHT;
			break;
		case GP2X_BUTTON_DOWNLEFT:
			fakeevent1.key.keysym.sym = SDLK_DOWN;
			fakeevent2.key.keysym.sym = SDLK_LEFT;
			break;
		case GP2X_BUTTON_DOWNRIGHT:
			fakeevent1.key.keysym.sym = SDLK_DOWN;
			fakeevent2.key.keysym.sym = SDLK_RIGHT;
			break;
		case GP2X_BUTTON_SELECT:
			fakeevent1.key.keysym.sym = SDLK_RETURN;
			break;
		case GP2X_BUTTON_START:
			fakeevent1.key.keysym.sym = SDLK_ESCAPE;
			break;
		case GP2X_BUTTON_L:
			fakeevent1.key.keysym.sym = SDLK_TAB;
			break;
		case GP2X_BUTTON_R:
			fakeevent1.key.keysym.sym = SDLK_BACKSPACE;
			break;
		case GP2X_BUTTON_A:
			fakeevent1.key.keysym.sym = SDLK_RCTRL;
			break;
		case GP2X_BUTTON_B:
			fakeevent1.key.keysym.sym = SDLK_RSHIFT;
			break;
		case GP2X_BUTTON_X:
			fakeevent1.key.keysym.sym = SDLK_UP;
			break;
		case GP2X_BUTTON_Y:
			fakeevent1.key.keysym.sym = SDLK_DOWN;
			break;
		case GP2X_BUTTON_VOLUP:
			if( value == 1)
				volume_direction = VOLUME_UP;
			else
				volume_direction = VOLUME_NOCHG;
			break;
		case GP2X_BUTTON_VOLDOWN:
			if( value == 1)
				volume_direction = VOLUME_DOWN;
			else
				volume_direction = VOLUME_NOCHG;
			break;
	}

	if( fakeevent1.key.keysym.sym != SDLK_UNKNOWN )
	{
		SDL_PushEvent (&fakeevent1);
	}
	if( fakeevent2.key.keysym.sym != SDLK_UNKNOWN )
	{
		SDL_PushEvent (&fakeevent2);
	}

	WIZ_AdjustVolume(volume_direction);
}
