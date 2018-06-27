#include "../niveau/niveau.h"
#include "environnement.h"

/*Permet de faire évoluer le temps de l'ensemble des objets du niveau
 * On met à joueur le temps du mur qui bouge ici car il est indépendant du niveau
 * a priori
 */
void environnementEvoluerTemps(Niveau* niveau)
{
 niveauEvoluerTemps(niveau);
 mur_qui_bougeEvoluerTemps(niveauLireMurQuiBouge(niveau));
//fprintf(stderr, "  %d  ", niveauLireTempsRestant(niveau));

/* Test si il reste du temps pour le joueur*/
if (niveauLireTempsRestant(niveau) <= 0 && niveauLireEtat(niveau) == NIVEAU_ETAT_EN_COURS)
   niveauEcrireEtat(niveau, NIVEAU_ETAT_PLUS_DE_TEMPS);
   
}





/* On fait évoluer chaque type d'objet (par exemple pour la chute des pierres...*/
void environnementEvoluer(Niveau* niveau)
{
 environnementEvoluerListeDiamant(niveau);
 environnementEvoluerListePierre(niveau);    
 environnementEvoluerListeExplosion(niveau);    
 environnementEvoluerListeEnnemi(niveau);
 environnementEvoluerSlim(niveau);
 
 environnementEvoluerTemps(niveau);
 
 /*Ouvre la porte si le nombre suffisant de diamant a été pris*/
 if (niveauLireNombreDiamantRestant(niveau) <=0)
  niveauEcrirePorte(niveau, NIVEAU_PORTE_OUVERTE);
      
     
}
