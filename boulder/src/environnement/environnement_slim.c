#include <stdlib.h>
#include <stdio.h>
#include "time.h"

#include "../niveau/niveau.h"
#include "environnement.h"

static void environnementFaire_grandir_absolument_slim (Niveau* niveau);
static void  environnementTransformerSlim(Niveau* niveau);


 /****************************
 utiliser lors de l'affichage du slim
 permet de rendre l'animation plus lente
 ***************************************/
 /*
 
 static int permutation_slim (int i) 
 {
   switch (i) {
     case 0: return 0;
     case 1: return 0;
     case 2: return 1;
     case 3: return 1;     
     default: return i;
     }
     
}
*/
/********************************
 Appeler à chaque boucle principale
 Fait grandir le slim de manière aléatoire
 ************************************/


 void environnementEvoluerSlim (Niveau* niveau)
 {

 int i,j;
 int x=0;
 int y=0;
 int randomValue ;
 int faut_sortir = 0;
 static int slim_boucle = 0;

 if (niveauLireEtat(niveau)!=NIVEAU_ETAT_EN_COURS)
 {
   slim_boucle=0;
   return;
 } 
  
 srand(time(NULL)); /*Initialisation du compteur aléatoire*/
 slim_boucle = (slim_boucle +1 ) % slimLireVitesse(niveauLireSlim(niveau));

 if (slim_boucle ==0){
   for (j=1;j<=niveauLireLongueurY(niveau); j++) {
     for (i=1;i<=niveauLireLongueurX(niveau);i++) {
       if (faut_sortir == 0) { /*astuce pour pas que ça grandisse d'un coup d'un seul en une boucle*/
         x=0;
         y=0;  
         randomValue = (int)((float)rand() / RAND_MAX * (3)); /*aléatoire*/
         if  (niveauLireMatrice(niveau, i,j) == ELEMENT_SLIM) {
           switch(randomValue) {
             case 0: 
               x++;
               break;
             case 1:
               x--;
               break;
             case 2:
                y++;
                break;
             case 3:
                y--;
                break;
            default:
                break;
           }
           switch (niveauLireMatrice(niveau, i+x,j+y)) {

/* le slim évolue si c'est vide ou de la terre*/
             case ELEMENT_VIDE:
             case ELEMENT_TERRE:
                niveauEcrireMatrice(niveau, i+x,j+y,ELEMENT_SLIM);
                faut_sortir = 1;
                break;
             default:
                break;
           }   
         }
       }
     }
   }
 if (faut_sortir == 0)

  environnementFaire_grandir_absolument_slim (niveau);
 }
 
}


/**********************************
Si aucune valeur aléatoire n'a donnée qqch pour faire grandir le slim,
 on cherche toutes les possibilités pour le faire grandir qq part,
 dans la négative, le slim doit être transformé, soit en pierre, soit en diamant

 *************************************/
 
static void environnementFaire_grandir_absolument_slim (Niveau* niveau)
{
  int i,j,x,y;
  /*int a_grandi = 0;*/
  int randomValue ;
  short boucle_local_grandissement = 1;
  int faut_sortir = 0;
 
   for (j=1;j<=niveauLireLongueurY(niveau); j++) {
     for (i=1;i<=niveauLireLongueurX(niveau);i++) {
          if (faut_sortir == 0) { /*astuce pour pas que ça grandisse d'un coup d'un seul en une boucle*/
            x=0;
            y=0;  
            randomValue = (int)((float)rand() / RAND_MAX * (3));
         
            if  (niveauLireMatrice(niveau, i,j) == ELEMENT_SLIM) {
               boucle_local_grandissement =0;
        
               while ((boucle_local_grandissement <=4)) { //(faut_sortir == 0)
                  
                      boucle_local_grandissement++;
                 
                      x=0;
                      y=0;  
                   if (faut_sortir == 0) {
                      switch((randomValue+boucle_local_grandissement) % 4) {
                         case 0: 
                           x++;
                           break;
                         case 1:
                           x--;
                           break;
                         case 2:
                           y++;
                           break;
                         case 3:
                           y--;
                           break;
                         default:
                           break;
                     }
          
                    switch (niveauLireMatrice(niveau, i+x,j+y)) {
                      case ELEMENT_VIDE:
                      case ELEMENT_TERRE:
                         niveauEcrireMatrice(niveau, i+x,j+y, ELEMENT_SLIM);
                         faut_sortir = 1;
                         break;
                      default:
                         break;
                   }
                }
                
                
              }  
           
            
         }
         
       }
     }
   }   
   
   /* Le slim est bloqué, il faut le transformer*/
   if (faut_sortir ==0)
     environnementTransformerSlim(niveau); 
     
}


/* transforme le slim suivant son type*/
static void  environnementTransformerSlim(Niveau* niveau)
{
 Diamant* diamant;
 Pierre* pierre;
 
 int i,j;
   for (j=1;j<=niveauLireLongueurY(niveau); j++) {
     for (i=1;i<=niveauLireLongueurX(niveau);i++) {
       if  (niveauLireMatrice(niveau, i,j)== ELEMENT_SLIM) {
      if (slimLireType(niveauLireSlim(niveau)) == SLIM_TYPE_DIAMANT) {
          niveauEcrireMatrice(niveau, i,j, ELEMENT_DIAMANT);
          /* création de la structure de diamant pour chaque position avec initialisation*/
          diamant = diamantCreer();
          diamantEcrireX(diamant, i);
          diamantEcrireY(diamant, j);
          diamantEcrireType(diamant, DIAMANT_TYPE_DIAMANT);
          diamantEcrireTransformationChute(diamant, DIAMANT_TRANSFORMATION_DIAMANT);
          diamantEcrireDirection(diamant, DIAMANT_DIRECTION_BOUGE_PAS);
          liste_diamantAjouterFin(niveauLireListeDiamant(niveau), diamant);
          
          }
      else
          {
          /* idem avec pierre*/
             niveauEcrireMatrice(niveau, i,j, ELEMENT_PIERRE);
             pierre = pierreCreer();
             pierreEcrireX(pierre, i);
             pierreEcrireY(pierre, j);
             pierreEcrireType(pierre, PIERRE_TYPE_PIERRE);
          //   pierreEcrireTransformationChute(pierre, PIERRE_TRANSFORMATION_PIERRE);
             pierreEcrireDirection(pierre, PIERRE_DIRECTION_BOUGE_PAS);
             liste_pierreAjouterFin(niveauLireListePierre(niveau), pierre);
          }
              
              
          
          
           }  
         
         }
         
         } 
      
}

