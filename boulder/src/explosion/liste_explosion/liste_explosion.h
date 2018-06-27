#ifndef _LISTE_EXPLOSION_H
#define _LISTE_EXPLOSION_H
/*typedef Explosion ElementListe_explosion */
typedef Liste Liste_explosion;

/*constructeur et destructeur*/
void liste_explosionInitialiser (Liste_explosion* liste);
void liste_explosionDetruire (Liste_explosion* liste);

/*modificateurs d'états*/
/*void liste_explosionAjouter (Liste_explosion* liste, int position, Explosion* element);*/
void liste_explosionRetirer (Liste_explosion* liste, int position);
void liste_explosionAjouterFin(Liste_explosion* liste, Explosion* element);
/*void liste_explosionEcrire(Liste_explosion* liste, int position, Explosion* element);*/

/*accesseurs*/
Explosion* liste_explosionLire (Liste_explosion* liste, int position);
int liste_explosionTaille (Liste_explosion* liste);
/*int liste_explosionEstVide (Liste_explosion* liste);*/
Explosion* liste_explosionLire(Liste_explosion* liste, int position);



int liste_explosionRechercher(Liste_explosion* liste, int x, int y);
#endif
