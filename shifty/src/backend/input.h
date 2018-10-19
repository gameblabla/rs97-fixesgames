#ifndef _INPUT_H_
#define _INPUT_H_

enum KeyNames
{
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_ROTATE_LEFT,
	KEY_ROTATE_RIGHT,
	KEY_DROP,
	KEY_OK,
	KEY_BACK,
	KEY_COUNT
};

extern int keys[2048];

int *playerKeys[KEY_COUNT];

void input();
void defaultKeymap(int **keyMap, int playerNum);

#endif /* _INPUT_H_ */
