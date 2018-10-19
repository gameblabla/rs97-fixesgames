#include <SDL.h>

#include "keypad.h"

#define DG_KEY_A SDLK_LCTRL
#define DG_KEY_B SDLK_LALT
#define DG_KEY_X SDLK_SPACE
#define DG_KEY_Y SDLK_LSHIFT

#define DG_KEY_L SDLK_TAB
#define DG_KEY_R SDLK_BACKSPACE
#define DG_KEY_START  SDLK_RETURN
#define DG_KEY_SELECT SDLK_ESCAPE

#define DG_KEY_UP    SDLK_UP
#define DG_KEY_DOWN  SDLK_DOWN
#define DG_KEY_LEFT  SDLK_LEFT
#define DG_KEY_RIGHT SDLK_RIGHT


static unsigned int pad_data = 0;
static int pad_quit = 0;


int pad_is_quit(void)
{
	return pad_quit;
}

unsigned int pad_getdata(void)
{
	return pad_data;
}

unsigned int pad_event(SDL_Event *evt)
{
	if (evt->type == SDL_QUIT)
	{
		pad_quit = 1;
	}
	else 
	if (evt->type == SDL_KEYDOWN)
	{
		switch(evt->key.keysym.sym)
		{
			case DG_KEY_A:
				pad_data |= PAD_BUTTON1;
			break;
			case DG_KEY_B:
				pad_data |= PAD_BUTTON2;
				break;
			case DG_KEY_X:
				pad_data |= PAD_BUTTON3;
				break;
			case DG_KEY_Y:
				pad_data |= PAD_BUTTON4;
				break;
			case DG_KEY_L:
				pad_data |= PAD_L;
				break;
			case DG_KEY_R:
				pad_data |= PAD_R;
				break;
			case DG_KEY_START:
				pad_data |= PAD_START;
				break;
			case DG_KEY_SELECT:
				pad_data |= PAD_SELECT;
				break;
			case DG_KEY_UP:
				pad_data |= PAD_UP;
				break;
			case DG_KEY_DOWN:
				pad_data |= PAD_DOWN;
				break;
			case DG_KEY_LEFT:
				pad_data |= PAD_LEFT;
				break;
			case DG_KEY_RIGHT:
				pad_data |= PAD_RIGHT;
				break;
		}
	}
	else
	if (evt->type == SDL_KEYUP)
	{
		
		switch(evt->key.keysym.sym)
		{
			case DG_KEY_A:
				pad_data &= ~PAD_BUTTON1;
				break;
			case DG_KEY_B:
				pad_data &= ~PAD_BUTTON2;
				break;
			case DG_KEY_X:
				pad_data &= ~PAD_BUTTON3;
				break;
			case DG_KEY_Y:
				pad_data &= ~PAD_BUTTON4;
				break;
			case DG_KEY_L:
				pad_data &= ~PAD_L;
				break;
			case DG_KEY_R:
				pad_data &= ~PAD_R;
				break;
			case DG_KEY_START:
				pad_data &= ~PAD_START;
				break;
			case DG_KEY_SELECT:
				pad_data &= ~PAD_SELECT;
				break;
			case DG_KEY_UP:
				pad_data &= ~PAD_UP;
				break;
			case DG_KEY_DOWN:
				pad_data &= ~PAD_DOWN;
				break;
			case DG_KEY_LEFT:
				pad_data &= ~PAD_LEFT;
				break;
			case DG_KEY_RIGHT:
				pad_data &= ~PAD_RIGHT;
				break;
		}		
	}
	
	return pad_data;
}

unsigned int pad_poll(void)
{
	SDL_Event evt;

	while(SDL_PollEvent(&evt) && !pad_quit)
	{
		pad_event(&evt);
	}
	
	return pad_data;
}


#ifdef PAD_TEST

SDL_Surface *gMain;

int pad_init(void)
{
	if (SDL_Init(SDL_INIT_VIDEO))
		return -1;

	gMain = SDL_SetVideoMode(320,240,16,0);
	
	if (!gMain)
		return -1;
}

void pad_release(void)
{
	SDL_Quit();
}

int main(int argc,char *argv[])
{
	unsigned data,olddata;
	
	pad_init();
	
	olddata = 0;
	
	while(!pad_quit)
	{
		do
		{
			SDL_Delay(10);
			data = pad_poll();
		}while(!pad_quit && olddata == data);
		
		printf("data = %x\n",data);
		
		olddata = data;
	}
	
	pad_release();
	
	return 0;
}

#endif
