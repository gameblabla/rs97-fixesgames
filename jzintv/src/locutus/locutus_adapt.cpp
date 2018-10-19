// ======================================================================== //
//  Adapt the Locutus class to the periph_t "class" in jzIntv               //
// ======================================================================== //

#include "locutus.hpp"
#include "luigi.hpp"

#include "locutus_adapt.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

class t_cpu_cache : public t_cpu_cache_if
{
  private:
    cp1600_t *const cp1600;

  public:
    t_cpu_cache( cp1600_t *cp1600_ ) : cp1600( cp1600_ ) { }

    virtual void invalidate( uint16_t addr_lo, uint16_t addr_hi )
    {
        if ( cp1600 )
            cp1600_invalidate( cp1600, addr_lo, addr_hi );
    }
};

struct t_locutus_priv
{
    t_locutus    locutus;
    t_cpu_cache  cpu_cache;

    t_locutus_priv( cp1600_t *cp1600 ) : cpu_cache( cp1600 ) { } 
};


extern "C" uint_32 locutus_read( periph_p periph, periph_p, 
                                 uint_32 addr,    uint_32 )
{
    t_locutus_wrap *wrap = (t_locutus_wrap *)periph;

    try
    {
        return wrap->locutus_priv->locutus.intv_read( addr );
    } catch ( string &s )
    {
        cerr << "EXCEPTION: " << s << endl;
        exit(1);
    }
}

extern "C" void locutus_write( periph_p periph, periph_p, 
                               uint_32 addr,    uint_32 data )
{
    t_locutus_wrap *wrap = (t_locutus_wrap *)periph;

    try
    {
        return wrap->locutus_priv->locutus.intv_write( addr, data );
    } catch ( string &s )
    {
        cerr << "EXCEPTION: " << s << endl;
        exit(1);
    }
}

extern "C" void locutus_reset( periph_p periph )
{
    t_locutus_wrap *wrap = (t_locutus_wrap *)periph;

    try
    {
        wrap->locutus_priv->locutus.intv_reset();
    } catch ( string &s )
    {
        cerr << "EXCEPTION: " << s << endl;
        exit(1);
    }
}

extern "C" void locutus_dtor( periph_p periph )
{
    t_locutus_wrap *wrap = (t_locutus_wrap *)periph;

    delete wrap->locutus_priv;          
    wrap->locutus_priv = 0;
}

extern "C" int make_locutus
(
    t_locutus_wrap  *loc_wrap,      /*  pointer to a Locutus wrapper    */
    const char      *luigi_file,    /*  LUIGI file to load into Locutus */
    cp1600_t        *cp1600
)
{
    t_locutus_priv *priv     = new t_locutus_priv( cp1600 );
    loc_wrap->locutus_priv   = priv;

    ifstream ifs(luigi_file, ios::binary | ios::ate);

    if ( !ifs.is_open() )
    {
        cerr << "could not open " << luigi_file << endl;
        return -1;
    }


    ifstream::pos_type pos = ifs.tellg();

    t_byte_vec luigi_data(pos);

    ifs.seekg(0, ios::beg);
    ifs.read((char *)&luigi_data[0], pos);


    try
    {
        t_luigi::deserialize( priv->locutus, luigi_data );
    } catch ( string &s )
    {
        cerr << "EXCEPTION: " << s << endl;
        return -1;
    }

    priv->locutus.intv_reset();

    loc_wrap->periph.read       = locutus_read;
    loc_wrap->periph.write      = locutus_write;
    loc_wrap->periph.peek       = locutus_read;
    loc_wrap->periph.poke       = locutus_write;
    loc_wrap->periph.reset      = locutus_reset;
    loc_wrap->periph.dtor       = locutus_dtor; 

    loc_wrap->periph.tick       = NULL;
    loc_wrap->periph.min_tick   = ~0U;
    loc_wrap->periph.max_tick   = ~0U;

    loc_wrap->periph.addr_base  = 0;
    loc_wrap->periph.addr_mask  = ~0U;
    loc_wrap->periph.ser_init   = NULL; // no support for serializer() 

    return 0;
}
