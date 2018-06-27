
// Includes.
#include <stdlib.h>
#include <unistd.h>
#include "SDL.h"
//#include "SDL_image.h"
#include <math.h>
#include <time.h>
#include <assert.h>

#include "ctypes.h"

#include "preca.h"
#include "breaker.h"
#include "frame.h"
#include "sprites.h"
#include "animspr.h"
#include "anims.h"
#include "dust.h"
#include "fire.h"
#include "mst.h"
#include "monsters.h"
#include "font.h"
#include "menu.h"
#include "SDL_mixer.h"
#include "SDL_sound.h"

#include "snd.h"

// Define.
#define SCR_Width 320
#define SCR_Height 240

#define	SHADOW_OfsX	4
#define	SHADOW_OfsY	4

#define	GFX_NbBkg	5
#define	MENU_NbBkg	2

#define	MOUSE_BtnLeft	1	// Masques binaires.
#define	MOUSE_BtnRight	2

#define	SPR_Palette_Idx	128	// 0 à x : Palette du décor / x à 256 : Palette des sprites.

int clavier_actif;

// Types de variables.
struct SGene
{
	SDL_Surface *pScreen;	// Ptr sur le buffer écran.
	SDL_Surface *hwscreen;
	SDL_Surface *pLevel;	// Ptr sur l'image de fond d'un level.
	SDL_Surface *pLev[GFX_NbBkg];	// Les images de fond.
	SDL_Surface *pBackground;		// Ptr sur l'images de fond des menus.
	SDL_Rect	*pBkgRect;			// Ptr sur le rect pour déplacer le blit.
	SDL_Rect	sBkgRect;			// Rect pour déplacer le blit.
	SDL_Surface *pBkg[MENU_NbBkg];	// Les images de fond.

	u8	*pKeys;		// Buffer clavier.

	u8	nScreenMode;	// 0 = Windowed / 1 = Full screen.
	SDL_Color pColors[256];	// Palette générale, à réinitialiser au changement de mode.
	SDL_Color pSprColors[256 - SPR_Palette_Idx];	// Palette des sprites.

	s16	pSinCos[256 + 64];	// Table contenant les sin et cos * 256, sur 256 angles.
	s16	*pSin;			// Ptrs sur les tables.
	s16	*pCos;

	s32	nMousePosX, nMousePosY;	// Position de la souris.
	u8	nMouseButtons;		// Boutons de la souris
	
	s32	nFadeVal;		// 0 = Noir / 256 = Couleurs finales.

};

// Structure d'échange entre les différents modules.
struct SExg
{
	u32	nExitCode;	// Pour sortie du jeu. Tjs à 0, sauf pour sortie.
	u32	nLevel;		// Level atteint au game over.
	u32	nScore;		// Score au game over.


};

// Variables générales.
extern struct SGene gVar;
extern struct SExg gExg;


