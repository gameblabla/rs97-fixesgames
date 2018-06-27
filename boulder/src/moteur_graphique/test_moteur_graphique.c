#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../evenement/touche/touche.h"

#include "../moteur_graphique/moteur_graphique.h"

void oki () { fprintf(stderr, " = OK\n"); }

void test_surface ()
{

  G_surface* surface;
  fprintf(stderr, "**Test de surface");
  
 
  surface = g_surfaceCreer(10,10);   
  
  g_surfaceDetruire(surface);

  oki ();
     
     
}


void test_chargement_fenetre ()
{
 G_fenetre fenetre;
 
  fprintf(stderr, "**Test de fenetre");
  
  g_fenetreInitialiser(&fenetre, 50,50, PAS_PLEIN_ECRAN);
 
 g_fenetreDetruire(&fenetre);
//while(!toucheEstAppuyer(T_SPACE));
  oki ();
     
     
}

void test_fonts ()
{
 G_fenetre fenetre;
 G_surface* surface;
 G_font* font;
 FILE* f;
 
  fprintf(stderr, "**Test de des fonts"); 
 f = fopen("./times.ttf", "r");
 if (f==NULL)
   fprintf(stderr, "ddd");
 font = g_fontCharger("./times.ttf", 15);
 if (font==NULL)
  fprintf(stderr, "Erreur ouverture du font");
 g_fenetreInitialiser(&fenetre, 500,500,PAS_PLEIN_ECRAN);
 
 surface= g_surfaceEcrireTexte(font, "hello", g_couleurCreer(g_fenetreLireSurface(&fenetre),255,255,255),
                                          g_couleurCreer(g_fenetreLireSurface(&fenetre),0,0,0));
                                          
 g_surfaceCopie(surface, NULL, g_fenetreLireSurface(&fenetre), NULL);
 g_fenetreAfficher(&fenetre);
while(!toucheEstAppuyer(T_SPACE));
  
  
 
 g_fenetreDetruire(&fenetre);
//while(!toucheEstAppuyer(T_SPACE));
  oki ();
     
     
}

int main ()
{
    fprintf(stderr, "=== TEST MODULE MOTEUR GRAPHIQUE===\n");
  //  test_surface ();
  //  test_chargement_fenetre ();
   test_fonts ();
    
    
    
    
    fprintf(stderr, " MODULE OK\n");
 return 0;
}
