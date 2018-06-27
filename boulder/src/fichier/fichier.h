#ifndef _FICHIER_H
#define _FICHIER_H
#define FIN_DE_FICHIER EOF
/* Topic: Documentation du module Fichier
 * Ce module permet la gestion de flux de fichier en lecture
 * en utilisant certaines opérations définies.
 */
 
typedef FILE Fichierl;

/**Function: fichierlOuvrir
 * Fonction permettant l'ouverture d'un fichier par rapport à son chemin d'accès
 * 
 * Paramètre:
 * c - chemin d'accès au fichier
 *
 * Retour: 
 *  Pointeur vers un fichierl
 *
 * Erreur:
 *  Si on ne trouve pas le fichier sur le disque dur, le pointeur NULL est renvoyé
 */
Fichierl* fichierlOuvrir(char* c);

/**Function: fichierlLireEntier
 * Fonction permettant de lire un entier dans un flux de type *Fichierl*
 *
 * Paramètres:
 * f - fichier à lire
 * i - l'entier lu sera écrit dans i
 *
 * Retour:
 *  Si l'entier a été lu, la fonction retourne EXIT_SUCCESS
 *
 * Erreur:
 *  Si la fin de fichier a été atteint avant d'obtenir un entier, FIN_DE_FICHIER est retourné
 */
int fichierlLireEntier (Fichierl* f, int* i);

/**Function: fichierlLireCaractere
 * Fonction permettant de lire un caractère dans un flux de type *Fichierl*. Les caractères spéciaux
 * tel que \n ou \0 sont ignorés
 * Paramètres:
 * f - fichier à lire
 * lu - le caractère lu sera écrit dans i
 *
 * Retour:
 *  Si l'entier a été lu, la fonction retourne EXIT_SUCCESS
 *
 * Erreur:
 *  Si la fin de fichier a été atteint avant d'obtenir un caractère, FIN_DE_FICHIER est retourné
 */
int fichierlLireCaractere (Fichierl*f, char* lu);

/**Function: fichierlLireLigne
 * Fonction permettant de lire une ligne de caractères dans un flux de type *Fichierl*. Les caractères spéciaux
 * tel que \n ou \0 sont ignorés
 * Paramètres:
 * f - fichier à lire
 * lu - on écrira dans cette chaine
 *
 * Retour:
 *  Si on a bien obtenu un caractère, la fonction retourne EXIT_SUCCESS
 *
 * Erreur:
 *  Si la fin de fichier a été atteint avant d'obtenir un caractère, FIN_DE_FICHIER est retourné.
 *  Si le fichier pointe sur NULL, EXIT_FAILURE est retourné
 */
int fichierlLireLigne (Fichierl* f, char* c);

/**Function: fichierlLireEgale
 * Fonction permettant de lire une ligne de caractères dans un flux de type *Fichierl* jusqu'à ce que le symbole = est rencontré. Les caractères spéciaux
 * tel que \n ou \0 sont ignorés
 * Paramètres:
 * f - fichier à lire
 * lu - on écrira dans cette chaine
 *
 * Retour:
 *  Si il n'existe plus de ligne, la fonction retourne EXIT_SUCCESS.
 *  c a été modifié tel que son dernier caractère soit \0
 *
 * Erreur:
 *  Si la fin de fichier a été atteint avant d'obtenir un caractère, FIN_DE_FICHIER est retourné.
 *  Si le fichier pointe sur NULL, EXIT_FAILURE est retourné
 */
int fichierlLireEgale (Fichierl* f, char* c);

/**Function: fichierlFermer
 * Fonction permettant la fermeture d'un *fichierl* 
 * 
 * Paramètre:
 * f - flux de fichier
 *
 * Retour: 
 *  EXIT_SUCCESS si on a bien réussi à fermer le fichier
 *
 * Erreur:
 *  Dans le cas d'un problème, EXIT_FAILURE est retourné.
 */
int fichierlFermer(Fichierl *f);

#endif
