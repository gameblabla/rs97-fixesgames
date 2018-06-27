#ifndef _MOTEUR_GRAPHIQUE_H
#define _MOTEUR_GRAPHIQUE_H
#include <SDL.h>

#ifndef SANS_TTF
#include <SDL_ttf.h>
#endif

/**************************
 * IMPORTANT :
 *
 * La création d'un module moteur_graphique, permet de rendre assez 
 * indépendant le jeu du moteur SDL
 */
 


/*Title: Module moteur_graphique
 * 
 */
 
 
/*Topic: Documentation du module moteur_graphique
 * Ce module permet la réalisation d'application graphique en mode double buffering.
 * Il est compatible sous tous les systèmes Windows, Linux, UNIX, Mac
 */
 

typedef SDL_Surface G_surface;
typedef SDL_Event G_evenement;


typedef SDL_Rect G_rectangle;


typedef Uint32 G_couleur;

#ifndef SANS_TTF


typedef TTF_Font G_font;
#else
typedef int G_font;
#endif

typedef enum {
        PLEIN_ECRAN,
        PAS_PLEIN_ECRAN
        } G_fenetre_mode;
        


typedef struct {
        int resolutionx;
        int resolutiony;
        G_surface* surface;
        G_fenetre_mode mode;
} G_fenetre;

/**Classes: couleur
 * Type abstrait de donnée couleur utilisé dans les surfaces
 */
/*
 * Function: g_couleurCreer
 * Permet la création d'une couleur RGB en fonction d'une surface, il y a dependances à cause notamment
 * de différence entre une surface 32 ou 16 bits par exemple.
 *
 * Paramètres:
 * surface - la couleur est définie selon la surface
 * R - niveau de rouge (entre 0 et 255)
 * G - niveau de vert (entre 0 et 255)
 * B - niveau de bleu (entre 0 et 255)
 *
 * Retour:
 * La couleur de niveau RGB
 *
 * Précondition:
 * R, G et B doivent être entre 0 et 255
 */
G_couleur g_couleurCreer(G_surface* surface, int R, int G, int B);

/**Classes: rectangle
 * Les copies de surface se réalise à partir de ce type abstrait
 */
 
/*
 * Function: g_rectangleInitialiser
 * Permet d'initialiser un rectangle à une certaine position et d'une certaine taille
 *
 * Paramètres:
 * x - position horizontale
 * y - position verticale
 * w - taille horizontale
 * h - taille verticale
 *
 */
void g_rectangleInitialiser(G_rectangle* rect, int x, int y, int w, int h);



/*Classes: font
 * Le type abstrait font
*/
 /** Function: g_fontCharger
 * Permet de charger une police TTF à une certaine taille
 *
 * Paramètres:
 * fichier - le fichier du disque dur au format TTF
 * taille - la taille de la police voulue
 *
 * Retour: 
 *  Renvoit un pointeur de type G_font correpondant au font chargé
 *
 * Erreur:
 *  Si le fichier n'est pas trouvé, NULL est retourné
 */
G_font*   g_fontCharger(char* fichier, int taille);
/** Function: g_fontDetruire
 * Permet la déstruction d'une font
 *
 * Paramètre:
 * font - la font que l'on souhaite détruire
 */
void g_fontDetruire(G_font* font);


/*Classes: G_surface
 * Ce type abstrait est une surface graphique se trouvant en mémoire vidéo. Il est similaire à une image
 */

/* Function: g_surfaceCreer
 * Permet la création d'une surface d'une certaine taille, la surface est en 32 bits.
 *
 * Paramètres:
 * x - taille horizontale de la surface souhaitée
 * y - taille verticale de la surface souhaitée
 *
 * Retour:
 *  Retourne un pointeur vers la surface qui a été allouée
 *
 * Erreur:
 *  Si la fonction n'a pas réussi à allouer la mémoire pour la surface, NULL est renvoyé
 */
G_surface* g_surfaceCreer(int x, int y);

/**Function: g_surfaceEcrireCouleur
 * Permet d'écrire à une certaine position d'une surface une couleur
 *
 * Paramètres:
 * surface - la surface ou l'on souhaite écrire
 * x - la position horizontale
 * y - la position verticale
 * couleur - la couleur que l'on souhaite écrire
*/
void       g_surfaceEcrireCouleur(G_surface* surface, int x, int y, G_couleur couleur);

/**Function: g_surfaceCopie
 * Permet de copier une surface vers une autre à certaines positions et à certaine taille
 *
 * Paramètres:
 * surface1 - la surface source
 * rect1 - le rectangle contient les origines et les longueurs que l'on souhaite copier
 * surface2 - la surface destination
 * rect2 - le rectangle contient les origines et les longueurs de l'endroit que l'on souhaite copier
 *
 * Si l'on souhaite copier toute la surface source, on peut plaçer rect1 à NULL, si l'on souhaite uniquement
 * placé la source dans la destination à une certaine position sans se soucier des tailles, il suffit
 * d'initialiser un rectangle avec les tailles à 0
 */
void       g_surfaceCopie(G_surface* surface1, G_rectangle* rect1, G_surface* surface2, G_rectangle* rect2);

/**Function: g_surfaceEcrireTexte
 * Ecrit un texte dans une surface et renvoit la surface correspondante, la surface est allouée automatiquement
 *
 * Paramètres:
 * font - le font de police que l'on souhaite
 * texte - le texte que l'on souhaite écrire
 * couleur_texte - la couleur du texte
 * couleur_font - la couleur du fond
 *
 * Retour:
 *  Retourne une surface contenant le texte aux couleurs indiqués et au font précisé
 */
G_surface*      g_surfaceEcrireTexte(G_font* font, char* texte, G_couleur couleur_texte, G_couleur couleur_fond);

/**Function: g_surfaceChargerBMP
 * Permet de charger un fichier BMP du disque dur directement dans un type surface
 * Les tailles de la surface sont automatiquement écrite, la mémoire est automatiquement allouée.
 * La surface est allouée en mémoire vidéo.
 *
 * Paramètre:
 * c - chemin d'accès au fichier BMP
 *
 * Retour:
 * La fonction retourne un pointeur de surface correspondante à l'image BMP, si le fichier est introuvable
 * ou si la mémoire n'a pas été allouée, le pointeur NULL sera retourné.
 */
G_surface* g_surfaceChargerBMP(char* c);

/**Function g_surfaceLongueurx
 * Permet de lire la longueur d'une surface
 *
 * Paramètre:
 * s - la surface que l'on souhaite déterminer la longueur
 *
 * Retour:
 * La longueur de la surface
 */
int g_surfaceLongueurx(G_surface* s);

/**Function g_surfaceLongueury
 * Permet de lire la hauteur d'une surface
 *
 * Paramètre:
 * s - la surface que l'on souhaite déterminer la hauteur
 *
 * Retour:
 * La hauteur de la surface
 */
int g_surfaceLongueury(G_surface* s);

/**Function: g_surfaceEfface
 * Permet d'effacer toutes les couleurs de la surface par un rectangle noir recouvrant le tout
 *
 * Paramètre:
 * s - la surface que l'on souhaite effacer
 */
void g_surfaceEfface(G_surface* s);

/**Function: g_surfaceDetruire
 * Permet de détruire une surface que l'on a alloué par g_surfaceCreer ou g_surfaceCharger
 *
 * Paramètre:
 * s - la surface que l'on souhaite détruire
 */
void       g_surfaceDetruire(G_surface* surface);


/*Classes: G_fenetre
 * Ce type abstrait de donné est la fenetre principale graphique de l'application. Elle doit être unique.
 * Le type nécessite une initialisation préliminaire, et une destruction finale. La 
 * G_fenetre* contient
 * une surface graphique qui representera ce qui est affiché dedans.
 */

/*
 * Function: g_fenetreInitialiser
 * Permet l'initialisation d'une *G_fenetre* en fonction de resolution et d'un mode plein écran ou non
 *
 * Paramètres: 
 * fenetre - la fenetre à initialiser
 * resolutionx - la résolution verticale de la fenetre
 * resolutiony - la résolution horizontale de la fenetre
 * mode - le mode plein écran ou non plein écran, les valeurs possibles sont: PLEIN_ECRAN et PAS_PLEIN_ECRAN
 *
 * Retour: 
 * Si la fonction a réussi à initialiser la fenetre, EXIT_SUCCESS est retournée, dans le cas contraire
 * EXIT_FAILURE est renvoyé
 */
int        g_fenetreInitialiser(G_fenetre* fenetre, int resolutionx, int resolutiony,  G_fenetre_mode mode);

/**Function: g_fenetreLireSurface
 * Permet de récuperer la surface graphique d'une fenetre
 *
 * Paramètres: 
 * fenetre - la fenetre que l'on souhaite recuperer la surface
 *
 * Retour:
 * Un pointeur vers la surface à recuperer
 */
G_surface* g_fenetreLireSurface(G_fenetre*);

/**Function: g_fenetreAfficher
 * Permet d'afficher la fenetre à l'écran. Donc de mettre à jour la représentation
 * Il est conseillé de la mettre à jour à chaque tour de boucle, l'utilité de cette fonction réside
 * dans l'utilisation du double buffering pour éviter le phénomène de clignotement
 */
void       g_fenetreAfficher(G_fenetre*);

/**Function: g_fenetreDetruire
 * Detruit la fenetre
 *
 * Paramètre:
 * fenetre - la fenetre que l'on souhaite détruire
 */
void       g_fenetreDetruire(G_fenetre* fenetre); /*quitte tout*/


#endif

