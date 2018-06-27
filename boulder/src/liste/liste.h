#ifndef _LISTE_H
#define _LISTE_H

/* Topic: Documentation du module Liste
 * Module permettant l'utilisation d'une liste généralisée à tous les types.
 * 
 * Une liste doit toujours être initialisée avant l'utilisation de celle-ci.
 * Lorsque l'utilisateur n'en a plus besoin, il faut la détruire par libérer l'espace
 * alloué par celle-ci.
 */

typedef void ElementListe;


typedef struct Liste_cellule_ {
        ElementListe* element;
        struct Liste_cellule_ * suivant;
        struct Liste_cellule_ * precedent;
        } Liste_cellule;
        
typedef struct Liste_ {
        Liste_cellule* premier;
        Liste_cellule* dernier;
        int            taille;
        Liste_cellule* derniere_cellule_lue;
        int            derniere_position;
               
        } Liste;
        
/*constructeur et destructeur*/
/**Function: listeInitialiser
 * Initialisation d'une liste
 *
 * Paramètres:
 * liste - liste devant être initialisée
 */
void listeInitialiser (Liste* liste);

/**Function: listeDetruire
 * Destruction de toute la liste à partir de son pointeur
 *
 * Paramètres
 * liste - liste devant être détruite
 */
void listeDetruire (Liste* liste);

/*modificateurs d'états*/

/**Function: listeAjouter
 * Permet d'ajouter un pointeur vers un élément à une liste
 *
 * Paramètres
 * liste - la liste ou doit être ajouté l'élément
 * position - la position ou ajouter l'élément
 * element - le pointeur vers l'élément à ajouter
 *
 * Erreur:
 *  Dans le cas d'une limite mémoire, un message d'erreur sera affiché sur la sortie stderr
 *  Dans le cas d'une tentative d'ajout hors borne, un message d'erreur est affiché sur la sortie stderr
 */
void listeAjouter (Liste* liste, int position, ElementListe* element);

/**Function: listeRetirer
 * Permet de retirer un élément d'une liste
 *
 * Paramètres
 * liste - la liste ou doit être retiré l'élément
 * position - la position ou retirer l'élément
 *
 * Erreur:
 *  Dans le cas d'une tentative d'ajout hors borne, un message d'erreur est affiché sur la sortie stderr
 */
void listeRetirer (Liste* liste, int position);

/**Function: listeAjouterFin
 * Permet d'ajouter un élément à la fin d'un fichier
 *
 * Paramètres
 * liste - la liste ou doit être ajoutél'élément
 * element - l'élément à ajouter
 *
 * Erreur:
 *  Dans le cas d'un problème mémoire, un message d'erreur est affiché sur la sortie stderr
 */
void listeAjouterFin(Liste* liste, ElementListe* element);

void listeEcrire(Liste* liste, int position, ElementListe* element);

/*accesseurs*/
/**Function: listeLire
 * Permet de recuperer un élément d'une liste
 *
 * Paramètre:
 * liste - la liste ou l'on doit lire l'élément
 * position - position de l'élément
 *
 * Retour: 
 * Un pointeur vers l'élément lu
 *
 * Erreur:
 *  Dans le cas d'une tentative d'écriture hors borne, le pointeur NULL est retourné
 */
ElementListe* listeLire (Liste* liste, int position);

/**Function: listeTaille
 *  Détermine la taille d'une liste
 * Paramètre:
 * liste - liste à déterminer la taille
 *
 * Retour:
 *  Le taille de la liste
 */
int listeTaille (Liste* liste);

int listeEstVide (Liste* liste);
//ElementListe* listeLire(Liste* liste, int position);
#endif
