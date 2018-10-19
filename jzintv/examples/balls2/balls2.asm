;;==========================================================================;;
;; Joe Zbiciak's PSYCHO BALLS                                               ;;
;; Copyright 2002, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  TO BUILD IN BIN+CFG FORMAT:                                             *;
;*      as1600 -o balls2.bin -l balls2.lst balls2.asm                       *;
;*                                                                          *;
;*  TO BUILD IN ROM FORMAT:                                                 *;
;*      as1600 -o balls2.rom -l balls2.lst balls2.asm                       *;
;* ======================================================================== *;



            ROMW    16              ; Use 16-bit ROM width

;------------------------------------------------------------------------------
; Include system information
;------------------------------------------------------------------------------
            INCLUDE "../library/gimini.asm"

;------------------------------------------------------------------------------
; Global constants and configuration.
;------------------------------------------------------------------------------
GROM        EQU     $3000           ; GROM base address
GRAM        EQU     $3800           ; GRAM base address
TSKQM       EQU     $7              ; Task queue is 8 entries large
MAXTSK      EQU     2               ; Right now allow 4 active tasks

;------------------------------------------------------------------------------
; Allocate 8-bit variables in Scratch RAM
;------------------------------------------------------------------------------
SCRATCH     ORG     $100, $100, "-RWBN"
ISRVEC      RMB     2               ; Always at $100 / $101

MOB_BUSY    RMB     1               ; FLAG: MOB update is busy

            ; Task-oriented 8-bit variables
TSKQHD      RMB     1               ; Task queue head
TSKQTL      RMB     1               ; Task queue tail
TSKDQ       RMB     2*(TSKQM+1)     ; Task data queue
WTIMER      RMB     1               ; Wait timer
OVRFLO      RMB     1               ; Number of overflows observed
TSKACT      RMB     1               ; Number of active tasks

_SCRATCH    EQU     $               ; end of scratch area

;------------------------------------------------------------------------------
; Allocate 16-bit variables in System RAM
;------------------------------------------------------------------------------
SYSTEM      ORG     $2F0, $2F0, "-RWBN"

STACK       RMB     20              ; Reserve 32 words for the stack
STICSH      RMB     32              ; STIC Shadow
XPOS        RMB     8               ; X positions
YPOS        RMB     8               ; Y positions
XVEL        RMB     8               ; X velocities
YVEL        RMB     8               ; Y velocities
RNDLO       RMB     1               ;\__ Random number generator
RNDHI       RMB     1               ;/
SWPOS       RMB     1               ; Sweeper position
SWDIR       RMB     1               ; Sweeper direction

SLPTR       RMB     1
SNDLIST     RMB     3               ; Random sounds
LASTSND     EQU     $

            ; Task-oriented 16-bit variables
TSKQ        RMB     (TSKQM + 1)     ; Task queue
TSKTBL      RMB     (MAXTSK * 4)    ; Timer task table

_SYSTEM     EQU     $               ; end of system area

SWCHAR      EQU     95*8 + X_RED
SWPER       EQU     60 * 5 + 72     ; sweep every 5 seconds (6 on PAL)

;------------------------------------------------------------------------------
; Support for polling ECScable
;------------------------------------------------------------------------------
EC_LOC      EQU     $CF00
EC_MAG      EQU     $69
EC_POLL     EQU     $CF01


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
ONES:       DECLE   C_BLU, C_BLU    ; Initial color stack 0 and 1: Blue
            DECLE   C_BLU, C_BLU    ; Initial color stack 2 and 3: Blue
            DECLE   C_BLU           ; Initial border color: Blue
;------------------------------------------------------------------------------


;; ======================================================================== ;;
;;  TITLE  -- Display our modified title screen & copyright date.           ;;
;; ======================================================================== ;;
TITLE:      PROC
            BYTE    102, 'Psycho Balls #2', 0
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
MAIN:       PROC
            DIS 

            ; ------------------------------------------------------------- ;
            ;  Reset the stack pointer.                                     ;
            ; ------------------------------------------------------------- ;
            MVII    #STACK, R6

            ; ------------------------------------------------------------- ;
            ;  Initialize random number generator from contents of RAM.     ;
            ;  Zero out the RAM as we go.                                   ;
            ; ------------------------------------------------------------- ;
            MVII    #$100,  R4
            MVII    #$130,  R3
            CLRR    R2
@@rnd:
            ADD@    R4,     R0
            ADD@    R4,     R1
            SUBI    #2,     R4
            MVO@    R2,     R4
            MVO@    R2,     R4
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
            ;  Set up ISR.  This feeds the STIC.                            ;
            ; ------------------------------------------------------------- ;
            MVII    #BALLISR, R0
            MVO     R0,     ISRVEC
            SWAP    R0
            MVO     R0,     ISRVEC+1

            ; ------------------------------------------------------------- ;
            ;  Clear the screen and display a message.                      ;
            ; ------------------------------------------------------------- ;
            CALL    CLRSCR          ; Clear the screen

            CALL    PRINT.FLS       ; Display our message.
            DECLE   C_YEL           ; Yellow
            DECLE   $200 + 5*20 + 4 ; Row #5, colukmn #4 on screen
            STRING  'Psycho Balls!'
            BYTE    0

            ; ------------------------------------------------------------- ;
            ;  Set up the initial status of the balls in the STIC Shadow    ;
            ; ------------------------------------------------------------- ;
            CALL    MEMCPY
            DECLE   STICSH, BALLINIT, 24

            ; ------------------------------------------------------------- ;
            ;  Randomly generate initial positions, velocities.             ;
            ; ------------------------------------------------------------- ;
            CALL    RAND_ARRAY
            DECLE   XPOS,   8,  15  ; Array, length, bits/elem
            CALL    RAND_ARRAY
            DECLE   YPOS,   8,  15  ; Array, length, bits/elem
            CALL    RAND_ARRAY
            DECLE   XVEL,   8,  10  ; Array, length, bits/elem
            CALL    RAND_ARRAY
            DECLE   YVEL,   8,  10  ; Array, length, bits/elem

            ; ------------------------------------------------------------- ;
            ;  Make the XVEL/YVEL values signed.  Relies on XVEL, YVEL      ;
            ;  being adjacent.                                              ;
            ; ------------------------------------------------------------- ;
            MVII    #XVEL,  R4
            MOVR    R4,     R5
            MVII    #16,    R1

@@fix_vel:  MVI@    R4,     R0
            SUBI    #512,   R0
            MVO@    R0,     R5
            DECR    R1
            BNEQ    @@fix_vel


            ; ------------------------------------------------------------- ;
            ;  Set up the Sweeper restart timer.  This is the time between  ;
            ;  restarts of the sweeper.  The sweeper itself is self-timed.  ;
            ;  NOTE:  Be careful!  If you restart the sweeper while it is   ;
            ;  active, weird things can happen!  Make sure that             ;
            ;  SWPER > 72*update_period.                                    ;
            ; ------------------------------------------------------------- ;
            CALL    STARTTASK
            DECLE   0
            DECLE   SW_START            ; Restart the sweeper every SWPER.
            DECLE   SWPER*2, SWPER*2    ;


            MVII    #2,     R0
            MVO     R0,     TSKACT      ; Task #1 will be sweeper.
            

            ; ------------------------------------------------------------- ;
            ;  Everything is set up.  Enable interrupts.                    ;
            ; ------------------------------------------------------------- ;
            EIS
            CALL    RUNQ                ; let 'er rip
            DECR    PC                  ; can't get here

            ENDP

;; ======================================================================== ;;
;;  MOB_UPDATE -- Update the XPOS and YPOS arrays from XVEL, YVEL           ;;
;; ======================================================================== ;;
MOB_UPDATE  PROC
            PSHR    R5

            ; ------------------------------------------------------------- ;
            ;  Update XPOS by XVEL and merge w/ X reg in STIC shadow.       ;
            ; ------------------------------------------------------------- ;
            MVII    #XPOS,      R4
            MVII    #XVEL,      R5
            MVII    #STICSH+0,  R3
            MVII    #8,         R2
@@update_x:
            MVI@    R4,     R0          ; Get X position
            DECR    R4
            ADD@    R5,     R0          ; Add X velocity

            CMPI    #$9F00, R0          ;\
            BNC     @@xok1              ; |
            MVII    #$9F00, R0          ; |__ Try to keep them onscreen
@@xok1:     CMPI    #$0800, R0          ; |   (note unsigned compares!)
            BC      @@xok2              ; |
            MVII    #$0800, R0          ;/
@@xok2:     MVO@    R0,     R4          ; Store new X position

            
            SWAP    R0                  ;\__ Retain integer portion of 
            ANDI    #$00FF, R0          ;/   X position
            MVI@    R3,     R1          ;\__ Get X register and clear old
            ANDI    #$FF00, R1          ;/   X position
            XORR    R0,     R1          ; Merge new X position
            MVO@    R1,     R3          ; Store new X register
            INCR    R3

            DECR    R2
            BNEQ    @@update_x

            ; ------------------------------------------------------------- ;
            ;  Update YPOS by YVEL and merge w/ Y reg in STIC shadow.       ;
            ; ------------------------------------------------------------- ;
            MVII    #YPOS,      R4
            MVII    #YVEL,      R5
            MVII    #STICSH+8,  R3
            MVII    #8,         R2
@@update_y:
            MVI@    R4,     R0          ; Get X position
            DECR    R4
            ADD@    R5,     R0          ; Add X velocity

            CMPI    #$5F00, R0          ;\
            BLT     @@yok1              ; |
            MVII    #$5F00, R0          ; |__ Try to keep them onscreen.
@@yok1:     CMPI    #$0800, R0          ; |
            BGT     @@yok2              ; |
            MVII    #$0800, R0          ;/
@@yok2:     MVO@    R0,     R4          ; Store new X position
            
            SWAP    R0                  ;\__ Retain integer portion of 
            ANDI    #$007F, R0          ;/   Y position
            MVI@    R3,     R1          ;\__ Get Y register and clear old
            ANDI    #$FF80, R1          ;/   Y position
            XORR    R0,     R1          ; Merge new Y position
            MVO@    R1,     R3          ; Store new Y register
            INCR    R3

            DECR    R2
            BNEQ    @@update_y

            ; ------------------------------------------------------------- ;
            ;  Check for any collisions.  Randomize XVEL/YVEL on collision. ;
            ;  Record up to three random numbers from collisions to play    ;
            ;  on the PSG.                                                  ;
            ; ------------------------------------------------------------- ;
            MVII    #STICSH+$18, R4
            MVII    #SNDLIST, R0
            MVO     R0,     SLPTR
            CLRR    R1
@@check_coll:
            MVI@    R4,     R2          ; Get next collision word
            ANDI    #$3FF,  R2          ; Any bits set?
            BEQ     @@no_coll           ; No:  No collision

            MVII    #11,    R0          ;\
            CALL    RAND                ; |-- Number = -512 to 511
            SUBI    #1024,  R0          ;/
            MVII    #XVEL,  R3          ;\__ Point to X velocity
            ADDR    R1,     R3          ;/
            MVO@    R0,     R3          ; Store new XVEL

            MVII    #11,    R0          ;\
            CALL    RAND                ; |-- Number = -512 to 511
            SUBI    #1024,  R0          ;/
            ADDI    #YVEL-XVEL, R3      ; Point to Y velocity
            MVO@    R0,     R3          ; Store new YVEL

            MVI     SLPTR,  R3
            CMPI    #LASTSND,R3
            BEQ     @@no_snd
            MVII    #10,    R0
            CALL    RAND
            ADDI    #$40,   R0
            MVO@    R0,     R3
            INCR    R3
            MVO     R3,     SLPTR
@@no_snd:

@@no_coll:  INCR    R1
            CMPI    #8,     R1
            BNEQ    @@check_coll

            ; ------------------------------------------------------------- ;
            ;  Fill out the silly sounds w/ silence if some are unused.     ;
            ; ------------------------------------------------------------- ;
            MVI     SLPTR,  R5
            CLRR    R0                  ; fill unused sounds w/ 0s.
            INCR    PC                  ; skip MVO@ on first iter.
@@pl:       MVO@    R0,     R5
            CMPI    #LASTSND, R5
            BNEQ    @@pl

            ; ------------------------------------------------------------- ;
            ;  Play up to three silly sounds based on collisions.           ;
            ; ------------------------------------------------------------- ;
            MVII    #$38,   R0
            MVO     R0,     PSG0.chan_enable

            MVII    #PSG0.chn_a_lo,  R3 ; Period registers
            MVII    #PSG0.chn_a_vol, R4 ; Volume registers
            MVII    #SNDLIST,        R5 ; Silly sounds
            MVII    #3,     R2

@@play:     MVI@    R5,     R0          ; get silly tone
            MVO@    R0,     R3          ; \
            SWAP    R0                  ;  |__ Write out low and high
            ADDI    #4,     R3          ;  |   halves
            MVO@    R0,     R3          ; /
            SUBI    #3,     R3          ; Point at next period register

            TSTR    R0                  ; Was it actually a tone?
            BEQ     @@quiet             ; $0000 means quiet
            MVII    #$F,    R0          ; non-zero:  Make volume max.
@@quiet:    MVO@    R0,     R4          ; Write the volume register

            DECR    R2
            BNEQ    @@play

            ; ------------------------------------------------------------- ;
            ;  Done with MOB updates, so clear busy-flag.                   ;
            ; ------------------------------------------------------------- ;
            CLRR    R0
            MVO     R0,     MOB_BUSY
 
            PULR    PC
            ENDP


;; ======================================================================== ;;
;;  SW_START   -- Start the sweeper every so often.                         ;;
;; ======================================================================== ;;
SW_START    PROC
            PSHR    R5
            ; ------------------------------------------------------------- ;
            ;  Set up the 'sweeper'.  He keeps the balls cleaned off the    ;
            ;  edges (or tries to).                                         ;
            ; ------------------------------------------------------------- ;
            MVII    #$200,  R1
            MVO     R1,     SWPOS       ; upper left corner
            MVII    #1,     R1
            MVO     R1,     SWDIR       ; going to the right.

            CALL    STARTTASK           ;\
            DECLE   1                   ; |__ Start the sweeper task
            DECLE   SW_UPDATE           ; |
            DECLE   2,      2           ;/    60Hz
            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  SW_UPDATE  -- Update the XPOS and YPOS arrays from XVEL, YVEL           ;;
;; ======================================================================== ;;
SW_UPDATE   PROC
            PSHR    R5

            ; ------------------------------------------------------------- ;
            ;  Update the sweeper.                                          ;
            ; ------------------------------------------------------------- ;
            MVI     SWPOS,  R1          ; Get sweeper position
            MVI     SWDIR,  R2          ;\__ Move sweeper
            ADDR    R2,     R1          ;/
            TSTR    R2
            BEQ     @@leave
            MVII    #SWCHAR,R0          ;\ 
            XOR@    R1,     R0          ; |-- toggle displayed/undisplayed
            MVO@    R0,     R1          ;/
            MVO     R1,     SWPOS       ; Save updated position

            ; check for corners and update direction
            CMPI    #$200 + 19, R1
            BNEQ    @@not_ur
            MVII    #20,    R2          ; Move down at upper-right corner
            B       @@sw_done
@@not_ur:   CMPI    #$200 + 11*20 + 19, R1
            BNEQ    @@not_lr
            MVII    #$FFFF, R2          ; Move left at lower-right corner
            B       @@sw_done
@@not_lr:   CMPI    #$200 + 11*20, R1
            BNEQ    @@not_ll
            MVII    #$FFEC, R2          ; Move up at lower-left corner
            B       @@sw_done
@@not_ll:   CMPI    #$200,  R1
            BNEQ    @@sw_done
            MVII    #2,     R3
            CALL    STOPTASK            ; Stop ourselves.
            CLRR    R2                  ; Move right at upper-left corner
@@sw_done   MVO     R2,     SWDIR

@@leave:    PULR    PC
            ENDP

;; ======================================================================== ;;
;;  RAND_ARRAY -- Fill an array with random values.                         ;;
;; ======================================================================== ;;
RAND_ARRAY  PROC
            MVI@    R5,     R4          ; Output pointer
            MVI@    R5,     R1          ; Length
            MVI@    R5,     R0          ; bits/number
            PSHR    R5

@@loop:     PSHR    R0
            CALL    RAND
            MVO@    R0,     R4
            PULR    R0
            DECR    R1
            BNEQ    @@loop

            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  BALLINIT -- Initial STIC register state for our balls.                  ;;
;; ======================================================================== ;;
BALLINIT    PROC
@@x0        DECLE   STIC.mobx_visb + STIC.mobx_intr
@@x1        DECLE   STIC.mobx_visb + STIC.mobx_intr
@@x2        DECLE   STIC.mobx_visb + STIC.mobx_intr
@@x3        DECLE   STIC.mobx_visb + STIC.mobx_intr
@@x4        DECLE   STIC.mobx_visb + STIC.mobx_intr
@@x5        DECLE   STIC.mobx_visb + STIC.mobx_intr
@@x6        DECLE   STIC.mobx_visb + STIC.mobx_intr
@@x7        DECLE   STIC.mobx_visb + STIC.mobx_intr

@@y0        DECLE   STIC.moby_ysize2
@@y1        DECLE   STIC.moby_ysize2
@@y2        DECLE   STIC.moby_ysize2
@@y3        DECLE   STIC.moby_ysize2
@@y4        DECLE   STIC.moby_ysize2
@@y5        DECLE   STIC.moby_ysize2
@@y6        DECLE   STIC.moby_ysize2
@@y7        DECLE   STIC.moby_ysize2

@@a0        DECLE   STIC.moba_fg8 + STIC.moba_gram
@@a1        DECLE   STIC.moba_fg9 + STIC.moba_gram
@@a2        DECLE   STIC.moba_fgA + STIC.moba_gram
@@a3        DECLE   STIC.moba_fgB + STIC.moba_gram
@@a4        DECLE   STIC.moba_fgC + STIC.moba_gram
@@a5        DECLE   STIC.moba_fgD + STIC.moba_gram
@@a6        DECLE   STIC.moba_fgE + STIC.moba_gram
@@a7        DECLE   STIC.moba_fgF + STIC.moba_gram
            ENDP

;; ======================================================================== ;;
;;  BALL -- Picture of our ball.                                            ;;
;; ======================================================================== ;;
BALL        PROC
            DECLE   %00111100
            DECLE   %01111110
            DECLE   %11011011
            DECLE   %11111111
            DECLE   %10111101
            DECLE   %11011011
            DECLE   %01100110
            DECLE   %00111100
            ENDP

;; ======================================================================== ;;
;;  BALLISR -- Main ISR.  This feeds the STIC                               ;;
;; ======================================================================== ;;
BALLISR     PROC

            ; ------------------------------------------------------------- ;
            ;  Set up colorstack/border.                                    ;
            ; ------------------------------------------------------------- ;
            MVII    #C_BLU, R0          ;\__ Make screen blue
            MVO     R0,     $28         ;/
            MVII    #C_BLK, R0          ;\__ Make border black
            MVO     R0,     $2C         ;/

            MVO     R0,     $20         ; Keep the screen on
            MVI     $21,    R0          ; Keep the screen color-stack

            CALL    MEMCPY              ; Copy 8 words to GRAM card #0
            DECLE   $3800,  BALL, 8 

            ; ------------------------------------------------------------- ;
            ;  Update the MOBs, unless the previous MOB update isn't done.  ;
            ; ------------------------------------------------------------- ;
            MVI     MOB_BUSY, R1
            TSTR    R1
            BNEQ    @@skip_mobs

            ; ------------------------------------------------------------- ;
            ;  Copy MOB registers to the STIC from the shadow copy.         ;
            ; ------------------------------------------------------------- ;
            CALL    MEMCPY
            DECLE   $0000, STICSH, 24   ; Copy 24 words to MOB registers

            ; ------------------------------------------------------------- ;
            ;  Copy collision registers from the STIC to the shadow copy.   ;
            ; ------------------------------------------------------------- ;
            CALL    MEMCPY
            DECLE   STICSH+$18, $18, 8  ; Copy 8 words from collision regs

            ; ------------------------------------------------------------- ;
            ;  Clear the collision registers.                               ;
            ; ------------------------------------------------------------- ;
            CLRR    R0
            MVII    #$18,   R4          ;\
            MVO@    R0,     R4          ; |
            MVO@    R0,     R4          ; |
            MVO@    R0,     R4          ; |
            MVO@    R0,     R4          ; |-- Clear the collision registers
            MVO@    R0,     R4          ; |   in the STIC
            MVO@    R0,     R4          ; |
            MVO@    R0,     R4          ; |
            MVO@    R0,     R4          ;/

            MVII    #1,     R1
            MVO     R1,     MOB_BUSY
            MVII    #MOB_UPDATE, R0
            JSRD    R5,     QTASK

@@skip_mobs:
            MVI     OVRFLO, R0
            TSTR    R0
            B       @@no_ovrflo
            
            SLL     R0, 2
            SLL     R0, 1
            ADDI    #$87, R0
            MVO     R0, $200 + 21

@@no_ovrflo
            CALL    DOTIMER             ; Update timer-based tasks.
            B       $1014               ; return via standard vector
            ENDP


;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
            INCLUDE "../library/print.asm"       ; PRINT.xxx routines
            INCLUDE "../library/rand.asm"        ; RAND routine
            INCLUDE "../library/fillmem.asm"     ; CLRSCR/FILLZERO/FILLMEM
            INCLUDE "../library/memcpy.asm"      ; MEMCPY
            INCLUDE "../task/timer.asm"          ; timer-based task routines
            INCLUDE "../task/taskq.asm"          ; RUNQ task-queue routines

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
