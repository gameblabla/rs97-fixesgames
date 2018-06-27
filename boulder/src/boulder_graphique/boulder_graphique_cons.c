#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "../moteur_graphique/moteur_graphique.h"

#include "../fichier/fichier.h"
#include "../niveau/niveau.h"
#include "../environnement/environnement.h"


#include "boulder_graphique.h"
void                   ecran_joueurDetruireSprite(Ecran_joueur* ecran);


/* Allocation de la mémoire nécessaire et initialisation*/
Ecran_joueur*          ecran_joueurCreer(int x, int y)
{
  Ecran_joueur* ecran= malloc(sizeof(Ecran_joueur));
  ecran->couleurs = malloc(sizeof(Ecran_joueur_couleurs));
  ecran->surface = g_surfaceCreer(x, y);
  ecran->ensemble_sprite = malloc(sizeof(Ecran_joueur_sprite));
  ecran->j_commence_decalagex=4;
  ecran->j_commence_decalagey=3;
  ecran->j_decalagex = 9;
  ecran->j_decalagey= 3;
  return ecran;
}



void                   ecran_joueurInitialiser( Ecran_joueur* ecran, Niveau* niveau, 
                                                     Joueur* joueur,
                                                     int nombre_de_spriteX,
                                                     int nombre_de_spriteY,
                                                     int zoom,
                                                     G_font* font)
{
  
 Ecran_joueur_sprite* c = ecran->ensemble_sprite;
  ecran->font = font;
  ecran->niveau = niveau;
  ecran->j_courant = joueur;
  ecran->nombre_de_spriteX = nombre_de_spriteX;
  ecran->nombre_de_spriteY=nombre_de_spriteY;
  ecran->zoom = zoom;
  ecran->taille_spritex = 8*zoom; /* 16 correspond à la taille en pixel d'un sprite sans aucun zoom*/
  ecran->taille_spritey = 8*zoom;
  ecran->ensemble_sprite->compteur_diamant = 0;
  ecran->ensemble_sprite->compteur_ennemi_type_rien = 0;
  ecran->ensemble_sprite->compteur_ennemi_type_diamant = 0;
  ecran->ensemble_sprite->compteur_joueur = 0;
  ecran->ensemble_sprite->compteur_slim = 0;
  ecran->ensemble_sprite->compteur_mur_qui_bouge = 0;
  ecran->resolutionx = ecran->taille_spritex * nombre_de_spriteX;
  ecran->resolutiony = ecran->taille_spritey * nombre_de_spriteY;
  
  c->compteur_joueur = 0;
 c->compteur_ennemi_type_diamant = 0;
 c->compteur_ennemi_type_rien = 0;
 
 c->compteur_diamant = 0;
 c->compteur_slim = 0;
 c->compteur_mur_qui_bouge = 0;
 c->compteur_porte_fin = 0;
  
  ecran_joueurCalculOrigine(ecran);
  
}



void                   ecran_joueurCalculOrigine(Ecran_joueur* ecran)
{
 ecran->j_originex = 1.F;
 ecran->j_originey = 1.F;                       

}

/* Liberétion de la place mémoire*/
void      ecran_joueurDetruire(Ecran_joueur* ecran)
{
  ecran_joueurDetruireSprite(ecran);
  g_surfaceDetruire(ecran->surface);
  free(ecran->ensemble_sprite);
  free(ecran->couleurs);
  free(ecran);        
          
}


void                   ecran_joueurDetruireSprite(Ecran_joueur* ecran)
{
  int i;
  Ecran_joueur_sprite* e = ecran->ensemble_sprite;
  
/* on détruit en mémoire chaque sprite*/
  for (i=0; i<4; i++)
    g_surfaceDetruire( e->joueur_gauche[i]);
  for (i=0; i<4; i++)
    g_surfaceDetruire( e->joueur_droite[i]);
  for (i=0; i<1; i++)
    g_surfaceDetruire( e->joueur_bouge_pas[i]);
  for (i=0; i<1; i++)
    g_surfaceDetruire( e->mur[i]);
  for (i=0; i<4; i++)
    g_surfaceDetruire( e->mur_qui_bouge[i]);
  for (i=0; i<1; i++)
    g_surfaceDetruire( e->pierre[i]);
  for (i=0; i<1; i++)
    g_surfaceDetruire( e->bombe[i]);
    
  for (i=0; i<4; i++)
    g_surfaceDetruire( e->diamant[i]);
  for (i=0; i<2; i++)
    g_surfaceDetruire( e->ennemi_type_diamant[i]);
  for (i=0; i<4; i++)
    g_surfaceDetruire( e->ennemi_type_rien[i]);
  for (i=0; i<2; i++)
    g_surfaceDetruire( e->slim[i]);
 for (i=0; i<1; i++)
    g_surfaceDetruire( e->vide[i]);
 for (i=0; i<1; i++)
    g_surfaceDetruire( e->terre[i]);
 for (i=0; i<2; i++)
    g_surfaceDetruire( e->bord[i]);
 for (i=0; i<3; i++)
    g_surfaceDetruire( e->explosion[i]);
 for (i=0; i<4; i++)
    g_surfaceDetruire( e->diamant_type_transformation[i]);
        
    
  
  
                       
}


/* fonction permettant le chargement de chaque type de sprite*/

void ecran_joueurChargementSpriteJoueurBougePas (Ecran_joueur* ecran, char* s)
{
  G_surface* sprite;
  sprite = ecran_joueurChargementSprite(ecran, s);
  ecran->ensemble_sprite->joueur_bouge_pas[0] = sprite;
}

void ecran_joueurChargementSpriteJoueurGauche (Ecran_joueur* ecran, 
                                              char* s1, char* s2, char* s3, char* s4)
{
  G_surface* sprite0;
  G_surface* sprite1;
  G_surface* sprite2;
  G_surface* sprite3;
  
  sprite0 = ecran_joueurChargementSprite(ecran, s1);
  sprite1 = ecran_joueurChargementSprite(ecran, s2);
  sprite2 = ecran_joueurChargementSprite(ecran, s3);
  sprite3 = ecran_joueurChargementSprite(ecran, s4);
  
  ecran->ensemble_sprite->joueur_gauche[0] = sprite0;
  ecran->ensemble_sprite->joueur_gauche[1] = sprite1;
  ecran->ensemble_sprite->joueur_gauche[2] = sprite2;
  ecran->ensemble_sprite->joueur_gauche[3] = sprite3;
  
}

void ecran_joueurChargementSpriteJoueurDroite (Ecran_joueur* ecran, 
                                              char* s1, char* s2, char* s3, char* s4)
{
  G_surface* sprite0;
  G_surface* sprite1;
  G_surface* sprite2;
  G_surface* sprite3;
  
  sprite0 = ecran_joueurChargementSprite(ecran, s1);
  sprite1 = ecran_joueurChargementSprite(ecran, s2);
  sprite2 = ecran_joueurChargementSprite(ecran, s3);
  sprite3 = ecran_joueurChargementSprite(ecran, s4);
  
  ecran->ensemble_sprite->joueur_droite[0] = sprite0;
  ecran->ensemble_sprite->joueur_droite[1] = sprite1;
  ecran->ensemble_sprite->joueur_droite[2] = sprite2;
  ecran->ensemble_sprite->joueur_droite[3] = sprite3;
}
  
   
void ecran_joueurChargementSpriteMur (Ecran_joueur* ecran, char* s)
{
  G_surface* sprite;
  sprite = ecran_joueurChargementSprite(ecran, s);
  ecran->ensemble_sprite->mur[0] = sprite;
}



void ecran_joueurChargementSpriteMurQuiBouge (Ecran_joueur* ecran, 
                                              char* s1, char* s2, char* s3, char* s4)
{
  G_surface* sprite0;
  G_surface* sprite1;
  G_surface* sprite2;
  G_surface* sprite3;
  
  sprite0 = ecran_joueurChargementSprite(ecran, s1);
  sprite1 = ecran_joueurChargementSprite(ecran, s2);
  sprite2 = ecran_joueurChargementSprite(ecran, s3);
  sprite3 = ecran_joueurChargementSprite(ecran, s4);
  
  ecran->ensemble_sprite->mur_qui_bouge[0] = sprite0;
  ecran->ensemble_sprite->mur_qui_bouge[1] = sprite1;
  ecran->ensemble_sprite->mur_qui_bouge[2] = sprite2;
  ecran->ensemble_sprite->mur_qui_bouge[3] = sprite3;
}
  
void ecran_joueurChargementSpritePierre (Ecran_joueur* ecran, char* s)
{
  G_surface* sprite;
  sprite = ecran_joueurChargementSprite(ecran, s);
  ecran->ensemble_sprite->pierre[0] = sprite;
} 

void ecran_joueurChargementSpritePierre_type_explosion (Ecran_joueur* ecran, char* s)
{
  G_surface* sprite;
  sprite = ecran_joueurChargementSprite(ecran, s);
  ecran->ensemble_sprite->bombe[0] = sprite;
} 


void ecran_joueurChargementSpriteDiamant (Ecran_joueur* ecran, 
                                              char* s1, char* s2, char* s3, char* s4)
{
  G_surface* sprite0;
  G_surface* sprite1;
  G_surface* sprite2;
  G_surface* sprite3;
  
  sprite0 = ecran_joueurChargementSprite(ecran, s1);
  sprite1 = ecran_joueurChargementSprite(ecran, s2);
  sprite2 = ecran_joueurChargementSprite(ecran, s3);
  sprite3 = ecran_joueurChargementSprite(ecran, s4);
  
  ecran->ensemble_sprite->diamant[0] = sprite0;
  ecran->ensemble_sprite->diamant[1] = sprite1;
  ecran->ensemble_sprite->diamant[2] = sprite2;
  ecran->ensemble_sprite->diamant[3] = sprite3;
}

void ecran_joueurChargementSpriteDiamantTypeTransformation (Ecran_joueur* ecran, 
                                              char* s1, char* s2, char* s3, char* s4)
{
  G_surface* sprite0;
  G_surface* sprite1;
  G_surface* sprite2;
  G_surface* sprite3;
  
  sprite0 = ecran_joueurChargementSprite(ecran, s1);
  sprite1 = ecran_joueurChargementSprite(ecran, s2);
  sprite2 = ecran_joueurChargementSprite(ecran, s3);
  sprite3 = ecran_joueurChargementSprite(ecran, s4);
  
  ecran->ensemble_sprite->diamant_type_transformation[0] = sprite0;
  ecran->ensemble_sprite->diamant_type_transformation[1] = sprite1;
  ecran->ensemble_sprite->diamant_type_transformation[2] = sprite2;
  ecran->ensemble_sprite->diamant_type_transformation[3] = sprite3;
}


void ecran_joueurChargementSpriteEnnemiTypeRien (Ecran_joueur* ecran, 
                                              char* s1, char* s2, char* s3, char* s4)
{
  G_surface* sprite0;
  G_surface* sprite1;
  G_surface* sprite2;
  G_surface* sprite3;
  
  sprite0 = ecran_joueurChargementSprite(ecran, s1);
  sprite1 = ecran_joueurChargementSprite(ecran, s2);
  sprite2 = ecran_joueurChargementSprite(ecran, s3);
  sprite3 = ecran_joueurChargementSprite(ecran, s4);
  
  ecran->ensemble_sprite->ennemi_type_rien[0] = sprite0;
  ecran->ensemble_sprite->ennemi_type_rien[1] = sprite1;
  ecran->ensemble_sprite->ennemi_type_rien[2] = sprite2;
  ecran->ensemble_sprite->ennemi_type_rien[3] = sprite3;
}

void ecran_joueurChargementSpriteEnnemiTypeDiamant (Ecran_joueur* ecran, 
                                              char* s1, char* s2) /*, char* s3, char* s4)*/
{
  G_surface* sprite0;
  G_surface* sprite1;
 /* G_surface* sprite2;
  G_surface* sprite3;
  */
  sprite0 = ecran_joueurChargementSprite(ecran, s1);
  sprite1 = ecran_joueurChargementSprite(ecran, s2);
/*  sprite2 = ecran_joueurChargementSprite(ecran, s3);
  sprite3 = ecran_joueurChargementSprite(ecran, s4);
  */
  ecran->ensemble_sprite->ennemi_type_diamant[0] = sprite0;
  ecran->ensemble_sprite->ennemi_type_diamant[1] = sprite1;
 /* ecran->ensemble_sprite->ennemi_type_diamant[2] = sprite2;
  ecran->ensemble_sprite->ennemi_type_diamant[3] = sprite3;
*/
}

void ecran_joueurChargementSpriteExplosion (Ecran_joueur* ecran, 
                                              char* s1, char* s2, char* s3) /*, char* s4)*/
{
  G_surface* sprite0;
  G_surface* sprite1;
  G_surface* sprite2;
  /*G_surface* sprite3;
  */
  sprite0 = ecran_joueurChargementSprite(ecran, s1);
  sprite1 = ecran_joueurChargementSprite(ecran, s2);
  sprite2 = ecran_joueurChargementSprite(ecran, s3);
/*  sprite3 = ecran_joueurChargementSprite(ecran, s4);
  */
  ecran->ensemble_sprite->explosion[0] = sprite0;
  ecran->ensemble_sprite->explosion[1] = sprite1;
  ecran->ensemble_sprite->explosion[2] = sprite2;
/*  ecran->ensemble_sprite->ennemi_type_diamant[3] = sprite3;
*/
}


void ecran_joueurChargementSpriteVide (Ecran_joueur* ecran, char* s)
{
  G_surface* sprite;
  sprite = ecran_joueurChargementSprite(ecran, s);
  ecran->ensemble_sprite->vide[0] = sprite;
} 

void ecran_joueurChargementSpriteTerre (Ecran_joueur* ecran, char* s)
{
  G_surface* sprite;
  sprite = ecran_joueurChargementSprite(ecran, s);
  ecran->ensemble_sprite->terre[0] = sprite;
} 

void ecran_joueurChargementSpriteBord (Ecran_joueur* ecran, char* s, char* s2)
{
  G_surface* sprite;
  G_surface* sprite2;
  
  sprite = ecran_joueurChargementSprite(ecran, s);
  sprite2 = ecran_joueurChargementSprite(ecran, s2);
  ecran->ensemble_sprite->bord[0] = sprite;
  ecran->ensemble_sprite->bord[1] = sprite2;
} 


void ecran_joueurChargementSpriteSlim (Ecran_joueur* ecran, 
                                              char* s1, char* s2)
{
  G_surface* sprite0;
  G_surface* sprite1;

  
  sprite0 = ecran_joueurChargementSprite(ecran, s1);
  sprite1 = ecran_joueurChargementSprite(ecran, s2);
 
  
  ecran->ensemble_sprite->slim[0] = sprite0;
  ecran->ensemble_sprite->slim[1] = sprite1;

}


