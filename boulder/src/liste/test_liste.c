#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "liste.h"

void oki ()
{
 fprintf(stderr, " TEST OK\n");  
}

/*test initialisation*/
void test_listeInitialiser_tailleNulle () 
{
    
    Liste liste;
    fprintf(stderr, "*** Test de listeInitialiser_tailleNulle =");
    listeInitialiser(&liste);
    assert(liste.taille == 0); 
    oki();
}

void test_listeInitialiser_champ_premierNulle ()
{
  Liste liste;
  fprintf(stderr, "*** Test de listeInitialiser_premierNulle =");
  listeInitialiser(&liste);
  assert(liste.premier == NULL); 
  oki();
}
void test_listeInitialiser_champ_dernierNulle ()
{
  Liste liste;
  fprintf(stderr, "*** Test de listeInitialiser_dernierNulle =");
  listeInitialiser(&liste);
  assert(liste.dernier == NULL); 
  oki();
}

void test_listeInitialiser_champ_dernierePositionNulle ()
{
  Liste liste;
  fprintf(stderr, "*** Test de listeInitialiser_dernierepositionNulle =");
  listeInitialiser(&liste);
  assert(liste.derniere_position == 0); 
  oki();
}

void test_listeInitialiser_champ_derniereCelluleNulle ()
{
  Liste liste;
  fprintf(stderr, "*** Test de listeInitialiser_derniereCelluleNulle =");
  listeInitialiser(&liste);
  assert(liste.derniere_cellule_lue == NULL); 
  oki();
}
/*
void test_listeInitialiser_champ_derniereFonction ()
{
  Liste liste;
  fprintf(stderr, "*** Test de listeInitialiser_derniereFonction =");
  listeInitialiser(&liste);
  assert(liste.derniere_fonction == FONCTION_MODIFICATRICE_ADRESSE); 
  oki();
}

*/


  
  



/*test ajoute*/
void test_listeAjouter_listevide ()
{
  Liste liste;    
  int i = 45;
  fprintf(stderr, "*** Test de listeAjouter_listevide =");     
  listeInitialiser(&liste);

  listeAjouter(&liste, 1, &i);
  assert ((* (int*) liste.premier->element) == i);
  assert((* (int*) liste.dernier->element) == i);
  assert(liste.derniere_cellule_lue == NULL);
 /* assert(liste.derniere_fonction == FONCTION_MODIFICATRICE_ADRESSE);*/
  listeDetruire(&liste);
  oki();  
}

void test_listeAjouter_liste2elements_1 ()
{
  Liste liste;
  int i = 45;
  int j= 12;
  
  fprintf(stderr, "*** Test de listeAjouter_liste2elements_1 =");       
  listeInitialiser(&liste);
  listeAjouter(&liste, 1, &i);  
  listeAjouter(&liste, 1, &j);
  assert((* (int*)liste.premier->element) == j);
  assert((* (int*)liste.dernier->element) == i);
  assert(liste.derniere_cellule_lue == NULL);
 /* assert(liste.derniere_fonction == FONCTION_MODIFICATRICE_ADRESSE);*/
 listeDetruire(&liste);
  oki ();   
}

void test_listeAjouter_liste2elements_2 ()
{
  Liste liste;
  int i = 45;
  int j= 12;
  
  fprintf(stderr, "*** Test de listeAjouter_liste2elements_2 =");       
  listeInitialiser(&liste);
  listeAjouter(&liste, 1, &i);  
  listeAjouter(&liste, 2, &j);
  assert((* (int*) liste.premier->element) == i);
  assert((* (int*)liste.dernier->element) == j);
  assert(liste.derniere_cellule_lue == NULL);
 /* assert(liste.derniere_fonction == FONCTION_MODIFICATRICE_ADRESSE);*/
 listeDetruire(&liste);
  oki ();   
}

void test_listeAjouter_horsborne ()
{
      Liste liste;
  int i = 45;
  int j= 12;
  
  fprintf(stderr, "*** Test de listeAjouter_horsborne =");       
  listeInitialiser(&liste);
  listeAjouter(&liste, -1, &i);  
  listeAjouter(&liste, 2, &j); 
  assert(liste.premier == NULL);
  assert(liste.dernier == NULL);
 listeDetruire(&liste);
  oki ();
     
}


/*test taille*/
void test_listeTaille()
{
 Liste liste;
 int j;
 int i = 4;
 fprintf(stderr, "*** Test de listeTaille: ");
 listeInitialiser(&liste);
 for (j=1; j<=3;j++)
 { 
   listeAjouter(&liste, 1, &i);
   assert(listeTaille(&liste) == j);
 }
 listeDetruire(&liste);
 oki ();
 
}





/*test lire*/

void test_listeLire_horsborne ()
{
 Liste liste;  
 fprintf(stderr, "*** Test de listeLire_horsborne: ");
 listeInitialiser(&liste);  
 assert(listeLire(&liste, -1)== NULL);
 assert(listeLire(&liste, 10) == NULL);
 listeDetruire(&liste);
 oki ();
}

void test_listeLire_elements ()
{
  Liste liste;
  int i = 45;
  int j= 12;
  int *h;
  fprintf(stderr, "*** Test de listeLire_elements =");       
  listeInitialiser(&liste);
  listeAjouter(&liste, 1, &i);  
  listeAjouter(&liste, 2, &j); 
  h =listeLire(&liste, 1);
  assert(liste.derniere_position == 1);
  assert(*(int*)h == i);
  h = listeLire(&liste, 2);
  assert(*(int*)h == j);
  assert(liste.derniere_position == 2);
  assert(liste.derniere_cellule_lue != NULL);
 listeDetruire(&liste);
 /* assert(liste.derniere_fonction == FONCTION_NON_MODIFICATRICE_ADRESSE);*/
  oki ();
}





/*test ecrire*/
void test_listeEcrire_horsborne ()
{
   int i = 42;  
   Liste liste;   
     fprintf(stderr, "*** Test de listeEcrire_hors_borne ="); 
   listeInitialiser(&liste);
   listeEcrire(&liste, 12, &i);
   listeEcrire(&liste, -1, &i);
   
   assert(listeTaille(&liste) == 0);
 listeDetruire(&liste);
   oki ();
     
}

void test_listeEcrire_1element()
{
  Liste liste;
  int i = 45;
  int j = 46;
  int *h;
  fprintf(stderr, "*** Test de listeEcrire_1element =");      
  listeInitialiser(&liste);
  listeAjouter(&liste, 1, &i);

  listeEcrire(&liste, 1, &j);
  assert(liste.derniere_position == 1);
  h = listeLire(&liste, 1);
  assert(liste.derniere_position == 1);
  assert(*(int*)h == j);
 listeDetruire(&liste);
  oki ();
}


/*test retire*/
void test_listeRetire_1element ()
{
  Liste liste;
  int i = 45;

  fprintf(stderr, "*** Test de listeRetire_1element =");       
  listeInitialiser(&liste);
  listeAjouter(&liste, 1, &i);     
  listeRetirer(&liste,1);
  assert(listeTaille(&liste) == 0);
 listeDetruire(&liste);
  oki ();
}

void test_listeRetire_liste_de_deux_elements ()
{
  Liste liste;
  int i = 4;
  int j = 6;
  int *h;

  fprintf(stderr, "*** Test de listeRetire_de_deux_elements =");       
  listeInitialiser(&liste);
  listeAjouter(&liste, 1, &i);  
  listeAjouter(&liste, 1, &j);   
  listeRetirer(&liste,1);
  h= listeLire(&liste,1);
  assert(*h = j);
 listeDetruire(&liste);
  oki ();     
     
}

void test_listeRetire_horsborne ()
{
  Liste liste;
  int i = 4;
  int j = 6;
  int *h;
  fprintf(stderr, "*** Test de listeRetire_horsborne =");       
  listeInitialiser(&liste);
  listeAjouter(&liste, 1, &i);  
  listeAjouter(&liste, 1, &j);   
  listeRetirer(&liste,5);
  listeRetirer(&liste,-1);
 
  h =listeLire(&liste, 1);
  assert(*(int*)h == j);
  h = listeLire(&liste, 2);
  assert(*(int *)h == i);    
  assert(listeTaille(&liste) == 2);
 listeDetruire(&liste);
  oki ();
     
}

/*Destruction*/

void test_listeDetruire_listevide ()
{
   Liste liste; 
   fprintf(stderr, "*** Test de listeDetruire_listevide =");  
   listeInitialiser(&liste);
   listeDetruire(&liste);
   assert(listeTaille(&liste) == 0);  
 listeDetruire(&liste);
   oki ();  
}

void test_listeDetruire_liste_2_elements ()
{
    Liste liste;
  int i = 4;
  int j = 6;

  fprintf(stderr, "*** Test de listeDetruire_de_deux_elements =");       
  listeInitialiser(&liste);
  listeAjouter(&liste, 1, &i);  
  listeAjouter(&liste, 1, &j);   
  assert(listeTaille(&liste) ==2);  
  listeDetruire(&liste);
  assert(listeTaille(&liste) == 0);
 listeDetruire(&liste);
  oki ();   
}



/*test AjouterFin*/
void test_AjouterFin_listevide ()
{
  Liste liste;
  int i = 4;
  int j = 6;
  int k = 7;  
  int* h;
  
  fprintf(stderr, "*** Test de listeAjouterFin_listevide =");       
  listeInitialiser(&liste);
  listeAjouterFin(&liste, (int*) &i);  
  listeAjouterFin(&liste, &j);   
  listeAjouterFin(&liste, &k);

  h=listeLire(&liste, 1);
  assert(*(int*)h == i);
  h=listeLire(&liste, 2);
  assert(*(int*)h == j);
  h=listeLire(&liste, 3);
  assert(*(int*)h == k);
    
    listeDetruire(&liste);  
  oki ();   
}

int main ()
{
 fprintf(stderr, "=====Test de la liste avec des int=====\n");
 
 
 /*init*/
  fprintf(stderr,"* Test de listeInitialiser\n");
  test_listeInitialiser_tailleNulle () ;
  test_listeInitialiser_champ_premierNulle ();
  test_listeInitialiser_champ_dernierNulle ();
  /*test_listeInitialiser_champ_derniereFonction ();*/
  test_listeInitialiser_champ_derniereCelluleNulle ();
  test_listeInitialiser_champ_dernierePositionNulle ();
    
 /*Ajout*/
 fprintf(stderr,"* Test de listeAjout\n");
 test_listeAjouter_listevide ();
 test_listeAjouter_liste2elements_1 ();
 test_listeAjouter_liste2elements_2 ();
 test_listeAjouter_horsborne ();
 
 /*taille*/
  fprintf(stderr, "* Test de listeTaille\n");   
  test_listeTaille();  
 /*lire*/
 fprintf(stderr, "* Test de listeLire\n");
  test_listeLire_horsborne ();
  test_listeLire_elements ();
  
  fprintf(stderr, "* Test de listeEcrire\n");
  test_listeEcrire_1element();
  test_listeEcrire_horsborne ();
  
  /*retirer*/
  fprintf(stderr, "* Test de listeRetirer\n");
  test_listeRetire_1element ();
  test_listeRetire_liste_de_deux_elements ();
  test_listeRetire_horsborne ();
  
  
  /*ajouterfin*/
  fprintf(stderr, "* Test de listeAjouterFin\n");  
  test_AjouterFin_listevide ();
    
  /*detruire*/
  fprintf(stderr, "* Test de listeDetruire\n");
  test_listeDetruire_listevide ();
  test_listeDetruire_liste_2_elements ();
  
  fprintf(stderr, "TOUT OK\n");
  
  return EXIT_SUCCESS;  
}
