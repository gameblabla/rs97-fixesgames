;;==========================================================================;;
;; Joe Zbiciak's Bouncing Pixels demo                                       ;;
;; This is a somewhat revised version of the demo I created in 1999.        ;;
;; Copyright 2002, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  TO BUILD IN BIN+CFG FORMAT:                                             *;
;*      as1600 -o bncpix.bin -l bncpix.lst bncpix.asm                       *;
;*                                                                          *;
;*  TO BUILD IN ROM FORMAT:                                                 *;
;*      as1600 -o bncpix.rom -l bncpix.lst bncpix.asm                       *;
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

;------------------------------------------------------------------------------
; Allocate some variables in System RAM and Scratch RAM
;------------------------------------------------------------------------------
SCRATCH     ORG     $100, $100, "-RWBN"
ISRVEC      RMB     2               ; Always at $100 / $101
_SCRATCH    EQU     $               ; end of scratch area

SYSTEM      ORG     $2F0, $2F0, "-RWBN"
STACK       RMB     32              ; Reserve 32 words for the stack
VECSAV      RMB     1               ; Place to save ISR vector.
PIXREC      RMB     10*5            ; Room for 10 pixels
EOPIXREC    EQU     $               ; End of pixel records 
XSAVE       RMB     1               ; \
YSAVE       RMB     1               ;  |-- Needed by colored-squares routines
CSAVE       RMB     1               ; /    (PUTPIXELS/GETPIXELS)
BNCTMP      RMB     7               ; BOUNCEPIX temporary storage
_SYSTEM     EQU     $               ; end of system area

           
            ORG     $5000           ; Use default memory map
;------------------------------------------------------------------------------
; EXEC-friendly ROM header.
;------------------------------------------------------------------------------
ROMHDR:     BIDECLE ZERO            ; MOB picture base   (points to NULL list)
            BIDECLE ZERO            ; Process table      (points to NULL list)
            BIDECLE MAIN            ; Program start address
            BIDECLE ZERO            ; Bkgnd picture base (points to NULL list)
            BIDECLE ONES            ; GRAM pictures      (points to NULL list)
            BIDECLE TITLE           ; Cartridge title/date
            DECLE   $03C0           ; No ECS title, run code after title,
                                    ; ... no clicks
ZERO:       DECLE   $0000           ; Screen border control
            DECLE   $0000           ; 0 = color stack, 1 = f/b mode
            DECLE   C_BLK, C_BLK    ; Initial color stack 0 and 1: Black
            DECLE   C_BLK, C_BLK    ; Initial color stack 2 and 3: Black
            DECLE   C_BLK           ; Initial border color: Black
ONES:       DECLE   1, 0
;------------------------------------------------------------------------------




;; ======================================================================== ;;
;;  TITLE  -- Display our modified title screen & copyright date.           ;;
;; ======================================================================== ;;
TITLE:      PROC
            BYTE    102, 'Bouncing Pixels!', 0
            BEGIN
           
            ; Hook up an ISR that will set the screen to Cyan, and then
            ; restore the original interrupt handler.
            MVII    #ISRVEC,R4      ; Point to ISR Vector
            SDBD
            MVI@    R4,     R0      ; Get old vector into R0
            MVO     R0,     VECSAV  ; Save old vector 

            SUBI    #2,     R4      ;\
            MVII    #@@isr, R0      ; |
            MVO@    R0,     R4      ; |-- Set the vector to the title-
            SWAP    R0              ; |   screen ISR.
            MVO@    R0,     R4      ;/
        
           
            ; Patch the title string to say '=JRMZ=' instead of Mattel.
            CALL    PRINT.FLS       ; Write string (ptr in R5)
            DECLE   C_YEL, $23D     ; Yellow, Point to 'Mattel' in top-left
            STRING   '=JRMZ= Productions '  ; Guess who?
            STRING  '      presents'
            BYTE    0
           
            CALL    PRINT.FLS       ; Write string (ptr in R1)
            DECLE   C_YEL, $2C9     ; Yellow, Point to 'Mattel' in lower-right
            STRING  'Copr @ 2002 '  ; 
            STRING  '=JRMZ='        ; Guess who?  :-)
            BYTE    0

            ; Patch up the blue color bar to be yellow.
            MVI     $223,   R0
            XORI    #7,     R0
            MVO     R0,     $223

            ; Done.
            RETURN                  ; Return to EXEC for title screen display


            ; Title-screen ISR -- Set up colorstack and leave.
@@isr:      MVO     R0,     $20     ; Enable display

            MVII    #C_BLU, R0      ;\
            MVII    #$28,   R4      ; |
            MVO@    R0,     R4      ; |   Set color stack and border      
            MVO@    R0,     R4      ; |-- to Blue.  Don't need to worry   
            MVO@    R0,     R4      ; |   about interruptible here because
            MVO@    R0,     R4      ; |   we are in the start of an ISR.
            MVO@    R0,     R4      ;/

            MVI     VECSAV, R0      ;\
            MVO     R0,     ISRVEC  ; |__ Restore original ISR.
            SWAP    R0,     1       ; |
            MVO     R0,     ISRVEC+1;/

            JR      R5              ; Return from Interrupt

            ENDP

;; ======================================================================== ;;
;;  MAIN:  Here's our main program code.                                    ;;
;; ======================================================================== ;;
MAIN        PROC

            ;; ------------------------------------------------------------ ;;
            ;;  Set up our ISR and stack frame.  Do w/ interrupts off.      ;;
            ;; ------------------------------------------------------------ ;;
            DIS                     ; Disable interrupts while we set up

            MVII    #ISR,   R0      ; Set up a simple stub ISR.
            MVO     R0,     ISRVEC
            SWAP    R0
            MVO     R0,     ISRVEC+1

            MVII    #STACK, R6      ; Set up a new stack pointer

            EIS                     ; We're good to go now.


            ;; ------------------------------------------------------------ ;;
            ;;  Fill display with colored-square mode characters.           ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #$00F0, R1      ;\
            MVII    #$1000, R0      ; |__ Initialize screen to colored-
            MVII    #$0200, R4      ; |   squares mode.
            CALL    FILLMEM         ;/



            ;; ------------------------------------------------------------ ;;
            ;;  Draw the 'playfield'... the place where the pixels will do  ;;
            ;;  their bouncing.  The screen coordinates go from (0,0) to    ;;
            ;;  (39,23).                                                    ;;
            ;;                                                              ;;
            ;;                  (5,3)                  (34,3)               ;;
            ;;                   ########################                   ;;
            ;;                   #   (11,8)      (28,8) #                   ;;
            ;;                   #    #            #    #                   ;;
            ;;                   #    #            #    #                   ;;
            ;;                   #    #            #    #                   ;;
            ;;                   #    #            #    #                   ;;
            ;;                   #    ##############    #                   ;;
            ;;                   #  (11,16)     (28,16) #                   ;;
            ;;                  (5,19)                 (34,19)              ;;
            ;; ------------------------------------------------------------ ;;
                                                                            
            ;; ------------------------------------------------------------ ;;
            ;;  Draw horizontal lines                                       ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #5,     R1

@@hloop:    MVII    #1,     R0      ; Blue
            MVII    #3,     R2      ; y = 3
            CALL    PUTPIXELS       ; Draw upper row

            ; Are we in the range for the second horizontal line?
            CMPI    #11,    R1      
            BLT     @@skiph
            CMPI    #28,    R1
            BGT     @@skiph

            MVII    #2,     R0      ; Red
            MVII    #16,    R2      ; y = 16
            CALL    PUTPIXELS       ; Draw lower row

@@skiph:    INCR    R1
            CMPI    #34,    R1         
            BLE     @@hloop
            ; End of horizontal line loop.



            ;; ------------------------------------------------------------ ;;
            ;;  Draw vertical lines.  There are four of these.              ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #4,     R2
@@vloop:
            ; Far left side
            MVII    #5,     R1      ; x = 5
            MVII    #1,     R0      ; Blue

            CALL    PUTPIXELS       ; pixel(5, y) = blue

            ; Far right side
            MVII    #34,    R1      ; x = 34
            MVII    #1,     R0      ; Blue
            CALL    PUTPIXELS       ; pixel(34, y) = blue

            ; Are we in the row range for the inner to columns?
            CMPI    #8,     R2
            BLT     @@skipv
            CMPI    #15,    R2
            BGT     @@skipv

            ; Inner left side
            MVII    #11,    R1      ; x = 11
            MVII    #2,     R0      ; Red
            CALL    PUTPIXELS       ; pixel (11, y) = red

            ; Inner right side
            MVII    #28,    R1      ; x = 28
            MVII    #2,     R0      ; Red
            CALL    PUTPIXELS       ; pixel (28, y) = red

@@skipv:    INCR    R2
            CMPI    #19,    R2
            BLE     @@vloop
            ; End of vertical line loop.

            ;; ------------------------------------------------------------ ;;
            ;;  The main event:  Launch a bunch of bouncing pixels.         ;;
            ;; ------------------------------------------------------------ ;;

            MVII    #PIXREC,R4      ; 10 Pixel records, 5 words apiece
            MVII    #10,    R0      ; 10 bouncing pixels
            MVII    #1,     R1      ; All blue
            MVII    #5,     R2      ; Start at x = 5
            MVII    #1,     R3      ; This toggles between +/- 1
            MVII    #22,    R5      ; Row 22 
@@mkpixlp:  MVO@    R2,     R4      ; x = 5 + 3*k
            MVO@    R5,     R4      ; y = 22
            MVO@    R3,     R4      ; xvel = +/- 1
            NEGR    R3
            MVO@    R3,     R4      ; yvel = -/+ 1
            MVO@    R1,     R4      ; color = 1
            ADDI    #3,     R2
            DECR    R0
            BNEQ    @@mkpixlp


            ;; ------------------------------------------------------------ ;;
            ;;  Main loop:  Cycle through the pixels and bounce them!       ;;
            ;; ------------------------------------------------------------ ;;
@@mainlp:   MVII    #$3FF,  R3      ; Delay constant.

@@dly:      DECR    R3
            TSTR    R3
            BNEQ    @@dly

            MVII    #PIXREC,R4      ; Start of list of pixel records
@@drawpix:  CALL    BOUNCEPIX       ; Update current pixel, point to next
            CMPI    #EOPIXREC, R4   ; Are we at end yet?
            BLT     @@drawpix       ; No?  Keep going.

            B       @@mainlp        ; Go around the main loop again.

            ENDP


;; ======================================================================== ;;
;;  BOUNCEPIX -- Handle movement and bounce calculations for a single pix.  ;;
;; ======================================================================== ;;
BOUNCEPIX   PROC

@@xsv       EQU     BNCTMP + 0      ;\
@@ysv       EQU     BNCTMP + 1      ; |
@@xvsv      EQU     BNCTMP + 2      ; |
@@yvsv      EQU     BNCTMP + 3      ; |-- Temporary local storage for pixel
@@csv       EQU     BNCTMP + 4      ; |   updates. 
@@nxsv      EQU     BNCTMP + 5      ; |
@@nysv      EQU     BNCTMP + 6      ;/

            BEGIN                   ; Save return address
            PSHR    R4              ; Save pixel record pointer


            ;; ------------------------------------------------------------ ;;
            ;;  Update pixel position, and detect whether we hit something. ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #@@xsv, R5

            MVI@    R4,     R1      ; Load X position
            MVI@    R4,     R2      ; Load Y position
            MVO@    R1,     R5      ; Store X in temp. local storage (@@xsv)
            MVO@    R2,     R5      ; Store Y in temp. local storage (@@ysv)

            MVI@    R4,     R0      ; Load X velocity
            ADDR    R0,     R1      ; Add XVEL to X position
            MVO@    R0,     R5      ; Store XVEL to local storage: @@xvsv

            MVI@    R4,     R0      ; Load Y velocity
            ADDR    R0,     R2      ; Add YVEL to Y position
            MVO@    R0,     R5      ; Store YVEL in local storage: @@yvsv

            MVI@    R4,     R0      ; Load COLOR
            MVO@    R0,     R5      ; Store COLOR to local storage: @@csv

            MVO@    R1,     R5      ; Store candidate X coordinate: @@nxsv
            MVO@    R2,     R5      ; Store candidate Y coordinate: @@nysv

            CLRR    R3              ; Clear 'hit flags'

            ;; ------------------------------------------------------------ ;;
            ;;  Did we hit a screen border?                                 ;;
            ;; ------------------------------------------------------------ ;;
@@ytst:     
            TSTR    R2
            BMI     @@yhit          ; hit top
            CMPI    #23,    R2
            BLE     @@xtst          ; didn't hit bottom

@@yhit:     MVII    #2,     R3      ; Mark yvel to be inverted

@@xtst:     TSTR    R1
            BMI     @@xhit          ; hit left
            CMPI    #39,    R1
            BLE     @@btst          ; didn't hit right

@@xhit:     INCR    R3              ; Mark xvel to be inverted
            B       @@hits

            ;; ------------------------------------------------------------ ;;
            ;;  Did we already hit a border?  If so, then don't test for    ;;
            ;;  barriers.                                                   ;;
            ;; ------------------------------------------------------------ ;;
@@btst:     
            TSTR    R3
            BNEQ    @@hits

            ;; ------------------------------------------------------------ ;;
            ;;  Did we hit an interior barrier?                             ;;
            ;; ------------------------------------------------------------ ;;
            CALL    GETPIXELS
            CLRR    R3
            TSTR    R0
            BEQ     @@nohits        ; didn't hit barrier

            INCR    R0              ;\
            ANDI    #7,     R0      ; |
            BNEQ    @@putit         ; |-- Hit a barrier, so change its color
            INCR    R0              ; |
@@putit:    CALL    PUTPIXEL        ;/

            ;; ------------------------------------------------------------ ;;
            ;;  Did we hit it on x, y or both?                              ;;
            ;;  This works by testing whether there was a barrier along     ;;
            ;;  just X, just Y, both X and Y, or on just a corner.          ;;
            ;;                                                              ;;
            ;;  We accumulate collision status into the two LSBs of R3.     ;;
            ;;  Bit 0 means "need to invert X velocity", and bit 1 means    ;;
            ;;  "need to invert Y velocity."                                ;;
            ;; ------------------------------------------------------------ ;;

            MVI     @@xsv,  R1      ; Undo X movement, see if we still hit
            MVI     @@nysv, R2      ; Retain Y movement
            CALL    GETPIXEL
            CLRR    R3
    
            TSTR    R0
            BEQ     @@noxbar        ; No barrier along X

            MVII    #2,     R3      ; Invert Y if there's a horizontal barrier

@@noxbar:   MVI     @@nxsv, R1      ; Retain X movement
            MVI     @@ysv,  R2      ; Undo Y movement, see if we still hit
            PSHR    R3              ; Save hit status.
            CALL    GETPIXEL
            PULR    R3              ; Restore hit status.

            TSTR    R0
            BEQ     @@noybar        ; No barrier along Y

            INCR    R3              ; Invert X if there's a vertical barrier

       
            ;; ------------------------------------------------------------ ;;
            ;;  If we didn't see an x or y barrier, we hit a corner, so     ;;
            ;;  negate both X and Y velocity.                               ;;
            ;; ------------------------------------------------------------ ;;
@@noybar:   TSTR    R3
            BNEQ    @@hits

            MVII    #3,     R3
            B       @@hits

@@hits:
            ;; ------------------------------------------------------------ ;;
            ;;  Increment the color of the pixel.                           ;;
            ;; ------------------------------------------------------------ ;;
            MVI     @@csv,  R0
            INCR    R0
            CMPI    #7,     R0
            BLT     @@colorok
            MVII    #1,     R0
@@colorok:  MVO     R0,     @@csv

            ;; ------------------------------------------------------------ ;;
            ;;  Reflect x, y as needed.                                     ;;
            ;; ------------------------------------------------------------ ;;
            MVI     @@xvsv, R1      ; Old X velocity
            MVI     @@yvsv, R2      ; Old Y velocity

            ; If R3 is non-zero, invert X, Y velocity.
            SARC    R3,     1
            BNC     @@noinvx
            NEGR    R1
            MVO     R1,     @@xvsv
@@noinvx:
            SARC    R3,     1
            BNC     @@noinvy
            NEGR    R2
            MVO     R2,     @@yvsv
@@noinvy:   ADD     @@xsv,  R1      ; Add new xvel to old xpos
            ADD     @@ysv,  R2      ; Add new yvel to old ypos

            ;; ------------------------------------------------------------ ;;
@@nohits:   ;;  Save out the updated x, y positions (in R1, R2).            ;;
            ;; ------------------------------------------------------------ ;;
            PULR    R4
            MVO@    R1,     R4      ; Save new X to pixel record
            MVO@    R2,     R4      ; Save new Y to pixel record
            PSHR    R4
            MVO     R4,     @@nxsv

            MVI     @@csv,  R0      ; Get pixel color
            CALL    PUTPIXEL        ; Draw new pixel

            MVI     @@xsv,  R1      ; Get old X
            MVI     @@ysv,  R2      ; Get old Y
            CLRR    R0              ; Clear old pixel to black
            CALL    PUTPIXEL


            ;; ------------------------------------------------------------ ;;
            ;;  Now, write it all back out to the pixel's status record.    ;;
            ;; ------------------------------------------------------------ ;;
            PULR    R4
            MVII    #@@xvsv,R5
            MVI@    R5,     R0      ; New X velocity
            MVO@    R0,     R4
            MVI@    R5,     R0      ; New Y velocity
            MVO@    R0,     R4
            MVI@    R5,     R0      ; New color
            MVO@    R0,     R4
            ; R4 now points to next pixel record.

            RETURN                  ; Return.
            ENDP


;; ======================================================================== ;;
;;  ISR -- Simple Interrupt Service Routine                                 ;;
;; ======================================================================== ;;
ISR         PROC
            MVO     R0,     $20     ; Keep the lights on (display enable)

            MVII    #C_BLU, R0      ;\__ Blue screen border
            MVO     R0,     $2C     ;/

            MVI     $28,    R0      ;\    Cycle the color in CS0.  This
            INCR    R0              ; |-- causes colored-square pixel color
            MVO     R0,     $28     ;/    7 to shimmer onscreen.

            JR      R5              ; Return
            ENDP

;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
            INCLUDE "../library/print.asm"       ; PRINT.xxx routines
            INCLUDE "../library/fillmem.asm"     ; CLRSCR/FILLZERO/FILLMEM
            INCLUDE "../library/colorsq.asm"     ; colored-squares routines
            INCLUDE "../library/colorsq_sv.asm"  ; PUTPIXELS/GETPIXELS

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
