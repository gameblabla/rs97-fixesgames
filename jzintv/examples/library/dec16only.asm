;;==========================================================================;;
;;  NOTE:  THESE ROUTINES HAVE BEEN SUPERCEDED BY PRNUM16/PRNUM32!          ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;; ======================================================================== ;;
;;  GLOBAL VARIABLES USED BY THESE ROUTINES                                 ;;
;;                                                                          ;;
;;  Note that some of these routines may use one or more global variables.  ;;
;;  If you use these routines, you will need to allocate the appropriate    ;;
;;  space in either 16-bit or 8-bit memory as appropriate.  Each global     ;;
;;  variable is listed with the routines which use it and the required      ;;
;;  memory width.                                                           ;;
;;                                                                          ;;
;;  Example declarations for these routines are shown below, commented out. ;;
;;  You should uncomment these and add them to your program to make use of  ;;
;;  the routine that needs them.  Make sure to assign these variables to    ;;
;;  locations that aren't used for anything else.                           ;;
;; ======================================================================== ;;

                        ; Used by       Req'd Width     Description
                        ;-----------------------------------------------------
;DEC_0  EQU     $110    ; DEC16         8-bit           Temp. storage
;DEC_1  EQU     $111    ; DEC16         8-bit           Temp. storage


;; ======================================================================== ;;
;;  POW10                                                                   ;;
;;      Look-up table with powers of 10 as 32-bit numbers (little endian).  ;;
;; ======================================================================== ;;

POW10   PROC
@@5     WORD   100000     AND $FFFF, 100000     SHR 16  ; 10**5
@@4     WORD   10000      AND $FFFF, 10000      SHR 16  ; 10**4
@@3     WORD   1000       AND $FFFF, 1000       SHR 16  ; 10**3
@@2     WORD   100        AND $FFFF, 100        SHR 16  ; 10**2
@@1     WORD   10         AND $FFFF, 10         SHR 16  ; 10**1
@@0     WORD   1          AND $FFFF, 1          SHR 16  ; 10**0
        ENDP


;; ======================================================================== ;;
;;  DEC16                                                                   ;;
;;      Displays a 16-bit decimal number on the screen with leading blanks  ;;
;;      in a field up to 5 characters wide.  Displays all blanks if the     ;;
;;      number is zero.                                                     ;;
;;                                                                          ;;
;;  DEC16A                                                                  ;;
;;      Same as DEC16, only displays leading zeroes.                        ;;
;;                                                                          ;;
;;  DEC16B                                                                  ;;
;;      Same as DEC16, only leading zeros are controlled by bit 15 of R3.   ;;
;;      (If set, suppress leading zeros.  If clear, show leading zeros.)    ;;
;;                                                                          ;;
;;  DEC16C                                                                  ;;
;;      Same as DEC16B, except R1 contains an amount to add to the first    ;;
;;      digit.                                                              ;;
;;                                                                          ;;
;;  DEC16Z                                                                  ;;
;;      Same as DEC16, except displays a single zero if the value is zero.  ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Number to be displayed in decimal.                            ;;
;;      R1 -- (DEC16C only): Amount to add to initial digit.                ;;
;;      R2 -- Number of digits to suppress  (If R2<=5, it is 5-field_width) ;;
;;      R3 -- Color mask / screen format word (XORd with char index)        ;;
;;      R4 -- Screen offset (lower 8-bits)                                  ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0 -- Zeroed                                                        ;;
;;      R1 -- Trashed                                                       ;;
;;      R2 -- Remaining digits to suppress (0 if initially <= 5.)           ;;
;;      R3 -- Color mask, with bit 15 set if no digits displayed.           ;;
;;      R4 -- Pointer to character just right of string                     ;;
;;      R5 -- Trashed                                                       ;;
;;                                                                          ;;
;;  NOTES:                                                                  ;;
;;      Routine uses DEC_0, DEC_1 for temporary storage.  This routine      ;;
;;      is NOT REENTRANT, so be careful calling this routine from an        ;;
;;      an interrupt handler if you also call either from outside an        ;;
;;      interrupt handler as well.                                          ;;
;;                                                                          ;;
;;      To make this code re-entrant, save the contents of DEC_0 and        ;;
;;      DEC_1 before calling any variant of DEC16 from within an interrupt  ;;
;;      handler and then restore DEC_0 and DEC_1 afterwards.                ;;
;; ======================================================================== ;;
DEC16   PROC
@@so    EQU     DEC_0
@@fw    EQU     DEC_1

        SETC                    ; Prepare to set bit 15 of color mask
        INCR    R7              ; Skip the CLRC
DEC16A
        CLRC                    ; Prepare to clear bit 15 of clrmask
        SLL     R3,     1
        RRC     R3,     1       ; Set/clear bit 15 of color mask
DEC16B
        CLRR    R1
DEC16C
        PSHR    R5              ; Save return address
        MOVR    PC,     R5      ; (generate PC-relative address)
        SUBI    #$-POW10.4, R5  ; Point to '10000' entry in POW10
        MVO     R4,     @@so    ; Save screen offset 
        MVO     R2,     @@fw    ; Save field width
        MVII    #5,     R4      ; Iterate 5 (16-bit goes to 65536)
        MOVR    R1,     R2
        INCR    R7
@@digitlp:
        
        CLRR    R2              ; Start with division result == 0
        SDBD
        MVI@    R5,     R1      ; Load power of 10
        ADDI    #2,     R5      ; Point to next smaller power of 10
@@divloop:
        INCR    R2
        SUBR    R1,     R0      ; Divide by repeated subtraction
        BC      @@divloop
        ADDR    R1,     R0      ; Loop iterates 1 extra time: Fix it.
        DECR    R2              ; Fix extra iter.  Also test if 0
        BNEQ    @@disp          ; If digit != 0, display it.
        TSTR    R3              ; If digit == 0 and no lead 0, skip
        BMI     @@blank
@@disp:
        SLL     R3,     1       ; Clear "no leading 0" flag
        SLR     R3,     1       ; 
        MVI     @@fw,   R1      ; Get field width 
        DECR    R1              ; Are we in active field yet?
        BMI     @@ok            ; Yes: Go ahead and display
        MVO     R1,     @@fw    ; No: Save our count-down till field
        B       @@iter          ;     and don't display the digit.
@@blank:
        MOVR    R3,     R2      ; Blank character _just_ gets format
        MVI     @@fw,   R1      ; Get field width 
        DECR    R1              ; Are we in active field yet?
        BMI     @@drawit        ; Yes: Go ahead and display
        MVO     R1,     @@fw    ; No: Save our count-down till field
        B       @@iter          ;     and don't display the digit.
@@ok:
        ADDI    #$10,   R2      ; Pseudo-ASCII digits start at 0x10
        SLL     R2,     2       ; Put pseudo-ASCII char in position
        ADDR    R2,     R2      ; ... by shifting left 3
        XORR    R3,     R2      ; Merge with display format
@@drawit:
        MVI     @@so,   R1      ; Get screen offset
        XORI    #$200,  R1      ; Move to screen
        MVO@    R2,     R1      ; Put character on screen
        INCR    R1              ; Move the pointer
        MVO     R1,     @@so    ; Save the new offset
@@iter:
        DECR    R4              ; Count down our digit count
        BNEQ    @@digitlp       ; Keep iterating

        MVI     @@so,   R4      ; Restore offset
        MVI     @@fw,   R2      ; Restore digit suppress ocunt

        PULR    PC              ; Whew!  Done!
        ENDP

;; ======================================================================== ;;
;;  DEC16Z                                                                  ;;
;;      Same as DEC16, except a zero is displayed in the final position if  ;;
;;      the whole number's value is zero.                                   ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Lower 16-bits of number                                       ;;
;;      R2 -- Number of leading digits to suppress                          ;;
;;            (5 - field_width for DEC16Z).                                 ;;
;;      R3 -- Screen format word                                            ;;
;;      R4 -- Screen offset (lower 8-bits)                                  ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0 -- Zeroed                                                        ;;
;;      R1 -- Trashed                                                       ;;
;;      R2 -- If number == 0, unchanged.                                    ;;
;;            If number != 0, remaining digits to suppress.                 ;;
;;      R3 -- Color mask, unmodified.                                       ;;
;;      R4 -- Pointer to character just right of string                     ;;
;;      R5 -- Trashed                                                       ;;
;;                                                                          ;;
;;  NOTES:                                                                  ;;
;;       DEC16Z uses DEC_0..DEC_1 for temporary storage.                    ;;
;;       DEC16 variants are not re-entrant.  See DEC16 above for details.   ;;
;; ======================================================================== ;;
DEC16Z  PROC
        TSTR    R0              ; Is the number non-zero?
        BNEQ    DEC16           ; Yes:  Call DEC16
        MVII    #5,     R1      ; No:  Prepare to clear the field and draw 0.

@@dozero:
        SUBR    R2,     R1      ; Is our field wide enough to display the 0?
        BLE     @@nodisp        ; No:  Don't display it then.
        ADDI    #$200,  R4      ; Yes:  Calculate our screen pointer.
        INCR    R7              ; (skip first iteration of loop)
@@loop:
        MVO@    R3,     R4      ; Clear the leading digits.
        DECR    R1
        BNEQ    @@loop

        XORI    #$80,   R3      ; Now display a zero.
        MVO@    R3,     R4
        XORI    #$80,   R3      ; Leave R3 unchanged.

@@nodisp:
        JR      R5              ; Return to the caller.
        ENDP

;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;
