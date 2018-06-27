#include <stdio.h>

#include "ennemi.h"
#include <malloc.h>

/* Module permettant la gestion des ennemis*/


Ennemi* ennemiCreer ()
{
  Ennemi* ennemi =malloc(sizeof(Ennemi));
  if (ennemi == NULL) {
   fprintf(stderr, "Probleme allocation ennemi");
   return NULL;
  }
  ennemiEcrireCompteurGraphique(ennemi, 0);
  return ennemi;    
}

void ennemiDetruire(Ennemi* ennemi)
{
  free(ennemi);
}



int ennemiLireX (Ennemi* ennemi)
{
  return ennemi->x;
}

int ennemiLireY (Ennemi* ennemi)
{
  return ennemi->y;
}

void ennemiEcrireX(Ennemi* ennemi, int x)
{
  ennemi->x = x;
}

void ennemiEcrireY(Ennemi* ennemi, int y)
{
   ennemi->y = y;
}

Ennemi_direction ennemiLireDirection(Ennemi* ennemi)
{
  return (ennemi->direction);
}

void ennemiEcrireDirection(Ennemi* ennemi, Ennemi_direction direction)
{
  ennemi->direction = direction;     
}


Ennemi_type ennemiLireType (Ennemi* ennemi)
{
 return ennemi->type;   
}

void ennemiEcrireType(Ennemi* ennemi, Ennemi_type type)
{
  ennemi->type = type;    
}

int ennemiLireCompteurGraphique(Ennemi* mur)
{
  return mur->compteur_graphique;
}

void ennemiEcrireCompteurGraphique( Ennemi* mur, int i)
{
     mur->compteur_graphique = i;
}

void ennemiCopie(Ennemi* ennemi, int i, int j, Ennemi_type type, Ennemi_direction direction)
{
 ennemiEcrireX(ennemi, i);
 ennemiEcrireY(ennemi, j);
 ennemiEcrireType(ennemi, type);
 ennemiEcrireDirection(ennemi, direction);    
}
