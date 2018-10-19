;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;; ======================================================================== ;;
;;  MEMCPY    Copy one array to another                                     ;;
;;  MEMCPY.1  Alternate entry point                                         ;;
;;                                                                          ;;
;;  AUTHOR                                                                  ;;
;;      Joseph Zbiciak <intvnut AT gmail.com>                               ;;
;;                                                                          ;;
;;  REVISION HISTORY                                                        ;;
;;      08-Sep-2001 Initial Revision                                        ;;
;;                                                                          ;;
;;  INPUTS for MEMCPY                                                       ;;
;;      R5    Pointer to invocation record, followed by return address.     ;;
;;            Pointer to destination       1 DECLE                          ;;
;;            Pointer to source            1 DECLE                          ;;
;;            Length                       1 DECLE                          ;;
;;                                                                          ;;
;;  INPUTS for MEMCPY.1                                                     ;;
;;      R5    Return address                                                ;;
;;      R4    Pointer to destination                                        ;;
;;      R1    Pointer to source                                             ;;
;;      R0    Length                                                        ;;
;;                                                                          ;;
;;  OUTPUTS                                                                 ;;
;;      R0    $FFFF                                                         ;;
;;      R1    Last element copied                                           ;;
;;      R4    Points one element beyond destination array                   ;;
;;      R5    Points one element beyond source array                        ;;
;;                                                                          ;;
;;  TECHNIQUES                                                              ;;
;;      Unrolled 8x for speed.                                              ;;
;;      Special dispatch handles length % 8 != 0.                           ;;
;;                                                                          ;;
;;  CODESIZE                                                                ;;
;;      35 words                                                            ;;
;;                                                                          ;;
;;  CYCLES                                                                  ;;
;;      Not yet characterized.                                              ;;
;; ======================================================================== ;;
MEMCPY      PROC

            MVI@    R5,     R4      ;   8   Destination array
            MVI@    R5,     R1      ;   8   Source array
            MVI@    R5,     R0      ;   8   Length

@@1:        PSHR    R5              ;   9   Alternate entry point

            MOVR    R1,     R5      ;   6   Use auto-incr pointer

            MOVR    R0,     R1      ;   6   Copy array length
            SLR     R0,     2       ;   8   \___ Find floor(length / 8)
            SLR     R0,     1       ;   6   /
            ANDI    #7,     R1      ;   8   \
            ADDR    R1,     R1      ;   6    |__ Find address to jump to for
            SUBI    #@@end, R1      ;   8    |   first iteration, to handle
            NEGR    R1              ;   6   /    length % 8.
            MOVR    R1,     PC      ;   6   Jump into copy loop.


@@loop:     MVI@    R5,     R1      ;   8   
            MVO@    R1,     R4      ;   9   
            MVI@    R5,     R1      ;   8   
            MVO@    R1,     R4      ;   9   
            MVI@    R5,     R1      ;   8   
            MVO@    R1,     R4      ;   9   
            MVI@    R5,     R1      ;   8   
            MVO@    R1,     R4      ;   9   
            MVI@    R5,     R1      ;   8   
            MVO@    R1,     R4      ;   9   
            MVI@    R5,     R1      ;   8   
            MVO@    R1,     R4      ;   9   
            MVI@    R5,     R1      ;   8   
            MVO@    R1,     R4      ;   9   
            MVI@    R5,     R1      ;   8   
            MVO@    R1,     R4      ;   9   
            
@@end:      DECR    R0              ;   6   
            BPL     @@loop          ;  9/7  Iterate floor(length/8) + 1 times
                                    ;----
                                    ; 151*k - 2

@@done:     PULR    PC              ;  11   Return
            ENDP    


;; ======================================================================== ;;
;;  End of File:  memcpy.asm                                                ;;
;; ======================================================================== ;;
