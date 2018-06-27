#ifndef _MUR_QUI_BOUGE_H
#define _MUR_QUI_BOUGE_H

/* module de base gérant les murs qui bouge*/


/* état du mur*/
typedef enum {
        MUR_QUI_BOUGE_ETAT_ABSENT,
        MUR_QUI_BOUGE_ETAT_EN_COURS,
        MUR_QUI_BOUGE_ETAT_PAS_COMMENCER,
        MUR_QUI_BOUGE_ETAT_FINI
        } Mur_qui_bouge_etat;
        
typedef struct {
        Mur_qui_bouge_etat etat;
        int compteur_graphique;
        int tour_de_boucle_restant;
        
        /*private*/
        Temps private_temps;
        } Mur_qui_bouge;
        
/*initialisateur*/
void mur_qui_bougeInitialiser (Mur_qui_bouge* mur);

      /*accesseur*/ 
int mur_qui_bougeLireCompteurGraphique(Mur_qui_bouge* mur);
void mur_qui_bougeEcrireCompteurGraphique( Mur_qui_bouge* mur, int i);



Mur_qui_bouge_etat mur_qui_bougeLireEtat(Mur_qui_bouge* mur);
void mur_qui_bougeEcrireEtat(Mur_qui_bouge* mur, Mur_qui_bouge_etat etat);

void mur_qui_bougeEcrireTour( Mur_qui_bouge* mur, int i);

void mur_qui_bougeEvoluerTemps (Mur_qui_bouge* mur);

#endif


