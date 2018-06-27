#include <SDL/SDL.h>

#include "touche.h"




void oki () { fprintf(stderr, "= TEST OK\n"); }

int main (int nb, char ** b)
{
  SDL_Surface  *  screen;	
  screen = SDL_SetVideoMode(10L, 10, 32,SDL_HWSURFACE|SDL_DOUBLEBUF);
   
  fprintf(stderr, "====TEST DU MODULE TOUCHE====\n");
  
  fprintf(stderr, "* Appuyer sur HAUT ");
  while (toucheEstAppuyer(SDLK_UP)==0);
  oki ();
 
  fprintf(stderr, "* Appuyer sur a ");
  while (toucheEstAppuyer(SDLK_a)==0);
  oki ();  
  
  fprintf(stderr, "* Appuyer sur espace ");
  while (toucheEstAppuyer(SDLK_SPACE)==0);
  oki (); 
    
  fprintf(stderr, "* Appuyer sur gauche ");
  while (toucheEstAppuyer(SDLK_LEFT)==0);
  oki ();  
    
   SDL_Quit(); 
    
 return 0;   
}
