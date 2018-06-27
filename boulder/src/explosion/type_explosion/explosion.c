#include <stdio.h>

#include "explosion.h"
#include <malloc.h>

/* module pour les explosions, regarder diamants ou pierre, même principe*/

Explosion* explosionCreer ()
{
  Explosion* explosion= malloc(sizeof(Explosion));
  if (explosion == NULL) {
               fprintf(stderr, "Probleme allocation explosion\n");
               return NULL;
  }
  explosionEcrireCompteur(explosion,0);
  explosionEcrireEtat(explosion, PROGRESSION);
  return explosion;
}

void explosionDetruire(Explosion* explosion)
{
  free(explosion);
}

void explosionCopie(Explosion* explosion, Explosion_type type)
{
     explosionEcrireType(explosion, type);
}


int explosionLireX (Explosion* explosion)
{
  return explosion->x;
}

int explosionLireY (Explosion* explosion)
{
  return explosion->y;
}

void explosionEcrireX(Explosion* explosion, int x)
{
  explosion->x = x;
}

void explosionEcrireY(Explosion* explosion, int y)
{
   explosion->y = y;
}

Explosion_etat explosionLireEtat(Explosion* explosion)
{
  return (explosion->etat);
}

void explosionEcrireEtat(Explosion* explosion, Explosion_etat etat)
{
  explosion->etat = etat;     
}



void explosionEcrireCompteur(Explosion* explosion, int compteur)
{
  explosion->compteur = compteur;    
}

int explosionLireCompteur(Explosion* explosion)
{
  return explosion->compteur;    
}



void explosionEcrireDuree(Explosion* explosion, int duree)
{
  explosion->duree = duree;    
}

int explosionLireDuree(Explosion* explosion)
{
  return explosion->duree;    
}



Explosion_type explosionLireType(Explosion* explosion)
{
  return (explosion->type);
}

void explosionEcrireType(Explosion* explosion, Explosion_type type)
{
  explosion->type = type;     
}
