// ======================================================================== //
//  LOCUTUS -- aka. LTO Flash!                                              //
//                                                                          //
//  This class models Locutus, and serves as a structured container for     //
//  a Locutus compatible cartridge.  This may be initialized by or          //
//  serialized to a LUIGI file.                                             //
//                                                                          //
//  Note:  This is not tightly integrated into jzIntv at all.  Rather,      //
//  some adaptor C code will bridge between this C++ and jzIntv.            //
// ======================================================================== //

#include "locutus.hpp"
#include <cstring>
#include <algorithm>
#include <iterator>

using namespace std;


// ------------------------------------------------------------------------ //
//  Constructor                                                             //
// ------------------------------------------------------------------------ //
t_locutus::t_locutus()
{
    memset( (void *)&ram     [0]   , 0, sizeof( ram      ) );
    memset( (void *)&mem_map [0][0], 0, sizeof( mem_map  ) );
    memset( (void *)&pfl_map [0][0], 0, sizeof( pfl_map  ) );

    for (int i = 0; i < 256; i++)
        mem_perm[i][0].reset();

    for (int i = 0; i < 256; i++)
        mem_perm[i][1].reset();

    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
            pfl_perm[i][j].reset();

    initialized.reset();
}


// ------------------------------------------------------------------------ //
//  GET_INITIALIZED_SPAN_LIST  Get the spans of initialized memory          //
// ------------------------------------------------------------------------ //
t_addr_list t_locutus::get_initialized_span_list( void ) const
{
    uint32_t    s_addr  = 0, addr;
    bool        in_span = false;
    t_addr_list init_spans;

    for (addr = 0; addr < 512 * 1024; addr++)
    {
        if ( !in_span && initialized[addr] )
        {
            s_addr  = addr;
            in_span = true;

        } else if ( in_span && !initialized[addr] )
        {
            init_spans.push_back( t_addr_span( s_addr, addr - 1 ) );
            in_span = false;
        }
    }

    if (in_span)
        init_spans.push_back( t_addr_span( s_addr, addr - 1 ) );

    return init_spans;
}


// ======================================================================== //
//  Intellivision Interface!                                                //
// ======================================================================== //


// ------------------------------------------------------------------------ //
//  INTV_READ                                                               //
// ------------------------------------------------------------------------ //
uint16_t t_locutus::intv_read( const uint16_t intv_addr ) const
{
    if ( ( intv_addr <= 0x04FF                        ) ||
         ( intv_addr >= 0x1000 && intv_addr <= 0x1FFF ) ||
         ( intv_addr >= 0x3000 && intv_addr <= 0x47FF ) )
        return 0xFFFF;

    const uint8_t  para = intv_addr >> 8;
    const t_perm   perm = get_mem_perm( para, false );

    if ( perm[LOCUTUS_PERM_READ] )
    {
        const uint32_t map       = get_mem_map( para, false );
        const uint32_t locu_addr = ( map << 8 ) | ( intv_addr & 0xFF );

        return read( locu_addr );
    } else
    {
        return 0xFFFF;
    }
}

// ------------------------------------------------------------------------ //
//  DO_BANKSW_WRITE                                                         //
// ------------------------------------------------------------------------ //
void    t_locutus::do_banksw_write( const uint16_t addr, 
                                    const uint16_t data )
{
    uint16_t hchap_addr = ((addr & 0xF) << 12) | ((addr & 0x10) << 7);
    uint8_t  para       = hchap_addr >> 8;

    if ( !get_mem_perm( para, false )[LOCUTUS_PERM_BANKSW] )
        return;

    // Intellicart bank switching:
    //
    //  Each register accepts an 8-bit value.  (The upper 8 bits of the 
    //  value written are ignored.)  The lower 8 bits of the value written 
    //  specify the upper 8 bits of the target address for that 2K range.  
    //  The target address is combined with the CPU address in this manner:
    //
    //      target_addr = bank_select << 8;
    //      icart_addr  = (intv_addr & 0x07FF) + target_addr;

    for (int i = 0; i < 8; i++)
    {
        set_mem_map( para + i, false, (data + i) & 0xFF );
    }

    if (cpu_cache)
    {
        uint16_t cpu_addr = para << 8;
        cpu_cache->invalidate( cpu_addr, cpu_addr + 2047 );
    }
}

// ------------------------------------------------------------------------ //
//  DO_PAGEFLIP_WRITE                                                       //
// ------------------------------------------------------------------------ //
void    t_locutus::do_pageflip_write( const uint16_t intv_addr, 
                                      const uint16_t data )
{
    uint8_t chap = intv_addr >> 12;
    uint8_t page = data & 0xF;

    if ( chap == 0 || chap == 1 || chap == 3 )
        return;
    
    if ( !get_pageflip_perm( chap, 0 )[LOCUTUS_PERM_BANKSW] )
        return;

    t_perm perm   = get_pageflip_perm( chap, page );
    uint16_t map  = get_pageflip_map ( chap, page ) << 4; 
    uint8_t  para = chap << 4;

    perm[LOCUTUS_PERM_BANKSW] = false;

    if ( chap != 4 )
    {
        for (int i = 0; i < 16; i++)
        {
            set_mem_map ( para + i, false, map + i );
            set_mem_perm( para + i, false, perm    );
        }

        if (cpu_cache)
            cpu_cache->invalidate( para << 8, (para << 8) + 4095 );

    } else
    {
        // on $4xxx, only flip $4800 - $4FFF
        for (int i = 8; i < 16; i++)
        {
            set_mem_map ( para + i, false, map + i );
            set_mem_perm( para + i, false, perm    );
        }

        if (cpu_cache)
            cpu_cache->invalidate( 0x4800, 0x4FFF );
    }
}
    

// ------------------------------------------------------------------------ //
//  INTV_WRITE                                                              //
// ------------------------------------------------------------------------ //
void    t_locutus::intv_write( const uint16_t intv_addr,
                               const uint16_t data )
{
    if ( intv_addr >= 0x40 && intv_addr <= 0x5F)
    {
        do_banksw_write( intv_addr, data );
        return;
    }

    if ( ( intv_addr & 0x0FFF) == 0xFFF &&
         ((intv_addr & 0xF000) | 0x0A50 ) == ( data & 0xFFF0 ) )
    {
        do_pageflip_write( intv_addr, data );
        return;
    }

    bool reset_copy = intv_addr & 0x100;

    //  Locutus native memory mapping update
    if ( intv_addr >= 0x1000 && intv_addr <= 0x11FF )
    {
        uint8_t para = intv_addr & 0xFF;
        set_mem_map( para, reset_copy, data );
        return;
    }

    //  Locutus native memory permission update
    if ( intv_addr >= 0x1200 && intv_addr <= 0x13FF )
    {
        uint8_t para = intv_addr & 0xFF;
        t_perm  perm;

        perm[0] = data & (1 << 0);
        perm[1] = data & (1 << 1);
        perm[2] = data & (1 << 2);
        perm[3] = data & (1 << 3);

        set_mem_perm( para, reset_copy, perm );
        return;
    }

    //  Normal data write!  Who'd have thunk it?
    const uint8_t  para = intv_addr >> 8;
    const t_perm   perm = get_mem_perm( para, false );

    if ( perm[LOCUTUS_PERM_WRITE] )
    {
        const uint32_t map       = get_mem_map( para, false );
        const uint32_t locu_addr = ( map << 8 ) | (intv_addr & 0xFF);

        write( locu_addr, data );
    }
}

// ------------------------------------------------------------------------ //
//  INTV_RESET                                                              //
// ------------------------------------------------------------------------ //
void    t_locutus::intv_reset( void )
{
    for (int para = 0; para < 256; para++)
        set_mem_map ( para, false, get_mem_map( para, true ) );

    for (int para = 0; para < 256; para++)
        set_mem_perm( para, false, get_mem_perm( para, true ) );
}


