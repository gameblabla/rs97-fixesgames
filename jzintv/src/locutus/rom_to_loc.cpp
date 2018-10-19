// ======================================================================== //
//  Utilities:                                                              //
//                                                                          //
//      class t_rom_to_loc_impl                                             //
//                                                                          //
// ======================================================================== //

extern "C" 
{
#   include "config.h"
#   include "icart/icartrom.h"
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
#include "rom_to_loc.hpp"

// ======================================================================== //
//  CLASS T_ROM_TO_LOC_IMPL                                                 //
// ======================================================================== //
class t_rom_to_loc_impl
{
  private:
    icartrom_t* const   icartrom;
    t_locutus&          locutus;

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

    inline bool ic_get_bit( const uint32_t* bv, const int bit ) const
    {
        return ( bv[ bit >> 5 ] >> (bit & 0x1F) ) & 1;
    }

    inline bool ic_is_preload( const int para ) const
    {
        return ic_get_bit( icartrom->preload, para );
    }

    inline bool ic_is_readable( const int para ) const
    {
        return ic_get_bit( icartrom->readable, para );
    }

    inline bool ic_is_narrow( const int para ) const
    {
        return ic_get_bit( icartrom->narrow, para );
    }

    inline bool ic_is_writable( const int para ) const
    {
        return ic_get_bit( icartrom->writable, para );
    }

    inline bool ic_is_banksw( const int para ) const
    {
        return ic_get_bit( icartrom->dobanksw, para );
    }

  public:
    t_rom_to_loc_impl
    (
        icartrom_t*         icartrom_,
        t_locutus&          locutus_
    ) 
    :   icartrom( icartrom_ ),
        locutus ( locutus_  ),
        warnings( 0         ), 
        errors  ( 0         )
    { 
    }

    ~t_rom_to_loc_impl() 
    {
    }

    bool                process();
    bool                is_ok()             const   { return !errors;       }
    int                 get_errors()        const   { return errors;        }
    int                 get_warnings()      const   { return warnings;      }
    const t_string_vec& get_messages()      const   { return messages;      }
};


// ------------------------------------------------------------------------ //
//  T_ROM_TO_LOC_IMPL::PROCESS                                              //
// ------------------------------------------------------------------------ //
bool t_rom_to_loc_impl::process()
{
    for ( int para = 0; para < 256; para++ )
    {
        const bool is_preload  = ic_is_preload ( para );
        const bool is_readable = ic_is_readable( para );
        const bool is_writable = ic_is_writable( para );
        const bool is_narrow   = ic_is_narrow  ( para );
        const bool is_banksw   = ic_is_banksw  ( para );
        const int  base_addr   = para << 8;


        t_perm perm;

        perm[ LOCUTUS_PERM_READ   ] = is_readable;
        perm[ LOCUTUS_PERM_WRITE  ] = is_writable;
        perm[ LOCUTUS_PERM_NARROW ] = is_narrow;
        perm[ LOCUTUS_PERM_BANKSW ] = is_banksw;

        locutus.set_mem_perm( para, false, perm );
        locutus.set_mem_perm( para, true,  perm );

        if ( is_readable || is_writable )
        {
            locutus.set_mem_map( para, false, para );
            locutus.set_mem_map( para, true,  para );
        }

        if ( is_preload )
        {
            for ( int ofs = 0; ofs < 256; ofs++ )
            {
                const int addr = base_addr + ofs;
                locutus.write( addr, icartrom->image[ addr ] );
            }
        }
    }

    return errors == 0;
}

// ------------------------------------------------------------------------ //
//  Forwarding from T_ROM_TO_LOC to T_ROM_TO_LOC_IMPL                       //
// ------------------------------------------------------------------------ //

t_rom_to_loc::t_rom_to_loc
(
    icartrom_t*         icartrom,
    t_locutus&          locutus
) 
{
    impl = new t_rom_to_loc_impl( icartrom, locutus );
}

t_rom_to_loc::~t_rom_to_loc()
{
    delete impl;
}

bool t_rom_to_loc::is_ok()         const    { return impl->is_ok();         }
bool t_rom_to_loc::process()                { return impl->process();       }
int  t_rom_to_loc::get_errors()    const    { return impl->get_errors();    }
int  t_rom_to_loc::get_warnings()  const    { return impl->get_warnings();  }

const t_string_vec& t_rom_to_loc::get_messages() const
{
    return impl->get_messages();
}


