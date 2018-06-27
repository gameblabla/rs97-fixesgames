#include <stdio.h>
#include <stdlib.h>

#include "../moteur_graphique/moteur_graphique.h"
#include "../fichier/fichier.h"
#include "../niveau/niveau.h"
#include "../environnement/environnement.h"

#include "../boulder_graphique/boulder_graphique.h"

//#define _JEU_DEBUG

static void jeu_un_joueur_animation_debut(G_fenetre* fenetre, Ecran_joueur* ecran, Niveau* niveau,  G_rectangle* rect  );
static void jeu_chargement_sprite (Ecran_joueur * ecran);
static void jeu_un_joueur_animation_gagne(G_fenetre* fenetre, Ecran_joueur* ecran, Niveau* niveau,  G_rectangle* rect  );
static void jeu_un_joueur_animation_mort(G_fenetre* fenetre, Ecran_joueur* ecran, Niveau* niveau, G_rectangle * rect );

int jeuJoueurVeutSortie()
{
 return toucheEstAppuyer(T_ESCAPE);
}


/* on charge tous les sprites*/
static void jeu_chargement_sprite (Ecran_joueur * ecran)
{
     
 ecran_joueurChargementSpriteJoueurBougePas (ecran, "./data/sprite/boulder.txt");
 ecran_joueurChargementSpriteJoueurGauche ( ecran, 
                                              "./data/sprite/boulderg1.txt",
                                              "./data/sprite/boulderg2.txt", 
                                              "./data/sprite/boulderg3.txt", 
                                              "./data/sprite/boulderg4.txt");
 ecran_joueurChargementSpriteJoueurDroite ( ecran, 
                                              "./data/sprite/boulderd1.txt",
                                              "./data/sprite/boulderd2.txt",
                                              "./data/sprite/boulderd3.txt",
                                              "./data/sprite/boulderd4.txt");
 ecran_joueurChargementSpriteMur ( ecran, "./data/sprite/mur.txt");
 ecran_joueurChargementSpriteMurQuiBouge ( ecran, 
                                              "./data/sprite/murbouge1.txt", 
                                              "./data/sprite/murbouge2.txt", 
                                              "./data/sprite/murbouge3.txt", 
                                              "./data/sprite/murbouge4.txt");
                                              
 ecran_joueurChargementSpritePierre (ecran, "./data/sprite/pierre.txt");
  ecran_joueurChargementSpritePierre_type_explosion (ecran, "./data/sprite/bombe.txt");
 ecran_joueurChargementSpriteDiamant ( ecran, 
                                              "./data/sprite/diamant1.txt", 
                                              "./data/sprite/diamant2.txt",
                                             "./data/sprite/diamant3.txt", 
                                              "./data/sprite/diamant4.txt");
 ecran_joueurChargementSpriteEnnemiTypeRien (ecran, 
                                              "./data/sprite/ennemi1t1.txt", 
                                              "./data/sprite/ennemi2t1.txt", 
                                              "./data/sprite/ennemi3t1.txt", 
                                              "./data/sprite/ennemi4t1.txt");
                                              

 ecran_joueurChargementSpriteEnnemiTypeDiamant (ecran, 
                                              "./data/sprite/ennemi1t2.txt", 
                                              "./data/sprite/ennemi2t2.txt");
 ecran_joueurChargementSpriteVide (ecran, "./data/sprite/vide.txt");
 ecran_joueurChargementSpriteTerre (ecran, "./data/sprite/terre.txt");
 ecran_joueurChargementSpriteBord (ecran, "./data/sprite/case.txt",
                                          "./data/sprite/casevide.txt");
 ecran_joueurChargementSpriteSlim (ecran, "./data/sprite/slim1.txt", 
                                              "./data/sprite/slim2.txt");    
     
   ecran_joueurChargementSpriteExplosion (ecran, "./data/sprite/explosion1.txt", 
                                              "./data/sprite/explosion2.txt",
                                              "./data/sprite/explosion3.txt"
                                              );    
     
      
  ecran_joueurChargementSpriteDiamantTypeTransformation(ecran, "./data/sprite/diamante1.txt",
                                                               "./data/sprite/diamante2.txt",
                                                               "./data/sprite/diamante3.txt",
                                                               "./data/sprite/diamante4.txt");
     
     
     
}

/* fonction pour le jeu en un joueur*/
/* une fonction aussi grosse servirait à faire fonctionner pour 2 joueurs*/
int jeu_un_joueur(G_fenetre* fenetre, Niveau* niveau, char* chemin_niveau, char* chemin_couleur)
{
 G_rectangle rect;
 G_font*  font_joueur;  
 Ecran_joueur* ecran;
 int i;
 
 niveauReinitialiser(niveau);
 /* on charge le fond*/
 font_joueur = g_fontCharger("./data/fonts/times.ttf", 30);
 
 /* on créait l'écran spécifique au joueur*/
 ecran = ecran_joueurCreer(320,240);

 /* on charge le niveau qu'ou souhaite*/
 niveauChargement(niveau, chemin_niveau);

 ecran_joueurInitialiser( ecran, 
                          niveau, 
                          liste_joueurLire(niveauLireListeJoueur(niveau),1),
                          20,
                          12,
                          2,/*zoom*/
                          font_joueur);    
                          
 /* on charge les couleurs*/
 ecran_joueurChargementCouleurs(ecran, chemin_couleur);
 
 g_rectangleInitialiser(&rect, 0,0, ecran_joueurLireResolutionx(ecran), ecran_joueurLireResolutiony(ecran));
  
 
 /* maintenant qu'on a les cxouleurs, on peut charger les sprites*/
 jeu_chargement_sprite (ecran);
 
 
 /* on fait la petite animation du début*/
 jeu_un_joueur_animation_debut(fenetre, ecran, niveau, &rect );

 /*
  *
  *
  * début jeu
  *
  *
  *
  *
  */
 niveauEcrireEtat(ecran_joueurLireNiveau(ecran), NIVEAU_ETAT_EN_COURS);
 
 
 while (!toucheEstAppuyer(T_ESCAPE)) {
     // fprintf(stderr, "Taille: %d", liste_diamantTaille(niveauLireListeDiamant(ecran_joueurLireNiveau(ecran))));
 /* on fait évoluer tous les objets*/
       environnementEvoluer(ecran_joueurLireNiveau(ecran));
 /* puis le joueur*/
       environnementEvoluerListeJoueur(ecran_joueurLireNiveau(ecran));  
 
 /* on raffraichit le graphisme*/
       ecran_joueurRaffraichissement(ecran);
 /* évolution des compteurs pour le graphisme*/
       ecran_joueurEvoluerCompteur(ecran);
       
 /* on affiche suivant le repositionnement par rapport au joueur*/
       if (ecran->repositionnement == REPOSITIONNEMENT_BOUGE_PAS)
       {
          ecran_joueurRepositionnementOrigine(ecran);
          g_surfaceCopie(ecran_joueurLireSurface(ecran), NULL, g_fenetreLireSurface(fenetre), &rect);
          g_fenetreAfficher(fenetre);
          tempsAttendre(100);
       }
       else 
       {                          
          for (i=1; i<=2; i++)
          { 
             ecran_joueurRepositionnementOrigine(ecran);
             g_surfaceCopie(ecran_joueurLireSurface(ecran), NULL, g_fenetreLireSurface(fenetre), &rect);
             g_fenetreAfficher(fenetre);
             tempsAttendre(50);
          }
       }
       
     /* on réalise les animations suivant létat du niveau*/  
       if (niveauLireEtat(ecran_joueurLireNiveau(ecran))== NIVEAU_ETAT_GAGNE)
       { 
         jeu_un_joueur_animation_gagne(fenetre, ecran, niveau,&rect );
         break;
       }
       if (niveauLireEtat(ecran_joueurLireNiveau(ecran))==NIVEAU_ETAT_MORT_DES_JOUEURS)
       {
         jeu_un_joueur_animation_gagne(fenetre, ecran, niveau, &rect );
         break;
       }
       if (niveauLireEtat(ecran_joueurLireNiveau(ecran))==NIVEAU_ETAT_GAME_OVER_PERD)
       {
         jeu_un_joueur_animation_mort(fenetre, ecran, niveau, &rect );
         break;       
       }
       if (niveauLireEtat(ecran_joueurLireNiveau(ecran))==NIVEAU_ETAT_PLUS_DE_TEMPS)
       {
        jeu_un_joueur_animation_mort(fenetre, ecran, niveau, &rect );
         break;   
       }
       /* petit cheat pour sauter un niveau*/
       if (toucheEstAppuyer(T_BACKSPACE))
       {
          niveauEcrireEtat(ecran_joueurLireNiveau(ecran), NIVEAU_ETAT_GAGNE); 
          break;
       }
 }
 
 

  
    
        
/*
 *
 * Destruction
 *
 *
 */        




   g_surfaceEfface(g_fenetreLireSurface(fenetre));
   g_fontDetruire(font_joueur);
   ecran_joueurDetruire(ecran);
   niveauDetruireSaufJoueur(niveau);  
   return EXIT_SUCCESS;  
}


static void jeu_un_joueur_animation_mort(G_fenetre* fenetre, Ecran_joueur* ecran, Niveau* niveau,  G_rectangle * rect )
{
 jeu_un_joueur_animation_gagne(fenetre, ecran, niveau, rect );            
}




static void jeu_un_joueur_animation_debut(G_fenetre* fenetre, Ecran_joueur* ecran, Niveau* niveau, G_rectangle * rect )
{
  int i;
  int j;
  Joueur* joueur; /*
  *
  *
  *  Animation debut
  *
  *
  *
  *
  *
  */
#ifndef _JEU_DEBUG
 for (i=1; i <= liste_joueurTaille(niveauLireListeJoueur(niveau)); i++) 
 {
   joueur = liste_joueurLire(niveauLireListeJoueur(niveau),i);
   niveauEcrireMatrice(niveau, joueurLireX(joueur), joueurLireY(joueur), ELEMENT_PORTE_DE_FIN);    
 }
 niveauEcrirePorte(niveau, NIVEAU_PORTE_OUVERTE);

for (i=1; i<20; i++)  {

 if (toucheEstAppuyer(T_i))
 {
    niveauEcrireEtat(ecran_joueurLireNiveau(ecran), NIVEAU_ETAT_GAGNE); 
    break;
 }
 
 environnementEvoluer(ecran_joueurLireNiveau(ecran));
 /*environnementEvoluerListeJoueur(ecran_joueurLireNiveau(ecran)); */
 ecran_joueurRaffraichissement(ecran);
 ecran_joueurEvoluerCompteur(ecran);
 if (ecran->repositionnement == REPOSITIONNEMENT_BOUGE_PAS)
 {
   ecran_joueurRepositionnementOrigine(ecran);
   g_surfaceCopie(ecran_joueurLireSurface(ecran), NULL, g_fenetreLireSurface(fenetre), rect);
   g_fenetreAfficher(fenetre);
   tempsAttendre(100);
 }
 else 
 {                          
   for (j=1; j<=2; j++)
   { 
     ecran_joueurRepositionnementOrigine(ecran);
     g_surfaceCopie(ecran_joueurLireSurface(ecran), NULL, g_fenetreLireSurface(fenetre), rect);
     g_fenetreAfficher(fenetre);
     tempsAttendre(50);
   }
 }
}
 for (i=1; i <= liste_joueurTaille(niveauLireListeJoueur(niveau)); i++) 
 {
   joueur = liste_joueurLire(niveauLireListeJoueur(niveau),i);
   niveauEcrireMatrice(niveau, joueurLireX(joueur), joueurLireY(joueur), ELEMENT_JOUEUR);    
 }
 niveauEcrirePorte(niveau, NIVEAU_PORTE_FERMEE);
        
 /* fin animation debut*/
#endif     
  
}



static void jeu_un_joueur_animation_gagne(G_fenetre* fenetre, Ecran_joueur* ecran, Niveau* niveau, G_rectangle* rect )
{
#ifndef _JEU_DEBUG
 int i;
 int j;
 /*
  *
  *
  *  Animation fin
  *
  *
  *
  *
  *
  */

for (i=1; i<20; i++)  {
 environnementEvoluer(ecran_joueurLireNiveau(ecran));
 /*environnementEvoluerListeJoueur(ecran_joueurLireNiveau(ecran)); */
 ecran_joueurRaffraichissement(ecran);
 ecran_joueurEvoluerCompteur(ecran);
 if (ecran->repositionnement == REPOSITIONNEMENT_BOUGE_PAS)
 {
   ecran_joueurRepositionnementOrigine(ecran);
   g_surfaceCopie(ecran_joueurLireSurface(ecran), NULL, g_fenetreLireSurface(fenetre), rect);
   g_fenetreAfficher(fenetre);
   tempsAttendre(100);
 }
 else 
 {                          
   for (j=1; j<=2; j++)
   { 
     ecran_joueurRepositionnementOrigine(ecran);
     g_surfaceCopie(ecran_joueurLireSurface(ecran), NULL, g_fenetreLireSurface(fenetre), rect);
     g_fenetreAfficher(fenetre);
     tempsAttendre(50);
   }
 }
}
        
#endif     
       
       
}

