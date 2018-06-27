#include "temps.h"


/* voir le .h*/

Temps tempsRecuperer()
{
  return (Temps) SDL_GetTicks();
}

short tempsInferieur(Temps a, Temps b)
{
  if (a<b)
    return 1;
  else
    return 0;
  return 0;
}

Temps tempsSomme(Temps a, Temps b)
{ 
  return (a+b);
}

void  tempsAttendre(Temps a)
{
      SDL_Delay (a);
}
