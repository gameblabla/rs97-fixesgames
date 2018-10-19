#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#include "piece.h"
#include "font.h"

SDL_Surface *video;
SDL_Surface *ScreenSurface;

unsigned char *vBuffer = NULL;

SDL_Color pltTbl[20] = {
	{255,255,255},  // Normal colors
	{168,168,168},
	{96, 96, 96},
	{0,  0,  0},

	{28*8,30*8, 9*8},  // Spout colors
	{30*8,25*8, 9*8},
	{30*8,17*8, 9*8},
	{20*8, 0, 0},

	{12*8,12*8,12*8}, // Wall color #1
	{21*8,21*8,0}, // Wall color #2
	{12*8,12*8,0}, // Wall color #3
	{255,  0,255}, // Wall color #4
	{168,  0,168}, // Wall color #5
	{96,  0, 96}, // Wall color #6
	{0,255,255}, // Wall color #7
	{0,168,168}, // Wall color #8
	{0, 96, 96}, // Wall color #9
};


void pceLCDDispStop()
{
}

void pceLCDDispStart()
{
}

void initSDL() {
	SDL_PixelFormat *pfrm;

	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	//video = SDL_SetVideoMode(320, 240, 16, SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE);
	ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
	video = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0, 0, 0, 0);
	if(video == NULL) {
		fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
		exit(1);
	}
	SDL_ShowCursor(SDL_DISABLE);

	{
		SDL_SetColors(video, pltTbl, 0, 20);
	}
}

void SDL_ScaleSurface(unsigned int Width, unsigned int Height) {
  unsigned short *buffer_scr = video->pixels;
  unsigned int W,H,ix,iy,x,y;
  x=0;
  y=0;
  W=320;
  H=240;
  ix=(128<<16)/W;
  iy=(88<<16)/H;

  do   
  {
    unsigned char *buffer_mem=vBuffer+((y>>16)*128);
    W=320; x=0;
    do {
	  SDL_Color c = pltTbl[buffer_mem[x>>16]];
	  *buffer_scr++=PIX_TO_RGB(video->format,c.r, c.g, c.b);
      x+=ix;
    } while (--W);
    y+=iy;
  } while (--H);
}

void pceLCDTrans() {
  if(SDL_MUSTLOCK(video)) SDL_LockSurface(video);
	SDL_ScaleSurface(320,240);
  if (SDL_MUSTLOCK(video)) SDL_UnlockSurface(video);
	//SDL_Flip(video);

	
	if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
	int x, y;
	uint32_t *s = video->pixels;
	uint32_t *d = ScreenSurface->pixels;
	for(uint8_t y = 0; y < 240; y++, s += 160, d += 320) 
		memmove(d, s, 1280); // double-line fix by pingflood, 2018
	/*for(y=0; y<240; y++){
		for(x=0; x<160; x++){
			*d++ = *s++;
		}
		d+= 160;
	}*/
	if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
	SDL_Flip(ScreenSurface);
}

unsigned char *keys;

int pcePadGet() {
	static int pad = 0;
	int i = 0, op = pad & 0x00ff;

	int k[] = {
		SDLK_UP,		SDLK_DOWN,		SDLK_LEFT,		SDLK_RIGHT,
		SDLK_UP,		SDLK_DOWN,		SDLK_LEFT,		SDLK_RIGHT,
#ifdef OPENDINGUX
		SDLK_LCTRL,			SDLK_LALT,			SDLK_LCTRL,			SDLK_LALT,
		SDLK_ESCAPE,			SDLK_RETURN,			SDLK_RETURN, 51
#else		
		SDLK_a,			SDLK_b,			SDLK_a,			SDLK_b,
		SDLK_x,			SDLK_y,			SDLK_y
#endif
// ALEK		SDLK_KP8,		SDLK_KP2,		SDLK_KP4,		SDLK_KP6,
// ALEK		SDLK_x,			SDLK_z,			SDLK_SPACE,		SDLK_RETURN,
// ALEK		SDLK_ESCAPE,	SDLK_LSHIFT,	SDLK_RSHIFT
	};

	int p[] = {
		PAD_UP,			PAD_DN,			PAD_LF,			PAD_RI,
		PAD_UP,			PAD_DN,			PAD_LF,			PAD_RI,
		PAD_A,			PAD_B,			PAD_A,			PAD_B,
		PAD_C,			PAD_D,			PAD_D,      PAD_D,
		-1
	};

	pad = 0;

	do {
		if(keys[k[i]] == SDL_PRESSED) {
			pad |= p[i];
		}
		i ++;
	} while(p[i] >= 0);

	pad |= (pad & (~op)) << 8;

// ALEK
	if (keys[SDLK_RETURN] == SDL_PRESSED) 
		pad |= TRG_ST;
// ALEK
		
	return pad;
}

int interval = 0;

void pceAppSetProcPeriod(int period) {
	interval = period;
}

int exec = 1;

void pceAppReqExit(int c) {
	exec = 0;
}

unsigned char *pceLCDSetBuffer(unsigned char *pbuff)
{
	if(pbuff) {
		vBuffer = pbuff;
	}
	return vBuffer;
}

int font_posX = 0, font_posY = 0, font_width = 4, font_height = 6;
unsigned char font_fgcolor = 3, font_bgcolor = 0, font_bgclear = 0;
const char *font_adr = FONT6;

void pceFontSetType(int type)
{
	const int width[] = {5, 8, 4};
	const int height[] = {10, 16, 6};
	const char* adr[] ={FONT6, FONT16, FONT6};

	type &= 3;
	font_width = width[type];
	font_height = height[type];
	font_adr = adr[type];
}

void pceFontSetTxColor(int color)
{
	font_fgcolor = (unsigned char)color;
}

void pceFontSetBkColor(int color)
{
	if(color >= 0) {
		font_bgcolor = (unsigned char)color;
		font_bgclear = 0;
	} else {
		font_bgclear = 1;
	}
}

void pceFontSetPos(int x, int y)
{
	font_posX = x;
	font_posY = y;
}

int pceFontPrintf(const char *fmt, ...)
{
	unsigned char *adr = vBuffer + font_posX + font_posY * 128;
	unsigned char *pC;
	char c[1024];
	va_list argp;

	va_start(argp, fmt);
	vsprintf(c, fmt, argp);
	va_end(argp);

	pC = c;
	while(*pC) {
		int i, x, y;
		const unsigned char *sAdr;
		if(*pC >= 0x20 && *pC < 0x80) {
			i = *pC - 0x20;
		} else {
			i = 0;
		}
		sAdr = font_adr + (i & 15) + (i >> 4) * 16 * 16;
		for(y = 0; y < font_height; y ++) {
			unsigned char c = *sAdr;
			for(x = 0; x < font_width; x ++) {
				if(c & 0x80) {
					*adr = font_fgcolor;
				} else if(font_bgclear == 0) {
					*adr = font_bgcolor;
				}
				adr ++;
				c <<= 1;
			}
			adr += 128 - font_width;
			sAdr += 16;
		}
		adr -= 128 * font_height - font_width;
		pC ++;
	}
}

#ifndef O_BINARY 
#define O_BINARY 0
#endif

int pceFileOpen(FILEACC *pfa, const char *fname, int mode)
{
	if(mode == FOMD_RD) {
		*pfa = open(fname, O_RDONLY | O_BINARY);
	} else if(mode == FOMD_WR) {
		*pfa = open(fname, O_CREAT | O_RDWR | O_BINARY | O_TRUNC, S_IREAD | S_IWRITE);
	}

	if(*pfa >= 0) {
		return 0;
	} else {
		return 1;
	}
}

int pceFileReadSct(FILEACC *pfa, void *ptr, int sct, int len)
{
	return read(*pfa, ptr, len);
}

int pceFileWriteSct(FILEACC *pfa, const void *ptr, int sct, int len)
{
	return write(*pfa, ptr, len);
}

int pceFileClose(FILEACC *pfa)
{
	close(*pfa);
	return 0;
}

int main(int argc, char *argv[])
{
	SDL_Event event;
	long nextTick, wait;
	int cnt = 0;

	initSDL();
	pceAppInit();

	SDL_WM_SetCaption("spout", NULL);

	nextTick = SDL_GetTicks() + interval;
	while(exec) {
		SDL_PollEvent(&event);
		keys = SDL_GetKeyState(NULL);

		wait = nextTick - SDL_GetTicks();
		if(wait > 0) {
			SDL_Delay(wait);
		}

		pceAppProc(cnt);

		nextTick += interval;
		cnt ++;

/*		if((keys[SDLK_ESCAPE] == SDL_PRESSED && (keys[SDLK_RETURN] == SDL_PRESSED )) || event.type == SDL_QUIT) {*/
/*			exec = 0;*/
/*		}*/
	}

	pceAppExit();

	exit(0);
}

