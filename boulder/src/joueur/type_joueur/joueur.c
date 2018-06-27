#include <malloc.h>

#include "../../evenement/touche/touche.h"

#include "joueur.h"


/* module gérant les joueurs*/

Joueur* joueurCreer ()
{
  Joueur* joueur =  malloc(sizeof(Joueur)); 
  if (joueur == NULL)
  {
             fprintf(stderr, "Probleme allocation memoire pour joueur");
             return NULL;
  }
  joueurEcrireDiamant(joueur, 0);
  joueurEcrireDirection(joueur, JOUEUR_DIRECTION_BOUGE_PAS);
  joueurEcrireCompteurTransformation(joueur, 0);
  joueurEcrireActif(joueur, 1);
  joueurEcrireEnTrainDePousser(joueur, 1);
  joueurEcrireForme(joueur, JOUEUR_FORME_BOULDER);
  joueurEcrireCompteurPousser(joueur, 0);
  joueurEcrireVie(joueur, NOMBRE_VIE_DEBUT);
  
  return joueur;  
}

void joueurDetruire(Joueur* joueur)
{
  free(joueur);
}



int joueurLireX (Joueur* joueur)
{
  return joueur->x;
}

int joueurLireY (Joueur* joueur)
{
  return joueur->y;
}

void joueurEcrireX(Joueur* joueur, int x)
{
  joueur->x = x;
}

void joueurEcrireY(Joueur* joueur, int y)
{
   joueur->y = y;
}

Joueur_direction joueurLireDirection(Joueur* joueur)
{
  return (joueur->direction);
}

void joueurEcrireDirection(Joueur* joueur, Joueur_direction direction)
{
  joueur->direction = direction;     
}



int joueurLireVie (Joueur* joueur)
{
 return joueur->vie;   
}

void joueurEcrireVie(Joueur* joueur, int vie)
{
  joueur->vie = vie;    
}

int joueurLireDynamite (Joueur* joueur)
{
 return joueur->dynamite;   
}

void joueurEcrireDynamite(Joueur* joueur, int dynamite)
{
  joueur->dynamite = dynamite;    
}

int joueurLireDiamant (Joueur* joueur)
{
 return joueur->diamant;   
}

void joueurEcrireDiamant(Joueur* joueur, int diamant)
{
  joueur->diamant = diamant;    
}



int joueurLireCompteurTransformation (Joueur* joueur)
{
 return joueur->compteur_transformation;   
}

void joueurEcrireCompteurTransformation(Joueur* joueur, int compteur)
{
  joueur->compteur_transformation = compteur;    
}

short joueurLireActif (Joueur* joueur)
{
 return joueur->actif;   
}

void joueurEcrireActif(Joueur* joueur, short actif)
{
  joueur->actif = actif;    
}

int joueurLireEnTrainDePousser (Joueur* joueur)
{
    return joueur->en_train_de_pousser;
}

void joueurEcrireEnTrainDePousser(Joueur* joueur, int pousser)
{
     joueur->en_train_de_pousser = pousser;
}

int joueurLireCompteurPousser (Joueur* joueur)
{
    return joueur->compteur_pousser;
}

void joueurEcrireCompteurPousser(Joueur* joueur, int compteur)
{
     joueur->compteur_pousser =compteur;
}


Joueur_forme joueurLireForme (Joueur* joueur)
{
   return joueur->forme;
}

void joueurEcrireForme (Joueur* joueur, Joueur_forme forme)
{
     joueur->forme = forme;
}

Joueur_etat joueurLireEtat(Joueur* joueur)
{
  return joueur->etat;
}

void joueurEcrireEtat(Joueur* joueur, Joueur_etat etat)
{
     joueur->etat = etat;
}


/* récupere la touche (au format SDLK)
 * chaque joueur dispose de possibilités de touche qui sont convertis en SDLK = Touche
 *
 */
Touche joueurLireTouche (Joueur* joueur, Touche_type type)
{
  switch (type) {
         case TOUCHE_DROITE:
             return joueur->touche_droite;       
         case TOUCHE_GAUCHE:
             return joueur->touche_gauche;
         case TOUCHE_HAUT:
             return joueur->touche_haut;
         case TOUCHE_BAS:
             return joueur->touche_bas;
         case TOUCHE_AUTOSUICIDE:
             return joueur->touche_autosuicide;
         case TOUCHE_COMBINER:
             return joueur->touche_combiner;
         case TOUCHE_DYNAMITE:
             return joueur->touche_dynamite;
         case TOUCHE_TRANSFORMER:
             return joueur->touche_transformer;
 
  }      
  return 0;     
}

/* Ecrit la touche*/
void joueurEcrireTouche (Joueur* joueur, Touche_type type, Touche touche)
{
    if (joueur==NULL)
    {
       fprintf(stderr, "\n* joueurEcrireTouche: joueur pointant sur NULL\n");
       return;
    } 
     
    switch (type) {
         case TOUCHE_DROITE:
             joueur->touche_droite = touche;       
             break;
         case TOUCHE_GAUCHE:
             joueur->touche_gauche = touche;
             break;
         case TOUCHE_HAUT:
             joueur->touche_haut = touche;
             break;
         case TOUCHE_BAS:
             joueur->touche_bas = touche;
             break;
         case TOUCHE_AUTOSUICIDE:
             joueur->touche_autosuicide = touche;
             break;
         case TOUCHE_COMBINER:
             joueur->touche_combiner = touche;
             break;
         case TOUCHE_DYNAMITE:
             joueur->touche_dynamite = touche;
             break;
         case TOUCHE_TRANSFORMER:
             joueur->touche_transformer = touche;
             break;
 
  }      
   return;  
}


/* Permet de savoir si un joueur appuie sur SA touche du haut, du bas..*/
int joueurToucheAppuie(Joueur* joueur, Touche_type touche_type) /*1 si oui 0 sinon*/
{
   return toucheEstAppuyer(joueurLireTouche(joueur, touche_type)) ;
}
