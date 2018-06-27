#ifndef _LISTE_JOUEUR_H
#define _LISTE_JOUEUR_H
/*typedef Joueur ElementListe_joueur */
typedef Liste Liste_joueur;

/*constructeur et destructeur*/
void liste_joueurInitialiser (Liste_joueur* liste);
void liste_joueurDetruire (Liste_joueur* liste);

/*modificateurs d'états*/
/*void liste_joueurAjouter (Liste_joueur* liste, int position, Joueur* element);*/
void liste_joueurRetirer (Liste_joueur* liste, int position);
void liste_joueurAjouterFin(Liste_joueur* liste, Joueur* element);
/*void liste_joueurEcrire(Liste_joueur* liste, int position, Joueur* element);*/

/*accesseurs*/
Joueur* liste_joueurLire (Liste_joueur* liste, int position);
int liste_joueurTaille (Liste_joueur* liste);
/*int liste_joueurEstVide (Liste_joueur* liste);*/
Joueur* liste_joueurLire(Liste_joueur* liste, int position);



int liste_joueurRechercher(Liste_joueur* liste, int x, int y);
#endif
