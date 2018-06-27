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
 * Un fichier de couleur est définie comme ça :
COULEUR_NOIR= 0 0 0
COULEUR_BLANC= 255 255 255
COULEUR_JAMBE= 255 153 0
COULEUR_TETE= 153 153 153
COULEUR_MOCHE= 255 153 153
 * ON peut permuter COULEUR_x et COULEUR_y,
 * on lit juste donc le fichier et on charge les couleurs comme il faut
 */

void                   ecran_joueurChargementCouleurs(Ecran_joueur* ecran, char* fichier)
{
    Fichierl* f;
    char c[50];
    int i;
    int c1;
    int c2;
    int c3;
    
    f = fichierlOuvrir(fichier);
    for (i=1; i<=5; i++)
    {
      if (fichierlLireEgale(f, c)==FIN_DE_FICHIER)
            fprintf(stderr,"** Erreur format fichier couleur\n");    
      if (fichierlLireEntier(f, &c1) == FIN_DE_FICHIER)
            fprintf(stderr,"** Erreur format fichier couleur\n"); 
      if (fichierlLireEntier(f, &c2) == FIN_DE_FICHIER)
            fprintf(stderr,"** Erreur llformat fichier couleur\n"); 
      if (fichierlLireEntier(f, &c3) == FIN_DE_FICHIER)
            fprintf(stderr,"** Erreur format fichier couleur\n"); 
        /*fprintf(stderr, "==='%s'===", c);
      */
      if (strcmp(c, "COULEUR_NOIR")==0)
      {
         ecran->couleurs->couleur_noir = g_couleurCreer(ecran_joueurLireSurface(ecran), c1, c2 ,c3);
      }
      else {
        if (strcmp(c, "COULEUR_BLANC")==0)
        {
           ecran->couleurs->couleur_blanc = g_couleurCreer(ecran_joueurLireSurface(ecran), c1, c2 ,c3);
        }
        else {
          if (strcmp(c, "COULEUR_TETE")==0)
          {
            ecran->couleurs->couleur_tete = g_couleurCreer(ecran_joueurLireSurface(ecran), c1, c2 ,c3);
          }
          else {
            if (strcmp(c, "COULEUR_JAMBE")==0)
            {
               ecran->couleurs->couleur_jambe = g_couleurCreer(ecran_joueurLireSurface(ecran), c1, c2 ,c3);
            }
            else {
              if (strcmp(c, "COULEUR_MOCHE")==0)
              {
                 ecran->couleurs->couleur_moche = g_couleurCreer(ecran_joueurLireSurface(ecran), c1, c2 ,c3);
              }
             else {
                fprintf(stderr,"** Erreur format fichier couleur\n");
             }
           }
         }
       }
     }
   }                 
     fichierlFermer(f);                      
}




