#ifndef CBmps_h_included
#define CBmps_h_included
/*********************************

SDL initializer and bitmap loader

CBmps structure and handling funcs

2000/6/4

*********************************/


/********************
インクルードファイル
********************/

#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/********************
	定数定義
********************/

//ファイル名の最大長(bytes)
#define MAX_FILENAME 255+1
#define MAX_RECT 64

/********************
	構造体
********************/



//画像データ
typedef struct{
	int index;
	SDL_Surface **bmp;
	int nums;
}CBmps;


CBmps* CBmpsInit(int);
int CBmpsFree(CBmps *);
int CBmpsLoad(CBmps *, char *);
int CBmpsLoadWzNum(CBmps* ,char *,int);
int CBmpsConvert(CBmps* );
int CBmpsSetTransparent(CBmps* ,int,int,int);
int CBmpsLoadFromFileWithDir(CBmps *,char *,char *);
#define CBmpsLoadFromFile(A,B)	CBmpsLoadFromFileWithDir(A,B,"./");
SDL_Surface* CScreenInitDefault(void);
SDL_Surface* CScreenInitDefaultHW(void);
SDL_Surface* CScreenInit(int,int,int,int);
int CBmpsBlit(CBmps *,SDL_Surface *,int,int,int);


#endif
