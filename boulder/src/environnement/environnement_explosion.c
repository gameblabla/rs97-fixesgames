#include <stdio.h>

#include "../niveau/niveau.h"
#include "environnement.h"

/*fonction privée, souvent utilisée*/
/* Ajoute une explosion de la taille d'un sprite d'un certain type*/
void environnementAjouterExplosion(Niveau* niveau, Explosion_type type, int i, int j)
{
  Explosion * explosion;
  Liste_explosion* liste = niveauLireListeExplosion(niveau);
  niveauEcrireMatrice(niveau, i, j, ELEMENT_EXPLOSION);
  explosion = explosionCreer();
  explosionEcrireX(explosion, i);
  explosionEcrireY(explosion, j);
  explosionEcrireType(explosion, type);
  explosionEcrireEtat(explosion, PROGRESSION); /* l'explosionj commence, il est en progression*/       
  liste_explosionAjouterFin(liste, explosion);     
  return;   
}

/* vidage de la liste si c'est une explosion de type absent*/
void environnementListeExplosionVidage( Niveau* niveau)
{
 int i;
 Liste_explosion* liste = niveauLireListeExplosion(niveau);
 Explosion* explosion;
 for (i=1; i<= liste_explosionTaille(liste); i++)
  {
    explosion = liste_explosionLire(liste, i);
    if (explosionLireType(explosion) == EXPLOSION_TYPE_ABSENT) 
     {
   //    liste_explosionRetirer(liste, i);
   //    i--;
     }
  }     
     
     
}

/* créer une explosion de taille 1 1 au position i, j, cela dépend de l'élément,
   par exemple, si c'est au bord du niveau, cela ne fait rien*/
void environnementCreerExplosion11(Niveau* niveau, Explosion_type type, int i, int j)
{
  int recherche;
  Pierre* pierre;
  Diamant* diamant;
  Ennemi* ennemi;
 //  fprintf(stderr ," i=%d, j=%d \n", i, j);
  switch(niveauLireMatrice(niveau, i, j)){
   case ELEMENT_BORD:
   case ELEMENT_PORTE_DE_FIN:   
   case ELEMENT_EXPLOSION:
        break;               
   
   case ELEMENT_ENNEMI:
        /* détermine le type de l'ennemi pour connaître le type d'explosion à faore*/
        ennemi = liste_ennemiLire(niveauLireListeEnnemi(niveau), 
               liste_ennemiRechercher(niveauLireListeEnnemi(niveau), i, j));
        ennemiEcrireType(ennemi, ENNEMI_TYPE_ABSENT);
        environnementAjouterExplosion(niveau, type, i, j); 
        
                     
         break;     
                      
   case ELEMENT_TERRE:
   case ELEMENT_MUR:
   case ELEMENT_MUR_QUI_BOUGE:
   case ELEMENT_SLIM:
   case ELEMENT_VIDE:
        environnementAjouterExplosion(niveau, type, i, j);         
        break;
        
   case ELEMENT_PIERRE:
        recherche = liste_pierreRechercher(niveauLireListePierre(niveau), i, j);
        if (recherche == 0) {
                      fprintf(stderr, "**probleme explosion pierre\n");
                      return;
        }
        pierre = liste_pierreLire(niveauLireListePierre(niveau), recherche);
        switch(pierreLireType(pierre))
        {
          case PIERRE_TYPE_PIERRE:
          case PIERRE_TYPE_EXPLOSION:
            environnementAjouterExplosion(niveau, type, i, j);
            //environnementCreerExplosion33(niveau, type, i, j); 
              
            pierreEcrireType(pierre, PIERRE_TYPE_ABSENT);
            break;
          default:
            break;
        }
        break;
   case ELEMENT_DIAMANT:
        recherche = liste_diamantRechercher(niveauLireListeDiamant(niveau), i, j);
        if (recherche == 0) {
                      fprintf(stderr, "**probleme explosion diamant\n");
                      return;
        }
        diamant = liste_diamantLire(niveauLireListeDiamant(niveau), recherche);
        diamantEcrireType(diamant, DIAMANT_TYPE_ABSENT);
        environnementAjouterExplosion(niveau, type, i, j);
        niveauEcrireMatrice(niveau, i, j, ELEMENT_EXPLOSION);
        break;        
   
  
   case ELEMENT_JOUEUR:
        /* tue le joueur*/
        environnementMortJoueur(niveau, liste_joueurLire(niveauLireListeJoueur(niveau),
                                               liste_joueurRechercher(niveauLireListeJoueur(niveau), i, j)
                                               ) 
                                               );
        break;
        
        
   default:
        fprintf(stderr, "**Explosion 11 elements manquants\n");
        break;    
                                   
  }   
     
     
}


/* Créer une vrai explosion de taille 3 3*/
void environnementCreerExplosion33(Niveau* niveau, Explosion_type type, int i, int j)
{
    int x;
    int y;
  //  fprintf(stderr, "appel 33: i=%d, j=%d\n", i, j);
    for (x=i-1; x<=i+1; x++)
     for (y=j-1; y<=j+1; y++)
       environnementCreerExplosion11(niveau, type, x, y);
}


/* fait évoluer l'ensemble des explosions du niveau*/
void environnementEvoluerListeExplosion(Niveau* niveau)
{
  Explosion* explosion;
  Diamant* diamant;
  Pierre *pierre;
  Liste_explosion* liste = niveauLireListeExplosion(niveau);
  int i;   
  for (i=1; i<= liste_explosionTaille(liste); i++)   
  {
   // fprintf(stderr, "jjj");
    explosion = liste_explosionLire(liste, i);
    switch(explosionLireEtat(explosion))
    {
      /* si l'explosion est en progression, il progresse si cela fait moins de 2 tours
       * qu'il est en progression, sinon, il passe en régression*/
      case PROGRESSION:
           switch(explosionLireCompteur(explosion))
           {
             case 0:
                  explosionEcrireCompteur(explosion, 1);
                  break;
             case 1:
                  explosionEcrireCompteur(explosion, 2);
                  break;
             case 2:
                  explosionEcrireCompteur(explosion, 1);
                  explosionEcrireEtat(explosion, REGRESSION);
                  break;
             default:
                  break;                                                                               
                                                   
           }
           break;
      case REGRESSION:    
           switch(explosionLireCompteur(explosion))
           {
             case 2:
                  explosionEcrireCompteur(explosion, 1);
                  break;
             case 1:
                  explosionEcrireCompteur(explosion, 0);
                  break;
             /* si il a fini de regresser, on transforme l'explosion en l'élément
               correspondant au type, par exemple, pierre, diamants ou rien*/
             case 0:
                  switch(explosionLireType(explosion))
                    {
                      case EXPLOSION_TYPE_PIERRE:
                           niveauEcrireMatrice(niveau, explosionLireX(explosion), explosionLireY(explosion), ELEMENT_PIERRE);
                           pierre = pierreCreer();
                           /* pâr exemple pour une pierre, on doit créer la pierre*/
                           if (pierre == NULL) {
                                       fprintf(stderr, "** probleme ajout pierre ds explosion\n");
                                       return;
                           }
                           /* initialiser les valeurs*/
                           pierreEcrireX(pierre, explosionLireX(explosion));
                           pierreEcrireY(pierre, explosionLireY(explosion));
                           pierreEcrireType(pierre,  PIERRE_TYPE_PIERRE);
                           //pierreEcrireTransformationChute( pierre, PIERRE_TRANSFORMATION_PIERRE);
                           pierreEcrireDirection(pierre, PIERRE_DIRECTION_BOUGE_PAS);
                           /* ajouter la pierre à la liste*/
                           liste_pierreAjouterFin(niveauLireListePierre(niveau), pierre);
                           break;
                      case EXPLOSION_TYPE_DIAMANT:
                           niveauEcrireMatrice(niveau, explosionLireX(explosion), explosionLireY(explosion), ELEMENT_DIAMANT);
                           diamant = diamantCreer();
                           if (diamant == NULL) {
                                       fprintf(stderr, "** probleme ajout diamant ds explosion\n");
                                       return;
                           }
                           diamantEcrireX(diamant, explosionLireX(explosion));
                           diamantEcrireY(diamant, explosionLireY(explosion));
                           diamantEcrireType(diamant,  DIAMANT_TYPE_DIAMANT);
                           diamantEcrireTransformationChute( diamant, DIAMANT_TRANSFORMATION_DIAMANT);
                           diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BOUGE_PAS);
                           liste_diamantAjouterFin(niveauLireListeDiamant(niveau), diamant);
                           break;
                      case EXPLOSION_TYPE_RIEN:
                           niveauEcrireMatrice(niveau, explosionLireX(explosion), explosionLireY(explosion), ELEMENT_VIDE);
                           break;  
                       default:
                           break;  
                     }                                        
                  /*il faut détruire l'explosion*/
                  explosionEcrireType(explosion, EXPLOSION_TYPE_ABSENT);
                   /*important car sinon avec le i++, on sauterait un élément*/
                  
                  
                  break;
                  
             default:
                  break;                                                                               
                                                   
           }           
           break;
      default: 
               break;
      
    }
  }  
   environnementListeExplosionVidage(niveau);   
}


