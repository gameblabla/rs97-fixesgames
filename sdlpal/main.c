//
// Copyright (c) 2009, Wei Mingzhi <whistler_wmz@users.sf.net>.
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "main.h"
#include "getopt.h"

#ifdef PAL_WIN95
  #define BITMAPNUM_SPLASH_UP         3
  #define BITMAPNUM_SPLASH_DOWN       4
  #define SPRITENUM_SPLASH_TITLE      0x47
  #define SPRITENUM_SPLASH_CRANE      0x49
  #define NUM_RIX_TITLE               0x5
#else
  #define BITMAPNUM_SPLASH_UP         0x26
  #define BITMAPNUM_SPLASH_DOWN       0x27
  #define SPRITENUM_SPLASH_TITLE      0x47
  #define SPRITENUM_SPLASH_CRANE      0x49
  #define NUM_RIX_TITLE               0x5
#endif

static void PAL_Init(int width, int height, int fullscreen)
{
  int ret;

  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) == -1){
    TerminateOnError("Could not initialize SDL: %s.\n", SDL_GetError());
  }

  ret = VIDEO_Init(width, height, fullscreen);
  if(ret != 0){
    TerminateOnError("Could not initialize Video: %d.\n", ret);
  }
  SDL_WM_SetCaption("Loading...", NULL);

  ret = PAL_InitGlobals();
  if(ret != 0){
    TerminateOnError("Could not initialize global data: %d.\n", ret);
  }

  ret = PAL_InitFont();
  if(ret != 0){
    TerminateOnError("Could not load fonts: %d.\n", ret);
  }

  ret = PAL_InitUI();
  if(ret != 0){
    TerminateOnError("Could not initialize UI subsystem: %d.\n", ret);
  }

  ret = PAL_InitText();
  if(ret != 0){
    TerminateOnError("Could not initialize text subsystem: %d.\n", ret);
  }

  //PAL_InitInput();
  PAL_InitResources();
  SOUND_OpenAudio();
}

void PAL_Shutdown(void)
{
  SOUND_CloseAudio();
  PAL_FreeFont();
  PAL_FreeResources();
  PAL_FreeGlobals();
  PAL_FreeUI();
  PAL_FreeText();
  PAL_ShutdownInput();
  VIDEO_Shutdown();
  UTIL_CloseLog();
  SDL_Quit();
}

void PAL_TrademarkScreen(void)
{
  PAL_SetPalette(3, FALSE);
  PAL_RNGPlay(6, 0, 1000, 25);
  UTIL_Delay(1000);
  PAL_FadeOut(1);
}

void PAL_SplashScreen(void)
{
  int fUseCD = TRUE;
  int dwTime, dwBeginTime;
  int cranepos[9][3], i, iImgPos = 200, iCraneFrame = 0, iTitleHeight;
  SDL_Color *palette = PAL_GetPalette(1, FALSE);
  SDL_Color rgCurrentPalette[256];
  SDL_Surface *lpBitmapDown, *lpBitmapUp;
  SDL_Rect srcrect, dstrect;
  LPSPRITE lpSpriteCrane;
  LPBITMAPRLE lpBitmapTitle;
  LPBYTE buf, buf2;

  if(palette == NULL){
    printf("%s, palette is null\n", __func__);
    return;
  }

  buf = (LPBYTE)UTIL_calloc(1, 320 * 200 * 2);
  buf2 = (LPBYTE)(buf + 320 * 200);
  lpSpriteCrane = (LPSPRITE)buf2 + 32000;

  lpBitmapDown = SDL_CreateRGBSurface(gpScreen->flags, 320, 200, 8,
    gpScreen->format->Rmask, gpScreen->format->Gmask, gpScreen->format->Bmask,
    gpScreen->format->Amask);
  lpBitmapUp = SDL_CreateRGBSurface(gpScreen->flags, 320, 200, 8,
    gpScreen->format->Rmask, gpScreen->format->Gmask, gpScreen->format->Bmask,
    gpScreen->format->Amask);

  SDL_SetPalette(lpBitmapDown, SDL_LOGPAL | SDL_PHYSPAL, VIDEO_GetPalette(), 0, 256);
  SDL_SetPalette(lpBitmapUp, SDL_LOGPAL | SDL_PHYSPAL, VIDEO_GetPalette(), 0, 256);

  PAL_MKFReadChunk(buf, 320 * 200, BITMAPNUM_SPLASH_UP, gpGlobals->f.fpFBP);
  Decompress(buf, buf2, 320 * 200);
  PAL_FBPBlitToSurface(buf2, lpBitmapUp);
  PAL_MKFReadChunk(buf, 320 * 200, BITMAPNUM_SPLASH_DOWN, gpGlobals->f.fpFBP);
  Decompress(buf, buf2, 320 * 200);
  PAL_FBPBlitToSurface(buf2, lpBitmapDown);
  PAL_MKFReadChunk(buf, 32000, SPRITENUM_SPLASH_TITLE, gpGlobals->f.fpMGO);
  Decompress(buf, buf2, 32000);
  lpBitmapTitle = (LPBITMAPRLE)PAL_SpriteGetFrame(buf2, 0);
  PAL_MKFReadChunk(buf, 32000, SPRITENUM_SPLASH_CRANE, gpGlobals->f.fpMGO);
  Decompress(buf, lpSpriteCrane, 32000);

  iTitleHeight = PAL_RLEGetHeight(lpBitmapTitle);
  lpBitmapTitle[2] = 0;
  lpBitmapTitle[3] = 0; // HACKHACK

  for(i = 0; i < 9; i++){
    cranepos[i][0] = RandomLong(300, 600);
    cranepos[i][1] = RandomLong(0, 80);
    cranepos[i][2] = RandomLong(0, 8);
  }

  if(!SOUND_PlayCDA(7)){
    fUseCD = FALSE;
    PAL_PlayMUS(NUM_RIX_TITLE, TRUE, 2);
  }

  PAL_ProcessEvent();
  PAL_ClearKeyState();
  dwBeginTime = SDL_GetTicks();
  srcrect.x = 0;
  srcrect.w = 320;
  dstrect.x = 0;
  dstrect.w = 320;

  while(TRUE){
    PAL_ProcessEvent();
    dwTime = SDL_GetTicks() - dwBeginTime;

    if(dwTime < 15000){
      for(i=0; i<256; i++){
        rgCurrentPalette[i].r = (BYTE)(palette[i].r * ((float)dwTime/15000));
        rgCurrentPalette[i].g = (BYTE)(palette[i].g * ((float)dwTime/15000));
        rgCurrentPalette[i].b = (BYTE)(palette[i].b * ((float)dwTime/15000));
      }
    }

    VIDEO_SetPalette(rgCurrentPalette);
    SDL_SetPalette(lpBitmapDown, SDL_LOGPAL | SDL_PHYSPAL, VIDEO_GetPalette(), 0, 256);
    SDL_SetPalette(lpBitmapUp, SDL_LOGPAL | SDL_PHYSPAL, VIDEO_GetPalette(), 0, 256);

    if(iImgPos > 1){
      iImgPos--;
    }

    srcrect.y = iImgPos;
    srcrect.h = 200 - iImgPos;
    dstrect.y = 0;
    dstrect.h = srcrect.h;
    SDL_BlitSurface(lpBitmapUp, &srcrect, gpScreen, &dstrect);

      //
      // The lower part...
      //
      srcrect.y = 0;
      srcrect.h = iImgPos;

      dstrect.y = 200 - iImgPos;
      dstrect.h = srcrect.h;

      SDL_BlitSurface(lpBitmapDown, &srcrect, gpScreen, &dstrect);

      //
      // Draw the cranes...
      //
      for (i = 0; i < 9; i++)
      {
         LPCBITMAPRLE lpFrame = PAL_SpriteGetFrame(lpSpriteCrane,
            cranepos[i][2] = (cranepos[i][2] + (iCraneFrame & 1)) % 8);
         cranepos[i][1] += ((iImgPos > 1) && (iImgPos & 1)) ? 1 : 0;
         PAL_RLEBlitToSurface(lpFrame, gpScreen,
            PAL_XY(cranepos[i][0], cranepos[i][1]));
         cranepos[i][0]--;
      }
      iCraneFrame++;

      //
      // Draw the title...
      //
      if (PAL_RLEGetHeight(lpBitmapTitle) < iTitleHeight)
      {
         //
         // HACKHACK
         //
         WORD w = lpBitmapTitle[2] | (lpBitmapTitle[3] << 8);
         w++;
         lpBitmapTitle[2] = (w & 0xFF);
         lpBitmapTitle[3] = (w >> 8);
      }

      PAL_RLEBlitToSurface(lpBitmapTitle, gpScreen, PAL_XY(255, 10));
      VIDEO_UpdateScreen(NULL);

      //
      // Check for keypress...
      //
      if (g_InputState.dwKeyPress & (kKeyMenu | kKeySearch))
      {
         //
         // User has pressed a key...
         //
         lpBitmapTitle[2] = iTitleHeight & 0xFF;
         lpBitmapTitle[3] = iTitleHeight >> 8; // HACKHACK

         PAL_RLEBlitToSurface(lpBitmapTitle, gpScreen, PAL_XY(255, 10));

         VIDEO_UpdateScreen(NULL);

         if (dwTime < 15000)
         {
            //
            // If the picture has not completed fading in, complete the rest
            //
            while (dwTime < 15000)
            {
               for (i = 0; i < 256; i++)
               {
                  rgCurrentPalette[i].r = (BYTE)(palette[i].r * ((float)dwTime / 15000));
                  rgCurrentPalette[i].g = (BYTE)(palette[i].g * ((float)dwTime / 15000));
                  rgCurrentPalette[i].b = (BYTE)(palette[i].b * ((float)dwTime / 15000));
               }
               VIDEO_SetPalette(rgCurrentPalette);
#if SDL_VERSION_ATLEAST(2, 0, 0)
			   SDL_SetSurfacePalette(lpBitmapDown, gpScreen->format->palette);
			   SDL_SetSurfacePalette(lpBitmapUp, gpScreen->format->palette);
#else
			   SDL_SetPalette(lpBitmapDown, SDL_PHYSPAL | SDL_LOGPAL, VIDEO_GetPalette(), 0, 256);
			   SDL_SetPalette(lpBitmapUp, SDL_PHYSPAL | SDL_LOGPAL, VIDEO_GetPalette(), 0, 256);
#endif
               UTIL_Delay(8);
               dwTime += 250;
            }
            UTIL_Delay(500);
         }

         //
         // Quit the splash screen
         //
         break;
      }

      //
      // Delay a while...
      //
      PAL_ProcessEvent();
      while (SDL_GetTicks() - dwBeginTime < dwTime + 85)
      {
         SDL_Delay(1);
         PAL_ProcessEvent();
      }
   }

   SDL_FreeSurface(lpBitmapDown);
   SDL_FreeSurface(lpBitmapUp);
   free(buf);

   if (!fUseCD)
   {
      PAL_PlayMUS(0, FALSE, 1);
   }

   PAL_FadeOut(1);
}

int main(int argc, char *argv[])
{
  system("mkdir -p "PAL_SAVE_PREFIX);
  UTIL_OpenLog();

  PAL_Init(320, 480, 0);
  PAL_TrademarkScreen();
  PAL_SplashScreen();
  PAL_GameMain();
  assert(FALSE);
  return 0;
}

