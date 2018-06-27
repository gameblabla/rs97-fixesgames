#ifndef _LISTE_ENNEMI_H
#define _LISTE_ENNEMI_H
/*typedef Ennemi ElementListe_ennemi */
typedef Liste Liste_ennemi;

/*constructeur et destructeur*/
void liste_ennemiInitialiser (Liste_ennemi* liste);
void liste_ennemiDetruire (Liste_ennemi* liste);

/*modificateurs d'états*/
/*void liste_ennemiAjouter (Liste_ennemi* liste, int position, Ennemi* element);*/
void liste_ennemiRetirer (Liste_ennemi* liste, int position);
void liste_ennemiAjouterFin(Liste_ennemi* liste, Ennemi* element);
/*void liste_ennemiEcrire(Liste_ennemi* liste, int position, Ennemi* element);*/

/*accesseurs*/
Ennemi* liste_ennemiLire (Liste_ennemi* liste, int position);
int liste_ennemiTaille (Liste_ennemi* liste);
/*int liste_ennemiEstVide (Liste_ennemi* liste);*/
Ennemi* liste_ennemiLire(Liste_ennemi* liste, int position);



int liste_ennemiRechercher(Liste_ennemi* liste, int x, int y);
#endif
