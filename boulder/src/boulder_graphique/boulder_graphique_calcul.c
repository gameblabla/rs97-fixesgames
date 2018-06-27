#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "../moteur_graphique/moteur_graphique.h"

#include "../fichier/fichier.h"
#include "../niveau/niveau.h"
#include "../environnement/environnement.h"


#include "boulder_graphique.h"






/*
 *
 *
 * ORIGINE
 *
 *
 *
 */

void                   ecran_joueurRepositionnementOrigine(Ecran_joueur* ecran)
{
   Joueur* joueur = ecran->j_courant;
 
  /* on teste la position du joueur par rapport à l'extremité de la partie qui doit
    être affiché, si il est trop prêt, on commence à décaler et on repositionne les origines
    */ 
  if (joueurLireX(joueur) - ecran->j_originex >= ecran->nombre_de_spriteX - ecran->j_commence_decalagex 
       && ((ecran->j_originex+ecran->nombre_de_spriteX)-1 < niveauLireLongueurX(ecran->niveau))
       &&  joueurLireX(joueur) < niveauLireLongueurX(ecran->niveau)-ecran->j_commence_decalagex) {
     ecran->repositionnement = REPOSITIONNEMENT_DROITE; //fprintf(stderr, "d");
     ecran->j_boucle_decalage = 1; /* on commence juste à décaler, donc le compteur de boucle est à 1*/
   }
   /* idem mais pour une autre frontière*/  
   if (joueurLireX(joueur) - ecran->j_originex <= ecran->j_commence_decalagex -1
     &&  ((ecran->j_originex) >=1
      && joueurLireX(joueur) > ecran->j_commence_decalagex)
      ) 
   {
     ecran->repositionnement = REPOSITIONNEMENT_GAUCHE;/*fprintf(stderr ,"g");*/
     ecran->j_boucle_decalage = 1;
   }  
  
   if (joueurLireY(joueur) - ecran->j_originey > ecran->nombre_de_spriteY-ecran->j_commence_decalagey 
           &&
           ((ecran->j_originey +ecran->nombre_de_spriteY) < niveauLireLongueurY(ecran->niveau)) 
       //    &&  joueurLireY(joueur) < niveauLireLongueurY(ecran->niveau)-ecran->j_commence_decalagey
            ) {
     ecran->repositionnement = REPOSITIONNEMENT_BAS;//fprintf(stderr, "b");
     ecran->j_boucle_decalage = 1;
   }  
   if (joueurLireY(joueur) - ecran->j_originey <= ecran->j_commence_decalagey-1 && 
           ((ecran->j_originey) > 0) 
           && joueurLireY(joueur) > ecran->j_commence_decalagey) {
     ecran->repositionnement = REPOSITIONNEMENT_HAUT;//fprintf(stderr, "h");
     ecran->j_boucle_decalage = 1;
   }  
   
  
 // fprintf(stderr, "=GARDER=\n");
   if ((0<ecran->j_boucle_decalage) && (ecran->j_boucle_decalage < ecran->j_decalagex))  {
      switch (ecran->repositionnement ) {
        case REPOSITIONNEMENT_DROITE:
         if ((ecran->j_originex+ ecran->nombre_de_spriteX) <= niveauLireLongueurX(ecran->niveau))
            ecran->j_originex += 0.5; /* cela correspond au décalage de 1/2 sprites, avec 1 sprite, 
                                       * cela fait légerement plus sacadé*/

          break;
       case REPOSITIONNEMENT_GAUCHE:
          if ((ecran->j_originex) >1)
            ecran->j_originex -= 0.5;
     
          break;
       default:
        break;
      }
} 
    if ((0< ecran->j_boucle_decalage) && ( ecran->j_boucle_decalage < ecran->j_decalagey))  {
      switch (ecran->repositionnement) {
       case REPOSITIONNEMENT_BAS:
          if ((ecran->j_originey+ ecran->nombre_de_spriteY) <  niveauLireLongueurY(ecran->niveau))
            ecran->j_originey +=0.5;
     
          break;
          
        case REPOSITIONNEMENT_HAUT:
          if ((ecran->j_originey) > 1) /* on teste que l'on ne sorte pas du niveau*/
            ecran->j_originey -=0.5;
  
          break;
        default:
          break;          
       }
     }
     ecran->j_boucle_decalage++;
     if (ecran->j_boucle_decalage >= ecran->j_decalagex) {
        ecran->j_boucle_decalage = 0 ;
        ecran->repositionnement = REPOSITIONNEMENT_BOUGE_PAS;
     }
      
                       
                                          
                       
}






/*
 * Fonction permettant de faire évoluer tous les compteurs de chaque type de sprite*/
void ecran_joueurEvoluerCompteur(Ecran_joueur* ecran)
{
 Ecran_joueur_sprite* c = ecran->ensemble_sprite;
 c->compteur_joueur = (1+c->compteur_joueur) % 5; /* %5 car il y a 5 sprites pour l'animation*/
 c->compteur_ennemi_type_diamant = (1+c->compteur_ennemi_type_diamant) % 2;
 c->compteur_ennemi_type_rien = (1+c->compteur_ennemi_type_rien) % 4;
 
 c->compteur_diamant = (1+c->compteur_diamant) % 4;
 c->compteur_slim = (1+c->compteur_slim) % 4;
 c->compteur_mur_qui_bouge = (1+c->compteur_mur_qui_bouge) % 4;
 c->compteur_porte_fin = (1+c->compteur_porte_fin) % 8;
}
