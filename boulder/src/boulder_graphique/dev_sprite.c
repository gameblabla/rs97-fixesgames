#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../moteur_graphique/moteur_graphique.h"

#include "../fichier/fichier.h"
#include "../niveau/niveau.h"
#include "../environnement/environnement.h"


#include "boulder_graphique.h"
void machin (int argc, char** argv)
{

 G_fenetre fenetre;
  Ecran_joueur* ecran = ecran_joueurCreer(10,10);
  ecran_joueurInitialiser(ecran, NULL, NULL, 0,0,5, NULL);

   g_fenetreInitialiser(&fenetre, 320,240, PAS_PLEIN_ECRAN);

  G_surface* sprite;
  G_rectangle rect1;
  G_rectangle rect2;
  
  g_rectangleInitialiser(&rect1, 0,0,150,150);
  g_rectangleInitialiser(&rect2, 0,0,150,150);
  
  G_surface* ecran_principal =g_fenetreLireSurface(&fenetre);
    
  
  ecran_joueurChargementCouleurs(ecran, "./test/test_couleur.txt")   ;     
//fprintf(stderr, "  %d   ", argc);
 //if (argc == 1)
 sprite = ecran_joueurChargementSprite(ecran, "./test/sprite.txt");
// else
//  sprite = ecran_joueurChargementSprite(ecran, argv[1]); 

  g_surfaceCopie(sprite, &rect1, ecran_principal, &rect2);
  g_fenetreAfficher(&fenetre);
  
  while(!toucheEstAppuyer(T_SPACE));
  
  ecran_joueurDetruire(ecran);
//  g_surfaceDetruire(sprite);
  g_fenetreDetruire(&fenetre);

}

int main (int argc, char** argv)
{
 machin (argc, argv);
   

 return 0;
}
