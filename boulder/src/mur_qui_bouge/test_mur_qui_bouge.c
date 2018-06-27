#include <stdio.h>
#include <assert.h>

#include "mur_qui_bouge.h"


void oki () { fprintf(stderr," = TEST OK\n");}

void test_mur_qui_bougeEcrireCompteurGraphique ()
{
     fprintf(stderr, "* mur_qui_bougeEcrireCompteurGraphique ");
     Mur_qui_bouge mur;

     
     mur_qui_bougeEcrireCompteurGraphique(&mur, 12);
     assert(mur.compteur_graphique == 12);
     oki ();
}



void test_mur_qui_bougeLireCompteurGraphique ()
{
     fprintf(stderr, "* mur_qui_bougeLireCompteurGraphique ");
     Mur_qui_bouge mur;
     int i;
     
     mur_qui_bougeEcrireCompteurGraphique(&mur, 12);
     i= mur_qui_bougeLireCompteurGraphique(&mur);     
     assert(i==12);
     oki ();
}


void test_mur_qui_bougeEcrireEtat ()
{
     fprintf(stderr, "* mur_qui_bougeEcrireEtat ");
     Mur_qui_bouge mur;
     
     mur_qui_bougeEcrireEtat(&mur, MUR_QUI_BOUGE_ETAT_EN_COURS);
     assert(mur.etat == MUR_QUI_BOUGE_ETAT_EN_COURS);
     oki ();
}



void test_mur_qui_bougeLireEtat ()
{
     fprintf(stderr, "* mur_qui_bougeLireEtat ");
     Mur_qui_bouge mur;
     Mur_qui_bouge_etat etat;
     
     mur_qui_bougeEcrireEtat(&mur, MUR_QUI_BOUGE_ETAT_ABSENT);
     etat= mur_qui_bougeLireEtat(&mur);     
     assert(etat==MUR_QUI_BOUGE_ETAT_ABSENT);
     oki ();
}


void test_mur_qui_bougeEcrireTour ()
{
     fprintf(stderr, "* mur_qui_bougeEcrireTour ");
     Mur_qui_bouge mur;

     
     mur_qui_bougeEcrireTour(&mur, 12);
     assert(mur.tour_de_boucle_restant == 12);
     oki ();
}

/*

void test_mur_qui_bougeLireTour ()
{
     fprintf(stderr, "* mur_qui_bougeLireTour ");
     Mur_qui_bouge mur;
     int i;
     
     mur_qui_bougeEcrireTour(&mur, 12);
     i= mur_qui_bougeLireTour(&mur);     
     assert(i==12);
     oki ();
}
*/

int main ()
{
    fprintf(stderr, "====TEST DU MODULE MUR_QUI_BOUGE=====\n");
    test_mur_qui_bougeEcrireEtat ();
    test_mur_qui_bougeLireEtat ();
    test_mur_qui_bougeEcrireCompteurGraphique ();
    test_mur_qui_bougeLireCompteurGraphique ();
    fprintf(stderr, "MODULE OK \n\n");
    return 0;
}

