#ifndef _NIVEAU_H
#define _NIVEAU_H



#include "../liste/liste.h"
#include "../evenement/touche/touche.h"
#include "../evenement/temps/temps.h"

#include "../joueur/type_joueur/joueur.h"
#include "../diamant/type_diamant/diamant.h"
#include "../pierre/type_pierre/pierre.h"
#include "../explosion/type_explosion/explosion.h"
#include "../ennemi/type_ennemi/ennemi.h"

#include "../joueur/liste_joueur/liste_joueur.h"
#include "../diamant/liste_diamant/liste_diamant.h"
#include "../pierre/liste_pierre/liste_pierre.h"
#include "../ennemi/liste_ennemi/liste_ennemi.h"
#include "../explosion/liste_explosion/liste_explosion.h"

#include "../slim/slim.h"
#include "../mur_qui_bouge/mur_qui_bouge.h"
#include "../fichier/fichier.h"

/* Topic: Documentation du module Niveau
 * Ce module permet l'utilisation du type abstrait niveau, notamment par la possibilité de charger
 * un niveau du disque dur.
 */

typedef enum {
        ELEMENT_DIAMANT,
        ELEMENT_ENNEMI,
        ELEMENT_JOUEUR,
        ELEMENT_PIERRE,
        ELEMENT_MUR,
        ELEMENT_MUR_QUI_BOUGE,
        ELEMENT_VIDE,
        ELEMENT_TERRE,
        ELEMENT_BORD,
        ELEMENT_SLIM,
        ELEMENT_PORTE_DE_FIN,
        ELEMENT_EXPLOSION
        } Niveau_element;

typedef enum {
        NIVEAU_ETAT_INTRODUCTION,
        NIVEAU_ETAT_EN_COURS,
        NIVEAU_ETAT_GAGNE,
        NIVEAU_ETAT_MORT_DES_JOUEURS,
        NIVEAU_ETAT_PLUS_DE_TEMPS,
        NIVEAU_ETAT_GAME_OVER_GAGNE,
        NIVEAU_ETAT_GAME_OVER_PERD
        } Niveau_etat;

typedef enum {
        NIVEAU_PORTE_FERMEE,
        NIVEAU_PORTE_OUVERTE
        } Niveau_porte;
        
                
typedef struct {
        Niveau_element* matrice;
        int nombre_de_diamant_restant;
        int nombre_de_joueur;
        Liste_pierre liste_pierre;
        Liste_diamant liste_diamant;
        Liste_ennemi liste_ennemi;
        Liste_explosion liste_explosion;
        
        Liste_joueur liste_joueur;
        
        Slim          slim;
        Mur_qui_bouge mur;
        int longueurx;
        int longueury;
        int temps_restant;
        Niveau_etat etat;
        Niveau_porte porte;
        
        /*private*/
        Temps private_temps;
        int numero_niveau;
        } Niveau;
        
        
/**Function: niveauInitialiser
 * Cette fonction initialise un niveau, notamment en initialisant les listes que contient celui-ci
 *
 * Paramètre:
 * niveau - le niveau que l'on souhaite initialiser
 */
void niveauInitialiser(Niveau* niveau);

void niveauReinitialiser(Niveau* niveau);


/**Function: niveauDetruire
 * Cette fonction détruire un niveau, notamment en détruisant toutes les listes qu'il contient
 *
 * Paramètre:
 * niveau - le niveau à détruire
 */
void niveauDetruire(Niveau* niveau);

void niveauDetruireSaufJoueur(Niveau* niveau);
     
/**Function: niveauLireMatrice
 * Permet de lire un élément du niveau se trouvant au position i et j de celui-ci.
 *
 * Paramètres:
 * niveau - le niveau ou l'on souhaite lire
 * i - position horizontale du niveau
 * j - position verticale du niveau
 *
 * Retour:
 * L'élément lu, qui se trouve parmi les valeurs:
 *    -  ELEMENT_DIAMANT,
 *    -  ELEMENT_ENNEMI,
 *    -  ELEMENT_JOUEUR,
 *    -  ELEMENT_PIERRE,
 *    -  ELEMENT_MUR,
 *    -  ELEMENT_MUR_QUI_BOUGE,
 *    -  ELEMENT_VIDE,
 *    -  ELEMENT_TERRE,
 *    -  ELEMENT_BORD,
 *    -  ELEMENT_SLIM,
 *    -  ELEMENT_PORTE_DE_FIN,
 *    -  ELEMENT_EXPLOSION
 *
 * Erreur:
 *  Dans le cas d'une tentative de lecture hors borne, un message d'erreur est affiché sur la sortie
 *  stderr, et ELEMENT_VIDE est renvoyé.
 */
Niveau_element niveauLireMatrice(Niveau* niveau, int i, int j);

/**Function: niveauEcrireMatrice
 *Permet d'écrire un élément dans un niveau à une certaine position
 *
 * Paramètres:
 * niveau - le niveau ou l'on souhaite écrire
 * i - position horizontale du niveau
 * j - position verticale du niveau
 * element - l'élément que l'on souhaite ajouter
 *
 * Erreur:
 *  Dans le cas d'une tentative d'écriture hors borne, un message d'erreur est affiché sur la sortie 
 *  stderr
 */
void niveauEcrireMatrice(Niveau* niveau, int i, int j, Niveau_element element);

/* décremente lenombre de diamant restant à prendre*/
void niveauDecrementerDiamantRestant(Niveau* niveau);

/* récupere le nombre de diamant restants à prendre*/
int  niveauLireNombreDiamantRestant(Niveau* niveau);

/* fait évoluer le temps du niveau*/
void niveauEvoluerTemps(Niveau* niveau);

/* lit le temps restant en int (en seconde)*/
int niveauLireTempsRestant(Niveau* niveau);

/*accesseurs*/
int niveauLireLongueurX(Niveau* niveau);
int niveauLireLongueurY(Niveau* niveau);

Niveau_etat niveauLireEtat(Niveau* niveau);
void niveauEcrireEtat(Niveau* niveau, Niveau_etat etat);


int niveauLireNombreDeJoueur(Niveau* niveau);


/*accesseurs des listes des objets*/
Liste_pierre* niveauLireListePierre(Niveau* niveau);
Liste_diamant* niveauLireListeDiamant(Niveau* niveau);
Liste_ennemi* niveauLireListeEnnemi(Niveau* niveau);
Liste_joueur* niveauLireListeJoueur(Niveau* niveau);
Liste_explosion* niveauLireListeExplosion(Niveau* niveau);

Mur_qui_bouge* niveauLireMurQuiBouge(Niveau* niveau);
Slim* niveauLireSlim(Niveau* niveau);


Niveau_porte niveauLirePorte(Niveau* niveau);

void niveauEcrirePorte(Niveau* niveau, Niveau_porte porte);


/*permet de charger un niveau au format fichier dans un niveau au format structure*/
int niveauChargement(Niveau* niveau, char*); 



#endif
