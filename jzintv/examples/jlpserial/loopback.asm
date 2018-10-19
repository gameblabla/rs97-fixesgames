
        ROMW    16              ; Use 16-bit ROM width
        ORG     $5000           ; Use default memory map

FRAMING     EQU $35A
PARITY      EQU $35B
OVERRUN     EQU $35C
OK_COUNT    EQU $35D
ROTATE      EQU $103

LAST_RXS    EQU $350
LAST_RX     EQU $351
LAST_TXS    EQU $352
LAST_TX     EQU $353

;------------------------------------------------------------------------------
; Include system information
;------------------------------------------------------------------------------
        INCLUDE "../library/gimini.asm"

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
ONES:   DECLE   C_BLU, C_BLU    ; Initial color stack 0 and 1: Blue
        DECLE   C_BLU, C_BLU    ; Initial color stack 2 and 3: Blue
        DECLE   C_BLU           ; Initial border color: Blue
;------------------------------------------------------------------------------

        INCLUDE "jlpserial.asm"

;; ======================================================================== ;;
;;  TITLE  -- Display our modified title screen & copyright date.           ;;
;; ======================================================================== ;;
TITLE:  PROC
        BYTE    106, 'Loopy McLoopback', 0
MAIN:   
        ; Patch the title string to say '=JRMZ=' instead of Mattel.
        CALL    PRINT.FLS       ; Write string (ptr in R5)
        DECLE   C_WHT, $23D     ; White, Point to 'Mattel' in top-left
        STRING  '=JRMZ='        ; Guess who?  :-)
        STRING  ' Productions' 
        BYTE    0

        CALL    PRINT.FLS       ; Write string (ptr in R1)
        DECLE   C_WHT, $2D0     ; White, Point to 'Mattel' in lower-right
        STRING  '2006 =JRMZ='   ; Guess who?  :-)
        BYTE    0

        MVII    #ISR,   R0
        MVO     R0,     $100
        SWAP    R0
        MVO     R0,     $101

        EIS

        CLRR    R0
        MVO     R0,     OK_COUNT
        MVO     R0,     ROTATE  
        MVO     R0,     FRAMING
        MVO     R0,     PARITY
        MVO     R0,     OVERRUN


        CALL    JLP_SERINIT

        MVI     JLP_UART.baud, R0
        MVII    #7,     R3
        MVII    #$205,  R4
        CALL    PRNUM16.l

        MVII    #$87,   R0
        MVO@    R0,     R4
        MVO@    R0,     R4
        
        CLRR    R0
        MVO@    R0,     R4

        MVII    #(('b' - $20) SHL 3) OR 7, R0
        MVO@    R0,     R4
        MVII    #(('p' - $20) SHL 3) OR 7, R0
        MVO@    R0,     R4
        MVII    #(('s' - $20) SHL 3) OR 7, R0
        MVO@    R0,     R4

;@@loop:
;        CALL    JLP_RECV
;        CALL    JLP_XMIT
;        B       @@loop

        MVII    #JLP_UART.rx, R1
        MVII    #JLP_UART.tx, R2
        CLRR    R3

@@loop: CLRR    R0
@@rx:   ADD@    R1,         R0
        BEQ     @@rx
        
@@tx:   CMP@    R2,         R3
        BNEQ    @@tx

        MVO@    R0,         R2
        B       @@loop


        TSTR    R1
        BEQ     @@no_error

        MVI     OVERRUN,    R2
        SARC    R1
        ADCR    R2
        MVO     R2,         OVERRUN

        MVI     FRAMING,    R2
        SARC    R1
        ADCR    R2
        MVO     R2,         FRAMING

        MVI     PARITY,     R2
        SARC    R1
        ADCR    R2
        MVO     R2,         PARITY
        
@@xmit: CALL    JLP_XMIT
        B       @@loop

@@no_error:
        MVI     OK_COUNT,   R2
        INCR    R2
        MVO     R2,     OK_COUNT
        CALL    JLP_XMIT
        B       @@loop


@@sb_fail: CALL    PRINT.FLS
        DECLE   7, $200, "SETBAUD Failed", 0

        ; Done.
        DECR    PC
        ENDP


ISR     PROC 
        MVO     R0,     $20
        JR      R5

;       MVI     ROTATE, R1
;       INCR    R1
;       MVO     R1,     ROTATE
;       ANDI    #3,     R1
;       ADDI    #@@table, R1
;       MVI@    R1,     PC
;@@table:
;       DECLE   @@a, @@b, @@c, @@d;


@@a:;   MVI     OK_COUNT,R0
        MVI     LAST_RXS, R0
        MVII    #7,     R1
        MVII    #$200,  R4
        CALL    HEX16
;       B       $1014

@@b:  ; MVI     FRAMING,  R0
      ; MVI     LAST_RX, R0
;       MVI     JLP_UART.mode, R0
        MVII    #7,     R1
        MVII    #$205,  R4
        CALL    HEX16
;       B       $1014

@@c:  ; MVI     PARITY,   R0
      ; MVI     LAST_TXS, R0
;       MVI     JLP_UART.stat, R0
        MVII    #7,     R1
        MVII    #$20A,  R4
        CALL    HEX16
;       B       $1014

@@d:  ; MVI     OVERRUN, R0
      ; MVI     LAST_TX, R0
        MVI     JLP_UART.baud, R0
        MVII    #7,     R1
        MVII    #$20F,  R4
        CALL    HEX16

        MVI     OK_COUNT,R0
        MVII    #7,     R1
        MVII    #$214,  R4
        CALL    HEX16

        MVI     FRAMING,  R0
        MVII    #7,     R1
        MVII    #$219,  R4
        CALL    HEX16

        MVI     PARITY,   R0
        MVII    #7,     R1
        MVII    #$21E,  R4
        CALL    HEX16

        MVI     OVERRUN, R0
        MVII    #7,     R1
        MVII    #$223,  R4
        CALL    HEX16

        MVI     LAST_TX,R0
        MVII    #7,     R1
        MVII    #$228,  R4
        CALL    HEX16

        MVI     LAST_RX,  R0
        MVII    #7,     R1
        MVII    #$22D,  R4
        CALL    HEX16

        B       $1014

        ENDP

;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
        INCLUDE "../library/print.asm"       ; PRINT.xxx routines
        INCLUDE "../library/fillmem.asm"     ; CLRSCR/FILLZERO/FILLMEM
        INCLUDE "../library/hex16.asm"         
        INCLUDE "../library/prnum16.asm"


