;;==========================================================================;;
;; Joe Zbiciak's MAZE DEMO.                                                 ;;
;; Copyright 2001, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  TO BUILD IN BIN+CFG FORMAT:                                             *;
;*      as1600 -o mazedemo.bin -l mazedemo.lst mazedemo.asm                 *;
;*                                                                          *;
;*  TO BUILD IN ROM FORMAT:                                                 *;
;*      as1600 -o mazedemo.rom -l mazedemo.lst mazedemo.asm                 *;
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
;*                   Copyright (c) 2000, Joseph Zbiciak                     *;
;* ======================================================================== *;


        ROMW    10              ; Use standard GI 10-bit ROM width
        ORG     $5000           ; Use default memory map

;------------------------------------------------------------------------------
; Magic memory locations
;------------------------------------------------------------------------------
VIDEN   EQU     $0020           ; Video enable handshake
RNDLO   EQU     $035E           ; \_ Where we store random numbers.
RNDHI   EQU     $035F           ; /  (Used by library routine "RAND")
DLY     EQU     $103            ; Delay value set by EXEC title screen

BORD    EQU     $104            ; Color we wish the border to be.
HDLY    EQU     $105            ; Desired horizontal delay
VDLY    EQU     $106            ; Desired vertical delay

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
        DECLE   9, 9, 9, 9, 9   ; Initial color stack / border color: Cyan
ONES:   DECLE   1
;------------------------------------------------------------------------------


;; ======================================================================== ;;
;;  TITLE  -- Display our modified title screen & copyright date.           ;;
;; ======================================================================== ;;
TITLE:  PROC
        BYTE    101, 'Maze Demo', 0
        BEGIN

        ; ----------------------------------------------------------------- ;
        ;  Set up a custom ISR to handle screen color changes and keep the  ;
        ;  video enabled.                                                   ;
        ; ----------------------------------------------------------------- ;
        SDBD
        MVII    #ISR,   R0
        MVO     R0,     $100    ; \
        SWAP    R0              ;  |-- Set up our ISR 
        MVO     R0,     $101    ; /    (this sequence is non-interruptible)

       
        ; ----------------------------------------------------------------- ;
        ;  Start out with a Cyan screen, then patch the title the way we    ;
        ;  want it to be (overwrite MATTEL with my initials.  :-)           ;
        ; ----------------------------------------------------------------- ;
        MVII    #$09,   R0      ; Color 9 is Cyan (pastels.)
        MVO     R0,     BORD
        CLRR    R1
        MVO     R1,     HDLY    
        MVO     R1,     VDLY    

        ; Patch the title string to say '=JRMZ=' instead of Mattel.
        CALL    PRINT.FLS       ; Write string (ptr in R5)
        DECLE   0, $23D         ; Black, Point to 'Mattel' in top-left
        STRING  '=JRMZ='        ; Guess who?  :-)
        STRING  ' Productions' 
        BYTE    0

        CALL    PRINT.FLS       ; Write string (ptr in R1)
        DECLE   0, $2D0         ; Black, Point to 'Mattel' in lower-right.
        STRING  '2002 =JRMZ='   ; Guess who?  :-)
        BYTE    0

        ; Make all the rest of the text black to match
        MVII    #$243,  R3      ; Start after first JRMZ
        MVII    #7,     R0      ; Mask = 0xFFF8.  That's the 1s complement...
        COMR    R0              ; ...of 7.  (Saves an SDBD.)
        MVII    #146,   R2      ; We only need to touch 146 words.

        ; ----------------------------------------------------------------- ;
        ;  Make the characters on the display all black so they stand out   ;
        ;  better against the cyan.                                         ;
        ; ----------------------------------------------------------------- ;
@@titlelp:
        MVI@    R3,     R1      ; Read a word from the display
        ANDR    R0,     R1      ; Mask the foreground color
        MVO@    R1,     R3      ; Write the word back
        INCR    R3              ; Point to next word
        DECR    R2              ; Decrement our loop count
        BNEQ    @@titlelp       ; Iterate.

        ; Done.
        RETURN                  ; Return to caller

        ENDP


;; ======================================================================== ;;
;;  MAIN:  Here's our main program code.                                    ;;
;; ======================================================================== ;;
MAIN:   PROC

        ; "Local" variables
@@xc    EQU     $300            ; X card position
@@xp    EQU     $301            ; X pixel position
@@yc    EQU     $302            ; Y card position
@@yp    EQU     $303            ; Y pixel position
@@dir   EQU     $304            ; Direction 
@@tdir  EQU     $305            ; Directions we've tried at this intersection
@@move  EQU     $305            ; Updated pixel coordinate
@@axis  EQU     $306            ; Pointer to axis that was updated

        CLRR    R0
        MVO     R0,     BORD    ; Black border
        MVII    #3,     R1
        MVO     R1,     HDLY    ; Horizontal delay == 3
        INCR    R1
        MVO     R1,     VDLY    ; Vertical delay == 4
        EIS


        ; ----------------------------------------------------------------- ;
        ;  Draw maze as a set of black passages.  Seed pixel is at (1,1)    ;
        ;  Each 2x2 block of colored squares will end up being one of       ;
        ;  the following with this algorithm:                               ;
        ;                                                                   ;
        ;       ##   #.  ##  #.                                             ;
        ;       #.   #.  ..  ..                                             ;
        ; ----------------------------------------------------------------- ;

@@newmaze:
        ; ----------------------------------------------------------------- ;
        ;  Start off a new maze.                                            ;
        ; ----------------------------------------------------------------- ;
        MVII    #$00F0, R1      ; Initialize a new maze by filling the screen.
        MVII    #$0200, R4      ;
        MVII    #$1492, R0      ;
        CALL    FILLMEM         ; Fill screen w/ red colored squares

        MVII    #$26D,  R2      ; Start in middle

        MVII    #19,    R3      ; \
        MVO     R3,     @@xp    ;  |
        SLR     R3,     1       ;  |
        MVO     R3,     @@xc    ;  |__ Initialize our starting position to
        MVII    #11,    R3      ;  |   the middle of the screen.
        MVO     R3,     @@yp    ;  |
        SLR     R3,     1       ;  |
        MVO     R3,     @@yc    ; /

        MVII    #2,     R0      ; \
        CALL    RAND            ;  |-- Pick an initial starting direction
        MVO     R0,     @@dir   ; /

@@mazelp:
        ; ----------------------------------------------------------------- ;
        ;  Main maze loop:  Clear current posn, and pick an exit from it.   ;
        ; ----------------------------------------------------------------- ;
        MVI@    R2,     R0      ; Get pixel square
        ANDI    #$D1FF, R0      ; Clear lower right corner
        XORI    #$0200, R0      ; Put our blue pixel there
        MVO@    R0,     R2      ; Store it to the display

        ; ----------------------------------------------------------------- ;
        ;  (Delay loop -- this slows the demo down to a sane speed)         ;
        ; ----------------------------------------------------------------- ;
        MVI     DLY,    R3      ; Get delay value (3..6) set by title screen
        ADDR    PC,     R3      ; PC-relative lookup table just before loop
        MVI@    R3,     R3      ; Read from table.
        B       @@dly
        DECLE   $001, $100, $200, $3FF
@@dly:  SARC    R0,     2       ; 
        SARC    R0,     2
        SARC    R0,     2
        DECR    R3
        BNEQ    @@dly


        ; ----------------------------------------------------------------- ;
        ;  Pick a direction to exit by.                                     ;
        ; ----------------------------------------------------------------- ;
        CLRR    R3

        ; ----------------------------------------------------------------- ;
        ;  Note:  R0 is compared against the straightness factor.  We can   ;
        ;  control how twisty the maze is by controlling how often we just  ;
        ;  try to go straight.                                              ;
        ; ----------------------------------------------------------------- ;
        MVII    #4,     R0
        CALL    RAND
        CMPI    #3,     R0      ; Try straight-ahead 3/16th of the time
        BGT     @@pickdir1

        MVI     @@dir,  R0
        B       @@trystraight

@@pickdir:
        CMPI    #$F,    R3      ; If we've tried all four dirs, we're trapped
        BEQ     @@trapped


@@pickdir1:
        MVII    #2,     R0
        CALL    RAND
@@trystraight:
        ANDI    #3,     R0      ; bit 1 is 'x/y', bit 0 is '-1 / +1'
        MVO     R0,     @@dir

        ; ----------------------------------------------------------------- ;
        ;  See if we've already tried this direction.                       ;
        ; ----------------------------------------------------------------- ;
        MVII    #1,     R1
        ANDI    #2,     R0
        BEQ     @@pick1
        SLL     R1,     2
@@pick1:
        XOR     @@dir,  R0
        BEQ     @@pick0
        ADDR    R1,     R1
@@pick0:
        MVO     R1,     @@tdir
        ; ----------------------------------------------------------------- ;
        ;  Ok, R1 is now 1, 2, 4 or 8 depending on which direction we're    ;
        ;  trying.  See if that bit is already set in R3.                   ;
        ; ----------------------------------------------------------------- ;
        ANDR    R3,     R1
        BNEQ    @@pickdir1      ; Yes it was, pick another.

        ; ----------------------------------------------------------------- ;
        ;  No it wasn't.  Add it to the set of directions we've tried.      ;
        ; ----------------------------------------------------------------- ;
        XOR     @@tdir, R3


        ; ----------------------------------------------------------------- ;
        ;  Update X and Y according to the direction we're trying.          ;
        ; ----------------------------------------------------------------- ;
        MVI     @@dir,  R0

        ; ----------------------------------------------------------------- ;
        ;  Select X or Y based on bit 1.                                    ;
        ; ----------------------------------------------------------------- ;
        MVII    #$300,  R4
        ANDI    #2,     R0      ; Get bit 2.
        ADDR    R0,     R4      ; $300 or $302 ==> X or Y

        ; ----------------------------------------------------------------- ;
        ;  Add or subtract 1 based on bit 0.                                ;
        ; ----------------------------------------------------------------- ;
        XOR     @@dir,  R0      ; Get bit 0
        ADDR    R0,     R0      ; Multiply it by 2.
        ADD@    R4,     R0      ; Add it to the value
        DECR    R0              ; Subtract 1.  val' = val + 2 * bit0 - 1.

        ; ----------------------------------------------------------------- ;
        ;  Are we within the screen borders?                                ;
        ; ----------------------------------------------------------------- ;
        BMI     @@pickdir       ; Yes:  Off top/left

        CMP@    R4,     R0      ; Check right/bottom
        BGE     @@pickdir       ; Yes:  Off right/bottom

        SUBI    #2,     R4      ; Reset R4
        MVO     R0,     @@move  ; Store candidate coordinate update
        MVO     R4,     @@axis  ; ... and which coordinate to update


        ; ----------------------------------------------------------------- ;
        ;  Ok, update our cardtab address and test the pixel there.         ;
        ;  Assume horizontal move, and see if it's really a vertical move.  ;
        ; ----------------------------------------------------------------- ;
        MVI     @@dir,  R0      ; Our random number
        MVII    #1,     R1      ; Default to +/- 1 update on pointer.
        MVII    #$D83F, R4      ; horiz move: mask pixels 2, 3
        ANDI    #2,     R0
        BEQ     @@xmove
@@ymove:
        MVII    #20,    R1      ; +/- 20 update for vertical move.
        MVII    #$F1C3, R4      ; vert move: mask pixels 1, 3
@@xmove:

        XOR     @@dir,  R0      ; Now look at bit 0 to see if negative dir.
        BNEQ    @@pmove
@@nmove:
        NEGR    R1              ; Negate movement if bit 0 clear.
@@pmove:
        ADDR    R2,     R1      ; R1 has candidate CARDTAB address.

        MVI@    R1,     R0      ; Load word from candidate destination
        ANDI    #$2400, R0      ; Look at lower-right pixel.
                                ; (Hack:  Ignore bit 0 so we can use the
                                ;  blue pixel as a "status" pixel.)

        BEQ     @@pickdir       ; Zero?  This intersection's already taken.

        ; ----------------------------------------------------------------- ;
        ;  Non zero?  Ok, we can advance into this square (whoo hoo!)
        ; ----------------------------------------------------------------- ;
@@domove:
        ; ----------------------------------------------------------------- ;
        ;  Move the candidate x/y position into our actual x/y position
        ; ----------------------------------------------------------------- ;
        MVI     @@axis, R5
        MVI     @@move, R0
        MVO@    R0,     R5

        ; ----------------------------------------------------------------- ;
        ;  Record the move in the new square (for backtrack purposes)       ;
        ; ----------------------------------------------------------------- ;
        MVI     @@dir,  R0
        MVI@    R1,     R3
        SLL     R3,     2
        RRC     R0,     2       ; Move two lsbs into to msbs
        RRC     R3,     2
        MVO@    R3,     R1      ; Store in bits 14, 15 of word.

        ; ----------------------------------------------------------------- ;
        ;  Clear blue 'location' pixel.                                     ;
        ; ----------------------------------------------------------------- ;
        MVII    #$FDFF, R0
        AND@    R2,     R0
        MVO@    R0,     R2

        ; ----------------------------------------------------------------- ;
        ;  Clear pixels (drawing the maze)                                  ;
        ;                                                                   ;
        ;  If we're moving in a negative direction, mask the current        :
        ;  word.  If we're moving in a positive direction, mask the         :
        ;  new word.                                                        ;
        ; ----------------------------------------------------------------- ;

        SLLC    R3,     2
        BNOV    @@domask        ; Mask old position if negative dir move.
        MOVR    R1,     R2      ; Make "new position" current position.
@@domask:
        AND@    R2,     R4      ; Mask the pixel.
        MVO@    R4,     R2      ; Display it.

        MOVR    R1,     R2      ; Make new position current position for sure.

        ; ----------------------------------------------------------------- ;
        ;  Continue recursing down the maze.                                ;
        ; ----------------------------------------------------------------- ;
        B       @@mazelp

@@trapped:
        ; Recurse back a level
        MVI@    R2,     R0      ; Get current pixel.
        MOVR    R0,     R1      ; Copy it.
        ANDI    #$11FF, R0      ; Mask away our blue "location pixel"
        MVO@    R0,     R2      ; Display it.

        RLC     R1,     2       ; Grab the top two bits
        RLC     R1,     2       ; Put them in the bottom two bits.
        ANDI    #3,     R1      ; Throw away the rest of the bits.
        MVO     R1,     @@dir

        ; ----------------------------------------------------------------- ;
        ;  Decode the direction bits in R1/@@dir, and update coordinates.   ;
        ;  This is similar to how "pickdir" does it, except that we         ;
        ;  interpret the positive/negative bit the opposite way since       ;
        ;  we're backtracking now.                                          ;
        ; ----------------------------------------------------------------- ;
        MVII    #$300,  R3
        ANDI    #2,     R1      ; Bit 1 selects between X or Y
        ADDR    R1,     R3      ; $300 is X, $302 is Y

        MVI@    R3,     R0      ; Get the coordinate that we need to update.
        XOR     @@dir,  R1      ; Get bit 0.
        ADDR    R1,     R1      ; Multiply bit 0 by 2.
        SUBR    R1,     R0      ; Subtract it from the coordinate
        INCR    R0              ; Add 1.  val' = val - 2*bit0 + 1.
        MVO@    R0,     R3      ; Store updated coordinate.


        ; ----------------------------------------------------------------- ;
        ;  Decode direction bits and update CARDTAB pointer.                ;
        ; ----------------------------------------------------------------- ;
        MVI     @@dir,  R0
        MVII    #1,     R1      ; Assume horiz move by default.  +/- 1 update.
        ANDI    #2,     R0      ; Vertical move if bit 1 set.
        BEQ     @@xtmove        ; Yes:  Horizontal move.
        MVII    #20,    R1      ; Vertical move.  +/- 20 update.
@@xtmove:
        XOR     @@dir,  R0      ; Get bit 0.

        BEQ     @@ptmove        ; If bit 0's set, negate the update amount.
                                ; This is opposite of what pickdir does.
        NEGR    R1
@@ptmove:
        ADDR    R1,     R2      ; Now update the pointer.


        ; ----------------------------------------------------------------- ;
        ;  Check to see if we're at the starting location.  This is the     ;
        ;  termination condition for our recursion.                         ;
        ; ----------------------------------------------------------------- ;
        CMPI    #$26D,  R2
        BNEQ    @@mazelp        ; Not at start?  Keep recursing!

        ; ----------------------------------------------------------------- ;
        ;  We're back at the start.  The maze is done.                      ;
        ; ----------------------------------------------------------------- ;
        MVII    #$9,    R0
        MVO     R0,     BORD    ; Make border cyan to signify completion.

        CLRR    R3              ; Sit around awhile.
@@here: NOP 
        NOP 
        NOP 
        NOP 
        NOP 
        NOP 
        DECR    R3
        BNEQ    @@here

        MVO     R3,     BORD    ; Make border black again.
        B       @@newmaze       ; Make a new maze.

        ENDP


;; ======================================================================== ;;
;;  ISR  -- A simple ISR that keeps the screen on and sets our border       ;;
;;          color and position.                                             ;;
;; ======================================================================== ;;
ISR     PROC

        ; Set our border color
        MVI     BORD,   R0
        MVII    #$28,   R4
        MVO@    R0,     R4
        MVO@    R0,     R4
        MVO@    R0,     R4
        MVO@    R0,     R4
        MVO@    R0,     R4

        ; Shift display over 3 pixels, down 4 pixels.
        MVI     HDLY,   R0
        MVO     R0,     $30
        MVI     VDLY,   R0
        MVO     R0,     $31

        ; Enable video
        MVO     R0,     VIDEN

        JR      R5
        ENDP

;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
        INCLUDE "../library/rand.asm"        ; RAND         
        INCLUDE "../library/print.asm"       ; PRINT.xxx 
        INCLUDE "../library/fillmem.asm"     ; CLRSCR/FILLZERO/FILLMEM
