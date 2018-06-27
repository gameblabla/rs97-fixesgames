#include "ennemi.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void oki ()
{
 fprintf(stderr, " TEST OK\n");  
}

void test_ennemiCreer_et_ennemiDetruire ()
{
  fprintf(stderr, "*** Test de ennemiCreer_et_ennemiDetruire =");
  Ennemi* ennemi;
  ennemi = ennemiCreer();
  assert(ennemi != NULL);
  
  ennemiDetruire(ennemi);   
     
  oki ();
}

void test_ennemiEcrireX ()
{
  fprintf(stderr, "*** Test de ennemiEcrireX =");
  Ennemi* ennemi;
  ennemi = ennemiCreer();
  
  ennemiEcrireX(ennemi, 12);
  assert(ennemi->x == 12);
  ennemiEcrireX(ennemi, 13);
  assert(ennemi->x == 13);
  
  ennemiDetruire(ennemi);   
     
  oki ();
}

void test_ennemiEcrireY ()
{
  fprintf(stderr, "*** Test de ennemiEcrireY =");
  Ennemi* ennemi;
  ennemi = ennemiCreer();
  
  ennemiEcrireY(ennemi, 12);
  assert(ennemi->y == 12);
  ennemiEcrireY(ennemi, 13);
  assert(ennemi->y == 13);
  
  ennemiDetruire(ennemi);   
     
  oki ();
}


void test_ennemiLireX ()
{
  fprintf(stderr, "*** Test de ennemiLireY =");
  Ennemi* ennemi;
  int i;

  ennemi = ennemiCreer();
  
  ennemiEcrireX(ennemi, 12);
  i = ennemiLireX(ennemi) ;
  assert(i == 12);
  ennemiEcrireX(ennemi, 13);
  i = ennemiLireX(ennemi) ;
  assert(i == 13);
  
  ennemiDetruire(ennemi);   
     
  oki ();
}



void test_ennemiLireY ()
{
  fprintf(stderr, "*** Test de ennemiLireY =");
  Ennemi* ennemi;
  int i;
  ennemi = ennemiCreer();
  
  ennemiEcrireY(ennemi, 12);
  i = ennemiLireY(ennemi) ;
  assert(i == 12);
  ennemiEcrireY(ennemi, 13);
  i = ennemiLireY(ennemi) ;
  assert(i == 13);
  
  ennemiDetruire(ennemi);   
     
  oki ();
}

void test_ennemiEcrireDirection ()
{
  fprintf(stderr, "*** Test de ennemiEcrireDirection =");
  Ennemi* ennemi;
  ennemi = ennemiCreer();
  
  ennemiEcrireDirection(ennemi, ENNEMI_DIRECTION_HAUT);
  assert(ennemi->direction == ENNEMI_DIRECTION_HAUT);
  ennemiEcrireDirection(ennemi, ENNEMI_DIRECTION_BOUGE_PAS);
  assert(ennemi->direction == ENNEMI_DIRECTION_BOUGE_PAS);

  
  ennemiDetruire(ennemi);   
     
  oki ();  
     
     
     
}

void test_ennemiLireDirection ()
{
  fprintf(stderr, "*** Test de ennemiLireDirection =");
  Ennemi* ennemi;
  ennemi = ennemiCreer();
  Ennemi_direction direction;
  ennemiEcrireDirection(ennemi, ENNEMI_DIRECTION_HAUT);
  direction = ennemiLireDirection(ennemi);
  assert(direction == ENNEMI_DIRECTION_HAUT);
  ennemiEcrireDirection(ennemi, ENNEMI_DIRECTION_BOUGE_PAS);
  direction = ennemiLireDirection(ennemi);
  assert(direction == ENNEMI_DIRECTION_BOUGE_PAS);

  
  ennemiDetruire(ennemi);   
     
  oki ();  
     
}






void test_ennemiEcrireType ()
{
  fprintf(stderr, "*** Test de ennemiLireType =");
  Ennemi* ennemi;
  ennemi = ennemiCreer();
  
  ennemiEcrireType(ennemi, ENNEMI_TYPE_DIAMANT);
  assert(ennemi->type == ENNEMI_TYPE_DIAMANT);
  ennemiEcrireType(ennemi, ENNEMI_TYPE_RIEN);
  assert(ennemi->type == ENNEMI_TYPE_RIEN);
  ennemiEcrireType(ennemi, ENNEMI_TYPE_PIERRE);
  assert(ennemi->type == ENNEMI_TYPE_PIERRE);
  
  ennemiDetruire(ennemi);   
     
  oki ();  
     
     
     
}




void test_ennemiLireType ()
{
  fprintf(stderr, "*** Test de ennemiEcrireType =");
  Ennemi* ennemi;
  int type;
  ennemi = ennemiCreer();
  
  ennemiEcrireType(ennemi, ENNEMI_TYPE_DIAMANT);
  type = ennemiLireType(ennemi);
  assert(type == ENNEMI_TYPE_DIAMANT);
  ennemiEcrireType(ennemi, ENNEMI_TYPE_PIERRE);
  type = ennemiLireType(ennemi);
  assert(type ==ENNEMI_TYPE_PIERRE);
  ennemiEcrireType(ennemi, ENNEMI_TYPE_RIEN);
  type = ennemiLireType(ennemi);
  assert(type == ENNEMI_TYPE_RIEN);
  
  ennemiDetruire(ennemi);   
     
  oki ();  
     
     
     
}


int main ()
{
    
   fprintf(stderr, "=====Test du module ENNEMI=====\n");  
   fprintf(stderr,"* Test de ennemiCreer et ennemiDetruire\n"); 
    
   test_ennemiCreer_et_ennemiDetruire ();
   
   fprintf(stderr,"* Test de ennemiEcrireX\n");
   test_ennemiEcrireX ();
   
   fprintf(stderr,"* Test de ennemiEcrireY\n");
   test_ennemiEcrireY ();
    
   fprintf(stderr,"* Test de ennemiLireX\n");
   test_ennemiEcrireX ();
   
   fprintf(stderr,"* Test de ennemilireY\n");
   test_ennemiEcrireY (); 
    
   fprintf(stderr,"* Test de ennemiEcrireDirection\n");
   test_ennemiEcrireDirection ();
   
   fprintf(stderr,"* Test de ennemiLireDirection\n");
   test_ennemiLireDirection ();
  /* fprintf(stderr,"* Test de ennemiLireTransformationChute\n");
   test_ennemiLireTransformationChute ();
    fprintf(stderr,"* Test de ennemiEcrireTransformationChute\n");
   test_ennemiEcrireTransformationChute ();
   */
     fprintf(stderr,"* Test de ennemiEcrireType\n");
     
   test_ennemiEcrireType ();
  fprintf(stderr,"* Test de ennemiLireType\n");
     
   test_ennemiLireType (); 
   
   
   fprintf(stderr, "TOUT OK\n\n"); 
   return 0; 
}
