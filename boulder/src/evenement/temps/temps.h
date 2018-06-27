#include <SDL.h>

/* Module du temps (peu évolué), utilise la SDL.
 Permet d'être indépendant de SDL*/

#define SECONDES *1000

typedef  float Temps;


/* permet de récuperer le temps actuel*/

Temps tempsRecuperer();

/* booléen pour savoir si un temps a est inférieur à un temps b*/
short tempsInferieur(Temps a, Temps b);

/* additionne deux temps*/
Temps tempsSomme(Temps a, Temps b);

/* attend un certain temps*/
void  tempsAttendre(Temps a);
