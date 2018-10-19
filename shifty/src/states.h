#ifndef _STATES_H_
#define _STATES_H_

typedef enum State
{
	STATE_NONE,
	STATE_TITLE,
	STATE_GAME
} State;

extern int quit;
extern State stateNew;

void logic();
void drawing();

#endif /* _STATES_H_ */
