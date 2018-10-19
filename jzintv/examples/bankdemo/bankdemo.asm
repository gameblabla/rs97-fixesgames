;;==========================================================================;;
;; Joe Zbiciak's Bankswitch Demo                                            ;;
;; Copyright 2002, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  TO BUILD IN BIN+CFG FORMAT:                                             *;
;*      as1600 -o bankdemo.bin -l bankdemo.lst bankdemo.asm                 *;
;*                                                                          *;
;*  TO BUILD IN ROM FORMAT:                                                 *;
;*      as1600 -o bankdemo.rom -l bankdemo.lst bankdemo.asm                 *;
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


        ROMW    16              ; Use 16-bit ROM

;------------------------------------------------------------------------------
; Include system information
;------------------------------------------------------------------------------
        INCLUDE "../library/gimini.asm"


        ORG     $5000           ; Use default memory map
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
        BYTE    102, 'Bankswitch Demo', 0
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


;; ======================================================================== ;;
;;  MAIN:  Here's our main program code.                                    ;;
;; ======================================================================== ;;
MAIN:   PROC
        BEGIN

        CALL    CLRSCR          ; Clear the screen


        ;; ---------------------------------------------------------------- ;;
        ;;  Select the first message and display it.                        ;;
        ;; ---------------------------------------------------------------- ;;
        CALL    PRINT.FLS       ; Display our message.
        DECLE   X_WHT           ; White
        DECLE   $200 + 2*20 + 0 ; Row #0, column #0 on screen
                ;01234567890123456789 
        STRING  "MAP $1000 -> $6000: ", 0

        MVII    #$60,   R1      ; Range to bankswitch = $6000 - $67FF
        MVII    #$10,   R2      ; Point it to $1000 - $17FF in Intellicart
        CALL    IC_SETBANK      ; Do it.

        CALL    PRINT.FLP       ; Display message 1 from bankswitched memory
        DECLE   X_YEL           ; Yellow
        DECLE   $200 + 3*20 + 5 ; Row #1, column #1 on screen
                ;01234567890123456789 
        DECLE   MSG1



        ;; ---------------------------------------------------------------- ;;
        ;;  Select the second message and display it.                       ;;
        ;; ---------------------------------------------------------------- ;;
        CALL    PRINT.FLS       ; Display our message.
        DECLE   X_WHT           ; White
        DECLE   $200 + 5*20 + 0 ; Row #0, column #0 on screen
                ;01234567890123456789 
        STRING  "MAP $1800 -> $6000: ", 0

        MVII    #$60,   R1      ; Range to bankswitch = $6000 - $67FF
        MVII    #$18,   R2      ; Point it to $1800 - $1FFF in Intellicart
        CALL    IC_SETBANK      ; Do it.

        CALL    PRINT.FLP       ; Display message 1 from bankswitched memory
        DECLE   X_YEL           ; Yellow
        DECLE   $200 + 6*20 + 5 ; Row #1, column #1 on screen
                ;01234567890123456789 
        DECLE   MSG2
        

        ;; ---------------------------------------------------------------- ;;
        ;;  Select the first message, but display it with hardcoded addr.   ;;
        ;;  Ordinarily, hard-coded addresses are a bad idea.  I'm using     ;;
        ;;  one here for illustration purposes only.                        ;;
        ;; ---------------------------------------------------------------- ;;
        CALL    PRINT.FLS       ; Display our message.
        DECLE   X_WHT           ; White
        DECLE   $200 + 8*20 + 0 ; Row #0, column #0 on screen
                ;01234567890123456789 
        STRING  "MAP $1000 -> $6000: ", 0

        MVII    #$60,   R1      ; Range to bankswitch = $6000 - $67FF
        MVII    #$10,   R2      ; Point it to $1000 - $17FF in Intellicart
        CALL    IC_SETBANK      ; Do it.

        CALL    PRINT.FLP       ; Display message 1 from bankswitched memory
        DECLE   X_YEL           ; Yellow
        DECLE   $200 + 9*20 + 5 ; Row #1, column #1 on screen
                ;01234567890123456789 
        DECLE   $6000           ; <-- NOTE:  Address is hardcoded!


        

        RETURN                  ; Return to the EXEC and sit doing nothing.
        ENDP

;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
        INCLUDE "../library/print.asm"       ; PRINT.xxx routines
        INCLUDE "../library/fillmem.asm"     ; CLRSCR/FILLZERO/FILLMEM
        INCLUDE "../library/ic_banksw.asm"   ; IC_xxx routines

;; ======================================================================== ;;
;;  BANKSWITCHED AREAS                                                      ;;
;;                                                                          ;;
;;  We set up two areas that are loaded in the lower part of the            ;;
;;  Intellicart address space.  We will be able to access these areas       ;;
;;  using the Intellicart's bank switching.                                 ;;
;;                                                                          ;;
;;  Our two areas are set up to appear at $6000 in the Intellivision        ;;
;;  memory map, but load at $1000 and $1800 in the Intellicart memory map,  ;;
;;  respectively.                                                           ;;
;;                                                                          ;;
;;  If you look at the listing file or at the symbol table, you'll notice   ;;
;;  that MSG1 and MSG2 both show up as having assembled at $6000.  In the   ;;
;;  ROM image, though, they're set to load at $1000 and $1800.  If you      ;;
;;  assemble to a BIN+CFG, you'll see this as a [preload] section in the    ;;
;;  CFG.                                                                    ;;
;;                                                                          ;;
;;  One thing to keep in mind:  $1000 and $1800 are addresses in the        ;;
;;  INTELLICART address map, and not necessarily in the Intellivision       ;;
;;  memory map.  The Intellicart has its own private address space that     ;;
;;  ranges from $0000 - $FFFF.  It allows the Intellivision to see its      ;;
;;  memory via two means:                                                   ;;
;;                                                                          ;;
;;   -- DIRECT MAP.  This is the default.  In a direct mapped address       ;;
;;      range, there is a 1-to-1 correspondence between Intellivision       ;;
;;      and Intellicart addresses.                                          ;;
;;                                                                          ;;
;;   -- BANKSWITCHED.  A bankswitched range maps a fixed range in the       ;;
;;      Intellivision address map to an arbitrary range in the              ;;
;;      Intellicart memory map.  There is no restriction on where the       ;;
;;      bankswitched range points inside the Intellicart address map.       ;;
;;                                                                          ;;
;;  In this demo, we've configured $6000 - $67FF to be bankswitched, so     ;;
;;  that we can see into unmapped regions of Intellicart address space.     ;;
;; ======================================================================== ;;

;; ------------------------------------------------------------------------ ;;
;;  MSG1 loads at $1000, but we will eventually see it at $6000.            ;;
;; ------------------------------------------------------------------------ ;;
        ORG     $6000,  $1000, "-RWB"
MSG1    STRING  "Message 1", 0

;; ------------------------------------------------------------------------ ;;
;;  MSG2 loads at $1800, but we will eventually see it at $6000.            ;;
;; ------------------------------------------------------------------------ ;;
        ORG     $6000,  $1800, "-RWB"
MSG2    STRING  "Message 2", 0

;; ------------------------------------------------------------------------ ;;
;;  Set up a read-only bankswitched area from $6000 - $67FF.  Nothing is    ;;
;;  loaded at $6000 in the Intellicart memory map -- this merely marks the  ;;
;;  range as readable and bankswitched in the Intellivision's memory map.   ;;
;;  It'll show up as a [bankswitch] section in the .CFG file if you         ;;
;;  assemble as BIN+CFG.                                                    ;;
;; ------------------------------------------------------------------------ ;;
        ORG     $6000,  $6000,  "=RB"
        RMB     $0800

;; ======================================================================== ;;
;;  End of "bankdemo".                                                      ;;
;; ======================================================================== ;;
