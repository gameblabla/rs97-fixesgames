// ======================================================================== //
//  Utilities:                                                              //
//                                                                          //
//      class t_loc_to_bin_impl                                             //
//      class t_bin_to_loc_impl                                             //
//                                                                          //
//  You figure it out.  ;-)                                                 //
// ======================================================================== //

extern "C" 
{
#   include "config.h"
#   include "lzoe/lzoe.h"
#   include "bincfg/bincfg.h"
#   include <errno.h>
}

#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>
#include <list>
#include <map>
#include <algorithm>

using namespace std;

#include "locutus_types.hpp"
#include "locutus.hpp"
#include "bin_to_loc.hpp"

// ======================================================================== //
//  CLASS T_BIN_TO_LOC_IMPL                                                 //
// ======================================================================== //
class t_bin_to_loc_impl
{
  private:
    const char* const   bin_fn;
    const char* const   cfg_fn;
    t_locutus&          locutus;

    LZFILE* const       f_cfg;
    bc_cfgfile_t* const bincfg;

    int                 warnings;
    int                 errors;

    t_string_vec        messages;

    void record_error( string error ) 
    {
        messages.push_back( string("ERROR: ") + error );
        ++errors;
    }

    void record_warning( string warning ) 
    {
        messages.push_back( string("WARNING: ") + warning );
        ++warnings;
    }

    void add_segment( const uint16_t intv_s_addr, const uint32_t locu_s_addr,
                      const uint16_t span_len,    const t_perm   perm,
                      const uint16_t *data,       const int      page );

  public:
    t_bin_to_loc_impl
    (
        const char* const   bin_fn_,
        const char* const   cfg_fn_,
        t_locutus&          locutus_,
        const bool          do_macros = true
    ) 
    :   bin_fn  ( bin_fn_  ), 
        cfg_fn  ( cfg_fn_  ), 
        locutus ( locutus_ ),
        f_cfg   ( cfg_fn_ ? lzoe_fopen( cfg_fn_, "r" ) : 0 ),
        bincfg  ( bc_parse_cfg( f_cfg, bin_fn_, f_cfg ? cfg_fn_ : 0 ) ),
        warnings( 0 ), 
        errors  ( 0 )
    { 
        // ---------------------------------------------------------------- //
        //  If we opened a config file, close it.                           //
        // ---------------------------------------------------------------- //
        if ( f_cfg )
            lzoe_fclose( f_cfg );

#ifndef BC_NODOMACRO
        // ---------------------------------------------------------------- //
        //  Apply any statically safe macros.                               //
        // ---------------------------------------------------------------- //
        if ( do_macros )
            bc_do_macros( bincfg, 0 );
#endif

        // ---------------------------------------------------------------- //
        //  Populate the config with corresponding BIN data.                //
        // ---------------------------------------------------------------- //
        if ( bc_read_data( bincfg ) )
            record_error( string("Error reading data for CFG file.") );

    }

    ~t_bin_to_loc_impl() 
    {
#ifndef BC_NOFREE
        // ---------------------------------------------------------------- //
        //  Discard the parsed config                                       //
        // ---------------------------------------------------------------- //
        bc_free_cfg( bincfg );
#endif
    }

    bool                process();
    bool                is_ok()             const   { return !errors;       }
    int                 get_errors()        const   { return errors;        }
    int                 get_warnings()      const   { return warnings;      }
    const t_string_vec& get_messages()      const   { return messages;      }
};


// ------------------------------------------------------------------------ //
//  T_BIN_TO_LOC_IMPL::PROCESS                                              //
//                                                                          //
//  This is mostly ganked from icart/icartbin.c, but upgraded to support    //
//  paged ROM segments.                                                     //
// ------------------------------------------------------------------------ //
bool t_bin_to_loc_impl::process()
{
    bc_memspan_t* span;
    t_hunk_vec paged_hunks;

    // -------------------------------------------------------------------- //
    //  Traverse the memspan list, calling "add_segment" on each non-paged  //
    //  segment, and queuing up the paged segments for a second pass.       //
    // -------------------------------------------------------------------- //
    for ( span = bincfg->span ; span ; span = (bc_memspan_t *)span->l.next )
    {
        const int span_len = span->e_addr - span->s_addr + 1;

        // ---------------------------------------------------------------- //
        //  If this span has an ECS page associated with it, queue it.  We  //
        //  will handle this in a second pass so that we can assign the     //
        //  preload addresses in a canonical order.                         //
        // ---------------------------------------------------------------- //
        if ( (span->flags & BC_SPAN_EP) != 0 )
        {
            // ------------------------------------------------------------ //
            //  We currently do not support writeable page-flipped spans.   //
            //  Gotta fix that, but requires changes to .CFG format.        //
            // ------------------------------------------------------------ //
            if ( ( span->flags & BC_SPAN_W ) != 0 )
            {
                char buf[120];

                sprintf( buf, 
                        "Skipping writeable paged segment at "
                        "$%.4X - $%.4X PAGE %.1X", 
                        span->s_addr, span->e_addr, span->epage );

                record_warning( string(buf) );
                continue;
            }

            // ------------------------------------------------------------ //
            //  Pageflipped, but no valid page?                             //
            //  That's a paddlin'                                           //
            // ------------------------------------------------------------ //
            if ( span->epage == BC_SPAN_NOPAGE )
            {
                char buf[120];

                sprintf( buf, 
                        "Invalid segment is marked 'paged' but without a "
                        "valid page number at $%.4X - $%.4X", 
                        span->s_addr, span->e_addr );

                record_error( string(buf) );
                continue;
            }

            // ------------------------------------------------------------ //
            //  Pageflipped, but page number greater than 15?               //
            //  That's a paddlin'                                           //
            // ------------------------------------------------------------ //
            if ( span->epage > 15 )
            {
                char buf[120];

                sprintf( buf, 
                        "Invalid page number %.2X for page at $%.4X - $%.4X", 
                        span->epage, span->s_addr, span->e_addr );

                record_error( string(buf) );
                continue;
            }

            // ------------------------------------------------------------ //
            //  Bankswitched and pageflipped at the same time?              //
            //  That's a paddlin'.                                          //
            // ------------------------------------------------------------ //
            if ( ( span->flags & BC_SPAN_B ) != 0 )
            {
                char buf[120];

                sprintf( buf, 
                        "Invalid segment is both bankswitched "
                        "and paged at $%.4X - $%.4X PAGE %.1X", 
                        span->s_addr, span->e_addr, span->epage );

                record_error( string(buf) );
                continue;
            }

            // ------------------------------------------------------------ //
            //  Bankswitched but not readable?                              //
            //  That's a paddlin'.                                          //
            // ------------------------------------------------------------ //
            if ( ( span->flags & BC_SPAN_R ) == 0 )
            {
                char buf[120];

                sprintf( buf, 
                        "Invalid segment is paged but not readable "
                        "at $%.4X - $%.4X PAGE %.1X", 
                        span->s_addr, span->e_addr, span->epage );

                record_error( string(buf) );
                continue;
            }

            // ------------------------------------------------------------ //
            //  No preload data?  Oh you better believe that's a paddlin'!  //
            // ------------------------------------------------------------ //
            if ( ( span->flags & BC_SPAN_PL ) == 0 )
            {
                char buf[120];

                sprintf( buf, 
                        "Invalid segment is paged but has no preload "
                        "flag at $%.4X - $%.4X PAGE %.1X", 
                        span->s_addr, span->e_addr, span->epage );
                
                record_error( string(buf) );
                continue;
            }

            if ( !span->data )
            {
                char buf[120];

                sprintf( buf, 
                        "Invalid segment is paged but has no preload "
                        "data at $%.4X - $%.4X PAGE %.1X", 
                        span->s_addr, span->e_addr, span->epage );

                record_error( string(buf) );
                continue;
            }


            paged_hunks.push_back( 
                t_bin_hunk( span->s_addr, span->e_addr, span->epage, true,
                            span->data ) );

            continue;
        }

        // ---------------------------------------------------------------- //
        //  An actual non-paged chunk.  Imagine that.                       //
        // ---------------------------------------------------------------- //
        t_perm perm;

        if ( span->flags & BC_SPAN_R ) perm.set( LOCUTUS_PERM_READ   );
        if ( span->flags & BC_SPAN_W ) perm.set( LOCUTUS_PERM_WRITE  );
        if ( span->flags & BC_SPAN_N ) perm.set( LOCUTUS_PERM_NARROW );
        if ( span->flags & BC_SPAN_B ) perm.set( LOCUTUS_PERM_BANKSW );

        bool has_data = ( span->flags & ( BC_SPAN_PL | BC_SPAN_PK ) ) != 0;

        if ( has_data && !span->data )
        {
            char buf[120];

            sprintf( buf, 
                    "Skipping segment marked preload, but missing data at "
                    "$%.4X - $%.4X", 
                    span->s_addr, span->e_addr );

            record_error( string( buf ) );
            continue;
        }

        add_segment
        ( 
            span->s_addr,  // address in Intellivision memory map
            span->s_addr,  // address in Locutus memory map
            span_len,      // length of span in words
            perm,          // span permissions
            span->data,    // data associated with span, if any
            PAGE_NONE      // not a paged ROM
        );
    }

    // -------------------------------------------------------------------- //
    //  If there are any paged chunks, sort them into canonical order and   //
    //  allocate them at the end of Locutus's memory space.  For now, just  //
    //  grow from the 512K boundary downward.                               //
    // -------------------------------------------------------------------- //
    if ( paged_hunks.size() > 0 )
    {
        uint32_t locu_addr = 0x80000;

        sort( paged_hunks.begin(), paged_hunks.end() );

        t_hunk_vec::const_iterator paged_span_i = paged_hunks.begin();
        t_perm perm( 0x9 );     // readable, page-flipped only.

        while ( paged_span_i != paged_hunks.end() )
        {
            int span_len = paged_span_i->e_addr - paged_span_i->s_addr + 1;

            locu_addr -= 0x1000;

            add_segment
            (
                paged_span_i->s_addr,       // address in INTV memory map
                locu_addr,                  // address in Locutus memory map
                span_len,                   // length of paged_span_i in words
                perm,                       // paged_span_i permissions
                &(paged_span_i->data[0]),   // data assoc with paged_span_i
                paged_span_i->page          // not a paged ROM
            );

            ++paged_span_i;
        }
    }

    return errors == 0;
}

// ------------------------------------------------------------------------ //
//  T_BIN_TO_LOC_IMPL::ADD_SEGMENT                                          //
// ------------------------------------------------------------------------ //
void t_bin_to_loc_impl::add_segment
( 
    const uint16_t intv_s_addr, 
    const uint32_t locu_s_addr,
    const uint16_t span_len,    
    const t_perm   perm,
    const uint16_t *data,       
    const int      page 
)
{
    // -------------------------------------------------------------------- //
    //  Check for overflow.  This should never trigger, but just in case..  //
    // -------------------------------------------------------------------- //
    if ( uint32_t(intv_s_addr) + uint32_t(span_len) > 0x10000 )
    {
        char buf[120];

        sprintf(buf, "Address overflow on span %.4X len %.4X",
                intv_s_addr, span_len);

        record_error( string(buf) );
        return;
    }

    // -------------------------------------------------------------------- //
    //  If there was a data payload, copy it into Locutus now.              //
    // -------------------------------------------------------------------- //
    if ( data )
        for ( int i = 0; i < span_len; i++ )
        {
            const uint32_t locu_addr = locu_s_addr + i;
            locutus.write( locu_addr, data[i] );
        }

    // -------------------------------------------------------------------- //
    //  If this was not paged, or if it was page 0 of a chapter, put it in  //
    //  the default after-reset memory map.                                 //
    // -------------------------------------------------------------------- //
    if ( page == 0 || page == PAGE_NONE )
    {
        const int paras    = (span_len + 255) >> 8;
        const int para     = intv_s_addr >> 8;
        const int locu_map = locu_s_addr >> 8;
        t_perm    p_perm   = perm;

        if ( page == 0 )
            p_perm[ LOCUTUS_PERM_BANKSW ] = false;
        
        for ( int i = 0; i < paras; ++i)
        {
            locutus.set_mem_perm( para + i, true, p_perm       );
            locutus.set_mem_map ( para + i, true, locu_map + i );
        }
    }

    // -------------------------------------------------------------------- //
    //  If this was paged, also put it in the page flip map.                //
    // -------------------------------------------------------------------- //
    if ( page != PAGE_NONE )
    {
        const int chaps    = (span_len + 4095) >> 12;
        const int chap     = intv_s_addr >> 12;
        const int locu_map = locu_s_addr >> 12;

        for ( int i = 0; i < chaps; ++i )
            locutus.set_pageflip( chap + i, page, locu_map + i, perm );
    }
}

// ------------------------------------------------------------------------ //
//  Forwarding from T_BIN_TO_LOC to T_BIN_TO_LOC_IMPL                       //
// ------------------------------------------------------------------------ //

t_bin_to_loc::t_bin_to_loc
(
    const char* const   bin_fn,
    const char* const   cfg_fn,
    t_locutus&          locutus,
    const bool          do_macros
) 
{
    impl = new t_bin_to_loc_impl( bin_fn, cfg_fn, locutus, do_macros );
}

t_bin_to_loc::~t_bin_to_loc()
{
    delete impl;
}

bool t_bin_to_loc::is_ok()         const    { return impl->is_ok();         }
bool t_bin_to_loc::process()                { return impl->process();       }
int  t_bin_to_loc::get_errors()    const    { return impl->get_errors();    }
int  t_bin_to_loc::get_warnings()  const    { return impl->get_warnings();  }

const t_string_vec& t_bin_to_loc::get_messages() const
{
    return impl->get_messages();
}

