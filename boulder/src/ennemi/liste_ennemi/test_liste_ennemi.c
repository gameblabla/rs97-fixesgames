#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#include "../type_ennemi/ennemi.h"
#include "../../liste/liste.h"

#include "liste_ennemi.h"

/*On dispose d'aucune indication sur l'implémentation de la liste*/
/*On réalise les tests avec les axiomes de celle-ci*/

void oki ()
{
 fprintf(stderr, " TEST OK\n");  
}

void test_TailleNulle_avec_listeInitialiser ()
{
  Liste_ennemi liste;
  int taille;
  fprintf(stderr, "* test_TailleNulle_avec_listeInit: ");
  
  liste_ennemiInitialiser(&liste);
  taille = liste_ennemiTaille(&liste);
  assert(taille ==0);  
  listeDetruire(&liste); 
  oki ();  
}

void test_TailleNulle1_avec_AjouterFin ()
{
  Liste_ennemi liste;
  int taille;
  Ennemi* ennemi;
  fprintf(stderr, "* test_TailleNulle1_avec_ajouterFin: ");
  
  liste_ennemiInitialiser(&liste);
  liste_ennemiAjouterFin(&liste, ennemi);
  taille = liste_ennemiTaille(&liste);
  assert(taille ==1);   
  liste_ennemiDetruire(&liste);
  oki ();  
}

void test_TailleNulle2_avec_AjouterFin ()
{
  Liste_ennemi liste;
  int taille;
  Ennemi* ennemi;
  fprintf(stderr, "* test_TailleNulle2_avec_ajouterFin: ");
  
  liste_ennemiInitialiser(&liste);
  liste_ennemiAjouterFin(&liste, ennemi);
  liste_ennemiAjouterFin(&liste, ennemi);
  taille = liste_ennemiTaille(&liste);
  assert(taille ==2);   
  liste_ennemiDetruire(&liste);
  oki ();  
}

void test_Lire_avec_AjouterFin ()
{
  Liste_ennemi liste;
  Ennemi* ennemi;
  Ennemi* ennemi2;
  fprintf(stderr, "* test_Lire_avec_AjouterFin: ");
  
  ennemi= ennemiCreer();
  ennemiEcrireX(ennemi,13);
  ennemiEcrireY(ennemi, 14);
  
/*  fprintf(stderr, "--- %d ----", ennemi);*/
  liste_ennemiInitialiser(&liste);
  liste_ennemiAjouterFin(&liste, ennemi);
  /*liste_ennemiAjouterFin(&liste, ennemi);*/
  ennemi2 = liste_ennemiLire(&liste, 1);
 /* fprintf(stderr, "-LIRE %d--%d-",ennemiLireY(ennemi2),ennemiLireX(ennemi2));*/
  
 /* fprintf(stderr, "----%d ----- %d -----", ennemi, ennemi2);*/
  assert(ennemi2 == ennemi);   
  
  ennemiDetruire(ennemi);
  liste_ennemiDetruire(&liste);
  oki ();  
}


void test_Lire_avec_ListeVide ()
{
  Liste_ennemi liste;
  Ennemi* ennemi;
  fprintf(stderr, "* test_Lire_avec_ListeVide: ");
  
 

  liste_ennemiInitialiser(&liste);
  ennemi = liste_ennemiLire(&liste, 4);
  assert(ennemi == NULL);   
  
  ennemiDetruire(ennemi);
  liste_ennemiDetruire(&liste);
  oki ();  
}

void test_retirer_Liste1 ()
{
  Liste_ennemi liste;
  Ennemi* ennemi;
  int taille;
  fprintf(stderr, "* test_retirer_Liste1: ");
  ennemi=ennemiCreer();
 
  liste_ennemiInitialiser(&liste);
  liste_ennemiAjouterFin(&liste,ennemi);
  liste_ennemiRetirer(&liste, 1);
  taille = liste_ennemiTaille(&liste);
  assert(taille == 0);   
  
  ennemiDetruire(ennemi);
  liste_ennemiDetruire(&liste);
  oki ();  
}

void test_retirer_ListeVide ()
{
  Liste_ennemi liste;
   int taille;
  fprintf(stderr, "* test_retirer_ListeVide: ");

 
  liste_ennemiInitialiser(&liste);
  liste_ennemiRetirer(&liste, 1);
  assert(taille == 0); 
  
  liste_ennemiDetruire(&liste);
  oki ();  
}


void test_rechercher_listevide ()
{
  Liste_ennemi liste;   
  int i;
  fprintf(stderr, "* test_rechercher_listevide: ");
  
  liste_ennemiInitialiser(&liste);
  i = liste_ennemiRechercher(&liste, 1,2); 
  assert(i == 0);  
  liste_ennemiDetruire(&liste);
  oki ();  
     
}

void test_rechercher_liste1 ()
{
  Liste_ennemi liste;   
  Ennemi* ennemi;
  int i;
  fprintf(stderr, "* test_rechercher_liste1: ");
  ennemi = ennemiCreer();
  liste_ennemiInitialiser(&liste);
  ennemiEcrireX(ennemi, 1);
  ennemiEcrireY(ennemi, 2);
  liste_ennemiAjouterFin(&liste, ennemi);
  i = liste_ennemiRechercher(&liste, 1,2); 
 
  assert(i == 1);  
  
  ennemiDetruire(ennemi);
  liste_ennemiDetruire(&liste);
  oki ();  
     
}

void test_rechercher_liste2 ()
{
  Liste_ennemi liste;   
  Ennemi* ennemi;
  Ennemi* ennemi2;
  int i;
  fprintf(stderr, "* test_rechercher_liste2: ");
  ennemi = ennemiCreer();
  ennemi2 = ennemiCreer();
  
  
  liste_ennemiInitialiser(&liste);
  ennemiEcrireX(ennemi, 4);
  ennemiEcrireY(ennemi, 2);
  ennemiEcrireX(ennemi2, 1);
  ennemiEcrireY(ennemi2, 2);
 
  liste_ennemiAjouterFin(&liste, ennemi);
  liste_ennemiAjouterFin(&liste, ennemi2);
  
  
  i = liste_ennemiRechercher(&liste, 1,2); 
 
  assert(i == 2);  
  
  ennemiDetruire(ennemi);
  ennemiDetruire(ennemi2);
  liste_ennemiDetruire(&liste);
  oki ();  
     
}



int main ()
{
    fprintf(stderr, "=====TEST MODULE LISTE_ENNEMI======\n");
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
