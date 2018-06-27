#include <SDL.h>

#include "touche.h"

/* voir le .h*/

/*
 Ce module ne fournit en réalité qu'une seule fonction, la fonction
 ToucheEstAppuyer(la touche au format T_a ou T_UP)
 
 Beaucoup plus pratique et simple d'utilisation que les autres modules de touches
 */

static Tableau_touche* touche_tableauInitialiser ();
static int             Evenement_toucheRecuperer (Evenement_touche* event);


Tableau_touche* touche_tableauInitialiser ()
{
    return SDL_GetKeyState(NULL);        
}

int             Evenement_toucheRecuperer (Evenement_touche* event)
{
  return SDL_PollEvent(event);
}

int             toucheEstAppuyer(Touche touche)
{
 /* static car il ne faut pas perdre les données dans event*/
  static Evenement_touche       event;
  Tableau_touche*               keyarray;
  
  keyarray = touche_tableauInitialiser();
  Evenement_toucheRecuperer(&event);
  return keyarray[touche];                               
}
