#ifndef _SLIM_H
#define _SLIM_H
typedef int Slim_vitesse;
typedef enum {
        SLIM_TYPE_DIAMANT,
        SLIM_TYPE_PIERRE
        } Slim_type;
        
typedef struct {
        Slim_type type;
        int compteur_graphique;
        int vitesse;
        } Slim;
        
        
int slimLireCompteurGraphique(Slim* mur);
void slimEcrireCompteurGraphique( Slim* mur, int i);

Slim_vitesse slimLireVitesse(Slim* mur);
void slimEcrireVitesse( Slim* mur, Slim_vitesse i);


Slim_type slimLireType(Slim* mur);
void slimEcrireType(Slim* mur, Slim_type type);

#endif

