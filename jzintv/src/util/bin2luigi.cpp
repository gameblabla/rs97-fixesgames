/* ======================================================================== */
/*  Takes a BIN (and optional CFG) and generates a .LUIGI from it.          */
/* ======================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
extern "C" 
{
#    include "config.h"
#    include "lzoe/lzoe.h"
#    include "file/file.h"
}
#include "locutus/locutus.hpp"
#include "locutus/bin_to_loc.hpp"
#include "locutus/luigi.hpp"

#include <string>
using namespace std;

t_locutus locutus;

void show_messages(const t_bin_to_loc& bin_to_loc)
{
    const t_string_vec& messages = bin_to_loc.get_messages();
    t_string_vec::const_iterator messages_i = messages.begin();

    int warnings = bin_to_loc.get_warnings();
    int errors   = bin_to_loc.get_errors();

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
    char bin_fn[1024], cfg_fn[1024], rom_fn[1024];
    int fn_len;
    bool ok = true;

    if (argc != 2)
    {
        fprintf(stderr, "usage: bin2luigi foo[.bin]\n");
        exit(1);
    }

    /* -------------------------------------------------------------------- */
    /*  Generate .BIN, .CFG, and .ROM filenames from argument filename.     */
    /*  If the argument lacks a .BIN extension, add one.                    */
    /* -------------------------------------------------------------------- */
    strncpy(bin_fn, argv[1], 1017);
    bin_fn[1017] = 0;

    fn_len = strlen(bin_fn);
    if (stricmp(bin_fn + fn_len - 4, ".bin") != 0 &&
        stricmp(bin_fn + fn_len - 4, ".int") != 0 &&
        stricmp(bin_fn + fn_len - 4, ".itv") != 0 &&
        !file_exists(bin_fn))
    {
        strcpy(bin_fn + fn_len, ".bin");
        fn_len += 4;
        if (!file_exists(bin_fn))
        {
            fprintf(stderr, "Could not find '%s' or '%s'\n", argv[1], bin_fn);
            exit(1);
        }
    }

    strcpy(cfg_fn, bin_fn);
    strcpy(rom_fn, bin_fn);

    strcpy(cfg_fn + fn_len - 4, ".cfg");
    strcpy(rom_fn + fn_len - 4, ".luigi");

    /* -------------------------------------------------------------------- */
    /*  Use t_bin_to_loc to populate Locutus.                               */
    /* -------------------------------------------------------------------- */
    try 
    {
        t_bin_to_loc bin_to_loc( bin_fn, cfg_fn, locutus, true );
    
        if ( !bin_to_loc.is_ok() || !bin_to_loc.process() )
        {
            fprintf(stderr, "Errors encountered during conversion\n");
            ok = false;
        } 

        show_messages( bin_to_loc );

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

            FILE *fo = fopen( rom_fn, "wb" );

            if ( !fo )
            {
                perror("fopen()");
                fprintf(stderr, "Could not open %s for writing\n", rom_fn);
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
