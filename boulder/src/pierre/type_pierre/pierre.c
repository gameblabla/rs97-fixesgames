#include <stdio.h>

#include "pierre.h"
#include <malloc.h>

/* voir diamant, même principe*/

Pierre* pierreCreer ()
{
  Pierre* pierre= malloc(sizeof(Pierre));
  if (pierre == NULL) {
    fprintf(stderr, "Probleme allocation espace pour pierre");
    return NULL;
  }
  pierreEcrireType(pierre, PIERRE_TYPE_PIERRE);
  pierreEcrireDirection(pierre, PIERRE_DIRECTION_BOUGE_PAS);
  return pierre;   
}

void pierreCopie(Pierre* pierre, int i, int j, Pierre_type chute)
{
     
  pierreEcrireX(pierre, i);
  pierreEcrireY(pierre, j);
  pierreEcrireType(pierre, chute);
}

void pierreDetruire(Pierre* pierre)
{
  free(pierre);
}



int pierreLireX (Pierre* pierre)
{
  return pierre->x;
}

int pierreLireY (Pierre* pierre)
{
  return pierre->y;
}

void pierreEcrireX(Pierre* pierre, int x)
{
  pierre->x = x;
}

void pierreEcrireY(Pierre* pierre, int y)
{
   pierre->y = y;
}

Pierre_direction pierreLireDirection(Pierre* pierre)
{
  return (pierre->direction);
}

void pierreEcrireDirection(Pierre* pierre, Pierre_direction direction)
{
  pierre->direction = direction;     
}

Pierre_type pierreLireType (Pierre* pierre)
{
   return pierre->type;                            
}

void pierreEcrireType(Pierre* pierre, Pierre_type transformation)
{
  pierre->type = transformation;     
}

int pierreLirePoids (Pierre* pierre)
{
 return pierre->poids;   
}

void pierreEcrirePoids(Pierre* pierre, int poids)
{
  pierre->poids = poids;    
}
