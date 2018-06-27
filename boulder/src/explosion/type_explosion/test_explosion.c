#include "explosion.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void oki ()
{
 fprintf(stderr, " TEST OK\n");  
}

void test_explosionCreer_et_explosionDetruire ()
{
  fprintf(stderr, "*** Test de explosionCreer_et_explosionDetruire =");
  Explosion* explosion;
  explosion = explosionCreer();
  assert(explosion != NULL);
  
  explosionDetruire(explosion);   
     
  oki ();
}

void test_explosionEcrireX ()
{
  fprintf(stderr, "*** Test de explosionEcrireX =");
  Explosion* explosion;
  explosion = explosionCreer();
  
  explosionEcrireX(explosion, 12);
  assert(explosion->x == 12);
  explosionEcrireX(explosion, 13);
  assert(explosion->x == 13);
  
  explosionDetruire(explosion);   
     
  oki ();
}

void test_explosionEcrireY ()
{
  fprintf(stderr, "*** Test de explosionEcrireY =");
  Explosion* explosion;
  explosion = explosionCreer();
  
  explosionEcrireY(explosion, 12);
  assert(explosion->y == 12);
  explosionEcrireY(explosion, 13);
  assert(explosion->y == 13);
  
  explosionDetruire(explosion);   
     
  oki ();
}


void test_explosionLireX ()
{
  fprintf(stderr, "*** Test de explosionLireY =");
  Explosion* explosion;
  int i;

  explosion = explosionCreer();
  
  explosionEcrireX(explosion, 12);
  i = explosionLireX(explosion) ;
  assert(i == 12);
  explosionEcrireX(explosion, 13);
  i = explosionLireX(explosion) ;
  assert(i == 13);
  
  explosionDetruire(explosion);   
     
  oki ();
}



void test_explosionLireY ()
{
  fprintf(stderr, "*** Test de explosionLireY =");
  Explosion* explosion;
  int i;
  explosion = explosionCreer();
  
  explosionEcrireY(explosion, 12);
  i = explosionLireY(explosion) ;
  assert(i == 12);
  explosionEcrireY(explosion, 13);
  i = explosionLireY(explosion) ;
  assert(i == 13);
  
  explosionDetruire(explosion);   
     
  oki ();
}


void test_explosionLireEtat ()
{
  fprintf(stderr, "*** Test de explosionLireEtat =");
  Explosion* explosion;
  explosion = explosionCreer();
  
  explosionEcrireEtat(explosion, REGRESSION);
  assert(explosion->etat == REGRESSION);
  explosionEcrireEtat(explosion, PROGRESSION);
  assert(explosion->etat == PROGRESSION);

  
  explosionDetruire(explosion);   
     
  oki ();  
     
     
     
}






void test_explosionEcrireEtat ()
{
  fprintf(stderr, "*** Test de explosionEcrireEtat =");
  Explosion* explosion;
  Explosion_etat etat;
  explosion = explosionCreer();
  
  explosionEcrireEtat(explosion, PROGRESSION);
  etat = explosionLireEtat(explosion);
  assert(etat == PROGRESSION);
  explosionEcrireEtat(explosion, REGRESSION);
  etat = explosionLireEtat(explosion);
  assert(etat == REGRESSION);

  
  explosionDetruire(explosion);   
     
  oki ();  
     
     
     
}

/****************/
void test_explosionEcrireCompteur ()
{
  fprintf(stderr, "*** Test de explosionLireCompteur =");
  Explosion* explosion;
  explosion = explosionCreer();
  
  explosionEcrireCompteur(explosion, 12);
  assert(explosion->compteur == 12);
  explosionEcrireCompteur(explosion, 14);
  assert(explosion->compteur == 14);
  explosionEcrireCompteur(explosion, -1);
  assert(explosion->compteur == -1);
  
  explosionDetruire(explosion);   
     
  oki ();  
     
     
     
}




void test_explosionLireCompteur ()
{
  fprintf(stderr, "*** Test de explosionEcrireCompteur =");
  Explosion* explosion;
  int compteur;
  explosion = explosionCreer();
  
  explosionEcrireCompteur(explosion, 12);
  compteur = explosionLireCompteur(explosion);
  assert(compteur == 12);
  explosionEcrireCompteur(explosion, 14);
  compteur = explosionLireCompteur(explosion);
  assert(compteur == 14);
  explosionEcrireCompteur(explosion, -1);
  compteur = explosionLireCompteur(explosion);
  assert(compteur == -1);
  
  explosionDetruire(explosion);   
     
  oki ();  
     
     
     
}


/****************/
void test_explosionEcrireDuree ()
{
  fprintf(stderr, "*** Test de explosionLireDuree =");
  Explosion* explosion;
  explosion = explosionCreer();
  
  explosionEcrireDuree(explosion, 12);
  assert(explosion->duree == 12);
  explosionEcrireDuree(explosion, 14);
  assert(explosion->duree == 14);
  explosionEcrireDuree(explosion, -1);
  assert(explosion->duree == -1);
  
  explosionDetruire(explosion);   
     
  oki ();  
     
     
     
}




void test_explosionLireDuree ()
{
  fprintf(stderr, "*** Test de explosionEcrireDuree =");
  Explosion* explosion;
  int duree;
  explosion = explosionCreer();
  
  explosionEcrireDuree(explosion, 12);
  duree = explosionLireDuree(explosion);
  assert(duree == 12);
  explosionEcrireDuree(explosion, 14);
  duree = explosionLireDuree(explosion);
  assert(duree == 14);
  explosionEcrireDuree(explosion, -1);
  duree = explosionLireDuree(explosion);
  assert(duree == -1);
  
  explosionDetruire(explosion);   
     
  oki ();  
     
     
     
}



void test_explosionLireType ()
{
  fprintf(stderr, "*** Test de explosionLireType =");
  Explosion* explosion;
  explosion = explosionCreer();
  
  explosionEcrireType(explosion, EXPLOSION_TYPE_DIAMANT);
  assert(explosion->type == EXPLOSION_TYPE_DIAMANT);
  explosionEcrireType(explosion, EXPLOSION_TYPE_PIERRE);
  assert(explosion->type == EXPLOSION_TYPE_PIERRE);

  
  explosionDetruire(explosion);   
     
  oki ();  
     
     
     
}






void test_explosionEcrireType ()
{
  fprintf(stderr, "*** Test de explosionEcrireType =");
  Explosion* explosion;
  Explosion_type type;
  explosion = explosionCreer();
  
  explosionEcrireType(explosion, EXPLOSION_TYPE_DIAMANT);
  type = explosionLireType(explosion);
  assert(type ==EXPLOSION_TYPE_DIAMANT);
  explosionEcrireType(explosion, EXPLOSION_TYPE_PIERRE);
  type = explosionLireType(explosion);
  assert(type == EXPLOSION_TYPE_PIERRE);

  
  explosionDetruire(explosion);   
     
  oki ();  
     
     
     
}



int main ()
{
    
   fprintf(stderr, "=====Test du module EXPLOSION=====\n");  
   fprintf(stderr,"* Test de explosionCreer et explosionDetruire\n"); 
    
   test_explosionCreer_et_explosionDetruire ();
   
 
   test_explosionEcrireX ();
   

   test_explosionEcrireY ();
    

   test_explosionEcrireX ();
   

   test_explosionEcrireY (); 
    

   test_explosionLireEtat ();

   test_explosionEcrireEtat ();
   
     
   test_explosionEcrireDuree ();

     
   test_explosionLireDuree (); 
     
   test_explosionEcrireCompteur ();

     
   test_explosionLireCompteur (); 
   test_explosionEcrireType ();
   test_explosionLireType ();
   
   fprintf(stderr, "TOUT OK\n\n"); 
   return 0; 
}


