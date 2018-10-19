;;==========================================================================;;
;; Joe Zbiciak's SCANHAND DEMO                                              ;;
;; Copyright 2002, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  TO BUILD IN BIN+CFG FORMAT:                                             *;
;*      as1600 -o handdemo.bin -l handdemo.lst handdemo.asm                 *;
;*                                                                          *;
;*  TO BUILD IN ROM FORMAT:                                                 *;
;*      as1600 -o handdemo.rom -l handdemo.lst handdemo.asm                 *;
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
; Global constants and configuration.
;------------------------------------------------------------------------------

TSKQM       EQU     $7              ; Task queue is 8 entries large
SCAN_ECS    EQU     1               ; Allow scanning ECS hand controllers
                                    ; comment out SCAN_ECS to build w/out ECS.

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
    IF DEFINED SCAN_ECS
SH_LR2      RMB     3               ;\
SH_FL2      EQU     SH_LR2 + 1      ; |-- Three bytes for ECS left controller
SH_LV2      EQU     SH_LR2 + 2      ;/
SH_LR3      RMB     3               ;\
SH_FL3      EQU     SH_LR3 + 1      ; |-- Three bytes for ECS right controller
SH_LV3      EQU     SH_LR3 + 2      ;/
    ENDI

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
STICSH      RMB     24              ; Room for X, Y, and A regs only.

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
            STRING  102
                    ;01234567890123456789
            STRING  '   Hand Controller  '
            STRING  '    Scanning Demo   '
            STRING  '                    ', 0
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
          
            EIS

            MVII    #HAND,  R0      ;\__ Set up scanhand dispatch table
            MVO     R0,     SHDISP  ;/

@@loop:     
            ;; ------------------------------------------------------------ ;;
            ;;  Display the controller status screen.                       ;;
            ;; ------------------------------------------------------------ ;;
            CALL    PRINT.FLS
            DECLE   C_WHT, $200
                    ;01234567890123456789
            STRING  "  L    R    L    R  ",0

            CALL    PRINT.FLS
            DECLE   C_TAN, $200 + 20
                    ;01234567890123456789
            ;       ".123..123..123..123."
            STRING  231, "123", 230
            STRING  231, "123", 230
            STRING  231, "123", 230
            STRING  231, "123", 230
            STRING  " 456  456  456  456 "
            ;       ".789..789..789..789."
            STRING  231, "789", 230
            STRING  231, "789", 230
            STRING  231, "789", 230
            STRING  231, "789", 230
            STRING  " C0E  C0E  C0E  C0E "
            ;       " /#\  /#\  /#\  /#\ "
            STRING  " ",152, 241, 153, " "
            STRING  " ",152, 241, 153, " "
            STRING  " ",152, 241, 153, " "
            STRING  " ",152, 241, 153, " "
            ;       " ###  ###  ###  ### "
            STRING  " ",196, 127, 197, " "
            STRING  " ",196, 127, 197, " "
            STRING  " ",196, 127, 197, " "
            STRING  " ",196, 127, 197, " "
            ;       " \#/  \#/  \#/  \#/ "
            STRING  " ",154, 240, 155, " "
            STRING  " ",154, 240, 155, " "
            STRING  " ",154, 240, 155, " "
            STRING  " ",154, 240, 155, " ", 0

            CALL    PRINT.FLS
            DECLE   C_GRN, $200 + 8*20
            STRING  " ---  ---  ---  --- "
            STRING  " ---  ---  ---  --- "
            STRING  " ---  ---  ---  --- ", 0

            CALL    PRINT.FLS
            DECLE   X_WHT, $200 + 11*20
            STRING  "  MASTER    E.C.S.  ",0


            ;; ------------------------------------------------------------ ;;
            ;;  Go fix color-stack down columns, so that controllers are    ;;
            ;;  brown and rest of screen is blue                            ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #$200 + 20 + 1, R4
            MVII    #STIC.cs_advance, R2
            MOVR    R4,     R5
            MVII    #7 * 4, R1
@@cslp:     
            MVI@    R4,     R0
            XORR    R2,     R0
            MVO@    R0,     R5
            ADDI    #2,     R4
            ADDI    #2,     R5
            MVI@    R4,     R0
            XORR    R2,     R0
            MVO@    R0,     R5
            INCR    R4
            INCR    R5
            DECR    R1
            BNEQ    @@cslp

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
;;  HAND    Dispatch table.                                                 ;;
;; ======================================================================== ;;
HAND        PROC
            DECLE   HIT_KEYPAD
            DECLE   HIT_ACTION
            DECLE   HIT_DISC
            ENDP

;; ======================================================================== ;;
;;  CALC_OFS   -- Return 5 * controller number in R3.                       ;;
;; ======================================================================== ;;
CALC_OFS    PROC
            MOVR    R2,     R3
            ANDI    #$FF00, R3
            SLL     R3,     2
            ADDR    R2,     R3
            SWAP    R3
            ANDI    #$00FF, R3
            JR      R5
            ENDP
            

;; ======================================================================== ;;
;;  HIT_KEYPAD -- Someone hit a key on a keypad.                            ;;
;; ======================================================================== ;;
HIT_KEYPAD  PROC
            PSHR    R5
            PSHR    R2

            ;; ------------------------------------------------------------ ;;
            ;;  Display the event's numeric information onscreen.           ;;
            ;; ------------------------------------------------------------ ;;
            CALL    CALC_OFS        ;\ 
            MVII    #$200 + 1, R4   ; |- Go to correct column by controller #
            ADDR    R3,     R4      ;/
            PSHR    R4              ; Save column pointer
            ADDI    #8 * 20,  R4    ; Offset to "keypad" row
            CALL    SHOW_IT         ; Show the event.

            ;; ------------------------------------------------------------ ;;
            ;;  Now show the event on the little controller picture.        ;;
            ;; ------------------------------------------------------------ ;;
            PULR    R4              ;\__ offset to top of keypad 
            ADDI    #20,    R4      ;/

            ;; ------------------------------------------------------------ ;;
            ;;  Is this a key-release or a new key?                         ;;
            ;; ------------------------------------------------------------ ;;
            PULR    R2
            ANDI    #$FF,   R2      ; ignore controller # bits now.
            CMPI    #$80,   R2      ; Is it key release?
            BLT     @@new_key       ; If it's >= $80, it is.  Otherwise no.

            ;; ------------------------------------------------------------ ;;
            ;;  Clear out any previously displayed keys.                    ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #4,     R0
            MVII    #$FFF8, R1
            MVII    #C_TAN, R2
            MOVR    R4,     R5
@@clr_lp:   
            MVI@    R4,     R3  ; get character
            ANDR    R1,     R3  ; remove color
            XORR    R2,     R3  ; make it TAN
            MVO@    R3,     R5  ; store character
            MVI@    R4,     R3  ; get character  
            ANDR    R1,     R3  ; remove color   
            XORR    R2,     R3  ; make it TAN    
            MVO@    R3,     R5  ; store character
            MVI@    R4,     R3  ; get character  
            ANDR    R1,     R3  ; remove color   
            XORR    R2,     R3  ; make it TAN    
            MVO@    R3,     R5  ; store character
            ADDI    #17,    R4  ; move to next row
            MOVR    R4,     R5
            DECR    R0          ; do four rows
            BNEQ    @@clr_lp

            PULR    PC          ; Done!

            ;; ------------------------------------------------------------ ;;
            ;;  Display the newly received key event.                       ;;
            ;; ------------------------------------------------------------ ;;
@@new_key:
            ADDI    #@@ofs, R2  ; Turn key number into display offset
            ADD@    R2,     R4  ; Generate pointer to number
            MVI@    R4,     R0  ; Get the number
            ANDI    #$FFF8, R0  ; Clear out the color
            XORI    #C_WHT, R0  ; Make it white
            DECR    R4          ;\__ Display the key in white
            MVO@    R0,     R4  ;/

            PULR    PC          ; Done!

@@ofs:      DECLE             3*20 + 1              ;   0
            DECLE   0*20 + 0, 0*20 + 1, 0*20 + 2    ; 1 2 3
            DECLE   1*20 + 0, 1*20 + 1, 1*20 + 2    ; 4 5 6
            DECLE   2*20 + 0, 2*20 + 1, 2*20 + 2    ; 7 8 9
            DECLE   3*20 + 0,           3*20 + 2    ; C   E

            ENDP

;; ======================================================================== ;;
;;  HIT_ACTION -- Someone hit a key on a keypad.                            ;;
;; ======================================================================== ;;
HIT_ACTION  PROC
            PSHR    R5
            PSHR    R2

            ;; ------------------------------------------------------------ ;;
            ;;  Display the event's numeric information onscreen.           ;;
            ;; ------------------------------------------------------------ ;;
            CALL    CALC_OFS        ;\ 
            MVII    #$200 + 1, R4   ; |- Go to correct column by controller #
            ADDR    R3,     R4      ;/
            PSHR    R4              ; Save column pointer
            ADDI    #9 * 20,  R4    ; Offset to "action" row
            CALL    SHOW_IT         ; Show the event.

            PULR    R4
            PULR    R2

            ANDI    #$FF,   R2  ; Ignore controller number now.

            ;; ------------------------------------------------------------ ;;
            ;;  Now, update the four action button pictures.                ;;
            ;; ------------------------------------------------------------ ;;
@@show_act: MOVR    R4,     R3
            MVII    #$FFF8, R1  ; color-clearing mask
            MVII    #C_TAN, R4  ; Default:  Tan
            DECR    R2          ; Is it the top action button(s)? (R2 == 1)
            BNEQ    @@not_top
            MVII    #C_WHT, R4
@@not_top:
            ADDI    #19,    R3  ; point to top-left button
            MVI@    R3,     R0  ; Get top-left
            ANDR    R1,     R0  ; Clear color
            XORR    R4,     R0  ; set to white or tan
            MVO@    R0,     R3  ; store top-left

            ADDI    #4,     R3  ; point to top-right button
            MVI@    R3,     R0  ; Get top-right
            ANDR    R1,     R0  ; clear color
            XORR    R4,     R0  ; set to white or tan
            MVO@    R0,     R3  ; store top-right

            MVII    #C_TAN, R4  ; Default: Tan
            DECR    R2          ; Is it the lower-left button? (R2 == 2)
            BNEQ    @@not_lft
            MVII    #C_WHT, R4
@@not_lft:
            ADDI    #36,    R3  ; Point to lower-left button
            MVI@    R3,     R0  ; Get lower-left
            ANDR    R1,     R0  ; Clear color
            XORR    R4,     R0  ; set to white or tan
            MVO@    R0,     R3  ; store lower-left
            
            MVII    #C_TAN, R4  ; Default: Tan
            DECR    R2          ; Is it the right-left button? (R2 == 3)
            BNEQ    @@not_rgt
            MVII    #C_WHT, R4
@@not_rgt:
            ADDI    #4,     R3  ; Point to lower-right button
            MVI@    R3,     R0  ; Get lower-right
            ANDR    R1,     R0  ; Clear color
            XORR    R4,     R0  ; set to white or tan
            MVO@    R0,     R3  ; store lower-right
            
            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  HIT_DISC   -- Someone hit a key on a keypad.                            ;;
;; ======================================================================== ;;
HIT_DISC    PROC
            PSHR    R5
            PSHR    R2

            ;; ------------------------------------------------------------ ;;
            ;;  Display the event's numeric information onscreen.           ;;
            ;; ------------------------------------------------------------ ;;
            CALL    CALC_OFS
            MOVR    R3,     R4
            PSHR    R4
            ADDI    #$200 + 1 + 10*20,  R4
            CALL    SHOW_IT

            PULR    R4              ; Controller number * 5
            PULR    R2              ; Full event information

            
            MOVR    R2,     R1      ;\
            SWAP    R1              ; |__ Separate into R1 == controller #
            ANDI    #$3,    R1      ; |   and R2 == direction.
            ANDI    #$FF,   R2      ;/

            ;; ------------------------------------------------------------ ;;
            ;;  If DISC is released, clear the arrow for the DISC.          ;;
            ;; ------------------------------------------------------------ ;;
            CMPI    #$80,   R2
            BLT     @@pressed

            ;; ------------------------------------------------------------ ;;
            ;;  Released, so clear MOB #n, where n == controller #.         ;;
            ;; ------------------------------------------------------------ ;;
            CLRR    R0
            ADDI    #STICSH,R1      ; point R1 at STIC shadow for MOB #n
            MVO@    R0,     R1      ; clear X register
            ADDI    #8,     R1
            MVO@    R0,     R1      ; clear Y register
            ADDI    #8,     R1
            MVO@    R0,     R1      ; clear A register
            
            PULR    PC

            ;; ------------------------------------------------------------ ;;
            ;;  Display an arrow onscreen to show what direction the DISC   ;;
            ;;  is pointing.                                                ;;
            ;; ------------------------------------------------------------ ;;
@@pressed:
            MOVR    R4,     R3      ;\
            SLL     R3,     2       ; |-- R3 = 40*controller #
            SLL     R3,     1       ;/

            MVII    #@@arrows,  R5  ; point to arrows
            ADDR    R2,     R5      ;\
            ADDR    R2,     R5      ; |-- 3 decles per direction #.
            ADDR    R2,     R5      ;/

            MVII    #STICSH,R4      ;\__ point to shadow for MOB #n
            ADDR    R1,     R4      ;/

            ADD@    R5,     R3      ; Calculate X for MOB #n
            MVO@    R3,     R4      ; Store to shadow X register
            ADDI    #7,     R4
            MVI@    R5,     R3      ; Get Y for MOB #n
            MVO@    R3,     R4      ; Store to shadow Y register
            ADDI    #7,     R4
            MVI@    R5,     R3      ; Get A for MOB #n
            MVO@    R3,     R4      ; Store to shadow X register
            

            PULR    PC              ; Done!
    
            ;; ------------------------------------------------------------ ;;
            ;;  Lookup-table for the 16 arrows.  Contains the X, Y, and A   ;;
            ;;  register settings for each of the 16 directions.  Add the   ;;
            ;;  controller-specific X offset to the X value to display it   ;;
            ;;  correctly.                                                  ;;
            ;; ------------------------------------------------------------ ;;
@@arrows:   
@@0         DECLE   28    + STIC.mobx_visb 
            DECLE   53    + STIC.moby_yres 
            DECLE   X_WHT + STIC.moba_gram + 0 * 16

@@1         DECLE   28    + STIC.mobx_visb 
            DECLE   52    + STIC.moby_yres 
            DECLE   X_WHT + STIC.moba_gram + 1 * 16

@@2         DECLE   28    + STIC.mobx_visb 
            DECLE   52    + STIC.moby_yres 
            DECLE   X_WHT + STIC.moba_gram + 2 * 16

@@3         DECLE   28    + STIC.mobx_visb 
            DECLE   52    + STIC.moby_yres 
            DECLE   X_WHT + STIC.moba_gram + 3 * 16

@@4         DECLE   26    + STIC.mobx_visb 
            DECLE   52    + STIC.moby_yres 
            DECLE   X_WHT + STIC.moba_gram + 4 * 16

@@5         DECLE   20    + STIC.mobx_visb 
            DECLE   52    + STIC.moby_yres + STIC.moby_xflip
            DECLE   X_WHT + STIC.moba_gram + 3 * 16

@@6         DECLE   20    + STIC.mobx_visb 
            DECLE   52    + STIC.moby_yres + STIC.moby_xflip
            DECLE   X_WHT + STIC.moba_gram + 2 * 16

@@7         DECLE   20    + STIC.mobx_visb 
            DECLE   52    + STIC.moby_yres + STIC.moby_xflip
            DECLE   X_WHT + STIC.moba_gram + 1 * 16

@@8         DECLE   20    + STIC.mobx_visb 
            DECLE   53    + STIC.moby_yres + STIC.moby_xflip
            DECLE   X_WHT + STIC.moba_gram + 0 * 16

@@9         DECLE   20    + STIC.mobx_visb 
            DECLE   60    + STIC.moby_yres + STIC.moby_xflip + STIC.moby_yflip
            DECLE   X_WHT + STIC.moba_gram + 1 * 16

@@10        DECLE   20    + STIC.mobx_visb 
            DECLE   60    + STIC.moby_yres + STIC.moby_xflip + STIC.moby_yflip
            DECLE   X_WHT + STIC.moba_gram + 2 * 16

@@11        DECLE   20    + STIC.mobx_visb 
            DECLE   60    + STIC.moby_yres + STIC.moby_xflip + STIC.moby_yflip
            DECLE   X_WHT + STIC.moba_gram + 3 * 16

@@12        DECLE   22    + STIC.mobx_visb 
            DECLE   60    + STIC.moby_yres + STIC.moby_xflip + STIC.moby_yflip
            DECLE   X_WHT + STIC.moba_gram + 4 * 16

@@13        DECLE   28    + STIC.mobx_visb 
            DECLE   60    + STIC.moby_yres + STIC.moby_yflip
            DECLE   X_WHT + STIC.moba_gram + 3 * 16

@@14        DECLE   28    + STIC.mobx_visb 
            DECLE   60    + STIC.moby_yres + STIC.moby_yflip
            DECLE   X_WHT + STIC.moba_gram + 2 * 16

@@15        DECLE   28    + STIC.mobx_visb 
            DECLE   60    + STIC.moby_yres + STIC.moby_yflip
            DECLE   X_WHT + STIC.moba_gram + 1 * 16

            ENDP

;; ======================================================================== ;;
;;  SHOW_IT    -- Show what we were passed.                                 ;;
;; ======================================================================== ;;
SHOW_IT     PROC
            PSHR    R5
            PSHR    R2
            PSHR    R4

            MVII    #$200 + 8*20, R4
            MOVR    R4,     R5
            MVII    #$FFFF XOR X_PUR, R2
            MVII    #C_GRN, R3          
            MVII    #60,    R1
@@grey:     MVI@    R4,     R0
            ANDR    R2,     R0
            XORR    R3,     R0
            MVO@    R0,     R5
            DECR    R1
            BNEQ    @@grey

            PULR    R4
            PULR    R0

            CLRR    R2
            MVO     R2,     $200 +  8*20 + 4
            MVO     R2,     $200 +  9*20 + 4
            MVO     R2,     $200 + 10*20 + 4
            NOP
            MVO     R2,     $200 +  8*20 + 9
            MVO     R2,     $200 +  9*20 + 9
            MVO     R2,     $200 + 10*20 + 9
            NOP
            MVO     R2,     $200 +  8*20 + 14
            MVO     R2,     $200 +  9*20 + 14
            MVO     R2,     $200 + 10*20 + 14
            NOP
            MVO     R2,     $200 +  8*20 + 19
            MVO     R2,     $200 +  9*20 + 19
            MVO     R2,     $200 + 10*20 + 19
            NOP

            MVII    #C_YEL, R1
            CALL    HEX12

            MVII    #C_YEL + 10 * 8, R0
            MVO@    R0,     R4
            PULR    PC

            ENDP

;; ======================================================================== ;;
;;  ISR -- Just keep the screen on, and copy the STIC shadow over.          ;;
;; ======================================================================== ;;
ISR         PROC
            PSHR    R5

            MVO     R0,     STIC.viden  ; Enable display
            MVI     STIC.mode, R0       ; ...in color-stack mode

            MVII    #C_BLU, R0          ;\
            MVO     R0,     STIC.cs0    ; |__ Set display to blue
            MVO     R0,     STIC.cs2    ; |
            MVO     R0,     STIC.bord   ;/
            MVII    #C_BRN, R0          ;\
            MVO     R0,     STIC.cs1    ; |-- Set controllers to brown
            MVO     R0,     STIC.cs3    ;/

            CALL    MEMCPY              ;\__ Copy over the STIC shadow.
            DECLE   $0000, STICSH, 24   ;/

            PULR    PC                  ; return
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

            PULR    R5
            B       ISR
            ENDP

;; ======================================================================== ;;
;;  GRAMIMG -- Arrow pictures and other graphics to load into GRAM.         ;;
;; ======================================================================== ;;
GRAMIMG     PROC

@@aro0:     ; Left-pointing arrow
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000100
            DECLE   %00000010
            DECLE   %11111111
            DECLE   %00000010
            DECLE   %00000100

@@aro1:     ; 22.5-degree arrow
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000111
            DECLE   %00000011
            DECLE   %00000101
            DECLE   %00001000
            DECLE   %00010000
            DECLE   %00100000
            DECLE   %01000000
            DECLE   %10000000

@@aro2:     ; 45-degree arrow
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00000000
            DECLE   %00011110
            DECLE   %00000110
            DECLE   %00001010
            DECLE   %00001000
            DECLE   %00010000
            DECLE   %00010000
            DECLE   %00100000
            DECLE   %00100000
            DECLE   %01000000
            DECLE   %01000000
            DECLE   %10000000
            DECLE   %10000000

@@aro3:     ; 67.5-degree arrow
            DECLE   %00000000
            DECLE   %00010000
            DECLE   %00111000
            DECLE   %01010100
            DECLE   %00010000
            DECLE   %00100000
            DECLE   %00100000
            DECLE   %00100000
            DECLE   %00100000
            DECLE   %01000000
            DECLE   %01000000
            DECLE   %01000000
            DECLE   %01000000
            DECLE   %10000000
            DECLE   %10000000
            DECLE   %10000000

@@aro4:     ; 90-degree arrow
            DECLE   %00100000
            DECLE   %01110000
            DECLE   %10101000
            DECLE   %00100000
            DECLE   %00100000
            DECLE   %00100000
            DECLE   %00100000
            DECLE   %00100000
            DECLE   %00100000
            DECLE   %00100000
            DECLE   %00100000
            DECLE   %00100000
            DECLE   %00100000
            DECLE   %00100000
            DECLE   %00100000
            DECLE   %00100000

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
            INCLUDE "../task/taskq.asm"          ; RUNQ/QTASK

