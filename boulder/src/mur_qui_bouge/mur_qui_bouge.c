#include "../evenement/temps/temps.h"

#include "mur_qui_bouge.h"

int mur_qui_bougeLireCompteurGraphique(Mur_qui_bouge* mur)
{
  return mur->compteur_graphique;
}

void mur_qui_bougeEcrireCompteurGraphique( Mur_qui_bouge* mur, int i)
{
     mur->compteur_graphique = i;
}

Mur_qui_bouge_etat mur_qui_bougeLireEtat(Mur_qui_bouge* mur)
{
   return mur->etat;
}

void mur_qui_bougeEcrireEtat(Mur_qui_bouge* mur, Mur_qui_bouge_etat etat)
{
     mur->etat=etat;
}



void mur_qui_bougeEcrireTour( Mur_qui_bouge* mur, int i)
{
     mur->tour_de_boucle_restant = i;
}

void mur_qui_bougeInitialiser (Mur_qui_bouge* mur)
{
   mur->private_temps = tempsRecuperer();
}

void mur_qui_bougeEvoluerTemps (Mur_qui_bouge* mur)
{
 static int bo=  0;
 if( mur->etat == MUR_QUI_BOUGE_ETAT_EN_COURS)
 {
 
  if (bo==0) {
    mur->private_temps = tempsRecuperer();
    bo=1;
  }
  if (tempsInferieur(tempsRecuperer(), tempsSomme(mur->private_temps, 1 SECONDES))==0) {
     mur->private_temps = tempsRecuperer ();
     mur->tour_de_boucle_restant--;    
     if (mur->tour_de_boucle_restant <= 0 && mur->etat!= MUR_QUI_BOUGE_ETAT_ABSENT) {
        mur->etat = MUR_QUI_BOUGE_ETAT_FINI;
        mur->tour_de_boucle_restant--;
     }
  }
 }

}

