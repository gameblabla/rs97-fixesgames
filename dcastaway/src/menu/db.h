#ifndef DB_H
#define DB_H

#include<SDL.h>

#include "dcastaway.h"
#include "config.h"
#include "unzip.h"
#include "st/st.h"

#define DBFILE DATA_PREFIX "stgames23.gz"
#define CACHEFILE "stcache3.txt"

/* should be a true CRC32 but instead its a varient of the crc32 found all over
 * the net; using it so as to easily match up with various other CRC32 db's
 * around the net.
 */
Uint32 db_calculate_id ( char *buf, Uint32 length );
Uint32 db_calculate_id_on_file ( char *filename );
void db_crc_init ( void );

/* retrieving a file to crc32 it is slow; as such, we'll keep a cache of the
 * already calculated values. Should make things faster.
 */
typedef struct _cache_t {
  char c_filename [ 128 ];
  Uint32 c_crc32;
  /* could use char **c_crc32[] but why bother? Just cache the CRC32 of the
   * file in question, since building the CRC is the slow part. Likewise,
   * game lookup should be fast enough, so no need to cache char **c_gamelist
   * or the like.
   */
  struct _cache_t *next;
} cache_t;

void cache_free(void);
Uint8 cache_retrieve (void);
Uint8 cache_save ( void );
cache_t *cache_find ( char *filename );
Uint8 cache_append ( char *filename, Uint32 crc );
cache_t *cache_head ( void ); // should not use!

/* The actual database detail is hidden in the below functions; could just use
 * cheesie strstr() to look up, or a fast hashtable or the like. Whatever :)
 */
//typedef struct {
//  char **db_gamelist;
//} db_t;

Uint8 db_retrieve ( void );
Uint8 db_find ( Uint32 crc, Uint16 *r_num, char ***r_gamelist );
void db_describe ( Uint32 crc, char *r_buf, Uint16 buflen );
void db_free ( Uint8 numgames, char **gamelist );

#endif
