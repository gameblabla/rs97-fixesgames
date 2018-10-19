
        ROMW    16              ; Use 16-bit ROM width
        ORG     $5000           ; Use default memory map

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


;; ======================================================================== ;;
;;  TITLE  -- Display our modified title screen & copyright date.           ;;
;; ======================================================================== ;;
TITLE:  PROC
        BYTE    106, 'Loopy McLoopback', 0
MAIN:   BEGIN

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

        CALL    CC3_SERINIT
@@loop:
        CALL    CC3_RECV
        CALL    CC3_XMIT
        B       @@loop

        ; Done.
        RETURN                  ; Return to EXEC for title screen display
        ENDP

        INCLUDE "cc3serial.asm"


ISR     PROC 
        MVO     R0,     $20
        JR      R5
        ENDP

;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
        INCLUDE "../library/print.asm"       ; PRINT.xxx routines
        INCLUDE "../library/fillmem.asm"     ; CLRSCR/FILLZERO/FILLMEM
        INCLUDE "../library/hex16.asm"         

