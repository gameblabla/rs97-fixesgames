#include <stdio.h>
#include <stdlib.h>
#include <string.h>




#include "niveau.h"

/*prototypes interne*/
static int niveau_matriceExiste(Niveau* niveau);
static Niveau_element* niveau_matriceCreer (int taille);
static void niveau_matriceDetruire(Niveau* niveau);
static void niveauDetruireListe(Niveau* niveau);




/* FONCTION SUR LA MATRICE*/
int niveau_matriceExiste(Niveau* niveau)
{
    return (int) niveau->matrice;
}

Niveau_element* niveau_matriceCreer (int taille)
{
  return (Niveau_element*) malloc(taille);
}

static void niveau_matriceDetruire(Niveau* niveau)
{
  free(niveau->matrice);     
  niveau->matrice = NULL;          
}


Niveau_element niveauLireMatrice(Niveau* niveau, int i, int j)
{
 if (i<1 || i> niveauLireLongueurX(niveau) || j<1 || j > niveauLireLongueurY(niveau)) {
   fprintf(stderr, "erreur lecture hors borne de la matrice a x=%d y=%d\n", i, j);
   return ELEMENT_VIDE;
 }
 return niveau->matrice[(j-1)*niveau->longueurx+(i-1)];              
               
}

void niveauEcrireMatrice(Niveau* niveau, int i, int j, Niveau_element element)
{
 if (i<1 || i> niveauLireLongueurX(niveau) || j<1 || j > niveauLireLongueurY(niveau)) {
   fprintf(stderr, "erreur ecriture hors borne de la matrice a x=%d y=%d\n", i, j);
   return;
 } 
 niveau->matrice[(j-1)*niveau->longueurx +(i-1)] = element;    
}







/* initialisation du niveau*/
void niveauInitialiser(Niveau* niveau)
{
  niveau->private_temps = tempsRecuperer ();
  niveau->matrice = NULL;
  niveauEcrireEtat(niveau, NIVEAU_ETAT_INTRODUCTION);
   mur_qui_bougeEcrireEtat( niveauLireMurQuiBouge(niveau), MUR_QUI_BOUGE_ETAT_PAS_COMMENCER);
  liste_joueurInitialiser(niveauLireListeJoueur(niveau));
  liste_diamantInitialiser(niveauLireListeDiamant(niveau));
  liste_ennemiInitialiser(niveauLireListeEnnemi(niveau));
  liste_explosionInitialiser(niveauLireListeExplosion(niveau));
  liste_pierreInitialiser(niveauLireListePierre(niveau));
  mur_qui_bougeInitialiser(niveauLireMurQuiBouge(niveau));
  niveauEcrirePorte(niveau, NIVEAU_PORTE_FERMEE);
}

/* on réinitialise le niveau car au passage suivant un niveau,
 on ne le détruit pas pour garder les statistiques*/
void niveauReinitialiser(Niveau* niveau)
{
  niveau->private_temps = tempsRecuperer ();
  niveau->matrice = NULL;
  niveauEcrireEtat(niveau, NIVEAU_ETAT_INTRODUCTION);
   mur_qui_bougeEcrireEtat( niveauLireMurQuiBouge(niveau), MUR_QUI_BOUGE_ETAT_PAS_COMMENCER);
  liste_diamantInitialiser(niveauLireListeDiamant(niveau));
  liste_ennemiInitialiser(niveauLireListeEnnemi(niveau));
  liste_explosionInitialiser(niveauLireListeExplosion(niveau));
  liste_pierreInitialiser(niveauLireListePierre(niveau));
  mur_qui_bougeInitialiser(niveauLireMurQuiBouge(niveau));
  niveauEcrirePorte(niveau, NIVEAU_PORTE_FERMEE);
}

/* on détruit l'ensemble des listes*/
void niveauDetruireListe(Niveau* niveau)
{
 liste_pierreDetruire(niveauLireListePierre(niveau));
 liste_diamantDetruire(niveauLireListeDiamant(niveau));
 liste_explosionDetruire(niveauLireListeExplosion(niveau));
 liste_ennemiDetruire(niveauLireListeEnnemi(niveau));
}

/* on détruit tout le niveau*/
void niveauDetruire(Niveau* niveau)
{
  niveauDetruireListe(niveau);  
 niveau_matriceDetruire(niveau);
 liste_joueurDetruire(niveauLireListeJoueur(niveau));   
}

/* on détruit tous sauf les stats des joueurs, utilisé lors du passage de niveau*/
void niveauDetruireSaufJoueur(Niveau* niveau)
{
 niveauDetruireListe(niveau);  
 niveau_matriceDetruire(niveau);    
}
     

/* évolution du temps du niveau*/
void niveauEvoluerTemps(Niveau* niveau)
{
 static int b = 0;
 if (niveauLireEtat(niveau)==NIVEAU_ETAT_EN_COURS)
 {
  if (b==0)
  {
    niveau->private_temps = tempsRecuperer ();
    b=1;
    
  }
 
  if (tempsInferieur(tempsRecuperer(), tempsSomme(niveau->private_temps, 1 SECONDES))==0) {
   niveau->private_temps = tempsRecuperer ();
   niveau->temps_restant--;
   }
  if (niveauLireTempsRestant(niveau) <=0)
  {
   niveau->temps_restant=0;
   niveauEcrireEtat(niveau, NIVEAU_ETAT_PLUS_DE_TEMPS);
  }
 }
 else 
   b=0;
 
}





/* chargement à partir d'un fichier*/
/* Les fichiers sont enregistrés au format :
       
TEMPS_NIVEAU= 150
TEMPS_MUR= 1
VITESSE_SLIM= 1 
NOMBRE_DIAMANT= 17
LONGUEURX= 40
LONGUEURY= 22

########################################
#000000 00D0P 00000P0P000000000000P0000#
#0PBP000000 000000000PD00P0000 00000 00#
#0000000000 00 00000P0P00P00000000P0000#
#P0  000000000P000000P00P0000P000P00000#
#P0PP000000000PP00P00000000P000000P0P 0#
#000P00P00000000P00000P0 P00000000P0PP0#
#MMMMMMMMMMMMMMMMMMMMMMMMMMMMMM000P00P0#
#0 000P00D0 00P0P0000000000D0P0000000P0#
#00D00000P00000 00000000P  P00D0000P000#
#000P00P0P00000000000000PP0P00P00000000#
#0 00000P00000000PP 0000000P00P0D0000 0#
#0P00 00P0  00000D0PD00D0000P000P00D0P0#
#0DP00000000000000PPD00P00000000D00000P#
#00000000MMMMMMMMMMMMMMMMMMMMMMMMMMMMMM#
#  00000000 000D0000P00000P000P00000000#
#P 000000000PP00P00000000P000000P0P 00F#
#0P00P00000000P00000P0  0000D000P0PP000#
#0000PD00 00000000D000000P0PD000000P000#
#000 00 0P000P0PP00000000P0PD000000P00P#
#0D0000P000000 00000000 0P00P0000P000P0#
########################################

Il suffit de lire les longueurs et les données, puis de lire caractère par caractère et
d'allouer à chaque fois la mémoire pour chaque objet
*/



int niveauChargement (Niveau* niveau, char* c)
{
  Fichierl* f; 
  char chaine_tempo [50];
  int i;
  int j;
  int lecture_tempo_int;
  char lecture_tempo_char;
  Ennemi* ennemi;
  Pierre* pierre;
  Diamant* diamant;
  Joueur* joueur;
  Explosion* explosion;
  
  niveauDetruireListe(niveau);

  int controle = 0;
  
  if( (f =fichierlOuvrir(c)) == NULL) {
     fprintf(stderr, "Erreur de lecture du fichier %s\n", c);
     return EXIT_FAILURE;
  }
  for (i=1; i<=6; i++) {
   if( fichierlLireEgale(f, chaine_tempo) == FIN_DE_FICHIER)
     fprintf(stderr, "** Probleme lecture fichier\n");
  
  /* on charge les données relatives au niveau*/
   if (strcmp("TEMPS_NIVEAU", chaine_tempo)==0)
     {
       controle++;
       fscanf(f, "%d", &lecture_tempo_int);
       niveau->temps_restant = lecture_tempo_int;
       fscanf(f, "%c", &lecture_tempo_char);
     } 
     else
      if (  strcmp("TEMPS_MUR", chaine_tempo)==0)  
       {
         controle++;
         if (fichierlLireEntier(f, &lecture_tempo_int) == FIN_DE_FICHIER)
           fprintf(stderr, "** Probleme format fichier");
         mur_qui_bougeEcrireTour(&niveau->mur, lecture_tempo_int);
       }
       else
        if ( strcmp("VITESSE_SLIM", chaine_tempo)==0)
        {
             controle++;
             if (fichierlLireEntier(f, &lecture_tempo_int) == FIN_DE_FICHIER)
                fprintf(stderr, "** Probleme format fichier");
             slimEcrireVitesse(&niveau->slim, lecture_tempo_int);
        }
         else
        if ( strcmp("NOMBRE_DIAMANT", chaine_tempo)==0)
        {
             controle++;
             if (fichierlLireEntier(f, &lecture_tempo_int) == FIN_DE_FICHIER)
               fprintf(stderr, "** Probleme format fichier");
             niveau->nombre_de_diamant_restant = lecture_tempo_int;
        }
         else
         if ( strcmp("LONGUEURX", chaine_tempo)==0)
        {
             controle++;
             if (fichierlLireEntier(f, &lecture_tempo_int) == FIN_DE_FICHIER)
                fprintf(stderr, "** Probleme format fichier");
             niveau->longueurx = lecture_tempo_int;
        }
        else
           if ( strcmp("LONGUEURY", chaine_tempo)==0)
        {
             controle++;
             if (fichierlLireEntier(f, &lecture_tempo_int) == FIN_DE_FICHIER)
                fprintf(stderr, "** Probleme format fichier");
             niveau->longueury = lecture_tempo_int;
        }
           
  }           
  if (controle != 6) {
    fprintf(stderr, "Probleme chargement fichier, champs manquants\n"); 
    return EXIT_FAILURE;
  }  
  
  /*si la matrice existe déjà, on la détruit*/
  if (niveau_matriceExiste(niveau))
    niveau_matriceDetruire(niveau);

  /* on la recréait*/    
  niveau->matrice = niveau_matriceCreer(niveau->longueurx*niveau->longueury*sizeof(Niveau_element));
  
      /* on lit caractère par caractère*/
  for ( j=1; j<= niveau->longueury ; j++)
   for (i=1; i<= niveau->longueurx ; i++)
   {
      if (fichierlLireCaractere(f, &lecture_tempo_char) == FIN_DE_FICHIER)
       {
         fprintf(stderr, "* Probleme fichier, element manquants\n");
         return EXIT_FAILURE;
       }  
       
       switch(lecture_tempo_char)
       {
       /* pour chaque caractère, on convertit en élément*/
         case '#':
           niveauEcrireMatrice(niveau, i, j, ELEMENT_BORD); 

           break;
         case '0':
           niveauEcrireMatrice(niveau, i, j, ELEMENT_TERRE); 
           break;    
        case 'M':
          niveauEcrireMatrice(niveau, i, j, ELEMENT_MUR);
          break;            
        case ' ':
          niveauEcrireMatrice(niveau, i, j, ELEMENT_VIDE);
          break;   
        /* dans les cas suivants, ce sont des objets complexes, on alloue
        la mémoire et on initialise*/
        case 'x':
          explosion = explosionCreer();
          liste_explosionAjouterFin(niveauLireListeExplosion(niveau), explosion);
          niveauEcrireMatrice(niveau, i, j, ELEMENT_EXPLOSION);
          break;
        
        case '1':
          ennemi = ennemiCreer ();
          ennemiCopie(ennemi, i, j,  ENNEMI_TYPE_RIEN,ENNEMI_DIRECTION_GAUCHE);
          liste_ennemiAjouterFin(niveauLireListeEnnemi(niveau), ennemi);
          niveauEcrireMatrice(niveau, i, j, ELEMENT_ENNEMI);
          break;
        case '2':
          ennemi = ennemiCreer ();
          ennemiCopie(ennemi, i, j,  ENNEMI_TYPE_DIAMANT,ENNEMI_DIRECTION_GAUCHE);
          liste_ennemiAjouterFin(niveauLireListeEnnemi(niveau), ennemi);
          niveauEcrireMatrice(niveau, i, j, ELEMENT_ENNEMI);
          break;  
        case '3':
          ennemi = ennemiCreer ();
          ennemiCopie(ennemi, i, j,  ENNEMI_TYPE_PIERRE,ENNEMI_DIRECTION_GAUCHE);
          liste_ennemiAjouterFin(niveauLireListeEnnemi(niveau), ennemi);
          niveauEcrireMatrice(niveau, i, j, ELEMENT_ENNEMI);
          break;                         
        case 'D':
          diamant= diamantCreer();
          diamantCopie(diamant, i, j, DIAMANT_TYPE_DIAMANT, DIAMANT_TRANSFORMATION_DIAMANT);
          liste_diamantAjouterFin(niveauLireListeDiamant(niveau), diamant);
          niveauEcrireMatrice(niveau, i, j, ELEMENT_DIAMANT);
          break;
        case 'E':
          diamant= diamantCreer();
          diamantCopie(diamant, i, j, DIAMANT_TYPE_TRANSFORMATION, DIAMANT_TRANSFORMATION_DIAMANT);
          liste_diamantAjouterFin(niveauLireListeDiamant(niveau), diamant);
          niveauEcrireMatrice(niveau, i, j, ELEMENT_DIAMANT);
          break;
        case 'P':
          pierre= pierreCreer();
          pierreCopie(pierre, i, j, PIERRE_TYPE_PIERRE); 
          liste_pierreAjouterFin(niveauLireListePierre(niveau),pierre);
          niveauEcrireMatrice(niveau, i, j, ELEMENT_PIERRE);
          break;
        case 'o':
          pierre= pierreCreer();
          pierreCopie(pierre, i, j, PIERRE_TYPE_EXPLOSION); 
          liste_pierreAjouterFin(niveauLireListePierre(niveau),pierre);
          niveauEcrireMatrice(niveau, i, j, ELEMENT_PIERRE);
          break;            
        case 'S':
          niveauEcrireMatrice(niveau, i, j, ELEMENT_SLIM);
          slimEcrireType(niveauLireSlim(niveau), SLIM_TYPE_DIAMANT);
          break;
        case 'T':
          niveauEcrireMatrice(niveau, i, j, ELEMENT_SLIM);
          slimEcrireType(niveauLireSlim(niveau), SLIM_TYPE_PIERRE);
          break;             
        case '-':
          niveauEcrireMatrice(niveau, i, j, ELEMENT_MUR_QUI_BOUGE);
          break;
        case 'F':
          niveauEcrireMatrice(niveau, i, j, ELEMENT_PORTE_DE_FIN);
          break;
        case 'B':
          if (liste_joueurTaille(niveauLireListeJoueur(niveau)) == 0) {
            niveau->nombre_de_joueur++;
            joueur = joueurCreer();
            joueurEcrireEtat(joueur , JOUEUR_ETAT_VIVANT);
            liste_joueurAjouterFin(niveauLireListeJoueur(niveau), joueur);
          }
          else
          joueur = liste_joueurLire(niveauLireListeJoueur(niveau),1);  
             
          joueurEcrireTouche(joueur, TOUCHE_BAS, JOUEUR1_TOUCHE_BAS);
          joueurEcrireTouche(joueur, TOUCHE_HAUT, JOUEUR1_TOUCHE_HAUT);
          joueurEcrireTouche(joueur, TOUCHE_GAUCHE, JOUEUR1_TOUCHE_GAUCHE);
          joueurEcrireTouche(joueur, TOUCHE_DROITE, JOUEUR1_TOUCHE_DROITE);
          joueurEcrireTouche(joueur, TOUCHE_COMBINER, JOUEUR1_TOUCHE_COMBINER);
          joueurEcrireTouche(joueur, TOUCHE_AUTOSUICIDE, JOUEUR1_TOUCHE_AUTOSUICIDE);
          joueurEcrireX(joueur, i);
          joueurEcrireY(joueur, j);
          joueurEcrireForme(joueur, JOUEUR_FORME_BOULDER);
          niveauEcrireMatrice(niveau, i, j, ELEMENT_JOUEUR);  
          joueurEcrireEtat(joueur, JOUEUR_ETAT_VIVANT);
          break;
         default:
           break;                       
                                       
                                 
                                 
                                 
       }
       
       
       
       
       
       
       
       
       
   }      
  
  fichierlFermer(f);   
  return EXIT_SUCCESS;   
}


/* accesseurs*/
Mur_qui_bouge* niveauLireMurQuiBouge(Niveau* niveau)
{
   return &niveau->mur;
}

Slim* niveauLireSlim(Niveau* niveau)
{
      return &niveau->slim;
}

void niveauDecrementerDiamantRestant(Niveau* niveau)
{
     niveau->nombre_de_diamant_restant--;
}

int niveauLireNombreDeJoueur(Niveau* niveau)
{
    return niveau->nombre_de_joueur;
}

int niveauLireLongueurX(Niveau* niveau)
{
    return niveau->longueurx;
}

int niveauLireLongueurY(Niveau* niveau)
{
    return niveau->longueury;
}

Liste_pierre* niveauLireListePierre(Niveau* niveau)
{
              return &niveau->liste_pierre;
}

Liste_diamant* niveauLireListeDiamant(Niveau* niveau)
{
               return &niveau->liste_diamant;
}

Liste_ennemi* niveauLireListeEnnemi(Niveau* niveau)
{
              return &niveau->liste_ennemi;
}

Liste_joueur* niveauLireListeJoueur(Niveau* niveau)
{            
             return &niveau->liste_joueur;
}

Liste_explosion* niveauLireListeExplosion(Niveau* niveau)
{
            return &niveau->liste_explosion;
}


Niveau_etat niveauLireEtat(Niveau* niveau)
{
  return niveau->etat;
}
  
void niveauEcrireEtat(Niveau* niveau, Niveau_etat etat)
{
     niveau->etat = etat;
}

int  niveauLireNombreDiamantRestant(Niveau* niveau)
{
     return niveau->nombre_de_diamant_restant;
}


Niveau_porte niveauLirePorte(Niveau* niveau)
{
             return niveau->porte;
}

void niveauEcrirePorte(Niveau* niveau, Niveau_porte porte)
{
     niveau->porte = porte;
}

int niveauLireTempsRestant(Niveau* niveau)
{
    return niveau->temps_restant;
}
