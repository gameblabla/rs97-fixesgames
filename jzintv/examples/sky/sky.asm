;;==========================================================================;;
;; Joe Zbiciak's HELLO WORLD.                                               ;;
;; Copyright 2002, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  TO BUILD IN BIN+CFG FORMAT:                                             *;
;*      as1600 -o hello.bin -l hello.lst hello.asm                          *;
;*                                                                          *;
;*  TO BUILD IN ROM FORMAT:                                                 *;
;*      as1600 -o hello.rom -l hello.lst hello.asm                          *;
;* ======================================================================== *;

        ROMW    16              ; Use standard GI 10-bit ROM width
        ORG     $5000           ; Use default memory map

;------------------------------------------------------------------------------
; Include system information
;------------------------------------------------------------------------------
        INCLUDE "../library/gimini.asm"

;------------------------------------------------------------------------------
; System variables
;------------------------------------------------------------------------------
DELAY   EQU     $103            ; Delay until next scrolling colors update
MCOLOR  EQU     $104            ; Starting color for next update


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
ONES:   DECLE   C_CYN, C_CYN    ; Initial color stack 0 and 1: Cyan
        DECLE   C_CYN, C_CYN    ; Initial color stack 2 and 3: Cyan
        DECLE   C_CYN           ; Initial border color: Cyan
;------------------------------------------------------------------------------


TITLE:  BYTE    102, 'Sky Shading', 0
;; ======================================================================== ;;
;;  MAIN:  Here's our main program code.                                    ;;
;; ======================================================================== ;;
MAIN:   PROC

        CALL    CLRSCR          ; Clear the screen
        MVII    #INITISR, R0
        MVO     R0,     $100
        SWAP    R0
        MVO     R0,     $101

        CALL    PRINT.FLS
        DECLE   240*8 + C_BLU, $200
                ;01234567890123456789
        STRING  "00000000000000000000"
        STRING  "44444444444444444444"
        STRING  "11111111111111111111"
        STRING  "33333333333333333333", 0

        CALL    PRINT.FLS
        DECLE   240*8 + C_WHT, $200 + 5*20
                ;01234567890123456789
        STRING  "33333333333333333333", 0

        CALL    PRINT.FLS
        DECLE   240*8 + C_TAN, $200 + 6*20
                ;01234567890123456789
        STRING  "22222222222222222222"
        STRING  "00000000000000000000"
        STRING  "00000000000000000000"
        STRING  "44444444444444444444"
        STRING  "11111111111111111111"
        STRING  "33333333333333333333", 0

        MVI     $200 + 6*20, R0
        XORI    #$2000, R0
        MVO     R0,     $200 + 6*20

        CLRR    R0
        MVO     R0,     DELAY

        EIS

        DECR    PC              ; spin forever

        ENDP


;; ======================================================================== ;;
;;  INITISR -- Initialization ISR that copies our GRAM image over and       ;;
;;             sets up the MOBs.                                            ;;
;; ======================================================================== ;;
INITISR PROC
        BEGIN

        MVII    #ISR,   R0
        MVO     R0,     $100
        SWAP    R0
        MVO     R0,     $101

        ; Copy our bitmaps to GRAM.
        CALL    MEMUNPK
        DECLE   $3800,  CHARS,  CHARS.end-CHARS

        ; Copy our MOB information to the STIC
        CALL    MEMCPY
        DECLE   $0,  MOBINIT, MOBINIT.end - MOBINIT

        RETURN
        ENDP

;; ======================================================================== ;;
;;  ISR:  Simple ISR to set the colorstack etc. as we like it               ;;
;; ======================================================================== ;;
ISR     PROC
        BEGIN

        MVI     STIC.mode, R0       ; color stack mode
        MVO     R0, STIC.viden      ; enable display

        MVII    #C_CYN, R0          ; Cyan for first color stack entry
        MVO     R0,     STIC.cs0
        MVII    #C_BRN, R0          ; Brown for second color stack entry
        MVO     R0,     STIC.cs1
        CLRR    R0
        MVO     R0,     STIC.bord   ; Black border

        MVI     DELAY,  R0
        DECR    R0
        BPL     @@delay_ok

        MVII    #$10,   R3          ; Point to attribute registers
        MVI     MCOLOR, R0
        INCR    R0
        ANDI    #$F,    R0
        MVO     R0,     MCOLOR
        MVII    #8,     R1
@@color_loop:
        MVI@    R3,     R4
        ANDI    #$EFF8, R4          ; clear out the color
        MOVR    R0,     R2
        ADDI    #$0FF8, R2          ; Fix up pastel color numbers
        ANDI    #$1007, R2          ;
        XORR    R2,     R4          ; merge in new color
        MVO@    R4,     R3          ; store it
        INCR    R3                  ; go to next MOB
        INCR    R0
        ANDI    #$F,    R0          ; go to next color
        DECR    R1                  ; 
        BNEQ    @@color_loop


        MVII    #1,      R0
@@delay_ok:
        MVO     R0,     DELAY
        RETURN
        ENDP


;; ======================================================================== ;;
;;  MOBINIT -- Settings for the 8 MOBs at startup.                          ;;
;; ======================================================================== ;;
MOBINIT PROC
@@cx    EQU     80
@@cy    EQU     50 - 16
@@col   EQU     X_PUR

        ; X Registers
        DECLE   STIC.mobx_visb + STIC.mobx_xsize + @@cx - 70
        DECLE   STIC.mobx_visb + STIC.mobx_xsize + @@cx - 50
        DECLE   STIC.mobx_visb + STIC.mobx_xsize + @@cx - 30
        DECLE   STIC.mobx_visb + STIC.mobx_xsize + @@cx - 10
        DECLE   STIC.mobx_visb + STIC.mobx_xsize + @@cx + 10
        DECLE   STIC.mobx_visb + STIC.mobx_xsize + @@cx + 30
        DECLE   STIC.mobx_visb + STIC.mobx_xsize + @@cx + 50
        DECLE   STIC.mobx_visb + STIC.mobx_xsize + @@cx + 70

        ; Y Registers
        DECLE   STIC.moby_yres + STIC.moby_ysize8 + @@cy
        DECLE   STIC.moby_yres + STIC.moby_ysize8 + @@cy
        DECLE   STIC.moby_yres + STIC.moby_ysize8 + @@cy
        DECLE                    STIC.moby_ysize8 + @@cy + 16
        DECLE   STIC.moby_yres + STIC.moby_ysize8 + @@cy
        DECLE   STIC.moby_yres + STIC.moby_ysize8 + @@cy
        DECLE   STIC.moby_yres + STIC.moby_ysize8 + @@cy
        DECLE   STIC.moby_yres + STIC.moby_ysize8 + @@cy

        ; A Registers
        DECLE   STIC.moba_gram +  6*8 + @@col
        DECLE   STIC.moba_gram +  8*8 + @@col
        DECLE   STIC.moba_gram + 10*8 + @@col
        DECLE   STIC.moba_gram +  5*8 + @@col
        DECLE   STIC.moba_gram + 12*8 + @@col
        DECLE   STIC.moba_gram + 14*8 + @@col
        DECLE   STIC.moba_gram + 16*8 + @@col
        DECLE   STIC.moba_gram + 16*8 + @@col

@@end:  
        ENDP

;; ======================================================================== ;;
;;  CHARS -- Character pictures to copy to GRAM.                            ;;
;; ======================================================================== ;;
CHARS   PROC

        ; ########
        ; ########
        ; ########
        ; ########
        ; ########
        ; ########
        ; ########
        ; ########
@@0:    DECLE   $FFFF, $FFFF, $FFFF, $FFFF
    
        ; .#.#.#.#
        ; #.#.#.#.
        ; .#.#.#.#
        ; #.#.#.#.
        ; .#.#.#.#
        ; #.#.#.#.
        ; .#.#.#.#
        ; #.#.#.#.
@@1:    DECLE   $AA55, $AA55, $AA55, $AA55
           
        ; ######## FF
        ; ######## FF
        ; .###.### 77
        ; ##.##.#. DA
        ; .#.#.#.# 55
        ; #.#.#.#. AA
        ; ...#...# 11
        ; ........ 00
@@2:    DECLE   $FFFF XOR $FFFF, $FFFF XOR $DA77, $FFFF XOR $AA55, $FFFF XOR $0011

        ; ........
        ; #...#...
        ; ........
        ; ..#...#.
        ; ........
        ; #...#...
        ; ........
        ; ..#...#.
@@3:    DECLE   $8800, $2200, $8800, $2200

        ; ########
        ; .###.###
        ; ########
        ; ##.###.#
        ; ########
        ; .###.###
        ; ########
        ; ##.###.#
@@4:    DECLE   $77FF, $DDFF, $77FF, $DDFF

        ; ........  00
        ; ########  FF
        ; ########  FF
        ; ........  00
        ; ........  00
        ; ........  00
        ; ........  00
        ; ........  00
@@5:    DECLE   $FF00,  $00FF, $0000, $0000


        ; .######.  7E
        ; ########  FF
        ; ##....##  C3
        ; .##.....  60
        ; ..##....  30
        ; ...##...  18
        ; ....##..  0C
        ; .....##.  06

        ; ##....##  C3
        ; ########  FF
        ; .######.  7E
        ; ........  00
        ; ........  00
        ; ........  00
        ; ........  00
        ; ........  00
@@6:    DECLE   $FF7E, $60C3, $1830, $060C
        DECLE   $FFC3, $007E, $0000, $0000

        ; ####....  F0
        ; #####...  F8
        ; .##.##..  6C
        ; .##..##.  66
        ; .##...##  63
        ; .##...##  63
        ; .##...##  63
        ; .##..##.  66

        ; .##.##..  6C
        ; #####...  F8
        ; ####....  F0
        ; ........  00
        ; ........  00
        ; ........  00
        ; ........  00
        ; ........  00
@@8:    DECLE   $F8F0, $666C, $6363, $6663
        DECLE   $F86C, $00F0, $0000, $0000

        ; ##....##  C3
        ; ##...##.  C6
        ; ##..##..  CC
        ; ##.##...  D8
        ; ####....  F0
        ; ###.....  E0
        ; ####....  F0
        ; ##.##...  D8

        ; ##..##..  CC
        ; ##...##.  C6
        ; ##....##  C3
        ; ........  00
        ; ........  00
        ; ........  00
        ; ........  00
        ; ........  00
@@10:   DECLE   $C6C3, $D8CC, $E0F0, $D8F0
        DECLE   $C6CC, $00C3, $0000, $0000

        ; #####...  F8
        ; #####...  F8
        ; ...##...  18
        ; ...##...  18
        ; ...##...  18
        ; ...##...  18
        ; ...##.##  1B
        ; ...##.##  1B

        ; ...##.##  1B
        ; ########  FF
        ; ########  FF
        ; ........  00
        ; ........  00
        ; ........  00
        ; ........  00
        ; ........  00
@@12:   DECLE   $F8F8, $1818, $1818, $1B1B
        DECLE   $FF1B, $00FF, $0000, $0000

        ; ####....  F0
        ; ####....  F0
        ; ##......  C0
        ; ##......  C0
        ; ##......  C0
        ; ##......  C0
        ; ########  FF
        ; ########  FF

        ; ##....##  C3
        ; ########  FF
        ; ########  FF
        ; ........  00
        ; ........  00
        ; ........  00
        ; ........  00
        ; ........  00
@@14:   DECLE   $F0F0, $C0C0, $C0C0, $FFFF
        DECLE   $FFC3, $00FF, $0000, $0000

        ; .######.  7E
        ; ########  FF
        ; ##....##  C3
        ; ##....##  C3
        ; ##....##  C3
        ; ##....##  C3
        ; ##....##  C3
        ; ##....##  C3
        ; ##....##  C3
        ; ########  FF
        ; .######.  7E
        ; ........  00
        ; ........  00
        ; ........  00
        ; ........  00
        ; ........  00
@@16:   DECLE   $FF7E, $C3C3, $C3C3, $C3C3
        DECLE   $FFC3, $007E, $0000, $0000


@@end
        ENDP
        
;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
        INCLUDE "../library/print.asm"       ; PRINT.xxx routines
        INCLUDE "../library/fillmem.asm"     ; CLRSCR/FILLZERO/FILLMEM
        INCLUDE "../library/memunpk.asm"     ; MEMUNPK
        INCLUDE "../library/memcpy.asm"      ; MEMCPY

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


