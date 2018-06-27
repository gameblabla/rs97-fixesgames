#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "../moteur_graphique/moteur_graphique.h"

#include "../fichier/fichier.h"
#include "../niveau/niveau.h"
#include "../environnement/environnement.h"


#include "boulder_graphique.h"

static G_couleur char2couleur (Ecran_joueur* ecran, char c);

/* Les sprites sont définis comme cela :
       LONGUEURX= 16
LONGUEURY= 16

0000000000000000
0000990000990000
0099009999009900
0099009999009900
0000999999990000
0000009999000000
0000777777770000
0055007777005500
0000559999550000
0000007777000000
0000009999000000
0000557777550000
0000550000550000
0000550000550000
0000550000550000
0077770000777700
*/
/* 0 va correspondre à la couleur de fond, 5, couleur des jambes, 7 des pieds
  les couleurs ne sont pas définis d'avance, cela dépend du niveau
  comme cela, on utilise qu'un sprite pour plein de niveau avec plein de couleur différentes
 */
 /* Cela préssupose que l'on connaisse déjà les couleurs du niveau
 */  

G_surface*      ecran_joueurChargementSprite(Ecran_joueur* ecran, char* fichier)
{
  Fichierl* f;
  G_surface* sprite;
  int i,j;
  int zi; int zj;
  int x;
  int y;
  char c[50];
  char pixel;
  
  f = fichierlOuvrir(fichier);
  if (f==NULL) {
    fprintf(stderr, "** Probleme fichier inexistant\n");
    return NULL;
  }
   /* On détermine la résolution du sprite, ils seronts tous de taille 16 16*/
  for (i=1; i<=2; i++)
  {
    fichierlLireEgale(f, c);    
    if (strcmp(c, "LONGUEURX"))
    {
      fichierlLireEntier(f, &x);            
    }
    else {
      if (strcmp(c, "LONGUEURY"))
      {
        fichierlLireEntier(f,  &y);
      }
      else
        fprintf(stderr, "**Probleme format fichier sprite");     
         
    }
      
      
  }

 /* on créait notre surface suivant le facteur de zoom*/
  sprite = g_surfaceCreer(x*ecran->zoom, y*ecran->zoom);

   
    /* on écrit sur la surface la bonne couleur*/
  for (j=0; j<y; j++)
   for (i=0; i<x; i++)                    
   {
      
       fichierlLireCaractere(f, &pixel);
       for(zj=1; zj<=ecran->zoom; zj++)
        for (zi=1; zi<=ecran->zoom; zi++)
         g_surfaceEcrireCouleur(sprite, zi-1+i*ecran->zoom, zj-1+j*ecran->zoom, char2couleur(ecran, pixel));
   }      
 fichierlFermer(f);         
 return sprite;                      
}


/* convertit l'écriture du fichier en couleur*/
static G_couleur char2couleur (Ecran_joueur* ecran, char c)
{
 switch (c)
 {
    case '9':
         return ecran->couleurs->couleur_tete;
    case '5':
         return ecran->couleurs->couleur_jambe;
    case '7':
         return ecran->couleurs->couleur_blanc;
    case '2':
         return ecran->couleurs->couleur_moche;
    case '0':
    default:
         return ecran->couleurs->couleur_noir;    
        
        
 }         
 return ecran->couleurs->couleur_noir;           
}


