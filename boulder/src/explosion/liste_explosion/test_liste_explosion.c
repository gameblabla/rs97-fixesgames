#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#include "../type_explosion/explosion.h"
#include "../../liste/liste.h"

#include "liste_explosion.h"

/*On dispose d'aucune indication sur l'implémentation de la liste*/
/*On réalise les tests avec les axiomes de celle-ci*/

void oki ()
{
 fprintf(stderr, " TEST OK\n");  
}

void test_TailleNulle_avec_listeInitialiser ()
{
  Liste_explosion liste;
  int taille;
  fprintf(stderr, "* test_TailleNulle_avec_listeInit: ");
  
  liste_explosionInitialiser(&liste);
  taille = liste_explosionTaille(&liste);
  assert(taille ==0);  
  listeDetruire(&liste); 
  oki ();  
}

void test_TailleNulle1_avec_AjouterFin ()
{
  Liste_explosion liste;
  int taille;
  Explosion* explosion;
  fprintf(stderr, "* test_TailleNulle1_avec_ajouterFin: ");
  
  liste_explosionInitialiser(&liste);
  liste_explosionAjouterFin(&liste, explosion);
  taille = liste_explosionTaille(&liste);
  assert(taille ==1);   
  liste_explosionDetruire(&liste);
  oki ();  
}

void test_TailleNulle2_avec_AjouterFin ()
{
  Liste_explosion liste;
  int taille;
  Explosion* explosion;
  fprintf(stderr, "* test_TailleNulle2_avec_ajouterFin: ");
  
  liste_explosionInitialiser(&liste);
  liste_explosionAjouterFin(&liste, explosion);
  liste_explosionAjouterFin(&liste, explosion);
  taille = liste_explosionTaille(&liste);
  assert(taille ==2);   
  liste_explosionDetruire(&liste);
  oki ();  
}

void test_Lire_avec_AjouterFin ()
{
  Liste_explosion liste;
  Explosion* explosion;
  Explosion* explosion2;
  fprintf(stderr, "* test_Lire_avec_AjouterFin: ");
  
  explosion= explosionCreer();
  explosionEcrireX(explosion,13);
  explosionEcrireY(explosion, 14);
  
/*  fprintf(stderr, "--- %d ----", explosion);*/
  liste_explosionInitialiser(&liste);
  liste_explosionAjouterFin(&liste, explosion);
  /*liste_explosionAjouterFin(&liste, explosion);*/
  explosion2 = liste_explosionLire(&liste, 1);
 /* fprintf(stderr, "-LIRE %d--%d-",explosionLireY(explosion2),explosionLireX(explosion2));*/
  
 /* fprintf(stderr, "----%d ----- %d -----", explosion, explosion2);*/
  assert(explosion2 == explosion);   
  
  explosionDetruire(explosion);
  liste_explosionDetruire(&liste);
  oki ();  
}


void test_Lire_avec_ListeVide ()
{
  Liste_explosion liste;
  Explosion* explosion;
  fprintf(stderr, "* test_Lire_avec_ListeVide: ");
  
 

  liste_explosionInitialiser(&liste);
  explosion = liste_explosionLire(&liste, 4);
  assert(explosion == NULL);   
  
  explosionDetruire(explosion);
  liste_explosionDetruire(&liste);
  oki ();  
}

void test_retirer_Liste1 ()
{
  Liste_explosion liste;
  Explosion* explosion;
  int taille;
  fprintf(stderr, "* test_retirer_Liste1: ");
  explosion=explosionCreer();
 
  liste_explosionInitialiser(&liste);
  liste_explosionAjouterFin(&liste,explosion);
  liste_explosionRetirer(&liste, 1);
  taille = liste_explosionTaille(&liste);
  assert(taille == 0);   
  
  explosionDetruire(explosion);
  liste_explosionDetruire(&liste);
  oki ();  
}

void test_retirer_ListeVide ()
{
  Liste_explosion liste;
   int taille;
  fprintf(stderr, "* test_retirer_ListeVide: ");

 
  liste_explosionInitialiser(&liste);
  liste_explosionRetirer(&liste, 1);
  assert(taille == 0); 
  
  liste_explosionDetruire(&liste);
  oki ();  
}


void test_rechercher_listevide ()
{
  Liste_explosion liste;   
  int i;
  fprintf(stderr, "* test_rechercher_listevide: ");
  
  liste_explosionInitialiser(&liste);
  i = liste_explosionRechercher(&liste, 1,2); 
  assert(i == 0);  
  liste_explosionDetruire(&liste);
  oki ();  
     
}

void test_rechercher_liste1 ()
{
  Liste_explosion liste;   
  Explosion* explosion;
  int i;
  fprintf(stderr, "* test_rechercher_liste1: ");
  explosion = explosionCreer();
  liste_explosionInitialiser(&liste);
  explosionEcrireX(explosion, 1);
  explosionEcrireY(explosion, 2);
  liste_explosionAjouterFin(&liste, explosion);
  i = liste_explosionRechercher(&liste, 1,2); 
 
  assert(i == 1);  
  
  explosionDetruire(explosion);
  liste_explosionDetruire(&liste);
  oki ();  
     
}

void test_rechercher_liste2 ()
{
  Liste_explosion liste;   
  Explosion* explosion;
  Explosion* explosion2;
  int i;
  fprintf(stderr, "* test_rechercher_liste2: ");
  explosion = explosionCreer();
  explosion2 = explosionCreer();
  
  
  liste_explosionInitialiser(&liste);
  explosionEcrireX(explosion, 4);
  explosionEcrireY(explosion, 2);
  explosionEcrireX(explosion2, 1);
  explosionEcrireY(explosion2, 2);
 
  liste_explosionAjouterFin(&liste, explosion);
  liste_explosionAjouterFin(&liste, explosion2);
  
  
  i = liste_explosionRechercher(&liste, 1,2); 
 
  assert(i == 2);  
  
  explosionDetruire(explosion);
  explosionDetruire(explosion2);
  liste_explosionDetruire(&liste);
  oki ();  
     
}



int main ()
{
    fprintf(stderr, "=====TEST MODULE LISTE_EXPLOSION======\n");
    fprintf(stderr, "*** test_Taille\n");
    test_TailleNulle_avec_listeInitialiser ();
    test_TailleNulle1_avec_AjouterFin ();
    test_TailleNulle2_avec_AjouterFin ();
    
     fprintf(stderr, "*** test_Lire\n");
    test_Lire_avec_AjouterFin ();
   test_Lire_avec_ListeVide ();
   
   fprintf(stderr, "*** test_retirer\n");
   test_retirer_Liste1 ();
   test_retirer_ListeVide ();
   
    fprintf(stderr, "*** test_rechercher\n");
   test_rechercher_listevide ();
   test_rechercher_liste1 ();
   
   
   fprintf(stderr, "TOUT OK \n\n");
    return 0;
}
