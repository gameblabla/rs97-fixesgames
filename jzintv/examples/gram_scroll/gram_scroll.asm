;;==========================================================================;;
;; Joe Zbiciak's GRAM-based Scrolling Demo #1:  Small Squares               ;;
;; Copyright 2002, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  TO BUILD IN BIN+CFG FORMAT:                                             *;
;*      as1600 -o gram_scroll.bin -l gram_scroll.lst gram_scroll.asm        *;
;*                                                                          *;
;*  TO BUILD IN ROM FORMAT:                                                 *;
;*      as1600 -o gram_scroll.rom -l gram_scroll.lst gram_scroll.asm        *;
;* ======================================================================== *;

            ROMW    16              ; Use 16-bit ROM

;------------------------------------------------------------------------------
; Include system information
;------------------------------------------------------------------------------
            INCLUDE "../library/gimini.asm"

;------------------------------------------------------------------------------
; Declare variable in scratch (8-bit) and system (16-bit) RAM
; The "-RWBN" specifies that the Intellicart should not map this region.
;------------------------------------------------------------------------------
SCRATCHRAM  ORG     $100,   $100,   "-RWBN"  ; declare vars in scratch RAM
ISRVEC      RMB     2
_EOSCRATCH  EQU     $

SYSTEMRAM   ORG     $2F0,   $2F0,   "-RWBN"  ; declare vars in system RAM
STACK       RMB     32      ; reserve 32 words for stack
XVEL        RMB     1
YVEL        RMB     1
XPOS        RMB     1
YPOS        RMB     1
RNDLO       RMB     1
RNDHI       RMB     1
_EOSYSTEM   EQU     $


            ORG     $5000           ; Use default memory map
;------------------------------------------------------------------------------
; EXEC-friendly ROM header.
;------------------------------------------------------------------------------
ROMHDR:     BIDECLE ZERO            ; MOB picture base   (points to NULL list)
            BIDECLE ZERO            ; Process table      (points to NULL list)
            BIDECLE INIT            ; Program start address
            BIDECLE ZERO            ; Bkgnd picture base (points to NULL list)
            BIDECLE ONES            ; GRAM pictures      (points to NULL list)
            BIDECLE TITLE           ; Cartridge title/date
            DECLE   $03C0           ; No ECS title, run code after title,
                                    ; ... no clicks
ZERO:       DECLE   $0000           ; Screen border control
            DECLE   $0000           ; 0 = color stack, 1 = f/b mode
ONES:       DECLE   C_BLU, C_BLU    ; Initial color stack 0 and 1: Blue
            DECLE   C_BLU, C_BLU    ; Initial color stack 2 and 3: Blue
            DECLE   C_BLU           ; Initial border color: Blue
;------------------------------------------------------------------------------


;; ======================================================================== ;;
;;  TITLE  -- Display our modified title screen & copyright date.           ;;
;; ======================================================================== ;;
TITLE:      PROC
            BYTE    102, 'GRAM Scroll #1', 0
            BEGIN
          
            ; Patch the title string to say '=JRMZ=' instead of Mattel.
            CALL    PRINT.FLS       ; Write string (ptr in R5)
            DECLE   C_WHT, $23D     ; White, Point to 'Mattel' in top-left
            STRING  '=JRMZ='        ; Guess who?  :-)
            STRING  ' Productions' 
            BYTE    0
          
            CALL    PRINT.FLS       ; Write string (ptr in R1)
            DECLE   C_WHT, $2D0     ; White, Point to 'Mattel' in lower-right
            STRING  '2002 =JRMZ='   ; Guess who?  :-)
            BYTE    0
          
            ; Done.
            RETURN                  ; Return to EXEC for title screen display
            ENDP



;;==========================================================================;;
;; INIT -- initialize the main program                                      ;;
;;==========================================================================;;
INIT:       PROC

            ; Set the ISR to the rest of our initialization
            MVII    #MAIN,  R0          ;\
            MVO     R0,     ISRVEC      ; |_ Continue initialization inside
            SWAP    R0                  ; |  MAIN when the first interrupt
            MVO     R0,     ISRVEC+1    ;/   happens.

            EIS
@@spin:     DECR    PC
            ENDP

;;==========================================================================;;
;; MAIN -- The main program                                                 ;;
;;==========================================================================;;
MAIN:       PROC

            ; ============================================================= ;
            ;  Set up everything, and jettison EXEC support.                ;
            ; ============================================================= ;
            DIS
            MVII    #STACK, R6

            ; ------------------------------------------------------------- ;
            ;  Clear the screen to black and enable the display.            ;
            ; ------------------------------------------------------------- ;
            MVO     R0,     $20
            CLRR    R0
            MVO     R0,     $28
            MVO     R0,     $29
            MVO     R0,     $2A
            MVO     R0,     $2B
            MVO     R0,     $2C

            ; ------------------------------------------------------------- ;
            ;  Initialize random number generator from contents of RAM.     ;
            ; ------------------------------------------------------------- ;
            MVII    #$100,  R4
            MVII    #$130,  R3
@@rnd:     
            ADD@    R4,     R0
            ADD@    R4,     R1
            DECR    R3
            BNEQ    @@rnd
           
            TSTR    R0
            BNEQ    @@r0_ok
            MOVR    PC,     R0
@@r0_ok:
            TSTR    R1
            BNEQ    @@r1_ok
            MOVR    PC,     R1
@@r1_ok:
            MVO     R0,     RNDLO
            MVO     R1,     RNDHI


            ; ------------------------------------------------------------- ;
            ;  Set up the interrupt service routine.                        ;
            ; ------------------------------------------------------------- ;
            MVII    #ISR,   R0
            MVO     R0,     ISRVEC
            SWAP    R0
            MVO     R0,     ISRVEC+1

            ; ------------------------------------------------------------- ;
            ;  Fill the screen with the grid.                               ;
            ; ------------------------------------------------------------- ;
            MVII    #$0801, R0
            MVII    #$00F0, R1
            MVII    #$0200, R4
            CALL    FILLMEM

            CALL    PRINT.FLS
            DECLE   X_GRN,  $200 + 6*20 + 3
            STRING  "GRAM Scrolling", 0
            MVII    #$801,  R0
            MVO     R0,     $200 + 6*20 + 7
           
            ; ------------------------------------------------------------- ;
            ;  Initial scroll velocity is 0.0 for both X and Y.             ;
            ; ------------------------------------------------------------- ;
            CLRR    R0
            MVO     R0,     XVEL
            MVO     R0,     YVEL

            EIS

            ; ============================================================= ;
            ;  MAIN LOOP                                                    ;
            ;   -- Display current velocity and "position".                 ;
            ;   -- Cycle the random number generator.                       ;
            ;  Everything else happens in the ISR.                          ;
            ; ============================================================= ;
@@loop:     
            MVI     XVEL,   R0
            MVII    #C_YEL, R1
            MVII    #$200,  R4
            CALL    HEX16

            MVI     YVEL,   R0
            MVII    #C_YEL, R1
            MVII    #$205,  R4
            CALL    HEX16

            MVI     XPOS,   R0
            MVII    #C_YEL, R1
            MVII    #$20A,  R4
            CALL    HEX16

            MVI     YPOS,   R0
            MVII    #C_YEL, R1
            MVII    #$20F,  R4
            CALL    HEX16

            MVII    #$10,   R2
@@rloop:
            MVII    #$10,   R0
            CALL    RAND
            DECR    R2
            BNEQ    @@rloop

            B       @@loop

            ENDP

;; ======================================================================== ;;
;;  SHIFT_DAT                                                               ;;
;;  This array just contains 1 SHL x, for 0 <= x <= 7.                      ;;
;; ======================================================================== ;;
SHIFT_DAT   PROC
            DECLE   00000001b
            DECLE   00000010b
            DECLE   00000100b
            DECLE   00001000b
            DECLE   00010000b
            DECLE   00100000b
            DECLE   01000000b
            DECLE   10000000b
            ENDP

;; ======================================================================== ;;
;;  ISR                                                                     ;;
;;  This is where all the magic happens.                                    ;;
;;   -- Update the horizontal and vertical velocities randomly              ;;
;;   -- Update our horizontal and vertical positions                        ;;
;;   -- Update the GRAM picture based on the horiz/vert position.           ;;
;; ======================================================================== ;;
ISR         PROC
            PSHR    R5

            ; ============================================================= ;
            ;  Make sure display is enabled.                                ;
            ; ============================================================= ;
            MVO     R0,     $20     

            ; ============================================================= ;
            ;  Update our X velocity.  Clamp the velocity to +/- 512.       ;
            ; ============================================================= ;
            MVI     XVEL,   R1
            MVI     RNDLO,  R0
            MOVR    R0,     R3
            
            ; ------------------------------------------------------------- ;
            ;   Take the four LSBs of the random number as our X update.    ;
            ;   Bit 0 determines + or -, bits 1 - 3 add to the XVEL.        ;
            ; ------------------------------------------------------------- ;
            SARC    R0,     1
            ANDI    #7,     R0
            BNC     @@n0
            NEGR    R0             
@@n0:       ADDR    R0,     R1

            ; ------------------------------------------------------------- ;
            ;  Clamp to +/- 512 range.                                      ;
            ; ------------------------------------------------------------- ;
            CMPI    #$01FF, R1
            BLT     @@okx1
            MVII    #$01FF, R1
            B       @@okx   
@@okx1:     CMPI    #$FE00, R1
            BGT     @@okx
            MVII    #$FE00, R1
@@okx:
            MVO     R1,     XVEL

            ; ============================================================= ;
            ;  Update our Y velocity.  Clamp the velocity to +/- 512.       ;
            ; ============================================================= ;
            MVI     YVEL,   R2

            ; ------------------------------------------------------------- ;
            ;   Take the next four bits of the random number as our Y       ;
            ;   update.  Bit 4 determines + or -, bits 5 - 7 add to the     ;
            ;   YVEL.                                                       ;
            ; ------------------------------------------------------------- ;
            SLR     R3,     2
            SLR     R3,     2
            SARC    R3,     1
            ANDI    #7,     R3
            BNC     @@n1
            NEGR    R3
@@n1:       ADDR    R3,     R2

            ; ------------------------------------------------------------- ;
            ;  Clamp to +/- 512 range.                                      ;
            ; ------------------------------------------------------------- ;
            CMPI    #$01FF, R2
            BLT     @@oky1
            MVII    #$01FF, R2
            B       @@oky   
@@oky1      CMPI    #$FE00, R2
            BGT     @@oky
            MVII    #$FE00, R2
@@oky:
            MVO     R2,     YVEL



            ; ============================================================= ;
            ;  Update our position based on velocity.                       ;
            ; ============================================================= ;
            ADD     XPOS,   R1
            MVO     R1,     XPOS
            ADD     YPOS,   R2
            MVO     R2,     YPOS

            ; ============================================================= ;
            ;  The lower byte of the position is our "fraction."  The       ;
            ;  upper byte gives us the integer portion of the position.     ;
            ;  We look at the 3 LSBs of the integer to determine the H and  ;
            ;  V offset.                                                    ;
            ; ============================================================= ;
            SWAP    R1,     1
            ANDI    #7,     R1      ; Horizontal offset 0..7 
            SWAP    R2,     1
            ANDI    #7,     R2      ; Vertical offset 0..7 

            ; ============================================================= ;
            ;  Generate a picture in GRAM that reflects our new offsets.    ;
            ; ============================================================= ;
            ADDI    #SHIFT_DAT, R1  ; Get shifted pixel for column
            MVI@    R1,     R0      ; 

            MVII    #$3800, R4      ; Point to GRAM card #0.
            MVO@    R0,     R4      ; \                             
            MVO@    R0,     R4      ;  |__ Draw first part of column
            MVO@    R0,     R4      ;  |                            
            MVO@    R0,     R4      ; /                             
            MVII    #$FF,   R1      ; Moved here for interruptibility
            MVO@    R0,     R4      ; \                             
            MVO@    R0,     R4      ;  |__ Draw second part of column
            MVO@    R0,     R4      ;  |                            
            MVO@    R0,     R4      ; /                             

            SUBR    R2,     R4      ; \__ Rewind to draw solid row.
            DECR    R4              ; /
            MVO@    R1,     R4      ; Draw solid row

@@null:     PULR    PC
            ENDP


;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
            INCLUDE "../library/print.asm"       ; PRINT.xxx routines
            INCLUDE "../library/rand.asm"        ; RAND routine
            INCLUDE "../library/fillmem.asm"     ; CLRSCR/FILLZERO/FILLMEM
            INCLUDE "../library/hex16.asm"       ; HEX16

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
