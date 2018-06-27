#ifndef _EXPLOSION_H
#define _EXPLOSION_H

/*
  Etat de progression ou de régression de l'explosion*/
typedef enum {
        PROGRESSION,
        REGRESSION
        } Explosion_etat;


/* Pour savoir en quoi doit être transformé l'explosion*/
typedef enum {
        EXPLOSION_TYPE_PIERRE,
        EXPLOSION_TYPE_DIAMANT,
        EXPLOSION_TYPE_RIEN,
        EXPLOSION_TYPE_ABSENT
        
        } Explosion_type;        

        
        
        
        
/* structure de l'explosion*/
typedef struct {
        int x;
        int y;
        Explosion_type type;
        Explosion_etat etat;
        int compteur;
        int duree;
} Explosion;





Explosion* explosionCreer ();
void explosionDetruire(Explosion* explosion);


int explosionLireX (Explosion* explosion);
int explosionLireY (Explosion* explosion);
void explosionEcrireX(Explosion* explosion, int x);
void explosionEcrireY(Explosion* explosion, int y);


Explosion_type explosionLireType(Explosion* explosion);
void explosionEcrireType(Explosion* explosion, Explosion_type type);

Explosion_etat explosionLireEtat (Explosion* explosion);
void explosionEcrireEtat(Explosion* explosion, Explosion_etat etat);

int explosionLireCompteur (Explosion* explosion);
void explosionEcrireCompteur(Explosion* explosion, int compteur);

int explosionLireDuree (Explosion* explosion);
void explosionEcrireDuree(Explosion* explosion, int duree);
#endif
