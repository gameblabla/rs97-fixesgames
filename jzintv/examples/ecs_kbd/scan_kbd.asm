;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2010                                     *;
;* ======================================================================== *;


;; ======================================================================== ;;
;;  ECS Keyboard Scanner                                                    ;;
;;                                                                          ;;
;;  Unlike the hand-controller scanner, this code currently does not        ;;
;;  generate events for the event queue.  It can easily be wrapped with     ;;
;;  code to do that, though.                                                ;;
;;                                                                          ;;
;;  There are two scanners:  The keyboard scanner which returns ASCII       ;;
;;  codes, and the synth scanner which maintains a bitmap of pressed        ;;
;;  notes and a list of changes between calls.  This file only contains     ;;
;;  the keyboard scanner.                                                   ;;
;;                                                                          ;;
;;  The keyboard scanner does a "transposed scan" as compared to the ECS's  ;;
;;  default scanner.  This allows it to correctly pick up the SHIFT key.    ;;
;;  It also does an extra scanning step to try to pick out the CTL key.     ;;
;;                                                                          ;;
;;  This keyboard scanner makes no attempt to gracefully handle multiple    ;;
;;  keys pressed at one time.  Rather, it scans the keyboard, decodes,      ;;
;;  and returns the new key value if it's different than the last key it    ;;
;;  returned.  Otherwise, it returns KEY.NONE.                              ;;
;; ======================================================================== ;;

            BYTEVAR ECS_KEY_LAST

KEY.LEFT    EQU     $1C     ; \   Can't be generated otherwise, so perfect
KEY.RIGHT   EQU     $1D     ;  |_ candidates.  Could alternately send 8 for
KEY.UP      EQU     $1E     ;  |  left... not sure...
KEY.DOWN    EQU     $1F     ; /   
KEY.ENTER   EQU     $A      ; Newline
KEY.ESC     EQU     27
KEY.NONE    EQU     $FF

KBD_DECODE  PROC
@@no_mods   DECLE   KEY.NONE, "ljgda"                       ; col 7
            DECLE   KEY.ENTER, "oute", KEY.NONE             ; col 6
            DECLE   "08642", KEY.RIGHT                      ; col 5
            DECLE   KEY.ESC, "97531"                        ; col 4
            DECLE   "piyrwq"                                ; col 3
            DECLE   ";khfs", KEY.UP                         ; col 2
            DECLE   ".mbcz", KEY.DOWN                       ; col 1
            DECLE   KEY.LEFT, ",nvx "                       ; col 0
                                                         
@@shifted   DECLE   KEY.NONE, "LJGDA"                       ; col 7
            DECLE   KEY.ENTER, "OUTE", KEY.NONE             ; col 6
            DECLE   ")*-$\"/"                               ; col 5
            DECLE   KEY.ESC, "(/+#="                        ; col 4
            DECLE   "PIYRWQ"                                ; col 3
            DECLE   ":KHFS^"                                ; col 2
            DECLE   ">MBCZ?"                                ; col 1
            DECLE   "%<NVX "                                ; col 0

@@control   DECLE   KEY.NONE, $C, $A, $7, $4, $1            ; col 7
            DECLE   KEY.ENTER, $F, $15, $14, $5, KEY.NONE   ; col 6
            DECLE   "}~_!'", KEY.RIGHT                      ; col 5
            DECLE   KEY.ESC, "{&@`~"                        ; col 4
            DECLE   $10, $9, $19, $12, $17, $11             ; col 3
            DECLE   "|", $B, $8, $6, $13, KEY.UP            ; col 2
            DECLE   "]", $D, $2, $3, $1A, KEY.DOWN          ; col 1
            DECLE   KEY.LEFT, "[", $0E, $16, $18, $20       ; col 0
            ENDP

SCAN_KBD    PROC

            ;; ------------------------------------------------------------ ;;
            ;;  Try to find CTRL and SHIFT first.                           ;;
            ;;  Shift takes priority over control.                          ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #KBD_DECODE.no_mods, R3 ; neither shift nor ctrl

            ; maybe DIS here
            MVI     $F8,        R0
            ANDI    #$3F,       R0
            XORI    #$80,       R0          ; transpose scan mode
            MVO     R0,         $F8
            ; maybe EIS here
                               
            MVII    #$7F,       R1          ; \_ drive column 7 to 0
            MVO     R1,         $FF         ; /
            MVI     $FE,        R2          ; \
            ANDI    #$40,       R2          ;  > look for a 0 in row 6
            BEQ     @@have_shift            ; /

            MVII    #$BF,       R1          ; \_ drive column 6 to 0
            MVO     R1,         $FF         ; /
            MVI     $FE,        R2          ; \
            ANDI    #$20,       R2          ;  > look for a 0 in row 5
            BNEQ    @@done_shift_ctrl       ; /

            MVII    #KBD_DECODE.control, R3
            B       @@done_shift_ctrl

@@have_shift:
            MVII    #KBD_DECODE.shifted, R3

@@done_shift_ctrl:

            ;; ------------------------------------------------------------ ;;
            ;;  Start at col 7 and work our way to col 0.                   ;;
            ;; ------------------------------------------------------------ ;;
            CLRR    R2              ; col pointer
            MVII    #$FF7F, R1

@@col:      MVO     R1,     $FF
            MVI     $FE,    R0
            XORI    #$FF,   R0
            BNEQ    @@maybe_key

@@cont_col: ADDI    #6,     R2
            SLR     R1
            CMPI    #$FF,   R1
            BNEQ    @@col

            MVII    #KEY.NONE,  R0
            B       @@none

            ;; ------------------------------------------------------------ ;;
            ;;  Looks like a key is pressed.  Let's decode it.              ;;
            ;; ------------------------------------------------------------ ;;
@@maybe_key:
            MOVR    R2,     R4
            SARC    R0,     2
            BC      @@got_key       ; row 0
            BOV     @@got_key1      ; row 1
            ADDI    #2,     R4 
            SARC    R0,     2               
            BC      @@got_key       ; row 2
            BOV     @@got_key1      ; row 3
            ADDI    #2,     R4 
            SARC    R0,     2               
            BC      @@got_key       ; row 4
            BNOV    @@cont_col      ; row 5
@@got_key1: INCR    R4
@@got_key:
            
            ADDR    R3,     R4      ; add modifier offset
            MVI@    R4,     R0

            CMPI    #KEY.NONE, R0   ; if invalid, keep scanning
            BEQ     @@cont_col

            CMP     ECS_KEY_LAST, R0
@@none:     MVO     R0,         ECS_KEY_LAST
            BNEQ    @@new
            MVII    #KEY.NONE,  R0

            
@@new:      ; maybe DIS here
            MVI     $F8,        R1  ; \
            ANDI    #$3F,       R1  ;  > set both I/O ports to "input"
            MVO     R1,         $F8 ; /
            ; maybe EIS here
            JR      R5
            ENDP
            
