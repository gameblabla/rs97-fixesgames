#include "font.h"

#include <SDL.h>
#include "game.h"
#include "main.h"
#include "video.h"

void dTextCentered(char *string, int y, int rcolor)
{
	int len;

	for(len = 0; string[len]; ++len)
	{
		if(string[len] == '\n')
		{
			break;
		}
	}

	++len;

	len = SCREEN_W/2 - (len*6 + (len-1)*3)/2;

	dText(string, len, y, rcolor);
}

void dText(char *string, int x, int y, int rcolor)
{
	int sep = 3;
	int maxW = 6;
	int maxH = 10;
	int i;
	int j;
	int origX = x;
	SDL_Rect r;

	SDL_Rect display[7] =
	{
		// x, y, w, h
		{ 0, y, maxW, 1 },			// top
		{ 0, y, 1, maxH/2 },			// top-left
		{ maxW, y, 1, maxH/2 },			// top-right
		{ 0, y + maxH/2 - 1, maxW, 1 },		// middle
		{ 0, y + maxH/2 - 1, 1, maxH/2+1 },	// bottom-left
		{ maxW, y + maxH/2 - 1, 1, maxH/2+1 },	// bottom-right
		{ 0, y + maxH - 1, maxW, 1 }		// bottom
	};

	for(i = 0; string[i]; i++)
	{
		if(string[i] == '\n')
		{
			dText(&string[i+1], origX, y+ maxH + sep, rcolor);
			return;
		}

		for(j = 0; j < 7; j++) // Update display position.
		{
			display[j].x = x + i*maxW;

			if(j == 2 || j == 5)
			{
				display[j].x += maxW - 1;
			}
		}

		if(string[i] == '?') // Generate a random mini-block
		{
			switch(rand() % 6)
			{
				case 0:
					string[i] = 'r';
				break;
				case 1:
					string[i] = 'g';
				break;
				case 2:
					string[i] = 'b';
				break;
				case 3:
					string[i] = 'm';
				break;
				case 4:
					string[i] = 'y';
				break;
				case 5:
					string[i] = 'c';
				break;

				default:
				break;
			}
		}

		switch(string[i])
		{
			case '1':
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
			break;
			case '2':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[3], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case '3':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[3], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case '4':
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[3], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
			break;
			case '5':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[3], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case '6':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[3], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case '7':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
			break;
			case '8':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[3], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case '9':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[3], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case '0':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case 'A':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[3], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
			break;
			case 'B':
/*				SDL_FillRect(screen, &display[1], rcolor);*/
/*				SDL_FillRect(screen, &display[3], rcolor);*/
/*				SDL_FillRect(screen, &display[4], rcolor);*/
/*				SDL_FillRect(screen, &display[5], rcolor);*/
/*				SDL_FillRect(screen, &display[6], rcolor);*/
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[3], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case 'C':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case 'E':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[3], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case 'F':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[3], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
			break;
			case 'G':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case 'H':
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[3], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
			break;
			case 'I':
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
			break;
			case 'J':
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case 'L':
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case 'N':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
			break;
			case 'O':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case 'P':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[3], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
			break;
			case 'S':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[3], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case 'T':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
			break;
			case 'U':
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case '[':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[4], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case 'Y':
				SDL_FillRect(screen, &display[1], rcolor);
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[3], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
			break;
			case ']':
				SDL_FillRect(screen, &display[0], rcolor);
				SDL_FillRect(screen, &display[2], rcolor);
				SDL_FillRect(screen, &display[5], rcolor);
				SDL_FillRect(screen, &display[6], rcolor);
			break;
			case '-':
				SDL_FillRect(screen, &display[3], rcolor);
			break;
			// Mini-blocks.
			case 'e':
				r.x = x + i*maxW;
				r.y = y;
				r.w = maxW;
				r.h = maxH;
				if(blinkTimer > 10)
				{
					SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 255, 255, 255));
				}
			break;
			case 'w':
				r.x = x + i*maxW;
				r.y = y;
				r.w = maxW;
				r.h = maxH;
				SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 255, 255, 255));
			break;
			case 'r':
				r.x = x + i*maxW;
				r.y = y;
				r.w = maxW;
				r.h = maxH;
				SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 255, 0, 0));
			break;
			case 'g':
				r.x = x + i*maxW;
				r.y = y;
				r.w = maxW;
				r.h = maxH;
				SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 0, 100, 0));
			break;
			case 'b':
				r.x = x + i*maxW;
				r.y = y;
				r.w = maxW;
				r.h = maxH;
				SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 0, 0, 255));
			break;
			case 'm':
				r.x = x + i*maxW;
				r.y = y;
				r.w = maxW;
				r.h = maxH;
				SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 255, 0, 255));
			break;
			case 'y':
				r.x = x + i*maxW;
				r.y = y;
				r.w = maxW;
				r.h = maxH;
				SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 255, 255, 0));
			break;
			case 'c':
				r.x = x + i*maxW;
				r.y = y;
				r.w = maxW;
				r.h = maxH;
				SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 0, 200, 200));
			break;
			// Other symbols.
			case ':':
				r.x = x + i*maxW;
				r.y = y + 2;
				r.w = 1;
				r.h = 1;
				SDL_FillRect(screen, &r, rcolor);
				r.y = y + maxH - 2;
				SDL_FillRect(screen, &r, rcolor);
			break;
			case '.':
				r.x = x + i*maxW;
				r.y = y + maxH;
				r.w = 1;
				r.h = 1;
				SDL_FillRect(screen, &r, rcolor);
			break;
			case '/':
				r.x = x + i*maxW + 5;
				r.y = y;
				r.w = 1;
				r.h = 4;
				SDL_FillRect(screen, &r, rcolor);
				r.x = x + i*maxW + 4;
				r.y = y + 4;
				r.w = 1;
				r.h = 4;
				SDL_FillRect(screen, &r, rcolor);
				r.x = x + i*maxW + 3;
				r.y = y + 8;
				r.w = 1;
				r.h = 3;
				SDL_FillRect(screen, &r, rcolor);
			break;
			case '+':
				r.x = x + i*maxW + maxW/2;
				r.y = y + 3;
				r.w = 1;
				r.h = 5;
				SDL_FillRect(screen, &r, rcolor);
				r.x = x + i*maxW + 1;
				r.y = y + maxH/2;
				r.w = 5;
				r.h = 1;
				SDL_FillRect(screen, &r, rcolor);
			break;
			case 'x':
				r.x = x + i*maxW + 1;
				r.y = y + 3;
				r.w = 1;
				r.h = 1;
				SDL_FillRect(screen, &r, rcolor);
				r.x = x + i*maxW + maxW - 1;
				r.y = y + 3;
				r.w = 1;
				r.h = 1;
				SDL_FillRect(screen, &r, rcolor);
				r.x = x + i*maxW + 2;
				r.y = y + 4;
				r.w = 1;
				r.h = 1;
				SDL_FillRect(screen, &r, rcolor);
				r.x = x + i*maxW + maxW - 2;
				r.y = y + 4;
				r.w = 1;
				r.h = 1;
				SDL_FillRect(screen, &r, rcolor);
				r.x = x + i*maxW + 3;
				r.y = y + 5;
				r.w = 1;
				r.h = 1;
				SDL_FillRect(screen, &r, rcolor);
				r.x = x + i*maxW + 1;
				r.y = y + maxH - 3;
				r.w = 1;
				r.h = 1;
				SDL_FillRect(screen, &r, rcolor);
				r.x = x + i*maxW + maxW - 1;
				r.y = y + maxH - 3;
				r.w = 1;
				r.h = 1;
				SDL_FillRect(screen, &r, rcolor);
				r.x = x + i*maxW + 2;
				r.y = y + maxH - 4;
				r.w = 1;
				r.h = 1;
				SDL_FillRect(screen, &r, rcolor);
				r.x = x + i*maxW + maxW - 2;
				r.y = y + maxH - 4;
				r.w = 1;
				r.h = 1;
				SDL_FillRect(screen, &r, rcolor);
			break;


			default:
			break;
		}

		x += sep;
	}
}
