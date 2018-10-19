;;==========================================================================;;
;; Joe Zbiciak's Tap Counting Demo                                          ;;
;; Copyright 2008, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  TO BUILD IN BIN+CFG FORMAT:                                             *;
;*      as1600 -i ../library -o counttap.bin -l counttap.lst counttap.asm   *;
;*                                                                          *;
;*  TO BUILD IN ROM FORMAT:                                                 *;
;*      as1600 -i ../library -o counttap.rom -l counttap.lst counttap.asm   *;
;* ======================================================================== *;

;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;


        ROMW    16              ; Use 16-bit ROM width
        ORG     $5000           ; Use default memory map

;------------------------------------------------------------------------------
; Include system information
;------------------------------------------------------------------------------
        INCLUDE "gimini.asm"

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
        BYTE    108, 'Tap Counting Demo', 0

;; ======================================================================== ;;
;;  MAIN   -- Where our program starts (immediately after title string).    ;;
;; ======================================================================== ;;
MAIN:   MVII    #ISR,   R0
        MVO     R0,     $100
        SWAP    R0
        MVO     R0,     $101
        EIS

        CALL    CLRSCR          ; Clear the screen

        CALL    PRINT.FLS       ; Display our message.
        DECLE   C_YEL           ; Yellow
        DECLE   $200 + 4*20 + 2 ; Row #4, colukmn #2 on screen
              ;01234567890123456789
        STRING  'Controller taps:'
        BYTE    0

        CLRR    R0              ; \
        PSHR    R0              ;  |- Put a 32-bit count of 0 on the 
        PSHR    R0              ; /   top of stack

@@loop:
        CALL    WAITKEY         ; Wait for a tap on the controller

        PULR    R1              ; \_ Get our current count from top of stack
        PULR    R0              ; /

        INCR    R0              ; Add 1 to lower half
        BNEQ    @@nocarry
        INCR    R1              ; Add 1 to upper half if lower half wrapped
@@nocarry

        PSHR    R0              ; \_ Remember current count on stack
        PSHR    R1              ; /

        ; To print the number, we must set up the following:
        ; R1:R0 contains 32-bit count
        ; R2 contains the width of the field 
        ; R3 contains the "format word" (generally just the color)
        ; R4 contains where to display it
        MVII    #8,     R2              ; A nice, ridiculously wide field.
        MVII    #C_WHT, R3              ; Display it in white
        MVII    #$200 + 6*20 + 6, R4    ; Row 6, column 6
        CALL    PRNUM32.z               ; Display with leading zeroes.

        B       @@loop          ; Wait for the next tap

        ENDP

;; ======================================================================== ;;
;;  ISR   -- A simple ISR to keep the screen enabled.                       ;;
;; ======================================================================== ;;
ISR     PROC
        MVO     R0,     $20     ; STIC handshake to keep display enabled

        MVII    #C_BLU, R0
        MVO     R0,     $28     ; Set color stack 0 to blue
        MVO     R0,     $2C     ; Set border to blue

        JR      R5              ; Return
        ENDP

;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
        INCLUDE "print.asm"     ; PRINT.xxx routines
        INCLUDE "prnum32.asm"   ; PRNUM32.x routines (include before PRNUM16)
        INCLUDE "prnum16.asm"   ; PRNUM16.x routines
        INCLUDE "wnk.asm"       ; WAITKEY routine
        INCLUDE "fillmem.asm"   ; CLRSCR/FILLZERO/FILLMEM
