#include "pierre.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void oki ()
{
 fprintf(stderr, " TEST OK\n");  
}

void test_pierreCreer_et_pierreDetruire ()
{
  fprintf(stderr, "*** Test de pierreCreer_et_pierreDetruire =");
  Pierre* pierre;
  pierre = pierreCreer();
  assert(pierre != NULL);
  
  pierreDetruire(pierre);   
     
  oki ();
}

void test_pierreEcrireX ()
{
  fprintf(stderr, "*** Test de pierreEcrireX =");
  Pierre* pierre;
  pierre = pierreCreer();
  
  pierreEcrireX(pierre, 12);
  assert(pierre->x == 12);
  pierreEcrireX(pierre, 13);
  assert(pierre->x == 13);
  
  pierreDetruire(pierre);   
     
  oki ();
}

void test_pierreEcrireY ()
{
  fprintf(stderr, "*** Test de pierreEcrireY =");
  Pierre* pierre;
  pierre = pierreCreer();
  
  pierreEcrireY(pierre, 12);
  assert(pierre->y == 12);
  pierreEcrireY(pierre, 13);
  assert(pierre->y == 13);
  
  pierreDetruire(pierre);   
     
  oki ();
}


void test_pierreLireX ()
{
  fprintf(stderr, "*** Test de pierreLireY =");
  Pierre* pierre;
  int i;

  pierre = pierreCreer();
  
  pierreEcrireX(pierre, 12);
  i = pierreLireX(pierre) ;
  assert(i == 12);
  pierreEcrireX(pierre, 13);
  i = pierreLireX(pierre) ;
  assert(i == 13);
  
  pierreDetruire(pierre);   
     
  oki ();
}



void test_pierreLireY ()
{
  fprintf(stderr, "*** Test de pierreLireY =");
  Pierre* pierre;
  int i;
  pierre = pierreCreer();
  
  pierreEcrireY(pierre, 12);
  i = pierreLireY(pierre) ;
  assert(i == 12);
  pierreEcrireY(pierre, 13);
  i = pierreLireY(pierre) ;
  assert(i == 13);
  
  pierreDetruire(pierre);   
     
  oki ();
}

void test_pierreEcrireDirection ()
{
  fprintf(stderr, "*** Test de pierreEcrireDirection =");
  Pierre* pierre;
  pierre = pierreCreer();
  
  pierreEcrireDirection(pierre, PIERRE_DIRECTION_HAUT);
  assert(pierre->direction == PIERRE_DIRECTION_HAUT);
  pierreEcrireDirection(pierre, PIERRE_DIRECTION_BOUGE_PAS);
  assert(pierre->direction == PIERRE_DIRECTION_BOUGE_PAS);

  
  pierreDetruire(pierre);   
     
  oki ();  
     
     
     
}

void test_pierreLireDirection ()
{
  fprintf(stderr, "*** Test de pierreLireDirection =");
  Pierre* pierre;
  pierre = pierreCreer();
  Pierre_direction direction;
  pierreEcrireDirection(pierre, PIERRE_DIRECTION_HAUT);
  direction = pierreLireDirection(pierre);
  assert(direction == PIERRE_DIRECTION_HAUT);
  pierreEcrireDirection(pierre, PIERRE_DIRECTION_BOUGE_PAS);
  direction = pierreLireDirection(pierre);
  assert(direction == PIERRE_DIRECTION_BOUGE_PAS);

  
  pierreDetruire(pierre);   
     
  oki ();  
     
}


void test_pierreLireTransformationChute ()
{
  fprintf(stderr, "*** Test de pierreLireType =");
  Pierre* pierre;
  pierre = pierreCreer();
  
  pierreEcrireType(pierre, PIERRE_TYPE_PIERRE);
  assert(pierre->type == PIERRE_TYPE_PIERRE);
  pierreEcrireType(pierre, PIERRE_TYPE_DIAMANT);
  assert(pierre->type == PIERRE_TYPE_DIAMANT);
  pierreEcrireType(pierre, PIERRE_TYPE_EXPLOSION);
  assert(pierre->type == PIERRE_TYPE_EXPLOSION);
  
  pierreDetruire(pierre);   
     
  oki ();  
     
     
     
}






void test_pierreEcrireTransformationChute ()
{
  fprintf(stderr, "*** Test de pierreEcrireType =");
  Pierre* pierre;
  Pierre_type chute;
  pierre = pierreCreer();
  
  pierreEcrireType(pierre, PIERRE_TYPE_PIERRE);
  chute = pierreLireType(pierre);
  assert(chute == PIERRE_TYPE_PIERRE);
  pierreEcrireType(pierre, PIERRE_TYPE_DIAMANT);
  chute = pierreLireType(pierre);
  assert(chute == PIERRE_TYPE_DIAMANT);
  pierreEcrireType(pierre, PIERRE_TYPE_EXPLOSION);
  chute = pierreLireType(pierre);
  assert(chute == PIERRE_TYPE_EXPLOSION);
  
  pierreDetruire(pierre);   
     
  oki ();  
     
     
     
}

void test_pierreEcrirePoids ()
{
  fprintf(stderr, "*** Test de pierreLirePoids =");
  Pierre* pierre;
  pierre = pierreCreer();
  
  pierreEcrirePoids(pierre, 12);
  assert(pierre->poids == 12);
  pierreEcrirePoids(pierre, 14);
  assert(pierre->poids == 14);
  pierreEcrirePoids(pierre, -1);
  assert(pierre->poids == -1);
  
  pierreDetruire(pierre);   
     
  oki ();  
     
     
     
}




void test_pierreLirePoids ()
{
  fprintf(stderr, "*** Test de pierreEcrirePoids =");
  Pierre* pierre;
  int poids;
  pierre = pierreCreer();
  
  pierreEcrirePoids(pierre, 12);
  poids = pierreLirePoids(pierre);
  assert(poids == 12);
  pierreEcrirePoids(pierre, 14);
  poids = pierreLirePoids(pierre);
  assert(poids == 14);
  pierreEcrirePoids(pierre, -1);
  poids = pierreLirePoids(pierre);
  assert(poids == -1);
  
  pierreDetruire(pierre);   
     
  oki ();  
     
     
     
}


int main ()
{
    
   fprintf(stderr, "=====Test du module PIERRE=====\n");  
   fprintf(stderr,"* Test de pierreCreer et pierreDetruire\n"); 
    
   test_pierreCreer_et_pierreDetruire ();
   
   fprintf(stderr,"* Test de pierreEcrireX\n");
   test_pierreEcrireX ();
   
   fprintf(stderr,"* Test de pierreEcrireY\n");
   test_pierreEcrireY ();
    
   fprintf(stderr,"* Test de pierreLireX\n");
   test_pierreEcrireX ();
   
   fprintf(stderr,"* Test de pierrelireY\n");
   test_pierreEcrireY (); 
    
   fprintf(stderr,"* Test de pierreEcrireDirection\n");
   test_pierreEcrireDirection ();
   
   fprintf(stderr,"* Test de pierreLireDirection\n");
   test_pierreLireDirection ();
   fprintf(stderr,"* Test de pierreLireType\n");
   test_pierreLireTransformationChute ();
    fprintf(stderr,"* Test de pierreEcrireType\n");
   test_pierreEcrireTransformationChute ();
   
     fprintf(stderr,"* Test de pierreEcrirePoids\n");
     
   test_pierreEcrirePoids ();
  fprintf(stderr,"* Test de pierreLirePoids\n");
     
   test_pierreLirePoids (); 
   
   
   fprintf(stderr, "TOUT OK\n\n"); 
   return 0; 
}
