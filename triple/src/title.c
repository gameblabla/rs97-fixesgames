#include "title.h"

#include <stdlib.h>
#include "font.h"
#include "game.h"
#include "input.h"
#include "states.h"
#include "video.h"

char logo[] =
{
'?','?','?',' ','?','?',' ',' ','?',' ','?','?',' ',' ','?',' ',' ','?','?', '\n', \
' ','?',' ',' ','?',' ','?',' ','?',' ','?',' ','?',' ','?',' ',' ','?',' ', '\n', \
' ','?',' ',' ','?','?',' ',' ','?',' ','?','?',' ',' ','?',' ',' ','?','?', '\n', \
' ','?',' ',' ','?',' ','?',' ','?',' ','?',' ',' ',' ','?',' ',' ','?',' ', '\n', \
' ','?',' ',' ','?',' ','?',' ','?',' ','?',' ',' ',' ','?','?',' ','?','?'
};

char logo2[] =
{
'?','?','?',' ','?','?',' ',' ',' ','?',' ',' ','?','?',' ',' ','?',' ',' ','?','?',' ','?','?',' ', '\n', \
' ','?',' ',' ','?',' ','?',' ','?',' ','?',' ','?',' ','?',' ','?',' ',' ','?',' ',' ','?',' ','?', '\n', \
' ','?',' ',' ','?','?',' ',' ','?','?','?',' ','?','?',' ',' ','?',' ',' ','?','?',' ','?',' ','?', '\n', \
' ','?',' ',' ','?',' ','?',' ','?',' ','?',' ','?',' ',' ',' ','?',' ',' ','?',' ',' ','?',' ','?', '\n', \
' ','?',' ',' ','?',' ','?',' ','?',' ','?',' ','?',' ',' ',' ','?','?',' ','?','?',' ','?','?',' '
};


int toHiscoreTimer;
int clearLogo;

void titleUnload()
{
}

void titleLoad()
{
	toHiscoreTimer = 600;
	score = 0;
	clearLogo = 0;
}

void titleLogic()
{
	if(!toHiscoreTimer-- && !clearLogo)
	{
		programStateNew = STATE_HISCORE;
	}

	if(!(toHiscoreTimer % 15)) // Every 0.25 seconds change the logo colors.
	{
		int i;

		for(i = 0; logo[i]; ++i)
		{
			if(logo[i] != ' ' && logo[i] != '\n')
			{
				logo[i] = '?';
			}
		}
		for(i = 0; logo2[i]; ++i)
		{
			if(logo2[i] != ' ' && logo2[i] != '\n')
			{
				logo2[i] = '?';
			}
		}
	}

	if(keys[KEY_BACK] && !clearLogo)
	{
		keys[KEY_BACK] = 0;
		clearLogo = 1;
	}

	if(keys[KEY_START] && !clearLogo)
	{
		keys[KEY_START] = 0;
		programStateNew = STATE_GAME;
	}

	if(keys[KEY_OK] && !clearLogo)
	{
		keys[KEY_OK] = 0;
		programStateNew = STATE_GAME;
	}

	if(clearLogo)
	{
		//if(!(clearLogo % 2))
		{
			int clearedChar = 0;
			int logoLen = strlen(logo);
			int logo2Len = strlen(logo2);
			int c;
			int i;

			for(i = 0; logo[i]; ++i)
			{
				if(logo[i] != ' ' && logo[i] != '\n')
				{
					c = rand() % logoLen;

					while(!clearedChar)
					{
						if(c >= logoLen)
						{
							c = 0;
						}

						if(logo[c] == ' ')
						{
							c++;
							continue;
						}
						if(logo[c] == '\n')
						{
							c++;
							continue;
						}

						logo[c] = ' ';
						clearedChar = 1;
					}

					break;
				}
			}

			clearedChar = 0;
			c = rand() % logo2Len;

			for(i = 0; logo2[i]; ++i)
			{
				if(logo2[i] != ' ' && logo2[i] != '\n')
				{
					while(!clearedChar)
					{
						if(c >= logo2Len)
						{
							c = 0;
						}

						if(logo2[c] == ' ')
						{
							c++;
							continue;
						}
						if(logo2[c] == '\n')
						{
							c++;
							continue;
						}

						logo2[c] = ' ';
						clearedChar = 1;
					}

					break;
				}
			}

			if(i >= logo2Len)
			{
				clearLogo = 0;
				quit = 1;
			}
		}

		clearLogo++;
	}
}

void titleDraw()
{
	clearScreen();
	dTextCentered(logo, 10, SDL_MapRGB(screen->format, 255, 255, 0));
	dTextCentered(logo2, 80, SDL_MapRGB(screen->format, 255, 255, 0));
	if(!clearLogo)
	{
		dTextCentered("GO  - A\nBYE - SEL", SCREEN_H/2 + 50, SDL_MapRGB(screen->format, 255, 255, 0));
	}
}
