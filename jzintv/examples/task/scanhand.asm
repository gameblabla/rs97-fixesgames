;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;;==========================================================================;;
;; Joe Zbiciak's Task Routines:  Hand Controller Scanning.                  ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

;; ======================================================================== ;;
;;  GLOBAL VARIABLES USED BY THESE ROUTINES                                 ;;
;;                                                                          ;;
;;  Note that some of these routines may use one or more global variables.  ;;
;;  If you use these routines, you will need to allocate the appropriate    ;;
;;  space in either 16-bit or 8-bit memory as appropriate.  Each global     ;;
;;  variable is listed with the routines which use it and the required      ;;
;;  memory width.                                                           ;;
;;                                                                          ;;
;;  Example declarations for these routines are shown below, commented out. ;;
;;  You should uncomment these and add them to your program to make use of  ;;
;;  the routine that needs them.  Make sure to assign these variables to    ;;
;;  locations that aren't used for anything else.  I may show addresses in  ;;
;;  the table below:  Ignore these!  They are illustrative only.            ;;
;; ======================================================================== ;;

                        ; Used by     Req'd Width   Description
                        ;-----------------------------------------------------
;SHDISP EQU     SHDISP  ; SCANHAND    16-bit        Dispatch table
;SH_TMP EQU     $11F    ; SCANHAND    8-bit         Temporary storage.
;SH_LR0 EQU     $120    ; SCANHAND    8-bit         Last read value
;SH_FL0 EQU     SH_LR0+1; SCANHAND    8-bit         Flags for left ctrl
;SH_LV0 EQU     SH_LR0+2; SCANHAND    8-bit         Last valid value
;SH_LR1 EQU     $123    ; SCANHAND    8-bit         Last read value
;SH_FL1 EQU     SH_LR1+1; SCANHAND    8-bit         Flags for right ctrl
;SH_LV1 EQU     SH_LR1+2; SCANHAND    8-bit         Last valid value
; if SCAN_ECS is defined, the following additional variables are needed.
;SH_LR2 EQU     $126    ; SCANHAND    8-bit         Last read value
;SH_FL2 EQU     SH_LR2+1; SCANHAND    8-bit         Flags for ECS left ctrl
;SH_LV2 EQU     SH_LR2+2; SCANHAND    8-bit         Last valid value
;SH_LR3 EQU     $129    ; SCANHAND    8-bit         Last read value
;SH_FL3 EQU     SH_LR3+1; SCANHAND    8-bit         Flags for ECS right ctrl
;SH_LV3 EQU     SH_LR3+2; SCANHAND    8-bit         Last valid value

;; ======================================================================== ;;
;;  SCANHAND                                                                ;;
;;                                                                          ;;
;;  Scan the hand controllers and schedule tasks based on the input         ;;
;;  triggers we see.  Hand controller debouncing is done here too, but it   ;;
;;  is performed over several calls to SCANHAND.  Therefore, ensure that    ;;
;;  SCANHAND is called regularly, with a relatively even frequency.         ;;
;;                                                                          ;;
;;  CONTROLLER DEBOUNCING                                                   ;;
;;                                                                          ;;
;;  If SH_DEBOUNCE is defined, the debounce timer is initialized to its     ;;
;;  value.  Otherwise, the default is 12.  The default is suitable if you   ;;
;;  are calling SCANHAND very regularly from the background task in RUNQ.   ;;
;;  It is not suitable if you are calling SCANHAND from an ISR.             ;;
;;                                                                          ;;
;;  For ISR-driven hand-controller scanning, I'd recommend setting          ;;
;;  SH_DEBOUNCE to about 2 or 3.                                            ;;
;;                                                                          ;;
;;  Larger values of SH_DEBOUNCE reduce the responsiveness of SCANHAND,     ;;
;;  but improve its resistance to glitchy inputs.  Smaller values of        ;;
;;  SH_DEBOUNCE will improve the responsiveness at the expense of glitches. ;;
;;  If your code can tolerate the occasional glitchy input, go ahead and    ;;
;;  crank down the debounce count.                                          ;;
;;                                                                          ;;
;;  The valid range of SH_DEBOUNCE is 0 to 31.  Values outside this range   ;;
;;  will cause the code to assemble with the default value of 12.           ;;
;;                                                                          ;;
;;  DISPATCH TABLES                                                         ;;
;;                                                                          ;;
;;  Hand controller events are handled by jumping to functions defined in   ;;
;;  a 3-entry dispatch table.  This table may be stored in ROM.  The        ;;
;;  variable SHDISP stores a pointer to this table.  The table is           ;;
;;  formatted as follows:                                                   ;;
;;                                                                          ;;
;;       TABLE   DECLE   keypad_dispatch_function                           ;;
;;               DECLE   action_button_dispatch_function                    ;;
;;               DECLE   disc_dispatch_function                             ;;
;;                                                                          ;;
;;  NOTE:  This table format requires a 16-bit ROM width.  It is trivial    ;;
;;  to modify the code to support narrower ROMs, although I don't see the   ;;
;;  point.                                                                  ;;
;;                                                                          ;;
;;  DISPATCH FUNCTIONS                                                      ;;
;;                                                                          ;;
;;  Hand controller events are queued for processing using QTASK, and       ;;
;;  are processed along with all the other queued tasks.  When the          ;;
;;  task is executed, it will receive information about the controller      ;;
;;  event in the following format:                                          ;;
;;                                                                          ;;
;;       15                     9   8   7                         0         ;;
;;      +---------------------+-------+---+------------------------+        ;;
;;      |       RESERVED      |CTRL # |RLS|      Input Number      |        ;;
;;      +---------------------+-------+---+------------------------+        ;;
;;                                                                          ;;
;;  The reserved bits presently are returned as 0 but may someday return    ;;
;;  something else.  The "Input Number" depends on the type of dispatch.    ;;
;;  Those are documented below.  The fixed bits are defined as follows:     ;;
;;                                                                          ;;
;;        Bit #     Name      Description                                   ;;
;;      --------- ---------- ---------------------------------------        ;;
;;       15 - 10   Reserved   Reserved                                      ;;
;;                                                                          ;;
;;        9 - 8    CTRL #     Controller number:                            ;;
;;                               00 -- Master Component Left Side           ;;
;;                               01 -- Master Component Right Side          ;;
;;                               10 -- ECS Left Side                        ;;
;;                               11 -- ECS Right Side                       ;;
;;                                                                          ;;
;;          7      RLS        Set if key/input was released.                ;;
;;                            NOTE:  This code returns 0 for input #        ;;
;;                            on a key-release event!                       ;;
;;                                                                          ;;
;;        6 - 0    INPUT #    Actual key/input received.                    ;;
;;                                                                          ;;
;;  INPUT NUMBER DEFINITIONS                                                ;;
;;                                                                          ;;
;;  The Input Number says which input was received for a given type of      ;;
;;  input.  For all three (keypad, action, and disc), values >= $80 mean    ;;
;;  the previous input was released.                                        ;;
;;                                                                          ;;
;;  KEYPAD INPUT NUMBERS                                                    ;;
;;                                                                          ;;
;;       Number      Interpretation                                         ;;
;;       ----------- -----------------------------------------------        ;;
;;       0 - 9       Digit 0 through Digit 9                                ;;
;;       10          Clear                                                  ;;
;;       11          Enter                                                  ;;
;;       12 - 127    Reserved                                               ;;
;;       $80 - $FF   Keypad released.                                       ;;
;;                                                                          ;;
;;  ACTION BUTTON INPUT NUMBERS                                             ;;
;;                                                                          ;;
;;       Number      Interpretation                                         ;;
;;       ----------- -----------------------------------------------        ;;
;;       0           Reserved                                               ;;
;;       1           Top action button                                      ;;
;;       2           Bottom-left action button                              ;;
;;       3           Bottom-right action button                             ;;
;;       4 - 127     Reserved                                               ;;
;;       $80 - $FF   Action button released.                                ;;
;;                                                                          ;;
;;  DISC INPUT NUMBERS                                                      ;;
;;                                                                          ;;
;;       Number      Interpretation                                         ;;
;;       ----------- -----------------------------------------------        ;;
;;       0           Compass direction:  E                                  ;;
;;       1           Compass direction:  ENE                                ;;
;;       2           Compass direction:  NE                                 ;;
;;       3           Compass direction:  NNE                                ;;
;;       4           Compass direction:  N                                  ;;
;;       5           Compass direction:  NNW                                ;;
;;       6           Compass direction:  NW                                 ;;
;;       7           Compass direction:  WNW                                ;;
;;       8           Compass direction:  W                                  ;;
;;       9           Compass direction:  WSW                                ;;
;;       10          Compass direction:  SW                                 ;;
;;       11          Compass direction:  SSW                                ;;
;;       12          Compass direction:  S                                  ;;
;;       13          Compass direction:  SSE                                ;;
;;       14          Compass direction:  SE                                 ;;
;;       15          Compass direction:  ESE                                ;;
;;       16 - 127    Reserved                                               ;;
;;       $80 - $FF   DISC released.                                         ;;
;;                                                                          ;;
;;  ECS SUPPORT                                                             ;;
;;                                                                          ;;
;;  This code does support the ECS's hand controller ports.  It does        ;;
;;  not verify that both ports are set for "input".  It also does not       ;;
;;  check for the presence of the ECS.                                      ;;
;;                                                                          ;;
;;  To include ECS support, define the symbol "SCAN_ECS" prior to           ;;
;;  including this file.  Enabling ECS support should not affect the        ;;
;;  operation of this code even if the ECS is not present.  It just         ;;
;;  makes the code slightly larger and slower.                              ;;
;;                                                                          ;;
;;  INPUT PROCESSING                                                        ;;
;;                                                                          ;;
;;  This code handles two basic operating modes for the hand-controller:    ;;
;;  Keypad, and DISC + Action.  In Keypad mode, it can recognize only       ;;
;;  one keypad press at a time -- additional keypad presses are ignored.    ;;
;;  In DISC + Action mode, it can recognize one Action-button press at      ;;
;;  a time, and continuously-variable DISC inputs.                          ;;
;;                                                                          ;;
;;  The mode selection is automatic.  When the controller is idle (no       ;;
;;  input), the code waits for an input.  Upon seeing and debouncing an     ;;
;;  input, it first attempts to decode the input as a keypad press.  If     ;;
;;  it succeeds, it goes into Keypad mode, and triggers an event for the    ;;
;;  keypad press.                                                           ;;
;;                                                                          ;;
;;  If the input does not match a valid keypad value, it then attempts      ;;
;;  to decode the input as an Action-key press.  If it succeeds, then       ;;
;;  it goes into DISC+Action, and it triggers an event for the action       ;;
;;  key.                                                                    ;;
;;                                                                          ;;
;;  Next, if the input does not match a valid DISC value, it then           ;;
;;  attempts to decode the input as a DISC press.  If it succeeds, it       ;;
;;  goes into DISC+Action mode, and it triggers an event for the DISC       ;;
;;  press.                                                                  ;;
;;                                                                          ;;
;;  Finally, if the input is unrecognized (dirty controller?), it'll        ;;
;;  remember that it saw the input, but it won't trigger an event.          ;;
;;                                                                          ;;
;;  GLITCH HANDLING                                                         ;;
;;                                                                          ;;
;;  We all know that Intellivision hand controllers can be very glitchy.    ;;
;;  Decoding the controllers correctly in all cases with reasonable         ;;
;;  responsiveness and minimal "false inputs" is nearly impossible.         ;;
;;                                                                          ;;
;;  This code implements some rules to deal with the glitchiness.           ;;
;;                                                                          ;;
;;   -- If Bit #4 is set, only DISC and ACTION inputs will be recognized.   ;;
;;      This bit is only supposed to be set by DISC inputs.                 ;;
;;                                                                          ;;
;;   -- Once a KEYPAD input has been recognized, no further inputs will     ;;
;;      be decoded until the controller is released.  This is because       ;;
;;      the KEYPAD inputs alias all other inputs.                           ;;
;;                                                                          ;;
;;   -- Once a DISC or ACTION input has been recognized, only DISC          ;;
;;      and ACTION keypressed will be recognized until the controller       ;;
;;      is released.                                                        ;;
;;                                                                          ;;
;;   -- There is one exception to these rules:  Sometimes, a dirty contact  ;;
;;      on a keypad button will cause it to momentarily read as a DISC      ;;
;;      input.  In this case, the DISC input has one of four values (0, 4,  ;;
;;      8, 12).  If the second switch in the keypad closes, we ordinarily   ;;
;;      wouldn't return the keypress.  Thus, we'd be unable to register     ;;
;;      this keypress.                                                      ;;
;;                                                                          ;;
;;      The following rules govern overriding DISC mode to return the       ;;
;;      keypress in this case:                                              ;;
;;                                                                          ;;
;;       -- Last input was DISC only.  (No ACTION, no KEYPAD).              ;;
;;       -- No DISC bits were released.                                     ;;
;;       -- The new input EXACTLY matches a keypad value.                   ;;
;;                                                                          ;;
;;      If these criteria are met, a disc-up event is sent, and the code    ;;
;;      processes the new keypad event.  To the game, this sequence will    ;;
;;      look like a short tap of the DISC followed by the actual keypress.  ;;
;;                                                                          ;;
;;   -- Only one ACTION key will be recognized at a time.  The ACTION-key   ;;
;;      release event will only be sent when all ACTION keys are released.  ;;
;;                                                                          ;;
;;                                                                          ;;
;;  HAND CONTROLLER DOCS  (Condensed version)                               ;;
;;                                                                          ;;
;;       $1FE    Master Component Right Controller                          ;;
;;       $1FF    Master Component Left Controller                           ;;
;;       $0FE    ECS Right Controller                                       ;;
;;       $0FF    ECS Left Controller                                        ;;
;;                                                                          ;;
;;                                                                          ;;
;;               DISC    ACTION  KEYPAD                                     ;;
;;       Bit 0:  Down            Row 0 keys --------- 1    2    3           ;;
;;       Bit 1:  Right           Row 1 keys --------- 4    5    6           ;;
;;       Bit 2:  Up              Row 2 keys --------- 7    8    9           ;;
;;       Bit 3:  Left            Row 3 keys --------- C    0    E           ;;
;;       Bit 4:  Corner                               |    |    |           ;;
;;       Bit 5:            T/L   Col 2 keys --------- | -- | ---+           ;;
;;       Bit 6:            L/R   Col 1 keys --------- | ---+                ;;
;;       Bit 7:            T/R   Col 0 keys ----------+                     ;;
;;                                                                          ;;
;;       The 'corner' bit combines with the other bits from the DISC to     ;;
;;       sort among the various diagonals.                                  ;;
;; ======================================================================== ;;

SCANHAND    PROC
@@fd        EQU     $01                 ; FLAG:  Last had disc pressed
@@fa        EQU     $02                 ; FLAG:  Last had action pressed
@@fk        EQU     $04                 ; FLAG:  Last had keypad pressed
@@fmsk      EQU     @@fd+@@fa+@@fk      ; Mask for all three flags.

@@dbc       EQU     $07                 ; debounce mask clear-mask
@@dbd       EQU     $08                 ; debounce decrement

    IF DEFINED SH_DEBOUNCE
    IF (SH_DEBOUNCE > -1) AND (SH_DEBOUNCE < 32)

@@dbs       EQU     SH_DEBOUNCE*@@dbd   ; debounce counter initialization

    ENDI 
    ENDI 

    IF (DEFINED @@dbs) = 0

@@dbs       EQU     12 * @@dbd          ; debounce counter initialization

    ENDI

 
            PSHR    R5

    IF DEFINED SCAN_ECS

            MVI     $00F8,  R1      ;\
            TSTR    R1              ; |-- Check for an ECS
            BMI     @@no_ecs        ;/
            ANDI    #$3F,   R1      ;\___ Force both ECS controller ports 
            MVO     R1,     $00F8   ;/    to input mode.

            MVII    #$0FE,  R1      ; ECS Left controller
            MVII    #SH_LR3,R2      ; Point to last-hand reading for left side
            CALL    @@doside        ; Process ECS left hand controller

            MVII    #$0FF,  R1      ; ECS Right controller
            MVII    #SH_LR2,R2      ; Point to last-hand reading for right side
            CALL    @@doside        ; Process ECS right hand controller

@@no_ecs:
    ENDI

            MVII    #$1FE,  R1      ; Left controller
            MVII    #SH_LR1,R2      ; Point to last-hand reading for left side
            CALL    @@doside        ; Process the left hand controller

            MVII    #$1FF,  R1      ; Right controller
            MVII    #SH_LR0,R2      ; Point to last-hand reading for right side
            INCR    PC              ; (skip PSHR R5)

@@doside:   PSHR    R5

            ;; ------------------------------------------------------------ ;;
            ;;  Read the controller and see if it's changed since last.     ;;
            ;; ------------------------------------------------------------ ;;
            MVI@    R1,     R0      ; Read controller
            CMP@    R2,     R0      ; Is it same as last time?
            BEQ     @@same0         ; Yes -- count down bounce timers.

            ;; ------------------------------------------------------------ ;;
            ;;  Not same:  Set the bounce timer, update the last-read val   ;;
            ;;  and return.                                                 ;;
            ;; ------------------------------------------------------------ ;;
            MVO@    R0,     R2      ; Store updated "last-read value"

            INCR    R2              ; point to flags
            MVII    #@@dbc, R3      ;\ 
            AND@    R2,     R3      ; |- clear old debounce count and put
            XORI    #@@dbs, R3      ;/   in new debounce count.
            MVO@    R3,     R2      ; store updated flags

            PULR    PC              ; return.

            ;; ------------------------------------------------------------ ;;
            ;;  Input was the same.  Decrement the debounce timer and only  ;;
            ;;  process it when the timer is expired.                       ;;
            ;; ------------------------------------------------------------ ;;
@@same0:
            INCR    R2              ; point to flags
            MVI@    R2,     R3      ; get flags

            SUBI    #@@dbd, R3      ; decrement the debounce timer
            BMI     @@expired       ; ignore input after debounce expires
            MVO@    R3,     R2      ; Not expired, so save out update.

            CMPI    #@@dbd, R3
            BLE     @@expiring      ; if about to expire, process input

@@expired:  PULR    PC              ; Otherwise, exit.

@@expiring: INCR    R2              ; point to last valid value 
            MOVR    R0,     R4
            XORI    #$FF,   R4      ;
            CMP@    R2,     R4      ; Is it same as last-valid?  
            BEQ     @@expired       ; Same?  Ignore the input glitch.

            PSHR    R0              ; Save new input on stack.
                                    ; Keep in mind -- new input is still
                                    ; inverted at this point!

            ;; ------------------------------------------------------------ ;;
            ;;  If it's changed, first decide if we need to send any        ;;
            ;;  release events.                                             ;;
            ;; ------------------------------------------------------------ ;;

            ANDI    #@@dbc, R3      ; Get flags for this side

            ;; ------------------------------------------------------------ ;;
            ;;  Check for DISC release.  Also, check for a sloppy keypad    ;;
            ;;  sending disc-like events (grrr...).                         ;;
            ;; ------------------------------------------------------------ ;;
            MOVR    R3,     R4
            ANDI    #@@fd,  R4      ; Get "DISC" bit
            BEQ     @@check_rls_act ; Clear == no disc

            MVII    #$1F,   R4      ;\
            ANDR    R0,     R4      ; |-- If DISC field is clear, DISC is up.
            CMPI    #$1F,   R4      ;/
            BNEQ    @@check_false_disc

            ; do "disc up" event.
@@disc_up   XORI    #@@fd,  R3      ; clear DISC bit
            CALL    @@release
            DECLE   2

            B       @@check_rls_act

@@check_false_disc:
            MOVR    R0,     R4      ;\
            AND@    R2,     R4      ; |-- First condition:  Nothing released
            BNEQ    @@check_rls_act ;/

            MOVR    R3,     R4      ;\    
            ANDI    #@@fa,  R4      ; |-- Second condition:  Action key is
            BNEQ    @@check_rls_act ;/    NOT pressed.                    

            MOVR    R0,     R4      ;\
            XORI    #$FF,   R4      ; |                                   
            MVII    #@@pad, R5      ; |__ Third condition:  Exactly match
@@fd_lp:    CMP@    R5,     R4      ; |   one of the pad entries.         
            BEQ     @@disc_up       ; |                                   
            CMPI    #@@pad + 12, R5 ; |
            BNEQ    @@fd_lp         ;/

            ;; ------------------------------------------------------------ ;;
            ;;  Check for ACTION release.                                   ;;
            ;; ------------------------------------------------------------ ;;
@@check_rls_act:   
            MOVR    R3,     R4
            ANDI    #@@fa,  R4      ; Get "ACTION" bit
            BEQ     @@check_rls_key
            
            MOVR    R0,     R4      ;\
            AND@    R2,     R4      ; |-- See which bits are released.
            ANDI    #$E0,   R4      ;/
            BEQ     @@done_up_evts

            ; do "action up" event.
            XORI    #@@fa,  R3      ; clear ACTION bit
            CALL    @@release
            DECLE   1

            B       @@done_up_evts

            ;; ------------------------------------------------------------ ;;
            ;;  Check for KEYPAD release.                                   ;;
            ;; ------------------------------------------------------------ ;;
@@check_rls_key:
            MOVR    R3,     R4
            ANDI    #@@fk,  R4      ; Get "KEYPAD" bit
            BEQ     @@done_up_evts

            ; do "keypad up" event.
            XORI    #@@fk,  R3      ; clear KEYPAD bit
            CALL    @@release
            DECLE   0

;           B       @@done_up_evts

            ;; ------------------------------------------------------------ ;;
            ;;  Now, see if anything new has been pressed.  If we think     ;;
            ;;  the keypad was already pressed, we won't send anything new; ;;
            ;;  rather we will just update the "last valid" and exit.  If   ;;
            ;;  we think DISC or ACTION was already pressed, we will only   ;;
            ;;  return DISC or ACTION events.  If nothing was already       ;;
            ;;  pressed, we look for an exact matches only for keypad,      ;;
            ;;  disc and action buttons.                                    ;;
            ;; ------------------------------------------------------------ ;;
@@done_up_evts:

            PULR    R0              ; Get saved input from stack
            MVI@    R2,     R4      ; Get previous last-valid value
            MVO     R4,     SH_TMP  ; remember previous last-valid value
            XORI    #$FF,   R0      ; Invert input so that set bit == pressed
            MVO@    R0,     R2      ; Save updated last-valid value
            BNEQ    @@nonzero       ; non-zero input

            DECR    R2              ;
            MVO@    R0,     R2      ; clear flags if input goes null
            B       @@leave

@@nonzero
            DECR    R2              ; point back to flags
            MVO@    R3,     R2      ; Store updated flags
            ANDI    #$7,    R3      ; Any flags set?
            BEQ     @@exact_only

            ANDI    #@@fk,  R3      ; Keypad bit set?
            BEQ     @@exact_discact ; No:  Decode DISC and ACTION only.

            ;; ------------------------------------------------------------ ;;
            ;;  Keypad bit was set.  In this case, we can't reliably decode ;;
            ;;  any new keypresses on the controller, so just leave.        ;;
            ;; ------------------------------------------------------------ ;;
@@leave:    PULR    PC


            ;; ------------------------------------------------------------ ;;
            ;;  Lookup tables for keypad, action button and disc.           ;;
            ;; ------------------------------------------------------------ ;;
            ;        E    C    9    8    7    6    5    4    3    2    1    0
@@pad:      BYTE    $28, $88, $24, $44, $84, $22, $42, $82, $21, $41, $81, $48
            ;       rgt  lft  top
@@act:      BYTE    $C0, $60, $A0
            ;         ESE    SE     SSE     S     SSW    SW     WSW     W
@@disc:     BYTE    %10010,%10011,%00011,%00001,%10001,%11001,%01001,%01000
            ;         WNW    NW     NNW     N     NNE    NE     ENE     E
            BYTE    %11000,%11100,%01100,%00100,%10100,%10110,%00110,%00010

            ;; ------------------------------------------------------------ ;;
            ;;  Decode exact matches for KEYPAD, DISC and ACTION.           ;;
            ;; ------------------------------------------------------------ ;;
@@exact_only:
            MOVR    R0,     R4      ;\
            ANDI    #$10,   R4      ; |-- If bit 4 is set, cannot be keypad
            BNEQ    @@exact_discact ;/

            MVII    #@@pad, R4      ; Point to keypad lookup table
            MVII    #11,    R5      ; 12 possible keypad indices, 0 .. 11
@@keypad_lp CMP@    R4,     R0      ; Match?
            BEQ     @@got_key       ; Yes:  Send keypress
            DECR    R5              ; No;   Try next one
            BPL     @@keypad_lp     

            ;; ------------------------------------------------------------ ;;
            ;;  Decode exact matches for DISC and ACTION only.              ;;
            ;; ------------------------------------------------------------ ;;
@@exact_discact:
            MVI@    R2,     R3      ; Get flags
            ANDI    #@@fa,  R3      ; see if ACTION already set
            BNEQ    @@exact_disc    ; Only let one ACTION at a time.

            MVII    #@@act, R4      ; Point to Action table
            MOVR    R0,     R3      ;\__ Look only at 'action' bits.
            ANDI    #$E0,   R3      ;/
            MVII    #3,     R5      ; Three actions
@@action_lp CMP@    R4,     R3      ; Match?
            BEQ     @@got_act       ; Yes:  Send keypress
            DECR    R5              ; No:   Try next one
            BNEQ    @@action_lp     ;

            ;; ------------------------------------------------------------ ;;
            ;;  Decode exact matches for DISC only.                         ;;
            ;; ------------------------------------------------------------ ;;
@@exact_disc:
            MVI@    R2,     R3      ; Get flags
            ANDI    #@@fd,  R3      ; see if DISC is already set
            BEQ     @@do_disc       ; No?  Do DISC unconditionally

            MVI     SH_TMP, R3      ; Get previous valid input
            XORR    R0,     R3      ; \
            ANDI    #$1F,   R3      ;  |-- Skip if DISC is unchanged.
            BEQ     @@done          ; /

@@do_disc:  MVII    #@@disc,R4      ; Point to Action table
            MOVR    R0,     R3      ;\__ Look only at 'disc' bits.
            ANDI    #$1F,   R3      ;/
            MVII    #$0F,   R5      ; 16 directions
@@disc_lp   CMP@    R4,     R3      ; Match?
            BEQ     @@got_disc      ; Yes:  Send keypress
            DECR    R5              ; No:   Try next one
            BPL     @@disc_lp       ;

            ;; ------------------------------------------------------------ ;;
            ;;  If we get here, we don't know *what* we have, so ignore it. ;;
            ;; ------------------------------------------------------------ ;;
@@done:     PULR    PC

            ;; ------------------------------------------------------------ ;;
            ;;  Handle keypad dispatch.                                     ;;
            ;; ------------------------------------------------------------ ;;
@@got_key:  
            MVI@    R2,     R0      ;\
            ANDI    #$FF-@@fk, R0   ; |
            XORI    #@@fk,  R0      ; |-- set "keypad bit" in flags
            MVO@    R0,     R2      ;/

            CLRR    R4              ; We want first entry in dispatch table
            B       @@dispatch

            ;; ------------------------------------------------------------ ;;
            ;;  Handle action-button dispatch.                              ;;
            ;; ------------------------------------------------------------ ;;
@@got_act:  
            MVII    #@@exact_disc,R4;\                                  
            PSHR    R4              ; |                                 
            PSHR    R0              ; |__ Save some registers so we can 
            PSHR    R1              ; |   resume w/ decoding DISC inputs
            PSHR    R2              ; |
            MVII    #@@disc_rtn, R0 ; |                                 
            PSHR    R0              ;/                                  

            MVI@    R2,     R0      ;\
            ANDI    #$FF-@@fa, R0   ; |
            XORI    #@@fa,  R0      ; |-- set "action bit" in flags
            MVO@    R0,     R2      ;/

            MVII    #1,     R4      ; We want first entry in dispatch table
            B       @@dispatch

            ;; ------------------------------------------------------------ ;;
            ;;  Handle DISC dispatch.                                       ;;
            ;; ------------------------------------------------------------ ;;
@@got_disc:  
            MVI@    R2,     R0      ;\
            ANDI    #$FF - @@fd, R0 ; |__ set "disc bit" in flags
            XORI    #@@fd,  R0      ; |
            MVO@    R0,     R2      ;/

            MVII    #2,     R4      ; We want first entry in dispatch table
;           B       @@dispatch
            

            ;; ------------------------------------------------------------ ;;
            ;;  Do the actual dispatch by queuing a task.                   ;;
            ;; ------------------------------------------------------------ ;;
@@dispatch: 
            SARC    R1              ;\    Set bits 8 and 9 according to  
            BC      @@d_not_left    ; |__ hand controller:               
            XORI    #$100,  R5      ; |   00 = INTV left  01 = INTV right
@@d_not_left                        ; |   10 = ECS left   11 = ECS right 
    IF DEFINED SCAN_ECS             ; | \
            ANDI    #$80,   R1      ; |  |   (the ECS-specific bits are
            BNEQ    @@d_not_ecs     ; |  |--  conditionally compiled.)
            XORI    #$200,  R5      ; |  |
@@d_not_ecs                         ; | /
    ENDI                            ;/
           
            MVI     SHDISP, R2      ; Look up dispatch table pointer
            TSTR    R2              ;
            BEQ     @@leave         ; Leave if null dispatch table ptr.

            ADDR    R4,     R2      ; Point to desired dispatch (KEY/ACT/DISC)
            MVI@    R2,     R0      ; Get dispatch pointer
            TSTR    R0
            BEQ     @@leave         ; Leave if no dispatch for this one.

            MOVR    R5,     R1      ; Index becomes 'argument' to function
            PULR    R5
            JD      QTASK           ; Return via QTASK.
            

            ;; ------------------------------------------------------------ ;;
            ;;  Modified dispatch:  Return 0x80 as our index to indicate    ;;
            ;;  that an input has been released.  We don't return what the  ;;
            ;;  released input was other than that it was DISC/ACTION/KEY.  ;;
            ;; ------------------------------------------------------------ ;;
@@release:  MVI@    R5,     R4      ; Get offset into dispatch table
            PSHR    R5              ; save return address
            PSHR    R0              ; save R0
            MVII    #$80,   R5      ; release event = 0x80 (here for intr.)
            PSHR    R1              ; save R1
            PSHR    R2              ; save R2
            
            MVII    #@@rls_rtn, R0  ;\__ after dispatch, go to rls_rtn
            PSHR    R0              ;/

            B       @@dispatch      ; dispatch the event

@@disc_rtn:
@@rls_rtn:  PULR    R2
            PULR    R1
            PULR    R0
            PULR    PC

            ENDP

;; ======================================================================== ;;
;;  End of File:  scanhand.asm                                              ;;
;; ======================================================================== ;;
