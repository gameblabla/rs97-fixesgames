;;==========================================================================;;
;; INTV RAM Tests.  Performs tests on a range of RAM in the Intellivision.  ;;
;; This is most useful for testing the Intellicart's RAM.                   ;;
;; Copyright 2002, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  TO BUILD IN BIN+CFG FORMAT:                                             *;
;*      as1600 -o mem_test.bin -l mem_test.lst mem_test.asm                 *;
;*                                                                          *;
;*  TO BUILD IN ROM FORMAT:                                                 *;
;*      as1600 -o mem_test.rom -l mem_test.lst mem_test.asm                 *;
;* ======================================================================== *;

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
;*                   Copyright (c) 2002, Joseph Zbiciak                     *;
;* ======================================================================== *;

        ROMW    16

; ------------------------------------------------------------------------- ;
;  Test Configuration.                                                      ;
;  Modify these EQUates to reflect the range/width of memory to test.       ;
; ------------------------------------------------------------------------- ;
RAMLO   EQU     $A000           ; Start of RAM memory range.
RAMHI   EQU     $BFFF           ; End of RAM memory range (inclusive).
RAMAM   EQU     $1FFF           ; RAM Address Mask
RAMW    EQU     $10             ; Ram width in bits, 1 through 16 bits.

; ------------------------------------------------------------------------- ;
;  Mark the memory range as read/write.                                     ;
; ------------------------------------------------------------------------- ;
        ORG     RAMLO, RAMLO, "+RWB"
        RMB     RAMHI - RAMLO + 1

; ------------------------------------------------------------------------- ;
;  Variables.  We hide the 16-bit variables under the stack.                ;
; ------------------------------------------------------------------------- ;
                        ; Used by       Req'd Width   Description
                        ;---------------------------------------------------
RNDLO   EQU     $2F0    ; RAND          16-bit        Random number state
RNDHI   EQU     $2F1    ; RAND          16-bit        Random number state
TESTFXN EQU     $2F2    ; DOTEST        16-bit        Ptr to test function
BUSY    EQU     $2F3    ; ISR/WAIT      16-bit        Busy-wait counter
DEC_0   EQU     $110    ; DEC16         8-bit         Temp. storage
DEC_1   EQU     $111    ; DEC16         8-bit         Temp. storage
IPT     EQU     $2F4    ; the tests     16-bit        Iterations per test
T_ITER  EQU     $2F5    ; the tests     16-bit        Test number
ERRS    EQU     $2F6    ; the tests     16-bit        Total # of errors
SAVLO   EQU     $2F7    ; RANDSAVE/RSTR 16-bit        Saves rand num. seed
SAVHI   EQU     $2F8    ; RANDSAVE/RSTR 16-bit        Saves rand num. seed
SCRAM   EQU     $2F9    ; FILL_R        16-bit        Addr. scrambling pattern
CUR_L   EQU     $2FA    ; ISR/tests     16-bit        Current location
CUR_S   EQU     $2FB    ; ISR/tests     16-bit        Current status
APAT    EQU     $120    ; the tests     8-bit         Addressing pattern
ARROW   EQU     $130    ; ISR           8-bit         Current row w/ arrows
ABLNK   EQU     $131    ; ISR           8-bit         Blink counter for arrows
TBLNK   EQU     $132    ; ISR           8-bit         Blink counter for timer
T_DAY   EQU     $133    ; ISR           8-bit         Day counter
T_HRS   EQU     $134    ; ISR           8-bit         Hour counter
T_MIN   EQU     $135    ; ISR           8-bit         Minute counter
T_SEC   EQU     $136    ; ISR           8-bit         Second counter
T_CLN   EQU     $137    ; ISR           8-bit         Colons on or off?
HUPD    EQU     $138    ; ISR           8-bit         HEX update counter
BOS     EQU     $300    ; STACK         16-bit        Bottom of stack.
ITMP    EQU     $331


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

MASK    EQU     (1 SHL RAMW)-1  ; All 1s mask equal to RAM's width.
RAMSZ   EQU     RAMHI-RAMLO+1   ; Number of words of RAM.
HCNT    EQU     4               ; Number of ticks between "hex updates"
EDLY    EQU     30              ; Number of ticks to hold error addr up.
PDLY    EQU     120             ; Number of ticks to hold each full page


        ORG     $5000
;------------------------------------------------------------------------------
; EXEC-friendly ROM header.
;------------------------------------------------------------------------------
ROMHDR: BIDECLE ZERO            ; MOB picture base   (points to NULL list)
        BIDECLE ZERO            ; Process table      (points to NULL list)
        BIDECLE MAIN            ; Program start address
        BIDECLE ZERO            ; Bkgnd picture base (points to NULL list)
        BIDECLE ONES            ; GRAM pictures      (points to NULL list)
        BIDECLE TITLE           ; Cartridge title/date
        DECLE   $03C0           ; No ECS title, run code after title,
                                ; ... no clicks
ZERO:   DECLE   $0000           ; Screen border control
        DECLE   $0000           ; 0 = color stack, 1 = f/b mode
        DECLE   0, 0, 0, 0, 0   ; Initial color stack / border color: Black
ONES:   DECLE   1
;------------------------------------------------------------------------------


;; ======================================================================== ;;
;;  TITLE / TITLEFIX -- The Title Screen code.                              ;;
;; ======================================================================== ;;
TITLE:  BYTE    101, "RAM Tester", 0 

TITLEFIX:   PROC

            BEGIN

            MVII    #$100,    R4    ;\
            SDBD                    ; |__ remember old ISR address
            MVI@    R4,       R1    ; |
            MVO     R1,       ITMP  ;/

            MVII    #@@isr,   R0    ; \
            SUBI    #2,       R4    ;  |
            MVO@    R0,       R4    ;  |-- Make title screen cyan
            SWAP    R0              ;  |   via short ISR.
            MVO@    R0,       R4    ; /

            ; Patch the title string to say '=JRMZ=' instead of Mattel.
            CALL    PRINT.FLS       ; Write string (ptr in R5)
            DECLE   0, $23C         ; Black,  Point to 'Mattel' in top-left
            BYTE    "Zbiciak "      ; Guess who?  :-)
            BYTE    "Electronics",0

            CALL    PRINT.FLS       ; Write string (ptr in R1)
            DECLE   0, $2D0         ; Black,  Point to 'Mattel' in lower-right
            BYTE    "2001 =JRMZ=",0 ; Guess who?  :-)

            ; Make all the rest of the text black to match
            MVII    #$243,  R3      ; Start after first JRMZ
            MVII    #7,     R0      ; Mask = 0xFFF8.  That's the 1s compl...
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

@@isr:
            ; Cyan screen
            MVII    #9,     R0      ; Cyan title screen
            MVO     R0,     $28     ; Color stack 0 = Cyan
            MVO     R0,     $29     ; Color stack 1 = Cyan
            MVO     R0,     $2A     ; Color stack 2 = Cyan
            MVO     R0,     $2B     ; Color stack 3 = Cyan
            MVO     R0,     $2C     ; Border color = Cyan

            MVI     ITMP,   PC

            ENDP


;; ======================================================================== ;;
;;  MAIN -- main program                                                    ;;
;; ======================================================================== ;;
MAIN:   PROC

        JSRD    R5,     ISRSPIN

        DIS
        CLRR    R0
        MVII    #$0028, R4
        MVO@    R0,     R4      ; Black screen.
        INCR    R0
        MVO@    R0,     R4      ; CS0 == Black
        CLRR    R0
        MVO@    R0,     R4      ; CS1 == Blue
        MVO@    R0,     R4      ; CS2 == Black
        MVO@    R0,     R4      ; CS3 == Black

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

        EIS
        MVII    #1,     R0
        MVO     R0,     APAT    ; Start "RANDOM", will get toggled to LINEAR.

@@test_loop:
        MVI     T_ITER, R0      ; Increment the test iteration number
        INCR    R0
        MVO     R0,     T_ITER

        MVI     APAT,   R0      ; Get our current addressing mode 
        XORI    #1,     R0      ; Toggle RANDOM or LINEAR.
        MVO     R0,     APAT    ;
        BNEQ    @@dotests       ; If it's now RANDOM, don't update IPT yet.

        MVI     IPT,    R0      ; Double the iterations-per-test
        SLL     R0,     1       ; to the next "power_of_2 - 1"
        INCR    R0
        CMPI    #$7FFF, R0      ; Don't let it go over 32767
        BLE     @@ipt_ok
        MVII    #1,     R0      ; If it does, restart it at 1.
@@ipt_ok:
        MVO     R0,     IPT

@@dotests:
        CALL    MAINSCRN

        MVII    #1,     R0

@@loop:
        CALL    SET_ARRO
        PSHR    R0
        MVII    #@@ret, R5
        MOVR    R0,     R4
        ADDR    R0,     R4
        ADDI    #LINFXN-2, R4
        MVI     APAT,   R2
        TSTR    R2
        BEQ     @@dispat
        ADDI    #RNDFXN-LINFXN, R4
@@dispat:
        SDBD
        MVI@    R4,     PC      ; Call the test.
@@ret:
        PULR    R0
        INCR    R0
        CMPI    #6,     R0
        BLT     @@loop

        CLRR    R0
        CALL    SET_ARRO
;       CALL    MSG_DONE
        MVII    #PDLY,  R0
        CALL    WAIT

        B       @@test_loop

        ENDP

LINFXN  PROC
        WORD    FILL_L
        WORD    WLK1_L
        WORD    WLK0_L
        WORD    CHKR_L
        WORD    RAND_L
        ENDP

RNDFXN  PROC
        WORD    FILL_R
        WORD    WLK1_R
        WORD    WLK0_R
        WORD    CHKR_R
        WORD    RAND_R
        ENDP



;; ======================================================================== ;;
;;  ISR -- The Interrupt Service routine.                                   ;;
;; ======================================================================== ;;
ISR     PROC
        PSHR    R5

        MVI     BUSY,   R0      ; Decrement the busy-wait counter.
        DECR    R0
        BMI     @@zero
        MVO     R0,     BUSY

@@zero:
        MVI     $21,    R0      ; force color-stack mode
        MVO     R5,     $20     ; enable display

        ; Blink the "Current Test" arrows, if enabled.
        MVI     ARROW,  R0
        TSTR    R0
        BEQ     @@noarrow

        MVI     ABLNK,  R1
        DECR    R1
        BMI     @@redisp        ; Timer expired, re-display arrows
        MVO     R1,     ABLNK
        SUBI    #20,    R1      ; Are we at the "delete arrow" threshold?
        BNEQ    @@do_stat       ; No:  Do nothing.
        MVII    #C_BLU, R1      ; Undisplay by displaying in blue.
        B       @@undisp

@@redisp:
        MVII    #40,    R1
        MVO     R1,     ABLNK   ; Reset blink counter to 60
        MVII    #C_GRN, R1      ; R1 == GREEN 

@@undisp:
        MVII    #ROW_4+1,R4     ; Offset down to Row #1 for first test.
        ADDR    R0,     R4      ; Point to correct row.

        XORI    #$20F0, R1      ; R1 == '>' in desired color.
        MVO@    R1,     R4      ; Output the left part of arrow
        XORI    #$2010, R1      ; R1 == '<' in desired color.
        ADDI    #10,    R4
        MVO@    R1,     R4      ; Output the right part of arrow
        MVII    #$2000, R0
        MVO@    R0,     R4

@@do_stat:
        ; Update the memory location/pass counter/error thingy
        MVI     ARROW,  R4
        ADDI    #ROW_4+14, R4
        MVI     CUR_L,  R0

        MVI     CUR_S,  R1
        DECR    R1
        BMI     @@no_stat
        BEQ     @@pass_count

        MVI     HUPD,   R1
        DECR    R1
        BPL     @@no_hupd

        MVII    #C_TAN, R1
        CALL    HEX16
        MVII    #HCNT,  R1
@@no_hupd:
        MVO     R1,     HUPD
        B       @@done_stat


@@pass_count:
        MVII    #C_TAN, R3
        MVII    #1,     R2
        CALL    DEC16Z

@@noarrow:
@@no_stat:
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

;; ======================================================================== ;;
;;  WAIT -- Do a busy-wait.  Number of ticks is in R0.                      ;;
;; ======================================================================== ;;
WAIT    PROC

        MVO     R0,     BUSY
@@spin: MVI     BUSY,   R0
        DECR    R0
        BPL     @@spin
        JR      R5

        ENDP


;;==========================================================================;;
;; MSG_ERR  -- Display "ERR!"                                               ;;
;;==========================================================================;;
MSG_ERR PROC
        PSHR    R5
        MVII    #C_RED, R1
        CALL    PRINT.S    
ERR:    BYTE    "ERR!", 0
        PULR    PC
        ENDP

;; ======================================================================== ;;
;;  MSG_OK   -- Display "*OK*"                                              ;;
;; ======================================================================== ;;
MSG_OK  PROC
        PSHR    R5
        MVII    #C_YEL, R1
        CALL    PRINT.S    
OK:     BYTE    "*OK*", 0
        PULR    PC
        ENDP
        
;; ======================================================================== ;;
;;  MSG_DONE -- Display "*Done*" and wait for tap on the pad.               ;;
;; ======================================================================== ;;
MSG_DONE PROC
        PSHR    R5
        MVII    #$200 + 11*20 + 7, R4
        MVII    #C_BLU, R1
        CALL    PRINT.S    
        BYTE    "*Done*", 0
        B       WAIT_TAP + 1    ; Wait for a tap on the keypad.
        ENDP

;; ======================================================================== ;;
;;  MSG_MORE -- Display "More..." and wait for tap on the pad.              ;;
;; ======================================================================== ;;
MSG_MORE PROC
        PSHR    R5
        MVII    #$200 + 11*20 + 6, R4
        MVII    #C_RED, R1
        CALL    PRINT.S    
        BYTE    "More...", 0
        B       WAIT_TAP + 1    ; Wait for a tap on the keypad.
        ENDP


;; ======================================================================== ;;
;;  WAITHAND -- Waits for controller to be pressed/released                 ;;
;; ======================================================================== ;;
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

;; ======================================================================== ;;
;;  WAIT_TAP -- Wait for controller to go through press/release cycle.      ;;
;; ======================================================================== ;;
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
;; DISPERRS -- Displays current error count.                                ;;
;;==========================================================================;;
DISPERRS    PROC
        PSHR    R5
        MVII    #C_TAN, R1      ; Tan  
        MVII    #ROW_3, R4      ; Row 2, Col 0
        CALL    PRINT.S    
        BYTE    "Total Errors: ", 0

        MVI     ERRS,   R0
        TSTR    R0
        BNEQ    @@errs
        MVII    #C_YEL, R1
        CALL    PRINT.S    
        BYTE    "None.", 0
        PULR    PC
@@errs:
        MVII    #C_RED, R3
        CLRR    R2
        PULR    R5
        J       DEC16A
        ENDP

;; ======================================================================== ;;
;;  SET_ARRO -- Put arrows next to the current test.  Test # in R0.         ;;
;; ======================================================================== ;;
SET_ARRO    PROC
        PSHR    R5
        PSHR    R0
        PSHR    R1

        CLRR    R5
    
        MVI     ARROW,  R1
        MVO     R5,     ARROW   ; Disable current arrow (if any)

        TSTR    R1              ; Do we have a current blinking arrow?
        BEQ     @@no_cur_arrow  ; No.. do nothing.
        ADDI    #ROW_4+1,R1     ; Yes... erase it.
        MVO@    R5,     R1
        ADDI    #11,    R1
        MVO@    R5,     R1
        INCR    R1
        MVO@    R5,     R1

@@no_cur_arrow:
        TSTR    R0              ; Do we have a new arrow?
        BEQ     @@no_new_arrow  ; No... do nthing

        SLL     R0,     2       ; Yes... find its location on the screen,
        MOVR    R0,     R1      ; ... and store it so the ISR can do the
        SLL     R0,     2       ; ... right thing.
        ADDR    R0,     R1      ; R1 = ARROW * 20

        CLRR    R0
        MVO     R0,     ABLNK   ; Reset the blink timer. (non-interruptible)
        MVO     R1,     ARROW   ; Set the arrow pointer. (non-interruptible)

@@no_new_arrow:
        PULR    R1
        PULR    R0
        PULR    PC              ; Return.
        ENDP


;; ======================================================================== ;;
;;  MAINSCRN -- Displays the main memory-test screen.                       ;;
;; ======================================================================== ;;
MAINSCRN    PROC
        PSHR    R5

        MVII    #1,     R0
        CALL    WAIT

        ; Fill display memory with zeros.
        MVII    #$0F0 - $1C,  R1
        MVII    #$200,  R4
        CALL    FILLZERO


        ;                   1111111111
        ;         01234567890123456789
        ;       +----------------------+
        ;$200  0| > RAM Test: #_____ < |
        ;$214  1|                      |
        ;$228  2| IPT: ##### A: LINEAR |
        ;$23C  3| Total Errors: #####  |  
        ;$250  4|                      |
        ;$264  5|   Fill Const  XXXX   |
        ;$278  6|   Walking 1s  XXXX   |
        ;$28C  7|   Walking 0s  XXXX   |
        ;$2A0  8|   CheckerBrd  XXXX   |
        ;$2B4  9|   Rand Value  XXXX   |
        ;$2C8 10|                      |
        ;$2DC 11| Elapsed: 00:00:00:00 |
        ;       +----------------------+

        MVII    #C_BLU, R1
        MVII    #ROW_B, R4
        CALL    PRINT.S    
        BYTE    "Elapsed:", 0

        MVII    #C_GRN, R1      ; Green
        MVII    #ROW_0, R4      ; Row 0, Col 0
        CALL    PRINT.S    
        BYTE    "> RAM Test: #      <", 0

        SUBI    #7,     R4
        MVI     T_ITER, R0
        CLRR    R2
        MOVR    R1,     R3
        CALL    DEC16A          ; Display Iters per Test

        MVII    #C_TAN, R1      ; Tan  
        MVII    #ROW_2, R4      ; Row 2, Col 0
        CALL    PRINT.S    
        BYTE    "IPT:       A: ", 0

        MVII    #RANDOM,R0
        MVI     APAT,   R2
        TSTR    R2
        BNEQ    @@random
        ADDI    #LINEAR-RANDOM, R0
@@random
        MVII    #C_YEL, R1
        CALL    PRINT.R    

        MVII    #ROW_2 + 5, R4  ; Row 2, Col 5
        MVII    #C_YEL, R3
        MVI     IPT,    R0
        CLRR    R2
        CALL    DEC16A

        CALL    DISPERRS

        JSR     R5,     @@do_loop

        BYTE    "Fill Const", 0
        BYTE    "Walking 1s", 0
        BYTE    "Walking 0s", 0
        BYTE    "CheckerBrd", 0
        BYTE    "Rand Value", 0

@@do_loop:
        MVII    #5,     R2
        MVII    #C_WHT, R1      ; White
        MVII    #ROW_5 + 2, R4  ; Row 5, Col 2
@@loop:
        PSHR    R2
        MOVR    R5,     R0
        CALL    PRINT.R    
        ADDI    #10,    R4
        PULR    R2
        DECR    R2
        BNEQ    @@loop
        
        PULR    PC
        ENDP

RANDOM: BYTE    "RANDOM", 0
LINEAR: BYTE    "LINEAR", 0

;; ======================================================================== ;;
;;  RANDSAVE -- Save the current random-number seed.                        ;;
;; ======================================================================== ;;
RANDSAVE    PROC
        PSHR    R5
        MVI     RNDLO,  R5
        MVO     R5,     SAVLO
        MVI     RNDHI,  R5
        MVO     R5,     SAVHI
        PULR    PC
        ENDP

;; ======================================================================== ;;
;;  RANDRSTR -- Restore the current random-number seed.                     ;;
;; ======================================================================== ;;
RANDRSTR    PROC
        PSHR    R5
        MVI     SAVLO,  R5
        MVO     R5,     RNDLO
        MVI     SAVHI,  R5
        MVO     R5,     RNDHI
        PULR    PC
        ENDP

;; ======================================================================== ;;
;;  RANDADDR -- Return a random address that's within the RAM.              ;;
;; ======================================================================== ;;
RANDADDR    PROC
        PSHR    R5
        MVII    #$10,   R0
        CALL    RAND

        ANDI    #RAMAM, R0
        INCR    PC
@@loop:
        SLR     R0,     1
        CMPI    #RAMSZ, R0
        BGE     @@loop

        ADDI    #RAMLO, R0
        
        PULR    PC
        ENDP

;; ======================================================================== ;;
;;  FAULT  -- Do error report from one of the following tests.              ;;
;; ======================================================================== ;;
FAULT   PROC
        PSHR    R5
        INCR    R3              ; Incr the error count for this function.
        PSHR    R0
        PSHR    R1
        PSHR    R2
        PSHR    R3
        PSHR    R4

        MVI     CUR_S,  R0
        NEGR    R0
        MVO     R0,     CUR_S   ; Disable address display

        DECR    R4
        MOVR    R4,     R0
        MVII    #C_RED, R1
        MVI     ARROW,  R4
        ADDI    #ROW_4 + 14, R4 ; Get correct row for ERR.
        CALL    HEX16

        MVII    #C_RED + 8, R1
        MVO@    R1,     R4      ; Put a red ! after location.

        MVI     ERRS,   R0
        INCR    R0
        MVO     R0,     ERRS
        CALL    DISPERRS        ; Update the error display

        MVII    #EDLY,  R0
        CALL    WAIT            ; Hold this info for 1/2 second.

        MVI     CUR_S,  R0
        NEGR    R0
        MVO     R0,     CUR_S   ; Re-enable address display

        PULR    R4
        PULR    R3
        PULR    R2
        PULR    R1
        PULR    R0
        PULR    PC              ; Continue the test.
        ENDP

;; ======================================================================== ;;
;;  FINISH -- Do final status report from one of following tests.           ;;
;; ======================================================================== ;;
FINISH  PROC
        CLRR    R0
        MVO     R0,     CUR_S

        MVI     ARROW,  R4      ; Get current row
        ADDI    #ROW_4 + 14, R4 ; Display OK or ERR final status
        PULR    R5              ; Get return address.
        TSTR    R3
        BNEQ    MSG_ERR         ; Return through MSG_ERR
        B       MSG_OK          ; Return through MSG_OK
        ENDP


;; ======================================================================== ;;
;;  FILL_L -- Fill Memory Test, Linear. Fills with all 0/1, reads it back   ;;
;; ======================================================================== ;;
FILL_L  PROC
        PSHR    R5

        MVII    #1,     R0
        MVO     R0,     CUR_S   ; Set our status to "Show Pass Number"
        
        CLRR    R3
        CLRR    R1
        MVI     IPT,    R2
@@oloop:
        MVI     IPT,    R0
        SUBR    R2,     R0
        INCR    R0

        MVO     R0,     CUR_L

        MVII    #RAMLO, R4
        MVII    #RAMSZ, R0

        SARC    R0,     1
        BNC     @@f_nobit0
        MVO@    R1,     R4      ; Store 0s or 1s

@@f_nobit0:
        BEQ     @@f_done
        SARC    R0,     1
        BNC     @@f_nobit1
        MVO@    R1,     R4      ; Store 0s or 1s
        MVO@    R1,     R4      ; Store 0s or 1s

@@f_nobit1:
        BEQ     @@f_done
        SARC    R0,     1
        BNC     @@f_nobit2
        MVO@    R1,     R4      ; Store 0s or 1s
        MVO@    R1,     R4      ; Store 0s or 1s
        MVO@    R1,     R4      ; Store 0s or 1s
        MVO@    R1,     R4      ; Store 0s or 1s

        B       @@f_nobit2

@@f_iloop8:     
        MVO@    R1,     R4      ; Store 0s or 1s
        MVO@    R1,     R4      ; Store 0s or 1s
        MVO@    R1,     R4      ; Store 0s or 1s
        MVO@    R1,     R4      ; Store 0s or 1s
        NOP                     ; interruptible! (for STIC)
        MVO@    R1,     R4      ; Store 0s or 1s
        MVO@    R1,     R4      ; Store 0s or 1s
        MVO@    R1,     R4      ; Store 0s or 1s
        MVO@    R1,     R4      ; Store 0s or 1s

        DECR    R0
@@f_nobit2:
        BNEQ    @@f_iloop8

@@f_done:

@@r_oloop:
        MVII    #RAMLO, R4
        MVII    #RAMSZ, R0

        SARC    R0,     1
        BNC     @@r_nobit0
        CMP@    R4,     R1
        BEQ     @@r_ok0
        CALL    FAULT  
@@r_ok0 TSTR    R0
        BEQ     @@r_done

@@r_iloop2:     
        CMP@    R4,     R1      ; Read back 0s or 1s
        BEQ     @@r_ok1
        CALL    FAULT  

@@r_ok1 CMP@    R4,     R1      ; Read back 0s or 1s
        BEQ     @@r_ok2
        CALL    FAULT  

@@r_ok2 DECR    R0
@@r_nobit0:
        BNEQ    @@r_iloop2

@@r_done:

        XORI    #MASK,  R1      ; Do it again with all 1's
        BNEQ    @@oloop

        DECR    R2              ; Repeat for IPT iterations.
        BNEQ    @@oloop

        B       FINISH
        ENDP

;; ======================================================================== ;;
;;  FILL_R -- Fill Memory Test, Random.  Fills with all 0/1, reads it back  ;;
;; ======================================================================== ;;
FILL_R  PROC
        PSHR    R5
        
        MVII    #1,     R0
        MVO     R0,     CUR_S   ; Set our status to "Show Pass Number"

        CLRR    R3
        CLRR    R1
        MVI     IPT,    R2
        MVII    #$10,   R0
        CALL    RAND
        ANDI    #RAMAM, R0
        MVO     R0,     SCRAM
@@oloop:

        MVI     IPT,    R0
        SUBR    R2,     R0
        INCR    R0

        MVO     R0,     CUR_L

        MVII    #RAMLO, R4
        MVII    #RAMSZ, R0

@@f_iloop:      
        MOVR    R4,     R5
        XOR     SCRAM,  R4 
        MVO@    R1,     R4      ; Store 0s or 1s
        MOVR    R5,     R4
        INCR    R4
        DECR    R0
        BNEQ    @@f_iloop

@@f_done:

@@r_oloop:
        MVII    #RAMLO, R4
        MVII    #RAMSZ, R0

@@r_iloop2:     
        MOVR    R4,     R5
        XOR     SCRAM,  R4 
        CMP@    R4,     R1      ; Read back 0s or 1s
        BEQ     @@r_ok
        PSHR    R5
        CALL    FAULT  
        PULR    R5

@@r_ok: MOVR    R5,     R4
        INCR    R4
        DECR    R0
@@r_nobit0:
        BNEQ    @@r_iloop2

@@r_done:

        XORI    #MASK,  R1      ; Do it again with all 1's
        BNEQ    @@oloop

        DECR    R2              ; Repeat for IPT iterations.
        BNEQ    @@oloop

        B       FINISH

        ENDP

;; ======================================================================== ;;
;;  WLK1_L -- Walking 1 Test, Linear.                                       ;;
;; ======================================================================== ;;
WLK1_L  PROC

        PSHR    R5
        CLRR    R3              ; Clear error count

        MVII    #2,     R0
        MVO     R0,     CUR_S   ; Set our status to "Show Address"
        
        MVII    #RAMLO, R2      ; Init pointer to start of RAM
        MVII    #RAMSZ, R0      ; Init outer loop counter.

@@o_loop:
        PSHR    R0

        MOVR    R2,     R0
        MVO     R2,     CUR_L

        MVI     IPT,    R0      ; Get Iters per Test (inner loop count)
@@m_loop:
        MVII    #1,     R1      ; Start off walking 1 pattern.

@@i_loop:
        MVO@    R1,     R2      ; Store the walking-1 pat
        CMP@    R2,     R1      ; Read it back
        BEQ     @@ok            ; Are they the same?
        MOVR    R2,     R4      ; No:  Report a fault.
        INCR    R4
        CALL    FAULT

@@ok:   ADDR    R1,     R1      ; March the walking-1 along.
        ANDI    #MASK,  R1      ; Is it still in our mask?
        BNEQ    @@i_loop        ; Yes:  Keep looping.

        DECR    R0              ; Do this IPT times.
        BNEQ    @@m_loop

@@i_done:
        INCR    R2
        PULR    R0
        DECR    R0
        BNEQ    @@o_loop

        B       FINISH

        ENDP

;; ======================================================================== ;;
;;  WLK1_R -- Walking 1 Test, Random.                                       ;;
;; ======================================================================== ;;
WLK1_R  PROC

        PSHR    R5
        CLRR    R3              ; Clear error count

        MVII    #2,     R0
        MVO     R0,     CUR_S   ; Set our status to "Show Address"
        
        MVII    #RAMLO, R2      ; Init pointer to start of RAM
        MVII    #RAMSZ, R0      ; Init outer loop counter.

@@o_loop:
        PSHR    R0

        CALL    RANDADDR
        MOVR    R0,     R2
        MVO     R0,     CUR_L


        MVI     IPT,    R0      ; Get Iters per Test (inner loop count)
@@m_loop:
        MVII    #1,     R1      ; Start off walking 1 pattern.

@@i_loop:
        MVO@    R1,     R2      ; Store the walking-1 pat
        CMP@    R2,     R1      ; Read it back
        BEQ     @@ok            ; Are they the same?
        MOVR    R2,     R4      ; No:  Report a fault.
        INCR    R4
        CALL    FAULT

@@ok:   ADDR    R1,     R1      ; March the walking-1 along.
        ANDI    #MASK,  R1      ; Is it still in our mask?
        BNEQ    @@i_loop        ; Yes:  Keep looping.

        DECR    R0              ; Do this IPT times.
        BNEQ    @@m_loop

@@i_done:
        INCR    R2
        PULR    R0
        DECR    R0
        BNEQ    @@o_loop

        B       FINISH

        ENDP

;; ======================================================================== ;;
;;  WLK0_L -- Walking 0 Test, Linear.                                       ;;
;; ======================================================================== ;;
WLK0_L  PROC

        PSHR    R5
        CLRR    R3              ; Clear error count

        MVII    #2,     R0
        MVO     R0,     CUR_S   ; Set our status to "Show Address"
        
        MVII    #RAMLO, R2      ; Init pointer to start of RAM
        MVII    #RAMSZ, R0      ; Init outer loop counter.

@@o_loop:
        PSHR    R0

        MVO     R2,     CUR_L


        MVI     IPT,    R0      ; Get Iters per Test (inner loop count)
@@m_loop:
        MVII    #1,     R1      ; Start off walking 0 pattern.

@@i_loop:
        XORI    #MASK,  R1
        MVO@    R1,     R2      ; Store the walking-0 pat
        CMP@    R2,     R1      ; Read it back
        BEQ     @@ok            ; Are they the same?
        MOVR    R2,     R4      ; No:  Report a fault.
        INCR    R4
        CALL    FAULT

@@ok:   XORI    #MASK,  R1
        ADDR    R1,     R1      ; March the walking-1 along.
        ANDI    #MASK,  R1      ; Is it still in our mask?
        BNEQ    @@i_loop        ; Yes:  Keep looping.

        DECR    R0              ; Do this IPT times.
        BNEQ    @@m_loop

        INCR    R2
        PULR    R0
        DECR    R0
        BNEQ    @@o_loop

        B       FINISH

        ENDP

;; ======================================================================== ;;
;;  WLK0_R -- Walking 0 Test, Random.                                       ;;
;; ======================================================================== ;;
WLK0_R  PROC

        PSHR    R5
        CLRR    R3              ; Clear error count
        
        MVII    #2,     R0
        MVO     R0,     CUR_S   ; Set our status to "Show Address"

        MVII    #RAMLO, R2      ; Init pointer to start of RAM
        MVII    #RAMSZ, R0      ; Init outer loop counter.

@@o_loop:
        PSHR    R0

        CALL    RANDADDR
        MOVR    R0,     R2
        MVO     R0,     CUR_L

        MVI     IPT,    R0      ; Get Iters per Test (inner loop count)
@@m_loop:
        MVII    #1,     R1      ; Start off walking 0 pattern.

@@i_loop:
        XORI    #MASK,  R1
        MVO@    R1,     R2      ; Store the walking-0 pat
        CMP@    R2,     R1      ; Read it back
        BEQ     @@ok            ; Are they the same?
        MOVR    R2,     R4      ; No:  Report a fault.
        INCR    R4
        CALL    FAULT

@@ok:   XORI    #MASK,  R1
        ADDR    R1,     R1      ; March the walking-1 along.
        ANDI    #MASK,  R1      ; Is it still in our mask?
        BNEQ    @@i_loop        ; Yes:  Keep looping.

        DECR    R0              ; Do this IPT times.
        BNEQ    @@m_loop

        INCR    R2
        PULR    R0
        DECR    R0
        BNEQ    @@o_loop

        B       FINISH

        ENDP

CKPAT   PROC
        DECLE   $5555
        DECLE   $3333
        DECLE   $0F0F
        DECLE   $00FF
        DECLE   $FFFF
        ENDP

;; ======================================================================== ;;
;;  CHKR_L -- Checkerboard Pattern, Linear.                                 ;;
;; ======================================================================== ;;
CHKR_L  PROC

        PSHR    R5
        CLRR    R3              ; Clear error count

        MVII    #2,     R0
        MVO     R0,     CUR_S   ; Set our status to "Show Address"
        
        MVII    #RAMLO, R2      ; Init pointer to start of RAM
        MVII    #RAMSZ, R0      ; Init outer loop counter.

@@o_loop:
        PSHR    R0

        MVO     R2,     CUR_L


        MVI     IPT,    R0      ; Get Iters per Test (inner loop count)
@@m_loop:
        PSHR    R0
        MVII    #CKPAT, R5      ; Point to the checker pattern.

        B       @@i_first       ; Do first iteration special.

@@i_loop:
        MVO@    R1,     R2      ; Store the checker pattern
        MVO@    R0,     R2      ; Store the inverse checker pattern
        MVO@    R1,     R2      ; Store the checker pattern
        CMP@    R2,     R1      ; Read it back
        BEQ     @@ok1           ; Are they the same?
        MOVR    R2,     R4      ; No:  Report a fault.
        INCR    R4
        CALL    FAULT

@@ok1:
        MVO@    R0,     R2      ; Store the inverse checker pattern
        MVO@    R1,     R2      ; Store the checker pattern
        MVO@    R0,     R2      ; Store the inverse checker pattern
        CMP@    R2,     R0      ; Read it back
        BEQ     @@ok2           ; Are they the same?
        MOVR    R2,     R4      ; No:  Report a fault.
        INCR    R4
        CALL    FAULT

@@ok2:
        PULR    R5
@@i_first:
        MVII    #MASK,  R0
        MOVR    R0,     R1
        AND@    R5,     R1      ; Get next checkerboard pattern
        XORR    R1,     R0      ; Make inverse.
        PSHR    R5              ; 
        BNEQ    @@i_loop        ; Iterate until mask goes away

        DECR    SP              ; Discard saved R5 on TOS.

@@i_done:
        PULR    R0              ; Get outer loop count

        DECR    R0              ; Do this IPT times.
        BNEQ    @@m_loop

        INCR    R2
        PULR    R0
        DECR    R0
        BNEQ    @@o_loop

        B       FINISH

        ENDP

;; ======================================================================== ;;
;;  CHKR_R -- Checkerboard Pattern, Random.                                 ;;
;; ======================================================================== ;;
CHKR_R  PROC

        PSHR    R5
        CLRR    R3              ; Clear error count

        MVII    #2,     R0
        MVO     R0,     CUR_S   ; Set our status to "Show Address"
        
        MVII    #RAMLO, R2      ; Init pointer to start of RAM
        MVII    #RAMSZ, R0      ; Init outer loop counter.

@@o_loop:
        PSHR    R0

        CALL    RANDADDR
        MOVR    R0,     R2
        MVO     R2,     CUR_L


        MVI     IPT,    R0      ; Get Iters per Test (inner loop count)
@@m_loop:
        PSHR    R0
        MVII    #CKPAT, R5      ; Point to the checker pattern.

        B       @@i_first       ; Do first iteration special.

@@i_loop:
        MVO@    R1,     R2      ; Store the checker pattern
        MVO@    R0,     R2      ; Store the inverse checker pattern
        MVO@    R1,     R2      ; Store the checker pattern
        CMP@    R2,     R1      ; Read it back
        BEQ     @@ok1           ; Are they the same?
        MOVR    R2,     R4      ; No:  Report a fault.
        INCR    R4
        CALL    FAULT

@@ok1:
        MVO@    R0,     R2      ; Store the inverse checker pattern
        MVO@    R1,     R2      ; Store the checker pattern
        MVO@    R0,     R2      ; Store the inverse checker pattern
        CMP@    R2,     R0      ; Read it back
        BEQ     @@ok2           ; Are they the same?
        MOVR    R2,     R4      ; No:  Report a fault.
        INCR    R4
        CALL    FAULT

@@ok2:
        PULR    R5
@@i_first:
        MVII    #MASK,  R0
        MOVR    R0,     R1
        AND@    R5,     R1      ; Get next checkerboard pattern
        XORR    R1,     R0      ; Make inverse.
        PSHR    R5              ; 
        BNEQ    @@i_loop        ; Iterate until mask goes away

        DECR    SP              ; Discard saved R5 on TOS.

@@i_done:
        PULR    R0              ; Get outer loop count

        DECR    R0              ; Do this IPT times.
        BNEQ    @@m_loop

        INCR    R2
        PULR    R0
        DECR    R0
        BNEQ    @@o_loop

        B       FINISH

        ENDP

;; ======================================================================== ;;
;;  RAND_L -- Random Value, Linear.                                         ;;
;; ======================================================================== ;;
RAND_L  PROC

        PSHR    R5
        CLRR    R3              ; Clear error count

        MVII    #2,     R0
        MVO     R0,     CUR_S   ; Set our status to "Show Address"
        
        MVII    #RAMLO, R2      ; Init pointer to start of RAM
        MVII    #RAMSZ, R1      ; Init outer loop counter.

@@o_loop:
        PSHR    R1
        MVO     R2,     CUR_L

        MVI     IPT,    R1      ; Get Iters per Test (inner loop count)
@@i_loop:
        MVII    #$10,   R0
        CALL    RAND
        ANDI    #MASK,  R0
        MVO@    R0,     R2      ; Store the checker pattern
        CMP@    R2,     R0      ; Read it back
        BEQ     @@ok            ; Are they the same?
        MOVR    R2,     R4      ; No:  Report a fault.
        INCR    R4
        CALL    FAULT

@@ok:
        DECR    R1              ; 
        BNEQ    @@i_loop        ; Iterate IPT times.

        INCR    R2
        PULR    R1
        DECR    R1
        BNEQ    @@o_loop

        B       FINISH

        ENDP

;; ======================================================================== ;;
;;  RAND_R -- Random Value, Random.                                         ;;
;; ======================================================================== ;;
RAND_R  PROC
        PSHR    R5
        CLRR    R3              ; Clear error count

        MVII    #2,     R0
        MVO     R0,     CUR_S   ; Set our status to "Show Address"
        
        MVII    #RAMLO, R2      ; Init pointer to start of RAM
        MVII    #RAMSZ, R1      ; Init outer loop counter.

@@o_loop:
        PSHR    R1

        CALL    RANDADDR
        MOVR    R0,     R2
        MVO     R2,     CUR_L

        MVI     IPT,    R1      ; Get Iters per Test (inner loop count)
@@i_loop:
        MVII    #$10,   R0
        CALL    RAND
        ANDI    #MASK,  R0
        MVO@    R0,     R2      ; Store the checker pattern
        CMP@    R2,     R0      ; Read it back
        BEQ     @@ok            ; Are they the same?
        MOVR    R2,     R4      ; No:  Report a fault.
        INCR    R4
        CALL    FAULT

@@ok:
        DECR    R1              ; 
        BNEQ    @@i_loop        ; Iterate IPT times.

        INCR    R2
        PULR    R1
        DECR    R1
        BNEQ    @@o_loop

        B       FINISH

        ENDP

;; ======================================================================== ;;
;;  ISRSPIN                                                                 ;;
;;      Sets ISR to the value in R5, and spins waiting for interrupts.      ;;
;;      Note:  Call this with interrupts disabled! (eg. JSRD)               ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R5 -- ISR function address                                          ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      ISR vector set to value in R5.                                      ;;
;;      Does not return.                                                    ;;
;; ======================================================================== ;;
ISRSPIN PROC
        MVII    #$2F0,  R6
        MOVR    R5,     R0
        MVO     R0,     $100
        SWAP    R0
        MVO     R0,     $101

        EIS
@@spinloop:
        DECR    PC
        ENDP

; ------------------------------------------------------------------------- ;
;  Library Includes.                                                        ;
; ------------------------------------------------------------------------- ;
        INCLUDE "../library/ic_banksw.asm"
        INCLUDE "../library/rand.asm"
        INCLUDE "../library/dec16only.asm"
        INCLUDE "../library/print.asm"
        INCLUDE "../library/fillmem.asm"
        INCLUDE "../library/hex16.asm"
