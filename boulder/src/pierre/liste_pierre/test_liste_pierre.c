#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#include "../type_pierre/pierre.h"
#include "../../liste/liste.h"

#include "liste_pierre.h"

/*On dispose d'aucune indication sur l'implémentation de la liste*/
/*On réalise les tests avec les axiomes de celle-ci*/

void oki ()
{
 fprintf(stderr, " TEST OK\n");  
}

void test_TailleNulle_avec_listeInitialiser ()
{
  Liste_pierre liste;
  int taille;
  fprintf(stderr, "* test_TailleNulle_avec_listeInit: ");
  
  liste_pierreInitialiser(&liste);
  taille = liste_pierreTaille(&liste);
  assert(taille ==0);  
  listeDetruire(&liste); 
  oki ();  
}

void test_TailleNulle1_avec_AjouterFin ()
{
  Liste_pierre liste;
  int taille;
  Pierre* pierre;
  fprintf(stderr, "* test_TailleNulle1_avec_ajouterFin: ");
  
  liste_pierreInitialiser(&liste);
  liste_pierreAjouterFin(&liste, pierre);
  taille = liste_pierreTaille(&liste);
  assert(taille ==1);   
  liste_pierreDetruire(&liste);
  oki ();  
}

void test_TailleNulle2_avec_AjouterFin ()
{
  Liste_pierre liste;
  int taille;
  Pierre* pierre;
  fprintf(stderr, "* test_TailleNulle2_avec_ajouterFin: ");
  
  liste_pierreInitialiser(&liste);
  liste_pierreAjouterFin(&liste, pierre);
  liste_pierreAjouterFin(&liste, pierre);
  taille = liste_pierreTaille(&liste);
  assert(taille ==2);   
  liste_pierreDetruire(&liste);
  oki ();  
}

void test_Lire_avec_AjouterFin ()
{
  Liste_pierre liste;
  Pierre* pierre;
  Pierre* pierre2;
  fprintf(stderr, "* test_Lire_avec_AjouterFin: ");
  
  pierre= pierreCreer();
  pierreEcrireX(pierre,13);
  pierreEcrireY(pierre, 14);
  
/*  fprintf(stderr, "--- %d ----", pierre);*/
  liste_pierreInitialiser(&liste);
  liste_pierreAjouterFin(&liste, pierre);
  /*liste_pierreAjouterFin(&liste, pierre);*/
  pierre2 = liste_pierreLire(&liste, 1);
 /* fprintf(stderr, "-LIRE %d--%d-",pierreLireY(pierre2),pierreLireX(pierre2));*/
  
 /* fprintf(stderr, "----%d ----- %d -----", pierre, pierre2);*/
  assert(pierre2 == pierre);   
  
  pierreDetruire(pierre);
  liste_pierreDetruire(&liste);
  oki ();  
}


void test_Lire_avec_ListeVide ()
{
  Liste_pierre liste;
  Pierre* pierre;
  fprintf(stderr, "* test_Lire_avec_ListeVide: ");
  
 

  liste_pierreInitialiser(&liste);
  pierre = liste_pierreLire(&liste, 4);
  assert(pierre == NULL);   
  
  pierreDetruire(pierre);
  liste_pierreDetruire(&liste);
  oki ();  
}

void test_retirer_Liste1 ()
{
  Liste_pierre liste;
  Pierre* pierre;
  int taille;
  fprintf(stderr, "* test_retirer_Liste1: ");
  pierre=pierreCreer();
 
  liste_pierreInitialiser(&liste);
  liste_pierreAjouterFin(&liste,pierre);
  liste_pierreRetirer(&liste, 1);
  taille = liste_pierreTaille(&liste);
  assert(taille == 0);   
  
  pierreDetruire(pierre);
  liste_pierreDetruire(&liste);
  oki ();  
}

void test_retirer_ListeVide ()
{
  Liste_pierre liste;
   int taille;
  fprintf(stderr, "* test_retirer_ListeVide: ");

 
  liste_pierreInitialiser(&liste);
  liste_pierreRetirer(&liste, 1);
  assert(taille == 0); 
  
  liste_pierreDetruire(&liste);
  oki ();  
}


void test_rechercher_listevide ()
{
  Liste_pierre liste;   
  int i;
  fprintf(stderr, "* test_rechercher_listevide: ");
  
  liste_pierreInitialiser(&liste);
  i = liste_pierreRechercher(&liste, 1,2); 
  assert(i == 0);  
  liste_pierreDetruire(&liste);
  oki ();  
     
}

void test_rechercher_liste1 ()
{
  Liste_pierre liste;   
  Pierre* pierre;
  int i;
  fprintf(stderr, "* test_rechercher_liste1: ");
  pierre = pierreCreer();
  liste_pierreInitialiser(&liste);
  pierreEcrireX(pierre, 1);
  pierreEcrireY(pierre, 2);
  liste_pierreAjouterFin(&liste, pierre);
  i = liste_pierreRechercher(&liste, 1,2); 
 
  assert(i == 1);  
  
  pierreDetruire(pierre);
  liste_pierreDetruire(&liste);
  oki ();  
     
}

void test_rechercher_liste2 ()
{
  Liste_pierre liste;   
  Pierre* pierre;
  Pierre* pierre2;
  int i;
  fprintf(stderr, "* test_rechercher_liste2: ");
  pierre = pierreCreer();
  pierre2 = pierreCreer();
  
  
  liste_pierreInitialiser(&liste);
  pierreEcrireX(pierre, 4);
  pierreEcrireY(pierre, 2);
  pierreEcrireX(pierre2, 1);
  pierreEcrireY(pierre2, 2);
 
  liste_pierreAjouterFin(&liste, pierre);
  liste_pierreAjouterFin(&liste, pierre2);
  
  
  i = liste_pierreRechercher(&liste, 1,2); 
 
  assert(i == 2);  
  
  pierreDetruire(pierre);
  pierreDetruire(pierre2);
  liste_pierreDetruire(&liste);
  oki ();  
     
}



int main ()
{
    fprintf(stderr, "=====TEST MODULE LISTE_PIERRE======\n");
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
