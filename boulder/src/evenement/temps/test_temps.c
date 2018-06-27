#include "temps.h"


int main ()
{
  Temps t;	
 int i;
  t = tempsRecuperer();
  
  
  
  while(tempsInferieur(tempsRecuperer(), tempsSomme(t, 1 SECONDES))==1);      

for (i=1;i<=1000;i++) {
 t = tempsRecuperer();
 while(tempsInferieur(tempsRecuperer(), tempsSomme(t, 0.05 SECONDES))==1);  
 fprintf(stderr, "temps= %d\n", (int) -(t-tempsRecuperer()));// ecran->j_originex--;
}
  return 0;  
}
