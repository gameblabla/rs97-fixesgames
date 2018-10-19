#ifndef SHARED_H
#define SHARED_H

#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <SDL/SDL.h>

#define SYSVID_WIDTH  320
#define SYSVID_HEIGHT 223

#define GF_GAMEINIT    1
#define GF_MAINUI      2
#define GF_GAMEQUIT    3
#define GF_GAMERUNNING 4

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define PIX_TO_RGB(fmt, r, g, b) (((r>>fmt->Rloss)<<fmt->Rshift)| ((g>>fmt->Gloss)<<fmt->Gshift)|((b>>fmt->Bloss)<<fmt->Bshift))

typedef struct {
  unsigned int sndLevel;
  unsigned int m_ScreenRatio; // 0 = original show, 1 = full screen
  unsigned int Dingoo_Joy[12]; // each key mapping
  unsigned int m_DisplayFPS;
  char current_dir_rom[512];
} gamecfg;

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int uint;

extern SDL_Surface *video;
extern SDL_Surface *layer,*layerback,*layerbackgrey;

extern SDL_Event event;

extern unsigned short bufVideo[320*312];
extern gamecfg GameConf;
extern char gameName[512];
extern char current_conf_app[512];
extern unsigned int gameCRC;
extern uint video_height;
extern int FPS , pastFPS; 

extern void system_loadcfg(char *cfg_name);
extern void system_savecfg(char *cfg_name);

#endif
