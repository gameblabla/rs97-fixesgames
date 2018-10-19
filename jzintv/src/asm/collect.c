/* ======================================================================== */
/*  Collects all the ROM data for a given assembly, and then outputs it.    */
/*                                                                          */
/*  Intended to add some abstraction to the current ROM output code.        */
/* ======================================================================== */

#include "config.h"
#include "lzoe/lzoe.h"
#include "file/file.h"
#include "frasmdat.h"
#include "fragcon.h"
#include "as1600_types.h"
#include "protos.h"
#include "icart/icartrom.h"
#include "icart/icartbin.h"
#include "collect.h"

/* ------------------------------------------------------------------------ */
/*  Flags assocated with each word in the ROM image.                        */
/*  Currently identical to the set used by ICARTROM.                        */
/*  Uninitialized spans set the word to 0xFFFF and do not set FLAG_HADATA.  */
/* ------------------------------------------------------------------------ */
#define FLAG_READ       ICARTROM_READ       
#define FLAG_WRITE      ICARTROM_WRITE
#define FLAG_NARROW     ICARTROM_NARROW
#define FLAG_BANKSW     ICARTROM_BANKSW
#define FLAG_HASDATA    ICARTROM_PRELOAD
#define FLAG_PAGESW     (1 << 5)

#define ICART_TO_MODE(x)    ((x) & 0xF);
typedef struct rom_data_t
{
    uint_16 word;
    uint_16 flag;              
} rom_data_t;

typedef struct cfg_var_t
{
    const char *var;
    const char *string;
    int         value;
} cfg_var_t;

#ifdef BYTE_LE
LOCAL INLINE uint_16 host_to_be16(uint_16 x)
{
    return (x >> 8) | (x << 8);
}
#else
LOCAL INLINE uint_16 host_to_be16(uint_16 x)
{
    return x;
}
#endif


/* ------------------------------------------------------------------------ */
/*  ROM_DATA                                                                */
/*                                                                          */
/*  Allocate a worst-case structure for the largest ROM you could assemble. */
/*  This is far easier to get correct than a fancier data structure, and    */
/*  on any computer built this century (or even a smartphone built since    */
/*  2007), a relatively trivial amount of RAM to request.                   */
/* ------------------------------------------------------------------------ */
LOCAL rom_data_t rom_data[16][65536]; /* The ROM data.                      */
LOCAL int        rom_page[16];        /* pages present in each 4K window    */
LOCAL int        rom_err [16];         
LOCAL int        rom_is_pagesw;       /* At least one page-switch segment   */
LOCAL int        rom_is_banksw;       /* At least one bank-switch segment   */
LOCAL int        rom_is_confused;
LOCAL cfg_var_t *rom_cfg_var = NULL;  /* Configration variables             */
LOCAL int        rom_cfg_vars      = 0;
LOCAL int        rom_cfg_var_alloc = 0;

/* ------------------------------------------------------------------------ */
/*  These flags indicate whether it's safe to output a ROM of the           */
/*  indicated format.                                                       */
/*                                                                          */
/*  For example, Intellicart / CC3 do not support ECS-style paged ROMs.     */
/*  BIN + CFG doesn't *really* support Intellicart-style bankswitching.     */
/*  The CFG format allows specifying it, but jzIntv won't load it, so the   */
/*  assembler will output a warning when it sees it.                        */
/* ------------------------------------------------------------------------ */
int binfmt_ok = 1;      /* BIN + CFG                */
int romfmt_ok = 1;      /* Intellicart / CC3 .ROM   */

/* ------------------------------------------------------------------------ */
/*  COLLECT_INIT     -- Initialize the collector.                           */
/* ------------------------------------------------------------------------ */
void collect_init(void)
{
    int a, p, s;

    /* Initialize entire ROM image to "not present" */
    for (p = 0; p < 16; p++)
        for (a = 0; a < 65536; a++)
        {
            rom_data[p][a].word = 0xFFFF;
            rom_data[p][a].flag = 0;
        }

    /* Set entire ROM image to "unpaged", in the ECS-style paging sense */
    for (s = 0; s < 16; s++)
    {
        rom_page[s] = 0;
        rom_err [s] = 0;
    }

    rom_is_pagesw   = 0;
    rom_is_banksw   = 0;
    rom_is_confused = 0;
}

/* ------------------------------------------------------------------------ */
/*  COLLECT_ADDSEG   -- Add a segment of data to the ROM image.             */
/* ------------------------------------------------------------------------ */
const char *collect_addseg
(
    uint_16 *data,
    uint_32 addr,
    uint_32 len,
    sint_8  page,
    uint_8  set_attr,
    uint_8  clr_attr
)
{
    uint_16 addr_lo, addr_hi;
    uint_16 slot_lo, slot_hi, s;
    uint_16 flag, cum_flag;
    int p, i, a;

    /* -------------------------------------------------------------------- */
    /*  Overflow sanity check                                               */
    /* -------------------------------------------------------------------- */
    if (addr + len > 0x10000)
    {
        static int ao_err = 0;
        return ao_err++ ? NULL : "Address overflow (collect)";
    }

    /* -------------------------------------------------------------------- */
    /*  Page == -1 means unpaged; otherwise only allow 0 .. 15              */
    /* -------------------------------------------------------------------- */
    if (page > 15 || page < -1) 
        return "Page number out of range";

    /* -------------------------------------------------------------------- */
    /*  Bankswitched (Intellicart-style) or Paged (ECS-style)?              */
    /* -------------------------------------------------------------------- */
    if ((set_attr & ICARTROM_BANKSW) != 0) rom_is_banksw = 1;
    if (page >= 0)                         rom_is_pagesw = 1;

    if (rom_is_banksw && rom_is_pagesw)
    {
        return rom_is_confused++ 
                ? NULL 
                : "Cannot mix page-switching and bank-switching";
    }

    addr_lo = addr;
    addr_hi = addr + len - 1;
    slot_lo = addr_lo >> 12;
    slot_hi = addr_hi >> 12;

    /* -------------------------------------------------------------------- */
    /*  Check for page / unpaged consistency within 4K slots.               */
    /* -------------------------------------------------------------------- */
    for (s = slot_lo; s <= slot_hi; s++)
    {
        if ((rom_page[s] == -1 && page >= 0) ||
            (rom_page[s] > 0   && page <  0))
            return rom_err[s]++ 
                     ? NULL 
                     : "Mixture of paged and unpaged ROM in same 4K range";
    }

    /* -------------------------------------------------------------------- */
    /*  If it's paged, go ahead and set the page-present;  If it's unpaged, */
    /*  mark it unpaged.                                                    */
    /* -------------------------------------------------------------------- */
    for (s = slot_lo; s <= slot_hi; s++)
    {
        if (page >= 0)  rom_page[s] |= 1 << page;
        else            rom_page[s]  = -1;
    }

    if (page >= 0)
        set_attr |= FLAG_PAGESW;

    /* -------------------------------------------------------------------- */
    /*  Copy the data in, if any, and update flags.                         */
    /* -------------------------------------------------------------------- */
    p = page < 0 ? 0 : page;

    if (data)
    {
        for (i = 0, a = addr_lo; a <= addr_hi; a++, i++)
            rom_data[p][a].word = data[i];

        set_attr |= FLAG_HASDATA;
    }

    cum_flag = 0;
    for (a = addr_lo; a <= addr_hi; a++)
    {
        flag = rom_data[p][a].flag;
        flag &= ~clr_attr;
        flag |=  set_attr;
        rom_data[p][a].flag = flag;

        cum_flag |= flag;
    }

    /* -------------------------------------------------------------------- */
    /*  BIN+CFG warnings                                                    */
    /* -------------------------------------------------------------------- */
    if (binoutf && cfgoutf)
    {

        /* ---------------------------------------------------------------- */
        /*  Warn if we see initialized, writeable data, and BIN+CFG was     */
        /*  requested.  Such files really only work well with .ROM.         */
        /* ---------------------------------------------------------------- */
        if ((cum_flag & FLAG_HASDATA) != 0 &&
            (cum_flag & FLAG_WRITE)   != 0)
        {
            static int iw_warn = 0;

            if (!iw_warn)
            {
                iw_warn = 1;
                frp2warn("Writeable, initialized memory is not well "
                         "supported in BIN+CFG format");
            }
            binfmt_ok = 0;
        }

        /* ---------------------------------------------------------------- */
        /*  Warn if we see bankswitched ROM, and BIN+CFG requested.         */
        /* ---------------------------------------------------------------- */
        if (rom_is_banksw)
        {
            static int bs_warn = 0;

            if (!bs_warn)
            {
                bs_warn = 1;
                frp2warn("Bankswitched memory is not well supported "
                         "in BIN+CFG format");
            }
            binfmt_ok = 0;
        }

        /* ---------------------------------------------------------------- */
        /*  Warn if we see initialized, non-readable memory in a BIN+CFG.   */
        /* ---------------------------------------------------------------- */
        if ((cum_flag & FLAG_HASDATA) != 0 &&
            (cum_flag & FLAG_READ)    == 0)
        {
            static int nr_warn = 0;

            if (!nr_warn)
            {
                nr_warn = 1;
                frp2warn("Non-readable, initialized memory not well "
                         "supported in BIN+CFG format");
            }
            binfmt_ok = 0;
        }

        /* ---------------------------------------------------------------- */
        /*  Bankswitched RAM is right out.                                  */
        /* ---------------------------------------------------------------- */
        if ((cum_flag & FLAG_WRITE)  != 0 && 
            (cum_flag & FLAG_PAGESW) != 0)
        {
            binfmt_ok = romfmt_ok = 0;
            return rom_err[slot_lo]++ ? NULL : "Paged RAM not supported.";
        }
    }

    /* -------------------------------------------------------------------- */
    /*  ROM warnings                                                        */
    /* -------------------------------------------------------------------- */
    if (romoutf)
    {
        static int ps_warn = 0;

        if (rom_is_pagesw & !ps_warn)
        {
            ps_warn = 1;
            frp2warn("ROM format does not support page-switched output.");

            romfmt_ok = 0;
        }
    }

    if (!binfmt_ok && (!romoutf || !romfmt_ok) && !rom_is_confused)
    {
        return rom_is_confused++ 
                ? NULL 
                : "No compatible executable output format";
    }

    return NULL;
}

/* ------------------------------------------------------------------------ */
/*  COLLECT_CFG_VAR  -- Collect configuration variables for BIN+CFG format  */
/* ------------------------------------------------------------------------ */
void collect_cfg_var(const char *var, const char *string, int value)
{
    if (rom_cfg_vars == rom_cfg_var_alloc)
    {
        rom_cfg_var_alloc = rom_cfg_var_alloc ? rom_cfg_var_alloc << 1 : 16;
        rom_cfg_var = (cfg_var_t *)
                            realloc(rom_cfg_var, 
                                    rom_cfg_var_alloc * sizeof(cfg_var_t));
        if (!rom_cfg_var)
        {
            frp2error("Out of memory in collect_cfg_var");
            return;
        }
    }

    if (!rom_cfg_var)
        return;

    rom_cfg_var[rom_cfg_vars  ].var    = var;
    rom_cfg_var[rom_cfg_vars  ].string = string;
    rom_cfg_var[rom_cfg_vars++].value  = value;
}

LOCAL void output_romfmt(int and_bincfg);
LOCAL void output_bincfg(void);

/* ------------------------------------------------------------------------ */
/*  COLLECT_FLUSH    -- Write out everything that was collected.            */
/* ------------------------------------------------------------------------ */
void collect_flush(void)
{
    int want_bincfg = binoutf && cfgoutf;

    if (rom_is_confused)
    {
        frp2error("Unable to generate output file");
        return;
    }

    if (romfmt_ok && romoutf)
    {
        /* If we flagged BIN+CFG as faulty, use icb_write_bincfg() to write
           an Intellicart-compatible BIN+CFG at least. */
        output_romfmt(binfmt_ok == 0 && want_bincfg);
    }

    if (binfmt_ok && want_bincfg)
        output_bincfg();
}

LOCAL uint_16 binbuf[4096];

/* ------------------------------------------------------------------------ */
/*  OUTPUT_BINCFG                                                           */
/* ------------------------------------------------------------------------ */
LOCAL void output_bincfg(void)
{
    int addr, addr_lo, addr_hi, span_lo, span_hi, span_len, slot, page;
    int need_memattr = 0;
    int fileofs = 0, i;

    fprintf(cfgoutf, "[mapping]\n");

    addr_lo = addr_hi = -1;
    span_lo = span_hi = -1;
    span_len = -1;

    /* -------------------------------------------------------------------- */
    /*  Scan through by 4K chunks, outputting any defined ROM segments.     */
    /*  For paged segments, output exactly 4K sized chunks.  For non-paged  */
    /*  segments, output the exact words defined by the program image.      */
    /* -------------------------------------------------------------------- */
    for (page = 0; page < 16; page++)
    {
        for (slot = 0; slot < 16; slot++)
        {
            if (rom_page[slot] == 0 || rom_page[slot] == -1)
                continue;

            addr_lo = slot << 12;
            addr_hi = addr_lo | 0xFFF;

            /* if we get here, we're in a paged segment. */
            if (((rom_page[slot] >> page) & 1) == 0)
                continue;

            for (i = 0, addr = addr_lo; addr <= addr_hi; addr++, i++)
                binbuf[i] = host_to_be16( rom_data[page][addr].word );

            fprintf(cfgoutf, "$%.4X - $%.4X = $%.4X PAGE %.1X\n",
                    fileofs, fileofs + 0xFFF, addr_lo, page);

            fwrite((void *)binbuf, 2, 0x1000, binoutf);

            fileofs += 0x1000;
        }
    }

    for (slot = 0; slot < 16; slot++)
    {
        if (rom_page[slot] == 0)
            continue;

        addr_lo = slot << 12;
        addr_hi = addr_lo | 0xFFF;

        if (rom_page[slot] == -1)
        {
            span_lo = span_hi = -1;

            for (addr = addr_lo; addr <= addr_hi; addr++)
            {
                if ((rom_data[0][addr].flag & FLAG_WRITE) != 0)
                    need_memattr |= 1 << slot;

                if ((rom_data[0][addr].flag & FLAG_HASDATA) != 0)
                {
                    if (span_lo == -1) 
                    {
                        span_len = 0;
                        span_lo  = addr;
                    }
                    span_hi = addr;

                    binbuf[span_len++] = host_to_be16(rom_data[0][addr].word);
                }

                if ((rom_data[0][addr].flag & FLAG_HASDATA) == 0 || 
                    addr == addr_hi)
                {
                    if (span_lo >= 0)
                    {
                        fprintf(cfgoutf, "$%.4X - $%.4X = $%.4X\n",
                                fileofs, fileofs + span_len - 1, span_lo);

                        fwrite((void *)binbuf, 2, span_len, binoutf);

                        fileofs += span_len;
                        span_lo  = -1;
                        span_hi  = -1;
                        span_len = 0;
                    }
                }
            }
            continue;
        } 
    }

    /* -------------------------------------------------------------------- */
    /*  If we saw any RAM in the first scan, output the memattr section.    */
    /* -------------------------------------------------------------------- */
    if (need_memattr)
    {
        fprintf(cfgoutf, "\n[memattr]\n");

        for (slot = 0; slot < 16; slot++)
        {
            if (((need_memattr >> slot) & 1) == 0)
                continue;

            addr_lo = slot << 12;
            addr_hi = addr_lo | 0xFFF;

            /* First get the RAM 8s */
            for (addr = addr_lo; addr <= addr_hi; addr++)
            {
                uint_16 flag = rom_data[0][addr].flag;
                if ((flag & FLAG_WRITE ) != 0 &&
                    (flag & FLAG_NARROW) != 0)
                {
                    if (span_lo == -1) 
                        span_lo  = addr;
                    span_hi = addr;
                }

                if ((flag & FLAG_WRITE ) == 0 || 
                    (flag & FLAG_NARROW) == 0 ||
                    addr == addr_hi)
                {
                    if (span_lo >= 0)
                    {
                        fprintf(cfgoutf, "$%.4X - $%.4X = RAM 8\n",
                                span_lo, span_hi);

                        span_lo  = -1;
                        span_hi  = -1;
                    }
                }
            }

            /* Next get the RAM 16s */
            for (addr = addr_lo; addr <= addr_hi; addr++)
            {
                uint_16 flag = rom_data[0][addr].flag;
                if ((flag & FLAG_WRITE ) != 0 &&
                    (flag & FLAG_NARROW) == 0)
                {
                    if (span_lo == -1) 
                        span_lo  = addr;
                    span_hi = addr;
                }

                if ((flag & FLAG_WRITE ) == 0 || 
                    (flag & FLAG_NARROW) != 0 ||
                    addr == addr_hi)
                {
                    if (span_lo >= 0)
                    {
                        fprintf(cfgoutf, "$%.4X - $%.4X = RAM 16\n",
                                span_lo, span_hi);

                        span_lo  = -1;
                        span_hi  = -1;
                    }
                }
            }
        }
    }

    /* -------------------------------------------------------------------- */
    /*  Output any configuration variables.                                 */
    /* -------------------------------------------------------------------- */
    if (rom_cfg_vars)
    {
        fprintf(cfgoutf, "\n[vars]\n");
        for (i = 0; i < rom_cfg_vars; i++)
            if (rom_cfg_var[i].string)
                fprintf(cfgoutf, "%s = %s\n", 
                        rom_cfg_var[i].var, rom_cfg_var[i].string);
            else
                fprintf(cfgoutf, "%s = %d\n", 
                        rom_cfg_var[i].var, rom_cfg_var[i].value);
    }

    fflush(binoutf);
    fflush(cfgoutf);
}

extern ictype_t icart_type;

/* ------------------------------------------------------------------------ */
/*  OUTPUT_ROMFMT                                                           */
/* ------------------------------------------------------------------------ */
LOCAL void output_romfmt(int and_bincfg)
{
    icartrom_t *icart_rom;
    int addr;
    uint_32 size;
    uint_8 *rom_img;

    /* -------------------------------------------------------------------- */
    /*  Allocate an icart_rom, and populate it with the ROM data we've      */
    /*  collected.  We only need to scan page 0, because icart doesn't      */
    /*  support paged ROM.                                                  */
    /* -------------------------------------------------------------------- */
    icart_rom = CALLOC(icartrom_t, 1);
    icartrom_init(icart_rom);

    for (addr = 0; addr < 0x10000; addr++)
    {
        uint_16  flag = rom_data[0][addr].flag;
        uint_16 *data = flag & FLAG_HASDATA ? &rom_data[0][addr].word : 0;
        if (data || (flag & 0xF) != 0)
            icartrom_addseg(icart_rom, data, addr, 1, flag & 0xF, 0);
    }

    /* -------------------------------------------------------------------- */
    /*  Write out the ROM image next.                                       */
    /* -------------------------------------------------------------------- */
    rom_img = icartrom_genrom(icart_rom, &size, icart_type);

    if (rom_img)
    {
        fwrite(rom_img, 1, size, romoutf);
        fflush(romoutf);
        free(rom_img);
    } else
        frp2error("Internal Error:  Could not generate ROM image");

    /* -------------------------------------------------------------------- */
    /*  If we were asked to also write an Intellicart-specific BIN+CFG,     */
    /*  then do that also.                                                  */
    /* -------------------------------------------------------------------- */
    if (and_bincfg)
    {
        icb_write_bincfg(binoutf, cfgoutf, icart_rom, 0);
    }

    free(icart_rom);
}


