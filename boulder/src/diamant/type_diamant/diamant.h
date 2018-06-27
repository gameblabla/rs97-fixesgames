#ifndef _DIAMANT_H
#define _DIAMANT_H

/* Module permettant la gestion des diamants*/


/* La direction actuelle du diamant*/
typedef enum {
        DIAMANT_DIRECTION_BAS,
        DIAMANT_DIRECTION_GAUCHE,
        DIAMANT_DIRECTION_DROITE,
        DIAMANT_DIRECTION_HAUT,
        DIAMANT_DIRECTION_BOUGE_PAS
        } Diamant_direction;
        
        
/* Pour savoir si le diamant explose après la chute ou non*/
typedef enum {
        DIAMANT_TRANSFORMATION_DIAMANT,
        DIAMANT_TRANSFORMATION_EXPLOSION
        } Diamant_transformation_chute;
        
/* le type de diamant*/
typedef enum {
        DIAMANT_TYPE_DIAMANT,
        DIAMANT_TYPE_TRANSFORMATION,
        DIAMANT_TYPE_ABSENT
        } Diamant_type;
        
        
        
        
/* la position du diamant, son prix, (le compteur n'est en fait pas utilisé),
   sa direction et son type*/
typedef struct {
        int x;
        int y;
        Diamant_direction direction;
        Diamant_transformation_chute transformation;
        Diamant_type type;
        int valeur;
        int compteur_graphique;
} Diamant;


/* Constructeur*/
Diamant* diamantCreer ();

/*Destructeur*/
void diamantDetruire(Diamant* diamant);


/*Accesseurs*/
int diamantLireX (Diamant* diamant);
int diamantLireY (Diamant* diamant);
void diamantEcrireX(Diamant* diamant, int x);
void diamantEcrireY(Diamant* diamant, int y);


Diamant_direction diamantLireDirection(Diamant* diamant);
void diamantEcrireDirection(Diamant* diamant, Diamant_direction direction);

Diamant_transformation_chute diamantLireTransformationChute (Diamant* diamant);
void diamantEcrireTransformationChute(Diamant* diamant, Diamant_transformation_chute transformation);

int diamantLireValeur (Diamant* diamant);
void diamantEcrireValeur(Diamant* diamant, int valeur);

void diamantEcrireType(Diamant* diamant, Diamant_type type);
Diamant_type diamantLireType(Diamant* diamant);

int diamantLireCompteurGraphique(Diamant* mur);
void diamantEcrireCompteurGraphique( Diamant* mur, int i);

/*Permet d'écrire l'ensemble des données directement sur un diamant*/
void diamantCopie(Diamant* diamant, int i, int j, Diamant_type type, Diamant_transformation_chute chute);
#endif
