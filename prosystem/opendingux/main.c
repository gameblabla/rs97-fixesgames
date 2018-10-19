/*  prosystem 
    Copyright (C) 2012 Alekmaul

    ProSystem Emulator (original version)
	Copyright(C) 2005 Greg Stanton (see http://home.comcast.net/~gscottstanton/) 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include  <sys/time.h>

#include "shared.h"

#include "../emu/Bios.h"
#include "../emu/ProSystem.h"
#include "../emu/Sound.h"
#include "../emu/Database.h"

unsigned int m_Flag;
unsigned int interval;

unsigned int gameCRC;
gamecfg GameConf;
char gameName[512];
char current_conf_app[512];

unsigned long lastTick = 0, newTick;
int FPS = 0, pastFPS = 0; 

unsigned char keyboard_data[19];
uint video_height;

SDL_Surface *video;
SDL_Surface *layer,*layerback,*layerbackgrey;

unsigned short bufVideo[320*312];
unsigned short atari_pal16[256] = {0};
unsigned int atari_pal32[256] = {0};

SDL_Event event;

#define tchepres(a) \
   keyboard_data[GameConf.Dingoo_Joy[a]] = 1;

unsigned long SDL_UXTimerRead(void) {
	struct timeval tval; // timing
  
  	gettimeofday(&tval, 0);
	return (((tval.tv_sec*1000000) + (tval.tv_usec )));
}

void SDL_ScaleSurface(void) {
#ifdef _VIDOD32_
	unsigned int *buffer_scr = video->pixels;
#else
	unsigned short *buffer_scr = video->pixels;
#endif
	unsigned int W,H,ix,iy,x,y;
	static char buffer[32];

	x=0;
	y=10<<16; // 26 (visible) - 16 (display)
	W=320;
	H=480;
	ix=(320<<16)/W;
	iy=(video_height<<16)/H;

	if(SDL_MUSTLOCK(video)) SDL_LockSurface(video);

	do   
	{
		unsigned short *buffer_mem=(bufVideo+((y>>16)*320));
		W=320; x=0;
		do {
#ifdef _VIDOD32_
			*buffer_scr++=atari_pal32[buffer_mem[x>>16]];
#else
			*buffer_scr++=atari_pal16[buffer_mem[x>>16]];
#endif
			x+=ix;
		} while (--W);
		y+= iy;
		//buffer_scr+= 320;
	} while (--H);

	if (GameConf.m_DisplayFPS) {
		pastFPS++;
		newTick = SDL_UXTimerRead();
		if ((newTick-lastTick)>1000000) {
			FPS = pastFPS;
			pastFPS = 0;
			lastTick = newTick;
			
		}
		sprintf(buffer,"%02d",FPS);
		print_string_video(300,1,buffer);
	}

	if (SDL_MUSTLOCK(video)) SDL_UnlockSurface(video);
	SDL_Flip(video);
}

void initSDL(void) 
{
#ifdef _VIDOD32_
	unsigned char vidbpp = 32;
#else
	unsigned char vidbpp = 16;
#endif

	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);
	
	video = SDL_SetVideoMode(320, 480, vidbpp, /*SDL_DOUBLEBUF | */SDL_HWSURFACE );
	if(video == NULL) {
		fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
		exit(1);
	}
	SDL_ShowCursor(SDL_DISABLE);

	// Init new layer to add background and text
	layer = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0,0,0,0);
	if(layer == NULL) {
		fprintf(stderr, "Couldn't create surface: %s\n", SDL_GetError());
		exit(1);
	}
	layerback = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0,0,0,0);
	if(layerback == NULL) {
		fprintf(stderr, "Couldn't create surface: %s\n", SDL_GetError());
		exit(1);
	}
	layerbackgrey = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0,0,0,0);
	if(layerbackgrey == NULL) {
		fprintf(stderr, "Couldn't create surface: %s\n", SDL_GetError());
		exit(1);
	}
}

int main(int argc, char *argv[]) {
	unsigned long nextTick, currentTick, wait;
	unsigned int index;

	// Get init file directory & name
	getcwd(current_conf_app, 512);
	sprintf(current_conf_app,"%s//prosystem.cfg",current_conf_app);
	
	initSDL();
	sound_InitSDL();
	
	m_Flag = GF_MAINUI;
	system_loadcfg(current_conf_app);
	GameConf.m_ScreenRatio = 1;

	SDL_WM_SetCaption("prosystem", NULL);

    //load rom file via args if a rom path is supplied
	if(argc > 1) {
		strcpy(gameName,argv[1]);
		m_Flag = GF_GAMEINIT;
	}

	while (m_Flag != GF_GAMEQUIT) {
		SDL_PollEvent(&event);
		unsigned char *keys = SDL_GetKeyState(NULL);
  
		switch (m_Flag) {
			case GF_MAINUI:
				SDL_PauseAudio(1);
				screen_showtopmenu();
				SDL_FillRect(video, 0, SDL_MapRGB(video->format, 0, 0, 0));

				if (cartridge_IsLoaded()) {
					SDL_PauseAudio(0);
					nextTick = SDL_UXTimerRead() + interval;
				}
				break;

			case GF_GAMEINIT:
				// Try to load bios
				if (bios_Load("./7800.rom")) {
					bios_enabled = true;
				}
				// load card game if ok
				if (cartridge_Load(gameName) != 0) {	
					// Initialize the virtual console emulation 
					database_Load(cartridge_digest);
					prosystem_Reset();
					// Init palette
					SDL_Color color;
					for(index = 0; index < 256; index++) {
						color.r = palette_data[(index * 3) + 0];
						color.g = palette_data[(index * 3) + 1];
						color.b = palette_data[(index * 3) + 2];
						atari_pal16[index] = PIX_TO_RGB(video->format, color.r, color.g, color.b);
						atari_pal32[index] = (color.r<<16) | (color.g<<8) | color.b;
					}
					m_Flag = GF_GAMERUNNING;

					// Init timing
					double period = 1.0 / (double) prosystem_frequency;
					period = period * 1000000;
					interval = (int) period;
					nextTick = SDL_UXTimerRead() + interval;
					
					SDL_PauseAudio(0);
				} 
				else {
					fprintf(stderr,"cant't load %s : %s",gameName,SDL_GetError());
					m_Flag = GF_GAMEQUIT;
				}
				break;
		
			case GF_GAMERUNNING:
				currentTick = SDL_UXTimerRead(); 
				wait = (nextTick - currentTick);
				if (wait > 0) {
					if (wait < 1000000) 
						usleep(wait);
				}
        
				prosystem_ExecuteFrame(keyboard_data);
				sound_Store();

				SDL_ScaleSurface();

				memset(keyboard_data,0,sizeof(keyboard_data));
				if (keys[SDLK_UP] == SDL_PRESSED) { tchepres(0); } // UP
				if (keys[SDLK_DOWN] == SDL_PRESSED) { tchepres(1); } // DOWN
				if (keys[SDLK_RIGHT] == SDL_PRESSED) { tchepres(3); } // RIGHT
				if (keys[SDLK_LEFT] == SDL_PRESSED) { tchepres(2); } // LEFT

				if (keys[SDLK_LCTRL] == SDL_PRESSED) { tchepres(4); }  // BUTTON #1
				if (keys[SDLK_LALT] == SDL_PRESSED) { tchepres(5); }  // BUTTON #2
				if (keys[SDLK_SPACE] == SDL_PRESSED) { tchepres(6); }  // KEYPAD #3
				if (keys[SDLK_LSHIFT] == SDL_PRESSED) { tchepres(7); }  // KEYPAD #4

				if (keys[SDLK_BACKSPACE] == SDL_PRESSED) { tchepres(8); } // KEYPAD ##
				if (keys[SDLK_TAB] == SDL_PRESSED) { tchepres(9); } // KEYPAD #*
			
				if ((keys[SDLK_ESCAPE] == SDL_PRESSED) && (keys[SDLK_RETURN] == SDL_PRESSED )) { 
					m_Flag = GF_MAINUI;
				}
				else if ( (keys[SDLK_RETURN] == SDL_PRESSED) )  { tchepres(10); }
				else if ( (keys[SDLK_ESCAPE] == SDL_PRESSED) ) { tchepres(11); }

				nextTick += interval;

				break;
		}
	}
    
	SDL_PauseAudio(1);
	prosystem_Close();
	
	// Free memory
	SDL_FreeSurface(layerbackgrey);
	SDL_FreeSurface(layerback);
	SDL_FreeSurface(layer);
	SDL_FreeSurface(video);
	
	SDL_QuitSubSystem(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
	
	exit(0);
}

#define DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);
// Table of CRC-32's of all single-byte values (made by make_crc_table)
unsigned int crc_table[256] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

unsigned long crc32 (unsigned int crc, const unsigned char *buf, unsigned int len) {
  if (buf == 0) return 0L;
  crc = crc ^ 0xffffffffL;
  while (len >= 8) {
    DO8(buf);
    len -= 8;
  }
  if (len) do {
    DO1(buf);
  } while (--len);
  return crc ^ 0xffffffffL;
}
