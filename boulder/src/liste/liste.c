#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>

#include "liste.h"

/* liste généralisée (avec des void*) chaînée à double sens, plus d'informations
dans le fichier .h*/


Liste_cellule* celluleCreer ();
void celluleDetruire (Liste_cellule* cellule) ;
void celluleModifier (Liste_cellule* cellule, ElementListe* element, Liste_cellule* precedent, Liste_cellule* suivant);







void celluleDetruire (Liste_cellule* cellule) 
{
 free (cellule);     
}

Liste_cellule* celluleCreer ()
{
 return ((Liste_cellule* ) malloc(sizeof(Liste_cellule)));      
}

void celluleModifier (Liste_cellule* cellule, ElementListe* element, Liste_cellule* precedent, Liste_cellule* suivant)
{
  cellule->element = element;
  cellule->precedent = precedent;
  cellule->suivant = suivant;  
     
}

Liste_cellule* celluleObtenir (Liste* liste, int position)
{
  int i;
  Liste_cellule* cellule_en_cours;
  
  if ((listeTaille(liste) < position) || (position<1))
  {
    fprintf(stderr, "\n** Module Liste: celluleObtenir: Lecture hors borne\n");             
    return NULL;
  }

  if ((liste->derniere_cellule_lue != NULL) && 
     (position == liste->derniere_position+1))
  {
     cellule_en_cours = liste->derniere_cellule_lue->suivant;                             
  }
  else
  {
    if ((liste->derniere_cellule_lue != NULL) && 
     (position == liste->derniere_position))
       cellule_en_cours = liste->derniere_cellule_lue;
    else {   
       cellule_en_cours = liste->premier;              
       for (i=1; i< position; i++)
         cellule_en_cours = cellule_en_cours->suivant;  
   }
  }
  
  
  
  return cellule_en_cours;               
}







void listeInitialiser (Liste* liste)
{
  liste->premier = NULL;
  liste->dernier = NULL;
  liste->taille = 0;
  liste->derniere_cellule_lue = NULL;
  liste->derniere_position = 0;
}

int listeTaille(Liste* liste)
{
 return liste->taille;   
}

ElementListe* listeLire(Liste* liste, int position)
{
  Liste_cellule* cellule_en_cours;
  
  cellule_en_cours = celluleObtenir(liste, position);
  if (cellule_en_cours == NULL)
  {
    fprintf(stderr, "\n** Module Liste: listeLire: Lecture hors borne\n");                     
   // fprintf(stderr, " lecture en %d mais taille %d ", position, listeTaille(liste));
    return NULL;
  }
      
  liste->derniere_cellule_lue = cellule_en_cours;
  liste->derniere_position = position;
  return (cellule_en_cours->element);            
             
}

void listeEcrire(Liste* liste, int position, ElementListe* element)
{
 Liste_cellule* cellule_en_cours ;
 
 cellule_en_cours = celluleObtenir(liste, position);
 if (cellule_en_cours == NULL)
 {
      fprintf(stderr, "\n**Module Liste: celluleObtenir: Ecriture hors borne\n");
      return;
 }
   
 liste->derniere_cellule_lue = cellule_en_cours;
 liste->derniere_position = position;
 cellule_en_cours->element = element; 
     
}

void listeRetirer (Liste* liste, int position)
{
  Liste_cellule* cellule_en_cours;
  
  cellule_en_cours = celluleObtenir(liste, position);
  if (cellule_en_cours == NULL)
    return;
  
  if (position == 1) {
     if (listeTaille(liste) == 1) {
       celluleDetruire(liste->premier); 
     } 
     else {   
       celluleDetruire(liste->premier);
       liste->premier = liste->premier->suivant;
       liste->premier->precedent = NULL;
     }
  } 
  else {
     if (position == liste->taille) {
        celluleDetruire(liste->dernier);
        liste->dernier = liste->dernier->precedent;
        liste->dernier->suivant = NULL;
     }
     else {
        
        cellule_en_cours->precedent->suivant = cellule_en_cours->suivant;
        cellule_en_cours->suivant->precedent = cellule_en_cours->precedent;
        celluleDetruire(cellule_en_cours);
     }
  }
  
  liste->taille--;
  liste->derniere_cellule_lue = NULL;
  liste->derniere_position = position;
  return;
}


void listeAjouter (Liste* liste, int position, ElementListe* element)
{

  int i;
  Liste_cellule* cellule_en_cours;
  Liste_cellule* cellule_cree;
  
  if ((position < 1) || (position > (liste->taille+1)))
  {
     fprintf(stderr, "\n** Module Liste: listeAjouter: Lecture hors borne\n");
     return;
  } 
  
  cellule_cree = celluleCreer();
  if (cellule_cree == NULL)
  { 
      fprintf(stderr, "\n** Module Liste: listeAjouter: Allocation d'une nouvelle cellule impossible\n");
      return;
  }

       
  if (position==1) {
     if (liste->taille == 0) {
        liste->premier = cellule_cree;
        liste->dernier = cellule_cree;
        celluleModifier(cellule_cree, element, NULL, NULL);               
     }
     else {
          
        liste->premier->precedent = cellule_cree;
        celluleModifier(cellule_cree, element, NULL, liste->premier);
        liste->premier = cellule_cree; 
     }
  }
  else {
     if (position == liste->taille+1) {
        liste->dernier->suivant = cellule_cree;
        celluleModifier(cellule_cree, element, liste->dernier, NULL);
        liste->dernier = cellule_cree;
     }
     else {
        for (i=1; i<=position;i++)    
          cellule_en_cours = cellule_en_cours->suivant;
        
        celluleModifier(cellule_cree, element, cellule_en_cours, cellule_en_cours->suivant);
        cellule_en_cours->suivant->precedent = cellule_en_cours;
        cellule_en_cours->suivant = cellule_cree;
    }
  }
   
  liste->taille++;
  liste->derniere_cellule_lue = NULL;
  return;      
     
}

void listeAjouterFin(Liste* liste, ElementListe* element)
{
 listeAjouter(liste, liste->taille+1, element);    
}

void listeDetruire(Liste* liste)
{
  Liste_cellule* suivant;
  Liste_cellule* en_cours = liste->premier;
  int i;
  int taille = listeTaille(liste);
  
  if (liste->taille == 0)
    return;

    
  for (i=1; i<=taille; i++) {
    suivant = en_cours->suivant;
    celluleDetruire(en_cours);
    en_cours = suivant;
    liste->taille--;
  }
  if (liste->taille != 0)
    fprintf(stderr, "\n**Module Liste: listeDetruire: probleme coherence de taille");  
}

