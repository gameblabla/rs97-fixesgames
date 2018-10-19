;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2010                                     *;
;* ======================================================================== *;

;;==========================================================================;;
;; Joe Zbiciak's Simple Synthesizer                                         ;;
;; Copyright 2010, Joe Zbiciak, intvnut AT gmail DOT com                    ;;
;;==========================================================================;;

;; ======================================================================== ;;
;;  Set up the cartridge                                                    ;;
;; ======================================================================== ;;

            INCLUDE "../macro/cart.mac"
            REQ_ECS     ; ECS unit required!
            ROMSETUP 16K, 2010, "Simple Synthesizer", MAIN, 32

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

            WORDARRAY   STICSH, 24              ; STIC shadow

            BYTEARRAY   COLSTK, 5               ; Color-stack shadow
BORDER      EQU         COLSTK + 4              ; Border-color shadow

            BYTEVAR     NTSC_PAL                ; 1 = NTSC, 2 = PAL
            WORDVAR     NOTE_TBL

            BYTEARRAY   ACTIVE, 6
            BYTEVAR     TPOSE 
            BYTEVAR     VOLUME
            BYTEVAR     CSTK_FGBG
            BYTEVAR     HDLY
            BYTEVAR     VDLY

;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
            INCLUDE "../library/gimini.asm"
            INCLUDE "../library/fillmem.asm"
            INCLUDE "../library/print.asm"
            INCLUDE "../library/prnum16.asm"
            INCLUDE "../library/memcpy.asm"
            INCLUDE "../library/memunpk.asm"
            INCLUDE "../library/wnk.asm"

            INCLUDE "../macro/util.mac"
            INCLUDE "../macro/stic.mac"
            INCLUDE "../macro/gfx.mac"
            INCLUDE "../macro/print.mac"

            INCLUDE "../ecs_kbd/scan_syn.asm"

            INCLUDE "../task/scanhand.asm"
            INCLUDE "../task/timer.asm"
            INCLUDE "../task/taskq.asm"

            INCLUDE "notes.asm"
            INCLUDE "synth_gfx.asm"

ISRRET      EQU     $1014

;; ======================================================================== ;;
;;  MAIN:   Where it all happens.                                           ;;
;; ======================================================================== ;;
MAIN        PROC
            ;; ------------------------------------------------------------ ;;
            ;;  Title screen and initialization                             ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #$25D,  R1      ;\
            MVII    #$102,  R4      ; |-- Clear all of memory
            CALL    FILLZERO        ;/

            CALL    INIT_SYN        ; Initialize the synthesizer keyboard
            
                                    ;     01234567890123456789
            PRINT_CSTK  1,  2, White,      ">>> SDK-1600 <<<"
            PRINT_CSTK  2,  6, White,          "Presents"
            PRINT_CSTK  6,  1, Yellow,    "Simple Synthesizer"
            PRINT_CSTK 10,  3, White,       "Copyright 2010"

            SETISR  INIT_ISR,   R0

            EIS
            CLRR    R0
@@ntscpal   CMP     NTSC_PAL,   R0  ; Wait for machine type detect to complete
            BEQ     @@ntscpal

            CALL    WAITKEY         ; Keep title screen up until key hit

            CALL    CLRSCR          ; Clear the title screen after key-tap

            ;; ------------------------------------------------------------ ;;
            ;;  Set up our synthesizer callbacks and note table.            ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #PLAY_NOTE, R0
            MVO     R0,         SYNDN
            MVII    #RLS_NOTE,  R0
            MVO     R0,         SYNUP

            MVI     NTSC_PAL,   R0
            DECR    R0
            BEQ     @@ntsc
            MVII    #NOTES.PAL - 1,     R0
            B       @@set_notes
@@ntsc:     MVII    #NOTES.NTSC - 1,    R0
@@set_notes MVO     R0,         NOTE_TBL
           
            MVII    #24,        R0
            MVO     R0,         TPOSE

            MVII    #10,        R0
            MVO     R0,         VOLUME

            ;; ------------------------------------------------------------ ;;
            ;;  Set up our hand controller dispatch.                        ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #HAND,      R0
            MVO     R0,         SHDISP

            ;; ------------------------------------------------------------ ;;
            ;;  Set up tasks for the main show.                             ;;
            ;; ------------------------------------------------------------ ;;
            CALL    STARTTASK
            DECLE   0
            DECLE   SCAN_SYN
            DECLE   2,2

            CALL    STARTTASK
            DECLE   1
            DECLE   UPD_KEY_MOBS
            DECLE   4, 4 

            MVII    #2,     R0
            MVO     R0,     TSKACT

            ;; ------------------------------------------------------------ ;;
            ;;  Draw the synthesizer keyboard graphic.                      ;;
            ;; ------------------------------------------------------------ ;;
            CALL    DRAW_SYNTH
            MVII    #1,     R0
            MVO     R0,     CSTK_FGBG
            MVII    #4,     R0
            MVO     R0,     HDLY

            ;; ------------------------------------------------------------ ;;
            ;;  Draw our initial status onscreen.                           ;;
            ;; ------------------------------------------------------------ ;;
            CALL    UPD_STAT

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


;; ======================================================================== ;;
;;  Synthesizer handlers.  This implements a very simple state machine.     ;;
;; ======================================================================== ;;

CHBASE      PROC
            DECLE   $1F0,$1F1,$1F2,$0F0,$0F1,$0F2
            ENDP


;; ======================================================================== ;;
;;  PLAY_NOTE   Add a note to the list of notes playing if a slot is        ;;
;;              currently available.                                        ;;
;; ======================================================================== ;;
PLAY_NOTE   PROC
            PSHR    R5

            MVII    #ACTIVE,R5
            MVII    #6,     R2
            CLRR    R0
@@slot:     CMP@    R5,     R0          ; \
            BEQ     @@open              ;  |_ First slot w/ 0 is open
            DECR    R2                  ;  |
            BNEQ    @@slot              ; /

            ; None open?  Drop the note.
            PULR    PC

@@open      INCR    R1                  ; offset note by 1
            DECR    R5
            MVO@    R1,     R5          ; allocate the slot

            ADDI    #CHBASE-ACTIVE-1,R5 ; Get PSG channel base for note
            MVI@    R5,         R5

            ADD     NOTE_TBL,   R1      
            ADD     TPOSE,      R1      ; Transpose if requested
            MVI@    R1,         R1      ; Get note period

            MVO@    R1,         R5      ; period LSBs
            ADDI    #3,         R5
            SWAP    R1
            MVO@    R1,         R5      ; period MSBs
            MVI     VOLUME,     R1      ; 
            ADDI    #6,         R5
            MVO@    R1,         R5      ; volume

            PULR    PC
            ENDP
            
;; ======================================================================== ;;
;;  RLS_NOTE    Remove a note from the list of currently playing notes.     ;;
;; ======================================================================== ;;
RLS_NOTE    PROC
            PSHR    R5

            MVII    #ACTIVE,R5
            MVII    #6,     R2
            INCR    R1                  ; Offset note by 1
@@slot:     CMP@    R5,     R1          ; \
            BEQ     @@found_it          ;  |_ Find first slot that matches
            DECR    R2                  ;  |
            BNEQ    @@slot              ; /

            ; None open?  We must have dropped that note
            PULR    PC
@@found_it:
            CLRR    R0
            DECR    R5
            MVO@    R0,     R5          ; Free up the slot

            
            ADDI    #CHBASE-ACTIVE-1,R5 ; Get PSG channel base for note
            MVI@    R5,     R5

            ADDI    #$B,    R5          ; Clear the volume first
            MVO@    R0,     R5

            INCR    R0
            SUBI    #$C,    R5          ; \
            MVO@    R0,     R5          ;  |_ Set the channel period to
            ADDI    #3,     R5          ;  |  $0101 when not in use
            MVO@    R0,     R5          ; /

            PULR    PC
            ENDP

           
;; ======================================================================== ;;
;;  HAND    Dispatch table                                                  ;;
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
            SWAP    R2,     2
            BMI     @@ignore
            ANDI    #$FF,   R2

            PSHR    R2
            PSHR    R5
            MVII    #UPD_STAT,  R0
            JSRD    R5,     QTASK
            PULR    R5
            PULR    R2

            ADDI    #@@tbl, R2
            MVI@    R2,     PC
@@tbl       DECLE   @@reset_tpose   ; [0]        Reset transpose

            DECLE   @@up_octave     ; [1] / [4]  up/down octave
            DECLE   @@up_halfstep   ; [2] / [5]  up/down half-step
            DECLE   @@up_volume     ; [3] / [6]  up/down volume

            DECLE   @@dn_octave     ; [1] / [4]  up/down octave
            DECLE   @@dn_halfstep   ; [2] / [5]  up/down half-step
            DECLE   @@dn_volume     ; [3] / [6]  up/down volume

            DECLE   @@ignore        ; [7], [8], [9]:  Ignore
            DECLE   @@ignore        ; [7], [8], [9]:  Ignore
            DECLE   @@ignore        ; [7], [8], [9]:  Ignore

            DECLE   @@reset_tpose   ; [C]        Reset transpose
            DECLE   @@reset_volume  ; [E]        Reset volume

@@reset_tpose:
            MVII    #24,    R0
            MVO     R0,     TPOSE
            B       REFRESH_TONES

@@up_halfstep:
            MVII    #1,     R0
            B       @@transpose

@@up_octave:
            MVII    #12,    R0
            B       @@transpose

@@dn_halfstep:
            MVII    #-1,    R0
            B       @@transpose

@@dn_octave:
            MVII    #-12,   R0

@@transpose
            ADD     TPOSE,  R0
            BMI     @@ignore        

            CMPI    #84-48, R0
            BGT     @@ignore

            MVO     R0,     TPOSE
            B       REFRESH_TONES

@@reset_volume:
            MVII    #10,    R0
            MVO     R0,     VOLUME
            B       @@ignore

@@up_volume:
            MVII    #1,     R0
            B       @@volume
@@dn_volume:
            MVII    #-1,    R0
@@volume:   ADD     VOLUME, R0
            BMI     @@ignore
            CMPI    #15,    R0
            BGT     @@ignore

            MVO     R0,     VOLUME

@@ignore:   JR      R5
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
;;  REFRESH_TONES   Go re-press all of the keys currently playing.          ;;
;; ======================================================================== ;;
REFRESH_TONES   PROC
            ; to-do

            JR  R5
            ENDP

;; ======================================================================== ;;
;;  UPD_STAT    Update the synthesizer status information onscreen          ;;
;; ======================================================================== ;;
UPD_STAT    PROC
            PSHR    R5

            PRINT_FGBG  7, 4, Green, Black,   "VOLUME:"
            PRINT_FGBG  9, 5, Green, Black,    "RANGE:"
            PRINT_FGBG 10, 9, Green, Black,        "TO"

            MVII    #disp_ptr(7, 12), R4
            MVI     VOLUME, R0
            MVII    #C_YEL, R3
            MVII    #2,     R2
            CALL    PRNUM16.b

            MVI     TPOSE,  R0
            MVII    #disp_ptr(9, 12), R4
            MVII    #C_YEL, R3
            CALL    PR_NOTE

            CLRR    R0
@@eol1      MVO@    R0,     R4
            CMPI    #disp_ptr(9, 20), R4
            BLT     @@eol1

            MVI     TPOSE,  R0
            ADDI    #48,    R0
            MVII    #disp_ptr(10,12), R4
            MVII    #C_YEL, R3
            CALL    PR_NOTE

            CLRR    R0
@@eol2      MVO@    R0,     R4
            CMPI    #disp_ptr(10,20), R4
            BLT     @@eol2

            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  PR_NOTE     Display the name of a note                                  ;;
;;              0 is C1, 84 is C8.                                          ;;
;; ======================================================================== ;;
PR_NOTE     PROC
@@sharp     EQU     $8000

            ;; ------------------------------------------------------------ ;;
            ;;  Find out what octave we're in and save it.                  ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #$80,   R2
@@octave:   ADDI    #8,     R2
            SUBI    #12,    R0
            BGE     @@octave

            XORR    R3,     R2      ; Merge color information into octave

            ADDI    #@@ntbl+12, R0  ; Index into note-name table

            MOVR    R0,     R1
            MVI@    R1,     R0
            MOVR    R0,     R1
            BMI     @@is_sharp

            XORR    R3,     R0
            MVO@    R0,     R4      ; Write note name
            MVO@    R2,     R4      ; Write octave

            JR      R5

@@is_sharp:
            XORR    R3,     R1
            MVO@    R1,     R4      ; Write note name for sharp

            MVII    #$800 + 11*8, R1
            XORR    R3,     R1      ; Write sharp symbol
            MVO@    R1,     R4      

            MVO@    R2,     R4      ; Write octave

            MVII    #15*8,  R1
            XORR    R3,     R1
            MVO@    R1,     R4      ; Write '/' symbol

            ADDI    #8,     R0      ; Increment letter of note 
            CMPI    #(ASC('H',0) - 32) * 8 + @@sharp, R0
            BNC     @@flat_ok
            SUBI    #56,    R0      ; Wrap from G back to A
@@flat_ok:
            XORR    R3,     R0
            MVO@    R0,     R4      ; Write note name for flat

            MVII    #$800 + 10*8, R1
            XORR    R3,     R1      ; Write flat symbol
            MVO@    R1,     R4      

            MVO@    R2,     R4      ; Write octave

            JR      R5


@@ntbl:     DECLE   (ASC('C',0) - 32) * 8
            DECLE   (ASC('C',0) - 32) * 8 + @@sharp
            DECLE   (ASC('D',0) - 32) * 8 
            DECLE   (ASC('D',0) - 32) * 8 + @@sharp
            DECLE   (ASC('E',0) - 32) * 8 
            DECLE   (ASC('F',0) - 32) * 8 
            DECLE   (ASC('F',0) - 32) * 8 + @@sharp
            DECLE   (ASC('G',0) - 32) * 8 
            DECLE   (ASC('G',0) - 32) * 8 + @@sharp
            DECLE   (ASC('A',0) - 32) * 8 
            DECLE   (ASC('A',0) - 32) * 8 + @@sharp
            DECLE   (ASC('B',0) - 32) * 8 

            ENDP

;; ======================================================================== ;;
;;  ISR -- Just keep the screen on, and copy the STIC shadow over.          ;;
;; ======================================================================== ;;
ISR    PROC
            ;; ------------------------------------------------------------ ;;
            ;;  Enable video and update STIC mob shadow.                    ;;
            ;; ------------------------------------------------------------ ;;
            MVO     R0,     STIC.viden  ; Enable display

            CLRR    R0
            CMP     CSTK_FGBG,  R0
            BEQ     @@cstk
            MVO     R0,     STIC.mode   ; ...foreground/background mode
            B       @@done_fgbg
@@cstk      MVI     STIC.mode,  R0      ; ...color stack mode
@@done_fgbg:

            MVII    #COLSTK,    R4
            MVII    #STIC.cs0,  R5

            REPEAT  5
            MVI@    R4,         R0      ; \_ Copy over color-stack shadow
            MVO@    R0,         R5      ; /
            ENDR

            MVII    #STICSH,    R4
            CLRR    R5

            REPEAT 24
            MVI@    R4,         R0
            MVO@    R0,         R5
            ENDR

            MVI     HDLY,       R0
            MVO     R0,         $30
            MVI     VDLY,       R0
            MVO     R0,         $31

            MVII    #ISRRET,    R5
            B       DOTIMER

            ENDP

;; ======================================================================== ;;
;;  INIT_ISR    Clear up things, detect NTSC vs. PAL, and then fall into    ;;
;;              normal ISR duties.                                          ;;
;; ======================================================================== ;;
INIT_ISR    PROC

            ;; ------------------------------------------------------------ ;;
            ;;  Initialize the STIC and the STIC shadow                     ;;
            ;; ------------------------------------------------------------ ;;
            CLRR    R0
            CLRR    R5
            MVII    #STICSH,R4
            MVII    #24,    R1
@@loop1:
            MVO@    R0,     R4          ; clear STIC shadow
            MVO@    R0,     R5          ; clear STIC registers
            DECR    R1
            BNEQ    @@loop1

            MVO     R0,     $30         ; \
            MVO     R0,     $31         ;  |- border/scroll regs
            MVO     R0,     $32         ; /

            ;; ------------------------------------------------------------ ;;
            ;;  Unpack the synthesizer graphics into GRAM.                  ;;
            ;; ------------------------------------------------------------ ;;
            CALL    MEMUNPK
            DECLE   $3800, SYNGFX, SYNGFX.len

            ;; ------------------------------------------------------------ ;;
            ;;  Reset PSG channel enables to something sane.                ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #$38,   R0
            MVO     R0,     $1F8
            MVO     R0,     $0F8

            ;; ------------------------------------------------------------ ;;
            ;;  Start PAL/NTSC autodetect.                                  ;;
            ;; ------------------------------------------------------------ ;;
            SETISR  NTSC_PAL1,  R0
            B       ISRRET
            ENDP


;; ======================================================================== ;;
;;  NTSC_PAL1   \_ Autodetect NTSC vs. PAL by counting cycles between       ;;
;;  NTSC_PAL2   /  interrupts.  NTSC is approx 14932; PAL is approx 20000.  ;;
;; ======================================================================== ;;
NTSC_PAL1   PROC
            SETISR  NTSC_PAL2,  R0
            EIS

            CLRR    R2
@@loop:     INCR    R2              ; \_ 15 cycles
            B       @@loop          ; /

            ENDP

NTSC_PAL2   PROC
            SUBI    #8,         SP
           
            ; NTSC should go around a little under 995 times;
            ; PAL should go around a little under 1333 times.
            ; Midpoint is 1164.  Above that, it's PAL, below is NTSC.
            CMPI    #1164,      R2
            BLT     @@ntsc

            MVII    #2,         R2
            B       @@common

@@ntsc      MVII    #1,         R2
@@common:   MVO     R2,         NTSC_PAL

            SETISR  ISR,        R0
            B       ISRRET
            ENDP
