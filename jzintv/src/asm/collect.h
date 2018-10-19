/* ======================================================================== */
/*  Collects all the ROM data for a given assembly, and then outputs it.    */
/*                                                                          */
/*  Intended to add some abstraction to the current ROM output code.        */
/* ======================================================================== */
#ifndef COLLECT_H_
#define COLLECT_H_

/* ------------------------------------------------------------------------ */
/*  COLLECT_INIT     -- Initialize the collector.                           */
/* ------------------------------------------------------------------------ */
void collect_init(void);

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
);

/* ------------------------------------------------------------------------ */
/*  COLLECT_FLUSH    -- Write out everything that was collected.            */
/* ------------------------------------------------------------------------ */
void collect_flush(void);

/* ------------------------------------------------------------------------ */
/*  COLLECT_CFG_VAR  -- Collect configuration variables for BIN+CFG format  */
/* ------------------------------------------------------------------------ */
void collect_cfg_var(const char *var, const char *string, int value);

#endif
