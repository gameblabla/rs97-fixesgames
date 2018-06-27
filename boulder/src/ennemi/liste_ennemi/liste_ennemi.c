
#include "../type_ennemi/ennemi.h"
#include "../../liste/liste.h"
#include "liste_ennemi.h"

/*constructeur et destructeur*/
void liste_ennemiInitialiser (Liste_ennemi* liste)
{
  listeInitialiser(liste);     
}


void liste_ennemiDetruire (Liste_ennemi* liste)
{
  int i;
  Ennemi* e;
  for (i=1; i<= liste_ennemiTaille(liste); i++)
  {
      e = liste_ennemiLire(liste, i);
      ennemiDetruire(e);
  }
  listeDetruire(liste);    
}


/*modificateurs d'états*/
/*void liste_ennemiAjouter (Liste_ennemi* liste, int position,Ennemi* element)
{
  listeAjouter(liste, position, (Ennemi*) element);     
}
*/
void liste_ennemiRetirer (Liste_ennemi* liste, int position)
{
  listeRetirer(liste, position);     
}

void liste_ennemiAjouterFin(Liste_ennemi* liste, Ennemi* element)
{
  listeAjouterFin(liste, (Ennemi*) element);     
}
/*
void liste_ennemiEcrire(Liste_ennemi* liste, int position, Ennemi* element)
{
     listeEcrire(liste, position, (Ennemi*) element);
}
*/
/*accesseurs*/

int liste_ennemiTaille (Liste_ennemi* liste)
{
  return listeTaille(liste);
}



Ennemi* liste_ennemiLire(Liste_ennemi* liste, int position)
{
  return (Ennemi*) listeLire(liste, position);             
}


int liste_ennemiRechercher(Liste_ennemi* liste, int x, int y)
{
 int i = 0;
 Ennemi* ennemi;

 for (i=1; i<= liste_ennemiTaille(liste);i++) {
  ennemi = liste_ennemiLire(liste, i);     
  if (ennemiLireType(ennemi) == ENNEMI_TYPE_ABSENT)
    continue;
  
  if (ennemiLireX(ennemi) == x && ennemiLireY(ennemi) ==y)   
    return i;
 }       
 return 0;
        
}
