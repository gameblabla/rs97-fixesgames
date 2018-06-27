#include "../niveau/niveau.h"
#include "environnement.h"

/* pour les tester si l'ennemi doit exploser à cause du slim*/
void environnementEnnemiTestExplosionSlim(Niveau* niveau, Ennemi* ennemi)
{
  int x = ennemiLireX(ennemi);
  int y = ennemiLireY(ennemi);
     
   /* on matte tout autour de l'ennemi voir si ya du slim, si oui, on explose
    * et on se transforme soit en diamant, soit en rien suivant le type*/  
  if ( (niveauLireMatrice(niveau, x, y+1) == ELEMENT_SLIM)
    || (niveauLireMatrice(niveau, x, y-1) == ELEMENT_SLIM)
    || (niveauLireMatrice(niveau, x+1, y) == ELEMENT_SLIM)
    || (niveauLireMatrice(niveau, x-1, y) == ELEMENT_SLIM))
   { 
    switch (ennemiLireType(ennemi))
    {
      case ENNEMI_TYPE_DIAMANT:
           environnementCreerExplosion33(niveau, EXPLOSION_TYPE_DIAMANT, x, y);
           break;
     case ENNEMI_TYPE_RIEN:
      environnementCreerExplosion33(niveau, EXPLOSION_TYPE_RIEN, x, y);
      break;
     case ENNEMI_TYPE_PIERRE:
      environnementCreerExplosion33(niveau, EXPLOSION_TYPE_PIERRE, x, y);
      break;
     default:
      break;
     }  
   }   
       
   return;  
}
/*test pour voir si l'ennemi tue le joueur*/
void environnementEnnemiTestExplosionJoueur(Niveau* niveau, Ennemi* ennemi)
{
  int x = ennemiLireX(ennemi);
  int y = ennemiLireY(ennemi);
  int jx;
  int jy;
  int b= 0;
  Joueur* joueur;
  Liste_joueur* liste= niveauLireListeJoueur(niveau);
  
  if (ennemiLireType(ennemi) == ENNEMI_TYPE_ABSENT)
    return;
  
     
  if ( (niveauLireMatrice(niveau, x, y+1) == ELEMENT_JOUEUR))
     { 
        jx = x; jy = y+1; b=1;
     }
     
  if ( (niveauLireMatrice(niveau, x, y-1) == ELEMENT_JOUEUR))
     { 
        jx = x; jy = y-1; b=1;
     }   
  if ( (niveauLireMatrice(niveau, x+1, y) == ELEMENT_JOUEUR))
     { 
        jx = x+1; jy = y; b=1;
     } 
  if ( (niveauLireMatrice(niveau, x-1, y) == ELEMENT_JOUEUR))
     { 
        jx = x-1; jy = y; b=1;
     } 
  if (b==1)
  {

     joueur = liste_joueurLire(liste, liste_joueurRechercher( liste, jx, jy));
     if (joueur == NULL)
     {
               fprintf(stderr, "*probleme erreur lecture joueur");    
               return;
     }
     
      if (joueurLireForme(joueur) == JOUEUR_FORME_BOULDER && niveauLireEtat(niveau)==NIVEAU_ETAT_EN_COURS)
          {
             environnementMortJoueur(niveau, joueur);
          }
       
   }   
     
   return;  
}


/* évolution des ennemis*/
void environnementEvoluerEnnemi(Niveau* niveau, Ennemi* ennemi)
{
   int x = ennemiLireX(ennemi);
   int y = ennemiLireY(ennemi);

 /*cherche le type de l'ennemi*/
  if (ennemiLireType(ennemi) == ENNEMI_TYPE_ABSENT)
    return;
   
   /* on teste si l'ennemi explose à cause du slim ou d'un joueur*/
   environnementEnnemiTestExplosionSlim(niveau, ennemi);
   environnementEnnemiTestExplosionJoueur(niveau, ennemi);
   
   if (ennemiLireType(ennemi) != ENNEMI_TYPE_ABSENT)
   {
    /* gère la direction suivant la direction précedente,
        j'ai repris l'algorithme de base du jeu d'origine*/
     switch(ennemiLireDirection(ennemi))
     {
       case ENNEMI_DIRECTION_GAUCHE:
          if (niveauLireMatrice(niveau, x, y-1) == ELEMENT_VIDE)
          {
            niveauEcrireMatrice(niveau, x, y, ELEMENT_VIDE);
            niveauEcrireMatrice(niveau, x, y-1, ELEMENT_ENNEMI);
            ennemiEcrireY(ennemi, y-1);      
            ennemiEcrireDirection(ennemi, ENNEMI_DIRECTION_HAUT);                      
          }                                     
          else {
             if (niveauLireMatrice(niveau, x-1, y) == ELEMENT_VIDE)
             {
               niveauEcrireMatrice(niveau, x, y, ELEMENT_VIDE);
               niveauEcrireMatrice(niveau, x-1, y, ELEMENT_ENNEMI);
               ennemiEcrireX(ennemi, x-1);
             }  
             else
               ennemiEcrireDirection(ennemi, ENNEMI_DIRECTION_BAS);   
               
          }                              
          
          
          break;
          
          
          
       case ENNEMI_DIRECTION_HAUT:
            
         if (niveauLireMatrice(niveau, x+1, y) == ELEMENT_VIDE)
          {
            niveauEcrireMatrice(niveau, x, y, ELEMENT_VIDE);
            niveauEcrireMatrice(niveau, x+1, y, ELEMENT_ENNEMI);
            ennemiEcrireX(ennemi, x+1);      
            ennemiEcrireDirection(ennemi, ENNEMI_DIRECTION_DROITE);                      
          }                                     
          else {
             if (niveauLireMatrice(niveau, x, y-1) == ELEMENT_VIDE)
             {
               niveauEcrireMatrice(niveau, x, y, ELEMENT_VIDE);
               niveauEcrireMatrice(niveau, x, y-1, ELEMENT_ENNEMI);
               ennemiEcrireY(ennemi, y-1);
             }  
             else
               ennemiEcrireDirection(ennemi, ENNEMI_DIRECTION_GAUCHE);   
               
          }                              
          
          
          break;

       
       case ENNEMI_DIRECTION_DROITE:
            
            
          if (niveauLireMatrice(niveau, x, y+1) == ELEMENT_VIDE)
          {
            niveauEcrireMatrice(niveau, x, y, ELEMENT_VIDE);
            niveauEcrireMatrice(niveau, x, y+1, ELEMENT_ENNEMI);
            ennemiEcrireY(ennemi, y+1);      
            ennemiEcrireDirection(ennemi, ENNEMI_DIRECTION_BAS);                      
          }                                     
          else {
             if (niveauLireMatrice(niveau, x+1, y) == ELEMENT_VIDE)
             {
               niveauEcrireMatrice(niveau, x, y, ELEMENT_VIDE);
               niveauEcrireMatrice(niveau, x+1, y, ELEMENT_ENNEMI);
               ennemiEcrireX(ennemi, x+1);
             }  
             else
               ennemiEcrireDirection(ennemi, ENNEMI_DIRECTION_HAUT);   
               
          }                              
          
         break;
       
       case ENNEMI_DIRECTION_BAS:
          if (niveauLireMatrice(niveau, x-1, y) == ELEMENT_VIDE)
          {
            niveauEcrireMatrice(niveau, x, y, ELEMENT_VIDE);
            niveauEcrireMatrice(niveau, x-1, y, ELEMENT_ENNEMI);
            ennemiEcrireX(ennemi, x-1);      
            ennemiEcrireDirection(ennemi, ENNEMI_DIRECTION_GAUCHE);                      
          }                                     
          else {
             if (niveauLireMatrice(niveau, x, y+1) == ELEMENT_VIDE)
             {
               niveauEcrireMatrice(niveau, x, y, ELEMENT_VIDE);
               niveauEcrireMatrice(niveau, x, y+1, ELEMENT_ENNEMI);
               ennemiEcrireY(ennemi, y+1);
             }  
             else
               ennemiEcrireDirection(ennemi, ENNEMI_DIRECTION_DROITE);   
               
          }                   
         break;     
         
         
                                  
      default:
              break;
    
     }          
                              
                     
}  
   
     
}


/* on supprime les ennemi de type absent de l'ensemble de la liste*/
void environnementListeEnnemiVidage( Niveau* niveau)
{
 int i;
 Liste_ennemi* liste = niveauLireListeEnnemi(niveau);
 Ennemi* ennemi;
 /*for (i=1; i<= liste_ennemiTaille(liste); i++)
  {
    ennemi = liste_ennemiLire(liste, i);
    if (ennemiLireType(ennemi) == ENNEMI_TYPE_ABSENT) 
     {
       liste_ennemiRetirer(liste, i);
       i--;
     }
  }     
  */
  i=1;
  while(i<=   liste_ennemiTaille(liste))
  {
    ennemi = liste_ennemiLire(liste, i);
    if (ennemiLireType(ennemi) == ENNEMI_TYPE_ABSENT) 
     {
  //     liste_ennemiRetirer(liste, i);
  //     i--;
     }           
    i++;            
  } 
    
}


/* on fait évoluer toute la liste*/
void environnementEvoluerListeEnnemi(Niveau* niveau)
{
 int i;
 Liste_ennemi* liste = niveauLireListeEnnemi(niveau);
 for (i=1; i<= liste_ennemiTaille(liste); i++)
 {
     environnementEvoluerEnnemi(niveau, liste_ennemiLire(liste, i));
  }    
  environnementListeEnnemiVidage(niveau);  
    
     
     
     
}

