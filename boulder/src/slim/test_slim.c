#include <stdio.h>
#include <assert.h>

#include "slim.h"


void oki () { fprintf(stderr," = TEST OK\n");}

void test_slimEcrireCompteurGraphique ()
{
     fprintf(stderr, "* slimEcrireCompteurGraphique ");
     Slim mur;

     
     slimEcrireCompteurGraphique(&mur, 12);
     assert(mur.compteur_graphique == 12);
     oki ();
}



void test_slimLireCompteurGraphique ()
{
     fprintf(stderr, "* slimLireCompteurGraphique ");
     Slim mur;
     int i;
     
     slimEcrireCompteurGraphique(&mur, 12);
     i= slimLireCompteurGraphique(&mur);     
     assert(i==12);
     oki ();
}


void test_slimEcrireType ()
{
     fprintf(stderr, "* slimEcrireType ");
     Slim mur;
     
     slimEcrireType(&mur, SLIM_TYPE_DIAMANT);
     assert(mur.type == SLIM_TYPE_DIAMANT);
     oki ();
}



void test_slimLireType ()
{
     fprintf(stderr, "* slimLireType ");
     Slim mur;
     Slim_type type;
     
     slimEcrireType(&mur, SLIM_TYPE_PIERRE);
     type= slimLireType(&mur);     
     assert(type==SLIM_TYPE_PIERRE);
     oki ();
}


void test_slimEcrireVitesse ()
{
     fprintf(stderr, "* slimEcrireVitesse ");
     Slim mur;

     
     slimEcrireVitesse(&mur, 12);
     assert(mur.vitesse == 12);
     oki ();
}



void test_slimLireVitesse ()
{
     fprintf(stderr, "* slimLireVitesse ");
     Slim mur;
     Slim_vitesse i;
     
     slimEcrireVitesse(&mur, 12);
     i= slimLireVitesse(&mur);     
     assert(i==12);
     oki ();
}

int main ()
{
    fprintf(stderr, "====TEST DU MODULE MUR_QUI_BOUGE=====\n");
    test_slimEcrireType ();
    test_slimLireType ();
    test_slimEcrireCompteurGraphique ();
    test_slimLireCompteurGraphique ();
    fprintf(stderr, "MODULE OK \n\n");
    return 0;
}

