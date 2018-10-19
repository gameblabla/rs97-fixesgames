/* ======================================================================== */
/*  Takes a .ROM and generates a .LUIGI from it.                            */
/* ======================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
extern "C" 
{
#   include "config.h"
#   include "lzoe/lzoe.h"
#   include "file/file.h"
#   include "icart/icartrom.h"
}
#include "locutus/locutus.hpp"
#include "locutus/rom_to_loc.hpp"
#include "locutus/luigi.hpp"

#include <string>
using namespace std;

t_locutus   locutus;
icartrom_t  icart;

void show_messages(const t_rom_to_loc& rom_to_loc)
{
    const t_string_vec& messages = rom_to_loc.get_messages();
    t_string_vec::const_iterator messages_i = messages.begin();

    int warnings = rom_to_loc.get_warnings();
    int errors   = rom_to_loc.get_errors();

    if ( warnings ) printf("%3d warnings\n", warnings);
    if ( errors   ) printf("%3d errors\n",   errors  );
    
    while ( messages_i != messages.end() )
    {
        puts( messages_i->c_str() );
        ++messages_i;
    }
}

/* ======================================================================== */
/*  MAIN                                                                    */
/*  This is the main program.  The action happens here.                     */
/* ======================================================================== */
int main(int argc, char *argv[])
{
    char rom_fn[1024], loc_fn[1024];
    int fn_len;
    bool ok = true;

    if (argc != 2)
    {
        fprintf(stderr, "usage: rom2luigi foo[.rom]\n");
        exit(1);
    }

    /* -------------------------------------------------------------------- */
    /*  Generate .BIN, .CFG, and .ROM filenames from argument filename.     */
    /*  If the argument lacks a .BIN extension, add one.                    */
    /* -------------------------------------------------------------------- */
    strncpy(rom_fn, argv[1], 1017);
    rom_fn[1017] = 0;

    fn_len = strlen(rom_fn);
    if (stricmp(rom_fn + fn_len - 4, ".rom") != 0 &&
        stricmp(rom_fn + fn_len - 4, ".cc3") != 0 &&
        !file_exists(rom_fn))
    {
        strcpy(rom_fn + fn_len, ".rom");
        fn_len += 4;

        if (file_exists(rom_fn))
            goto ok;

        strcpy(rom_fn + fn_len - 4, ".cc3");
        if (file_exists(rom_fn))
            goto ok;

        rom_fn[fn_len - 4] = 0;

        fprintf(stderr, "Could not find '%s', '%s.rom' or '%s.cc3'\n", 
                argv[1], rom_fn, rom_fn);
        exit(1);
    }
ok:

    strcpy(loc_fn, rom_fn);
    strcpy(loc_fn + fn_len - 4, ".luigi");

    /* -------------------------------------------------------------------- */
    /*  Use t_rom_to_loc to populate Locutus.                               */
    /* -------------------------------------------------------------------- */
    try 
    {
        icartrom_readfile( rom_fn, &icart );

        t_rom_to_loc rom_to_loc( &icart, locutus );
    
        if ( !rom_to_loc.is_ok() || !rom_to_loc.process() )
        {
            fprintf(stderr, "Errors encountered during conversion\n");
            ok = false;
        } 

        show_messages( rom_to_loc );
    } catch ( string& s )
    {
        fprintf(stderr, "caught exception: %s\n", s.c_str() );
        exit(1);
    }

    /* -------------------------------------------------------------------- */
    /*  If all went well, serialize and write the file.                     */
    /* -------------------------------------------------------------------- */
    if ( ok )
    {
        try 
        {
            t_byte_vec data = t_luigi::serialize( locutus );

            FILE *fo = fopen( loc_fn, "wb" );

            if ( !fo )
            {
                perror("fopen()");
                fprintf(stderr, "Could not open %s for writing\n", loc_fn);
                exit(1);
            }

            long long written = fwrite( (void *)&data[0], 1, data.size(), fo );

            if ( written != (long long) data.size() )
            {
                perror("fwrite()");
                fprintf(stderr, "Error writing output (short write?)\n");
                exit(1);
            }
            fclose( fo );

        } catch ( string& s )
        {
            fprintf(stderr, "caught exception: %s\n", s.c_str() );
            exit(1);
        }
    }

    return ok ? 0 : 1;
}
