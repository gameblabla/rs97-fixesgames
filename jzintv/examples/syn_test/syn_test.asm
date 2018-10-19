;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2010                                     *;
;* ======================================================================== *;

;;==========================================================================;;
;; Joe Zbiciak's ECS Synthesizer Keyboard Test                              ;;
;; Copyright 2010, Joe Zbiciak, intvnut AT gmail DOT com                    ;;
;;==========================================================================;;

;; ======================================================================== ;;
;;  Set up the cartridge                                                    ;;
;; ======================================================================== ;;

            INCLUDE "../macro/cart.mac"

            REQ_ECS
            ROMSETUP 16K, 2010, "Synth Keyboard Test", START, 32

            INCLUDE "../library/gimini.asm"
            INCLUDE "../library/fillmem.asm"
            INCLUDE "../library/print.asm"
            INCLUDE "../library/prnum16.asm"
            INCLUDE "../library/memcpy.asm"
            INCLUDE "../library/wnk.asm"

            INCLUDE "../macro/stic.mac"
            INCLUDE "../macro/print.mac"

            INCLUDE "../ecs_kbd/scan_syn.asm"



            BYTEARRAY   COLSTK, 5               ; Color-stack shadow
BORDER      EQU         COLSTK + 4              ; Border-color shadow


START       PROC
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
            PRINT_CSTK  5,  8, Yellow,           "ECS"           
            PRINT_CSTK  6,  2, Yellow,     "Synthesizer Test"
            PRINT_CSTK 10,  3, White,       "Copyright 2010"

            MVII    #ISR,       R0
            MVO     R0,         $100
            SWAP    R0
            MVO     R0,         $101

            EIS
            CALL    WAITKEY
            CALL    CLRSCR          ; Clear the title screen after key-tap

            ;; ------------------------------------------------------------ ;;
            ;;  Clear the screen and print some usage info.                 ;;
            ;; ------------------------------------------------------------ ;;
            CALL    CLRSCR

                                    ;     01234567890123456789
            PRINT_CSTK  0,  0, Yellow,   "Press keys on the"
            PRINT_CSTK  1,  0, Yellow,   "synthesizer keyboard"
            PRINT_CSTK  2,  0, Yellow,   "to see the key down"
            PRINT_CSTK  3,  0, Yellow,   "and key up events."
           
            ;; ------------------------------------------------------------ ;;
            ;;  Set up our synthesizer callbacks.                           ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #PR_SYNDN,  R0
            MVO     R0,         SYNDN
            MVII    #PR_SYNUP,  R0
            MVO     R0,         SYNUP
           
            ;; ------------------------------------------------------------ ;;
            ;;  Loop forever scanning the synthesizer.                      ;;
            ;; ------------------------------------------------------------ ;;
@@loop:     CALL    SCAN_SYN
            B       @@loop
           
            ENDP
           
           
           
;; ======================================================================== ;;
;;  SCROLL_UP:  Simple screen scrolling macro                               ;;
;; ======================================================================== ;;
MACRO       SCROLL_UP
            CALL    MEMCPY
            DECLE   $200 + 20 * 4
            DECLE   $200 + 20 * 5
            DECLE   $F0 - 20 * 5
ENDM



;; ======================================================================== ;;
;;  PR_SYNDN    PRint when a SYNthesizer key has been pressed ("down")      ;;
;; ======================================================================== ;;
PR_SYNDN    PROC
            PSHR    R5
            PSHR    R1          ; key value

            SCROLL_UP

            CALL    PRINT.FLS
            DECLE   C_WHT, $200 + 11*20 + 0, "Pressed:  ", 0

            PULR    R0
            MVII    #C_TAN, R3
            CALL    PRNUM16.l

            CLRR    R0
            MVO@    R0,     R4
            MVO@    R0,     R4

            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  PR_SYNUP    PRint when a SYNthesizer key has been released ("up")       ;;
;; ======================================================================== ;;
PR_SYNUP    PROC
            PSHR    R5
            PSHR    R1          ; key value

            SCROLL_UP

            CALL    PRINT.FLS
            DECLE   C_WHT, $200 + 11*20 + 0, "Released: ", 0

            PULR    R0
            MVII    #C_TAN, R3
            CALL    PRNUM16.l

            CLRR    R0
            MVO@    R0,     R4
            MVO@    R0,     R4

            PULR    PC
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

            B       $1014

            ENDP
