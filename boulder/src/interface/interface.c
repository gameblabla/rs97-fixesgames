#include <stdio.h>
#include <stdlib.h>

#include "../moteur_graphique/moteur_graphique.h"
#include "../fichier/fichier.h"
#include "../niveau/niveau.h"
#include "../environnement/environnement.h"

#include "../boulder_graphique/boulder_graphique.h"
#include "../jeu/jeu.h"
#include "../menu/menu.h"

#include "interface.h"



int main_interface ()
{
 G_fenetre fenetre;
 Niveau niveau;

 char chemin_niveau_actuel[64];
 char chemin_couleur_actuel[64];
 Fichierl* fichier_niveau = NULL;
 Menu_resultat res;
 
 /* création de la fenetre d'affichage*/ 
 g_fenetreInitialiser(&fenetre, 320,240, PAS_PLEIN_ECRAN);
 
 /* initialiser du niveau*/
 niveauInitialiser(&niveau);
 
 
 /* c'est le menu d'introduction*/
 res =  menu(&fenetre, MENU_INTRODUCTION);
 
 /*on récupere ce que le joueur veut, et on regarde suivant le cas*/
 
 switch(res)
 {
   case MENU_UN_JOUEUR:
   case MENU_UN_JOUEUR_MODE_FLO:
     switch(res)
     {
      case MENU_UN_JOUEUR:   
      /* il veut joueur au mode normal, on charge le fichier contenant la liste
      des niveaux*/
        fichier_niveau = fichierlOuvrir("./data/niveau/liste_niveau.txt");  
        break;
      case MENU_UN_JOUEUR_MODE_FLO:
      /* il veut joueur au mode flo, on charge le fichier contenant la liste
      des niveaux*/
        fichier_niveau = fichierlOuvrir("./data/niveau/fliste_niveau.txt");  
        break;
       default:
        break;
      }
      while (!jeuJoueurVeutSortie())
      {
         switch(niveauLireEtat(&niveau))
         {
         /* intercepte l'état du niveau et réalise les animations en conséquences*/
          case NIVEAU_ETAT_INTRODUCTION:
            fichierlLireLigne(fichier_niveau, chemin_niveau_actuel);
            fichierlLireLigne(fichier_niveau, chemin_couleur_actuel);
            jeu_un_joueur(&fenetre, &niveau, chemin_niveau_actuel, chemin_couleur_actuel);
            break;
          
          case NIVEAU_ETAT_PLUS_DE_TEMPS:
          case NIVEAU_ETAT_MORT_DES_JOUEURS:
               /* fin du niveau, on quitte et on recommence le niveau*/
            res =  menu(&fenetre, MENU_NIVEAU_PERDU);
            if (res==MENU_QUITTER)
              break;
            jeu_un_joueur(&fenetre, &niveau, chemin_niveau_actuel, chemin_couleur_actuel);
            break;
         
          case NIVEAU_ETAT_GAME_OVER_PERD:
               /* c'est la fin*/
             menu(&fenetre,  MENU_GAME_OVER_PERD);  
             break; 
          case NIVEAU_ETAT_GAGNE:
               /* il a gagné, on va au niveau suivant*/
            if (fichierlLireLigne(fichier_niveau, chemin_niveau_actuel)==FIN_DE_FICHIER ||
                fichierlLireLigne(fichier_niveau, chemin_couleur_actuel)==FIN_DE_FICHIER)
            {
               menu(&fenetre, MENU_GAME_OVER_GAGNE);
            }
            else
            {
               menu(&fenetre, MENU_NIVEAU_REUSSI);
               jeu_un_joueur(&fenetre, &niveau, chemin_niveau_actuel, chemin_couleur_actuel);
            }    
            break;
          default:
            break;
         }
      }   
      fichierlFermer(fichier_niveau);   
         break;
  default:
         break;
 } 



 liste_joueurDetruire(niveauLireListeJoueur(&niveau));
 niveauDetruire(&niveau);
 g_fenetreDetruire(&fenetre);
 
 return EXIT_SUCCESS;  
}
