
#include "../type_explosion/explosion.h"
#include "../../liste/liste.h"
#include "liste_explosion.h"

/*constructeur et destructeur*/
void liste_explosionInitialiser (Liste_explosion* liste)
{
  listeInitialiser(liste);     
}


void liste_explosionDetruire (Liste_explosion* liste)
{
  listeDetruire(liste);    
}


/*modificateurs d'états*/
/*void liste_explosionAjouter (Liste_explosion* liste, int position,Explosion* element)
{
  listeAjouter(liste, position, (Explosion*) element);     
}
*/
void liste_explosionRetirer (Liste_explosion* liste, int position)
{
  listeRetirer(liste, position);     
}

void liste_explosionAjouterFin(Liste_explosion* liste, Explosion* element)
{
  listeAjouterFin(liste, (Explosion*) element);     
}
/*
void liste_explosionEcrire(Liste_explosion* liste, int position, Explosion* element)
{
     listeEcrire(liste, position, (Explosion*) element);
}
*/
/*accesseurs*/

int liste_explosionTaille (Liste_explosion* liste)
{
  return listeTaille(liste);
}



Explosion* liste_explosionLire(Liste_explosion* liste, int position)
{
  return (Explosion*) listeLire(liste, position);             
}
#include <stdio.h>

int liste_explosionRechercher(Liste_explosion* liste, int x, int y)
{
 int i = 0;
 Explosion* explosion;

 for (i=1; i<= liste_explosionTaille(liste);i++) {
  explosion = liste_explosionLire(liste, i);     

  if (explosionLireX(explosion) == x && explosionLireY(explosion) ==y)   
    return i;
 }       
 return 0;
        
}
