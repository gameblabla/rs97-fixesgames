#include "video.h"

#include <SDL.h>

SDL_Surface *ScreenSurface;
SDL_Surface *screen;
SDL_Surface *screenScaled;
int scale;
Uint32 curTicks;
Uint32 lastTicks = 0;

int initSDL()
{
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK))
	{
		return -1;
	}

	SDL_WM_SetCaption("Shisen-Seki", NULL);
	SDL_WM_SetIcon(loadImage("data/gfx/icon.bmp"), NULL);
	SDL_ShowCursor(SDL_DISABLE);

	updateScale();

	if(screen == NULL)
	{
		return -1;
	}

	if(SDL_NumJoysticks() > 0)
	{
		SDL_JoystickOpen(0);
	}

	return 0;
}

void deinitSDL()
{
	if(SDL_NumJoysticks() > 0)
	{
		SDL_JoystickClose(0);
	}

	if (screen && scale > 1)
	{
		SDL_FreeSurface(screen);
	}

	SDL_Quit();
}

void updateScale()
{
	if (screen && screen != screenScaled)
	{
		SDL_FreeSurface(screen);
	}

	//screenScaled = SDL_SetVideoMode(SCREEN_W * scale, SCREEN_H * scale, SCREEN_BPP, SDL_HWSURFACE | SDL_DOUBLEBUF);
	ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_SWSURFACE);
	screenScaled = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_W * scale, SCREEN_H * scale, SCREEN_BPP, 0, 0, 0, 0);
	screen = scale > 1 ? SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_W, SCREEN_H, SCREEN_BPP, 0, 0, 0, 0) : screenScaled;
}

SDL_Surface *loadImage(char *fileName)
{
	SDL_Surface *loadedImage;
	SDL_Surface *optimizedImage;
	Uint32 colorKey;

	if (!fileName)
	{
		fprintf(stderr, "ERROR: Filename is empty.");
		return NULL;
	}

	loadedImage = SDL_LoadBMP(fileName);

	if (!loadedImage)
	{
		fprintf(stderr, "ERROR: Failed to load image: %s\n", fileName);
		return NULL;
	}

	optimizedImage = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, loadedImage->w, loadedImage->h, SCREEN_BPP, 0, 0, 0, 0);
	SDL_BlitSurface(loadedImage, NULL, optimizedImage, NULL);
	SDL_FreeSurface(loadedImage);

	if (!optimizedImage)
	{
		fprintf(stderr, "ERROR: Failed to optimize image: %s\n", fileName);
		return NULL;
	}

	colorKey = SDL_MapRGB(optimizedImage->format, 255, 0, 255); // Set transparency to magenta.
	SDL_SetColorKey(optimizedImage, SDL_SRCCOLORKEY, colorKey);

	return optimizedImage;
}

void clipImage(SDL_Rect *source, int tileWidth, int tileHeight, int rowLength, int numOfTiles)
{
	int i;
	int j;
	int k;
	int l;

	for(i = 0, k = 0; k < numOfTiles; i+= tileHeight)
	{
		for(j = 0, l = 0; l < rowLength; j+= tileWidth)
		{
			if (k >= numOfTiles)
			{
				return;
			}
			source[k].x = j;
			source[k].y = i;
			source[k].w = tileWidth;
			source[k].h = tileHeight;
			++k;
			++l;
		}
		l = 0;
	}
}

void drawImage(SDL_Surface *source, SDL_Rect *clip, SDL_Surface *destination, int x, int y)
{
	SDL_Rect offset;

	offset.x = x;
	offset.y = y;

	SDL_BlitSurface(source, clip, destination, &offset);
}

void drawRectangle(SDL_Surface *source, int x, int y, int w, int h, int color)
{
	SDL_Rect r;
/*	int rcolor = SDL_MapRGB(screen->format, 0, 0, 255);*/

	// Top wall.
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = 1;
	SDL_FillRect(source, &r, color);

	// Bottom wall.
	r.x = x;
	r.y = y + h - 1;
	r.w = w;
	r.h = 1;
	SDL_FillRect(source, &r, color);

	// Left wall.
	r.x = x;
	r.y = y;
	r.w = 1;
	r.h = h;
	SDL_FillRect(source, &r, color);

	// Right wall.
	r.x = x + w - 1;
	r.y = y;
	r.w = 1;
	r.h = h;
	SDL_FillRect(source, &r, color);
}

int frameLimiter()
{
	int t;

#if defined(NO_FRAMELIMIT)
	return 0;
#endif

	curTicks = SDL_GetTicks();
	t = curTicks - lastTicks;

	if(t >= 1000/FPS)
	{
		lastTicks = curTicks;
		return 0;
	}

	SDL_Delay(1);
	return 1;
}

void clearScreen()
{
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
}
