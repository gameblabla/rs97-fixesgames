#include "slim.h"

/*voir diamant, même principe, ce ne sont que des accesseurs*/

int slimLireCompteurGraphique(Slim* mur)
{
  return mur->compteur_graphique;
}

void slimEcrireCompteurGraphique( Slim* mur, int i)
{
     mur->compteur_graphique = i;
}

Slim_type slimLireType(Slim* mur)
{
   return mur->type;
}

void slimEcrireType(Slim* mur, Slim_type type)
{
     mur->type=type;
}

Slim_vitesse slimLireVitesse(Slim* mur)
{
  return mur->vitesse;
}

void slimEcrireVitesse( Slim* mur, Slim_vitesse i)
{
     mur->vitesse = i;
}
