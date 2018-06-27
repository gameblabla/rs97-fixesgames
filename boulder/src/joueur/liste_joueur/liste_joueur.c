#include "../../evenement/touche/touche.h"

#include "../type_joueur/joueur.h"
#include "../../liste/liste.h"

#include "liste_joueur.h"

/*constructeur et destructeur*/
void liste_joueurInitialiser (Liste_joueur* liste)
{
  listeInitialiser(liste);     
}


void liste_joueurDetruire (Liste_joueur* liste)
{
  int i;
  Joueur* j;
  for (i=1; i<= liste_joueurTaille(liste); i++)
  {
      j = liste_joueurLire(liste, i);
      joueurDetruire(j);
  }
  listeDetruire(liste);    
}


/*modificateurs d'états*/
/*void liste_joueurAjouter (Liste_joueur* liste, int position,Joueur* element)
{
  listeAjouter(liste, position, (Joueur*) element);     
}
*/
void liste_joueurRetirer (Liste_joueur* liste, int position)
{
  listeRetirer(liste, position);     
}

void liste_joueurAjouterFin(Liste_joueur* liste, Joueur* element)
{
  listeAjouterFin(liste, (Joueur*) element);     
}
/*
void liste_joueurEcrire(Liste_joueur* liste, int position, Joueur* element)
{
     listeEcrire(liste, position, (Joueur*) element);
}
*/
/*accesseurs*/

int liste_joueurTaille (Liste_joueur* liste)
{
  return listeTaille(liste);
}



Joueur* liste_joueurLire(Liste_joueur* liste, int position)
{
  return (Joueur*) listeLire(liste, position);             
}


int liste_joueurRechercher(Liste_joueur* liste, int x, int y)
{
 int i = 0;
 Joueur* joueur;

 for (i=1; i<= liste_joueurTaille(liste);i++) {
  joueur = liste_joueurLire(liste, i);     

  if (joueurLireX(joueur) == x && joueurLireY(joueur) ==y)   
    return i;
 }       
 return 0;
        
}
