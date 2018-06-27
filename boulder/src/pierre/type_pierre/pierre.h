#ifndef _PIERRE_H
#define _PIERRE_H
typedef enum {
        PIERRE_DIRECTION_BAS,
        PIERRE_DIRECTION_GAUCHE,
        PIERRE_DIRECTION_DROITE,
        PIERRE_DIRECTION_HAUT,
        PIERRE_DIRECTION_BOUGE_PAS
        } Pierre_direction;
        
typedef enum {
        PIERRE_TYPE_PIERRE,
        PIERRE_TYPE_DIAMANT,
        PIERRE_TYPE_EXPLOSION,
        PIERRE_TYPE_ABSENT
        } Pierre_type;
        

        
        
        
        
        
typedef struct {
        int x;
        int y;
        Pierre_direction direction;
        Pierre_type type;
        int poids;
} Pierre;





Pierre* pierreCreer ();
void pierreDetruire(Pierre* pierre);


int pierreLireX (Pierre* pierre);
int pierreLireY (Pierre* pierre);
void pierreEcrireX(Pierre* pierre, int x);
void pierreEcrireY(Pierre* pierre, int y);


Pierre_direction pierreLireDirection(Pierre* pierre);
void pierreEcrireDirection(Pierre* pierre, Pierre_direction direction);

Pierre_type pierreLireType (Pierre* pierre);
void pierreEcrireType(Pierre* pierre, Pierre_type transformation);

int pierreLirePoids (Pierre* pierre);
void pierreEcrirePoids(Pierre* pierre, int poids);

void pierreCopie(Pierre* pierre, int i, int j, Pierre_type chute);
#endif
