;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;; ======================================================================== ;;
;;  MEMUNPK   Unpack a 16-bit array to an 8-bit array.                      ;;
;;  MEMUNPK.1 Alternate entry point                                         ;;
;;                                                                          ;;
;;  AUTHOR                                                                  ;;
;;      Joseph Zbiciak <intvnut AT gmail.com>                               ;;
;;                                                                          ;;
;;  REVISION HISTORY                                                        ;;
;;      08-Sep-2001 Initial Revision                                        ;;
;;                                                                          ;;
;;  INPUTS for MEMUNPK                                                      ;;
;;      R5    Pointer to invocation record, followed by return address.     ;;
;;            Pointer to destination       1 DECLE                          ;;
;;            Pointer to source            1 DECLE                          ;;
;;            Length of source array       1 DECLE                          ;;
;;                                                                          ;;
;;  INPUTS for MEMUNPK.1                                                    ;;
;;      R5    Return address                                                ;;
;;      R4    Pointer to destination                                        ;;
;;      R1    Pointer to source                                             ;;
;;      R0    Length of source array                                        ;;
;;                                                                          ;;
;;  OUTPUTS                                                                 ;;
;;      R0    $FFFF                                                         ;;
;;      R1    Last 16-bit word copied, byte-swapped.                        ;;
;;      R4    Points one element beyond destination array                   ;;
;;      R5    Points one element beyond source array                        ;;
;;                                                                          ;;
;;  TECHNIQUES                                                              ;;
;;      Unrolled 4x for speed.                                              ;;
;;      Special dispatch handles length % 4 != 0.                           ;;
;;                                                                          ;;
;;  CODESIZE                                                                ;;
;;      ?? words                                                            ;;
;;                                                                          ;;
;;  CYCLES                                                                  ;;
;;      Not yet characterized.                                              ;;
;; ======================================================================== ;;
MEMUNPK     PROC

            MVI@    R5,     R4      ;   8   Destination array
            MVI@    R5,     R1      ;   8   Source array
            MVI@    R5,     R0      ;   8   Length

@@1:        PSHR    R5              ;   9   Alternate entry point

            MOVR    R1,     R5      ;   6   Use auto-incr pointer

            MOVR    R0,     R1      ;   6   Copy array length
            SLR     R0,     2       ;   8   Find floor(length / 4)
            ANDI    #3,     R1      ;   8   \
            SLL     R1,     2       ;   8    |__ Find address to jump to for
            SUBI    #@@end, R1      ;   8    |   first iteration, to handle
            NEGR    R1              ;   6   /    length % 4.
            MOVR    R1,     PC      ;   6   Jump into copy loop.

@@loop:     MVI@    R5,     R1      ;   8   \
            MVO@    R1,     R4      ;   9    |__ Copy first pair of bytes
            SWAP    R1,     1       ;   6    |
            MVO@    R1,     R4      ;   9   /
            MVI@    R5,     R1      ;   8   \
            MVO@    R1,     R4      ;   9    |__ Copy second pair of bytes
            SWAP    R1,     1       ;   6    |
            MVO@    R1,     R4      ;   9   /
            MVI@    R5,     R1      ;   8   \
            MVO@    R1,     R4      ;   9    |__ Copy third pair of bytes
            SWAP    R1,     1       ;   6    |
            MVO@    R1,     R4      ;   9   /
            MVI@    R5,     R1      ;   8   \
            MVO@    R1,     R4      ;   9    |__ Copy fourth pair of bytes
            SWAP    R1,     1       ;   6    |
            MVO@    R1,     R4      ;   9   /
            
@@end:      DECR    R0              ;   6   
            BPL     @@loop          ;  9/7  Iterate floor(length/4) + 1 times
                                    ;----
                                    ; 143*k - 2

@@done:     PULR    PC              ;  11   Return
            ENDP    


;; ======================================================================== ;;
;;  End of File:  memunpk.asm                                               ;;
;; ======================================================================== ;;
