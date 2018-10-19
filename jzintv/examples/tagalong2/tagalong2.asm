;;==========================================================================;;
;; Joe Zbiciak's Tag-Along Todd 2!                                          ;;
;; Copyright 2002, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  TO BUILD IN BIN+CFG FORMAT:                                             *;
;*      as1600 -o tagalong2.bin -l tagalong2.lst tagalong2.asm              *;
;*                                                                          *;
;*  TO BUILD IN ROM FORMAT:                                                 *;
;*      as1600 -o tagalong2.rom -l tagalong2.lst tagalong2.asm              *;
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
MAXTSK      EQU     2               ; Only one task
CAN         EQU     258 * 8 + C_DGR ; The can you're trying to collect
ARROW       EQU     259 * 8 + C_YEL ; The arrow for the menu.

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

            ; DEC16 temp storage
DEC_0       RMB     2
DEC_1       EQU     DEC_0 + 1

            ; Misc other stuff
GAME_LEN    RMB     1               ; Length of game in seconds
TIMELEFT    RMB     1               ; Time left in game.
SCORE       RMB     1               ; Score
NUM_CANS    RMB     1               ; Number of cans still onscreen.
M_ROW       RMB     1               ; menu row for arrow

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

            ; Misc other stuff
PLYR        PROC     
@@TXV       RMB     1               ; Target X velocity
@@XV        RMB     1               ; X velocity
@@XP        RMB     1               ; X position
@@TYV       RMB     1               ; Target Y velocity
@@YV        RMB     1               ; Y velocity
@@YP        RMB     1               ; Y position
            ENDP

TODD        PROC                    ; TODD's STATS
@@TXV       RMB     1               ; Target X velocity
@@XV        RMB     1               ; X velocity
@@XP        RMB     1               ; X position
@@TYV       RMB     1               ; Target Y velocity
@@YV        RMB     1               ; Y velocity
@@YP        RMB     1               ; Y position
            ENDP

RNDHI       RMB     1
RNDLO       RMB     1

MOB_BUSY    RMB     1               ; If non-zero, disables MOB updates

SKILL       RMB     1               ; Skill level [1 - 9]
DURATION    RMB     1               ; Game duration [1 - 9]
INIT_VEL    RMB     1               ; Initial velocity
TODD_VEL    RMB     1               ; Todd's velocity (game difficulty)
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
TITLE:      STRING  102, "Tag-Along Todd #2a", 0

;; ======================================================================== ;;
;;  MAIN:  Here's our main program code.                                    ;;
;; ======================================================================== ;;
MAIN:       PROC
            DIS
            MVII    #STACK, R6      ; Set up our stack

            MVII    #$25D,  R1      ;\
            MVII    #$102,  R4      ; |-- Clear all of memory
            CALL    FILLZERO        ;/

            MVO     PC,     MOB_BUSY; Disable MOBs for now.

            MVII    #INITISR, R0    ;\    Do GRAM initialization in ISR.
            MVO     R0,     ISRVEC  ; |__ INITISR will the point to the 
            SWAP    R0              ; |   regular ISR when it's done.
            MVO     R0,     ISRVEC+1;/    

            MVO     PC,     RNDHI
          
            EIS
@@gameloop: 
            CALL    TSCREEN         ; Show title screen
            CALL    RUNQ

            CALL    MENU            ; Get information
            CALL    RUNQ

            CALL    GAME            ; Run the game
            CALL    RUNQ

            CALL    GAMEOVER        ; Game over!
            CALL    RUNQ

            B       @@gameloop
            ENDP

;; ======================================================================== ;;
;;  HEXIT   Dispatch table that just calls SCHEDEXIT for everything.        ;;
;; ======================================================================== ;;
HEXIT       PROC
            DECLE   EXITPRESS
            DECLE   EXITPRESS
            DECLE   EXITPRESS
            ENDP

;; ======================================================================== ;;
;;  EXITPRESS -- Schedule an exit only when a key is pressed, not released. ;;
;; ======================================================================== ;;
EXITPRESS   PROC
            SWAP    R2,     2       ; test bit 7 of R2
            BPL     SCHEDEXIT       ; if clear, schedule the exit
            JR      R5              ; if set, ignore the keypress.
            ENDP

;; ======================================================================== ;;
;;  TSCREEN -- Title screen.                                                ;;
;; ======================================================================== ;;
TSCREEN     PROC
            PSHR    R5

            CALL    CLRSCR

            CALL    PRINT.FLS
            DECLE   C_BLU, $200 + 1*20 + 2
                    ;01234567890123456789
            STRING    ">>> SDK-1600 <<<  "
            STRING  "      presents",0

            CALL    PRINT.FLS
            DECLE   C_DGR, $200 + 6*20 + 2
                    ;01234567890123456789
            STRING    "Tag-Along Todd 2",0

            CALL    PRINT.FLS
            DECLE   C_BLK, $200 + 10*20 + 3
                    ;01234567890123456789
            STRING     "Copyright 2002", 0

            CLRR    R0
            MVO     R0,     TSKACT      ; No active tasks

            MVII    #HEXIT, R0
            MVO     R0,     SHDISP      ; Any controller input -> Exit screen

            MVO     PC,     MOB_BUSY    ; Disable MOB updates

            MVII    #STICSH, R4         ;\
            MVII    #24,    R1          ; |-- Clear away the MOBs
            CALL    FILLZERO            ;/

            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  M_HAND  Dispatch table for menu.                                        ;;
;; ======================================================================== ;;
M_HAND      PROC
            DECLE   M_DIGIT     ; Keypad dispatch
            DECLE   0           ; Action-button dispatch -> disabled
            DECLE   M_DISC      ; DISC dispatch
            ENDP

;; ======================================================================== ;;
;;  MENU    Display a menu onscreen.                                        ;;
;; ======================================================================== ;;
MENU        PROC
            PSHR    R5

            CALL    CLRSCR

            ;; ------------------------------------------------------------ ;;
            ;;  Display the menu screen.                                    ;;
            ;; ------------------------------------------------------------ ;;
            CALL    PRINT.FLS
            DECLE   C_DGR,  $200 + 2*20 + 1
                    ;01234567890123456789
            STRING  "Skill level [1-9]:", 0

            CALL    PRINT.FLS
            DECLE   C_DGR,  $200 + 5*20 + 1
                    ;01234567890123456789
            STRING  "Duration    [1-9]:", 0

            CALL    PRINT.FLS
            DECLE   C_BLU,  $200 + 3*20 + 9 
            STRING  "5", 0

            CALL    PRINT.FLS
            DECLE   C_BLU,  $200 + 6*20 + 9 
            STRING  "5", 0

            CALL    PRINT.FLS
            DECLE   C_BLU,  $200 + 9*20 + 8 
            STRING  "Go!", 0

            ;; ------------------------------------------------------------ ;;
            ;;  Set the defaults.                                           ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #$A0,   R0
            MVO     R0,     INIT_VEL
            MVII    #50,    R0
            MVO     R0,     GAME_LEN
            MVII    #5,     R0
            MVO     R0,     SKILL
            MVO     R0,     DURATION

            MVII    #3*20,  R0
            MVO     R0,     M_ROW

            ;; ------------------------------------------------------------ ;;
            ;;  Set the dispatch and menu animate task.                     ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #M_HAND,R0
            MVO     R0,     SHDISP

            CALL    STARTTASK           ;\
            DECLE   0                   ; |__ Blink the little arrow
            DECLE   M_BLINK             ; |
            DECLE   30,     30          ;/
            
            MVII    #1,     R0
            MVO     R0,     TSKACT
            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  M_DIGIT -- Enter a digit on the current menu row.                       ;;
;;             Pressing enter moves to next row, or if at last, starts game ;;
;; ======================================================================== ;;
M_DIGIT     PROC
            ANDI    #$FF,   R2          ; ignore controller #.
            CMPI    #$80,   R2
            BLT     @@press             ; ignore release events.
@@leave:    JR      R5

@@press:    TSTR    R2
            BEQ     @@leave             ; ignore 'zero'
            CMPI    #10,    R2
            BEQ     @@leave             ; ignore 'clear'
            BLT     @@digit

            ;; ------------------------------------------------------------ ;;
            ;;  Handle the [Enter] key:  Move to next row in menu, or if    ;;
            ;;  on last row of menu, start the game.                        ;;
            ;; ------------------------------------------------------------ ;;
@@enter:    MVI     M_ROW,  R4
            ADDI    #$200 + 12, R4
            CLRR    R0
            MVO@    R0,     R4          ; Clear arrow from current row

            MVI     M_ROW,  R1
            ADDI    #3*20,  R1          ; move to next row
            CMPI    #9*20,  R1          ; were we on last row?
            BGT     SCHEDEXIT           ; Enter on last row starts game. 

            MVO     R1,     M_ROW
            ADDI    #$200 + 12, R1
            MVII    #ARROW, R0
            MVO@    R0,     R1          ; Put arrow on current row
            JR      R5

            ;; ------------------------------------------------------------ ;;
            ;;  Handle digits [1] through [9].                              ;;
            ;; ------------------------------------------------------------ ;;
@@digit:    MVI     M_ROW,  R4
            CMPI    #6*20,  R4
            BGT     @@leave             ; Ignore digits when in last row
            BEQ     @@update_duration

            ;; ------------------------------------------------------------ ;;
            ;;  If we are on the top row, the input updates skill level.    ;;
            ;; ------------------------------------------------------------ ;;
            MVO     R2,     SKILL
            MOVR    R2,     R1          ; Update skill level
            SLL     R1,     2
            SLL     R1,     2
            SLL     R1,     1
            MVO     R1,     INIT_VEL    ; Todd's velocity = 0x20 * skill
            B       @@disp

            ;; ------------------------------------------------------------ ;;
            ;;  If we are on the middle row, the input updates the game     ;;
            ;;  duration.                                                   ;;
            ;; ------------------------------------------------------------ ;;
@@update_duration:
            MVO     R2,     DURATION
            MOVR    R2,     R1
            SLL     R1,     2
            ADDR    R2,     R1
            SLL     R1,     1
            MVO     R1,     GAME_LEN    ; Game length = 10 seconds * digit.

            ;; ------------------------------------------------------------ ;;
            ;;  Either way, if we're on the upper two rows, show the digit. ;;
            ;; ------------------------------------------------------------ ;;
@@disp:     ADDI    #$200+9,R4          ; offset to digit position in row.
            MOVR    R2,     R1
            SLL     R1,     2           ; move digit to card field in word
            SLL     R1,     1
            ADDI    #$80 + C_BLU, R1    ; add offset for '0', make digit blue
            MVO@    R1,     R4          ; show it.
            
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  M_DISC  -- Move the selection arrow between menu rows.                  ;;
;; ======================================================================== ;;
M_DISC      PROC
            ANDI    #$FF,   R2          ; ignore controller #.
            CMPI    #$80,   R2
            BLT     @@press             ; ignore release events.
@@leave:    JR      R5
@@press:
            MVI     M_ROW,  R0          ; Get menu row
            MOVR    R0,     R1          ; save old position

            CMPI    #2,     R2
            BLT     @@leave             ; Ignore 'east' (directions 0, 1)
            CMPI    #13,    R2
            BGT     @@leave             ; Ignore 'east' (directions 14, 15)
            CMPI    #5,     R2
            BLE     @@move_up           ; Directions 2 - 5:  Move up
            CMPI    #10,    R2
            BLT     @@leave             ; Ignore 'west' (directions 6 - 9)

        
@@move_dn:  ADDI    #3*20,  R0          ; Move down to next item
            CMPI    #9*20,  R0          ; Is it past the end?
            BGT     @@leave             ; Yes:  Ignore the input
            MVO     R0,     M_ROW       ; No:   Save the update

            ADDI    #$200 + 12, R1      ;\    Move to old arrow's position
            CLRR    R0                  ; |-- and clear the old arrow
            MVO@    R0,     R1          ;/
                                      
            ADDI    #3*20,  R1          ;\    Move to new arrow's position
            MVII    #ARROW, R0          ; |-- and draw the new arrow.
            MVO@    R0,     R1          ;/
            JR      R5                  ; return.
                                      
@@move_up   SUBI    #3*20,  R0          ; Move up to prev item
            CMPI    #3*20,  R0          ; Is it past the top
            BLT     @@leave             ; Yes:  Ignore the input
            MVO     R0,     M_ROW       ; No:   Save the update
                                      
            ADDI    #$200 + 12, R1      ;\    Move to old arrow's position
            CLRR    R0                  ; |-- and clear the old arrow
            MVO@    R0,     R1          ;/
                                      
            SUBI    #3*20,  R1          ;\    Move to new arrow's position
            MVII    #ARROW, R0          ; |-- and draw the new arrow.     
            MVO@    R0,     R1          ;/                                
            JR      R5                  ; return.                         
            
            ENDP

;; ======================================================================== ;;
;;  M_BLINK -- Blink the arrow on the menu.                                 ;;
;; ======================================================================== ;;
M_BLINK     PROC
            MVI     M_ROW,  R1          ; Get menu row
            ADDI    #$200 + 12, R1      ; Offset to arrow position
            MVII    #ARROW, R2          ;\
            XOR@    R1,     R2          ; |-- Toggle the arrow on and off.
            MVO@    R2,     R1          ;/
            JR      R5                  ; return.
            ENDP

;; ======================================================================== ;;
;;  GAMEOVER -- Game-over screen.                                           ;;
;; ======================================================================== ;;
GAMEOVER    PROC
            PSHR    R5

            ;; ------------------------------------------------------------ ;;
            ;;  Make a long 'ding'.                                         ;;
            ;;  Channel A Period = $0200     Channel A Volume = Envelope    ;;
            ;;  Envelope Period  = $3FFF     Envelope type = 0000           ;;
            ;;  Enables = Tone only on A, B, C.                             ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #$02,   R1
            MVO     R1,     PSG0.chn_a_hi
            MVII    #$38,   R1
            MVO     R1,     PSG0.chan_enable
            MVO     R1,     PSG0.chn_a_vol
            MVII    #$FF,   R1
            MVO     R1,     PSG0.envlp_lo
            MVII    #$3F,   R1
            MVO     R1,     PSG0.envlp_hi
            CLRR    R1
            MVO     R1,     PSG0.chn_a_lo
            MVO     R1,     PSG0.envelope

            ;; ------------------------------------------------------------ ;;
            ;;  Show the "Game Over!" message.  Also show the skill level   ;;
            ;;  and game duration, in case the player wants to make a       ;;
            ;;  screen-shot.                                                ;;
            ;; ------------------------------------------------------------ ;;
            CALL    PRINT.FLS
            DECLE   C_RED, $200 + 4*20 + 4
                    ;01234567890123456789
            STRING      " GAME OVER! ",0

            CALL    PRINT.FLS
            DECLE   C_BLU, $200 + 6*20 + 5
                    ;01234567890123456789
            STRING       " Skill:  ", 0

            MVI     SKILL,  R0
            SLL     R0,     2
            SLL     R0,     1
            ADDI    #$80 + C_DGR, R0
            MVO@    R0,     R4

            CALL    PRINT.FLS
            DECLE   C_BLU, $200 + 7*20 + 4
                    ;01234567890123456789
            STRING      " Length:  ", 0

            MVI     DURATION, R0
            SLL     R0,     2
            SLL     R0,     1
            ADDI    #$80 + C_DGR, R0
            MVO@    R0,     R4


            ;; ------------------------------------------------------------ ;;
            ;;  Force the MOBs to not update.  Clear them from the screen.  ;;
            ;; ------------------------------------------------------------ ;;
            MVO     PC,     MOB_BUSY

            MVII    #STICSH, R4
            MVII    #24,    R1
            CALL    FILLZERO

            ;; ------------------------------------------------------------ ;;
            ;;  Disable hand controllers for now, but schedule them to be   ;;
            ;;  set up in 2 seconds.  When they do get set up, pressing     ;;
            ;;  any key will go back to the menu.  This task is a one-shot. ;;
            ;; ------------------------------------------------------------ ;;
            CALL    STARTTASK
            DECLE   0
            DECLE   @@set_hexit
            DECLE   241,    241     ; Set up HEXIT as dispatch after 1 second

            CLRR    R0
            MVO     R0,     SHDISP  ; Until then, drop all controller input

            MVII    #1,     R0
            MVO     R0,     TSKACT

            PULR    PC              ; return to "RUNQ".


            ;; ------------------------------------------------------------ ;;
            ;;  This will get called in 1 second to set up the hand-        ;;
            ;;  controller dispatch.  The HEXIT dispatch will cause any     ;;
            ;;  keypress input to exit "GAME OVER" mode and go to the menu. ;;
            ;; ------------------------------------------------------------ ;;
@@set_hexit:
            MVII    #HEXIT, R0
            MVO     R0,     SHDISP
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  GAME -- Set up the game state.                                          ;;
;; ======================================================================== ;;
GAME        PROC
            PSHR    R5

            ;; ------------------------------------------------------------ ;;
            ;;  Set up the display.                                         ;;
            ;; ------------------------------------------------------------ ;;
            CALL    CLRSCR

            MVII    #$2000, R0
            MVO     R0,     $200 + 11*20    ; Bottom row is blue
            
            CALL    PRINT.FLS
            DECLE   C_YEL,  $200 + 11*20 + 1
                    ;01234567890123456789
            STRING   "Time:     Cans:    ",0

            MVII    #$80 + C_WHT, R0
            MVO     R0,     $200 + 11*20 + 18   ; 0 in 'score'

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
            DECLE   120, 40         ; 3Hz (three times a second)

            ;; ------------------------------------------------------------ ;;
            ;;  Set up round timer.                                         ;;
            ;; ------------------------------------------------------------ ;;
            CALL    STARTTASK
            DECLE   1
            DECLE   GAMETIME
            DECLE   2, 120

            MVI     GAME_LEN, R0
            INCR    R0
            MVO     R0,     TIMELEFT

            MVII    #2,     R0
            MVO     R0,     TSKACT

            ;; ------------------------------------------------------------ ;;
            ;;  Reset info for you and Todd.                                ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #PLYR,  R4
            MVII    #12,    R1
            CALL    FILLZERO

            ;; ------------------------------------------------------------ ;;
            ;;  Put you and Todd onscreen.                                  ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #$1000, R0
            MVO     R0,     PLYR.XP
            MVO     R0,     PLYR.YP

            MVII    #$9B00, R0
            MVO     R0,     TODD.XP
            MVII    #$5500, R0
            MVO     R0,     TODD.YP

            MVI     INIT_VEL,R0
            MVO     R0,     TODD_VEL

            ;; ------------------------------------------------------------ ;;
            ;;  Randomly display a dozen goodies.                           ;;
            ;; ------------------------------------------------------------ ;;
            CALL    DISPDOZ

            ;; ------------------------------------------------------------ ;;
            ;;  Enable MOBs.                                                ;;
            ;; ------------------------------------------------------------ ;;
            CLRR    R0
            MVO     R0,     MOB_BUSY

            ;; ------------------------------------------------------------ ;;
            ;;  Reset our score                                             ;;
            ;; ------------------------------------------------------------ ;;
            MVO     R0,     SCORE

            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  DISPDOZ  -- Display a dozen goodies.                                    ;;
;; ======================================================================== ;;
DISPDOZ     PROC
            PSHR    R5

            MVII    #CAN,   R2          ; Value to write to display pop can.
            MVII    #12,    R1          ; dozen == 12.  :-)
            MVO     R1,     NUM_CANS    ; Re-initialize our can counter.

@@gloop     MVII    #$8,    R0          ;\__ Generate 8 random bits
            CALL    RAND                ;/

            CMPI    #20*11, R0          ; Clamp to 0..219 (first 11 rows.)
            BGE     @@gloop             ; Too big?  Get another.

            ADDI    #$200,  R0          ; Make into a display offset.
            MOVR    R0,     R3          ; ... in a pointer-capable register
            CMP@    R3,     R2          ; Already a can there?
            BEQ     @@gloop             ; Yes:  Pick somewhere else.

            MVO@    R2,     R3          ; No:  Put a can there
            DECR    R1
            BNEQ    @@gloop


            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  GAMETIME -- The game timer.  Counts down remaining time in game.        ;;
;; ======================================================================== ;;
GAMETIME    PROC

            ;; ------------------------------------------------------------ ;;
            ;;  Todd gradually gets faster.                                 ;;
            ;; ------------------------------------------------------------ ;;
            MVI     TODD_VEL, R0
            INCR    R0
            MVO     R0,     TODD_VEL

            ;; ------------------------------------------------------------ ;;
            ;;  Count down the timer.                                       ;;
            ;; ------------------------------------------------------------ ;;
            MVI     TIMELEFT, R0
            DECR    R0
            BMI     SCHEDEXIT           ; Game is over if timer expires.
            MVO     R0,     TIMELEFT

            ;; ------------------------------------------------------------ ;;
            ;;  Make a short 'ding'.                                        ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #$40,   R1
            MVO     R1,     PSG0.chn_a_lo
            CLRR    R1
            MVO     R1,     PSG0.chn_a_hi
            MVII    #$38,   R1
            MVO     R1,     PSG0.chan_enable
            MVO     R1,     PSG0.chn_a_vol
            MVII    #$3F,   R1
            MVO     R1,     PSG0.envlp_lo
            CLRR    R1
            MVO     R1,     PSG0.envlp_hi
            MVO     R1,     PSG0.envelope
            
            ;; ------------------------------------------------------------ ;;
            ;;  Display 2-digit clock.  Time-left is still in R0.           ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #3,     R2          ; 2-digit field
            MVII    #C_WHT, R3      
            MVII    #$200+11*20+7, R4   ; Where to put time.
            B       DEC16A

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
;;  HIT_ACTION -- Someone hit an action key.                                ;;
;; ======================================================================== ;;
HIT_ACTION  PROC
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  HIT_DISC   -- Someone hit the disc.                                     ;;
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
            SARC    R1                  ; slow down a bit
            SARC    R2                  ; slow down a bit
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
            MVI     TODD_VEL,   R0      ; Player > Todd?  Move right
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
            MVI     TODD_VEL,   R1      ; Player > Todd?  Move right
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
            ;;  and apply the velocity to our position.  The routine        ;;
            ;;  @@update_velocity takes a pointer to a record with target   ;;
            ;;  velocity, current velocity, and current position.  It       ;;
            ;;  returns with R0 == current velocity, and R3 pointing to     ;;
            ;;  the current velocity location.                              ;;
            ;; ------------------------------------------------------------ ;;
            CALL    @@update_velocity   ; Update player's X velocity
            DECLE   PLYR.TXV
            INCR    R3                  ; Point to X position
            ADD@    R3,     R0          ; Add position to velocity
            CALL    @@clipx             ; Clip X position to display
            MVO@    R0,     R3          ; Store updated position

            CALL    @@update_velocity   ; Update player's Y velocity
            DECLE   PLYR.TYV
            INCR    R3                  ; Point to Y position
            ADD@    R3,     R0          ; Add position to velocity
            CALL    @@clipy             ; Clip Y position to display
            MVO@    R0,     R3          ; Store updated position

            CALL    @@update_velocity   ; Update Todd's X velocity
            DECLE   TODD.TXV
            INCR    R3                  ; Point to X position
            ADD@    R3,     R0          ; Add position to velocity
            CALL    @@clipx             ; Clip X position to display
            MVO@    R0,     R3          ; Store updated position

            CALL    @@update_velocity   ; Update Todd's Y velocity
            DECLE   TODD.TYV
            INCR    R3                  ; Point to Y position
            ADD@    R3,     R0          ; Add position to velocity
            CALL    @@clipy             ; Clip Y position to display
            MVO@    R0,     R3          ; Store updated position


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

            ;; ------------------------------------------------------------ ;;
            ;;  See if we're on a pop can.                                  ;;
            ;; ------------------------------------------------------------ ;;
            MVI     PLYR.YP,R1      ;\
            SWAP    R1              ; |
            SUBI    #4,     R1      ; |
            ANDI    #$F8,   R1      ; |
            MOVR    R1,     R2      ; |-- Generate row offset from Y coord.
            ADDR    R1,     R2      ; |
            SLR     R1,     1       ; |
            ADDR    R1,     R2      ;/
        
            MVI     PLYR.XP,R1      ;\
            SWAP    R1              ; |
            SUBI    #4,     R1      ; |
            ANDI    #$F8,   R1      ; |__ Generate column offset from X coord.
            SLR     R1,     2       ; |
            SLR     R1,     1       ; |
            ADDR    R1,     R2      ;/

            ADDI    #$200,  R2      ; Index into screen
            MVI@    R2,     R1
            CMPI    #CAN,   R1      ; A pop can here?
            BNEQ    @@no_can

            CLRR    R1
            MVO@    R1,     R2      ; Pick it up (clear it from screen)

            MVI     SCORE,  R0
            INCR    R0
            MVO     R0,     SCORE   ; Add 1 to score

            MVII    #$200+11*20+16, R4
            MVII    #$2,    R2      ; 3-digit field
            MVII    #C_WHT+$8000,R3 ; no leading zeros, score in white
            CALL    DEC16           ; Show updated score

            MVI     NUM_CANS, R0    ;\
            DECR    R0              ; |-- Decrement remaining can count
            MVO     R0, NUM_CANS    ;/
            BNEQ    @@some_left
            CALL    DISPDOZ         ; Display another dozen if we run out.
@@some_left:
@@no_can:

            ;; ------------------------------------------------------------ ;;
            ;;  See if Todd's caught us.  He's caught us if our coords are  ;;
            ;;  both within 4 pixels of each other.  This tight tolerance   ;;
            ;;  allows us to brush past Todd and not get caught.  :-)       ;;
            ;; ------------------------------------------------------------ ;;
            MVI     PLYR.XP,R0
            SUB     TODD.XP,R0
            ADCR    PC              ; skip NEGR if diff is positive
            NEGR    R0              ; make diff positive
            CMPI    #$400,  R0
            BC      @@todd_ok

            MVI     PLYR.YP,R0
            SUB     TODD.YP,R0
            ADCR    PC              ; skip NEGR if diff is positive
            NEGR    R0              ; make diff positive
            CMPI    #$400,  R0
            BC      @@todd_ok

            PULR    R5
            B       SCHEDEXIT       ; If Todd catches up to us, it's gameover

@@todd_ok
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


            ;; ------------------------------------------------------------ ;;
            ;;  Velocity update.  Add 1/4th the difference of our target    ;;
            ;;  velocity to our actual velocity.  We round the difference   ;;
            ;;  towards larger magnitude (eg. up if +ve, down if -ve) so    ;;
            ;;  that this finally converges on our target velocity.         ;;
            ;; ------------------------------------------------------------ ;;
@@update_velocity:
            MVI@    R5,         R3      ; Addr of target and current velocity
            MVI@    R3,         R0      ; Get target velocity
            INCR    R3                  ; Point to current velocity
            SUB@    R3,         R0      ; Find diff between target and current
            BMI     @@round_dn          ; Round -ve to -oo
            ADDI    #3,         R0      ; Round +ve to +oo
@@round_dn  SARC    R0,         2       ; Divide by 4 (right-shift by 2)
            ADD@    R3,         R0      ; Add update to current velocity
            MVO@    R0,         R3      ; Make updated velocity current.
            JR      R5

            ;; ------------------------------------------------------------ ;;
            ;;  Clipping code.  clipx clips for X dimension, clipy clips    ;;
            ;;  for Y dimension.  Goal:  Keep player and Todd onscreen.     ;;
            ;; ------------------------------------------------------------ ;;
@@clipx     CMPI    #$0900,     R0      ;\
            BC      @@xt_ok             ; |
            MVII    #$0900,     R0      ; |
            JR      R5                  ; |__ Limit X to 9 <= X <= 158
@@xt_ok     CMPI    #$9F00,     R0      ; |   (Note unsigned compares)
            BNC     @@xb_ok             ; |
            MVII    #$9F00,     R0      ; |
@@xb_ok     JR      R5                  ;/

@@clipy     CMPI    #$0900,     R0      ;\
            BC      @@yt_ok             ; |
            MVII    #$0900,     R0      ; |
            JR      R5                  ; |__ Limit Y to 9 <= Y <= 86
@@yt_ok     CMPI    #$5700,     R0      ; |   (Note unsigned compares)
            BNC     @@yb_ok             ; |
            MVII    #$5700,     R0      ; |
@@yb_ok     JR      R5                  ;/

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

            MVII    #C_GRY, R0          ;\___ Set main display to grey
            MVO     R0,     STIC.cs0    ;/
            MVII    #C_BLU, R0
            MVO     R0,     STIC.cs1    ;\___ Set border, bottom to blue
            MVO     R0,     STIC.bord   ;/

            ;; ------------------------------------------------------------ ;;
            ;;  Update STIC shadow and queue updates for MOB velocities.    ;;
            ;; ------------------------------------------------------------ ;;

            CALL    MEMCPY              ;\__ Copy over the STIC shadow.
            DECLE   $0000, STICSH, 24   ;/

            MVI     MOB_BUSY, R0        ; Skip MOB updates if told to.
            TSTR    R0
            BNEQ    @@no_mobs
            MVO     PC,     MOB_BUSY

            MVII    #MOB_UPDATE, R0
            JSRD    R5,   QTASK     ; Note JSRD:  Must disable ints for QTASK!
@@no_mobs:  

            ;; ------------------------------------------------------------ ;;
            ;;  Update timer-based tasks and return via stock interrupt     ;;
            ;;  return code.                                                ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #$1014, R5          ; return from interrupt address.
            B       DOTIMER             ; Update timer-based tasks.
            ENDP

;; ======================================================================== ;;
;;  INITISR -- Copy our GRAM image over, and then do the plain ISR.         ;;
;; ======================================================================== ;;
INITISR     PROC
            PSHR    R5

            CALL    MEMUNPK
            DECLE   $3800, GRAMIMG, GRAMIMG.end - GRAMIMG

            MVII    #ISR,   R0
            MVO     R0,     ISRVEC
            SWAP    R0
            MVO     R0,     ISRVEC + 1

            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  GRAMIMG -- Arrow pictures and other graphics to load into GRAM.         ;;
;;  These are stored two bytes-per-word.  Use MEMUNPK to load into GRAM.    ;;
;; ======================================================================== ;;
GRAMIMG     PROC

@@person:   ; Crappy person graphic.   (2 cards)
            ; ...#.... 0
            ; ..###... 1
            ; ..###... 2
            ; ...#.... 3
            ; ...#.... 4
            ; .#####.. 5
            ; #.###.#. 6
            ; #.###.#. 7
            ; #.###.#. 8
            ; #.###.#. 9
            ; ..###... A
            ; ..#.#... B
            ; ..#.#... C
            ; ..#.#... D
            ; ..#.#... E
            ; .##.##.. F
            DECLE   %0011100000010000 ;10
            DECLE   %0001000000111000 ;32
            DECLE   %0111110000010000 ;54
            DECLE   %1011101010111010 ;76
            DECLE   %1011101010111010 ;98
            DECLE   %0010100000111000 ;BA
            DECLE   %0010100000101000 ;DC
            DECLE   %0110110000101000 ;FE

@@can:      ; Pop Can graphic
            ; ........ 0
            ; ........ 1
            ; .###.... 2
            ; #...#... 3
            ; .#####.. 4
            ; ..#####. 5
            ; ...###.. 6
            ; ........ 7
            DECLE   %0000000000000000 ;10
            DECLE   %1000100001110000 ;32
            DECLE   %0011111001111100 ;54
            DECLE   %0000000000011100 ;76

@@arrow:    ; Arrow graphic
            ; ........ 0
            ; ........ 1
            ; ..##.... 2
            ; .##..... 3
            ; ######## 4
            ; .##..... 5
            ; ..##.... 6
            ; ........ 7
            DECLE   %0000000000000000 ; 10
            DECLE   %0110000000110000 ; 32
            DECLE   %0110000011111111 ; 54
            DECLE   %0000000000110000 ; 76

@@end:      
            ENDP

;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
            INCLUDE "../library/print.asm"       ; PRINT.xxx routines
            INCLUDE "../library/fillmem.asm"     ; CLRSCR/FILLZERO/FILLMEM
            INCLUDE "../library/memcpy.asm"      ; MEMCPY
            INCLUDE "../library/memunpk.asm"     ; MEMUNPK
            INCLUDE "../library/rand.asm"        ; RAND
            INCLUDE "../library/dec16only.asm"   ; DEC16
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
