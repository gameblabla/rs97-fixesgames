;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2010                                     *;
;* ======================================================================== *;


;; ======================================================================== ;;
;;  Scan the Synthesizer Keyboard                                           ;;
;;                                                                          ;;
;;  This code scans the synthesizer keyboard, and calls either SYNDN or     ;;
;;  SYNUP when it detects a new key pressed or released.  In the worst      ;;
;;  case, it could call SYNDN or SYNUP 49 times if all the keys get mashed  ;;
;;  simultaneously.                                                         ;;
;; ======================================================================== ;;

            WORDVAR     SYNDN
            WORDVAR     SYNUP
            BYTEARRAY   SYNSTAT,    7


;; ======================================================================== ;;
;;  INIT_SYN    Initialize the data structures used by SCAN_SYN             ;;
;; ======================================================================== ;;
INIT_SYN    PROC
            CLRR    R0
            MVII    #SYNSTAT,   R4
            MVO@    R0,         R4
            MVO@    R0,         R4
            MVO@    R0,         R4
            MVO@    R0,         R4
            NOP
            MVO@    R0,         R4
            MVO@    R0,         R4
            MVO@    R0,         R4
            MVII    #@@jrr5,    R0
            MVO     R0,         SYNDN
            MVO     R0,         SYNUP
@@jrr5      JR      R5
            ENDP

;; ======================================================================== ;;
;;  SCAN_SYN    Actually scan the synthesizer, calling the key down and     ;;
;;              key up callbacks (SYNDN/SYNUP) as it detects changes.       ;;
;; ======================================================================== ;;
SCAN_SYN    PROC
            PSHR    R5
            
            CLRR    R0                  ; row offset; initially 0

            ; maybe DIS
            MVI     $F8,        R1      ; \
            ANDI    #$3F,       R1      ;  |_ turn IO ports on for scan.
            XORI    #$40,       R1      ;  |
            MVO     R1,         $F8     ; /
            ; maybe EIS
            
            MVII    #$FE,       R1      ; \_ initial row strobe
            MVO     R1,         $FE     ; /

            CMP     $FE,        R1      ; \_ short circuit if ECS unavail
            BNEQ    @@leave             ; /

            MVII    #SYNSTAT,   R3

@@row_loop:
            MVI     $FF,        R1      ; Get new status of row
            XORI    #$FF,       R1      ; 1 means pressed; 0 means released


            MVI@    R3,         R2      ; Get previous status of row
            MVO@    R1,         R3      ; Store new status
            XORR    R1,         R2      ; What changed?
            BEQ     @@next_row          ; Nothing?  Go to the next row

            PSHR    R3
@@pr_deltas
            ANDR    R2,         R1      ; R1 contains "just pressed" vector
            XORR    R1,         R2      ; R2 contains "just released" vector

            BEQ     @@press_loop_init

            ; Do "release" before "press", to simplify key-to-voice allocation

            PSHR    R1                  ; save "just pressed"

@@release_loop:

            ADDI    #SYNDLTA,   R2      ; \_ Find right-most set bit and 
            MVI@    R2,         R2      ; /  remove it, returning its bit #

            MOVR    R2,         R1      ; \
            ANDI    #$FF,       R2      ;  |  Unpack:  MSB is bit #; LSB is
            XORR    R2,         R1      ;  |- bitfield minus rightmost 1 bit
            SWAP    R1                  ;  |  Bit # in R1, bitfield in R2
            ADDR    R0,         R1      ; /

            PSHR    R0                  ; \_ Save regs...
            PSHR    R2                  ; /

            MVII    #@@rls_rtn, R5      ; \_ Call syn-up
            MVI     SYNUP,      PC      ; /

@@rls_rtn:  PULR    R2
            PULR    R0

            TSTR    R2
            BNEQ    @@release_loop

            PULR    R1                  ; Restore "just pressed"

@@press_loop_init:
            MOVR    R1,         R2
            BEQ     @@done_pr

@@press_loop:
            ADDI    #SYNDLTA,   R2      ; \_ Find right most set bit and 
            MVI@    R2,         R2      ; /  remove it, returning its bit #

            MOVR    R2,         R1      ; \
            ANDI    #$FF,       R2      ;  |  Unpack:  MSB is bit #; LSB is
            XORR    R2,         R1      ;  |- bitfield minus rightmost 1 bit
            SWAP    R1                  ;  |  Bit # in R1, bitfield in R2
            ADDR    R0,         R1      ; /

            PSHR    R0                  ; \_ Save regs...
            PSHR    R2                  ; /

            MVII    #@@prs_rtn, R5      ; \_ Call syn-down
            MVI     SYNDN,      PC      ; /

@@prs_rtn:  PULR    R2
            PULR    R0

            TSTR    R2
            BNEQ    @@press_loop

@@done_pr:
            PULR    R3
@@next_row:
            INCR    R3                  ; Move to next state buffer entry
            ADDI    #8,         R0      ; Add 8 to row index

            MVI     $FE,        R1      ; \
            SETC                        ;  |_ Slide scan row over by 1.
            RLC     R1                  ;  |
            MVO     R1,         $FE     ; /

            CMPI    #$17F,      R1      ; Scanned 7 rows?
            BNEQ    @@row_loop          ; No:  Keep going

            ; maybe DIS
            MVI     $F8,        R1      ; \
            ANDI    #$3F,       R1      ;  |- Turn off output drivers.
            MVO     R1,         $F8     ; /
            ; maybe EIS

@@leave     PULR    PC
            ENDP




;; ======================================================================== ;;
;;  SYNDLTA     Decode table to speed finding the right-most set bit, as    ;;
;;              well as removing that bit.                                  ;;
;;                                                                          ;;
;;              MSB is bit #, LSB is new bitfield minus LSB.                ;;
;; ======================================================================== ;;

SYNDLTA     PROC
            LISTING "code"

@@b         QSET    $00

            REPEAT  256 / 4

            ; 00 case:  Go find actual right-most bit
@@nb        QSET    @@b AND (@@b - 1)
@@rmb       QSET    @@b XOR @@nb
@@num       QSET    0
            IF      @@rmb AND $F0
@@num       QSET    4
            ENDI
            IF      @@rmb AND $CC
@@num       QSET    2 + @@num
            ENDI
            IF      @@rmb AND $AA
@@num       QSET    1 + @@num
            ENDI
            DECLE   (@@num SHL 8) + @@nb

            ; 01 case: right most bit is bit 0
            DECLE   (0 SHL 8) + @@b

            ; 10 case: right most bit is bit 1
            DECLE   (1 SHL 8) + @@b

            ; 11 case: right most bit is bit 0, and bit 1 gets set
            DECLE   (0 SHL 8) + @@b + 2

@@b         QSET    @@b + 4
            ENDR

            LISTING "prev"
            ENDP

