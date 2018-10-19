;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;;==========================================================================;;
;;  NOTE:  IF YOU NEED ONLY HEX16, THAT IS CONTAINED IN A SEPARATE FILE     ;;
;;  CALLED "hex16.asm".  THIS FILE AND THAT ONE CANNOT BE USED TOGETHER.    ;;
;;==========================================================================;;

;; ======================================================================== ;;
;;  HEX16  -- Display a 4-digit hex number on the screen                    ;;
;;  HEX12M -- Display a 3-digit hex number on the screen (MSBs)             ;;
;;  HEX8M  -- Display a 2-digit hex number on the screen (MSBs)             ;;
;;  HEX4M  -- Display a 1-digit hex number on the screen (MSBs)             ;;
;;  HEX12  -- Display a 3-digit hex number on the screen (LSBs)             ;;
;;  HEX8   -- Display a 2-digit hex number on the screen (LSBs)             ;;
;;  HEX4   -- Display a 1-digit hex number on the screen (LSBs)             ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Hex number                                                    ;;
;;      R1 -- Color mask                                                    ;;
;;      R4 -- Screen offset                                                 ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0 -- trashed                                                       ;;
;;      R1 -- unmodified                                                    ;;
;;      R2 -- trashed                                                       ;;
;;      R3 -- zeroed                                                        ;;
;;      R4 -- points just to right of string                                ;;
;; ======================================================================== ;;
HEX16   PROC
        MVII    #4,     R3
        B       @@start
HEX12: 
        SLL     R0,     2
        SLL     R0,     2
HEX12M:
        MVII    #3,     R3
        B       @@start
HEX8:
        SWAP    R0
HEX8M:
        MVII    #2,     R3
        B       @@start
HEX4:
        SWAP    R0
        SLL     R0,     2
        SLL     R0,     2
HEX4M:
        MVII    #1,     R3

@@start:
        ; Rotate R0 left by 3, so that our digit will be in the correct
        ; position within the screen format word.
        MOVR    R0,     R2
        SLLC    R2,     2
        RLC     R0,     2       ; First, rotate by two bits...
        SLLC    R2,     1
        RLC     R0,     1       ; ... and then by one more.
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
        ADDI    #$38,   R2      ; If the digit >= A, add 6 << 3.
@@digit:
        ADDI    #$80,   R2      ; Generate proper GROM index.     
        XORR    R1,     R2      ; Merge in the screen format word 
        MVO@    R2,     R4      ; Display the digit to the screen.

        DECR    R3              ; Iterate up to three more times.
        BNE     @@loop

        JR      R5
        ENDP

;; ======================================================================== ;;
;;  End of File:  hexdisp.asm                                               ;;
;; ======================================================================== ;;
