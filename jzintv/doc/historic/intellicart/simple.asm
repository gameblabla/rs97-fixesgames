;;==========================================================================;;
;; INTV RAM Tests.  Performs tests on the Intellicart's Bank Switch modes.  ;;
;; Copyright 2000, Joe Zbiciak, im14u2c@primenet.com.                       ;;
;; http://www.primenet.com/~im14u2c/intv/                                   ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  This program is free software; you can redistribute it and/or modify    *;
;*  it under the terms of the GNU General Public License as published by    *;
;*  the Free Software Foundation; either version 2 of the License, or       *;
;*  (at your option) any later version.                                     *;
;*                                                                          *;
;*  This program is distributed in the hope that it will be useful,         *;
;*  but WITHOUT ANY WARRANTY; without even the implied warranty of          *;
;*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *;
;*  General Public License for more details.                                *;
;*                                                                          *;
;*  You should have received a copy of the GNU General Public License       *;
;*  along with this program; if not, write to the Free Software             *;
;*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.               *;
;* ======================================================================== *;
;*                   Copyright (c) 2000, Joseph Zbiciak                     *;
;* ======================================================================== *;

        ROMW    16
        ORG     $5000

; ------------------------------------------------------------------------- ;
;  Variables.  We hide the 16-bit variables under the stack.                ;
; ------------------------------------------------------------------------- ;
                        ; Used by       Req'd Width   Description
                        ;---------------------------------------------------
RNDLO   EQU     $2F0    ; RAND          16-bit        Random number state
RNDHI   EQU     $2F1    ; RAND          16-bit        Random number state
SAVLO   EQU     $2F2    ; RANDSAVE/RSTR 16-bit        Saves rand num. seed
SAVHI   EQU     $2F3    ; RANDSAVE/RSTR 16-bit        Saves rand num. seed
BUSY    EQU     $2F4    ; ISR/WAIT      16-bit        Busy-wait counter
WPAGE   EQU     $2F5    ; BANKTEST      16-bit        Current writeout page
RPAGE   EQU     $2F6    ; BANKTEST      16-bit        Current readback page
DEC_0   EQU     $110    ; DEC16/DEC32   8-bit         Temp. storage
DEC_1   EQU     $111    ; DEC16/DEC32   8-bit         Temp. storage
DEC_2   EQU     $112    ; DEC32         8-bit         Temp. storage
DEC_3   EQU     DEC_2+1 ; DEC32         8-bit         Must be adj. to DEC_2
TBLNK   EQU     $132    ; ISR           8-bit         Blink counter for timer
T_DAY   EQU     $133    ; ISR           8-bit         Day counter
T_HRS   EQU     $134    ; ISR           8-bit         Hour counter
T_MIN   EQU     $135    ; ISR           8-bit         Minute counter
T_SEC   EQU     $136    ; ISR           8-bit         Second counter
T_CLN   EQU     $137    ; ISR           8-bit         Colons on or off?
PGTMP   EQU     $138    ; BANKTEST      8-bit         Page # temporary
MPTMP   EQU     $139    ; BANKTEST      8-bit         Remapping temporary
BOS     EQU     $300    ; STACK         16-bit        Bottom of stack.


; ------------------------------------------------------------------------- ;
;  Handy Equates.                                                           ;
; ------------------------------------------------------------------------- ;
ROW_0   EQU     $200
ROW_1   EQU     ROW_0 + 20
ROW_2   EQU     ROW_1 + 20
ROW_3   EQU     ROW_2 + 20
ROW_4   EQU     ROW_3 + 20
ROW_5   EQU     ROW_4 + 20
ROW_6   EQU     ROW_5 + 20
ROW_7   EQU     ROW_6 + 20
ROW_8   EQU     ROW_7 + 20
ROW_9   EQU     ROW_8 + 20
ROW_A   EQU     ROW_9 + 20
ROW_B   EQU     ROW_A + 20

C_BLK   EQU     0               ; Black
C_BLU   EQU     1               ; Blue
C_RED   EQU     2               ; Red
C_TAN   EQU     3               ; Tan
C_DGR   EQU     4               ; Dark Green
C_GRN   EQU     5               ; Green
C_YEL   EQU     6               ; Yellow
C_WHT   EQU     7               ; White

EDLY    EQU     1800            ; Number of ticks to hold error message
PDLY    EQU     30              ; Number of ticks to hold passing message


; ------------------------------------------------------------------------- ;
ROMHDR:
        BYTE    $01,$00         ; Movable object data (ignored)
        WORD    $0000           ; RTAB (ignored)
        WORD    MAIN            ; Program start address
        WORD    $0000           ; Background graphics
        WORD    ROMHDR          ; Card table -- stored above in header. :-)
        WORD    TITLE           ; Title string

        BYTE    $C0             ; run code after title, clicks off
        BYTE    $00             ; -> to STIC $32
        BYTE    $00             ; 0 = color stack, 1 = f/b mode
        BYTE    0, 0, 0, 0      ; color stack elements 1 - 4
        BYTE    $00             ; border color
; ------------------------------------------------------------------------- ;

;;==========================================================================;;
;; BANKTBL   -- Initial bank mapping table.                                 ;;
;;==========================================================================;;
BANKTBL:    DECLE   $6000       ; Map $6000-$67FF to $0000 in Intellicart
            DECLE   $D000       ; Map $D000-$D7FF to $0000 in Intellicart
            DECLE   $0000       ; End of table

;;==========================================================================;;
;; TITLE / TITLEFIX -- The Title Screen code.                               ;;
;;==========================================================================;;
TITLE:  BYTE    100, "I.C. Bank Test", 0 

TITLEFIX:
        BEGIN

        ; Cyan screen
        MVII    #$09,   R0      ; Color 9 is Cyan (pastels.)
        MVO     R0,     $28     ; Color stack 0 = Cyan
        MVO     R0,     $29     ; Color stack 1 = Cyan
        MVO     R0,     $2A     ; Color stack 2 = Cyan
        MVO     R0,     $2B     ; Color stack 3 = Cyan
        MVO     R0,     $2C     ; Border color = Cyan

        ; Patch the title string to say '=JRMZ=' instead of Mattel.
        CLRR    R1              ; Black
        MVII    #$23C,  R4      ; First 'Mattel' in top-left
        CALL    DRAWSTRING2     ; Write string (ptr in R5)
        BYTE    "Zbiciak", 0    ; Guess who?  :-)

        CLRR    R1              ; Black
        MVII    #$2D0,  R4      ; Second 'Mattel' in lower-right
        CALL    DRAWSTRING2     ; Write string (ptr in R1)
        BYTE    "2000 =JRMZ=",0 ; Guess who?  :-)

        ; Make all the rest of the text black to match
        MVII    #$243,  R3      ; Start after first JRMZ
        MVII    #7,     R0      ; Mask = 0xFFF8.  That's the 1s complement...
        COMR    R0              ; ...of 7.  (Saves an SDBD.)
        MVII    #146,   R2      ; We only need to touch 146 words.

@@titlelp:
        MVI@    R3,     R1      ; Read a word from the display
        ANDR    R0,     R1      ; Mask the foreground color
        MVO@    R1,     R3      ; Write the word back
        INCR    R3              ; Point to next word
        DECR    R2              ; Decrement our loop count
        BNEQ    @@titlelp       ; Iterate.

        ; Done.
        RETURN                  ; Return to caller


;;==========================================================================;;
;; MAIN -- main program                                                     ;;
;;==========================================================================;;
MAIN:   PROC

        ; Fill system memory with zeros.
        MVII    #$260,  R1
        MVII    #$100,  R4
        CALL    FILLZERO

        ; Set the ISR
        MVII    #ISR,   R0
        MVO     R0,     $100
        SWAP    R0
        MVO     R0,     $101    ; Set our interrupt service routine to 'ISR'

        MVII    #BOS,   SP      ; Reset our stack.

        CLRR    R0
        MVII    #$0028, R4
        MVO@    R0,     R4      ; Black screen.
        INCR    R0
        MVO@    R0,     R4      ; CS0 == Black
        CLRR    R0
        MVO@    R0,     R4      ; CS1 == Blue
        MVO@    R0,     R4      ; CS2 == Black
        MVO@    R0,     R4      ; CS3 == Black

        MVII    #BANKTBL,   R4  ; Initial bank switch table
        CALL    IC_BANKTBL      ; Load the bank table.

        MVII    #ROW_B, R4
        MVII    #C_BLU, R1
        CALL    DRAWSTRING2
        BYTE    "Elapsed", 0

        EIS
@@test_loop:
        MVII    #$6000, R1
        MVII    #$6000, R2
        CALL    BANKTEST

        MVII    #$6000, R1
        MVII    #$D000, R2
        CALL    BANKTEST

        MVII    #$D000, R1
        MVII    #$6000, R2
        CALL    BANKTEST

        MVII    #$D000, R1
        MVII    #$D000, R2
        CALL    BANKTEST

        B       @@test_loop

        ENDP


;;==========================================================================;;
;; ISR -- The Interrupt Service routine.                                    ;;
;;==========================================================================;;
ISR     PROC
        PSHR    R5

        MVI     BUSY,   R0      ; Decrement the busy-wait counter.
        DECR    R0
        BMI     @@zero
        MVO     R0,     BUSY

@@zero:
        MVI     $21,    R0      ; force color-stack mode
        MVO     R5,     $20     ; enable display

@@done_stat:
        ; Update the timer clock at the bottom.
        MVI     TBLNK,  R0
        DECR    R0
        BMI     @@clock_upd
        MVO     R0,     TBLNK
        CMPI    #30,    R0
        BNEQ    @@no_colon

        MVII    #$D0,   R1
@@do_colon:
        MVII    #ROW_B + 11, R4
        MVO@    R1,     R4
        ADDI    #2,     R4
        MVO@    R1,     R4
        ADDI    #2,     R4
        MVO@    R1,     R4

@@no_colon:
        PULR    PC

@@clock_upd:
        MVII    #60,    R0
        MVO     R0,     TBLNK

        MVI     T_SEC,  R2
        INCR    R2
        MVO     R2,     T_SEC
        CMPI    #60,    R2
        BLT     @@time_ok
        CLRR    R2
        MVO     R2,     T_SEC

        MVI     T_MIN,  R2
        INCR    R2
        MVO     R2,     T_MIN
        CMPI    #60,    R2
        BLT     @@time_ok

        CLRR    R2
        MVO     R2,     T_MIN

        MVI     T_HRS,  R2
        INCR    R2
        MVO     R2,     T_HRS
        CMPI    #24,    R2
        BLT     @@time_ok

        MVI     T_DAY,  R2
        INCR    R2
        MVO     R2,     T_DAY

@@time_ok:

        MVI     T_DAY,  R0
        MVII    #3,     R2
        MVII    #C_BLU, R3
        MVII    #ROW_B + 9, R4
        CALL    DEC16A

        INCR    R4

        MVI     T_HRS,  R0
        MVII    #3,     R2
        CALL    DEC16A

        INCR    R4

        MVI     T_MIN,  R0
        MVII    #3,     R2
        CALL    DEC16A

        INCR    R4

        MVI     T_SEC,  R0
        MVII    #3,     R2
        CALL    DEC16A

        MVII    #$D0 + C_BLU, R1
        B       @@do_colon

        ENDP

;;==========================================================================;;
;; WAIT -- Do a busy-wait.  Number of ticks is in R0.                       ;;
;;==========================================================================;;
WAIT    PROC

        MVO     R0,     BUSY
@@spin: MVI     BUSY,   R0
        DECR    R0
        BPL     @@spin
        JR      R5

        ENDP


;;==========================================================================;;
;; CLRSCR -- Clear the screen.                                              ;;
;;==========================================================================;;
CLRSCR  PROC
        PSHR    R5
        MVII    #1,     R0
        CALL    WAIT
        PULR    R5
        ; Fill display memory with zeros.
        MVII    #$0F0,  R1
        MVII    #$200,  R4
        B       FILLZERO
        ENDP
        

;;==========================================================================;;
;; CLRSCR1 -- Clear most of the screen.                                     ;;
;;==========================================================================;;
CLRSCR1 PROC
        PSHR    R5
        MVII    #1,     R0
        CALL    WAIT
        PULR    R5
        ; Fill display memory with zeros.
        MVII    #$0F0 - 20,  R1
        MVII    #$200,  R4
        B       FILLZERO
        ENDP
        

;;==========================================================================;;
;; HEX16                                                                    ;;
;; Display a 4-digit hex number on the screen                               ;;
;;                                                                          ;;
;; INPUTS:                                                                  ;;
;; R0 -- Hex number                                                         ;;
;; R1 -- Color mask / screen format word                                    ;;
;; R4 -- Screen offset                                                      ;;
;;                                                                          ;;
;; OUTPUTS:                                                                 ;;
;; R0 -- rotated left by 3                                                  ;;
;; R1 -- unmodified                                                         ;;
;; R2 -- trashed                                                            ;;
;; R3 -- zeroed                                                             ;;
;; R4 -- points just to right of string                                     ;;
;;==========================================================================;;
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
        ADDI    #$38,   R2      ; If the digit >= A, add 6 << 3.
@@digit:
        ADDI    #$80,   R2      ; Generate proper GROM index.
        XORR    R1,     R2      ; Merge in the screen format word
        MVO@    R2,     R4      ; Display the digit to the screen.

        DECR    R3              ; Iterate three more times.
        BNE     @@loop

        JR      R5              ; Done!  Return.
        ENDP


;;==========================================================================;;
;; FILLZERO                                                                 ;;
;; Fills memory with zeros                                                  ;;
;;                                                                          ;;
;; FILLMEM                                                                  ;;
;; Fills memory with a constant                                             ;;
;;                                                                          ;;
;; INPUTS:                                                                  ;;
;; R0 -- Fill value (FILLMEM only)                                          ;;
;; R1 -- Number of words to fill                                            ;;
;; R4 -- Start of fill area                                                 ;;
;; R5 -- Return address                                                     ;;
;;                                                                          ;;
;; OUTPUTS:                                                                 ;;
;; R0 -- Zeroed if FILLZERO, otherwise untouched.                           ;;
;; R1 -- Zeroed                                                             ;;
;; R4 -- Points to word after fill area                                     ;;
;;==========================================================================;;
FILLZERO        PROC
        CLRR    R0              ; Start out with R0 zeroed for FILLZERO
FILLMEM
        MVO@    R0,     R4      ; Store R0 out at R4, and move along
        DECR    R1              ; Keep going until our count runs out
        BNEQ    FILLMEM
        JR      R5              ; Return to the caller.
        ENDP

;;==========================================================================;;
;; DRAWSTRING                                                               ;;
;; Puts an ASCIIZ string pointed to by R0 onscreen.                         ;;
;;                                                                          ;;
;; DRAWSTRING2                                                              ;;
;; Puts an ASCIIZ string after a JSR R5 onscreen.                           ;;
;;                                                                          ;;
;; INPUTS:                                                                  ;;
;; R0 -- String pointer (if DRAWSTRING)                                     ;;
;; R1 -- Screen format word                                                 ;;
;; R4 -- Output pointer                                                     ;;
;; R5 -- Return address (also, string if DRAWSTRING2).                      ;;
;;                                                                          ;;
;; OUTPUTS:                                                                 ;;
;; R0 -- Zero if DRAWSTRING2, one if DRAWSTRING                             ;;
;; R1 -- Untouched EXCEPT bit 15 is cleared.                                ;;
;; R4 -- Points just after displayed string.                                ;;
;; R5 -- Points just past end of string.                                    ;;
;; R2 and R3 are not modified.                                              ;;
;;==========================================================================;;
DRAWSTRING      PROC
        PSHR    R5              ; Save the return address
        MOVR    R0,     R5      ; Move our pointer into R5
        SETC                    ; Flag that we'll be returning via PULR PC
        INCR    R7              ; (skip the CLRC)
DRAWSTRING2:
        CLRC                    ; Flag that we'll be returning via JR R5
        SLL     R1,     1       
        RRC     R1,     1       ; Put flag into bit 15 of screen format word.
        MVI@    R5,     R0      ; Get first char of string
@@tloop:
        SUBI    #32,    R0      ; Shift ASCII range to charset
        SLL     R0,     2       ; Move it to position for BTAB word
        SLL     R0,     1
        XORR    R1,     R0      ; Merge with color info
        MVO@    R0,     R4      ; Write to display
        MVI@    R5,     R0      ; Get next character
        TSTR    R0              ; Is it NUL?
        BNEQ    @@tloop         ; --> No, keep copying then

        SLLC    R1,     1       ; Recover 'return' flag from screen fmt word,
        ADCR    R0              ; ... and put it in R0.
        SLR     R1,     1       ; Restore screen format word.
        ADDR    R0,     R7      ; If flag is set, return by PULR PC, else 
        JR      R5              ; ... return by the JR R5.
        PULR    R7
        ENDP

;;==========================================================================;;
;; MSG_ERR  -- Display "ERR!"                                               ;;
;;==========================================================================;;
MSG_ERR PROC
        PSHR    R5
        MVII    #C_RED, R1
        CALL    DRAWSTRING2
ERR:    BYTE    "ERR!", 0
        PULR    PC
        ENDP

;;==========================================================================;;
;; MSG_OK   -- Display "*OK*"                                               ;;
;;==========================================================================;;
MSG_OK  PROC
        PSHR    R5
        MVII    #C_YEL, R1
        CALL    DRAWSTRING2
OK:     BYTE    "*OK*", 0
        PULR    PC
        ENDP
        
;;==========================================================================;;
;; MSG_DONE -- Display "*Done*" and wait for tap on the pad.                ;;
;;==========================================================================;;
MSG_DONE PROC
        PSHR    R5
        MVII    #$200 + 11*20 + 7, R4
        MVII    #C_BLU, R1
        CALL    DRAWSTRING2
        BYTE    "*Done*", 0
        B       WAIT_TAP + 1    ; Wait for a tap on the keypad.
        ENDP

;;==========================================================================;;
;; MSG_MORE -- Display "More..." and wait for tap on the pad.               ;;
;;==========================================================================;;
MSG_MORE PROC
        PSHR    R5
        MVII    #$200 + 11*20 + 6, R4
        MVII    #C_RED, R1
        CALL    DRAWSTRING2
        BYTE    "More...", 0
        B       WAIT_TAP + 1    ; Wait for a tap on the keypad.
        ENDP


;;==========================================================================;;
;; WAITHAND -- Waits for controller to be pressed/released                  ;;
;;==========================================================================;;
WAITHAND        PROC
        B       @@waithand
@@waitpress:
        BNEQ    @@waitdone
@@waithand:
        MVII    #$100,  R1      ; Debounce factor
@@waitloop:
        MVII    #$1FE,  R4
        SDBD
        MVI@    R4,     R0
        COMR    R0
        BC      @@waitpress
        BNEQ    @@waithand
@@waitdone:
        DECR    R1
        BNEQ    @@waitloop
        JR      R5
        ENDP

;;==========================================================================;;
;; WAIT_TAP -- Wait for controller to go through press/release cycle.       ;;
;;==========================================================================;;
WAIT_TAP PROC
        PSHR    R5
        PSHR    R0
        PSHR    R1
        PSHR    R4
        SETC
        CALL    WAITHAND
        MOVR    R0,     R2
        SWAP    R2
        ANDR    R0,     R2
        ANDI    #$FF,   R2      ; Remember what was pressed.
        CLRC
        CALL    WAITHAND
        PULR    R4
        PULR    R1
        PULR    R0
        PULR    PC
        ENDP




;;==========================================================================;;
;; RAND                                                                     ;;
;; Returns random bits in R0.                                               ;;
;;                                                                          ;;
;; INPUTS:                                                                  ;;
;; R0 -- Number of bits desired                                             ;;
;; R5 -- Return address                                                     ;;
;; Random state in RNDLO, RNDHI                                             ;;
;;                                                                          ;;
;; OUTPUTS:                                                                 ;;
;; R0 -- N random bits.                                                     ;;
;; R1, R2, R3, R4 -- Saved and restored                                     ;;
;; R5 -- trashed.                                                           ;;
;;==========================================================================;;
RAND    PROC
        PSHR    R5              ; Save return address and R1..R4
        PSHR    R4
        PSHR    R3
        PSHR    R2
        PSHR    R1

        MVII    #1,     R1      ; Our initial mask word
        BEQ     @@nobits

        MVII    #$04C1, R5      ; period==(2**32 - 1) polynomial 
        MVII    #$1DB7, R4      ; (this is the CRC-32 polynomial)
        MVI     RNDHI,  R3      ; Read in our 32-bit random number state
        MVI     RNDLO,  R2

        TSTR    R3              ; If our random number generator is zero
        BNEQ    @@loop          ; jumpstart the process by forcing an XOR
        TSTR    R2              ; of our generator polynomal into R2/R3
        SETC                    ; up front.  Otherwise, we won't generate
        BEQ     @@forceit       ; any random numbers!
        
@@loop:
        SLLC    R2,     1       ; Shift our 32-bit random number left by 1
        RLC     R3,     1       ; ... by using the carry and an RLC.
@@forceit:
        SLL     R1,     1       ; Shift our mask bit left by 1
        BNC     @@nocarry
        XORR    R4,     R2      ; If the carry was set, XOR in our generator
        XORR    R5,     R3      ; polynomial.  
@@nocarry:
        DECR    R0              ; Keep generating bits.
        BNEQ    @@loop

        MVO     R3,     RNDHI   ; Store our new random number state.
        MVO     R2,     RNDLO

@@nobits:
        DECR    R1              ; Turn our mask bit into a mask word
        ANDR    R1,     R2      ; Mask the bits we actually want.
        MOVR    R2,     R0      ; Return our result in R0

        PULR    R1              ; Retstore our registers and return.
        PULR    R2
        PULR    R3
        PULR    R4
        PULR    PC

        ENDP



;;==========================================================================;;
;; POW10                                                                    ;;
;; Look-up table with powers of 10 as 32-bit numbers (little endian).       ;;
;;                                                                          ;;
;; NPW10                                                                    ;;
;; Same as POW10, only -(10**x) instead of 10**x.                           ;;
;;==========================================================================;;

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

;;==========================================================================;;
;; DEC16                                                                    ;;
;; Displays a 16-bit decimal number on the screen with leading blanks       ;;
;; in a field up to 5 characters wide.  Displays all blanks if the number   ;;
;; is zero.                                                                 ;;
;;                                                                          ;;
;; DEC16A                                                                   ;;
;; Same as DEC16, only displays leading zeroes.                             ;;
;;                                                                          ;;
;; DEC16B                                                                   ;;
;; Same as DEC16, only leading zeros are controlled by bit 15 of R3.        ;;
;; (If set, suppress leading zeros.  If clear, show leading zeros.)         ;;
;;                                                                          ;;
;; DEC16C                                                                   ;;
;; Same as DEC16B, except R1 contains an amount to add to the first digit.  ;;
;;                                                                          ;;
;; DEC16Z                                                                   ;;
;; Same as DEC16, except displays a single zero if the value is zero.       ;;
;; (Note:  DEC16Z is actually defined separately along with DEC32Z).        ;;
;;                                                                          ;;
;; INPUTS:                                                                  ;;
;; R0 -- Number to be displayed in decimal.                                 ;;
;; R1 -- (DEC16C only): Amount to add to initial digit.                     ;;
;; R2 -- Number of digits to suppress  (If R2<=5, it is 5-field_width)      ;;
;; R3 -- Color mask / screen format word (XORd with char index)             ;;
;; R4 -- Screen offset (lower 8-bits)                                       ;;
;;                                                                          ;;
;; OUTPUTS:                                                                 ;;
;; R0 -- Zeroed                                                             ;;
;; R1 -- Trashed                                                            ;;
;; R2 -- Remaining digits to suppress (0 if initially <= 5.)                ;;
;; R3 -- Color mask, with bit 15 set if no digits displayed.                ;;
;; R4 -- Pointer to character just right of string                          ;;
;; R5 -- Trashed                                                            ;;
;;                                                                          ;;
;; Routine uses DEC16_0, DEC16_1 for temporary storage.                     ;;
;;==========================================================================;;
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

        DIS
        MVI     @@so,   R5
        PSHR    R5
        MVI     @@fw,   R5
        PSHR    R5
        EIS

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
        MVI     @@fw,   R2      ; Restore digit suppress count

        DIS
        PULR    R5
        MVO     R5,     @@fw
        PULR    R5
        MVO     R5,     @@so
        EIS

        PULR    PC              ; Whew!  Done!
        ENDP

;;==========================================================================;;
;; DEC32                                                                    ;;
;; Displays a 32 bit number without leading zeros.  It performs this feat   ;;
;; by calling DEC16 multiple times.                                         ;;
;;                                                                          ;;
;; DEC32A                                                                   ;;
;; Same as DEC32, except leading zeros are displayed.                       ;;
;;                                                                          ;;
;; DEC32B                                                                   ;;
;; Same as DEC32, except leading zeros are controlled by bit 15 of R3       ;;
;; (If set, suppress leading zeros.  If clear, show leading zeros.)         ;;
;;                                                                          ;;
;; DEC32Z                                                                   ;;
;; Same as DEC32, except displays a single zero if the value is zero.       ;;
;; (Note:  DEC32Z is actually defined separately along with DEC16Z).        ;;
;;                                                                          ;;
;; INPUTS:                                                                  ;;
;; R0 -- Low half of 32-bit number                                          ;;
;; R1 -- High half of 32-bit number                                         ;;
;; R2 -- Number of leading digits to suppress (10 - field width)            ;;
;; R3 -- Screen format word                                                 ;;
;; R4 -- Screen offset (lower 8-bits)                                       ;;
;;                                                                          ;;
;; OUTPUTS:                                                                 ;;
;; R0 -- Zeroed                                                             ;;
;; R1 -- Trashed                                                            ;;
;; R2 -- Remaining digits to suppress (0 if initially <= 10.)               ;;
;; R3 -- Color mask, with bit 15 set if no digits displayed.                ;;
;; R4 -- Pointer to character just right of string                          ;;
;; R5 -- Trashed                                                            ;;
;;                                                                          ;;
;; Routine uses DEC_0..DEC_3 for temporary storage.                         ;;
;;==========================================================================;;
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

;;==========================================================================;;
;; DEC32Z                                                                   ;;
;; Same as DEC32, except a zero is displayed in the final position if       ;;
;; the whole number's value is zero.                                        ;;
;;                                                                          ;;
;; DEC16Z                                                                   ;;
;; Same as DEC16, except a zero is displayed in the final position if       ;;
;; the whole number's value is zero.                                        ;;
;;                                                                          ;;
;; INPUTS:                                                                  ;;
;; R0 -- Lower 16-bits of number                                            ;;
;; R1 -- Upper 16-bits of number (if DEC32Z)                                ;;
;; R2 -- Number of leading digits to suppress                               ;;
;;       (10 - field width for DEC32Z, 5 - field width for DEC16Z).         ;;
;; R3 -- Screen format word                                                 ;;
;; R4 -- Screen offset (lower 8-bits)                                       ;;
;;                                                                          ;;
;; OUTPUTS:                                                                 ;;
;; R0 -- Zeroed                                                             ;;
;; R1 -- Trashed                                                            ;;
;; R2 -- If number == 0, unchanged.  If != 0, remaining digits to suppress  ;;
;; R3 -- Color mask, unmodified.                                            ;;
;; R4 -- Pointer to character just right of string                          ;;
;; R5 -- Trashed                                                            ;;
;;                                                                          ;;
;; DEC16Z uses DEC_0..DEC_1 for temporary storage.                          ;;
;; DEC32Z uses DEC_0..DEC_3 for temporary storage.                          ;;
;;==========================================================================;;
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

;;==========================================================================;;
;; RANDSAVE -- Save the current random-number seed.                         ;;
;;==========================================================================;;
RANDSAVE    PROC
        PSHR    R5
        MVI     RNDLO,  R5
        MVO     R5,     SAVLO
        MVI     RNDHI,  R5
        MVO     R5,     SAVHI
        PULR    PC
        ENDP

;;==========================================================================;;
;; RANDRSTR -- Restore the current random-number seed.                      ;;
;;==========================================================================;;
RANDRSTR    PROC
        PSHR    R5
        MVI     SAVLO,  R5
        MVO     R5,     RNDLO
        MVI     SAVHI,  R5
        MVO     R5,     RNDHI
        PULR    PC
        ENDP

;;==========================================================================;;
;; BANKTEST -- Really simple bank test.  Fills up all of the Intellicart    ;;
;;             memory with random data, and then reads it back.             ;;
;;==========================================================================;;
BANKTEST    PROC
            PSHR    R5

            ;;--------------------------------------------------------------;;
            ;; Remember our initial parameters                              ;;
            ;;--------------------------------------------------------------;;
            MVO     R1,     WPAGE
            MVO     R2,     RPAGE

            ;;--------------------------------------------------------------;;
            ;; Clear the screen                                             ;;
            ;;--------------------------------------------------------------;;
            CALL    CLRSCR1

            ;;--------------------------------------------------------------;;
            ;; Put up our status display                                    ;;
            ;;--------------------------------------------------------------;;
            MVII    #ROW_0, R4
            MVII    #C_GRN, R1
            CALL    DRAWSTRING2
                    ;01234567890123456789;
            BYTE    ">> Bank Switching <<"
            BYTE    ">>  Simple Test:  <<",0

            MVII    #ROW_3, R4
            MVII    #C_WHT, R1
            CALL    DRAWSTRING2
                    ;01234567890123456789;
            BYTE    "Writeout page:  XXXX"
            BYTE    "Readback page:  XXXX"
            BYTE    "                    "
            BYTE    "   Filling at:      "
            BYTE    " Reading back:      " , 0

            MVII    #ROW_3 + 16, R4
            MVI     WPAGE,  R0
            MVII    #C_YEL, R1
            CALL    HEX16

            MVII    #ROW_4 + 16, R4
            MVI     RPAGE,  R0
            CALL    HEX16

            ;;--------------------------------------------------------------;;
            ;; Fill up the Intellicart's memory.  Avoid $5000-$5FFF, since  ;;
            ;; that's where this test program lives.  ;-)                   ;;
            ;;--------------------------------------------------------------;;
            CALL    RANDSAVE        ; Save the current random seed.
            MVI     WPAGE,  R1
            SWAP    R1
            CLRR    R2
            MVO     R1,     PGTMP   ; Initialize our page and remapping
            MVO     R2,     MPTMP   ; counters.

@@fill_olp:
;           CMPI    #$50,   R2      ; Skip $5000 - $5FFFF
;           BNEQ    @@ok_f          ; .. by adding $10 if we see $50.
;           ADDI    #$10,   R2      
;@ok_f:
;           MVO     R2,     MPTMP   ; Save the mapping for next time
;           CALL    IC_SETBANK      ; Actually flip the page.

            CLRR    R2
            MVO     R2,     $46
            MVO     R2,     $4D

            MVI     MPTMP,  R0      ; Get the mapping so we can show it.
            SWAP    R0              ; Put it in the MSBs.
            MVII    #C_TAN, R1      ; Display it....
            MVII    #ROW_6 + 16, R4
            CALL    HEX16

            MVII    #16,    R0
            CALL    RAND
            MOVR    R0,     R3
            MVII    #16,    R0
            CALL    RAND

            MVI     WPAGE,  R4      ; Get the page's Inty address
@@fill_ilp:
            INCR    R3
            XORR    R3,     R0
            MVO@    R0,     R4      ; Write them into the bankswitched page.

            DECR    R1
            BNEQ    @@fill_ilp

;           MVI     PGTMP,  R1      ; Get the page number
;           MVI     MPTMP,  R2      ; Get the page's current mapping.
;           ADDI    #$08,   R2      ; Increment the mapping.
;           ANDI    #$F8,   R2      ; Only keep the 5 MSBs.
;           BNEQ    @@fill_olp      ; If it goes to 0, we're done.

            ;;--------------------------------------------------------------;;
            ;; Actually read back the random values and make sure they all  ;;
            ;; match what we originally wrote.                              ;;
            ;;--------------------------------------------------------------;;
            CALL    RANDRSTR        ; Restore the random seed we saved.
            MVI     RPAGE,  R1
            SWAP    R1
            CLRR    R2
            MVO     R1,     PGTMP   ; Initialize our page and remapping
            MVO     R2,     MPTMP   ; counters.

@@read_olp:
;           CMPI    #$50,   R2      ; Skip $5000 - $5FFFF
;           BNEQ    @@ok_r          ; .. by adding $10 if we see $50.
;           ADDI    #$10,   R2      
;@ok_r:
;           MVO     R2,     MPTMP   ; Save the mapping for next time
;           CALL    IC_SETBANK      ; Actually flip the page.
            CLRR    R2
            MVO     R2,     $46
            MVO     R2,     $4D


            MVI     MPTMP,  R0      ; Get the mapping so we can show it.
            SWAP    R0              ; Put it in the MSBs.
            MVII    #C_TAN, R1      ; Display it....
            MVII    #ROW_7 + 16, R4
            CALL    HEX16

            MVII    #16,    R0
            CALL    RAND
            MOVR    R0,     R3
            MVII    #16,    R0
            CALL    RAND

            MVI     RPAGE,  R4      ; Get the page's Inty address
@@read_ilp:
            INCR    R3
            XORR    R3,     R0
            MVI@    R4,     R2      ; Compare the memory value with rand num.
            CMPR    R0,     R2
            BNEQ    @@fail

            DECR    R1
            BNEQ    @@read_ilp

;           MVI     PGTMP,  R1      ; Get the page number
;           MVI     MPTMP,  R2      ; Get the page's current mapping.
;           ADDI    #$08,   R2      ; Increment the mapping.
;           ANDI    #$F8,   R2      ; Only keep the 5 MSBs.
;           BNEQ    @@read_olp      ; If it goes to 0, we're done.

@@pass:
            MVII    #ROW_8 + 8, R4
            CALL    MSG_OK

            MVII    #PDLY,  R0
            CALL    WAIT

            PULR    PC

@@fail:
            PSHR    R0
            PSHR    R2
            PSHR    R4
            MVII    #ROW_8, R4
            MVII    #C_RED, R1
            CALL    DRAWSTRING2
            BYTE    "A=",0

            PULR    R0
            CALL    HEX16

            CALL    DRAWSTRING2
            BYTE    " D=",0

            PULR    R0
            CALL    HEX16

            CALL    DRAWSTRING2
            BYTE    " X=", 0

            PULR    R0
            CALL    HEX16

            MVII    #ROW_9 + 8, R4
            CALL    MSG_ERR
            
            MVII    #EDLY,  R0
            CALL    WAIT

            PULR    PC
            ENDP



;;==========================================================================;;
;; Intellicart Bank Switch Utility Routines                                 ;;
;; Copyright 2000, Joe Zbiciak.                                             ;;
;;                                                                          ;;
;; This file contains a number of useful routines that you're welcome       ;;
;; to use in your own software.  Please keep in mind that these routines    ;;
;; are licensed under the GNU General Public License, and so if you plan    ;;
;; to distribute a program which incorporates these routines, it too must   ;;
;; be distributed under the GNU General Public License.                     ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  This program is free software; you can redistribute it and/or modify    *;
;*  it under the terms of the GNU General Public License as published by    *;
;*  the Free Software Foundation; either version 2 of the License, or       *;
;*  (at your option) any later version.                                     *;
;*                                                                          *;
;*  This program is distributed in the hope that it will be useful,         *;
;*  but WITHOUT ANY WARRANTY; without even the implied warranty of          *;
;*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *;
;*  General Public License for more details.                                *;
;*                                                                          *;
;*  You should have received a copy of the GNU General Public License       *;
;*  along with this program; if not, write to the Free Software             *;
;*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.               *;
;* ======================================================================== *;
;*                   Copyright (c) 2000, Joseph Zbiciak                     *;
;* ======================================================================== *;


;;==========================================================================;;
;;  IC_BANK2REG  -- Converts a bank address to an Intellicart reg address.  ;;
;;                                                                          ;;
;;  Inputs:                                                                 ;;
;;    R1 -- 5 MSBs of bank address in bits 7..3                             ;;
;;    R5 -- Return address                                                  ;;
;;                                                                          ;;
;;  Outputs:                                                                ;;
;;    R1 -- Address of Intellicart base control register.                   ;;
;;    R5 -- Trashed.                                                        ;;
;;                                                                          ;;
;;==========================================================================;;
IC_BANK2REG PROC
            PSHR    R5                  ; Save return address

            MOVR    R1,     R5          ; Get Hi/Lo 2K bit from address in R5
            ADDR    R5,     R5
            ANDI    #$10,   R5

            SLR     R1,     2           ; Put 4 MSBs of page in 4 LSBs of R0
            SLR     R1,     2
            ANDI    #$0F,   R1          ; Keep only the four LSBs.
            ADDR    R5,     R1          ; Merge bits: 76543210 -> xxxx37654
            ADDI    #$40,   R1          ; Point it at Intellicart ctrl regs.

            PULR    PC                  ; Return.
            ENDP

;;==========================================================================;;
;; IC_SETBANK    -- Sets a bank for a given 2K page of memory.              ;;
;;                                                                          ;;
;;  Inputs:                                                                 ;;
;;    R1 -- 5 MSBs of bank address in bits 7..3                             ;;
;;    R2 -- New bank address to point to in 8 LSBs.                         ;;
;;    R5 -- Return address                                                  ;;
;;                                                                          ;;
;;  Outputs:                                                                ;;
;;    R1 -- Address of Intellicart base control register.                   ;;
;;    R2 -- Trashed.                                                        ;;
;;    R5 -- Trashed.                                                        ;;
;;                                                                          ;;
;;  Example:                                                                ;;
;;    To remap 0x7800-0x7FFF in the Inty's address space to point to        ;;
;;    0x4000-0x47FF in the cart's address space, pass in the following      ;;
;;    parameters:  R1 = 0x0078, R2 = 0x0040.                                ;;
;;                                                                          ;;
;;==========================================================================;;
IC_SETBANK  PROC
            PSHR    R5                  ; Save return address.
            CALL    IC_BANK2REG         ; Convert bank to ctrl reg address
            MVO@    R2,     R1          ; Write to control register.
            PULR    PC                  ; Return
            ENDP

;;==========================================================================;;
;; IC_BANKTBL    -- Resets the bank mappings for this cartridge using a     ;;
;;                  table of initial mappings.                              ;;
;;                                                                          ;;
;;  Inputs:                                                                 ;;
;;    R4 -- Pointer to bank initialization table (format below)             ;;
;;    R5 -- Return address                                                  ;;
;;                                                                          ;;
;;  Outputs:                                                                ;;
;;    R4 -- Points just past end of table.                                  ;;
;;    R5 -- Trashed.                                                        ;;
;;                                                                          ;;
;;  Table format:                                                           ;;
;;    The table is laid out as a series of byte pairs packed into 16-bit    ;;
;;    words.  The current format for each word is like so:                  ;;
;;                                                                          ;;
;;       15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0     ;;
;;      +---+---+---+---+---+---+---+---+-------------------------------+   ;;
;;      |    BANK NUMBER    |n/a|n/a|n/a| INTELLICART BANK ADDR MAPPING |   ;; 
;;      +---+---+---+---+---+---+---+---+-------------------------------+   ;;
;;                                                                          ;;
;;    To end the table, use a word filled with all zeros.                   ;;
;;                                                                          ;;
;;==========================================================================;;
IC_BANKTBL  PROC
            PSHR    R5
            PSHR    R2
            PSHR    R1

            B       @@1st_iter
@@loop:
            CALL    IC_SETBANK
@@1st_iter:
            MVI@    R4,     R2
            MOVR    R2,     R1
            SWAP    R1,     1
            BNEQ    @@loop

            PULR    R1
            PULR    R2
            PULR    PC
            ENDP

