#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../../evenement/touche/touche.h"
#include "../type_joueur/joueur.h"
#include "../../liste/liste.h"

#include "liste_joueur.h"

/*On dispose d'aucune indication sur l'implémentation de la liste*/
/*On réalise les tests avec les axiomes de celle-ci*/

void oki ()
{
 fprintf(stderr, " TEST OK\n");  
}

void test_TailleNulle_avec_listeInitialiser ()
{
  Liste_joueur liste;
  int taille;
  fprintf(stderr, "* test_TailleNulle_avec_listeInit: ");
  
  liste_joueurInitialiser(&liste);
  taille = liste_joueurTaille(&liste);
  assert(taille ==0);  
  listeDetruire(&liste); 
  oki ();  
}

void test_TailleNulle1_avec_AjouterFin ()
{
  Liste_joueur liste;
  int taille;
  Joueur* joueur;
  fprintf(stderr, "* test_TailleNulle1_avec_ajouterFin: ");
  
  liste_joueurInitialiser(&liste);
  liste_joueurAjouterFin(&liste, joueur);
  taille = liste_joueurTaille(&liste);
  assert(taille ==1);   
  liste_joueurDetruire(&liste);
  oki ();  
}

void test_TailleNulle2_avec_AjouterFin ()
{
  Liste_joueur liste;
  int taille;
  Joueur* joueur;
  fprintf(stderr, "* test_TailleNulle2_avec_ajouterFin: ");
  
  liste_joueurInitialiser(&liste);
  liste_joueurAjouterFin(&liste, joueur);
  liste_joueurAjouterFin(&liste, joueur);
  taille = liste_joueurTaille(&liste);
  assert(taille ==2);   
  liste_joueurDetruire(&liste);
  oki ();  
}

void test_Lire_avec_AjouterFin ()
{
  Liste_joueur liste;
  Joueur* joueur;
  Joueur* joueur2;
  fprintf(stderr, "* test_Lire_avec_AjouterFin: ");
  
  joueur= joueurCreer();
  joueurEcrireX(joueur,13);
  joueurEcrireY(joueur, 14);
  
/*  fprintf(stderr, "--- %d ----", joueur);*/
  liste_joueurInitialiser(&liste);
  liste_joueurAjouterFin(&liste, joueur);
  /*liste_joueurAjouterFin(&liste, joueur);*/
  joueur2 = liste_joueurLire(&liste, 1);
 /* fprintf(stderr, "-LIRE %d--%d-",joueurLireY(joueur2),joueurLireX(joueur2));*/
  
 /* fprintf(stderr, "----%d ----- %d -----", joueur, joueur2);*/
  assert(joueur2 == joueur);   
  
  joueurDetruire(joueur);
  liste_joueurDetruire(&liste);
  oki ();  
}


void test_Lire_avec_ListeVide ()
{
  Liste_joueur liste;
  Joueur* joueur;
  fprintf(stderr, "* test_Lire_avec_ListeVide: ");
  
 

  liste_joueurInitialiser(&liste);
  joueur = liste_joueurLire(&liste, 4);
  assert(joueur == NULL);   
  
  joueurDetruire(joueur);
  liste_joueurDetruire(&liste);
  oki ();  
}

void test_retirer_Liste1 ()
{
  Liste_joueur liste;
  Joueur* joueur;
  int taille;
  fprintf(stderr, "* test_retirer_Liste1: ");
  joueur=joueurCreer();
 
  liste_joueurInitialiser(&liste);
  liste_joueurAjouterFin(&liste,joueur);
  liste_joueurRetirer(&liste, 1);
  taille = liste_joueurTaille(&liste);
  assert(taille == 0);   
  
  joueurDetruire(joueur);
  liste_joueurDetruire(&liste);
  oki ();  
}

void test_retirer_ListeVide ()
{
  Liste_joueur liste;
   int taille;
  fprintf(stderr, "* test_retirer_ListeVide: ");

 
  liste_joueurInitialiser(&liste);
  liste_joueurRetirer(&liste, 1);
  assert(taille == 0); 
  
  liste_joueurDetruire(&liste);
  oki ();  
}


void test_rechercher_listevide ()
{
  Liste_joueur liste;   
  int i;
  fprintf(stderr, "* test_rechercher_listevide: ");
  
  liste_joueurInitialiser(&liste);
  i = liste_joueurRechercher(&liste, 1,2); 
  assert(i == 0);  
  liste_joueurDetruire(&liste);
  oki ();  
     
}

void test_rechercher_liste1 ()
{
  Liste_joueur liste;   
  Joueur* joueur;
  int i;
  fprintf(stderr, "* test_rechercher_liste1: ");
  joueur = joueurCreer();
  liste_joueurInitialiser(&liste);
  joueurEcrireX(joueur, 1);
  joueurEcrireY(joueur, 2);
  liste_joueurAjouterFin(&liste, joueur);
  i = liste_joueurRechercher(&liste, 1,2); 
 
  assert(i == 1);  
  
  joueurDetruire(joueur);
  liste_joueurDetruire(&liste);
  oki ();  
     
}

void test_rechercher_liste2 ()
{
  Liste_joueur liste;   
  Joueur* joueur;
  Joueur* joueur2;
  int i;
  fprintf(stderr, "* test_rechercher_liste2: ");
  joueur = joueurCreer();
  joueur2 = joueurCreer();
  
  
  liste_joueurInitialiser(&liste);
  joueurEcrireX(joueur, 4);
  joueurEcrireY(joueur, 2);
  joueurEcrireX(joueur2, 1);
  joueurEcrireY(joueur2, 2);
 
  liste_joueurAjouterFin(&liste, joueur);
  liste_joueurAjouterFin(&liste, joueur2);
  
  
  i = liste_joueurRechercher(&liste, 1,2); 
 
  assert(i == 2);  
  
  joueurDetruire(joueur);
  joueurDetruire(joueur2);
  liste_joueurDetruire(&liste);
  oki ();  
     
}



int main ()
{
    fprintf(stderr, "=====TEST MODULE LISTE_JOUEUR======\n");
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
