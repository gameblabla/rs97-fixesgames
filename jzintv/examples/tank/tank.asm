;;==========================================================================;;
;; Joe Zbiciak's Singed Earth?                                              ;;
;; Copyright 2003, Joe Zbiciak, im14u2c AT globalcrossing DOT net           ;;
;;==========================================================================;;

;------------------------------------------------------------------------------
; Include system information and macro libraries
;------------------------------------------------------------------------------
            INCLUDE "../library/gimini.asm"
            INCLUDE "../macro/util.mac"
            INCLUDE "../macro/stic.mac"
            INCLUDE "../macro/gfx.mac"
            INCLUDE "../macro/print.mac"

;------------------------------------------------------------------------------
; Global constants and configuration.
;------------------------------------------------------------------------------
TSKQM       EQU     $7              ; Task queue is 8 entries large

;------------------------------------------------------------------------------
; Allocate 8-bit variables in Scratch RAM
;------------------------------------------------------------------------------
SCRATCH     ORG     $100, $100, "-RWBN"

ISRVEC      RMB     2               ; Always at $100 / $101

            ; Task-oriented 8-bit variables
TSKQHD      RMB     1               ; Task queue head
TSKQTL      RMB     1               ; Task queue tail
TSKDQ       RMB     2*(TSKQM+1)     ; Task data queue

            ; Hand-controller 8-bit variables
SH_TMP      RMB     1               ; Temp storage.
SH_LR0      RMB     3               ;\
SH_FL0      EQU     SH_LR0 + 1      ; |-- Three bytes for left controller
SH_LV0      EQU     SH_LR0 + 2      ;/
SH_LR1      RMB     3               ;\
SH_FL1      EQU     SH_LR1 + 1      ; |-- Three bytes for right controller
SH_LV1      EQU     SH_LR1 + 2      ;/

            ; MOB data tables
MOB_PIC     RMB     16              ; MOB picture table
MOB_ATR     RMB     8               ; MOB attribute table
MOB_XYP     RMB     32              ; MOB X/Y position table
MOB_FLG     RMB     1               ; MOB update flags.

            ; Tank status variables
TNK0_VEL    RMB     1               ; Tank #0 desired velocity
TNK1_VEL    RMB     1               ; Tank #1 desired velocity
TNK0_MOV    RMB     1               ; Tank #0 movement (actual velocity)
TNK1_MOV    RMB     1               ; Tank #1 movement (actual velocity)
TNK0_AVL    RMB     1               ; Tank #0 angular velocity for turret
TNK1_AVL    RMB     1               ; Tank #1 angular velocity for turret
TNK0_ANG    RMB     1               ; Tank #0 turret angle (in 10 deg incrs.)
TNK1_ANG    RMB     1               ; Tank #1 turret angle (in 10 deg incrs.)
TNK0_POW    RMB     1               ; Tank #0 firing strength
TNK1_POW    RMB     1               ; Tank #1 firing strength
TNK0_FLP    RMB     1               ; Tank #0 flipped left/right
TNK1_FLP    RMB     1               ; Tank #1 flipped left/right

B0_TICK     RMB     1               ; Bullet 0 "no-hit timer" counter
B1_TICK     RMB     1               ; Bullet 1 "no-hit timer" counter

            ; Misc other stuff
FR_BUSY     RMB     1               ; Per-frame updates busy flag.
AVL_TICK    RMB     1               ; tick counter for angular velocity upd.
DO_STATS    RMB     1               ; need to draw status line.

_SCRATCH    EQU     $               ; end of scratch area



;------------------------------------------------------------------------------
; Allocate 16-bit variables in System RAM 
;------------------------------------------------------------------------------
SYSTEM      ORG     $2F0, $2F0, "-RWBN"
STACK       RMB     32              ; Reserve 32 words for the stack

            ; Task-oriented 16-bit variables
TSKQ        RMB     (TSKQM + 1)     ; Task queue

            ; Hand-controller 16-bit variables
SHDISP      RMB     1               ; ScanHand dispatch

            ; STIC shadow
MOB_DIS     RMB     1               ; MOB collision dispatch table ptr
MOB_IGN     RMB     1               ; MOB collision ignore table ptr
STICSH      RMB     32              ; Room for X, Y, and A regs only.

            ; Tank status
TNK0_PTS    RMB     1
TNK1_PTS    RMB     1

            ; Bullet details
BUL0_XVL    RMB     1               ; bullet 0's X velocity (8.8)
BUL0_YVL    RMB     1               ; bullet 0's Y velocity (8.8)
BUL1_XVL    RMB     1               ; bullet 1's X velocity (8.8)
BUL1_YVL    RMB     1               ; bullet 1's Y velocity (8.8)

_SYSTEM     EQU     $               ; end of system area


;------------------------------------------------------------------------------
; Misc equates
;------------------------------------------------------------------------------
ISRRET      EQU     $1014           ; EXEC's ISR return pointer

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
            STRING  103, "Tank", 0
            BEGIN
          
            ; Put up our own title screen.
            CALL    CLRSCR
                                    ;     01234567890123456789
            PRINT_CSTK  1,  2, White,      ">>> SDK-1600 <<<"
            PRINT_CSTK  2,  6, White,          "Presents"
            PRINT_CSTK  6,  4, Yellow,       "Singed Earth"
            PRINT_CSTK 10,  3, White,       "Copyright 2003"
          
            ; Done.
            RETURN                  ; Return to EXEC for title screen display
            ENDP

;; ======================================================================== ;;
;;  PROGRAM-SPECIFIC MACROS AND UTILITY FUNCTIONS.                          ;;
;; ======================================================================== ;;
            INCLUDE "util.asm"                  ; Utility functions, macros

;; ======================================================================== ;;
;;  DATA                                                                    ;;
;; ======================================================================== ;;
            INCLUDE "gfx_data.asm"              ; pictures
            INCLUDE "atr_data.asm"              ; STIC attribute records
            INCLUDE "mob_data.asm"              ; MOB collision dispatch tbls

;; ======================================================================== ;;
;;  PROGRAM MODULES                                                         ;;
;; ======================================================================== ;;
            INCLUDE "mob_ll.asm"                ; Low-level MOB routines
            INCLUDE "objects.asm"               ; Tank/bullet code
            INCLUDE "status.asm"                ; Status bar code

;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
            INCLUDE "../library/print.asm"      ; PRINT.xxx routines
            INCLUDE "../library/prnum16.asm"    ; PRNUM16.x routines
            INCLUDE "../library/fillmem.asm"    ; CLRSCR/FILLZERO/FILLMEM
            INCLUDE "../library/memcpy.asm"     ; MEMCPY
            INCLUDE "../library/memunpk.asm"    ; MEMCPY
            INCLUDE "../task/scanhand.asm"      ; SCANHAND
            INCLUDE "../task/taskq.asm"         ; RUNQ/QTASK

;; ======================================================================== ;;
;;  MAIN:  Here's our main program code.                                    ;;
;; ======================================================================== ;;
MAIN:       PROC
            DIS
            MVII    #STACK, R6      ; Set up our stack

            MVII    #$25D,  R1      ;\
            MVII    #$102,  R4      ; |-- Clear all of memory
            CALL    FILLZERO        ;/

            SETISR  INITISR,  R0    ;\    Do GRAM initialization in ISR.
                                    ; |-- INITISR will the point to the 
                                    ;/    regular ISR when it's done.
          
            ;; ------------------------------------------------------------ ;;
            ;;  Set up our hand-controller dispatch.                        ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #HAND,  R0      ;\__ Set up scanhand dispatch table
            MVO     R0,     SHDISP  ;/

            CALL    INIT_TANK
            CALL    INIT_STAT

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
            PRINT_CSTK 11, 0, Red, "SCHEDEXIT WAS CALLED"
            DECR    PC              ; Can't get here normally
          
            ENDP

STUB        JR      R5

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
            SWAP    R2
            MOVR    R2,         R1      ;\__ R1 gets tank #
            ANDI    #1,         R1      ;/
            ADDI    #TNK0_POW,  R1      ; point to tank firing strength
            SWAP    R2
            BPL     @@pressed
            JR      R5

@@pressed:  ANDI    #$FF,       R2      ;\__ [0] key means '10'
            BEQ     @@force10           ;/
            CMPI    #10,        R2      ;\__ [1]..[9] mean 1 through 9
            BLE     @@ok                ;/
@@force10:  MVII    #10,        R2      ;    [C], [E], and [0] are all 10

@@ok:       MVO@    R2,         R1      ; Store firing strength for this tank

            MVII    #1,         R1      ;\__ need to update status bar.
            MVO     R1,         DO_STATS;/
            JR      R5

            ENDP

;; ======================================================================== ;;
;;  HIT_ACTION -- Someone hit a key on a keypad.                            ;;
;; ======================================================================== ;;
HIT_ACTION  PROC
            SWAP    R2
            MOVR    R2,         R1      ;\__ R1 gets tank #
            ANDI    #1,         R1      ;/
            MOVR    R1,         R0      ; save tank #
            ADDI    #TNK0_AVL,  R1      ; point to turret angular velocity
            SWAP    R2
            BPL     @@pressed

@@stop_tur  CLRR    R2
@@upd_angv  MVO@    R2,         R1      ; store updated turret velocity
@@leaveR5   JR      R5

@@pressed   ANDI    #$FF,       R2
            DECR    R2                  ; 1 is "top", 2 is "left", 3 is "right"
            BNEQ    @@upd_angv          ; if it's 1 or 2, update angular vel.
            MVO@    R2,         R1      ; otherwise force angular vel to 0

@@fire:     B       FIRE_BULLET         ; Fire the bullet.  (Chained return)
            
            ENDP

;; ======================================================================== ;;
;;  HIT_DISC   -- Someone hit a key on a keypad.                            ;;
;; ======================================================================== ;;
HIT_DISC    PROC

            SWAP    R2
            MOVR    R2,     R1
            ANDI    #1,     R1      ; R1 gets Tank # (0 or 1)
            ADDI    #TNK0_VEL,R1
            SWAP    R2
            BPL     @@pressed

@@stop:     CLRR    R3
@@store:    MVO@    R3,     R1
            JR      R5

@@pressed:  ADDI    #4,     R2      ; rotate disc.  SSE thru NNE go right
            ANDI    #$F,    R2
            BEQ     @@stop          ; position 0 is south.  Discard.
            CMPI    #8,     R2
            BEQ     @@stop          ; position 8 is north.  Discard.
            BLT     @@go_right      ; less than 8 is right, greater is left.

            ;; go left
            MVII    #$FE,   R3      ; This velocity will be divided by 8
            B       @@store

@@go_right: ;; go right
            MVII    #$02,   R3      ; This velocity will be divided by 8
            B       @@store

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

            MVII    #C_CYN, R0          ;\__ sky is cyan
            MVO     R0,     STIC.cs0    ;/
            MVII    #C_BLU, R0          ;\__ status is blue
            MVO     R0,     STIC.cs1    ;/
            MVII    #C_GRY, R0          ;\__ border is grey
            MVO     R0,     STIC.bord   ;/

            CALL    MOB_ISR             ; Update MOBs


            MVI     FR_BUSY,    R0      ;\    test per-frame busy flag.  
            XORI    #1,         R0      ; |__ never let it be 1 more than
            MVO     R0,         FR_BUSY ; |   one frame at a time, though,   
            BEQ     ISRRET              ;/    in case task queue saturates.

            CALL    MOB_COLDISP         ; do collision dispatches in ISR ctx.

            MVII    #FR_TASK,   R0      ; \
            MVII    #ISRRET,    R5      ;  |- Queue the per-frame work
            JD      QTASK               ; /   and return via QTASK

            ENDP

;; ======================================================================== ;;
;;  FR_TASK -- per frame work to do.                                        ;;
;; ======================================================================== ;;
FR_TASK     PROC
            PSHR    R5
            ;; ------------------------------------------------------------ ;;
            ;;  Per-tick work to do.                                        ;;
            ;;   -- Move tanks based on velocity.                           ;;
            ;;   -- Update ballistics                                       ;;
            ;;   -- Update STIC shadow                                      ;;
            ;;   -- Scan for collision dispatches.                          ;;
            ;; ------------------------------------------------------------ ;;

            CALL    UPD_TANKS 
            CALL    UPD_BULLET
            CALL    MOB_STICSH
            CALL    UPD_STATUS

            CLRR    R0
            MVO     R0,         FR_BUSY ; let a new frame's work begin

            PULR    PC                  ; done
            ENDP


;; ======================================================================== ;;
;;  INITISR -- Copy our GRAM image over, and then do the plain ISR.         ;;
;; ======================================================================== ;;
INITISR     PROC
            DIS     ; this could take a couple frames

            ;; ------------------------------------------------------------ ;;
            ;;  Force the MOBs to all be disabled, with no collision bits.  ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #32,        R1      ;\
            MVII    #0,         R4      ; |-- Clear MOB registers
            CALL    FILLZERO            ;/

            ;; ------------------------------------------------------------ ;;
            ;;  Load our initial GRAM image.                                ;;
            ;; ------------------------------------------------------------ ;;
            CALL    MEMUNPK 
            DECLE   $3800, GFX_DATA.gram_img, GFX.gram_size

            ;; ------------------------------------------------------------ ;;
            ;;  Do the main ISR starting with the next frame.               ;;
            ;; ------------------------------------------------------------ ;;
            SETISR  ISR,        R0

            JE      ISRRET              ; return and reenable interrupts.
            ENDP



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
;*                   Copyright (c) 2003, Joseph Zbiciak                     *;
;* ======================================================================== *;
