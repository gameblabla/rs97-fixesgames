#include <stdio.h>
#include <stdlib.h>
#include <SDL_mixer.h>

#include "../evenement/touche/touche.h"
#include "../moteur_graphique/moteur_graphique.h"

#include "menu.h"
Mix_Music* music;

#define TAILLE_TEXTE 10
#define DECALAGE_INTER_TEXTE 2

static void Play_MusS()
{
	Mix_PlayMusic(music, 99);
}

static void Stop_MusS()
{
	Mix_HaltMusic();
}

Menu_resultat menu(G_fenetre* fenetre, Menu_etat etat)
{
  G_surface* fond;
  G_surface* surf;
  G_rectangle rect;
  int x_deb;
  int y_deb;
  
  G_surface* stexte;
  G_font* font_menu;
  G_couleur noir;
  G_couleur blanc;
  
  font_menu = g_fontCharger("./data/fonts/times.ttf", TAILLE_TEXTE);
  if (font_menu==NULL)
    fprintf(stderr, "\n** menu: Problème chargement font\n");  
 
  fond = g_surfaceChargerBMP("./data/image/bouldfond.bmp");
  if (fond==NULL)
    fprintf(stderr, "\n** menu: Problème chargement fond\n");
 
  surf = g_fenetreLireSurface(fenetre);
  noir =  g_couleurCreer(surf, 255,255,255);
  blanc =  g_couleurCreer(surf, 0,0,0);
 
  x_deb =  (g_surfaceLongueurx(surf)-g_surfaceLongueurx(fond))/2;
  y_deb = 200;
  
  g_rectangleInitialiser(&rect, x_deb,0,0,0);
  g_surfaceCopie(fond, NULL, surf, &rect);
  
  music = Mix_LoadMUS("./boulder.ogg");
  
  
  switch(etat)
  {
    case MENU_INTRODUCTION: 
    
		 Play_MusS();
		
         g_rectangleInitialiser(&rect, x_deb, y_deb ,0,0);
         stexte = g_surfaceEcrireTexte(font_menu, "Press B to start a normal game", blanc, noir);
         g_surfaceCopie(stexte, NULL, surf, &rect);
    
         g_rectangleInitialiser(&rect, x_deb, y_deb+TAILLE_TEXTE+DECALAGE_INTER_TEXTE ,0,0);
         stexte = g_surfaceEcrireTexte(font_menu, "Press X for alternative game mode", blanc, noir);
         g_surfaceCopie(stexte, NULL, surf, &rect);
  
         g_rectangleInitialiser(&rect, x_deb, y_deb+2*TAILLE_TEXTE+2*DECALAGE_INTER_TEXTE ,0,0);
         stexte = g_surfaceEcrireTexte(font_menu, "Press Select to quit the game", blanc, noir);
         g_surfaceCopie(stexte, NULL, surf, &rect);
  
         g_fenetreAfficher(fenetre);
            
     
        while(1)
        {        
			if (toucheEstAppuyer(T_ESCAPE))
			{
				Stop_MusS();
				Mix_FreeMusic(music);
				return MENU_QUITTER;
			}
			if (toucheEstAppuyer(T_LALT))
			{
				Stop_MusS();
				return MENU_UN_JOUEUR;
			}
			if (toucheEstAppuyer(T_SPACE))
			{
				Stop_MusS();
				return MENU_UN_JOUEUR_MODE_FLO;
			}
        }
        

        break;

    case MENU_NIVEAU_PERDU: 
		Play_MusS();
		g_rectangleInitialiser(&rect, x_deb, y_deb ,0,0);
		stexte = g_surfaceEcrireTexte(font_menu, "Press B to restart", blanc, noir);
		g_surfaceCopie(stexte, NULL, surf, &rect);
		g_rectangleInitialiser(&rect, x_deb, y_deb+TAILLE_TEXTE+DECALAGE_INTER_TEXTE ,0,0);
		stexte = g_surfaceEcrireTexte(font_menu, "Press Select to quit", blanc, noir);
		g_surfaceCopie(stexte, NULL, surf, &rect);
		g_fenetreAfficher(fenetre);
        while(1)
        {        
			if (toucheEstAppuyer(T_ESCAPE))
			{
				Stop_MusS();
				Mix_FreeMusic(music);
				return MENU_QUITTER;
			}
			if (toucheEstAppuyer(T_LALT))
			{
				Stop_MusS();
				return MENU_CONTINU;
			}
        }
        
        break;
     case MENU_GAME_OVER_PERD: 
		Play_MusS();
		g_rectangleInitialiser(&rect, x_deb, y_deb ,0,0);
		stexte = g_surfaceEcrireTexte(font_menu, "GAME OVER, you lost", blanc, noir);
		g_surfaceCopie(stexte, NULL, surf, &rect);
    
		g_rectangleInitialiser(&rect, x_deb, y_deb+TAILLE_TEXTE+DECALAGE_INTER_TEXTE ,0,0);
		stexte = g_surfaceEcrireTexte(font_menu, "Press Select to quit", blanc, noir);
		g_surfaceCopie(stexte, NULL, surf, &rect);
  
		g_fenetreAfficher(fenetre);
            
		while(1)
        {        
			if (toucheEstAppuyer(T_ESCAPE))
			{
				Stop_MusS();
				Mix_FreeMusic(music);
				return MENU_QUITTER;
			}
        }
        
        break;   
     case MENU_GAME_OVER_GAGNE: 
		Play_MusS();
		g_rectangleInitialiser(&rect, x_deb, y_deb ,0,0);
		stexte = g_surfaceEcrireTexte(font_menu, "Congrats, you won", blanc, noir);
		g_surfaceCopie(stexte, NULL, surf, &rect);
    
		g_rectangleInitialiser(&rect, x_deb, y_deb+TAILLE_TEXTE+DECALAGE_INTER_TEXTE ,0,0);
		stexte = g_surfaceEcrireTexte(font_menu, "Press Select to quit", blanc, noir);
		g_surfaceCopie(stexte, NULL, surf, &rect);
		g_fenetreAfficher(fenetre);
		
		while(1)
		{        
			if (toucheEstAppuyer(T_ESCAPE))
			{
				Stop_MusS();
				Mix_FreeMusic(music);
				return MENU_QUITTER;
			}
		}
        
        break;   
    case MENU_NIVEAU_REUSSI: 
		Play_MusS();
		g_rectangleInitialiser(&rect, x_deb, y_deb ,0,0);
		stexte = g_surfaceEcrireTexte(font_menu, "Press B to advance to the next level", blanc, noir);
		g_surfaceCopie(stexte, NULL, surf, &rect);
    
		g_rectangleInitialiser(&rect, x_deb, y_deb+TAILLE_TEXTE+DECALAGE_INTER_TEXTE ,0,0);
		stexte = g_surfaceEcrireTexte(font_menu, "Press Select to quit", blanc, noir);
		g_surfaceCopie(stexte, NULL, surf, &rect);
  
  
		g_fenetreAfficher(fenetre);
            
     
		while(1)
		{        
			if (toucheEstAppuyer(T_ESCAPE))
			{
				Stop_MusS();
				Mix_FreeMusic(music);
				return MENU_QUITTER;
			}
			if (toucheEstAppuyer(T_LALT))
			{
				Stop_MusS();
				return MENU_CONTINU;
			}
        }
        
        break;                  
		default:
			return  MENU_QUITTER;
    
	}  

    
}
