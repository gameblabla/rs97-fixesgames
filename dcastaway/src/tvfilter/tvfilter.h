#ifndef __TVFILTER__
#define __TVFILTER__

#include <stdio.h>
#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NO_USE_TV_FILTER
void TV_Quit(void);
void TV_SetFullScreen(SDL_Surface *, SDL_bool);
void TV_ToggleFullScreen(SDL_Surface *);
void TV_SetFilter(SDL_bool);
void TV_ToggleFilter(void);
void TV_SetDistorsioni(SDL_bool);
void TV_ToggleDistorsion(void);
void TV_SetScanlines(SDL_bool);
void TV_ToggleScanlines(void);
void TV_SetTV(SDL_bool);
void TV_ToggleTV(void);
void TV_ResizeWindow(int w, int h);
void TV_Flip(SDL_Surface *);
int TV_ConvertMousePosX(int x);
int TV_ConvertMousePosY(int y);
SDL_Surface *TV_Init(int w, int h, int bpp, int flags);
#else
#define TV_Quit() 
//#define TV_ToggleFullScreen() SDL_WM_ToggleFullScreen(screen)
#define TV_ToggleFullScreen(SCR) (SCR)=SDL_SetVideoMode((SCR)->w,(SCR)->h,(SCR)->format->BitsPerPixel,(SCR)->flags^SDL_FULLSCREEN)
#define TV_SetFullScreen(SCR,B) if (!((SCR)->flags&SDL_FULLSCREEN)) TV_ToggleFullScreen((SCR))
#define TV_SetFilter(B)
#define TV_ToggleFilter() 
#define TV_SetDistorsion(B)
#define TV_ToggleDistorsion() 
#define TV_SetScanlines(B)
#define TV_ToggleScanlines() 
#define TV_ResizeWindow(W, H)
#define TV_Flip(SF) SDL_Flip(SF)
#define TV_Init(W,H,BPP,FLAGS) SDL_SetVideoMode(W,H,BPP,FLAGS)
#define TV_ConvertMousePosX(X) (X)
#define TV_ConvertMousePosY(Y) (Y)
#endif

#ifdef __cplusplus
}
#endif

#endif
