
#include "../type_diamant/diamant.h"
#include "../../liste/liste.h"
#include "liste_diamant.h"
#include <stdio.h>

/*constructeur et destructeur*/
void liste_diamantInitialiser (Liste_diamant* liste)
{
  listeInitialiser(liste);     
}


void liste_diamantDetruire (Liste_diamant* liste)
{
  int i;
  Diamant* diamant;
  for (i=1; i<= liste_diamantTaille(liste); i++)
  {
      diamant = liste_diamantLire(liste, i);
      diamantDetruire(diamant);
  }
  listeDetruire(liste);    
}


/*modificateurs d'états*/
/*void liste_diamantAjouter (Liste_diamant* liste, int position,Diamant* element)
{
  listeAjouter(liste, position, (Diamant*) element);     
}
*/
void liste_diamantRetirer (Liste_diamant* liste, int position)
{
 // fprintf(stderr, " diamant= %d et taille= %d", position, liste_diamantTaille(liste));
  listeRetirer(liste, position);     
}

void liste_diamantAjouterFin(Liste_diamant* liste, Diamant* element)
{
  listeAjouterFin(liste, (Diamant*) element);     
}
/*
void liste_diamantEcrire(Liste_diamant* liste, int position, Diamant* element)
{
     listeEcrire(liste, position, (Diamant*) element);
}
*/
/*accesseurs*/

int liste_diamantTaille (Liste_diamant* liste)
{
  return listeTaille(liste);
}



Diamant* liste_diamantLire(Liste_diamant* liste, int position)
{
  return (Diamant*) listeLire(liste, position);             
}


int liste_diamantRechercher(Liste_diamant* liste, int x, int y)
{
 int i = 0;
 Diamant* diamant;

 for (i=1; i<= liste_diamantTaille(liste);i++) {
  diamant = liste_diamantLire(liste, i);     
  if (diamantLireType(diamant)==DIAMANT_TYPE_ABSENT)
    continue;
    
  if (diamantLireX(diamant) == x && diamantLireY(diamant) ==y)   
    return i;
 }       
//fprintf(stderr, "recherche non trouvé à : %d %d ", x, y);
 return 0;
        
}
