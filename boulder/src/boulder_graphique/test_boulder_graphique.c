#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../moteur_graphique/moteur_graphique.h"

#include "../fichier/fichier.h"
#include "../niveau/niveau.h"
#include "../environnement/environnement.h"


#include "boulder_graphique.h"
void oki () { fprintf(stderr, " = OK\n"); }


void test_chargement_couleurs ()
{
  Ecran_joueur* ecran = ecran_joueurCreer(10,10);
  ecran_joueurInitialiser(ecran, (Niveau*) NULL, (Joueur*) NULL, 0,0,1, NULL);
  fprintf(stderr, "**Test de chargement couleurs");
  ecran_joueurChargementCouleurs(ecran, "./test/test_couleur.txt")   ;
  assert(ecran->couleurs->couleur_noir == g_couleurCreer(ecran->surface, 0,0,0));
  assert(ecran->couleurs->couleur_blanc == g_couleurCreer(ecran->surface, 255,255,255));
   assert(ecran->couleurs->couleur_tete == g_couleurCreer(ecran->surface, 255,153,153));  
    assert(ecran->couleurs->couleur_jambe == g_couleurCreer(ecran->surface, 255,153,0)); 
     assert(ecran->couleurs->couleur_moche == g_couleurCreer(ecran->surface, 153,153,153));
  ecran_joueurDetruire(ecran);   
  oki ();
     
}

void test_chargement_sprite ()
{
  G_fenetre fenetre;
  Ecran_joueur* ecran = ecran_joueurCreer(10,10);
  ecran_joueurInitialiser(ecran, NULL, NULL, 0,0,5, NULL);
  //fprintf(stderr, "22");
   g_fenetreInitialiser(&fenetre, 500,500, PAS_PLEIN_ECRAN);
 //fprintf(stderr, "22");
  G_surface* sprite;
  G_rectangle rect1;
  G_rectangle rect2;
  
  g_rectangleInitialiser(&rect1, 0,0,150,150);
  g_rectangleInitialiser(&rect2, 0,0,150,150);
  
  G_surface* ecran_principal =g_fenetreLireSurface(&fenetre);
    
  fprintf(stderr, "**Test de chargement sprite");
  ecran_joueurChargementCouleurs(ecran, "./test/test_couleur.txt")   ;     

  sprite = ecran_joueurChargementSprite(ecran, "./test/boulder.txt");
 

  g_surfaceCopie(sprite, &rect1, ecran_principal, &rect2);
  g_fenetreAfficher(&fenetre);
  
  while(!toucheEstAppuyer(T_SPACE));
  
  ecran_joueurDetruire(ecran);
  g_surfaceDetruire(sprite);
  g_fenetreDetruire(&fenetre);

  oki ();
     
     
}


int main ()
{
    fprintf(stderr, "=== TEST MODULE BOULDER GRAPHIQUE===\n");
    test_chargement_couleurs ();
  //  test_chargement_sprite ();
    
    
    
    
    fprintf(stderr, " MODULE OK\n");
 return 0;
}
