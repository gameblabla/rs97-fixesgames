#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "niveau.h"

void oki () { fprintf(stderr, " = OK\n"); }




void test_niveauChargementChamps()
{
   fprintf(stderr, "* Test test_niveauChargementChamps");
   
   Niveau niveau;
   niveauInitialiser(&niveau);
   
   niveauChargement(&niveau, "./test/testniveauchargement.txt");
   assert(niveau.temps_restant == (Temps) 100);
   assert(niveau.mur.tour_de_boucle_restant == (Temps) 56);
   assert(niveau.longueurx==2);
   assert(niveau.longueury==3);
   assert(niveau.nombre_de_diamant_restant = 23);
   assert(niveau.slim.vitesse = 34);

    niveauDetruire(&niveau);
 
   oki ();  
}

void test_niveauChargementMatrice()
{
   fprintf(stderr, "* Test test_niveauChargementMatrice");
   
   Niveau niveau;
   niveauInitialiser(&niveau);
   
   niveauChargement(&niveau, "./test/testniveauchargement.txt");
   assert(niveau.temps_restant == (Temps) 100);
   assert(niveau.mur.tour_de_boucle_restant == (Temps) 56);
   assert(niveau.longueurx==2);
   assert(niveau.longueury==3);
   assert(niveau.nombre_de_diamant_restant = 23);
   assert(niveau.slim.vitesse = 34);
   assert(niveauLireMatrice(&niveau, 1,1) == ELEMENT_BORD);   
   assert(niveauLireMatrice(&niveau, 1,2) == ELEMENT_BORD);   
   assert(niveauLireMatrice(&niveau, 2,1) == ELEMENT_BORD);   
   assert(niveauLireMatrice(&niveau, 1,3) == ELEMENT_BORD);   
   assert(niveauLireMatrice(&niveau, 2,3) == ELEMENT_BORD);   
   
   assert(niveauLireMatrice(&niveau, 2,2) == ELEMENT_TERRE);
      niveauDetruire(&niveau);
   oki ();  
}

void test_niveauChargementMatricePierre()
{
   fprintf(stderr, "* Test test_niveauChargementMatricePierre");
   Pierre* pierre;
   Niveau niveau;
   niveauInitialiser(&niveau);
   
   niveauChargement(&niveau, "./test/testchargementpierre.txt");
   
   fprintf(stderr, "\n*** test presence pierre ");
   assert(niveauLireMatrice(&niveau, 2,2) == ELEMENT_PIERRE); 
   oki ();
     fprintf(stderr, "*** test liste pierre "); 
   assert(liste_pierreTaille(niveauLireListePierre(&niveau)) == 1);
   pierre = liste_pierreLire(niveauLireListePierre(&niveau),1);
   assert(pierreLireX(pierre) == 2);
   assert(pierreLireY(pierre) == 2);
      niveauDetruire(&niveau);
   oki ();  
}

void test_niveauChargementMatriceDiamant()
{
   fprintf(stderr, "* Test test_niveauChargementMatriceDiamant");
   Diamant* diamant;
   Niveau niveau;
   niveauInitialiser(&niveau);
   
   niveauChargement(&niveau, "./test/testchargementdiamant.txt");
 
   fprintf(stderr, "\n*** test presencediamant ");
   assert(niveauLireMatrice(&niveau, 2,2) == ELEMENT_DIAMANT);
   assert(niveauLireMatrice(&niveau, 1,2) == ELEMENT_DIAMANT);   
   oki ();
    fprintf(stderr, "*** test liste diamant ");
   assert(liste_diamantTaille(niveauLireListeDiamant(&niveau)) == 2);
   diamant = liste_diamantLire(niveauLireListeDiamant(&niveau),1);
   assert(diamantLireX(diamant) == 1);
   assert(diamantLireY(diamant) == 2);
   niveauDetruire(&niveau);
   oki ();  
}

void test_niveauChargementMatriceJoueur()
{
   fprintf(stderr, "* Test test_niveauChargementMatricejoueur");
   Joueur* joueur;
   Niveau niveau;
   niveauInitialiser(&niveau);
   
   niveauChargement(&niveau, "./test/testchargementjoueur.txt");
   fprintf(stderr, "\n*** test presence joueur ");
   assert(niveauLireMatrice(&niveau, 1,2) == ELEMENT_JOUEUR);
   oki ();
   fprintf(stderr, "*** test presence vide ");
   assert(niveauLireMatrice(&niveau, 4,1) == ELEMENT_VIDE);
   oki ();
   fprintf(stderr, "*** test presence porte fin ");
   assert(niveauLireMatrice(&niveau, 5,1) == ELEMENT_PORTE_DE_FIN);
   oki ();
   fprintf(stderr, "*** test presence mur ");
   
   assert(niveauLireMatrice(&niveau, 5,3) == ELEMENT_MUR);
   oki ();
   fprintf(stderr, "*** test presence ennemi ");
   
   assert(niveauLireMatrice(&niveau, 5,2) == ELEMENT_ENNEMI);
   assert(niveauLireMatrice(&niveau, 4,3) == ELEMENT_ENNEMI);
   oki ();
   fprintf(stderr, "*** test presence mur qui bouge ");
   
   assert(niveauLireMatrice(&niveau, 3,3) == ELEMENT_MUR_QUI_BOUGE);
   oki ();
   fprintf(stderr, "*** test presence slim ");
   
   assert(niveauLireMatrice(&niveau, 3,1) == ELEMENT_SLIM);
   oki ();    
    fprintf(stderr, "*** test liste joueur");        
   assert(liste_joueurTaille(niveauLireListeJoueur(&niveau)) == 1);
   joueur = liste_joueurLire(niveauLireListeJoueur(&niveau),1);
   assert(joueurLireX(joueur) == 1);
   assert(joueurLireY(joueur) == 2);
   niveauDetruire(&niveau);
      oki ();  
}

int main (int p, char ** argv)
{
  fprintf(stderr, "======TEST_MODULE NIVEAU=========\n");

  
  test_niveauChargementChamps();    
    
  test_niveauChargementMatrice();  
  test_niveauChargementMatricePierre();
  test_niveauChargementMatriceDiamant();
  test_niveauChargementMatriceJoueur();
    
   fprintf(stderr, "TOUT OK \n\n"); 
 return 0;   
}
