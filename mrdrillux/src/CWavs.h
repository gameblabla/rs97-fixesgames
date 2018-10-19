#ifndef CWavs_h
#define CWavs_h

#include "SDL.h"
#include "SDL_mixer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//サウンドデータ
typedef struct{
	int index;
	Mix_Chunk **wav;
	int nums;
	int musicchannel;
}CWavs;
CWavs* CWavsInit(int);
int CWavsFree(CWavs *);
int CWavsLoad(CWavs*,char *);
int CWavsLoadWzNum(CWavs*,char *,int);
int CAudioInitDefault(void);
int CAudioInit(int,int,int,int);
int CAudioClose(void);

int CWavsLoadFromFileWithDir(CWavs *,char *,char *);
#define CWavsLoadFromFile(A,B)	CWavsLoadFromFileWithDir(A,B,"./")

void CWavsPlay(CWavs *,int);
void CWavsPlayMusicStyle(CWavs *,int);

#endif
