;;==========================================================================;;
;;  Note:  These routines have been superceded by PRNUM16/PRNUM32!          ;;
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
;DEC_0  EQU     $110    ; DEC16/DEC32   8-bit           Temp. storage
;DEC_1  EQU     $111    ; DEC16/DEC32   8-bit           Temp. storage
;DEC_2  EQU     $112    ; DEC32         8-bit           Temp. storage
;DEC_3  EQU     DEC_2+1 ; DEC32         8-bit           Must be next to DEC_2


;; ======================================================================== ;;
;;  POW10                                                                   ;;
;;      Look-up table with powers of 10 as 32-bit numbers (little endian).  ;;
;;                                                                          ;;
;;  NPW10                                                                   ;;
;;      Same as POW10, only -(10**x) instead of 10**x.                      ;;
;; ======================================================================== ;;

POW10   PROC
@@9     WORD   1000000000 AND $FFFF, 1000000000 SHR 16  ; 10**9
@@8     WORD   100000000  AND $FFFF, 100000000  SHR 16  ; 10**8
@@7     WORD   10000000   AND $FFFF, 10000000   SHR 16  ; 10**7 
@@6     WORD   1000000    AND $FFFF, 1000000    SHR 16  ; 10**6
@@5     WORD   100000     AND $FFFF, 100000     SHR 16  ; 10**5
@@4     WORD   10000      AND $FFFF, 10000      SHR 16  ; 10**4
@@3     WORD   1000       AND $FFFF, 1000       SHR 16  ; 10**3
@@2     WORD   100        AND $FFFF, 100        SHR 16  ; 10**2
@@1     WORD   10         AND $FFFF, 10         SHR 16  ; 10**1
@@0     WORD   1          AND $FFFF, 1          SHR 16  ; 10**0
        ENDP

NPW10   PROC
@@9     WORD  -1000000000 AND $FFFF,-1000000000 SHR 16  ;-10**9
@@8     WORD  -100000000  AND $FFFF,-100000000  SHR 16  ;-10**8
@@7     WORD  -10000000   AND $FFFF,-10000000   SHR 16  ;-10**7 
@@6     WORD  -1000000    AND $FFFF,-1000000    SHR 16  ;-10**6
@@5     WORD  -100000     AND $FFFF,-100000     SHR 16  ;-10**5
@@4     WORD  -10000      AND $FFFF,-10000      SHR 16  ;-10**4
@@3     WORD  -1000       AND $FFFF,-1000       SHR 16  ;-10**3
@@2     WORD  -100        AND $FFFF,-100        SHR 16  ;-10**2
@@1     WORD  -10         AND $FFFF,-10         SHR 16  ;-10**1
@@0     WORD  -1          AND $FFFF,-1          SHR 16  ;-10**0
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
;;      (Note:  DEC16Z is actually defined separately along with DEC32Z).   ;;
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
;;      is NOT REENTRANT, so be careful calling this routine or DEC32 from  ;;
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
;;  DEC32                                                                   ;;
;;      Displays a 32 bit number without leading zeros.  It performs this   ;;
;;      feat by calling DEC16 multiple times.                               ;;
;;                                                                          ;;
;;  DEC32A                                                                  ;;
;;      Same as DEC32, except leading zeros are displayed.                  ;;
;;                                                                          ;;
;;  DEC32B                                                                  ;;
;;      Same as DEC32, except leading zeros are controlled by bit 15 of R3  ;;
;;      (If set, suppress leading zeros.  If clear, show leading zeros.)    ;;
;;                                                                          ;;
;;  DEC32Z                                                                  ;;
;;      Same as DEC32, except displays a single zero if the value is zero.  ;;
;;      (Note:  DEC32Z is actually defined separately along with DEC16Z).   ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Low half of 32-bit number                                     ;;
;;      R1 -- High half of 32-bit number                                    ;;
;;      R2 -- Number of leading digits to suppress (10 - field width)       ;;
;;      R3 -- Screen format word                                            ;;
;;      R4 -- Screen offset (lower 8-bits)                                  ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0 -- Zeroed                                                        ;;
;;      R1 -- Trashed                                                       ;;
;;      R2 -- Remaining digits to suppress (0 if initially <= 10.)          ;;
;;      R3 -- Color mask, with bit 15 set if no digits displayed.           ;;
;;      R4 -- Pointer to character just right of string                     ;;
;;      R5 -- Trashed                                                       ;;
;;                                                                          ;;
;;  NOTES:                                                                  ;;
;;      Routine uses DEC_0..DEC_3 for temporary storage.  This routine      ;;
;;      is NOT REENTRANT, so be careful calling this routine or DEC16 from  ;;
;;      an interrupt handler if you also call either from outside an        ;;
;;      interrupt handler as well.                                          ;;
;;                                                                          ;;
;;      To make this code re-entrant, save the contents of DEC_0 through    ;;
;;      DEC_3 before calling any variant of DEC32 from within an interrupt  ;;
;;      handler and then restore DEC_0 through DEC_3 afterwards.            ;;
;; ======================================================================== ;;
DEC32   PROC
@@so    EQU     DEC_0
@@fw    EQU     DEC_1
@@fmt   EQU     DEC_2 ; and DEC_3.  We store 16 bits here.

        SETC                    ; Prepare to set bit 15 of color mask
        INCR    R7              ; Skip the CLRC
DEC32A
        CLRC                    ; Prepare to clear bit 15 of clrmask
        SLL     R3,     1
        RRC     R3,     1       ; Set/clear bit 15 of color mask
DEC32B
        PSHR    R5              ; Save return address
        MVO     R2,     @@fw    ; Save field width
        MVO     R4,     @@so    ; Save screen offset
        MVO     R3,     @@fmt
        SWAP    R3
        MVO     R3,     @@fmt+1

        CLRR    R3
        PSHR    R3              ; Push accumulator (init'd to 0)

        ; Use division by repeated subtraction to generate a 16-bit
        ; value which represents the first 5 digits of the 10 digit number.
        MOVR    PC,     R5      ; (generate PC-relative address)
        SUBI    #$-NPW10.9, R5  ; Point to -10**9

@@digitlp:
        CLRR    R3
        SDBD
        MVI@    R5,     R2      ; Load low half of 32-bit -10**x
        SDBD
        MVI@    R5,     R4      ; Load high half of 32-bit -10**x
@@divlp:
        SLR     R3,     1
@@divlpb:
        INCR    R3
        ADDR    R2,     R0      ; Add the low half
        ADCR    R1              ; Add carry from low half
        RLC     R3,     1       ; See if adding the carry carried
        ADDR    R4,     R1      ; Add high half
        BC      @@divlp         ; Loop if we had a carry from either
        SARC    R3,     1       ;   upper half ADD.  (We can't get
        BC      @@divlpb        ;   a carry from both, though.)

        ; Subtract off the extra iteration
        SUBR    R2,     R0      ; Subtract the low half.
        ADCR    R1              ; Add in the "not-borrow"
        DECR    R1              ; Turn "not-borrow" into "borrow"
        SUBR    R4,     R1      ; Subtract the high half.

        DECR    R3
        BEQ     @@nxtdigit

        ; Take our count and multiply it by the appropriate power of 10.
        MOVR    R5,     R2
        MOVR    R3,     R4
        SUBR    PC,     R2
        ADDI    #$-NPW10.4, R2  ; Translate 10**x to 10**(x-5)
        BEQ     @@donemult
@@mult:
        ADDR    R4,     R4      ; To mult by 10, do (x<<1)+(x<<3)
        MOVR    R4,     R3
        SLL     R3,     2
        ADDR    R3,     R4
        ADDI    #$4,    R2
        BLT     @@mult
@@donemult:
        ADD@    SP,     R4      ; Add this to our 16-bit accum. 
        PSHR    R4              ; that we keep on top-of-stack
@@nxtdigit:
        CMPI    #NPW10.4, R5
        BLT     @@digitlp

        MVI     @@fw,   R2      ; Restore field width
        MVI     @@so,   R4      ; Restore screen offset
        MVI     @@fmt+1,R3      ; Restore fmt word
        SWAP    R3,     1       ; ...
        XOR     @@fmt,  R3      ; ...
        MVO     R0,     @@fmt   ; Save low byte of lower 16 bits 
        SWAP    R0,     1       ; ...
        MVO     R0,     @@fmt+1 ; Save high byte of lower 16 bits
        PULR    R0              ; Get accumulated word for display
        PSHR    R1              ; Save upper bit
        CALL    DEC16B          ; Display first five digits

        ; Now, our 32-bit number should be less than 100000.  That
        ; means R1 should be 0 or 1.  We display the last five digits
        ; as a single 16-bit number by handling that bit separately.

        MVI     @@fmt+1,R0      ; Restore lower 16 bits
        SWAP    R0,     1       ; ...
        XOR     @@fmt,  R0      ; ...

        PULR    R1              ; Get upper bit
        TSTR    R1              ; Was it zero?
        BEQ     @@noextra       ; Yes:  Nothing special to do
        MVII    #6,     R1      ; No: Add 6 to the leading digit
        ADDI    #5536,  R0      ; ... and "5536" to remaining digits
@@noextra:
        PULR    R5              ; Chain the return.
        B       DEC16C          ; Display remaining digits.  WHEW!
        ENDP

;; ======================================================================== ;;
;;  DEC32Z                                                                  ;;
;;      Same as DEC32, except a zero is displayed in the final position if  ;;
;;      the whole number's value is zero.                                   ;;
;;                                                                          ;;
;;  DEC16Z                                                                  ;;
;;      Same as DEC16, except a zero is displayed in the final position if  ;;
;;      the whole number's value is zero.                                   ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Lower 16-bits of number                                       ;;
;;      R1 -- Upper 16-bits of number (if DEC32Z)                           ;;
;;      R2 -- Number of leading digits to suppress                          ;;
;;            (10 - field width for DEC32Z, 5 - field width for DEC16Z).    ;;
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
;;       DEC32Z uses DEC_0..DEC_3 for temporary storage.                    ;;
;;       Neither routine is re-entrant.  See DEC16 and DEC32 above for      ;;
;;       more details.                                                      ;;
;; ======================================================================== ;;
DEC32Z  PROC
        TSTR    R1              ; Is upper half non-zero?
        BNEQ    DEC32           ; Yes:  Call DEC32.
        TSTR    R0              ; Is lower half non-zero?
        BNEQ    DEC32           ; Yes:  Call DEC32
        MVII    #10,    R1      ; No:  Prepare to clear field and draw the 
        B       @@dozero        ;      zero.
DEC16Z:
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
