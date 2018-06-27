#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "../moteur_graphique/moteur_graphique.h"

#include "../fichier/fichier.h"
#include "../niveau/niveau.h"
#include "../environnement/environnement.h"


#include "boulder_graphique.h"

/*
 * Fonctions accesseurs, le nom des fonctions sont assez explicites
 */
G_surface*             ecran_joueurLireSurface(Ecran_joueur* ecran)
{
  return ecran->surface;
}


int ecran_joueurLireResolutionx (Ecran_joueur* ecran)
{
    return ecran->resolutionx;
}

int ecran_joueurLireResolutiony (Ecran_joueur* ecran)
{
    return ecran->resolutiony;
}


Niveau* ecran_joueurLireNiveau(Ecran_joueur* ecran)
{
  return ecran->niveau; 
}

void ecran_joueurEcrireNiveau(Ecran_joueur* ecran, Niveau* niveau)
{
  ecran->niveau = niveau;
}
