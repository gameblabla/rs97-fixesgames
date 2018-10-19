#include "video.h"
#include "backend.h"
#include "scaler.h"

SDL_Surface *ScreenSurface;
SDL_Surface *screen;
SDL_Surface *screenScaled;
int screenScale = 1;
int fullscreen;
int showFps;
unsigned int fps;
int debugSlowMotion;

int videoInit()
{
	updateScale();

	if (!screen)
		return -1;

	return 0;
}

void videoClean()
{
		if (screenScale > 1)
			SDL_FreeSurface(screen);
}

void updateScale()
{
	if (screen != screenScaled)
		SDL_FreeSurface(screen);

	//screenScaled = SDL_SetVideoMode(SCREEN_W * screenScale, SCREEN_H * screenScale, SCREEN_BPP, SDL_HWSURFACE | SDL_DOUBLEBUF | (fullscreen ? SDL_FULLSCREEN : 0));
	ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
	screenScaled = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_W * screenScale, SCREEN_H * screenScale, SCREEN_BPP, 0,0,0,0);
	screen = screenScale > 1 ? SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_W, SCREEN_H, SCREEN_BPP, 0, 0, 0, 0) : screenScaled;
}

void frameCounter()
{
	static unsigned int frames;
	static Uint32 curTicks;
	static Uint32 lastTicks;
	Uint32 t;

	curTicks = SDL_GetTicks();
	t = curTicks - lastTicks;

	if (t >= 1000)
	{
		lastTicks = curTicks;
		fps = frames;
		frames = 0;
	}

	++frames;
}

int frameLimiter()
{
	static Uint32 curTicks;
	static Uint32 lastTicks;
	float t;

#if NO_FRAMELIMIT
	return 0;
#endif

	curTicks = SDL_GetTicks();
	t = curTicks - lastTicks;

	if (t >= 1000.0/FPS)
	{
		lastTicks = curTicks;
		return 0;
	}


	SDL_Delay(1);

	return 1;
}

void flipScreen()
{
	if (SDL_MUSTLOCK(screenScaled))
		SDL_LockSurface(screenScaled);

	switch (screenScale)
	{
		case 2:
			upscale2(screenScaled->pixels, screen->pixels);
		break;
		case 3:
			upscale3(screenScaled->pixels, screen->pixels);
		break;
		case 4:
			upscale4(screenScaled->pixels, screen->pixels);
		break;

		default:
		break;
	}

	if (SDL_MUSTLOCK(screenScaled))
		SDL_UnlockSurface(screenScaled);

	//SDL_Flip(screenScaled);
	{
		if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
		int x, y;
		uint32_t tmp;
		uint32_t *s = screenScaled->pixels;
		uint16_t *d = ScreenSurface->pixels;
		for(uint8_t y2 = 0; y2 < 240; y2++, s += 160, d += 320) 
			memmove(d, s, 1280); // double-line fix by pingflood, 2018
		/*for(y=0; y<240; y++){
			for(x=0; x<320; x++){
				tmp = *s++;
				*d++ = SDL_MapRGB(ScreenSurface->format, (tmp >> 16) & 0xff, (tmp >> 8) & 0xff, tmp & 0xff);
			}
			d+= 320;
		}*/
		if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
		SDL_Flip(ScreenSurface);
	}
	if (debugSlowMotion)
		SDL_Delay(250);
}

void clearScreen()
{
	SDL_FillRect(screen, NULL, 0);
}

Image *loadImage(const char *fileName)
{
	SDL_Surface *loadedImage = NULL;
	SDL_Surface *optimizedImage = NULL;
	Uint32 colorKey;

	if (!fileName)
		goto error;

	loadedImage = SDL_LoadBMP(fileName);

	if (!loadedImage)
		goto error;

	optimizedImage = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF | (fullscreen ? SDL_FULLSCREEN : 0), loadedImage->w, loadedImage->h, SCREEN_BPP, 0, 0, 0, 0);

	if (!optimizedImage)
		goto error;

	SDL_BlitSurface(loadedImage, NULL, optimizedImage, NULL);
	SDL_FreeSurface(loadedImage);

	if (!optimizedImage)
		goto error;

	colorKey = SDL_MapRGB(optimizedImage->format, 255, 0, 255); /* Set transparency to magenta. */
	SDL_SetColorKey(optimizedImage, SDL_SRCCOLORKEY, colorKey);

	return optimizedImage;

	error:
		SDL_FreeSurface(loadedImage);
		fprintf(stderr, "Failed to load file \"%s\".\n", fileName);
		return NULL;
}

void unloadImage(Image *image)
{
	SDL_FreeSurface(image);
}

void drawImage(Image *image, Rect *clip, int x, int y)
{
	SDL_Rect c;
	SDL_Rect r;

	if (!image)
		return;

	if (clip)
	{
		c.x = clip->x;
		c.y = clip->y;
		c.w = clip->w;
		c.h = clip->h;
	}

	r.x = x;
	r.y = y;

	SDL_BlitSurface(image, clip ? &c : NULL, screen, &r);
}

void drawDashedLine(int x, int y, unsigned int line, unsigned int length)
{
	SDL_Rect r;

	r.w = line;
	r.h = 1;
	r.x = x - r.w*2;
	r.y = y;

	while (r.x += r.w*2, r.x + r.w <= x + (int)length)
	{
		SDL_FillRect(screen, &r, 0xffffffff);
	}
}
