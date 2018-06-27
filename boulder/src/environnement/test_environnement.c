#include <stdio.h>
#include <assert.h>


#include "../niveau/niveau.h"
#include "environnement.h"
void oki () { fprintf(stderr, " = OK\n"); }

/*
 * PIERRE
 */

void test_environnementPierre_Vide()
{
 fprintf(stderr, "*** Test de environnementPierre_Vide");
 Niveau niveau;
 Pierre* pierre;
 
 niveauInitialiser(&niveau);
 niveauChargement(&niveau, "./test/testchargementpierre.txt");
 assert(niveauLireMatrice(&niveau, 2,2) == ELEMENT_PIERRE);
 assert(niveauLireMatrice(&niveau, 2,3) == ELEMENT_VIDE); 
 environnementEvoluerListePierre (&niveau);
 assert(niveauLireMatrice(&niveau, 2,2) == ELEMENT_VIDE);
 assert(niveauLireMatrice(&niveau, 2,3) == ELEMENT_PIERRE);
 pierre = liste_pierreLire(niveauLireListePierre(&niveau),1);
 assert(pierreLireX(pierre) == 2);
 assert(pierreLireY(pierre) == 3);
 
 environnementEvoluerListePierre (&niveau);
  
 assert(niveauLireMatrice(&niveau, 2,2) == ELEMENT_VIDE);
 assert(niveauLireMatrice(&niveau, 2,3) == ELEMENT_PIERRE);
 pierre = liste_pierreLire(niveauLireListePierre(&niveau),1);
 assert(pierreLireX(pierre) == 2);
 assert(pierreLireY(pierre) == 3);
 
 niveauDetruire(&niveau);     
  oki ();
}

void test_environnementPierre_Explosion()
{
 fprintf(stderr, "*** Test de environnementPierre_Explosion");
 Niveau niveau;
 Pierre* pierre;
 
 niveauInitialiser(&niveau);
 niveauChargement(&niveau, "./test/testchargementpierre_explosion.txt");
 assert(niveauLireMatrice(&niveau, 2,2) == ELEMENT_PIERRE);
 assert(niveauLireMatrice(&niveau, 2,3) == ELEMENT_EXPLOSION);
 assert(niveauLireMatrice(&niveau, 3,2) == ELEMENT_PIERRE);
 assert(niveauLireMatrice(&niveau, 3,3) == ELEMENT_VIDE);
  
 environnementEvoluerListePierre (&niveau);

 assert(niveauLireMatrice(&niveau, 2,2) == ELEMENT_PIERRE);
 assert(niveauLireMatrice(&niveau, 2,3) == ELEMENT_EXPLOSION);
 assert(niveauLireMatrice(&niveau, 3,2) == ELEMENT_VIDE);
 assert(niveauLireMatrice(&niveau, 3,3) == ELEMENT_PIERRE);
 assert(liste_pierreTaille(niveauLireListePierre(&niveau)) == 2);
 pierre = liste_pierreLire(niveauLireListePierre(&niveau),1);
 assert(pierreLireX(pierre) == 2);
 assert(pierreLireY(pierre) == 2);

  niveauDetruire(&niveau);     
  oki ();
}


/*
 * test explosion
 */

void test_environnementExplosion33_diamant()
{
 fprintf(stderr, "*** Test de environnementExplosion diamant");
 Niveau niveau;
 
 int i;
 
 niveauInitialiser(&niveau);
 niveauChargement(&niveau, "./test/testchargementexplosion_diamant.txt");
 environnementCreerExplosion33(&niveau, EXPLOSION_TYPE_DIAMANT, 2,2);
 for (i=1; i<=6;i++) {
 assert(niveauLireMatrice(&niveau, 2,2) == ELEMENT_EXPLOSION);
 assert(niveauLireMatrice(&niveau, 2,3) == ELEMENT_EXPLOSION);
 assert(niveauLireMatrice(&niveau, 3,3) == ELEMENT_EXPLOSION);
 assert(niveauLireMatrice(&niveau, 3,2) == ELEMENT_EXPLOSION);
 assert(niveauLireMatrice(&niveau, 1,1) == ELEMENT_BORD);
 assert(niveauLireMatrice(&niveau, 1,2) == ELEMENT_BORD);
 assert(niveauLireMatrice(&niveau, 2,1) == ELEMENT_BORD);
 assert(niveauLireMatrice(&niveau, 3,1) == ELEMENT_BORD);  
 assert(liste_explosionTaille(niveauLireListeExplosion(&niveau)) == 4);
/* fprintf(stderr, "   %d   ", explosionLireCompteur(
                         liste_explosionLire(niveauLireListeExplosion(&niveau),1)
                         ));
*/ environnementEvoluerListeExplosion (&niveau);
}
 assert(niveauLireMatrice(&niveau, 2,2) == ELEMENT_DIAMANT);
 assert(niveauLireMatrice(&niveau, 2,3) == ELEMENT_DIAMANT);
 assert(niveauLireMatrice(&niveau, 3,3) == ELEMENT_DIAMANT);
 assert(niveauLireMatrice(&niveau, 3,2) == ELEMENT_DIAMANT);
 assert(liste_diamantTaille(niveauLireListeDiamant(&niveau)) == 4);
  niveauDetruire(&niveau);     
  oki ();
}

 void test_environnementExplosion33_vide()
{
 fprintf(stderr, "*** Test de environnementExplosion vide");
 Niveau niveau;
 
 int i;
 
 niveauInitialiser(&niveau);
 niveauChargement(&niveau, "./test/testchargementexplosion_vide.txt");
 environnementCreerExplosion33(&niveau, EXPLOSION_TYPE_RIEN, 2,2);
 for (i=1; i<=6;i++) {
 assert(niveauLireMatrice(&niveau, 2,2) == ELEMENT_EXPLOSION);
 assert(niveauLireMatrice(&niveau, 2,3) == ELEMENT_EXPLOSION);
 assert(niveauLireMatrice(&niveau, 3,3) == ELEMENT_EXPLOSION);
 assert(niveauLireMatrice(&niveau, 3,2) == ELEMENT_EXPLOSION);
 assert(niveauLireMatrice(&niveau, 1,1) == ELEMENT_BORD);
 assert(niveauLireMatrice(&niveau, 1,2) == ELEMENT_BORD);
 assert(niveauLireMatrice(&niveau, 2,1) == ELEMENT_BORD);
 assert(niveauLireMatrice(&niveau, 3,1) == ELEMENT_BORD);  
 assert(liste_explosionTaille(niveauLireListeExplosion(&niveau)) == 4);
/* fprintf(stderr, "   %d   ", explosionLireCompteur(
                         liste_explosionLire(niveauLireListeExplosion(&niveau),1)
                         ));
*/ environnementEvoluerListeExplosion (&niveau);
}
 assert(niveauLireMatrice(&niveau, 2,2) == ELEMENT_VIDE);
 assert(niveauLireMatrice(&niveau, 2,3) == ELEMENT_VIDE);
 assert(niveauLireMatrice(&niveau, 3,3) == ELEMENT_VIDE);
 assert(niveauLireMatrice(&niveau, 3,2) == ELEMENT_VIDE);
 assert(liste_explosionTaille(niveauLireListeExplosion(&niveau)) == 0);
  niveauDetruire(&niveau);     
  oki ();
}

 
int main ()
{
      fprintf(stderr, "====TEST MODULE ENVIRONNEMENT====\n");
    fprintf(stderr, "* Test de environnementPierre\n");
    
     test_environnementPierre_Vide();
   test_environnementPierre_Explosion();
    fprintf(stderr, "* Test de environnementExplosion\n");
   
   test_environnementExplosion33_diamant();
   test_environnementExplosion33_vide();
   
     fprintf(stderr, "TOUT OK\n\n");  
    return 0;
}
