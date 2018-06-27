#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "../moteur_graphique/moteur_graphique.h"

#include "../fichier/fichier.h"
#include "../niveau/niveau.h"
#include "../environnement/environnement.h"


#include "boulder_graphique.h"











/* convertit un type d'élément en sprite (donne le bon suivant les animations*/
G_surface* element_niveau2sprite(Ecran_joueur* ecran, Niveau_element element, int i, int j)
{
  Ecran_joueur_sprite* e = ecran->ensemble_sprite;
  Joueur* joueur;
  Ennemi* ennemi;
  Explosion* explosion;
  Diamant* diamant;
  Pierre* pierre;
  
  /* Regarde pour chaque type d'élément*/
  switch (element)
  {
    case ELEMENT_VIDE:
         /* IL n'y a rien, aucune animation, on renvoit le sprite conrrespondant
         */
         return e->vide[0];
         break;
    case ELEMENT_PIERRE:
         /*
          Détermine le type de la pierre pour l'affichage*/
         pierre = liste_pierreLire(niveauLireListePierre(ecran->niveau),
                       liste_pierreRechercher(niveauLireListePierre(ecran->niveau),i, j));

         /* Suivant le type de la pierre*/
         switch(pierreLireType(pierre))
         {
          case PIERRE_TYPE_PIERRE:
                return e->pierre[0]; /*la pierre n'est pas animée, on renvoit toujours le 0*/
                break;
          case PIERRE_TYPE_EXPLOSION:
                return e->bombe[0];
                break;
          default:
                 return e->pierre[0];
                 break;
          }       
         break;
    case ELEMENT_MUR:
         return e->mur[0];
         break;
    case ELEMENT_TERRE:
         return e->terre[0];
    case ELEMENT_SLIM: /*le slim est animé*/
         return e->slim[e->compteur_slim/2];
         break;
    case ELEMENT_MUR_QUI_BOUGE:
         /* On cherche à savoir si le mur qui bouge est en train de bouger ou non*/
         if (mur_qui_bougeLireEtat(&ecran->niveau->mur) == MUR_QUI_BOUGE_ETAT_EN_COURS)
         {
           return e->mur_qui_bouge[e->compteur_mur_qui_bouge]; /*animation*/                                                                       
         }
         else
           return e->mur[0]; /*pas d'animation*/
         break;
    
    case ELEMENT_PORTE_DE_FIN:
         /* On cherche à savoir dans quel état est la porte, fermé ou ouverte*/
         if (niveauLirePorte(ecran->niveau) == NIVEAU_PORTE_FERMEE)
          return e->bord[0];
         else
         { 
           if (e->compteur_porte_fin<=3)
            return e->bord[0];
           else
             return e->bord[1];
         }
         break;
         
    case ELEMENT_JOUEUR:
         /* On détermine c'est quel joueur*/
        joueur = liste_joueurLire(niveauLireListeJoueur(ecran->niveau),
                            liste_joueurRechercher(niveauLireListeJoueur(ecran->niveau),i,j));
        if (joueur == NULL)
          break;
        if (joueurLireForme(joueur) == JOUEUR_FORME_BOULDER) {
        switch (joueurLireDirection(joueur))
        {
          /* L'animation est différente selon la direction*/
          case JOUEUR_DIRECTION_DROITE:
          case JOUEUR_DIRECTION_HAUT:
          /* le joueur dispose d'un compteur dans la structure graphique avancé, cela permet de
          * déterminer l'état de l'animation*/
             switch (e->compteur_joueur)
             {
               case 0:
                    return e->joueur_droite[0];
               case 1:
                    return e->joueur_droite[1];
               case 2:
                    return e->joueur_droite[2];
               case 3:
                    return e->joueur_droite[3];
               case 4:
                    return e->joueur_droite[2];                    
               case 5:
                    return e->joueur_droite[1];
               default:
                    return e->joueur_droite[0];   
               } 
          case JOUEUR_DIRECTION_GAUCHE:                    
          case JOUEUR_DIRECTION_BAS:
             switch (e->compteur_joueur)
             {
               case 0:
                    return e->joueur_gauche[0];
               case 1:
                    return e->joueur_gauche[1];
               case 2:
                    return e->joueur_gauche[2];
               case 3:
                    return e->joueur_gauche[3];
               case 4:
                    return e->joueur_gauche[2];                    
               case 5:
                    return e->joueur_gauche[1];
               default:
                    return e->joueur_gauche[0];      
             }          
         case JOUEUR_DIRECTION_BOUGE_PAS:
              return e->joueur_bouge_pas[0];
         default:                 
              return e->joueur_bouge_pas[0]; 
        }
        }
        else if (joueurLireForme(joueur) == JOUEUR_FORME_ENNEMI)
        {
          return e->ennemi_type_rien[(e->compteur_ennemi_type_rien+1)%4];     
        }
   
           break;
    case ELEMENT_ENNEMI:
         /* idem*/
        ennemi = liste_ennemiLire(niveauLireListeEnnemi(ecran->niveau),
                            liste_ennemiRechercher(niveauLireListeEnnemi(ecran->niveau),i,j)); 
        if (ennemi!=NULL)
          switch(ennemiLireType(ennemi))
          {
           case ENNEMI_TYPE_RIEN:
              return e->ennemi_type_rien[e->compteur_ennemi_type_rien];                               
           case ENNEMI_TYPE_DIAMANT:
            return e->ennemi_type_diamant[e->compteur_ennemi_type_diamant];  
           case ENNEMI_TYPE_PIERRE:
            return e->ennemi_type_diamant[e->compteur_ennemi_type_diamant];      
           default:
             return e->ennemi_type_diamant[e->compteur_ennemi_type_diamant];  
                                         
          }
        break;
    case ELEMENT_DIAMANT:
       diamant = liste_diamantLire(niveauLireListeDiamant(ecran->niveau),
                       liste_diamantRechercher(niveauLireListeDiamant(ecran->niveau),i, j));
       if (diamant!=NULL)
        {
         if (diamantLireType(diamant) == DIAMANT_TYPE_DIAMANT)
             return e->diamant[e->compteur_diamant];            
         else if (diamantLireType(diamant) == DIAMANT_TYPE_TRANSFORMATION)
             return e->diamant_type_transformation[e->compteur_diamant];
        }
       break;    
    case ELEMENT_EXPLOSION:
        explosion = liste_explosionLire(niveauLireListeExplosion(ecran->niveau),
                       liste_explosionRechercher(niveauLireListeExplosion(ecran->niveau),i, j));
        return e->explosion[explosionLireCompteur(explosion)];
        break;
    case ELEMENT_BORD:
      return e->bord[0];
      default:
         return e->vide[0];
         
   }         
           
           
           
    return e->vide[0];       
           
           
           
           
}

/* fonction local minimal*/
static int min (int a, int b)
{
       if (a<b) return a; else return b;
}


void                   ecran_joueurRaffraichissement(Ecran_joueur* ecran)
{
  int i;
  int j;
  /* static car appelé à chaque boucle, cela évite d'allouée de la mémoire à chaque tour
  de boucle, on alloue une seule fois*/
  static G_rectangle r;
  static G_rectangle r2;
  /* surface dédiée au texte*/
  G_surface* surface_txt;
  char texte[80];
  
  /* création d'un rectangle de sprite pour afficher les sprites*/
  g_rectangleInitialiser(&r2,0,0,ecran->nombre_de_spriteX*(ecran->taille_spritex-1), ecran->nombre_de_spriteY*(ecran->taille_spritey-1));
  
  /* on efface le tout*/
  g_surfaceEfface(ecran_joueurLireSurface(ecran));
  
  /* parcourt de la partie du niveau à afficher selon les bornes, puis on affiche
    chaque sprite à l'endroit correspondant*/
  for (j =  1+ecran->j_originey; 
         j<= min(ecran->nombre_de_spriteY +ecran->j_originey+1, ecran->j_originey+niveauLireLongueurY(ecran->niveau)); 
         j++)
    for (i =  1+ ecran->j_originex; 
              i<= min(ecran->nombre_de_spriteX+ecran->j_originex+1, ecran->j_originex+niveauLireLongueurX(ecran->niveau)) ; 
              i++)
    {
      g_rectangleInitialiser(&r,ecran->taille_spritex*(i-ecran->j_originex-1),ecran->taille_spritey*(j-ecran->j_originey-1),0,0); 
      g_surfaceCopie( element_niveau2sprite(ecran, niveauLireMatrice(ecran->niveau,
                                                   i-1,
                                                   j-1), 
                                                   i-1  , 
                                                   j-1),
                                                    &r2, ecran->surface, &r);
       
      
  
    }
                       
  /* pour le petit texte en bas*/
  g_rectangleInitialiser(&r,0 ,ecran->taille_spritey*(1+ecran->nombre_de_spriteY),0,0); 
  if (joueurLireVie(ecran->j_courant) >1)   
     sprintf(texte, "Lives: %d Diamonds: %d Time: %d    ", joueurLireVie(ecran->j_courant), niveauLireNombreDiamantRestant(ecran->niveau), niveauLireTempsRestant(ecran->niveau));
  else
     sprintf(texte, "Lives: %d Diamonds: %d Time: %d     ", joueurLireVie(ecran->j_courant), niveauLireNombreDiamantRestant(ecran->niveau), niveauLireTempsRestant(ecran->niveau));
 
  surface_txt =  g_surfaceEcrireTexte(ecran->font, texte , 
             g_couleurCreer(ecran_joueurLireSurface(ecran), 255,255,255), /*couleur du texte*/
             g_couleurCreer(ecran_joueurLireSurface(ecran), 0,0,0));      /*couleur de fond*/
                       
  g_surfaceCopie(surface_txt, NULL, ecran_joueurLireSurface(ecran), &r);                     
                       
                       
}
