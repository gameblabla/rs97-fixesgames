#include "diamant.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void oki ()
{
 fprintf(stderr, " TEST OK\n");  
}

void test_diamantCreer_et_diamantDetruire ()
{
  fprintf(stderr, "*** Test de diamantCreer_et_diamantDetruire =");
  Diamant* diamant;
  diamant = diamantCreer();
  assert(diamant != NULL);
  
  diamantDetruire(diamant);   
     
  oki ();
}

void test_diamantEcrireX ()
{
  fprintf(stderr, "*** Test de diamantEcrireX =");
  Diamant* diamant;
  diamant = diamantCreer();
  
  diamantEcrireX(diamant, 12);
  assert(diamant->x == 12);
  diamantEcrireX(diamant, 13);
  assert(diamant->x == 13);
  
  diamantDetruire(diamant);   
     
  oki ();
}

void test_diamantEcrireY ()
{
  fprintf(stderr, "*** Test de diamantEcrireY =");
  Diamant* diamant;
  diamant = diamantCreer();
  
  diamantEcrireY(diamant, 12);
  assert(diamant->y == 12);
  diamantEcrireY(diamant, 13);
  assert(diamant->y == 13);
  
  diamantDetruire(diamant);   
     
  oki ();
}


void test_diamantLireX ()
{
  fprintf(stderr, "*** Test de diamantLireY =");
  Diamant* diamant;
  int i;

  diamant = diamantCreer();
  
  diamantEcrireX(diamant, 12);
  i = diamantLireX(diamant) ;
  assert(i == 12);
  diamantEcrireX(diamant, 13);
  i = diamantLireX(diamant) ;
  assert(i == 13);
  
  diamantDetruire(diamant);   
     
  oki ();
}



void test_diamantLireY ()
{
  fprintf(stderr, "*** Test de diamantLireY =");
  Diamant* diamant;
  int i;
  diamant = diamantCreer();
  
  diamantEcrireY(diamant, 12);
  i = diamantLireY(diamant) ;
  assert(i == 12);
  diamantEcrireY(diamant, 13);
  i = diamantLireY(diamant) ;
  assert(i == 13);
  
  diamantDetruire(diamant);   
     
  oki ();
}

void test_diamantEcrireDirection ()
{
  fprintf(stderr, "*** Test de diamantEcrireDirection =");
  Diamant* diamant;
  diamant = diamantCreer();
  
  diamantEcrireDirection(diamant, DIAMANT_DIRECTION_HAUT);
  assert(diamant->direction == DIAMANT_DIRECTION_HAUT);
  diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BOUGE_PAS);
  assert(diamant->direction == DIAMANT_DIRECTION_BOUGE_PAS);

  
  diamantDetruire(diamant);   
     
  oki ();  
     
     
     
}

void test_diamantLireDirection ()
{
  fprintf(stderr, "*** Test de diamantLireDirection =");
  Diamant* diamant;
  diamant = diamantCreer();
  Diamant_direction direction;
  diamantEcrireDirection(diamant, DIAMANT_DIRECTION_HAUT);
  direction = diamantLireDirection(diamant);
  assert(direction == DIAMANT_DIRECTION_HAUT);
  diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BOUGE_PAS);
  direction = diamantLireDirection(diamant);
  assert(direction == DIAMANT_DIRECTION_BOUGE_PAS);

  
  diamantDetruire(diamant);   
     
  oki ();  
     
}


void test_diamantLireTransformationChute ()
{
  fprintf(stderr, "*** Test de diamantLireTransformationChute =");
  Diamant* diamant;
  diamant = diamantCreer();
  
  diamantEcrireTransformationChute(diamant, DIAMANT_TRANSFORMATION_DIAMANT);
  assert(diamant->transformation == DIAMANT_TRANSFORMATION_DIAMANT);
  diamantEcrireTransformationChute(diamant, DIAMANT_TRANSFORMATION_EXPLOSION);
  assert(diamant->transformation == DIAMANT_TRANSFORMATION_EXPLOSION);

  
  diamantDetruire(diamant);   
     
  oki ();  
     
     
     
}






void test_diamantEcrireTransformationChute ()
{
  fprintf(stderr, "*** Test de diamantEcrireTransformationChute =");
  Diamant* diamant;
  Diamant_transformation_chute chute;
  diamant = diamantCreer();
  
  diamantEcrireTransformationChute(diamant, DIAMANT_TRANSFORMATION_DIAMANT);
  chute = diamantLireTransformationChute(diamant);
  assert(chute == DIAMANT_TRANSFORMATION_DIAMANT);
  diamantEcrireTransformationChute(diamant, DIAMANT_TRANSFORMATION_EXPLOSION);
  chute = diamantLireTransformationChute(diamant);
  assert(chute == DIAMANT_TRANSFORMATION_EXPLOSION);

  
  diamantDetruire(diamant);   
     
  oki ();  
     
     
     
}

void test_diamantEcrireValeur ()
{
  fprintf(stderr, "*** Test de diamantLireValeur =");
  Diamant* diamant;
  diamant = diamantCreer();
  
  diamantEcrireValeur(diamant, 12);
  assert(diamant->valeur == 12);
  diamantEcrireValeur(diamant, 14);
  assert(diamant->valeur == 14);
  diamantEcrireValeur(diamant, -1);
  assert(diamant->valeur == -1);
  
  diamantDetruire(diamant);   
     
  oki ();  
     
     
     
}




void test_diamantLireValeur ()
{
  fprintf(stderr, "*** Test de diamantEcrireValeur =");
  Diamant* diamant;
  int valeur;
  diamant = diamantCreer();
  
  diamantEcrireValeur(diamant, 12);
  valeur = diamantLireValeur(diamant);
  assert(valeur == 12);
  diamantEcrireValeur(diamant, 14);
  valeur = diamantLireValeur(diamant);
  assert(valeur == 14);
  diamantEcrireValeur(diamant, -1);
  valeur = diamantLireValeur(diamant);
  assert(valeur == -1);
  
  diamantDetruire(diamant);   
     
  oki ();  
     
     
     
}


int main ()
{
    
   fprintf(stderr, "=====Test du module DIAMANT=====\n");  
   fprintf(stderr,"* Test de diamantCreer et diamantDetruire\n"); 
    
   test_diamantCreer_et_diamantDetruire ();
   
   fprintf(stderr,"* Test de diamantEcrireX\n");
   test_diamantEcrireX ();
   
   fprintf(stderr,"* Test de diamantEcrireY\n");
   test_diamantEcrireY ();
    
   fprintf(stderr,"* Test de diamantLireX\n");
   test_diamantEcrireX ();
   
   fprintf(stderr,"* Test de diamantlireY\n");
   test_diamantEcrireY (); 
    
   fprintf(stderr,"* Test de diamantEcrireDirection\n");
   test_diamantEcrireDirection ();
   
   fprintf(stderr,"* Test de diamantLireDirection\n");
   test_diamantLireDirection ();
   fprintf(stderr,"* Test de diamantLireTransformationChute\n");
   test_diamantLireTransformationChute ();
    fprintf(stderr,"* Test de diamantEcrireTransformationChute\n");
   test_diamantEcrireTransformationChute ();
   
     fprintf(stderr,"* Test de diamantEcrirevaleur\n");
     
   test_diamantEcrireValeur ();
  fprintf(stderr,"* Test de diamantLirevaleur\n");
     
   test_diamantLireValeur (); 
   
   
   fprintf(stderr, "TOUT OK\n\n"); 
   return 0; 
}
