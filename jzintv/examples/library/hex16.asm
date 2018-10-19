;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;;==========================================================================;;
;;  NOTE:  IF YOU NEED ANY OF HEX4, HEX8, HEX12, HEX4M, HEX8M or HEX12M,    ;;
;;  DO NOT USE THIS FILE.  USE "hexdisp.asm" INSTEAD.  THIS FILE AND THAT   ;;
;;  ONE CANNOT BE USED TOGETHER.                                            ;;
;;==========================================================================;;

;; ======================================================================== ;;
;;  HEX16                                                                   ;;
;;      Display a 4-digit hex number on the screen                          ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Hex number                                                    ;;
;;      R1 -- Color mask / screen format word                               ;;
;;      R4 -- Screen offset                                                 ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0 -- rotated left by 3                                             ;;
;;      R1 -- unmodified                                                    ;;
;;      R2 -- trashed                                                       ;;
;;      R3 -- zeroed                                                        ;;
;;      R4 -- points just to right of string                                ;;
;; ======================================================================== ;;
HEX16           PROC
        ; Rotate R0 left by 3, so that our digit will be in the correct
        ; position within the screen format word.
        MOVR    R0,     R3
        SLLC    R3,     2
        RLC     R0,     2       ; First, rotate by two bits...
        SLLC    R3,     1
        RLC     R0,     1       ; ... and then by one more.

        MVII    #4,     R3      ; Iterate through four digits.
@@loop:
        ; Rotate R0 left by 4, so that we can cycle through each digit
        ; one at a time.
        MOVR    R0,     R2
        SLLC    R2,     2
        RLC     R0,     2       ; First, rotate by two bits...
        SLLC    R2,     2
        RLC     R0,     2       ; ... and then by two more.

        ; Mask out a single hex digit
        MOVR    R0,     R2
        ANDI    #$78,   R2

        ; Is it A..F?  If so, add an offset so that the correct ASCII
        ; value is selected.  Otherwise do nothing special.
        CMPI    #$50,   R2      ; $50 is $A shifted left by 3.
        BLT     @@digit
        ADDI    #$38,   R2      ; If the digit >= A, add 7 << 3.
@@digit:
        ADDI    #$80,   R2      ; Generate proper GROM index.
        XORR    R1,     R2      ; Merge in the screen format word
        MVO@    R2,     R4      ; Display the digit to the screen.

        DECR    R3              ; Iterate three more times.
        BNE     @@loop

        JR      R5              ; Done!  Return.
        ENDP

;; ======================================================================== ;;
;;  End of File:  hex16.asm                                                 ;;
;; ======================================================================== ;;
