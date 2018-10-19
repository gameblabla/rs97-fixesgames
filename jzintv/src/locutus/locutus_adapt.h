#ifndef LOCUTUS_ADAPT_H_
#define LOCUTUS_ADAPT_H_

#ifdef __cplusplus
extern "C" 
{
#endif

#include "config.h"
#include "periph/periph.h"
#include "cp1600/cp1600.h"

typedef struct t_locutus_priv t_locutus_priv;

typedef struct t_locutus_wrap  
{
    periph_t        periph;         /*  Peripheral structure.           */
    t_locutus_priv *locutus_priv;   /*  Actual Locutus object ptr.      */
} t_locutus_wrap;


int make_locutus
(
    t_locutus_wrap  *loc_wrap,      /*  pointer to a Locutus wrapper    */
    const char      *luigi_file,    /*  LUIGI file to load into Locutus */
    cp1600_t        *cp1600
);



#ifdef __cplusplus
}
#endif

#endif
