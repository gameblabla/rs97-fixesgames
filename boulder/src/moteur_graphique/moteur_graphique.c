#include <stdlib.h>
#include <stdio.h>
#include <SDL_mixer.h>

#include "moteur_graphique.h"

/* plus d'informations dans le fichier .h*/

SDL_Surface *ScreenSurface;
G_couleur g_couleurCreer(G_surface* surface, int R, int G, int B)
{
   return SDL_MapRGB(surface->format,R,G,B);
}

void g_rectangleInitialiser(G_rectangle* rect, int x, int y, int w, int h)
{
 if (rect == NULL)
 {
  fprintf(stderr, "\n* Pointeur sur rectangle NULL dans l'appel g_rectangleInitialiser\n"); 
  return;         
 }
 rect->x=x; 
 rect->y=y;
 rect->w=w;
 rect->h=h; 
}


int g_fenetreInitialiser(G_fenetre* pfenetre, int resolutionx, int resolutiony,  G_fenetre_mode mode)
{
    G_surface * screen;
    if (pfenetre==NULL)
    {
       fprintf(stderr, "\n* g_fenetre pointant sur NULL dans l'appel de g_fenetreInitialiser\n");  
       return EXIT_FAILURE;              
    }
    
    SDL_Init(SDL_INIT_VIDEO);	
    pfenetre->resolutionx = resolutionx;
    pfenetre->resolutiony = resolutiony;
    pfenetre->mode = mode;	// preapare SDL
    if(pfenetre->mode == PLEIN_ECRAN){
       //screen = SDL_SetVideoMode(pfenetre->resolutionx,pfenetre->resolutiony,  16,SDL_SWSURFACE);
       ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
       screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0, 0, 0, 0);
    }
    else{
       //screen = SDL_SetVideoMode(pfenetre->resolutionx,pfenetre->resolutiony,  16,SDL_SWSURFACE); 
       ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_HWSURFACE);
       screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0, 0, 0, 0);
    }
    SDL_ShowCursor(0);
    if(screen == NULL){   
      fprintf(stderr, "\n* Impossible d'allouer une g_surface dans g_fenetreInitialiser\n"); 
      return EXIT_FAILURE;
    }
    
    Mix_OpenAudio(48000,MIX_DEFAULT_FORMAT,2,2048);
    Mix_AllocateChannels(8);
    
       // eneleve la souris
    pfenetre->surface = screen;
    

    
    return EXIT_SUCCESS;  
    
}


G_surface* g_fenetreLireSurface(G_fenetre* pfenetre)
{
  return pfenetre->surface;
}



void g_surfaceEcrireCouleur(G_surface* surface, int x, int y, G_couleur pixel)
{
     int bpp = surface->format->BytesPerPixel; 
    /* Here p is the address to the pixel we want to set */ 
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp; 
  
  
    switch(bpp) { 
    case 1: 
        *p = pixel; 
        break; 
  
    case 2: 
        *(Uint16 *)p = pixel; 
        break; 
  
    case 3: 
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) { 
            p[0] = (pixel >> 16) & 0xff; 
            p[1] = (pixel >> 8) & 0xff; 
            p[2] = pixel & 0xff; 
        } else { 
            p[0] = pixel & 0xff; 
            p[1] = (pixel >> 8) & 0xff; 
            p[2] = (pixel >> 16) & 0xff; 
        } 
        break; 
  
    case 4: 
        *(Uint32 *)p = pixel; 
        break; 
    }    
}

G_surface* g_surfaceCreer(int x, int y)
{
   return SDL_CreateRGBSurface(SDL_SWSURFACE, x,y,16,0,0,0,0);
}

void g_surfaceDetruire(G_surface* surface)
{
  SDL_FreeSurface(surface);     
}

void g_surfaceCopie(G_surface* surface1, G_rectangle* rect1, G_surface* surface2, G_rectangle* rect2)
{
  SDL_BlitSurface(surface1, rect1, surface2 , rect2);    
}

void g_surfaceEfface(G_surface* s)
{
 G_rectangle rect;
 g_rectangleInitialiser(&rect, 0,0, g_surfaceLongueurx(s), g_surfaceLongueury(s));
 
 SDL_FillRect(s,&rect, 0);     
     
}


int g_surfaceLongueurx(G_surface* s)
{
    return s->w;
}
int g_surfaceLongueury(G_surface* s)
{
    return s->h;
}

G_surface* g_surfaceChargerBMP(char* c)
{
   /*image charger en RAM*/
   G_surface* f = SDL_LoadBMP(c);	
   
   G_surface* surface= g_surfaceCreer( g_surfaceLongueurx(f), g_surfaceLongueury(f));  
   G_rectangle R;
   g_rectangleInitialiser(&R, 0,0,g_surfaceLongueurx(f),g_surfaceLongueury(f));
   /*copie de RAM vers VRAM*/
   g_surfaceCopie(f,NULL,surface,&R);	
   g_surfaceDetruire(f);     
  
   return surface;       
}

void       g_fenetreAfficher(G_fenetre* fenetre)
{
  G_rectangle rect;
  g_rectangleInitialiser(&rect, 0,0,fenetre->resolutionx, fenetre->resolutiony);
  
  //SDL_Flip(g_fenetreLireSurface(fenetre));
/*  SDL_FillRect(g_fenetreLireSurface(fenetre), &rect, 0);*/
            {
              if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
              int x, y;
              uint32_t *s = (uint32_t*)g_fenetreLireSurface(fenetre)->pixels;
              uint32_t *d = (uint32_t*)ScreenSurface->pixels;
              for(y=0; y<240; y++){
                for(x=0; x<160; x++){
                  *d++ = *s++;
                }
                d+= 160;
              }
              if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
              SDL_Flip(ScreenSurface);
            }

}

void       g_fenetreDetruire(G_fenetre* fenetre)
{
#ifndef SANS_TTF
 static int b = 0;
 if (b==0) 
 {
  TTF_Quit();
  b =1;
 }
#endif
 SDL_ShowCursor(1);
 g_surfaceDetruire(g_fenetreLireSurface(fenetre));
 Mix_CloseAudio();
 SDL_Quit();          
}



/*********************
 Font
 *********************/
 
 

G_font*   g_fontCharger(char* fichier, int taille)
{

taille = 12;

#ifndef SANS_TTF
static int b = 0;

 G_font* f;
 if (b==0)
 { 
   if (TTF_Init()<0) {
    fprintf(stderr, "\n**Probleme initialisation de TTF_init()\n");
    return NULL;
   }
   b=1;
 }   
    
    f = TTF_OpenFont(fichier, taille);    
    if (f==NULL)
      fprintf(stderr, "\n* Probleme de chargement du font dans l'appel g_fontCharger\n"); 
    return f;     
#else
    return (G_font*) NULL;
#endif    
}

void g_fontDetruire(G_font* font)
{
#ifndef SANS_TTF
     TTF_CloseFont(font);
#endif

}


G_surface*      g_surfaceEcrireTexte(G_font* font, char* texte, G_couleur couleur_texte, G_couleur couleur_fond)
{
#ifndef SANS_TTF
  SDL_Color t = {255,255,255};
  SDL_Color f = {0,0,0};
  if (font == NULL)
      fprintf(stderr, "\n* font absent dans l'appel de g_surfaceEcrireTexte\n");
               
   return  TTF_RenderText_Shaded(font, texte, t, f);
#else
     return (G_surface*) NULL;
#endif
}
   

