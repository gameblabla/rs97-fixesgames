
#include "../type_pierre/pierre.h"
#include "../../liste/liste.h"
#include "liste_pierre.h"

/*constructeur et destructeur*/
void liste_pierreInitialiser (Liste_pierre* liste)
{
  listeInitialiser(liste);     
}


void liste_pierreDetruire (Liste_pierre* liste)
{
  int i;
  Pierre* pierre;
  for (i=1; i<= liste_pierreTaille(liste); i++)
  {
      pierre = liste_pierreLire(liste, i);
      pierreDetruire(pierre);
  }
  
  listeDetruire(liste);    
}


/*modificateurs d'états*/
/*void liste_pierreAjouter (Liste_pierre* liste, int position,Pierre* element)
{
  listeAjouter(liste, position, (Pierre*) element);     
}
*/
void liste_pierreRetirer (Liste_pierre* liste, int position)
{
  listeRetirer(liste, position);     
}

void liste_pierreAjouterFin(Liste_pierre* liste, Pierre* element)
{
  listeAjouterFin(liste, (Pierre*) element);     
}
/*
void liste_pierreEcrire(Liste_pierre* liste, int position, Pierre* element)
{
     listeEcrire(liste, position, (Pierre*) element);
}
*/
/*accesseurs*/

int liste_pierreTaille (Liste_pierre* liste)
{
  return listeTaille(liste);
}



Pierre* liste_pierreLire(Liste_pierre* liste, int position)
{
  return (Pierre*) listeLire(liste, position);             
}
#include <stdio.h>

int liste_pierreRechercher(Liste_pierre* liste, int x, int y)
{
 int i = 0;
 Pierre* pierre;

 for (i=1; i<= liste_pierreTaille(liste);i++) {
  pierre = liste_pierreLire(liste, i);     

  if (pierreLireX(pierre) == x && pierreLireY(pierre) ==y)   
    return i;
 }       
 return 0;
        
}
