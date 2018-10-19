;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;; ======================================================================== ;;
;;  FILLZERO                                                                ;;
;;      Fills memory with zeros                                             ;;
;;                                                                          ;;
;;  FILLMEM                                                                 ;;
;;      Fills memory with a constant                                        ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Fill value (FILLMEM only)                                     ;;
;;      R1 -- Number of words to fill                                       ;;
;;      R4 -- Start of fill area                                            ;;
;;      R5 -- Return address                                                ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0 -- Zeroed if FILLZERO, otherwise untouched.                      ;;
;;      R1 -- Zeroed                                                        ;;
;;      R4 -- Points to word after fill area                                ;;
;; ======================================================================== ;;
CLRSCR      PROC
            MVII    #$0F0,  R1
            MVII    #$200,  R4
FILLZERO    CLRR    R0              ; Start out with R0 zeroed for FILLZERO
FILLMEM     MVO@    R0,     R4      ; Store R0 out at R4, and move along
            DECR    R1              ; Keep going until our count runs out
            BNEQ    FILLMEM
            JR      R5              ; Return to the caller.
            ENDP

;; ======================================================================== ;;
;;  End of File:  fillzero.asm                                              ;;
;; ======================================================================== ;;
