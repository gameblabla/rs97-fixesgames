;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

    IF (DEFINED PRNUM16) AND ((DEFINED _WITH_PRNUM32)) = 0
        ERR "Must INCLUDE PRNUM16 after PRNUM32."
    ENDI

;; ======================================================================== ;;
;;  PRNUM32.l     -- Print an unsigned 32-bit number left-justified.        ;;
;;  PRNUM32.b     -- Print an unsigned 32-bit number with leading blanks.   ;;
;;  PRNUM32.z     -- Print an unsigned 32-bit number with leading zeros.    ;;
;;                                                                          ;;
;;  AUTHOR                                                                  ;;
;;      Joseph Zbiciak  <im14u2c AT globalcrossing DOT net>                 ;;
;;                                                                          ;;
;;  REVISION HISTORY                                                        ;;
;;      30-Mar-2003 Initial complete revision                               ;;
;;                                                                          ;;
;;  INPUTS for all variants                                                 ;;
;;      R0  Lower 16 bits of number to print                                ;;
;;      R1  Upper 16 bits of number to print                                ;;
;;      R2  Width of field.  Must be >= 5.  Ignored by PRNUM32.l.           ;;
;;      R3  Format word, added to digits to set the color, etc.             ;;
;;          Note:  Bit 15 is ignored and manipulated internally.            ;;
;;      R4  Pointer to location on screen to print number                   ;;
;;      R5  Return address                                                  ;;
;;                                                                          ;;
;;  OUTPUTS                                                                 ;;
;;      R0  Zeroed                                                          ;;
;;      R1  Trashed                                                         ;;
;;      R2  Unmodified                                                      ;;
;;      R3  Unmodified, except for bit 15, which may be set.                ;;
;;      R4  Points to first character after field.                          ;;
;;      R5  Trashed                                                         ;;
;;                                                                          ;;
;;  DESCRIPTION                                                             ;;
;;      These routines print unsigned 32-bit numbers in a field 5 to 10     ;;
;;      positions wide.  The number is printed either in left-justified     ;;
;;      or right-justified format.  Right-justified numbers are padded      ;;
;;      with leading blanks or leading zeros.  Left-justified numbers       ;;
;;      are not padded on the right.                                        ;;
;;                                                                          ;;
;;      See PRNUM16 for a table illustrating output formats.                ;;
;;                                                                          ;;
;;  TECHNIQUES                                                              ;;
;;      This routine uses a 32-by-16 integer divide to cut the 32-bit       ;;
;;      number into two 5-digit segments.  It divides the initial number    ;;
;;      by 100000.  The quotient provides the upper half, and the           ;;
;;      remainder provides the lower half.                                  ;;
;;                                                                          ;;
;;      Both halves are printed by reusing the PRNUM16 routines.  The       ;;
;;      lower half is always printed using PRNUM16.z.  The upper half is    ;;
;;      printed using one of PRNUM16.l, PRNUM16.b, or PRNUM16.z, based on   ;;
;;      which entry point the caller used for invoking PRNUM32.             ;;
;;                                                                          ;;
;;      The number in the lower half can be in the range 00000...99999.     ;;
;;      Some of this range is outside the range of a 16-bit value.  We      ;;
;;      handle this by passing a special flag to PRNUM16 in bit 15 of       ;;
;;      the format word.  When set, PRNUM16 will add '5' to the leading     ;;
;;      digit of the number.  When clear, it leaves it unmodified.          ;;
;;      Thus, when our lower half is in the range 65536...99999, we cope    ;;
;;      by adding 15536 to the lower 16 bits and setting the "add 5" flag.  ;;
;;      This works, because the lower 16 bits are in the range 0 - 34463.   ;;
;;      After the 'tweak', they're in the range 15536 - 49999.              ;;
;;                                                                          ;;
;;  NOTES                                                                   ;;
;;      The left-justified variant ignores the field width.                 ;;
;;                                                                          ;;
;;      This code is fully reentrant, although it has a significant stack   ;;
;;      footprint.                                                          ;;
;;                                                                          ;;
;;      This code does not handle numbers which are too large to be         ;;
;;      displayed in the provided field.  If the number is too large,       ;;
;;      non-digit characters will be displayed in the initial digit         ;;
;;      position.  Also, the run time of this routine may get excessively   ;;
;;      large, depending on the magnitude of the overflow.                  ;;
;;                                                                          ;;
;;      This code requires PRNUM16.  Since PRNUM16 requires a minor tweak   ;;
;;      to support PRNUM32, it must either be included after PRNUM32, or    ;;
;;      assembled with the symbol "_WITH_PRNUM32" set.                      ;;
;;                                                                          ;;
;;  CODESIZE                                                                ;;
;;      81 words  (not including PRNUM16.)                                  ;;
;;                                                                          ;;
;;      To save code size, you can define the following symbols to omit     ;;
;;      some variants:                                                      ;;
;;                                                                          ;;
;;              _NO_PRNUM32.l:   Disables PRNUM32.l.  Saves 8 words.        ;;
;;              _NO_PRNUM32.b:   Disables PRNUM32.b.  Saves 8 words.        ;;
;;                                                                          ;;
;;      Defining both symbols saves 18 words total, because it omits        ;;
;;      some code shared by both routines.                                  ;;
;;                                                                          ;;
;;  STACK USAGE                                                             ;;
;;      This function uses up to 8 words of stack space.  That includes     ;;
;;      4 words of stack space consumed by PRNUM16 when printing the        ;;
;;      two halves of the number.                                           ;;
;; ======================================================================== ;;
    IF (DEFINED PRNUM32) = 0
PRNUM32 PROC

    
    IF (DEFINED _NO_PRNUM32.l) = 0
        ;; ---------------------------------------------------------------- ;;
        ;;  PRNUM32.l:  Print unsigned, left-justified.                     ;;
        ;; ---------------------------------------------------------------- ;;

@@l:    TSTR    R1
        BEQ     PRNUM16.l
        PSHR    R5
        MVII    #PRNUM16.l, R5
        B       @@com
    ENDI

    IF (DEFINED _NO_PRNUM32.b) = 0
        ;; ---------------------------------------------------------------- ;;
        ;;  PRNUM32.b:  Print unsigned with leading blanks.                 ;;
        ;; ---------------------------------------------------------------- ;;
@@b:    TSTR    R1
        BEQ     PRNUM16.b
        PSHR    R5
        MVII    #PRNUM16.b, R5
        B       @@com
    ENDI

        ;; ---------------------------------------------------------------- ;;
        ;;  PRNUM32.z:  Print unsigned with leading zeros.                  ;;
        ;; ---------------------------------------------------------------- ;;
@@z:    TSTR    R1
        BEQ     PRNUM16.z
        PSHR    R5
    IF NOT ((DEFINED _NO_PRNUM32.l) AND (DEFINED _NO_PRNUM32.b))
        MVII    #PRNUM16.z, R5
    ENDI

@@com   PSHR    R2              ; save our field width
        PSHR    R3              ; save our format word

        ;; ---------------------------------------------------------------- ;;
        ;;  Divide the 32-bit number by 100000, and remember the remainder. ;;
        ;;  This will give us two 5-digit numbers for the two halves.       ;;
        ;;  The upper half will always be in the range 0 to 42949, and so   ;;
        ;;  we can print it with one of the PRNUM16 routines.  The lower    ;;
        ;;  half is in the range 0...99999, so we have to do some fancy     ;;
        ;;  footwork in the case that it's above 65535.                     ;;
        ;;                                                                  ;;
        ;;  The loop below looks like it divides by 50000, but really we've ;;
        ;;  aligned the divisor within a 16-bit number.                     ;;
        ;; ---------------------------------------------------------------- ;;

        MVII    #50000, R2      ; 100000, right shifted by 1.
        MVII    #1,     R3      ; Divide is done when this bit falls out of R3
        B       @@div1          ; jump to the first divide step.

@@div:  RLC     R0,     1       ;\__ 32 bit left shift of numerator.
        RLC     R1,     1       ;/   
        BC      @@divf          ; last shift overflowed, force a subtract

@@div1: CMPR    R2,     R1      ; can we subtract from upper half?
        BNC     @@divn          ; nope, then don't subtract.

@@divf: SUBR    R2,     R1      ; subtract upper half. 
        SETC                    ; usu. the SUBR sets carry, except when forced

@@divn: RLC     R3,     1       ; left-shift the quotient, including new bit
        BNC     @@div           ; loop until quotient is full.


        ;; ---------------------------------------------------------------- ;;
        ;;  At this point, R1:R0 is our remainder, and R3 is our quotient.  ;;
        ;;  The remainder occupies the upper 17 bits of the R1:R0 pair.     ;;
        ;;  For now, hide this on the stack.  We do this dance carefully    ;;
        ;;  since we need to restore two items from the stack as well,      ;;
        ;;  and we don't want to lose the values in R4 or R5 either.        ;;
        ;;                                                                  ;;
        ;;  If there was no "upper half" (eg. our value is 65536...99999)   ;;
        ;;  go handle that case specially.  Urgl.                           ;;
        ;; ---------------------------------------------------------------- ;;

        SLLC    R0,     1       ; put 17th bit into carry
        MOVR    R3,     R0      ; save quotient into R0
        PULR    R3              ; recall our format word into R3
        PULR    R2              ; recall our field width

        RLC     R1,     1       ; R1 is bits 15 downto 0 of remainder
        SLL     R3,     1       ; \_ Force bit 15 of format word to 1 or 0.  
        RRC     R3,     1       ; /  This will add 5 to leading digit.
        TSTR    R3              ;
        BPL     @@lt64k         ; if there was no carry, rmdr is 0...65535
        ADDI    #15536, R1      ; add "15536" to rest of digits 
@@lt64k:
        TSTR    R0
        BEQ     @@notop         ; convert bottom into top if top = 0.

        PSHR    R2              ; save field width on stack
        PSHR    R3              ; save format word for bottom half
        PSHR    R1              ; save quotient for bottom half

        ;; ---------------------------------------------------------------- ;;
        ;;  Go ahead and print the first 5 digits of the result.            ;;
        ;; ---------------------------------------------------------------- ;;
        SUBI    #5,     R2      ; bottom accounts for 5 digits of field width
        ANDI    #$7FFF, R3      ; force bit 15 of R3 clear.

    IF (DEFINED _NO_PRNUM32.l) AND (DEFINED _NO_PRNUM32.b)
        CALL    PRNUM16.z
    ELSE
        MOVR    R5,     R1      ; R5 contains branch target.
        MVII    #@@ret, R5      ; remember return address.
        MOVR    R1,     PC      ; call one of PRNUM16.z, .b, or .l as needed
@@ret:  
    ENDI

        ;; ---------------------------------------------------------------- ;;
        ;;  Print the bottom half of the number.   We branch into the       ;;
        ;;  heart of PRNUM16, which effectively always leads with zeros.    ;;
        ;; ---------------------------------------------------------------- ;;
        MVII    #4,     R2      ; bottom is always exactly 5 digits.
        PULR    R0              ; get bits 15 downto 0 of remainder
        PULR    R3              ; get format word.
        INCR    SP              ; save garbage in R1 slot on stack
        MVII    #_PW10, R1      ; need to set up power-of-10 pointer first
        B       PRNUM16.dig1    ; chain return via PRNUM16.digit

        ;; ---------------------------------------------------------------- ;;
        ;;  If there was no "upper half", convert the "lower half" into an  ;;
        ;;  upper half.  This actually isn't too gross.                     ;;
        ;; ---------------------------------------------------------------- ;;
@@notop:
        MOVR    R1,     R0      ; convert bottom half to top half
    IF (DEFINED _NO_PRNUM32.l) AND (DEFINED _NO_PRNUM32.b)
        PULR    R5
        B       PRNUM16.z
    ELSE
        MOVR    R5,     R1      ; \ 
        PULR    R5              ;  |__ chain return via target PRNUM16 routine
        JR      R1              ; /
    ENDI

        ENDP
    ENDI
        
;; ======================================================================== ;;
;;  End of File:  prnum32.asm                                               ;;
;; ======================================================================== ;;
