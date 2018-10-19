#ifndef LOCUTUS_TYPES_HPP_
#define LOCUTUS_TYPES_HPP_

#include <list>
#include <vector>
#include <map>   
#include <utility>
#include <bitset>
#include <stdint.h>
#include <cstring>
#include <assert.h>

static const int LOCUTUS_PERM_READ   = 0;
static const int LOCUTUS_PERM_WRITE  = 1;   
static const int LOCUTUS_PERM_NARROW = 2;   // allow writes to 8 LSBs only
static const int LOCUTUS_PERM_BANKSW = 3;   // Bankswitch / Pageflip enable

typedef std::bitset< 4 >                        t_perm;
typedef std::pair< uint32_t, uint32_t >         t_addr_span;
typedef std::list< t_addr_span >                t_addr_list;
typedef std::vector< uint8_t >                  t_byte_vec;
typedef std::vector< uint16_t >                 t_word_vec;
typedef std::vector< std::string >              t_string_vec;
typedef std::map< std::string, std::string >    t_var_map;

static const int PAGE_NONE = 0xFF;

// ------------------------------------------------------------------------ //
//  T_BIN_HUNK                                                              //
// ------------------------------------------------------------------------ //
struct t_bin_hunk
{
    uint32_t    s_addr, e_addr;     // Locutus addresses 
    uint8_t     page;               // ECS page (0xFF if none)
    bool        mapped;             // True == map to INTV space at s_addr
    t_word_vec  data;               // actual ROM data 

    t_bin_hunk( const uint32_t      s_addr_, 
                const uint32_t      e_addr_,
                const int8_t        page_,
                const bool          mapped_,
                const t_word_vec    data_ )
    :
        s_addr( s_addr_ ), e_addr( e_addr_ ), page( page_ ), mapped( mapped_ ),
        data( data_ )
    { }

    t_bin_hunk( const uint32_t      s_addr_, 
                const uint32_t      e_addr_,
                const int8_t        page_,
                const bool          mapped_,
                const uint16_t*     data_ )
    :
        s_addr( s_addr_ ), e_addr( e_addr_ ), page( page_ ), mapped( mapped_ )
    { 
        data.resize( e_addr_ - s_addr_ + 1 );

        std::memcpy( (void*)&data[0], (const void *)&data_[0], 
                     sizeof( uint16_t ) * data.size() );
    }


    inline bool operator < ( const t_bin_hunk& rhs ) const 
    {
        return page <  rhs.page                         ? true
            :  page == rhs.page && s_addr < rhs.s_addr  ? true 
            :                                             false;
    }

    inline bool can_merge_with( const t_bin_hunk& rhs ) const
    {
        return e_addr == rhs.s_addr - 1 
            && page   == rhs.page 
            && mapped == rhs.mapped;
    }

    inline void merge_with( const t_bin_hunk& rhs )
    {
        assert( can_merge_with( rhs ) );

        e_addr = rhs.e_addr;
        data.insert( data.end(), rhs.data.begin(), rhs.data.end() );
    }
};

typedef std::vector< t_bin_hunk > t_hunk_vec;

// ------------------------------------------------------------------------ //
//  T_MEMATTR_SPAN                                                          //
// ------------------------------------------------------------------------ //
static const uint8_t MEMATTR_BAD = 0;
static const uint8_t MEMATTR_ROM = 1;
static const uint8_t MEMATTR_WOM = 2;
static const uint8_t MEMATTR_RAM = 3;

struct t_memattr_span
{
    uint16_t        s_addr, e_addr;
    uint8_t         type, width;

    t_memattr_span( const uint16_t s_addr_, const uint16_t e_addr_, 
                    const uint8_t type_,    const uint8_t width_ ) 
    : s_addr( s_addr_ ), e_addr( e_addr_ ), type( type_ ), width( width_ )
    { }

    inline bool operator < ( const t_memattr_span& rhs ) const 
    {
        return s_addr < rhs.s_addr ? true : false;
    }

    inline bool can_merge_with( const t_memattr_span& rhs ) const
    {
        return e_addr == rhs.s_addr - 1 
            && type   == rhs.type 
            && width  == rhs.width;
    }

    inline void merge_with( const t_memattr_span& rhs )
    {
        assert( can_merge_with( rhs ) );

        e_addr = rhs.e_addr;
    }
};

typedef std::vector< t_memattr_span > t_memattr_vec;

// ------------------------------------------------------------------------ //
//  T_BANKSW_SPAN                                                           //
// ------------------------------------------------------------------------ //
struct t_banksw_span
{
    uint16_t        s_addr, e_addr;

    t_banksw_span( const uint16_t s_addr_, const uint16_t e_addr_ )
    : s_addr( s_addr_ ), e_addr( e_addr_ ) 
    { }

    inline bool operator < ( const t_banksw_span& rhs ) const 
    {
        return s_addr < rhs.s_addr ? true : false;
    }

    inline bool can_merge_with( const t_banksw_span& rhs ) const
    {
        return e_addr == rhs.s_addr - 1;
    }

    inline void merge_with( const t_banksw_span& rhs )
    {
        assert( can_merge_with( rhs ) );

        e_addr = rhs.e_addr;
    }
};

typedef std::vector< t_banksw_span  > t_banksw_vec;

#endif 
