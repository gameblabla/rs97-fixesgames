#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#include "../type_diamant/diamant.h"
#include "../../liste/liste.h"

#include "liste_diamant.h"

/*On dispose d'aucune indication sur l'implémentation de la liste*/
/*On réalise les tests avec les axiomes de celle-ci*/

void oki ()
{
 fprintf(stderr, " TEST OK\n");  
}

void test_TailleNulle_avec_listeInitialiser ()
{
  Liste_diamant liste;
  int taille;
  fprintf(stderr, "* test_TailleNulle_avec_listeInit: ");
  
  liste_diamantInitialiser(&liste);
  taille = liste_diamantTaille(&liste);
  assert(taille ==0);  
  listeDetruire(&liste); 
  oki ();  
}

void test_TailleNulle1_avec_AjouterFin ()
{
  Liste_diamant liste;
  int taille;
  Diamant* diamant;
  fprintf(stderr, "* test_TailleNulle1_avec_ajouterFin: ");
  
  liste_diamantInitialiser(&liste);
  liste_diamantAjouterFin(&liste, diamant);
  taille = liste_diamantTaille(&liste);
  assert(taille ==1);   
  liste_diamantDetruire(&liste);
  oki ();  
}

void test_TailleNulle2_avec_AjouterFin ()
{
  Liste_diamant liste;
  int taille;
  Diamant* diamant;
  fprintf(stderr, "* test_TailleNulle2_avec_ajouterFin: ");
  
  liste_diamantInitialiser(&liste);
  liste_diamantAjouterFin(&liste, diamant);
  liste_diamantAjouterFin(&liste, diamant);
  taille = liste_diamantTaille(&liste);
  assert(taille ==2);   
  liste_diamantDetruire(&liste);
  oki ();  
}

void test_Lire_avec_AjouterFin ()
{
  Liste_diamant liste;
  Diamant* diamant;
  Diamant* diamant2;
  fprintf(stderr, "* test_Lire_avec_AjouterFin: ");
  
  diamant= diamantCreer();
  diamantEcrireX(diamant,13);
  diamantEcrireY(diamant, 14);
  
/*  fprintf(stderr, "--- %d ----", diamant);*/
  liste_diamantInitialiser(&liste);
  liste_diamantAjouterFin(&liste, diamant);
  /*liste_diamantAjouterFin(&liste, diamant);*/
  diamant2 = liste_diamantLire(&liste, 1);
 /* fprintf(stderr, "-LIRE %d--%d-",diamantLireY(diamant2),diamantLireX(diamant2));*/
  
 /* fprintf(stderr, "----%d ----- %d -----", diamant, diamant2);*/
  assert(diamant2 == diamant);   
  
  diamantDetruire(diamant);
  liste_diamantDetruire(&liste);
  oki ();  
}


void test_Lire_avec_ListeVide ()
{
  Liste_diamant liste;
  Diamant* diamant;
  fprintf(stderr, "* test_Lire_avec_ListeVide: ");
  
 

  liste_diamantInitialiser(&liste);
  diamant = liste_diamantLire(&liste, 4);
  assert(diamant == NULL);   
  
  diamantDetruire(diamant);
  liste_diamantDetruire(&liste);
  oki ();  
}

void test_retirer_Liste1 ()
{
  Liste_diamant liste;
  Diamant* diamant;
  int taille;
  fprintf(stderr, "* test_retirer_Liste1: ");
  diamant=diamantCreer();
 
  liste_diamantInitialiser(&liste);
  liste_diamantAjouterFin(&liste,diamant);
  liste_diamantRetirer(&liste,1);
  taille = liste_diamantTaille(&liste);
  assert(taille == 0);   
  
  diamantDetruire(diamant);
  liste_diamantDetruire(&liste);
  oki ();  
}

void test_retirer_ListeVide ()
{
  Liste_diamant liste;
   int taille;
  fprintf(stderr, "* test_retirer_ListeVide: ");

 
  liste_diamantInitialiser(&liste);
  liste_diamantRetirer(&liste, 1);
  assert(taille == 0); 
  
  liste_diamantDetruire(&liste);
  oki ();  
}


void test_rechercher_listevide ()
{
  Liste_diamant liste;   
  int i;
  fprintf(stderr, "* test_rechercher_listevide: ");
  
  liste_diamantInitialiser(&liste);
  i = liste_diamantRechercher(&liste, 1,2); 
  assert(i == 0);  
  liste_diamantDetruire(&liste);
  oki ();  
     
}

void test_rechercher_liste1 ()
{
  Liste_diamant liste;   
  Diamant* diamant;
  int i;
  fprintf(stderr, "* test_rechercher_liste1: ");
  diamant = diamantCreer();
  liste_diamantInitialiser(&liste);
  diamantEcrireX(diamant, 1);
  diamantEcrireY(diamant, 2);
  liste_diamantAjouterFin(&liste, diamant);
  i = liste_diamantRechercher(&liste, 1,2); 
 
  assert(i == 1);  
  
  diamantDetruire(diamant);
  liste_diamantDetruire(&liste);
  oki ();  
     
}

void test_rechercher_liste2 ()
{
  Liste_diamant liste;   
  Diamant* diamant;
  Diamant* diamant2;
  int i;
  fprintf(stderr, "* test_rechercher_liste2: ");
  diamant = diamantCreer();
  diamant2 = diamantCreer();
  
  
  liste_diamantInitialiser(&liste);
  diamantEcrireX(diamant, 4);
  diamantEcrireY(diamant, 2);
  diamantEcrireX(diamant2, 1);
  diamantEcrireY(diamant2, 2);
 
  liste_diamantAjouterFin(&liste, diamant);
  liste_diamantAjouterFin(&liste, diamant2);
  
  
  i = liste_diamantRechercher(&liste, 1,2); 
 
  assert(i == 2);  
  
  diamantDetruire(diamant);
  diamantDetruire(diamant2);
  liste_diamantDetruire(&liste);
  oki ();  
     
}



int main ()
{
    fprintf(stderr, "=====TEST MODULE LISTE_DIAMANT======\n");
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
