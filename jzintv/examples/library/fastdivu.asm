;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;; ======================================================================== ;;
;;  FASTDIVU      Divide two unsigned integers / fixed-pt numbers           ;;
;;  FASTDIVU.1    Alternate entry point:  denominator in register           ;;
;;  FASTDIVU.2    Alternate entry point:  all parameters in registers       ;;
;;                                                                          ;;
;;  AUTHOR                                                                  ;;
;;      Joseph Zbiciak <intvnut AT gmail.com>                               ;;
;;                                                                          ;;
;;  REVISION HISTORY                                                        ;;
;;      27-Sep-2001 Wrote fast version, shaving another ~200 cycles.        ;;
;;                                                                          ;;
;;  INPUTS for FASTDIVU                                                     ;;
;;      R0    Numerator                                                     ;;
;;      R5    Pointer to invocation record, followed by return address.     ;;
;;            Denominator                  1 DECLE                          ;;
;;            Fractional point             1 DECLE                          ;;
;;                                                                          ;;
;;  INPUTS for FASTDIVU.1                                                   ;;
;;      R0    Numerator                                                     ;;
;;      R1    Denominator                                                   ;;
;;      R5    Pointer to invocation record, followed by return address.     ;;
;;            Fractional point             1 DECLE                          ;;
;;                                                                          ;;
;;  INPUTS for FASTDIVU.2                                                   ;;
;;      R0    Numerator                                                     ;;
;;      R1    Denominator                                                   ;;
;;      R2    Fraction                                                      ;;
;;      R5    Return address                                                ;;
;;                                                                          ;;
;;  OUTPUTS                                                                 ;;
;;      R0    Remainder, left-shifted                                       ;;
;;      R1    Clobbered                                                     ;;
;;      R2    Clobbered                                                     ;;
;;      R3    Quotient                                                      ;;
;;      R4    Unmodified                                                    ;;
;;      R5    Return address                                                ;;
;;                                                                          ;;
;;  NOTES                                                                   ;;
;;      Both numerator and denominator MUST be positive, and less than      ;;
;;      0x8000.  This code may loop indefinitely if this is not heeded.     ;;
;;                                                                          ;;
;;      The remainder returned in R0 isn't directly usable.  This code      ;;
;;      can be modified to return a proper remainder by recording the       ;;
;;      number of left-shifts applied to the denominator in the norm        ;;
;;      loop, and applying that number of right-shifts to the remainder.    ;;
;;      Storing R2 after the normalization loop works for integer           ;;
;;      division (fractional point == 0) only.                              ;;
;;                                                                          ;;
;;      This code can perform fixed-point divide between two fixed point    ;;
;;      numbers, yielding a fixed-point result.  Given the numerator's      ;;
;;      fractional point X, the denominator's fractional point Y, and       ;;
;;      the desired fractional point Z, the required argument F for         ;;
;;      this divide is:  F = Z + Y - X.                                     ;;
;;                                                                          ;;
;;      This divide rounds towards zero.  All remainders are rounded down.  ;;
;;                                                                          ;;
;;  TECHNIQUES                                                              ;;
;;      Left-shifting method on numerator allows calculating fractional     ;;
;;      quotients as well as integer quotients.                             ;;
;;                                                                          ;;
;;  CODESIZE                                                                ;;
;;      28 words                                                            ;;
;;                                                                          ;;
;;  CYCLES                                                                  ;;
;;      Worst case analysis:                                                ;;
;;                                                                          ;;
;;      For FASTDIVU, cycles = 456 + 46*k, where k is # of quotient bits.   ;;
;;      The number of quotient bits 'k' is given by this equation:          ;;
;;                                                                          ;;
;;          k = ceil(log2(num)) - floor(log2(den)) + fractional_bits.       ;;
;;                                                                          ;;
;;      Subtract  8 cycles for FASTDIVU.1.                                  ;;
;;      Subtract 16 cycles for FASTDIVU.2.                                  ;;
;;                                                                          ;;
;;      A worst-case divide with 15 quotient bits should take no more       ;;
;;      than 1146 cycles.  (The actual cycle count will depend on the       ;;
;;      relative magnitude of the numbers being divided.)                   ;;
;;                                                                          ;;
;; ======================================================================== ;;
FASTDIVU    PROC         

            MVI@    R5,     R1      ;   8   Denominator
@@1:        MVI@    R5,     R2      ;   8   Fractional point
@@2:                                ;       Alt. entry: All args in regs 
            CLRR    R3              ;   6   Start w/ quotient == 0.
                                    ;----
                                    ;  22

            ;; ------------------------------------------------------------ ;;
            ;;  Normalize the divisor relative to the dividend.  We want    ;;
            ;;  to shift the denominator left as far as we can without      ;;
            ;;  making it larger than the numerator.  We achieve this by    ;;
            ;;  shifting it one position too far, then backing off.         ;;
            ;; ------------------------------------------------------------ ;;

@@norm:     INCR    R2              ;   6   \
            SLL     R1,     1       ;   6    |-- General normalize:  Shift
            CMPR    R1,     R0      ;   6   /    until denom > numer.
            BC      @@norm          ;   9/7
                                    ;------
                                    ;  27*k - 2
                                    ;  22        
                                    ;------
                                    ;  27*k + 20
                                    ;======      Assume max K of 15
                                    ; 425        (worst case)


@@over:     SLR     R1,     1       ;   6   Back off by one. 
            CMPR    R3,     R2      ;   6   Is our divide loop iter count
                                    ;       at least 1?   (Note R3==0)
            BLE     @@zero          ;   7/9 NO:  Return zero.
            INCR    PC              ;   7   Do first iter of divide
                                    ;------
                                    ;  26        (worst case)
                                    ; 425        (worst case)
                                    ;======      
                                    ; 451        (worst case)

            ;; ------------------------------------------------------------ ;;
            ;;  Perform the divide.  We iteratively subtract off our        ;;
            ;;  divisor from the dividend *IF* the dividend is greater or   ;;
            ;;  or equal to the divisor, and set the corresponding bit in   ;;
            ;;  the quotient.  If the dividend is smaller than the divisor, ;;
            ;;  we clear the corresponding quotient bit.  Next, we left-    ;;
            ;;  shift the dividend and calculate the next quotient bit.     ;;
            ;; ------------------------------------------------------------ ;;

@@div:      SLL     R0,     1       ;   6   Shift numerator left 1
@@div1st:   CMPR    R1,     R0      ;   6   Is numerator >= denominator ?
            BNC     @@b0            ;   7/9 NO:   Quotient bit is 0

@@b1:       RLC     R3,     1       ;   6   YES:  Quotient bit is 1
            SUBR    R1,     R0      ;   6
            DECR    R2              ;   6
            BNEQ    @@div           ;   9/7 Iterate for all quotient bits
                                    ;------
                                    ;  46*k - 2
@@zero:     JR      R5              ;   7   Return!
                                    ; 451
                                    ;======
                                    ; 456 + 46*k worst case.

@@b0:       SLL     R3,     1       ;   6   NO:   Quotient bit is 0
            DECR    R2              ;   6
            BNEQ    @@div           ;   9/7 Iterate for all quotient bits
                                    ;------
                                    ;  42*k - 2
@@done:     JR      R5              ;   7   Return!
                                    ; 451
                                    ;======
                                    ; 456 + 42*k 

            ENDP    

;; ======================================================================== ;;
;;  End of File:  fastdivu.asm                                              ;;
;; ======================================================================== ;;
