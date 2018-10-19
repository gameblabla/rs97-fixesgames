;;==========================================================================;;
;; Joe Zbiciak's ECS Keyboard Test                                          ;;
;; Copyright 2004, Joe Zbiciak, im14u2c AT globalcrossing DOT net           ;;
;;==========================================================================;;

;; ======================================================================== ;;
;;  Set up the cartridge                                                    ;;
;; ======================================================================== ;;
            INCLUDE "../macro/cart.mac"

            REQ_ECS
            ROMSETUP 16K, 2010, "ECS Keyboard Test", MAIN, 32

;;==========================================================================;;
;;                                                                          ;;
;;  Keyboard Layout:                                                        ;;
;;                                                                          ;;
;;                 [1] [2] [3] [4] [5] [6] [7] [8] [9] [0] [e]              ;;
;;                                                                          ;;
;;               [c] [Q] [W] [E] [R] [T] [Y] [U] [I] [O] [P]                ;;
;;                                                                          ;;
;;               [^]  [A] [S] [D] [F] [G] [H] [J] [K] [L] [;]               ;;
;;                                                                          ;;
;;             [<] [>] [Z] [X] [C] [V] [B] [N] [M] [,] [.]                  ;;
;;                         ________________________                         ;;
;;               [v] [s]  [________ SPACE _________] [s]  [r]               ;;
;;                                                                          ;;
;;==========================================================================;;


;------------------------------------------------------------------------------
; Include system information and macro libraries
;------------------------------------------------------------------------------
            INCLUDE "../library/gimini.asm"
            INCLUDE "../macro/util.mac"
            INCLUDE "../macro/stic.mac"
            INCLUDE "../macro/gfx.mac"
            INCLUDE "../macro/print.mac"

            INCLUDE "../ecs_kbd/scan_kbd.asm"

;; ======================================================================== ;;
;;  VARIABLES                                                               ;;
;; ======================================================================== ;;
TSKQM       EQU         15
MAXTSK      EQU         4
            BYTEVAR     OVRFLO                  ; overflow flag for taskq
            BYTEVAR     TSKQHD                  ; Task Queue head
            BYTEVAR     TSKQTL                  ; Task Queue tail
            WORDARRAY   TSKQ,    TSKQM + 1      ; Task Queue entries
            BYTEARRAY   TSKDQ,  (TSKQM + 1) * 2 ; Task Queue data

            BYTEVAR     TSKACT                  ; Timer tasks active
            WORDARRAY   TSKTBL, MAXTSK * 4      ; Timer task table

            WORDVAR     SHDISP                  ; SCANHAND Dispatch
            
            BYTEARRAY   SH_TMP, 7               ; SCANHAND temp variable
SH_LR0      EQU         SH_TMP + 1              ; \
SH_FL0      EQU         SH_TMP + 2              ;  |- Left controller vars
SH_LV0      EQU         SH_TMP + 3              ; /
SH_LR1      EQU         SH_TMP + 4              ; \
SH_FL1      EQU         SH_TMP + 5              ;  |- Right controller vars
SH_LV1      EQU         SH_TMP + 6              ; /

            BYTEARRAY   COLSTK, 5               ; Color-stack shadow
BORDER      EQU         COLSTK + 4              ; Border-color shadow
            
            BYTEVAR     CUR_ROW
            BYTEVAR     CUR_COL

;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
            INCLUDE "../library/print.asm"      ; PRINT.xxx routines
            INCLUDE "../library/hexdisp.asm"    ; HEXxx routines
            INCLUDE "../library/fillmem.asm"    ; CLRSCR/FILLZERO/FILLMEM
            INCLUDE "../library/memcpy.asm"     ; MEMCPY
            INCLUDE "../library/memunpk.asm"    ; MEMUNPK
            INCLUDE "../library/wnk.asm"        ; WAITKEY
            INCLUDE "../task/scanhand.asm"      ; Hand controller scanning
            INCLUDE "../task/timer.asm"         ; Timer-based tasks
            INCLUDE "../task/taskq.asm"         ; TASKQ 

ISRRET      EQU     $1014


;; ======================================================================== ;;
;;  MAIN:  Here's our main program code.                                    ;;
;; ======================================================================== ;;
MAIN:       PROC
            
            ;; ------------------------------------------------------------ ;;
            ;;  Title screen and initialization                             ;;
            ;; ------------------------------------------------------------ ;;
            SETISR  INITISR,  R0    ;\    Do GRAM initialization in ISR.
                                    ; |-- INITISR will the point to the 
                                    ;/    regular ISR when it's done.
          
            MVII    #$25D,  R1      ;\
            MVII    #$102,  R4      ; |-- Clear all of memory
            CALL    FILLZERO        ;/
            
                                    ;     01234567890123456789
            PRINT_CSTK  1,  2, White,      ">>> SDK-1600 <<<"
            PRINT_CSTK  2,  6, White,          "Presents"
            PRINT_CSTK  6,  2, Yellow,     "ECS Keyboard Test"
            PRINT_CSTK 10,  3, White,       "Copyright 2010"

            EIS
            CALL    WAITKEY
            CALL    CLRSCR          ; Clear the title screen after key-tap

            ;; ------------------------------------------------------------ ;;
            ;;  Set up our hand-controller dispatch.                        ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #HAND,  R0      ;\__ Set up scanhand dispatch table
            MVO     R0,     SHDISP  ;/

            ;; ------------------------------------------------------------ ;;
            ;;  Set up tasks for the main show.                             ;;
            ;; ------------------------------------------------------------ ;;
            CALL    STARTTASK
            DECLE   0
            DECLE   SCAN_KBD_EVT
            DECLE   2,2

            CALL    STARTTASK
            DECLE   1
            DECLE   CUR_BLINK   
            DECLE   10,10

            MVII    #2,     R0
            MVO     R0,     TSKACT

            MVII    #C_BLU, R0
            MVO     R0,     COLSTK + 1

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
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  HIT_ACTION -- Someone hit an action button.                             ;;
;; ======================================================================== ;;
HIT_ACTION  PROC
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  HIT_DISC   -- Someone hit a controller disc.                            ;;
;; ======================================================================== ;;
HIT_DISC    PROC
            JR      R5
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

            MVII    #COLSTK,    R4
            MVII    #STIC.cs0,  R5

            REPEAT  5
            MVI@    R4,         R0      ; \_ Copy over color-stack shadow
            MVO@    R0,         R5      ; /
            ENDR

            MVII    #ISRRET,    R5
            B       DOTIMER

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
;           CALL    MEMUNPK 
;           DECLE   $3800, GFX_DATA.gram_img, GFX.gram_size

            ;; ------------------------------------------------------------ ;;
            ;;  Do the main ISR starting with the next frame.               ;;
            ;; ------------------------------------------------------------ ;;
            SETISR  ISR,        R0

            JE      ISRRET              ; return and reenable interrupts.
            ENDP

;; ======================================================================== ;;
;;  SCAN_KBD_EVT    Scan the keyboard.  Queue an event if a key's pressed.  ;;
;; ======================================================================== ;;
SCAN_KBD_EVT    PROC
            PSHR    R5
            CALL    SCAN_KBD
            PULR    R5

            CMPI    #KEY.NONE,  R0
            BEQ     @@leave

            MOVR    R0,         R1
            MVII    #SCR_UPD,   R0
            JD      QTASK

@@leave:    JR      R5
            ENDP

;; ======================================================================== ;;
;;  SCR_UPD         Screen update                                           ;;
;; ======================================================================== ;;

SCR_UPD     PROC
            MVI     CUR_ROW,    R0      ; \
            SLL     R0,         2       ;  |
            MOVR    R0,         R1      ;  |_ Compute our posn based on row/col
            SLL     R0,         2       ;  |
            ADDR    R0,         R1      ;  |
            ADD     CUR_COL,    R1      ; /
            ADDI    #$200,      R1

            MVII    #C_WHT,     R0

            SUBI    #$20,       R2
            BGE     @@printable

            MVII    #C_RED,     R0
            ADDI    #$40,       R2
@@printable:

            SLL     R2,         2
            SLL     R2,         1
            ADDR    R0,         R2

            MVO@    R2,         R1

            INCR    R1
            MVI@    R1,         R0
            ANDI    #$DFFF,     R0
            MVO@    R0,         R1

            MVI     CUR_COL,    R1
            INCR    R1
            CMPI    #20,        R1
            BLT     @@col_ok

            MVI     CUR_ROW,    R1
            INCR    R1
            CMPI    #12,        R1
            BLT     @@row_ok

            CLRR    R1
@@row_ok:   MVO     R1,         CUR_ROW

            CLRR    R1
@@col_ok:   MVO     R1,         CUR_COL

            JR      R5
            ENDP


;; ======================================================================== ;;
;;  CUR_BLINK       Blink cursor                                            ;;
;; ======================================================================== ;;
CUR_BLINK   PROC
            MVI     CUR_ROW,    R0
            SLL     R0,         2
            MOVR    R0,         R1
            SLL     R0,         2
            ADDR    R0,         R1
            ADD     CUR_COL,    R1
            ADDI    #$200,      R1

            MVI@    R1,         R0
            XORI    #$2000,     R0
            MVO@    R0,         R1

            INCR    R1
            CMPI    #$2F0,      R1
            BEQ     @@leave

            MVI@    R1,         R0
            XORI    #$2000,     R0
            MVO@    R0,         R1

@@leave:    JR      R5
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
