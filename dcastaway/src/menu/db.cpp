#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<zlib.h>

#include "db.h"

static Uint8 *g_db = NULL;

Uint8 db_retrieve ( void ) {
  gzFile db;
  Uint32 length;

  /* try and open database file */
  db = gzopen ( DBFILE, "rb" );

  if ( ! db ) {
    return ( 0 ); // no file present
  }

  /* how long is the file? */
/*
  gzseek ( db, 0, SEEK_END );
  length = gztell ( db );

  gzseek ( db, 0, SEEK_SET );
*/
length=210*1024;
  /* reserve memory and load file into the buffer for later use */
  g_db = (Uint8*) calloc ( 1, length + 1 );

  if ( ! g_db ) {
    gzclose ( db );
    return ( 0 ); // not enough ram?!
  }

  memset ( g_db, '\0', length + 1 );

  gzread ( db, g_db, length );

  gzclose ( db );

  return ( 1 );
}

Uint8 db_find ( Uint32 crc, Uint16 *r_num, char ***r_gamelist ) {
  char crcbuf [ 10 ];
  char *match;
  char **gamelist;
  Uint16 numgames = 0;
  char *semi, *nl;
  char namebuf [ 100 ];
  int i;

  sprintf ( crcbuf, "%x", crc );

  match = strstr ( (const char*)g_db, crcbuf );

  if ( ! match ) {
    return ( 0 );
  }

  /* we foudn a crc match; just skip ahead to the next colon to find game
   * list. Sure, thats cheesie, but just to get this into testing..
   */
  match = strchr ( match, ':' );

  if ( ! match ) {
    return ( 0 ); // wtf?
  }

  match++; // skip past the colon

  /* build return array */
  gamelist = (char**) calloc ( 42, sizeof(char*) );

  /* how many games in this disk? build games list. */
  semi = strchr ( match, ';' );
  nl = strchr ( match, '\n' );

  if ( semi && semi < nl ) {
    char *end = strchr ( match, '\n' ); /* end of record */

    if ( ! end ) {
      end = strchr ( match, '\0' ); /* end of file?! */
    }

    strncpy ( namebuf, match, semi - match );
    namebuf[semi-match]=0;
    gamelist [ numgames ] = strdup ( namebuf );

    match = semi;
    match ++; // semi
    match ++; // space following semi

    numgames = 1; /* at least */

    while ( match <= end ) {

      semi = strchr ( match, ';' );
      nl = strchr ( match, '\n' );

      if ( semi && semi < nl ) {
	strncpy ( namebuf, match, semi - match );
        namebuf[semi-match]=0;
	for(i=0;i<numgames;i++)
		if (!strcmp(gamelist[i],namebuf))
			break;
	if (i>=numgames)
	{
		gamelist [ numgames ] = (char *)calloc(1,strlen(namebuf)+4);
    		strcpy(gamelist[numgames], namebuf );
		//printf ( "buf %s", namebuf ); do_wait_for_key_true();
		numgames++;
	}
	match = semi;
	match++; // skip past ;
	match++; // skip " " following the semi colon
      } else {
	strncpy ( namebuf, match, nl - match );
        namebuf[nl-match]=0;
	for(i=0;i<numgames;i++)
		if (!strcmp(gamelist[i],namebuf))
			break;
	if (i>=numgames)
	{
		gamelist [ numgames ] = (char *)calloc(1,strlen(namebuf)+4);
    		strcpy(gamelist[numgames], namebuf );
		//printf ( "buf %s", namebuf ); do_wait_for_key_true();
		numgames++;
	}
	match = nl;
	match++; // skip past \n
	break;
      }

    } // while

  } else {
    strncpy ( namebuf, match, nl - match );
    namebuf[nl-match]=0;
    gamelist [ numgames ] = (char *)calloc(1,strlen(namebuf)+4);
    strcpy(gamelist[numgames], namebuf );
    numgames = 1; /* at least */
  }

  /* return values */
  *r_num = numgames;
  *r_gamelist = gamelist;

  /* clear printf() displays */
//  printf("");

  return ( 1 );
}

#if 0
void db_describe ( Uint32 crc, char *r_buf, Uint16 buflen ) {
  Uint16 numgames;
  char **gamelist;
  Uint16 iter,iter2;
  char buf [ 1024 ] = "";

  if ( ! db_find ( crc, &numgames, &gamelist ) ) {
    return; // no match
  }

  for ( iter = 0; iter < numgames; iter++ ) {
	  for( iter2 = 0 ; iter2 < iter ; iter2){
		  if (!strcmp(gamelist[iter],gamelist[iter2]))
			  break;
	  }
	  if (iter2>=iter)
	  	continue;
    strcat ( buf, gamelist [ iter ] );

    if ( iter < ( numgames - 1 ) ) {
      strcat ( buf, ";" );
    }

  }
  if (buf[strlen(buf)-1]==';')
  	buf[strlen(buf)-1]=0;

  db_free ( numgames, gamelist );

  strncpy ( r_buf, buf, buflen );

  return;
}
#endif

static unsigned int _crcTable [256];

void db_crc_init ( void ) {
  Uint32 crc, poly;
  Sint32     i, j;

  poly = 0xEDB88320L;

  for (i=0; i<256; i++) {
    crc = i;
    for (j=8; j>0; j--) {
      if (crc&1) {
	crc = (crc >> 1) ^ poly;
      } else {
	crc >>= 1;
      }
    }
    _crcTable[i] = crc;
  }

  return;
}

/* using CRC32 varient; (odd quotient); see Dan Brown I think's website */
Uint32 db_calculate_id ( char *buf, Uint32 length ) {
  unsigned int x;
  register Uint32 crc = 0 ^ (-1);

  for( x = 0; x < length; x++ ) {
    crc = ( (crc>>8) & 0x00FFFFFF ) ^ _crcTable [ ( crc ^ buf[x] ) & 0xFF ];
  }

#ifdef USE_BIG_ENDIAN
  crc=((crc&0xFF)<<24 )|((crc&0xFF00)<<8 )|((crc&0xFF0000)>>8 )|((crc&0xFF000000)>>24 );
#endif
  return ( crc ^ -1 );
}

#if defined(CALCULE_CRC_FILES) || defined(DEBUG_FILEMANAGER)

Uint32 db_calculate_id_on_file ( char *filename ) {
  FILE *db;
  Uint32 length;
  char *buffer;
  Uint32 crc;

  /* is this a .zip file? If so, we handle it specially..
   */
  if (
	( strstr ( filename, ".ZIP" ) != NULL )  ||
	( strstr ( filename, ".zip" ) != NULL )  ||
	( strstr ( filename, ".Zip" ) != NULL )  ||
	( strstr ( filename, ".zIp" ) != NULL )  ||
	( strstr ( filename, ".ziP" ) != NULL )  ||
	( strstr ( filename, ".ZIp" ) != NULL )  ||
	( strstr ( filename, ".ZiP" ) != NULL )  ||
	( strstr ( filename, ".zIP" ) != NULL )  
     )
    {
    int romsize;
    char *buf = (char*)calloc ( 1024, 1024 );

#ifndef WIN32
    bzero ( buf, 1024 * 1024 );
#else
    memset(buf, 0, 1024 * 1024);
#endif

//    printf ( "Retrieve and check zip ..." );

    romsize = unzipdisk ( (unsigned char*)filename, (unsigned char*)buf );

//    printf ( "Calculating CRC ..." );

    crc = db_calculate_id ( buf, romsize );

    free ( buf );

  } else {

    /* try and open database file */
    db = fopen ( filename, "r" );

    if ( ! db ) {
      return ( 0 ); // no file present
    }

    /* how long is the file? */
    fseek ( db, 0, SEEK_END );
    length = ftell ( db );

    fseek ( db, 0, SEEK_SET );

    /* reserve memory and load file into the buffer for later use */
    buffer = (char *)calloc ( 1, length );

    if ( ! buffer ) {
      fclose ( db );
      return ( 0 ); // not enough ram?!
    }

    fread ( buffer, length, 1, db );

    fclose ( db );

    crc = db_calculate_id ( buffer, length );

    free ( buffer );

  } // .zip?

  return ( crc );
}
#endif

void db_free ( Uint8 numgames, char **gamelist ) {
  Uint8 iter;

  for ( iter = 0; iter < numgames; iter++ ) {
    free ( gamelist [ iter ] );
  }

  free ( gamelist );

  return;
}

static cache_t *g_cache = NULL;

cache_t *cache_head ( void ) {
  return ( g_cache );
}

void cache_free(void)
{
  cache_t *iter= g_cache;
  while ( iter ) {
	cache_t *next=iter->next;
	free(iter);
	iter=next;
  }
  g_cache=NULL;
}

Uint8 cache_retrieve (void) {
  FILE *c=fopen( CACHEFILE, "rb");
  cache_t *newbie, buf;
  Uint32 length, iter;

  if ( ! c ) 
    return 0;

  cache_free();
  fseek ( c, 0, SEEK_END );
  length = ftell ( c );
  fseek ( c, 0, SEEK_SET );

  if ( length == 0 ) {
    fclose(c);
    return 0;
  }

  iter = 0;
  while ( iter < length ) {

    if ( fread ( &buf, sizeof(cache_t), 1, c ) == 0 ) {
      break; // early done
    }

    newbie = (cache_t *)calloc ( 1, sizeof(cache_t) );

    if ( ! newbie ) {
	fclose(c);
	return 0;
    }

    strcpy ( newbie -> c_filename, buf.c_filename );
#ifdef USE_BIG_ENDIAN
     newbie->c_crc32=((buf.c_crc32&0xFF)<<24 )|((buf.c_crc32&0xFF00)<<8 )|((buf.c_crc32&0xFF0000)>>8 )|((buf.c_crc32&0xFF000000)>>24 );
#else
    newbie -> c_crc32 = buf.c_crc32;
#endif
    newbie -> next = g_cache;
    g_cache = newbie;

    iter += sizeof(cache_t);
  }

  fclose ( c );

  return 1;
}

#if 0
Uint8 cache_save ( void ) {
  FILE *c = fopen ( CACHEFILE, "wb" );
  cache_t *iter = g_cache;

  if ( ! c ) {
    return ( 0 );
  }

  while ( iter ) {
    fwrite ( iter, sizeof(cache_t), 1, c );
    iter = iter -> next;
  }

  fclose ( c );

  return ( 1 );
}
#endif

cache_t *cache_find ( char *filename ) {
  cache_t *iter = g_cache;

  while ( iter ) {
    if ( strcmp ( iter -> c_filename, filename ) == 0 ) {
      return ( iter );
    }

    iter = iter -> next;
  }
  return ( NULL );
}

Uint8 cache_append ( char *filename, Uint32 crc ) {
  cache_t *newbie = (cache_t *)calloc ( 1, sizeof(cache_t) );

  if ( ! newbie ) {
    return ( 0 );
  }

/*
  if ( ( strstr ( filename, ".ST" ) == NULL ) &&
       ( strstr ( filename, ".MSA" ) == NULL ) &&
       ( strstr ( filename, ".ZIP" ) == NULL ) )
  {
    return;
  }
*/

  memset ( newbie, '\0', sizeof(cache_t) );

  newbie -> next = g_cache;
  g_cache = newbie;
  strncpy ( newbie -> c_filename, filename,127 );
  newbie->c_filename[127]=0;
  newbie -> c_crc32 = crc;

//  cache_save();

  return ( 1 );
}
