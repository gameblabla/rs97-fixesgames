;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;; ======================================================================== ;;
;;  MEMSET    Fill array with value                                         ;;
;;  MEMSET.1  Alternate entry point                                         ;;
;;                                                                          ;;
;;  AUTHOR                                                                  ;;
;;      Joseph Zbiciak <intvnut AT gmail.com>                               ;;
;;                                                                          ;;
;;  REVISION HISTORY                                                        ;;
;;      08-Sep-2001 Initial Revision                                        ;;
;;                                                                          ;;
;;  INPUTS for MEMSET                                                       ;;
;;      R5    Pointer to invocation record, followed by return address.     ;;
;;            Pointer to destination       1 DECLE                          ;;
;;            Value to fill with           1 DECLE                          ;;
;;            Length                       1 DECLE                          ;;
;;                                                                          ;;
;;  INPUTS for MEMSET.1                                                     ;;
;;      R5    Return address                                                ;;
;;      R4    Pointer to destination                                        ;;
;;      R1    Value to fill with                                            ;;
;;      R0    Length                                                        ;;
;;                                                                          ;;
;;  OUTPUTS                                                                 ;;
;;      R0    Zeroed                                                        ;;
;;      R1    Fill value (unmodified)                                       ;;
;;      R4    Points one element beyond destination array                   ;;
;;      R5    Zeroed                                                        ;;
;;                                                                          ;;
;;  TECHNIQUES                                                              ;;
;;      Unrolled 8x for speed.                                              ;;
;;      Not-unrolled loop handles length % 8 != 0.                          ;;
;;                                                                          ;;
;;  CODESIZE                                                                ;;
;;      29 words                                                            ;;
;;                                                                          ;;
;;  CYCLES                                                                  ;;
;;      Not yet characterized.                                              ;;
;; ======================================================================== ;;
MEMSET      PROC

            MVI@    R5,     R4      ;   8   Destination array
            MVI@    R5,     R1      ;   8   Fill value       
            MVI@    R5,     R0      ;   8   Length

@@1:        PSHR    R5              ;   9   Alternate entry point

            MOVR    R0,     R5      ;   6   \
            ANDI    #7,     R5      ;   8    |-- Handle length % 8 iters,
            BEQ     @@l8_init       ;  7/9  /    if there are any.
                                    ;----
                                    ;  54   Fallthru case
                                    ;  56   Branch taken    

@@loop_1:   MVO@    R1,     R4      ;   9   \
            DECR    R5              ;   6    |-- Store one value at a time.
            BNEQ    @@loop_1        ;  9/7  /
                                    ;----
                                    ;  24*k - 2

@@l8_init:  SLR     R0,     2       ;   8   \
            SLR     R0,     1       ;   6    |-- Divide trip count by 8.
            BEQ     @@done          ;  7/9  /    Abort if it goes to 0.
                                    ;----
                                    ;  21   Fallthru case

@@loop_8:   MVO@    R1,     R4      ;   9   \
            MVO@    R1,     R4      ;   9    |__ Store four elements
            MVO@    R1,     R4      ;   9    |
            MVO@    R1,     R4      ;   9   /
            DECR    R0              ;   6   (Interruptible)
            MVO@    R1,     R4      ;   9   \
            MVO@    R1,     R4      ;   9    |__ Store four more elements
            MVO@    R1,     R4      ;   9    |
            MVO@    R1,     R4      ;   9   /
            BNEQ    @@loop_8        ;  9/7  Iterate length/8 times
                                    ;----
                                    ;  87*k - 2

@@done:     PULR    PC              ;  11   Return
            ENDP    


;; ======================================================================== ;;
;;  End of File:  memset.asm                                                ;;
;; ======================================================================== ;;
