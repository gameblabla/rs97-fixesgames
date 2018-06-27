#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <SDL/SDL.h>

#include "../../evenement/touche/touche.h"
#include "joueur.h"

void oki ()
{
 fprintf(stderr, " TEST OK\n");  
}

void test_joueurCreer_et_joueurDetruire ()
{
  fprintf(stderr, "*** Test de joueurCreer_et_joueurDetruire =");
  Joueur* joueur;
  joueur = joueurCreer();
  assert(joueur != NULL);
  
  joueurDetruire(joueur);   
     
  oki ();
}

void test_joueurEcrireX ()
{
  fprintf(stderr, "*** Test de joueurEcrireX =");
  Joueur* joueur;
  joueur = joueurCreer();
  
  joueurEcrireX(joueur, 12);
  assert(joueur->x == 12);
  joueurEcrireX(joueur, 13);
  assert(joueur->x == 13);
  
  joueurDetruire(joueur);   
     
  oki ();
}

void test_joueurEcrireY ()
{
  fprintf(stderr, "*** Test de joueurEcrireY =");
  Joueur* joueur;
  joueur = joueurCreer();
  
  joueurEcrireY(joueur, 12);
  assert(joueur->y == 12);
  joueurEcrireY(joueur, 13);
  assert(joueur->y == 13);
  
  joueurDetruire(joueur);   
     
  oki ();
}


void test_joueurLireX ()
{
  fprintf(stderr, "*** Test de joueurLireY =");
  Joueur* joueur;
  int i;

  joueur = joueurCreer();
  
  joueurEcrireX(joueur, 12);
  i = joueurLireX(joueur) ;
  assert(i == 12);
  joueurEcrireX(joueur, 13);
  i = joueurLireX(joueur) ;
  assert(i == 13);
  
  joueurDetruire(joueur);   
     
  oki ();
}



void test_joueurLireY ()
{
  fprintf(stderr, "*** Test de joueurLireY =");
  Joueur* joueur;
  int i;
  joueur = joueurCreer();
  
  joueurEcrireY(joueur, 12);
  i = joueurLireY(joueur) ;
  assert(i == 12);
  joueurEcrireY(joueur, 13);
  i = joueurLireY(joueur) ;
  assert(i == 13);
  
  joueurDetruire(joueur);   
     
  oki ();
}

void test_joueurEcrireDirection ()
{
  fprintf(stderr, "*** Test de joueurEcrireDirection =");
  Joueur* joueur;
  joueur = joueurCreer();
  
  joueurEcrireDirection(joueur, JOUEUR_DIRECTION_HAUT);
  assert(joueur->direction == JOUEUR_DIRECTION_HAUT);
  joueurEcrireDirection(joueur, JOUEUR_DIRECTION_BOUGE_PAS);
  assert(joueur->direction == JOUEUR_DIRECTION_BOUGE_PAS);

  
  joueurDetruire(joueur);   
     
  oki ();  
     
     
     
}

void test_joueurLireDirection ()
{
  fprintf(stderr, "*** Test de joueurLireDirection =");
  Joueur* joueur;
  joueur = joueurCreer();
  Joueur_direction direction;
  joueurEcrireDirection(joueur, JOUEUR_DIRECTION_HAUT);
  direction = joueurLireDirection(joueur);
  assert(direction == JOUEUR_DIRECTION_HAUT);
  joueurEcrireDirection(joueur, JOUEUR_DIRECTION_BOUGE_PAS);
  direction = joueurLireDirection(joueur);
  assert(direction == JOUEUR_DIRECTION_BOUGE_PAS);

  
  joueurDetruire(joueur);   
     
  oki ();  
     
}


/**************************************/

void test_joueurEcrireVie ()
{
  fprintf(stderr, "*** Test de joueurLireVie =");
  Joueur* joueur;
  joueur = joueurCreer();
  
  joueurEcrireVie(joueur, 12);
  assert(joueur->vie == 12);
  joueurEcrireVie(joueur, 14);
  assert(joueur->vie == 14);
  joueurEcrireVie(joueur, -1);
  assert(joueur->vie == -1);
  
  joueurDetruire(joueur);   
     
  oki ();  
     
     
     
}

void test_joueurLireVie()
{
  fprintf(stderr, "*** Test de joueurEcrireVie =");
  Joueur* joueur;
  int vie;
  joueur = joueurCreer();
  
  joueurEcrireVie(joueur, 12);
  vie = joueurLireVie(joueur);
  assert(vie == 12);
  joueurEcrireVie(joueur, 14);
  vie = joueurLireVie(joueur);
  assert(vie == 14);
  joueurEcrireVie(joueur, -1);
  vie = joueurLireVie(joueur);
  assert(vie == -1);
  
  joueurDetruire(joueur);   
     
  oki ();  
     
}



/**************************************/

void test_joueurEcrireDynamite ()
{
  fprintf(stderr, "*** Test de joueurLireDynamite =");
  Joueur* joueur;
  joueur = joueurCreer();
  
  joueurEcrireDynamite(joueur, 12);
  assert(joueur->dynamite == 12);
  joueurEcrireDynamite(joueur, 14);
  assert(joueur->dynamite == 14);
  joueurEcrireDynamite(joueur, -1);
  assert(joueur->dynamite == -1);
  
  joueurDetruire(joueur);   
     
  oki ();  
     
     
     
}

void test_joueurLireDynamite()
{
  fprintf(stderr, "*** Test de joueurEcrireDynamite =");
  Joueur* joueur;
  int vie;
  joueur = joueurCreer();
  
  joueurEcrireDynamite(joueur, 12);
  vie = joueurLireDynamite(joueur);
  assert(vie == 12);
  joueurEcrireDynamite(joueur, 14);
  vie = joueurLireDynamite(joueur);
  assert(vie == 14);
  joueurEcrireDynamite(joueur, -1);
  vie = joueurLireDynamite(joueur);
  assert(vie == -1);
  
  joueurDetruire(joueur);   
     
  oki ();  
     
}


/**************************************/

void test_joueurEcrireDiamant()
{
  fprintf(stderr, "*** Test de joueurLireDiamant =");
  Joueur* joueur;
  joueur = joueurCreer();
  
  joueurEcrireDiamant(joueur, 12);
  assert(joueur->diamant == 12);
  joueurEcrireDiamant(joueur, 14);
  assert(joueur->diamant == 14);
  joueurEcrireDiamant(joueur, -1);
  assert(joueur->diamant == -1);
  
  joueurDetruire(joueur);   
     
  oki ();  
     
     
     
}

void test_joueurLireDiamant()
{
  fprintf(stderr, "*** Test de joueurEcrireDiamant =");
  Joueur* joueur;
  int diamant;
  joueur = joueurCreer();
  
  joueurEcrireDiamant(joueur, 12);
  diamant = joueurLireDiamant (joueur);
  assert(diamant == 12);
  joueurEcrireDiamant(joueur, 14);
  diamant = joueurLireDiamant(joueur);
  assert(diamant == 14);
  joueurEcrireDiamant(joueur, -1);
  diamant = joueurLireDiamant(joueur);
  assert(diamant == -1);
  
  joueurDetruire(joueur);   
     
  oki ();  
     
}



/**************************************/

void test_joueurEcrireCompteurPousser()
{
  fprintf(stderr, "*** Test de joueurLireCompteurPousser =");
  Joueur* joueur;
  joueur = joueurCreer();
  
  joueurEcrireCompteurPousser(joueur, 12);
  assert(joueur->compteur_pousser == 12);
  joueurEcrireCompteurPousser(joueur, 14);
  assert(joueur->compteur_pousser == 14);
  joueurEcrireCompteurPousser(joueur, -1);
  assert(joueur->compteur_pousser == -1);
  
  joueurDetruire(joueur);   
     
  oki ();  
     
     
     
}

void test_joueurLireCompteurPousser()
{
  fprintf(stderr, "*** Test de joueurEcrireCompteurPousser =");
  Joueur* joueur;
  int compteur_pousser;
  joueur = joueurCreer();
  
  joueurEcrireCompteurPousser(joueur, 12);
  compteur_pousser = joueurLireCompteurPousser(joueur);
  assert(compteur_pousser == 12);
  joueurEcrireCompteurPousser(joueur, 14);
  compteur_pousser = joueurLireCompteurPousser(joueur);
  assert(compteur_pousser == 14);
  joueurEcrireCompteurPousser(joueur, -1);
  compteur_pousser = joueurLireCompteurPousser(joueur);
  assert(compteur_pousser == -1);
  
  joueurDetruire(joueur);   
     
  oki ();  
     
}


/**************************************/

void test_joueurEcrireCompteurTransformation()
{
  fprintf(stderr, "*** Test de joueurLireCompteurTransformation =");
  Joueur* joueur;
  joueur = joueurCreer();
  
  joueurEcrireCompteurTransformation(joueur, 12);
  assert(joueur->compteur_transformation == 12);
  joueurEcrireCompteurTransformation(joueur, 14);
  assert(joueur->compteur_transformation == 14);
  joueurEcrireCompteurTransformation(joueur, -1);
  assert(joueur->compteur_transformation == -1);
  
  joueurDetruire(joueur);   
     
  oki ();  
     
     
     
}

void test_joueurLireCompteurTransformation()
{
  fprintf(stderr, "*** Test de joueurEcrireCompteurTransformation =");
  Joueur* joueur;
  int compteur_transformation;
  joueur = joueurCreer();
  
  joueurEcrireCompteurTransformation(joueur, 12);
  compteur_transformation = joueurLireCompteurTransformation(joueur);
  assert(compteur_transformation == 12);
  joueurEcrireCompteurTransformation(joueur, 14);
  compteur_transformation = joueurLireCompteurTransformation(joueur);
  assert(compteur_transformation == 14);
  joueurEcrireCompteurTransformation(joueur, -1);
  compteur_transformation = joueurLireCompteurTransformation(joueur);
  assert(compteur_transformation == -1);
  
  joueurDetruire(joueur);   
     
  oki ();  
     
}



/**************************************/

void test_joueurEcrireActif()
{
  fprintf(stderr, "*** Test de joueurLireactif =");
  Joueur* joueur;
  joueur = joueurCreer();
  
  joueurEcrireActif(joueur, 12);
  assert(joueur->actif == 12);
  joueurEcrireActif(joueur, 14);
  assert(joueur->actif == 14);
  joueurEcrireActif(joueur, -1);
  assert(joueur->actif == -1);
  
  joueurDetruire(joueur);   
     
  oki ();  
     
     
     
}

void test_joueurLireActif()
{
  fprintf(stderr, "*** Test de joueurEcrireactif =");
  Joueur* joueur;
  short actif;
  joueur = joueurCreer();
  
  joueurEcrireActif(joueur, 12);
  actif = joueurLireActif (joueur);
  assert(actif == 12);
  joueurEcrireActif(joueur, 14);
  actif = joueurLireActif(joueur);
  assert(actif == 14);
  joueurEcrireActif(joueur, -1);
  actif = joueurLireActif(joueur);
  assert(actif == -1);
  
  joueurDetruire(joueur);   
     
  oki ();  
     
}



/**************************************/

void test_joueurEcrireEnTrainDePousser()
{
  fprintf(stderr, "*** Test de joueurEcrireEnTrainDePousser =");
  Joueur* joueur;
  joueur = joueurCreer();
  
  joueurEcrireEnTrainDePousser(joueur, 12);
  assert(joueur->en_train_de_pousser == 12);
  joueurEcrireEnTrainDePousser(joueur, 14);
  assert(joueur->en_train_de_pousser == 14);
  joueurEcrireEnTrainDePousser(joueur, -1);
  assert(joueur->en_train_de_pousser == -1);
  
  joueurDetruire(joueur);   
     
  oki ();  
     
     
     
}

void test_joueurLireEnTrainDePousser()
{
  fprintf(stderr, "*** Test de joueurLireEnTrainDePousser =");
  Joueur* joueur;
  int diamant;
  joueur = joueurCreer();
  
  joueurEcrireEnTrainDePousser(joueur, 12);
  diamant = joueurLireEnTrainDePousser (joueur);
  assert(diamant == 12);
  joueurEcrireEnTrainDePousser(joueur, 14);
  diamant = joueurLireEnTrainDePousser(joueur);
  assert(diamant == 14);
  joueurEcrireEnTrainDePousser(joueur, -1);
  diamant = joueurLireEnTrainDePousser(joueur);
  assert(diamant == -1);
  
  joueurDetruire(joueur);   
     
  oki ();  
     
}

/**************************************/

void test_joueurEcrireForme()
{
  fprintf(stderr, "*** Test de joueurEcrireForme =");
  Joueur* joueur;
  joueur = joueurCreer();
  
  joueurEcrireForme(joueur, JOUEUR_FORME_BOULDER);
  assert(joueur->forme == JOUEUR_FORME_BOULDER);
 
  joueurDetruire(joueur);   
     
  oki ();  
     
     
     
}

void test_joueurLireForme()
{
  fprintf(stderr, "*** Test de joueurLireForme =");
  Joueur* joueur;
  Joueur_forme forme;
  joueur = joueurCreer();
  
  joueurEcrireForme(joueur, JOUEUR_FORME_BOULDER);
  forme = joueurLireForme (joueur);
  assert(forme == JOUEUR_FORME_BOULDER);

  
  joueurDetruire(joueur);   
     
  oki ();  
     
}


int main ()
{
    
   fprintf(stderr, "=====Test du module JOUEUR=====\n");  
   fprintf(stderr,"* Test de joueurCreer et joueurDetruire\n"); 
    
   test_joueurCreer_et_joueurDetruire ();
   
 /*  fprintf(stderr,"* Test de joueurEcrireX\n");*/
   test_joueurEcrireX ();
   
 /*  fprintf(stderr,"* Test de joueurEcrireY\n");*/
   test_joueurEcrireY ();
    
  /* fprintf(stderr,"* Test de joueurLireX\n");*/
   test_joueurEcrireX ();
   
 /*  fprintf(stderr,"* Test de joueurlireY\n");*/
   test_joueurEcrireY (); 
    
 /*  fprintf(stderr,"* Test de joueurEcrireDirection\n");*/
   test_joueurEcrireDirection ();
   
/*   fprintf(stderr,"* Test de joueurLireDirection\n");*/
   test_joueurLireDirection ();
 /*  fprintf(stderr,"* Test de joueurLireDiamant\n");*/
 test_joueurLireDiamant ();
 /*   fprintf(stderr,"* Test de joueurEcrireDiamant\n");*/
 test_joueurEcrireDiamant ();

  /* fprintf(stderr,"* Test de joueurLireDynamite\n");*/
 test_joueurLireDynamite ();
  /*  fprintf(stderr,"* Test de joueurEcrireDynamite\n");*/
 test_joueurEcrireDynamite ();
   
   /*  fprintf(stderr,"* Test de joueurEcrireVie\n");*/
     
   test_joueurEcrireVie ();
 /* fprintf(stderr,"* Test de joueurLireVie\n");*/
     
   test_joueurLireVie (); 
 
   /*   fprintf(stderr,"* Test de joueurEcrireCompteurTransformation\n");*/
   test_joueurEcrireCompteurTransformation ();
/*  fprintf(stderr,"* Test de joueurLireCompteurTransformation\n");   */ 
   test_joueurLireCompteurTransformation(); 
   
 /*  fprintf(stderr,"* Test de joueurLireActif\n");     */
    test_joueurLireActif();
 /*  fprintf(stderr,"* Test de joueurEcrireActif\n");     */
    test_joueurEcrireActif();
   test_joueurLireEnTrainDePousser();
   test_joueurEcrireEnTrainDePousser();
   
   test_joueurLireCompteurPousser();
   test_joueurEcrireCompteurPousser();
   
   test_joueurEcrireForme();
   test_joueurLireForme();
   
   fprintf(stderr, "TOUT OK\n\n"); 
   return 0; 
}
