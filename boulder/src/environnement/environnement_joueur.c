#include "../niveau/niveau.h"
#include "environnement.h"

static void environnementTesterMortJoueur(Niveau * niveau);

/* dans le cas d'un appel de mort de joueur, modifie les variables du joueur*/
void environnementMortJoueur(Niveau* niveau,Joueur * joueur)
{
  niveauEcrireMatrice(niveau, joueurLireX(joueur), joueurLireY(joueur), ELEMENT_VIDE);
  joueurEcrireEtat(joueur, JOUEUR_ETAT_MORT);
  joueurEcrireVie(joueur, joueurLireVie(joueur)-1);
  
  /* ce teste sert, car si le joueur gagne, il y a encore un peu d'animation,
  et il arrive qu'un ennemi touche le joueur, ce test sert à ce que le joueur
  ne meurt pas alors qu'il avait fini*/
  if (niveauLireEtat(niveau) == NIVEAU_ETAT_EN_COURS)
  {
       environnementCreerExplosion33(niveau, EXPLOSION_TYPE_RIEN, joueurLireX(joueur), joueurLireY(joueur));
  }   
  return;
}


/* si tous les joueurs sont morts*/
void environnementGameOverJoueur(Niveau* niveau)
{
 Joueur* joueur;
 int i;
 int b =0;
 for (i=1; i<=liste_joueurTaille(niveauLireListeJoueur(niveau)); i++)
 {
  joueur = liste_joueurLire( niveauLireListeJoueur(niveau), i);
  if (joueurLireVie(joueur) != 0)
    b=1;
 }  
 if (b==0)
  niveauEcrireEtat(niveau, NIVEAU_ETAT_GAME_OVER_PERD);
  
}


/* fait évoluer le joueur*/
void environnementEvoluerJoueur(Niveau* niveau, Joueur* joueur)
{  
  int i=0;
  int j=0;   
  int x =joueurLireX(joueur);
  int y = joueurLireY(joueur);
  int touche_presser = 0;
  Pierre* pierre;
  Diamant* diamant;
  
  /* si il est mort, on fait rien*/
  if (joueurLireEtat(joueur) == JOUEUR_ETAT_MORT)
    return;
 
  /*suivant les appuies de touche, on le fait bouger*/
  
  /* Si il appuie sur ta touche d'autosuicide, il explose*/
  if (joueurToucheAppuie(joueur, TOUCHE_AUTOSUICIDE)) {
     environnementMortJoueur(niveau, joueur);
     return;
  }
  
  /*change la direction du joueur suivant la touche qu'il appuie, à ce stade,
   le joueur ne bouge pas dans le niveau, mais sa direction change (par exemple,
     utile pour les animations si le joueur est bloqué contre un mur*/    
  if  (joueurToucheAppuie(joueur, TOUCHE_HAUT)) {
      joueurEcrireDirection(joueur, JOUEUR_DIRECTION_HAUT);
      j--;
      touche_presser = 1; /* on indique qu'une touche est pressée, sert à combiner
        destouches et à ne garder que la première touche appuyée*/
  } 
  else 
  {
          if  (joueurToucheAppuie(joueur, TOUCHE_BAS)) {
              joueurEcrireDirection(joueur, JOUEUR_DIRECTION_BAS);
              j++;
              touche_presser = 1;         
          } 
          else 
          {
                 if (joueurToucheAppuie(joueur, TOUCHE_GAUCHE)) {
                    joueurEcrireDirection(joueur, JOUEUR_DIRECTION_GAUCHE);
                    i--; 
                    touche_presser = 1;
                 } 
                 else 
                 {
                        if (joueurToucheAppuie(joueur, TOUCHE_DROITE)) {
                           joueurEcrireDirection(joueur, JOUEUR_DIRECTION_DROITE);
                           i++;
                           touche_presser = 1;
                        }
                 }
          }
  
  }
     

     
  if (touche_presser)
  {
    /* test pour les combinaisons de touche, style pour creuser sur le côté sans bouger*/
      if (joueurToucheAppuie(joueur, TOUCHE_COMBINER))
      {
         switch(niveauLireMatrice(niveau, x+i,y+j))
         {
          /* le joueur ne bouge pas mais peut prendre un diamant à côté*/
            case ELEMENT_DIAMANT:
                 diamant = liste_diamantLire(niveauLireListeDiamant(niveau), 
                                liste_diamantRechercher(niveauLireListeDiamant(niveau), x+i, y+j)
                                );
                 if (diamant == NULL) {
                   fprintf(stderr, "probleme avec le positionnement des diamants ds liste\n");
                   break;
                 }
                 if (diamantLireType(diamant) == DIAMANT_TYPE_DIAMANT && joueurLireForme(joueur) == JOUEUR_FORME_BOULDER)
                 {   
                     joueurEcrireDiamant(joueur, joueurLireDiamant(joueur)+1);
                     if (niveauLireNombreDiamantRestant(niveau) >0)
                        niveauDecrementerDiamantRestant(niveau);
                     niveauEcrireMatrice(niveau, x+i, y+j, ELEMENT_VIDE);
                     diamantEcrireType(diamant, DIAMANT_TYPE_ABSENT);
                 }
                 break;
                 
            /* idem, il peut creuser*/
            case ELEMENT_TERRE:
                 niveauEcrireMatrice(niveau, x+i, y+j, ELEMENT_VIDE);
                 break;
            default:
                 break;
         }
      }   
      else
      {
         /* là, le joueur bouge vraiment*/
         switch(niveauLireMatrice(niveau, x+i,y+j))
         {           
         
            /* si il bouge vers un diamant*/
            case ELEMENT_DIAMANT:
               /* on récupere les données du diamants*/
                 diamant = liste_diamantLire(niveauLireListeDiamant(niveau), 
                                liste_diamantRechercher(niveauLireListeDiamant(niveau), x+i, y+j)
                                );
                 if (diamant == NULL) {
                   fprintf(stderr, "**probleme avec le positionnement des diamants ds liste\n");
                   break;
                 }
                // fprintf(stderr, "boulder a bouffe: %d %d\n", diamantLireX(diamant), diamantLireY(diamant));
               /* si le joueur est en forme normal, il peut prendre le diamant*/
                 if (joueurLireForme(joueur) == JOUEUR_FORME_BOULDER)
                 {  joueurEcrireDiamant(joueur, joueurLireDiamant(joueur)+1);
                    if (niveauLireNombreDiamantRestant(niveau) >0)
                      niveauDecrementerDiamantRestant(niveau);
                    if (diamantLireType(diamant) == DIAMANT_TYPE_TRANSFORMATION)
                    {
                                                 
                      joueurEcrireForme(joueur, JOUEUR_FORME_ENNEMI);
                      
                      /* on met le diamant en type absent pour qu'il n'y ait plus
                       d'interactions pendant ce tour de jeu*/
                      diamantEcrireType(diamant, DIAMANT_TYPE_ABSENT);   /*pas de break*/         
                      /* on vide la case ou était le joueur*/
                      niveauEcrireMatrice(niveau,  x, y, ELEMENT_VIDE);
                      /* on met le joueur à la nouvelle place*/
                      niveauEcrireMatrice(niveau, x+i, y+j, ELEMENT_JOUEUR);
                      /* on change les données du joueur*/
                      joueurEcrireX(joueur, x+i);
                      joueurEcrireY(joueur, y+j);
                      /* compteur pour savoir si le joueur pousse une pierre ou un diamant*/
                      joueurEcrireCompteurPousser(joueur, 0);
                    } 
                    else
                    {
                     
                      diamantEcrireType(diamant, DIAMANT_TYPE_ABSENT);   /*pas de break*/         
                      niveauEcrireMatrice(niveau,  x, y, ELEMENT_VIDE);
                      niveauEcrireMatrice(niveau, x+i, y+j, ELEMENT_JOUEUR);
                      joueurEcrireX(joueur, x+i);
                      joueurEcrireY(joueur, y+j);
                      joueurEcrireCompteurPousser(joueur, 0);    
                            
                    }
                    
                 }
                 else {
                      /* même principe*/
                      if (joueurLireForme(joueur) == JOUEUR_FORME_ENNEMI)
                      {  
                         joueurEcrireDiamant(joueur, joueurLireDiamant(joueur)+1);
                         if (diamantLireType(diamant) == DIAMANT_TYPE_TRANSFORMATION)
                         {
                            if (niveauLireNombreDiamantRestant(niveau) >0)
                                niveauDecrementerDiamantRestant(niveau);

                            joueurEcrireForme(joueur, JOUEUR_FORME_BOULDER);
                      
                            diamantEcrireType(diamant, DIAMANT_TYPE_ABSENT);   /*pas de break*/         
                            niveauEcrireMatrice(niveau,  x, y, ELEMENT_VIDE);
                            niveauEcrireMatrice(niveau, x+i, y+j, ELEMENT_JOUEUR);
                            joueurEcrireX(joueur, x+i);
                            joueurEcrireY(joueur, y+j);
                            joueurEcrireCompteurPousser(joueur, 0);
                         }
                    }
                 }
                 break;            
            
            /* même principe*/
            case ELEMENT_TERRE: case ELEMENT_VIDE :
                 niveauEcrireMatrice(niveau,  x, y, ELEMENT_VIDE);
                 niveauEcrireMatrice(niveau, x+i, y+j, ELEMENT_JOUEUR);
                 joueurEcrireX(joueur, x+i);
                 joueurEcrireY(joueur, y+j);
                 joueurEcrireCompteurPousser(joueur, 0);
                 
     
                 break;
            
            
            case ELEMENT_PORTE_DE_FIN:
                 if (niveauLirePorte(niveau) == NIVEAU_PORTE_OUVERTE)
                 {
                   niveauEcrireEtat(niveau, NIVEAU_ETAT_GAGNE);
                   niveauEcrireMatrice(niveau, x, y, ELEMENT_VIDE);
                   niveauEcrireMatrice(niveau, x+i, y+j, ELEMENT_JOUEUR);
                   joueurEcrireX(joueur, x+i);
                   joueurEcrireY(joueur, y+j);                            
                                             
                 } else
                 {
                   joueurEcrireCompteurPousser(joueur, 0);
                 }
                 break;
           
           
            case ELEMENT_ENNEMI:
                  if (joueurLireForme(joueur) == JOUEUR_FORME_BOULDER)
                      environnementMortJoueur(niveau, joueur);
           
                 joueurEcrireCompteurPousser(joueur, 0);
                 break;
            
            case ELEMENT_PIERRE:
                 joueurEcrireCompteurPousser(joueur, joueurLireCompteurPousser(joueur)+1);
                 
                 if ((joueurLireCompteurPousser(joueur) >=2) &&(joueurToucheAppuie(joueur, TOUCHE_GAUCHE) || joueurToucheAppuie(joueur, TOUCHE_DROITE)))
                 {
                     if (niveauLireMatrice(niveau, x+2*i, y) == ELEMENT_VIDE)
                       {pierre= liste_pierreLire(niveauLireListePierre(niveau), 
                                liste_pierreRechercher(niveauLireListePierre(niveau), x+i, y+j)
                                );    
                        if (pierre == NULL) {
                          fprintf(stderr, "**probleme avec le positionnement des pierres ds liste\n");
                          break;
                        }
                        niveauEcrireMatrice(niveau, x, y, ELEMENT_VIDE);
                        niveauEcrireMatrice(niveau, x+i, y+j, ELEMENT_JOUEUR);
                        niveauEcrireMatrice(niveau, x+2*i, y, ELEMENT_PIERRE);
                        pierreEcrireX(pierre, x+2*i);
                        pierreEcrireDirection(pierre, PIERRE_DIRECTION_BAS);
                        joueurEcrireX(joueur, x+i);
                       }
                 }                         
                 break;
            case ELEMENT_EXPLOSION:
            //     environnementMortJoueur(niveau, joueur);
                 break;
            case ELEMENT_SLIM: case ELEMENT_BORD: case ELEMENT_MUR:
            case ELEMENT_MUR_QUI_BOUGE:
                 break;
            default:
                fprintf(stderr, "**Element non existant ds le niveau pr deplacementJoueur\n");
                 break;   
                                                
           }
            
       
                 
            
       } 
  return;
  }
     
     
  joueurEcrireDirection(joueur, JOUEUR_DIRECTION_BOUGE_PAS);   
     
  
  return ;    
}




/* fait évoluer l'ensemble des joueurs du jeu*/
void environnementEvoluerListeJoueur(Niveau* niveau)
{
     int i;

     for (i=1; i<= liste_joueurTaille(niveauLireListeJoueur(niveau)); i++)
     {
        environnementEvoluerJoueur(niveau, liste_joueurLire(niveauLireListeJoueur(niveau), i));
     }
     
     
/* si si tous les joueurs sont morts*/
     environnementTesterMortJoueur(niveau);    
/* réalise les modifications si les joueurs sont tous morts*/
     environnementGameOverJoueur(niveau);
     
     return;
}


/* on teste si tous les joueurs sont morts*/
static void environnementTesterMortJoueur(Niveau * niveau)
{
  int i;
  int b = 0;
  
  Liste_joueur* liste = niveauLireListeJoueur(niveau);
  Joueur * joueur;
  
  for (i=1; i<= liste_joueurTaille(liste); i++)
     {
        joueur = liste_joueurLire(liste, i);
        if (joueurLireEtat(joueur) == JOUEUR_ETAT_VIVANT) {
          b = 1;     
          break;
        }
     }  
  if (b!=1)
    niveauEcrireEtat(niveau, NIVEAU_ETAT_MORT_DES_JOUEURS);
}

