#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include <SDL.h>

#define PAD_BUTTON1 (1) // A
#define PAD_BUTTON2 (1<<1) // B
#define PAD_BUTTON3 (1<<2) // X
#define PAD_BUTTON4 (1<<3) // Y
#define PAD_L (1<<4)
#define PAD_R (1<<5)
#define PAD_START (1<<6)
#define PAD_SELECT (1<<7)
#define PAD_UP (1<<8)
#define PAD_DOWN (1<<9)
#define PAD_LEFT (1<<10)
#define PAD_RIGHT (1<<11)

int pad_is_quit(void);
unsigned int pad_getdata(void);
unsigned int pad_event(SDL_Event *evt);
unsigned int pad_poll(void);

#endif
