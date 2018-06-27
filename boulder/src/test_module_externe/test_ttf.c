#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>



int main ()
{
  SDL_Surface* screen;
  TTF_Font* font;
  SDL_Surface* texte;
  SDL_Color t = {255,255,255};
  SDL_Color f = {0,0,0};

SDL_Init(SDL_INIT_VIDEO);	
       screen = SDL_SetVideoMode(788,500,  32,SDL_HWSURFACE|SDL_DOUBLEBUF); 
   TTF_Init(); 
      font = TTF_OpenFont("./times.ttf", 12);
       if (font==NULL) 
	 fprintf(stderr, "PRobleme à l con");
     texte=  TTF_RenderText_Shaded(font, "Test SDL_ttf", t, f);

        SDL_BlitSurface(texte, NULL, screen, NULL);
       SDL_Flip(screen);
       sleep (2);


       SDL_FreeSurface(texte);
SDL_Quit(); 

  return 0;
}
