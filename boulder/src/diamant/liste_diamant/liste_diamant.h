#ifndef _LISTE_DIAMANT_H
#define _LISTE_DIAMANT_H
/*typedef Diamant ElementListe_diamant */
typedef Liste Liste_diamant;

/*constructeur et destructeur*/
void liste_diamantInitialiser (Liste_diamant* liste);
void liste_diamantDetruire (Liste_diamant* liste);

/*modificateurs d'états*/
/*void liste_diamantAjouter (Liste_diamant* liste, int position, Diamant* element);*/
void liste_diamantRetirer (Liste_diamant* liste, int position);
void liste_diamantAjouterFin(Liste_diamant* liste, Diamant* element);
/*void liste_diamantEcrire(Liste_diamant* liste, int position, Diamant* element);*/

/*accesseurs*/
Diamant* liste_diamantLire (Liste_diamant* liste, int position);
int liste_diamantTaille (Liste_diamant* liste);
/*int liste_diamantEstVide (Liste_diamant* liste);*/
Diamant* liste_diamantLire(Liste_diamant* liste, int position);



int liste_diamantRechercher(Liste_diamant* liste, int x, int y);
#endif
