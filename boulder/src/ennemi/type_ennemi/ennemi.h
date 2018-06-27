#ifndef _ENNEMI_H
#define _ENNEMI_H
/* Module permettant la gestion des ennemis*/

/* Direction actuel de l'ennemi*/

typedef enum {
        ENNEMI_DIRECTION_BAS,
        ENNEMI_DIRECTION_GAUCHE,
        ENNEMI_DIRECTION_DROITE,
        ENNEMI_DIRECTION_HAUT,
        ENNEMI_DIRECTION_BOUGE_PAS
        } Ennemi_direction;
        
/* type d'ennemi*/
typedef enum {
        ENNEMI_TYPE_RIEN, /*et explosion*/
        ENNEMI_TYPE_DIAMANT, /*et explosion*/
        ENNEMI_TYPE_PIERRE,
        ENNEMI_TYPE_ABSENT
        } Ennemi_type;
        

        
        
        
/* position type et direction de l'ennemi, le compteur ne sert pas*/
        
typedef struct {
        int x;
        int y;
        Ennemi_direction direction;
        Ennemi_type type;
        int compteur_graphique;
} Ennemi;




/*Constructeur et déstructeur*/
Ennemi* ennemiCreer ();
void ennemiDetruire(Ennemi* ennemi);

/*Accesseurs*/
int ennemiLireX (Ennemi* ennemi);
int ennemiLireY (Ennemi* ennemi);
void ennemiEcrireX(Ennemi* ennemi, int x);
void ennemiEcrireY(Ennemi* ennemi, int y);


Ennemi_direction ennemiLireDirection(Ennemi* ennemi);
void ennemiEcrireDirection(Ennemi* ennemi, Ennemi_direction direction);

Ennemi_type ennemiLireType (Ennemi* ennemi);
void ennemiEcrireType(Ennemi* ennemi, Ennemi_type type);

int ennemiLireCompteurGraphique(Ennemi* mur);
void ennemiEcrireCompteurGraphique( Ennemi* mur, int i);

void ennemiCopie(Ennemi* ennemi, int i, int j, Ennemi_type type, Ennemi_direction direction);

#endif
