;------------------------------------------------------------------------------
;   Simple serial terminal 
;   Runs at 9600, 8-N-1
;   Requires a CC3
;
;   To build:
;       as1600 -c -o terminal -l terminal.lst terminal.asm
;------------------------------------------------------------------------------


;------------------------------------------------------------------------------
; Set up the cartridge
;------------------------------------------------------------------------------
        INCLUDE "../macro/cart.mac"
        INCLUDE "../library/gimini.asm"

        REQ_ECS
        ROMSETUP 42K, 2010, "CC3 Serial Demo", MAIN, 48

        INCLUDE "../cc3serial/cc3serial.asm"
        INCLUDE "../ecs_kbd/scan_kbd.asm"
        INCLUDE "ansi.asm"

;------------------------------------------------------------------------------
MAIN:   PROC


        MVII    #INITISR,   R0
        MVO     R0,     $100
        SWAP    R0
        MVO     R0,     $101

        EIS

        CALL    CC3_SERINIT
        MVII    #9600/2,    R0
        CALL    CC3_SETBAUD
        CALL    CLRSCR
        CALL    A_INIT
@@loop:
;       CALL    CC3_PUMP_RX
        CALL    CC3_RECV
;       BNC     @@disp

@@disp  MVII    #@@loop,    R5
        MVI     A_interp,   PC
;       B       @@loop

        ENDP

        BYTEVAR BLINKER

INITISR PROC
        DIS
        MVII    #$3200, R4
        MVII    #$3800, R5
        MVII    #$200,  R1
@@loop: MVI@    R4,     R0
        MVO@    R0,     R5
        DECR    R1
        BNEQ    @@loop
        
        MVII    #ISR,   R0
        MVO     R0,     $100
        SWAP    R0
        MVO     R0,     $101
        EIS

        MVII    #$AA,   R0
        MVII    #$55,   R1
        MVII    #$39F0, R4

        REPEAT  8
        MVO@    R0,     R4
        MVO@    R1,     R4
        ENDR

        B       $1014
        ENDP

ISR     PROC 
        MVO     R0,     $20
        
        MVII    #1,     R0
        MVO     R0,     $28
        MVO     R0,     $2C

        MVO     R0,     $21     ; fgbg mode

        MVI     A_r,    R0
        SLL     R0,     2
        SLL     R0,     1
        ADDI    #$88,   R0
        MVO     R0,     $08

        MVI     A_c,    R0
        SLL     R0,     2
        SLL     R0,     1
        ADDI    #$208,  R0
        MVO     R0,     $00

        MVI     BLINKER,R0
        INCR    R0
        MVO     R0,     BLINKER
        
        SLR     R0,     2
        ANDI    #$F,    R0
        ADDI    #$FF8,  R0
        ANDI    #$1007, R0
        ADDI    #$29F0, R0
        MVO     R0,     $10

;       CALL    CC3_PUMP_RX

        CALL    SCAN_KBD
        CMPI    #KEY.NONE,  R0
        BEQ     @@noxmit
        
        CALL    CC3_XMIT
@@noxmit
        B       $1014

;       MVII    #$1014, R5
;       B       CC3_PUMP_RX
        ENDP

;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
        INCLUDE "../library/fillmem.asm"  ; CLRSCR/FILLZERO/FILLMEM

