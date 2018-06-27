#ifndef _LISTE_PIERRE_H
#define _LISTE_PIERRE_H
/*typedef Pierre ElementListe_pierre */
typedef Liste Liste_pierre;

/*constructeur et destructeur*/
void liste_pierreInitialiser (Liste_pierre* liste);
void liste_pierreDetruire (Liste_pierre* liste);

/*modificateurs d'états*/
/*void liste_pierreAjouter (Liste_pierre* liste, int position, Pierre* element);*/
void liste_pierreRetirer (Liste_pierre* liste, int position);
void liste_pierreAjouterFin(Liste_pierre* liste, Pierre* element);
/*void liste_pierreEcrire(Liste_pierre* liste, int position, Pierre* element);*/

/*accesseurs*/
Pierre* liste_pierreLire (Liste_pierre* liste, int position);
int liste_pierreTaille (Liste_pierre* liste);
/*int liste_pierreEstVide (Liste_pierre* liste);*/
Pierre* liste_pierreLire(Liste_pierre* liste, int position);



int liste_pierreRechercher(Liste_pierre* liste, int x, int y);
#endif
