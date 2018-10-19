;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;; ======================================================================== ;;
;;  WAITKEY     Wait for release/press/release                              ;;
;;  WAITNOKEY   Wait for key release                                        ;;
;; ======================================================================== ;;
WAITKEY     PROC
            PSHR    R5
@@1:        CALL    WAITNOKEY   ; Wait for release-press-release
@@wk        MVI     $1FE,   R0
            XOR     $1FF,   R0
            BEQ     @@wk
            INCR    PC          ; skip PSHR R5 
WAITNOKEY   PSHR    R5

@@waitnokey MVII    #200,   R1  ; Debounce counter

@@stillnk   MVI     $1FE,   R0
            XOR     $1FF,   R0
            BNEQ    @@waitnokey

            DECR    R1          ; Avoid glitches:  Make sure it's good and gone
            BPL     @@stillnk

            PULR    PC
            ENDP
        
;; ======================================================================== ;;
;;  End of File:  wnk.asm                                                   ;;
;; ======================================================================== ;;
