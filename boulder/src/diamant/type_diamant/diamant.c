#include <stdio.h>
#include "diamant.h"
#include <malloc.h>

Diamant* diamantCreer ()
{
  /* on alloue*/
  Diamant* diamant= malloc(sizeof(Diamant));
  if (diamant == NULL) {
    fprintf(stderr, "Probleme d'allocation pour diamant\n");   
    return NULL;
  }
  
  /* on initialise*/
  diamantEcrireCompteurGraphique(diamant, 0);
  diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BOUGE_PAS);
  return diamant;
}

void diamantDetruire(Diamant* diamant)
{
  free(diamant);
}



int diamantLireX (Diamant* diamant)
{
  return diamant->x;
}

int diamantLireY (Diamant* diamant)
{
  return diamant->y;
}

void diamantEcrireX(Diamant* diamant, int x)
{
  diamant->x = x;
}

void diamantEcrireY(Diamant* diamant, int y)
{
   diamant->y = y;
}

Diamant_direction diamantLireDirection(Diamant* diamant)
{
  return (diamant->direction);
}

void diamantEcrireDirection(Diamant* diamant, Diamant_direction direction)
{
  diamant->direction = direction;     
}

Diamant_transformation_chute diamantLireTransformationChute (Diamant* diamant)
{
   return diamant->transformation;                            
}

void diamantEcrireTransformationChute(Diamant* diamant, Diamant_transformation_chute transformation)
{
  diamant->transformation = transformation;     
}

int diamantLireValeur (Diamant* diamant)
{
 return diamant->valeur;   
}

void diamantEcrireValeur(Diamant* diamant, int valeur)
{
  diamant->valeur = valeur;    
}


void diamantEcrireType(Diamant* diamant, Diamant_type type)
{
  diamant->type = type;    
}

Diamant_type diamantLireType(Diamant* diamant)
{
  if (diamant==NULL)
    fprintf(stderr, "probleme diamant NULL");
  return diamant->type;
}

int diamantLireCompteurGraphique(Diamant* mur)
{
  return mur->compteur_graphique;
}

void diamantEcrireCompteurGraphique( Diamant* mur, int i)
{
     mur->compteur_graphique = i;
}

void diamantCopie(Diamant* diamant, int i, int j, Diamant_type type, Diamant_transformation_chute chute)
{
  diamantEcrireX(diamant, i);
  diamantEcrireY(diamant, j);
  diamantEcrireType(diamant, type);
  diamantEcrireTransformationChute(diamant, chute);     
     
}
