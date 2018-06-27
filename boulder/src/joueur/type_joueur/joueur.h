#ifndef _JOUEUR_H
#define _JOUEUR_H
#define NOMBRE_VIE_DEBUT 6

#define JOUEUR1_TOUCHE_BAS T_DOWN
#define JOUEUR1_TOUCHE_HAUT T_UP
#define JOUEUR1_TOUCHE_DROITE T_RIGHT
#define JOUEUR1_TOUCHE_GAUCHE T_LEFT
#define JOUEUR1_TOUCHE_AUTOSUICIDE T_RETURN
#define JOUEUR1_TOUCHE_COMBINER T_LALT


/* L'ensemble des possibilités de mouvement du joueur*/
typedef enum {
        TOUCHE_BAS,
        TOUCHE_GAUCHE,
        TOUCHE_DROITE,
        TOUCHE_HAUT,
        TOUCHE_COMBINER,
        TOUCHE_DYNAMITE,
        TOUCHE_AUTOSUICIDE,
        TOUCHE_TRANSFORMER
        } Touche_type;

/* La direction du joueur*/
typedef enum {
        JOUEUR_DIRECTION_BAS,
        JOUEUR_DIRECTION_GAUCHE,
        JOUEUR_DIRECTION_DROITE,
        JOUEUR_DIRECTION_HAUT,
        JOUEUR_DIRECTION_POUSSE_GAUCHE,
        JOUEUR_DIRECTION_POUSSE_DROIT,
        JOUEUR_DIRECTION_BOUGE_PAS
        } Joueur_direction;
        
/* sa forme*/
typedef enum {
        JOUEUR_FORME_BOULDER,
        JOUEUR_FORME_ENNEMI
        } Joueur_forme;       

/* son état*/
typedef enum {
       JOUEUR_ETAT_MORT,
       JOUEUR_ETAT_VIVANT
       } Joueur_etat;        
        
typedef struct {
        int x;
        int y;
        Joueur_direction direction;
        int numero; /*numero du joueur*/
        int vie; /*nombre de vie*/
        int diamant;
        int compteur_transformation;
        short actif;
        int dynamite;

/*les touches au format SDLK = Touche du joueur*/
        Touche touche_droite;
        Touche touche_gauche;
        Touche touche_haut;
        Touche touche_bas;
        Touche touche_autosuicide;
        Touche touche_combiner;   
        Touche touche_dynamite;
        Touche touche_transformer;
        Joueur_forme forme;
        Joueur_etat etat;
        
/* booléen pour savoir si le joueur pousse une pierre*/
        int en_train_de_pousser;
/* compteur du poussage*/
        int compteur_pousser;
        int compteur_graphique;
} Joueur;




/* constructeur et déstructeur*/
Joueur* joueurCreer ();
void joueurDetruire(Joueur* joueur);

/*accesseur*/
int joueurLireX (Joueur* joueur);
int joueurLireY (Joueur* joueur);
void joueurEcrireX(Joueur* joueur, int x);
void joueurEcrireY(Joueur* joueur, int y);

Joueur_direction joueurLireDirection(Joueur* joueur);
void joueurEcrireDirection(Joueur* joueur, Joueur_direction direction);

int joueurLireVie (Joueur* joueur);
void joueurEcrireVie(Joueur* joueur, int vie);

int joueurLireDynamite (Joueur* joueur);
void joueurEcrireDynamite(Joueur* joueur, int dynamite);

int joueurLireDiamant (Joueur* joueur);
void joueurEcrireDiamant(Joueur* joueur, int diamant);

int joueurLireCompteurTransformation (Joueur* joueur);
void joueurEcrireCompteurTransformation(Joueur* joueur, int transformation);

short joueurLireActif (Joueur* joueur);
void joueurEcrireActif(Joueur* joueur, short actif);

int joueurLireEnTrainDePousser (Joueur* joueur);
void joueurEcrireEnTrainDePousser(Joueur* joueur, int pousser);

int joueurLireCompteurPousser (Joueur* joueur);
void joueurEcrireCompteurPousser(Joueur* joueur, int compteur);

Joueur_forme joueurLireForme (Joueur* joueur);
void joueurEcrireForme (Joueur* joueur, Joueur_forme forme);


/********
 * Fonction très importante, permet de faire abstraction de la touche assignée 
 au joueur
 * Si on veut savoir si un joueur QUELCONQUE appuie sur la touche pooiur aller en bas
  il suffit d'appeler joueurToucheAppuie(joueur, TOUCHE_BAS)
 */
int joueurToucheAppuie(Joueur* joueur, Touche_type touche_type); /*1 si oui 0 sinon*/


Touche joueurLireTouche (Joueur* joueur, Touche_type type);
void joueurEcrireTouche (Joueur* joueur, Touche_type type, Touche touche);

Joueur_etat joueurLireEtat(Joueur* joueur);
void joueurEcrireEtat(Joueur* joueur, Joueur_etat etat);


#endif
