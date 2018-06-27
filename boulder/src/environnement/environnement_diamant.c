#include <stdio.h>
#include <assert.h>

#include "../niveau/niveau.h"
#include "environnement.h"


/* Fonction rétirant de la liste des diamants les diamants de type absents*/
void environnementListeDiamantVidage( Niveau* niveau)
{
 int i;
 Liste_diamant* liste = niveauLireListeDiamant(niveau);
 Diamant* diamant;
 for (i=1; i<= liste_diamantTaille(liste); i++)
  {
    diamant = liste_diamantLire(liste, i);
    if (diamantLireType(diamant) == DIAMANT_TYPE_ABSENT) 
     {
   //    liste_diamantRetirer(liste, i);
   //    i--;
     }
  }     
   
 /* i=1;
  while (i<= liste_diamantTaille(liste))
  {
    diamant = liste_diamantLire(liste, i);
    if (diamantLireType(diamant) == DIAMANT_TYPE_ABSENT) 
     {
       liste_diamantRetirer(liste, i);
       i--;
     }      
    i++;    
  }  
   */  
}


/* évolution des diamants*/
void environnementEvoluerDiamant (Niveau* niveau, Diamant* diamant)
{
 int test;
 Pierre* pierre;
 Ennemi* ennemi;
 
 Mur_qui_bouge* mur = niveauLireMurQuiBouge(niveau);
 Liste_diamant* liste = niveauLireListeDiamant(niveau);
 if (diamant==NULL)
  fprintf(stderr, "\n* environnementEvoluerDiamant: NULL\n");
  
 /* on récupere la position du diamant*/
 int i = diamantLireX(diamant);
 int j = diamantLireY(diamant);
 //fprintf(stderr, "\ni=%d  , j=%d\n", i, j);
 
 
 /* si c'est un type ABSENT, on fait rien*/
 if (diamantLireType(diamant) == DIAMANT_TYPE_ABSENT)
   return;
 
 test= liste_diamantRechercher(liste, i, j);
 if (test == 0) {
                 fprintf(stderr, "**probleme rechercher dans environnementEvoluerDiamant\n");
                 return;
 }
 if (niveauLireMatrice(niveau, i, j) != ELEMENT_DIAMANT) 
 {
    fprintf(stderr, "**Probleme de matrice: environnementEvoluerDiamant\n");
    return ;
  }
  
 /* on regarde ce qu'il y a en dessous du diamant*/
 switch(niveauLireMatrice(niveau, i,j+1))
  {
    /*
     * RENCONTRE DU VIDE, on peut le faire tomber
     */
    case ELEMENT_VIDE:
         niveauEcrireMatrice(niveau,i,j, ELEMENT_VIDE);
         niveauEcrireMatrice(niveau,i,j+1, ELEMENT_DIAMANT);
         diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BAS);
         diamantEcrireY(diamant, j+1);
         break;
    
 
    /*
     * RENCONTRE D'UN ENNEMI, on le fait exploser
     */ 
 
    case ELEMENT_ENNEMI:
         niveauEcrireMatrice(niveau, i, j, ELEMENT_VIDE);
         diamantEcrireType(diamant, DIAMANT_TYPE_ABSENT);
         ennemi = liste_ennemiLire(niveauLireListeEnnemi(niveau), liste_ennemiRechercher(niveauLireListeEnnemi(niveau), i, j+1));
         switch (ennemiLireType(ennemi))
         {
          case ENNEMI_TYPE_DIAMANT:
           environnementCreerExplosion33(niveau, EXPLOSION_TYPE_DIAMANT ,i, j+1);
           break;
          case ENNEMI_TYPE_RIEN:
           environnementCreerExplosion33(niveau, EXPLOSION_TYPE_RIEN,  i, j+1);
           break;
          case ENNEMI_TYPE_PIERRE:
           environnementCreerExplosion33(niveau, EXPLOSION_TYPE_PIERRE,  i, j+1);
           break;
          default:
           break;
         }
         break;
 
    /*
     * RENCONTRE D'UN JOUEUR, on le tue
     */ 
 
    case ELEMENT_JOUEUR:
         if ( niveauLireEtat(niveau)==NIVEAU_ETAT_EN_COURS)
         {
           if (diamantLireDirection(diamant) == DIAMANT_DIRECTION_BAS) 
           {
             environnementMortJoueur(niveau, liste_joueurLire(niveauLireListeJoueur(niveau),
                                               liste_joueurRechercher(niveauLireListeJoueur(niveau), i, j+1)
                                               )
                                               );
               
                                         
                                         
           }
         }
         break;
  
  
    /*
     * RENCONTRE D'UN MUR, si le diamant tombe au bord d'un mur, il peut glisser mais perd sa vitesse, sinon, il s'arrête
     */
    
    case ELEMENT_MUR:
         if (niveauLireMatrice(niveau, i+1,j+1)== ELEMENT_VIDE && niveauLireMatrice(niveau, i+1,j) == ELEMENT_VIDE) 
         {
           niveauEcrireMatrice(niveau,i,j, ELEMENT_VIDE);
           niveauEcrireMatrice(niveau,i+1, j, ELEMENT_DIAMANT);
           diamantEcrireX(diamant, i+1);
           diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BOUGE_PAS);                            
         } 
         else {
              if (niveauLireMatrice(niveau, i-1,j+1)== ELEMENT_VIDE && niveauLireMatrice(niveau, i-1,j) == ELEMENT_VIDE) 
              {
                 niveauEcrireMatrice(niveau,i,j, ELEMENT_VIDE);
                 niveauEcrireMatrice(niveau,i-1, j, ELEMENT_DIAMANT);
                 diamantEcrireX(diamant, i-1);
                 diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BOUGE_PAS);                            
              }  else
                  diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BOUGE_PAS);    
         }     
         break;
   
   
    /*
     * RENCONTRE D'UN MUR QUI BOUGE, si le mur est activé, le diamant se transforme en pierre
     */ 
         
         
    case ELEMENT_MUR_QUI_BOUGE:
         if (mur_qui_bougeLireEtat(mur) == MUR_QUI_BOUGE_ETAT_PAS_COMMENCER && diamantLireDirection(diamant) ==DIAMANT_DIRECTION_BAS)
           mur_qui_bougeEcrireEtat(mur,MUR_QUI_BOUGE_ETAT_EN_COURS);
         if (mur_qui_bougeLireEtat(mur) == MUR_QUI_BOUGE_ETAT_EN_COURS) {
           niveauEcrireMatrice(niveau, i,j, ELEMENT_VIDE);
           if (niveauLireMatrice(niveau, i, j+2) == ELEMENT_VIDE) {
               niveauEcrireMatrice(niveau, i, j+2, ELEMENT_PIERRE);
               diamantEcrireType(diamant, DIAMANT_TYPE_ABSENT);
               pierre = pierreCreer();
               pierreEcrireX(pierre, i);
               pierreEcrireY(pierre, j+2);
               pierreEcrireType(pierre, PIERRE_TYPE_PIERRE);
               //pierreEcrireTransformationChute(diamant, PIERRE_TRANSFORMATION_PIERRE);
               pierreEcrireDirection(pierre, PIERRE_DIRECTION_BOUGE_PAS);
               liste_pierreAjouterFin(niveauLireListePierre(niveau), pierre);
           }    
          diamantEcrireType(diamant, DIAMANT_TYPE_ABSENT);
         }
         if (mur_qui_bougeLireEtat(mur) == MUR_QUI_BOUGE_ETAT_FINI) {
                    /*se comporte comme un mur normal*/
                    if (niveauLireMatrice(niveau, i+1,j+1)== ELEMENT_VIDE && niveauLireMatrice(niveau, i+1,j) == ELEMENT_VIDE) 
                      {
                          niveauEcrireMatrice(niveau,i,j, ELEMENT_VIDE);
                          niveauEcrireMatrice(niveau,i+1, j, ELEMENT_DIAMANT);
                          diamantEcrireX(diamant, i+1);
                          diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BOUGE_PAS);                            
                      } 
                      else {
                           if (niveauLireMatrice(niveau, i-1,j+1)== ELEMENT_VIDE && niveauLireMatrice(niveau, i-1,j) == ELEMENT_VIDE) 
                           {
                              niveauEcrireMatrice(niveau,i,j, ELEMENT_VIDE);
                              niveauEcrireMatrice(niveau,i-1, j, ELEMENT_DIAMANT);
                              diamantEcrireX(diamant, i-1);
                              diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BOUGE_PAS);                            
                           }  else
                           diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BOUGE_PAS);    
                      }     
         }
         if (mur_qui_bougeLireEtat(mur) == MUR_QUI_BOUGE_ETAT_ABSENT)
           fprintf(stderr, "** Probleme diamant a rencontre un mur qui bouge\n");
         break;
    
  
   /*
    * EXPLOSION, ça fait comme un mur, il s'arrête
    */
  
    case ELEMENT_EXPLOSION:
       /*  niveauEcrireMatrice(niveau, i, j, ELEMENT_VIDE);
         diamantEcrireType(diamant, DIAMANT_TYPE_ABSENT);
       */  
         break;     
    
    /* Le diamant peut glisser sur le côté si c'est vide*/
    case ELEMENT_DIAMANT: case ELEMENT_PIERRE:
         if (niveauLireMatrice(niveau, i+1,j+1) == ELEMENT_VIDE && (niveauLireMatrice(niveau, i+1,j) == ELEMENT_VIDE) )
         {
           niveauEcrireMatrice(niveau, i,j, ELEMENT_VIDE);
           niveauEcrireMatrice(niveau, i+1,j, ELEMENT_DIAMANT);
           diamantEcrireX(diamant, i+1);
           diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BAS);
                           
         }
         else {
           if (niveauLireMatrice(niveau, i-1,j+1) == ELEMENT_VIDE &&(niveauLireMatrice(niveau, i-1,j) == ELEMENT_VIDE))
           {
               niveauEcrireMatrice(niveau, i,j, ELEMENT_VIDE);
              niveauEcrireMatrice(niveau, i-1,j, ELEMENT_DIAMANT);
              diamantEcrireX(diamant, i-1);
              diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BAS);
                                         
           }    
           else
             diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BOUGE_PAS); 
         
         }
         break;
    /*   
     * RENCONTRE DE SLIM, DIAMANT, DIAMANT, BORD ou TERRE
     */   
    case ELEMENT_SLIM: 
    case ELEMENT_BORD: case ELEMENT_TERRE:  case ELEMENT_PORTE_DE_FIN:
         /*au cas ou le joueur pousse, il faut garder les directions gauche et droite*/
         if (diamantLireDirection(diamant) == DIAMANT_DIRECTION_BAS)
           diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BOUGE_PAS);                          
         break;                       
    
    /*
     * SINON ELEMENT NON EXISTANT, ERREUR
     */
    
    default:
       fprintf(stderr, "**element non existant dans environnementEvoluerDiamant\n");                            
       break;
  } 
     
  return;   
}

/* fait évoluer l'ensemble des diamants*/

void environnementEvoluerListeDiamant (Niveau* niveau)
{
 int i;
 Liste_diamant* liste = niveauLireListeDiamant(niveau);
 environnementListeDiamantVidage(niveau);
 for (i=1; i<= liste_diamantTaille(liste); i++)
 {
     environnementEvoluerDiamant(niveau, liste_diamantLire(liste, i));
  }    
  environnementListeDiamantVidage(niveau);

}

