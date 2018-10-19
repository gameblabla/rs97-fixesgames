// ======================================================================== //
//  LOCUTUS -- aka. LTO Flash!                                              //
//                                                                          //
//  This class models Locutus, and serves as a structured container for     //
//  a Locutus compatible cartridge.  This may be initialized by or          //
//  serialized to a LUIGI file.                                             //
//                                                                          //
//  Note:  This is not tightly integrated into jzIntv at all.  Rather,      //
//  some adaptor C code will bridge between this C++ and jzIntv.            //
//                                                                          //
//  Terms:                                                                  //
//   -- byte:       8 bits                                                  //
//   -- word:       16 bits                                                 //
//   -- para:       256 words (para is short for "paragraph")               //
//   -- page:       4K words (16 para)                                      //
//   -- chap:       A group of pages that map to the same 4K address span   //
//                                                                          //
// ======================================================================== //
#ifndef LOCUTUS_HPP_
#define LOCUTUS_HPP_

#include "locutus_types.hpp"
#include <string>

static const int LOCUTUS_FEAT_JLP_ACCEL     = 0;
static const int LOCUTUS_FEAT_JLP_SAVEGAME  = 1;

class t_cpu_cache_if                // interface back to the CPU decode cache
{
    public:
        virtual void invalidate( uint16_t addr_lo, uint16_t addr_hi ) = 0;
};    

static const uint32_t LOCUTUS_CAPACITY = 512 * 1024;

class t_locutus
{
  private:
    uint16_t        ram     [512 * 1024];
    uint16_t        mem_map [256][2];   // bits 7:0 get added to 11:8 of addr
    t_perm          mem_perm[256][2];
    uint16_t        pfl_map [16][16];   // bits 11:0 form bits 19:12 of addr
    t_perm          pfl_perm[16][16];

    t_cpu_cache_if* cpu_cache;

    std::bitset<64>               feature_flag;
    std::bitset<LOCUTUS_CAPACITY> initialized;

    void do_banksw_write  ( uint16_t addr, uint16_t data );
    void do_pageflip_write( uint16_t addr, uint16_t data );

    void validate_addr( const char* loc, const uint32_t addr ) const
    {
        if ( addr > LOCUTUS_CAPACITY )
            throw loc + std::string(": Address out of range");
    }

  public:

    t_locutus();

    // -------------------------------------------------------------------- //
    //  INTV-facing interface                                               //
    // -------------------------------------------------------------------- //
    uint16_t intv_read ( const uint16_t addr ) const;                    
    void     intv_write( const uint16_t addr, const uint16_t data );     
    void     intv_reset( void );                                         

    inline void set_cpu_cache ( t_cpu_cache_if* cpu_cache_ )
    {
        cpu_cache = cpu_cache_;
    }
                                                                         
    // -------------------------------------------------------------------- //
    //  General purpose interface                                           //
    //                                                                      //
    //  For mem_map, mem_perm, the "reset" argument selects between the     //
    //  currently active values (reset = false) or the values that will be  //
    //  loaded at reset (reset = true).                                     //
    // -------------------------------------------------------------------- //
    inline uint16_t read( const uint32_t addr ) const
    {
        validate_addr( "t_locutus::read", addr );
        return ram[addr];
    }

    inline void     write( const uint32_t addr, const uint16_t data )
    {
        validate_addr( "t_locutus::write", addr );
        ram[addr] = data;
        initialized[addr] = true;
    }
       
    inline uint16_t get_mem_map ( const uint8_t para, const bool reset ) const
    {
        return mem_map[para][reset];
    }

    inline void     set_mem_map ( const uint8_t  para, const bool reset, 
                                  const uint16_t mapping )
    {
        mem_map[para][reset] = mapping;
    }

    inline t_perm   get_mem_perm( const uint8_t  para, const bool reset ) const
    {
        return mem_perm[para][reset];
    }

    inline void     set_mem_perm( const uint8_t  para, const bool reset,
                                  const t_perm   perm )
    {
        mem_perm[para][reset] = perm;
    }

    inline uint16_t get_pageflip_map ( const uint8_t chap, 
                                       const uint8_t page ) const
    {
        return pfl_map[chap][page];
    }

    inline t_perm   get_pageflip_perm( const uint8_t  chap, 
                                       const uint8_t  page ) const
    {
        return pfl_perm[chap][page];
    }

    inline void     set_pageflip_map ( const uint8_t  chap, 
                                       const uint8_t  page,
                                       const uint16_t map  )
    {
        pfl_map[chap][page] = map;
    }

    inline void     set_pageflip_perm( const uint8_t  chap, 
                                       const uint8_t  page,
                                       const t_perm   perm )
    {
        pfl_perm[chap][page] = perm;
    }

    inline void     set_pageflip     ( const uint8_t  chap, 
                                       const uint8_t  page,
                                       const uint16_t map,
                                       const t_perm   perm )
    {
        set_pageflip_map ( chap, page, map  );
        set_pageflip_perm( chap, page, perm );
    }

    inline bool     get_initialized  ( const uint32_t addr ) const
    {
        validate_addr( "t_locutus::get_initialized", addr );
        return initialized[ addr ];
    }

    inline void     set_initialized  ( const uint32_t addr, const bool ini )
    {
        validate_addr( "t_locutus::set_initialized", addr );
        initialized[ addr ] = ini;
    }


    bool get_feature_flag( const int feat ) const 
    {
        return feature_flag[feat];
    }

    void set_feature_flag( const int feat, const bool value ) 
    {
        feature_flag[feat] = value;
    }

    t_addr_list get_initialized_span_list( void ) const;
};


#endif
