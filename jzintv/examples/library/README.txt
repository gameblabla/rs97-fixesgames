This directory contains a variety of functions and definitions that could
be considered "library" functions.  Several of the examples include these
files directly to access various functions, such as the random number
generator, the string-printing routines, and so on.

You are encouraged to use these routines in your programs, either by
INCLUDEing the file or copying the routine inline.  These routines
have been released into the public domain by their author, Joseph
Zbiciak.  You're welcome to use these for whatever purpose you like.

The documentation for each function is contained within the source file
itself.  Please consult the individual files for details.

    dec16dec32.asm      DEC16 and DEC32 routines
    dec16only.asm       DEC16 with DEC32 support removed for saving codesize
    dividivu.asm        Signed and unsigned divide
    fastdivu.asm        Unsigned divide only
    dist_fast.asm       Fast Euclidean distance estimation
    fillmem.asm         FILLMEM, FILLZERO and CLRSCR routines
    gimini.asm          Various constants and masks useful for Intellivision
    hex16.asm           HEX16 routine alone
    hexdisp.asm         HEX16 thru HEX4 and HEX12M thru HEX4M routines.
    ic_banksw.asm       Intellicart bank-switching support routines
    memcmp.asm          MEMCMP routine
    memcpy.asm          MEMCPY routine
    memset.asm          MEMSET routine (similar to FILLMEM)
    memunpk.asm         MEMUNPK routine
    print.asm           PRINT.xxx string display routines
    rand.asm            RAND random number generation routine
    sqrt.asm            SQRT square-root routine
    colorsq.asm         Baseline colored-squares routines
    colorsq_sv.asm      PUTPIXELS/GETPIXELS wrappers on PUTPIXEL/GETPIXEL
    colorsq_clip.asm    Clipping versions of PUTPIXELS/GETPIXELS
    ivoice.asm          Intellivoice driver routines
    saynum16.asm        Speaks the value of a 16-bit number 
    resrom.asm          Defines symbols for the Intellivoice's RESROM
    al2.asm             SP0256 Allophone Library

You may find it convenient to "INCLUDE" these files in your program,
rather than copying the source-code inline.  To do this, simply add
a line in your program such as the following:  (Note the use of 
forward-slashes for path separators.  This works identically under 
both Unix and DOS/Windows.)

    INCLUDE "/full/path/to/library/function.asm"

OR

    INCLUDE "../relative/path/to/library/function.asm"

Alternately, rather than hardcoding the path to the library in
the source code, you can use AS1600's Include Path facility to
specify places to search for INCLUDE files.  The Include Path
can be specified two ways:

 -- On the command line, with "-i /path/to/directory."  You may
    have as many -i arguments as you can fit on the commandline.

 -- With the AS1600_PATH environment variable.  You can specify
    multiple directories to search in the one variable.  The
    names for the individual directories are separated by ';'
    characters (semicolons).
    
The assembler will always search for include files in the following
order:

 1. The current directory.

 2. Any directories specified on the commandline, in the order 
    they were specified.

 3. Any directories specified in the AS1600_PATH listing.


