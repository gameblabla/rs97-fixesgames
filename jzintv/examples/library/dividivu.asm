;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;; ======================================================================== ;;
;;  DIVI      Divide two signed integers / fixed-pt numbers                 ;;
;;  DIVI.1    Alternate entry point:  denominator in register               ;;
;;  DIVI.2    Alternate entry point:  all parameters in registers           ;;
;;  DIVU      Divide two unsigned integers / fixed-pt numbers               ;;
;;  DIVU.1    Alternate entry point:  denominator in register               ;;
;;  DIVU.2    Alternate entry point:  all parameters in registers           ;;
;;                                                                          ;;
;;  AUTHOR                                                                  ;;
;;      Joseph Zbiciak <intvnut AT gmail.com>                               ;;
;;                                                                          ;;
;;  REVISION HISTORY                                                        ;;
;;      25-Sep-2001 Initial Revision                                        ;;
;;      27-Sep-2001 Rewrote normalization, shaving ~150 cycles.             ;;
;;                                                                          ;;
;;  INPUTS for DIVI, DIVU                                                   ;;
;;      R0    Numerator                                                     ;;
;;      R5    Pointer to invocation record, followed by return address.     ;;
;;            Denominator                  1 DECLE                          ;;
;;            Fractional point             1 DECLE                          ;;
;;                                                                          ;;
;;  INPUTS for DIVI.1, DIVU.1                                               ;;
;;      R0    Numerator                                                     ;;
;;      R1    Denominator                                                   ;;
;;      R5    Pointer to invocation record, followed by return address.     ;;
;;            Fractional point             1 DECLE                          ;;
;;                                                                          ;;
;;  INPUTS for DIVI.2, DIVU.2                                               ;;
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
;;      This divide rounds towards zero.  In other words, for signed        ;;
;;      division, both -1/2 == 0 and 1/2 == 0.  For unsigned division,      ;;
;;      all remainders are rounded down.                                    ;;
;;                                                                          ;;
;;  TECHNIQUES                                                              ;;
;;      Left-shifting method on numerator allows calculating fractional     ;;
;;      quotients as well as integer quotients.                             ;;
;;                                                                          ;;
;;      Negative numerators and denominators are handled by making both     ;;
;;      numerator and denominator positive up-front, and then negating      ;;
;;      the result if necessary.  (This step is obviously omitted in the    ;;
;;      case of an unsigned divide.)                                        ;;
;;                                                                          ;;
;;  CODESIZE                                                                ;;
;;      72 words.                                                           ;;
;;                                                                          ;;
;;      6 words may be shaved by omitting DIVU.                             ;;
;;      20 words may be shaved by omitting DIVI.                            ;;
;;                                                                          ;;
;;  CYCLES                                                                  ;;
;;      Worst case analysis:                                                ;;
;;                                                                          ;;
;;      For DIVI, cycles = 601 + 53*k, where k is # of quotient bits.       ;;
;;      For DIVU, cycles = 552 + 53*k, where k is # of quotient bits.       ;;
;;      The number of quotient bits 'k' is given by this equation:          ;;
;;                                                                          ;;
;;          k = ceil(log2(num)) - floor(log2(den)) + fractional_bits.       ;;
;;                                                                          ;;
;;      Subtract  8 cycles for DIVI.1 or DIVU.1.                            ;;
;;      Subtract 16 cycles for DIVI.2 or DIVU.2.                            ;;
;;                                                                          ;;
;;      A worst-case divide with 16 quotient bits that are all 1s should    ;;
;;      take no more than 1448 cycles.  (The actual cycle count will        ;;
;;      depend on the relative magnitude of the numbers being divided.)     ;;
;;                                                                          ;;
;; ======================================================================== ;;
DIVU        PROC         

            MVI@    R5,     R1      ;   8   Denominator
@@1:        MVI@    R5,     R2      ;   8   Fractional point
@@2:                                ;       Alt. entry: All args in regs

            CLRR    R3              ;   6   Start w/ quotient == 0.
            PSHR    R3              ;   9   Sign of result == positive
            B       DIVI.norm       ;   9   Reuse divi code for divide
                                    ;======
                                    ;  40  
            ENDP

DIVI        PROC
            MVI@    R5,     R1      ;   8   Denominator
@@1:        MVI@    R5,     R2      ;   8   Fractional point
@@2:                                ;       Alt. entry: All args in regs

            CLRR    R3              ;   6   Start w/ quotient == 0.

            ;; ------------------------------------------------------------ ;;
            ;;  Record sign of result, and make num, den positive.          ;;
            ;; ------------------------------------------------------------ ;;
            XORR    R1,     R0      ;   6   \
            PSHR    R0              ;   9    |-- Record sign of result
            XORR    R1,     R0      ;   6   /    on the stack.

            TSTR    R0              ;   6   \
            BEQ     @@zero          ;   7/9  |__ Make numerator positive
            BPL     @@npos          ;   7/9  |
            NEGR    R0              ;   6   /
@@npos:
            TSTR    R1              ;   6   \
            BEQ     @@zero          ;   7/9  |__ Make denominator positive
            BPL     @@dpos          ;   7/9  |
            NEGR    R1              ;   6   /
@@dpos:                             ;------
                                    ;  95  (worst case: both negative)
                                    ;  87  (typical:    both positive)
                                    ;  58  (best case:  numerator zero)
                                    ;======
                                    ;  95  (worst case: both negative)

            ;; ------------------------------------------------------------ ;;
            ;;  Normalize the divisor relative to the dividend.  We want    ;;
            ;;  to shift the denominator left as far as we can without      ;;
            ;;  making it larger than the numerator.  We achieve this by    ;;
            ;;  shifting it one position too far, then backing off.         ;;
            ;; ------------------------------------------------------------ ;;
@@norm:     TSTR    R1              ;   6   Is bit 15 of denominator set?
            BMI     @@norm3         ;   7/9 Yes:  Do very special norm.
            TSTR    R0              ;   6   Is bit 15 of numerator set?
            BPL     @@norm1         ;   9/7 If not, do general normalize.
                                    ;       Otherwise, do special normalize.

@@norm2:    INCR    R2              ;   6   \
            SLLC    R1,     1       ;   6    |-- Special normalize:  Shift
            BNC     @@norm2         ;   9/7 /    until bit falls off top.
                                    ;------
                                    ;  21*k - 2
            B       @@over          ;   9
                                    ;  26
                                    ;------
                                    ;  21*k + 20
                                    ;======      Assume max K of 16
                                    ; 369        (worst case)

@@norm1:    INCR    R2              ;   6   \
            SLL     R1,     1       ;   6    |-- General normalize:  Shift
            CMPR    R1,     R0      ;   6   /    until denom > numer.
            BC      @@norm1         ;   9/7
                                    ;------
                                    ;  27*k - 2
                                    ;  28
                                    ;======      Assume max K of 15
                                    ; 431        (worst case)


@@over:     RRC     R1,     1       ;   6   Back off by one. 
            INCR    PC              ;   6   Skip INCR below
@@norm3:    INCR    R2              ;   6   Very special norm...
            CMPR    R3,     R2      ;   6   Is our divide loop iter count
                                    ;       at least 1?   (Note R3==0)
            BLE     @@zero          ;   7/9 NO:  Return zero.
            B       @@div1st        ;   9   Do first iter of divide
                                    ;------
                                    ;  34        (worst case)
                                    ;  95        (worst case)
                                    ; 431        (worst case)
                                    ;======      
                                    ; 560        (worst case)

            ;; ------------------------------------------------------------ ;;
            ;;  Perform the divide.  We iteratively subtract off our        ;;
            ;;  divisor from the dividend *IF* the dividend is greater or   ;;
            ;;  or equal to the divisor, and set the corresponding bit in   ;;
            ;;  the quotient.  If the dividend is smaller than the divisor, ;;
            ;;  we clear the corresponding quotient bit.  Next, we left-    ;;
            ;;  shift the dividend and calculate the next quotient bit.     ;;
            ;; ------------------------------------------------------------ ;;

@@div:      SLLC    R0,     1       ;   6   Shift numerator left 1
            BC      @@b1            ;   7/9 If overflow, force bit 1.
@@div1st:   CMPR    R1,     R0      ;   6   Is numerator >= denominator ?
            BNC     @@b0            ;   7/9 NO:   Quotient bit is 0

@@b1:       RLC     R3,     1       ;   6   YES:  Quotient bit is 1
            SUBR    R1,     R0      ;   6
            DECR    R2              ;   6
            BNEQ    @@div           ;   9/7 Iterate for all quotient bits
                                    ;------
                                    ;  53*k - 2
            B       @@divdone       ;   9
                                    ;------
                                    ;  53*k + 7

@@b0:       SLL     R3,     1       ;   6   NO:   Quotient bit is 0
            DECR    R2              ;   6
            BNEQ    @@div           ;   9/7 Iterate for all quotient bits
                                    ;------
                                    ;  49*k - 2
                                    
                                    ;======
                                    ; 567 + 53*k worst case.
@@divdone:            
            ;; ------------------------------------------------------------ ;;
            ;;  Now apply the sign to the result.  The sign is in the MSB   ;;
            ;;  of the word we saved earlier on the stack.  For unsigned    ;;
            ;;  divides, the value saved on the stack is zero.              ;;
            ;; ------------------------------------------------------------ ;;
@@zero:     PULR    R1              ;   8
            TSTR    R1              ;   6
            BPL     @@qpos          ;   7/9
            NEGR    R3              ;   6
@@qpos:                             ;------
                                    ;  27         (worst case)
                                    ; 567 + 53*k  (worst case)
                                    ;======
                                    ; 594 + 53*k  (worst case)

@@done:     JR      R5              ;   7   Return.  R3 = R0 / R1
                                    ;======
                                    ; 601 + 53*k  (worst case)
            ENDP    

;; ======================================================================== ;;
;;  End of File:  dividivu.asm                                              ;;
;; ======================================================================== ;;
