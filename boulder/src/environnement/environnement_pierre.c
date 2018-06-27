#include <stdio.h>

#include "../niveau/niveau.h"
#include "environnement.h"

/* environnement des pierres, même principe que pour les diamants*/

void environnementListePierreVidage( Niveau* niveau)
{
 int i;
 Liste_pierre* liste = niveauLireListePierre(niveau);
 Pierre* pierre;
 for (i=1; i<= liste_pierreTaille(liste); i++)
  {
    pierre = liste_pierreLire(liste, i);
    if (pierreLireType(pierre) == PIERRE_TYPE_ABSENT) 
     {
     //  liste_pierreRetirer(liste, i);
     //  i--;
     }
  }     
     
     
}



void environnementEvoluerPierre (Niveau* niveau, Pierre* pierre)
{
 int test;
 Diamant* diamant;
 Ennemi* ennemi;
 
 Mur_qui_bouge* mur = niveauLireMurQuiBouge(niveau);
 Liste_pierre* liste = niveauLireListePierre(niveau);
 
 int i = pierreLireX(pierre);
 int j = pierreLireY(pierre);
 
 if (pierreLireType(pierre) == PIERRE_TYPE_ABSENT)
   return;
 
 test= liste_pierreRechercher(liste, i, j);
 if (test == 0) {
                 fprintf(stderr, "**probleme rechercher dans environnementEvoluerPierre\n");
                 return;
 }
 if (niveauLireMatrice(niveau, i, j) != ELEMENT_PIERRE) 
 {
    fprintf(stderr, "**Probleme de matrice: environnementEvoluerPierre\n");
    return ;
  }
 switch(niveauLireMatrice(niveau, i,j+1))
  {
    /*
     * RENCONTRE DU VIDE
     */
    case ELEMENT_VIDE:
         niveauEcrireMatrice(niveau,i,j, ELEMENT_VIDE);
         niveauEcrireMatrice(niveau,i,j+1, ELEMENT_PIERRE);
         pierreEcrireDirection(pierre, PIERRE_DIRECTION_BAS);
         pierreEcrireY(pierre, j+1);
         break;
    
 
    /*
     * RENCONTRE D'UN ENNEMI
     */ 
 
    case ELEMENT_ENNEMI:
        // niveauEcrireMatrice(niveau, i, j, ELEMENT_VIDE);
        // pierreEcrireType(pierre, PIERRE_TYPE_ABSENT);
         ennemi = liste_ennemiLire(
                     niveauLireListeEnnemi(niveau),  
                     liste_ennemiRechercher(niveauLireListeEnnemi(niveau), i, j+1));
        
         switch (ennemiLireType(ennemi))
         {
           case  ENNEMI_TYPE_DIAMANT:
                 environnementCreerExplosion33(niveau,  EXPLOSION_TYPE_DIAMANT,i, j+1);
                 break;
           case ENNEMI_TYPE_RIEN:
                environnementCreerExplosion33(niveau, EXPLOSION_TYPE_RIEN, i, j+1);
                break;
           case ENNEMI_TYPE_PIERRE:
                environnementCreerExplosion33(niveau, EXPLOSION_TYPE_PIERRE, i, j+1);
                break;
           default:
                break;
         }  
         break;

    /*
     * RENCONTRE D'UN JOUEUR
     */ 
 
    case ELEMENT_JOUEUR:
         if (niveauLireEtat(niveau)==NIVEAU_ETAT_EN_COURS)
         {
           if (pierreLireDirection(pierre) == PIERRE_DIRECTION_BAS) 
           {
             environnementMortJoueur(niveau, liste_joueurLire(niveauLireListeJoueur(niveau),
                                               liste_joueurRechercher(niveauLireListeJoueur(niveau), i, j+1)
                                               )
                                               );
               
                                         
                                         
           }
         }
         break;
  
  
    /*
     * RENCONTRE D'UN MUR
     */
    
    case ELEMENT_MUR:
        switch(pierreLireType(pierre))
          {
           case PIERRE_TYPE_PIERRE:
             if (niveauLireMatrice(niveau, i+1,j+1)== ELEMENT_VIDE && niveauLireMatrice(niveau, i+1,j) == ELEMENT_VIDE) 
             {
                niveauEcrireMatrice(niveau,i,j, ELEMENT_VIDE);
                niveauEcrireMatrice(niveau,i+1, j, ELEMENT_PIERRE);
                pierreEcrireX(pierre, i+1);
                pierreEcrireDirection(pierre, PIERRE_DIRECTION_BOUGE_PAS);                            
             } 
             else {
             if (niveauLireMatrice(niveau, i-1,j+1)== ELEMENT_VIDE && niveauLireMatrice(niveau, i-1,j) == ELEMENT_VIDE) 
              {
                 niveauEcrireMatrice(niveau,i,j, ELEMENT_VIDE);
                 niveauEcrireMatrice(niveau,i-1, j, ELEMENT_PIERRE);
                 pierreEcrireX(pierre, i-1);
                 pierreEcrireDirection(pierre, PIERRE_DIRECTION_BOUGE_PAS);                            
              }  
              else
                  pierreEcrireDirection(pierre, PIERRE_DIRECTION_BOUGE_PAS);    
              }
              break;
           case PIERRE_TYPE_EXPLOSION:
            if (pierreLireDirection(pierre) == PIERRE_DIRECTION_BAS)
              environnementCreerExplosion33(niveau, EXPLOSION_TYPE_RIEN, i, j);
              break;
           
           default:
             break;
          }              
         break;
   
   
    /*
     * RENCONTRE D'UN MUR QUI BOUGE
     */ 
         
         
    case ELEMENT_MUR_QUI_BOUGE:
         if (mur_qui_bougeLireEtat(mur) == MUR_QUI_BOUGE_ETAT_PAS_COMMENCER && pierreLireDirection(pierre) ==PIERRE_DIRECTION_BAS)
           mur_qui_bougeEcrireEtat(mur,MUR_QUI_BOUGE_ETAT_EN_COURS);
         if (mur_qui_bougeLireEtat(mur) == MUR_QUI_BOUGE_ETAT_EN_COURS) {
           niveauEcrireMatrice(niveau, i,j, ELEMENT_VIDE);
           if (niveauLireMatrice(niveau, i, j+2) == ELEMENT_VIDE) {
               niveauEcrireMatrice(niveau, i, j+2, ELEMENT_DIAMANT);
               pierreEcrireType(pierre, PIERRE_TYPE_ABSENT);
               diamant = diamantCreer();
               diamantEcrireX(diamant, i);
               diamantEcrireY(diamant, j+2);
               diamantEcrireType(diamant, DIAMANT_TYPE_DIAMANT);
               diamantEcrireTransformationChute(diamant, DIAMANT_TRANSFORMATION_DIAMANT);
               diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BOUGE_PAS);
               liste_diamantAjouterFin(niveauLireListeDiamant(niveau), diamant);
           }    
          pierreEcrireType(pierre, PIERRE_TYPE_ABSENT);
         }
         if (mur_qui_bougeLireEtat(mur) == MUR_QUI_BOUGE_ETAT_FINI) {
                    /*se comporte comme un mur normal*/
                    if (niveauLireMatrice(niveau, i+1,j+1)== ELEMENT_VIDE && niveauLireMatrice(niveau, i+1,j) == ELEMENT_VIDE) 
                      {
                          niveauEcrireMatrice(niveau,i,j, ELEMENT_VIDE);
                          niveauEcrireMatrice(niveau,i+1, j, ELEMENT_PIERRE);
                          pierreEcrireX(pierre, i+1);
                          pierreEcrireDirection(pierre, PIERRE_DIRECTION_BOUGE_PAS);                            
                      } 
                      else {
                           if (niveauLireMatrice(niveau, i-1,j+1)== ELEMENT_VIDE && niveauLireMatrice(niveau, i-1,j) == ELEMENT_VIDE) 
                           {
                              niveauEcrireMatrice(niveau,i,j, ELEMENT_VIDE);
                              niveauEcrireMatrice(niveau,i-1, j, ELEMENT_PIERRE);
                              pierreEcrireX(pierre, i-1);
                              pierreEcrireDirection(pierre, PIERRE_DIRECTION_BOUGE_PAS);                            
                           }  else
                           pierreEcrireDirection(pierre, PIERRE_DIRECTION_BOUGE_PAS);    
                      }     
         }
         if (mur_qui_bougeLireEtat(mur) == MUR_QUI_BOUGE_ETAT_ABSENT)
           fprintf(stderr, "** Probleme pierre a rencontre un mur qui bouge\n");
         break;
    
  
   /*
    * EXPLOSION
    */
  
    case ELEMENT_EXPLOSION:
       /*  niveauEcrireMatrice(niveau, i, j, ELEMENT_VIDE);
         pierreEcrireType(pierre, PIERRE_TYPE_ABSENT);
       */  
         break;     
    case ELEMENT_PIERRE: case ELEMENT_DIAMANT:
        switch(pierreLireType(pierre))
          {
           case PIERRE_TYPE_PIERRE:
             if (niveauLireMatrice(niveau, i+1,j+1) == ELEMENT_VIDE && (niveauLireMatrice(niveau, i+1,j) == ELEMENT_VIDE) )
             {
              niveauEcrireMatrice(niveau, i,j, ELEMENT_VIDE);
              niveauEcrireMatrice(niveau, i+1,j, ELEMENT_PIERRE);
              pierreEcrireX(pierre, i+1);
              pierreEcrireDirection(pierre, PIERRE_DIRECTION_BAS);
                           
             }
             else {
             if (niveauLireMatrice(niveau, i-1,j+1) == ELEMENT_VIDE &&(niveauLireMatrice(niveau, i-1,j) == ELEMENT_VIDE))
             {
               niveauEcrireMatrice(niveau, i,j, ELEMENT_VIDE);
               niveauEcrireMatrice(niveau, i-1,j, ELEMENT_PIERRE);
               pierreEcrireX(pierre, i-1);
               pierreEcrireDirection(pierre, PIERRE_DIRECTION_BAS);
             }     
             else
                 pierreEcrireDirection(pierre, PIERRE_DIRECTION_BOUGE_PAS); 
             }
             break;
          case PIERRE_TYPE_EXPLOSION:
            if (pierreLireDirection(pierre) == PIERRE_DIRECTION_BAS)
              environnementCreerExplosion33(niveau, EXPLOSION_TYPE_RIEN, i, j);
            break;
          default:
             break;
          }               
         break;
    /*   
     * RENCONTRE DE SLIM, PIERRE, DIAMANT, BORD ou TERRE
     */   
    case ELEMENT_SLIM: 
    case ELEMENT_BORD: case ELEMENT_TERRE:  case ELEMENT_PORTE_DE_FIN:
         /*au cas ou le joueur pousse, il faut garder les directions gauche et droite*/
         switch(pierreLireType(pierre))
          {
           case PIERRE_TYPE_PIERRE:
             if (pierreLireDirection(pierre) == PIERRE_DIRECTION_BAS)
                pierreEcrireDirection(pierre, PIERRE_DIRECTION_BOUGE_PAS);                          
             break;
           case PIERRE_TYPE_EXPLOSION:
            if (pierreLireDirection(pierre) == PIERRE_DIRECTION_BAS)
              environnementCreerExplosion33(niveau, EXPLOSION_TYPE_RIEN, i, j);
            break;
           default:
            break;
          }
         break;                       
    
    /*
     * SINON ELEMENT NON EXISTANT, ERREUR
     */
    
    default:
       fprintf(stderr, "**element non existant dans environnementEvoluerPierre\n");                            
       break;
  } 
     
  return;   
}

void environnementEvoluerListePierre (Niveau* niveau)
{
 int i;
 Liste_pierre* liste = niveauLireListePierre(niveau);
 for (i=1; i<= liste_pierreTaille(liste); i++)
 {
     environnementEvoluerPierre(niveau, liste_pierreLire(liste, i));
  }    
  environnementListePierreVidage(niveau);

}

