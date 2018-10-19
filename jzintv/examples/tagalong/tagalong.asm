;;==========================================================================;;
;; Joe Zbiciak's Tagalong Todd!                                             ;;
;; Copyright 2002, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  TO BUILD IN BIN+CFG FORMAT:                                             *;
;*      as1600 -o tagalong.bin -l tagalong.lst tagalong.asm                 *;
;*                                                                          *;
;*  TO BUILD IN ROM FORMAT:                                                 *;
;*      as1600 -o tagalong.rom -l tagalong.lst tagalong.asm                 *;
;* ======================================================================== *;


            ROMW    16              ; Use 16-bit ROM

;------------------------------------------------------------------------------
; Include system information
;------------------------------------------------------------------------------
            INCLUDE "../library/gimini.asm"

;------------------------------------------------------------------------------
; Global constants and configuration.
;------------------------------------------------------------------------------

TSKQM       EQU     $7              ; Task queue is 8 entries large
MAXTSK      EQU     1               ; Only one task
TODD_VEL    EQU     $AA

;------------------------------------------------------------------------------
; Allocate 8-bit variables in Scratch RAM
;------------------------------------------------------------------------------
SCRATCH     ORG     $100, $100, "-RWBN"

ISRVEC      RMB     2               ; Always at $100 / $101

            ; Task-oriented 8-bit variables
TSKQHD      RMB     1               ; Task queue head
TSKQTL      RMB     1               ; Task queue tail
TSKDQ       RMB     2*(TSKQM+1)     ; Task data queue
TSKACT      RMB     1               ; Number of active tasks

            ; Hand-controller 8-bit variables
SH_TMP      RMB     1               ; Temp storage.
SH_LR0      RMB     3               ;\
SH_FL0      EQU     SH_LR0 + 1      ; |-- Three bytes for left controller
SH_LV0      EQU     SH_LR0 + 2      ;/
SH_LR1      RMB     3               ;\
SH_FL1      EQU     SH_LR1 + 1      ; |-- Three bytes for right controller
SH_LV1      EQU     SH_LR1 + 2      ;/

            ; Misc other stuff

_SCRATCH    EQU     $               ; end of scratch area



;------------------------------------------------------------------------------
; Allocate 16-bit variables in System RAM 
;------------------------------------------------------------------------------
SYSTEM      ORG     $2F0, $2F0, "-RWBN"
STACK       RMB     32              ; Reserve 32 words for the stack

            ; Task-oriented 16-bit variables
TSKQ        RMB     (TSKQM + 1)     ; Task queue
TSKTBL      RMB     (MAXTSK * 4)    ; Timer task table

            ; Hand-controller 16-bit variables
SHDISP      RMB     1               ; ScanHand dispatch

            ; STIC shadow
STICSH      RMB     24              ; Room for X, Y, and A regs only.

TODD        PROC                    ; TODD's STATS
@@XP        RMB     1               ; X position
@@YP        RMB     1               ; Y position
@@XV        RMB     1               ; X velocity
@@YV        RMB     1               ; Y velocity
@@TXV       RMB     1               ; Target X velocity
@@TYV       RMB     1               ; Target Y velocity
            ENDP

PLYR        PROC
@@XP        RMB     1               ; X position
@@YP        RMB     1               ; Y position
@@XV        RMB     1               ; X velocity
@@YV        RMB     1               ; Y velocity
@@TXV       RMB     1               ; Target X velocity
@@TYV       RMB     1               ; Target Y velocity
            ENDP

MOB_BUSY    RMB     1
_SYSTEM     EQU     $               ; end of system area


;------------------------------------------------------------------------------
; EXEC-friendly ROM header.
;------------------------------------------------------------------------------
            ORG     $5000           ; Use default memory map
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
            STRING  102, "Tagalong Todd", 0
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
            MVII    #STACK, R6      ; Set up our stack

            MVII    #$25D,  R1      ;\
            MVII    #$102,  R4      ; |-- Clear all of memory
            CALL    FILLZERO        ;/

            MVII    #INITISR, R0    ;\    Do GRAM initialization in ISR.
            MVO     R0,     ISRVEC  ; |__ INITISR will the point to the 
            SWAP    R0              ; |   regular ISR when it's done.
            MVO     R0,     ISRVEC+1;/    
          
            ;; ------------------------------------------------------------ ;;
            ;;  Set up our hand-controller dispatch.                        ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #HAND,  R0      ;\__ Set up scanhand dispatch table
            MVO     R0,     SHDISP  ;/

            ;; ------------------------------------------------------------ ;;
            ;;  Set up Todd's AI.                                           ;;
            ;; ------------------------------------------------------------ ;;
            CALL    STARTTASK
            DECLE   0
            DECLE   TODDTASK
            DECLE   60, 60          ; 2Hz (twice a second)

            MVII    #1,     R0
            MVO     R0,     TSKACT


            ;; ------------------------------------------------------------ ;;
            ;;  Put you and Todd onscreen.                                  ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #$1000, R0
            MVO     R0,     PLYR.XP
            MVO     R0,     PLYR.YP

            MVII    #$4000, R0
            MVO     R0,     TODD.XP
            MVO     R0,     TODD.YP

            EIS

            ;; ------------------------------------------------------------ ;;
            ;;  Fall into the RUNQ.  We should never exit the RUNQ in this  ;;
            ;;  demo, since we never call SCHEDEXIT.                        ;;
            ;; ------------------------------------------------------------ ;;
            CALL    RUNQ            ; Run until a SCHEDEXIT happens

            ;; ------------------------------------------------------------ ;;
            ;;  If a SCHEDEXIT *does* happen (say, due to a bug), crash     ;;
            ;;  gracefully.                                                 ;;
            ;; ------------------------------------------------------------ ;;
            CALL    PRINT.FLS
            DECLE   C_RED, $200 + 11*20
                    ;01234567890123456789
            STRING  "SCHEDEXIT WAS CALLED",0

            DECR    PC              ; Can't get here
          
            ENDP

;; ======================================================================== ;;
;;  SINTBL  -- Sine table.  sin(disc_dir) * 511                             ;;
;; ======================================================================== ;;
SINTBL      PROC
            DECLE   $0000
            DECLE   $00C3
            DECLE   $0169
            DECLE   $01D8
            DECLE   $01FF
            DECLE   $01D8
            DECLE   $0169
            DECLE   $00C3
            DECLE   $0000
            DECLE   $FF3D
            DECLE   $FE97
            DECLE   $FE28
            DECLE   $FE01
            DECLE   $FE28
            DECLE   $FE97
            DECLE   $FF3D
            ENDP

;; ======================================================================== ;;
;;  HAND    Dispatch table.                                                 ;;
;; ======================================================================== ;;
HAND        PROC
            DECLE   HIT_KEYPAD
            DECLE   HIT_ACTION
            DECLE   HIT_DISC
            ENDP

;; ======================================================================== ;;
;;  HIT_KEYPAD -- Someone hit a key on a keypad.                            ;;
;; ======================================================================== ;;
HIT_KEYPAD  PROC
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  HIT_ACTION -- Someone hit a key on a keypad.                            ;;
;; ======================================================================== ;;
HIT_ACTION  PROC
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  HIT_DISC   -- Someone hit a key on a keypad.                            ;;
;; ======================================================================== ;;
HIT_DISC    PROC
            PSHR    R5

            ANDI    #$FF,   R2      ; Ignore controller number
            CMPI    #$80,   R2
            BLT     @@pressed

            CLRR    R0
            MVO     R0,     PLYR.TXV
            MVO     R0,     PLYR.TYV
            PULR    PC

@@pressed:  MOVR    R2,     R1
            ADDI    #4,     R1
            ANDI    #$F,    R1
            ADDI    #SINTBL,R2          ; sine pointer
            ADDI    #SINTBL,R1          ; cosine pointer
            MVI@    R2,     R2          ; sine for our direction
            MVI@    R1,     R1          ; cosine for our direction
            NEGR    R2
            MVO     R2,     PLYR.TYV    ; Set our target Y velocity to sine
            MVO     R1,     PLYR.TXV    ; Set our target X velocity to cosine

            PULR    PC

            ENDP

;; ======================================================================== ;;
;;  TODDTASK   -- Todd wants to find you!                                   ;;
;; ======================================================================== ;;
TODDTASK    PROC
            
            ;; ------------------------------------------------------------ ;;
            ;;  This is really simple:  Todd will pick one of 8 directions  ;;
            ;;  to walk in to try to move towards you.  He picks this only  ;;
            ;;  based on whether you're left/right or above/below him.      ;;
            ;; ------------------------------------------------------------ ;;
            
            CLRR    R0                  ; set X vel to 0
            CLRR    R1                  ; set Y vel to 0
            MVI     PLYR.XP,    R2
            MVI     TODD.XP,    R3
            SLR     R2,         2
            SLR     R3,         2

            CMPR    R3,         R2
            BEQ     @@xp0               ; Equal?  X-vel stays 0
            MVII    #TODD_VEL,  R0      ; Player > Todd?  Move right
            BGT     @@xp0               ; Player < Todd?  Move left
            NEGR    R0
@@xp0       

            SUBR    R3,         R2      ;\
            BPL     @@dx_pos            ; |
            NEGR    R2                  ; |__ If X coords are close enough
@@dx_pos    CMPI    #2,         R2      ; |   anyway, force target Xvel to 0
            BGE     @@xp_ok             ; |
            CLRR    R0                  ;/
@@xp_ok

            MVI     PLYR.YP,    R2
            MVI     TODD.YP,    R3
            SLR     R2,         2
            SLR     R3,         2

            CMPR    R3,         R2
            BEQ     @@yp0               ; Equal?  X-vel stays 0
            MVII    #TODD_VEL,  R1      ; Player > Todd?  Move right
            BGT     @@yp0               ; Player < Todd?  Move left
            NEGR    R1
@@yp0

            SUBR    R3,         R2      ;\
            BPL     @@dy_pos            ; |
            NEGR    R2                  ; |__ If Y coords are close enough
@@dy_pos    CMPI    #2,         R2      ; |   anyway, force target Yvel to 0
            BGE     @@yp_ok             ; |
            CLRR    R1                  ;/
@@yp_ok
            MVO     R0,     TODD.TXV    ; Set Todd's target X velocity
            MVO     R1,     TODD.TYV    ; Set Todd's target Y velocity
            
            JR      R5                  ; Leave
            ENDP

;; ======================================================================== ;;
;;  MOB_UPDATE -- This updates the player's and Todd's position and vel.    ;;
;; ======================================================================== ;;
MOB_UPDATE  PROC
            PSHR    R5

            ;; ------------------------------------------------------------ ;;
            ;;  Bring our actual velocity closer to our target velocity,    ;;
            ;;  and apply the velocity to our position.                     ;;
            ;; ------------------------------------------------------------ ;;
            MVI     PLYR.TXV,   R0
            SUB     PLYR.XV,    R0
            SARC    R0
            BMI     @@nr0
            ADCR    R0
@@nr0       SARC    R0
            BMI     @@nr1
            ADCR    R0
@@nr1       ADD     PLYR.XV,    R0
            MVO     R0,         PLYR.XV
            ADD     PLYR.XP,    R0
            MVO     R0,         PLYR.XP

            MVI     PLYR.TYV,   R0
            SUB     PLYR.YV,    R0
            SARC    R0
            BMI     @@nr2
            ADCR    R0
@@nr2       SARC    R0
            BMI     @@nr3
            ADCR    R0
@@nr3       ADD     PLYR.YV,    R0
            MVO     R0,         PLYR.YV
            ADD     PLYR.YP,    R0
            MVO     R0,         PLYR.YP

            MVI     TODD.TXV,   R0
            SUB     TODD.XV,    R0
            SARC    R0, 2
            ADD     TODD.XV,    R0
            MVO     R0,         TODD.XV
            ADD     TODD.XP,    R0
            MVO     R0,         TODD.XP

            MVI     TODD.TYV,   R0
            SUB     TODD.YV,    R0
            SARC    R0, 2
            ADD     TODD.YV,    R0
            MVO     R0,         TODD.YV
            ADD     TODD.YP,    R0
            MVO     R0,         TODD.YP


            ;; ------------------------------------------------------------ ;;
            ;;  Merge our position with our MOB registers.                  ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #@@mobr,    R4      ; MOB information template
            MVII    #STICSH,    R5

            MVI     PLYR.XP,    R0      ;\
            SWAP    R0                  ; |
            ANDI    #$00FF,     R0      ; |- Player X position
            XOR@    R4,         R0      ; |
            MVO@    R0,         R5      ;/

            MVI     TODD.XP,    R0      ;\
            SWAP    R0                  ; |
            ANDI    #$00FF,     R0      ; |- Todd X position
            XOR@    R4,         R0      ; |
            MVO@    R0,         R5      ;/

            ADDI    #6,         R5
            
            MVI     PLYR.YP,    R0      ;\
            SWAP    R0                  ; |
            ANDI    #$007F,     R0      ; |- Player Y position
            XOR@    R4,         R0      ; |
            MVO@    R0,         R5      ;/

            MVI     TODD.YP,    R0      ;\
            SWAP    R0                  ; |
            ANDI    #$007F,     R0      ; |- Todd Y position
            XOR@    R4,         R0      ; |
            MVO@    R0,         R5      ;/

            ADDI    #6,         R5

            MVI@    R4,         R0      ; \_ Player's A register
            MVO@    R0,         R5      ; /
            MVI@    R4,         R0      ; \_ Todd's A register
            MVO@    R0,         R5      ; /                     

            CLRR    R0
            MVO     R0,         MOB_BUSY

            PULR    PC

            ;; ------------------------------------------------------------ ;;
            ;;  Bits to copy into MOB registers.                            ;;
            ;; ------------------------------------------------------------ ;;
@@mobr      DECLE   STIC.mobx_visb      ; make player visible
            DECLE   STIC.mobx_visb      ; make Todd visible

            DECLE   STIC.moby_yres      ; make player 8x16 MOB
            DECLE   STIC.moby_yres      ; make Todd 8x16 MOB

            DECLE   STIC.moba_fg1 + STIC.moba_gram + 0*8    ; Player is blue
            DECLE   STIC.moba_fg2 + STIC.moba_gram + 0*8    ; Todd is red
            ENDP
    

;; ======================================================================== ;;
;;  ISR -- Just keep the screen on, and copy the STIC shadow over.          ;;
;; ======================================================================== ;;
ISR         PROC

            ;; ------------------------------------------------------------ ;;
            ;;  Basics:  Update color stack and video enable.               ;;
            ;; ------------------------------------------------------------ ;;
            MVO     R0,     STIC.viden  ; Enable display
            MVI     STIC.mode, R0       ; ...in color-stack mode

            MVII    #C_GRY, R0          ;\
            MVO     R0,     STIC.cs0    ; |__ Set display to grey
            MVO     R0,     STIC.cs2    ; |
            MVO     R0,     STIC.bord   ;/

            ;; ------------------------------------------------------------ ;;
            ;;  Update STIC shadow and queue updates for MOB velocities.    ;;
            ;; ------------------------------------------------------------ ;;
            MVI     MOB_BUSY, R0
            TSTR    R0
            BNEQ    @@no_mobs
            MVO     PC,     MOB_BUSY


            CALL    MEMCPY              ;\__ Copy over the STIC shadow.
            DECLE   $0000, STICSH, 24   ;/

            MVII    #MOB_UPDATE, R0
            JSRD    R5,   QTASK
@@no_mobs:  

            CALL    DOTIMER             ; Update timer-based tasks.
            B       $1014               ; return from interrupt.
            ENDP

;; ======================================================================== ;;
;;  INITISR -- Copy our GRAM image over, and then do the plain ISR.         ;;
;; ======================================================================== ;;
INITISR     PROC
            PSHR    R5

            CALL    MEMCPY
            DECLE   $3800, GRAMIMG, GRAMIMG.end - GRAMIMG

            MVII    #ISR,   R0
            MVO     R0,     ISRVEC
            SWAP    R0
            MVO     R0,     ISRVEC + 1

            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  GRAMIMG -- Arrow pictures and other graphics to load into GRAM.         ;;
;; ======================================================================== ;;
GRAMIMG     PROC

@@person:   ; Crappy person graphic.
            DECLE   %00010000
            DECLE   %00111000
            DECLE   %00111000
            DECLE   %00010000
            DECLE   %00010000
            DECLE   %01111100
            DECLE   %10111010
            DECLE   %10111010
            DECLE   %10111010
            DECLE   %10111010
            DECLE   %00111000
            DECLE   %00101000
            DECLE   %00101000
            DECLE   %00101000
            DECLE   %00101000
            DECLE   %01101100
@@end:      
            ENDP

;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
            INCLUDE "../library/print.asm"       ; PRINT.xxx routines
            INCLUDE "../library/fillmem.asm"     ; CLRSCR/FILLZERO/FILLMEM
            INCLUDE "../library/memcpy.asm"      ; MEMCPY
            INCLUDE "../library/hexdisp.asm"     ; HEX16/HEX12
            INCLUDE "../task/scanhand.asm"       ; SCANHAND
            INCLUDE "../task/timer.asm"          ; Timer-based task stuff
            INCLUDE "../task/taskq.asm"          ; RUNQ/QTASK


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
