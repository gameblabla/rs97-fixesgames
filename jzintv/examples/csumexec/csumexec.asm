;;==========================================================================;;
;; Checksum EXEC.  Attempts to identify a system by checksumming its ROMs.  ;;
;; Copyright 2000, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  TO BUILD IN BIN+CFG FORMAT:                                             *;
;*      as1600 -o csumexec.bin -l csumexec.lst csumexec.asm                 *;
;*                                                                          *;
;*  TO BUILD IN ROM FORMAT:                                                 *;
;*      as1600 -o csumexec.rom -l csumexec.lst csumexec.asm                 *;
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
;*                   Copyright (c) 2000, Joseph Zbiciak                     *;
;* ======================================================================== *;


        ROMW    16
        ORG     $5000

;------------------------------------------------------------------------------
; Include description of system.
;------------------------------------------------------------------------------
        INCLUDE "../library/gimini.asm"

;------------------------------------------------------------------------------
; EXEC-friendly ROM header.
;------------------------------------------------------------------------------
ROMHDR: BIDECLE ZERO            ; MOB picture base   (points to NULL list)
        BIDECLE ZERO            ; Process table      (points to NULL list)
        BIDECLE INIT            ; Program start address
        BIDECLE ZERO            ; Bkgnd picture base (points to NULL list)
        BIDECLE ONES            ; GRAM pictures      (points to NULL list)
        BIDECLE TITLE           ; Cartridge title/date
        DECLE   $03C0           ; No ECS title, run code after title,
                                ; ... no clicks
ZERO:   DECLE   $0000           ; Screen border control
        DECLE   $0000           ; 0 = color stack, 1 = f/b mode
        DECLE   C_BLK, C_BLK    ; Initial color stack 0 and 1: Black
        DECLE   C_BLK, C_BLK    ; Initial color stack 2 and 3: Black
        DECLE   C_BLK           ; Initial border color: Black
ONES:   DECLE   1
;------------------------------------------------------------------------------

SSAVE       EQU     $2F0
COUNT       EQU     $2F1
ROWDLY      EQU     $2F7
RANDLO      EQU     $2F8
RANDHI      EQU     $2F9
STACK       EQU     $300

SUMS        EQU     $330
TEMP        EQU     $331

;------------------------------------------------------------------------------

TITLEISR    PROC
            ENDP

TITLE:      BYTE    101, "ROM Checksummer", 0 
            

TITLEFIX:   PROC
            BEGIN

            MVII    #$100,    R4    ;\
            SDBD                    ; |__ remember old ISR address
            MVI@    R4,       R1    ; |
            MVO     R1,       TEMP  ;/

            MVII    #@@isr,   R0    ; \
            SUBI    #2,       R4    ;  |
            MVO@    R0,       R4    ;  |-- Make title screen cyan
            SWAP    R0              ;  |   via short ISR.
            MVO@    R0,       R4    ; /

            ; Patch the title string to say '=JRMZ=' instead of Mattel.
            CALL    PRINT.FLS       ; Write string (ptr in R5)
            DECLE   C_BLK           ; Black                     
            DECLE   $23C            ; First 'Mattel' in top-left
            BYTE    "Zbiciak "      ; Guess who?  :-)
            BYTE    "Electronics",0

            CALL    PRINT.FLS       ; Write string (ptr in R1)
            DECLE   C_BLK           ; Black                     
            DECLE   $2D0            ; Second 'Mattel' in lower-right
            BYTE    "2001 =JRMZ=",0 ; Guess who?  :-)

            ; Make all the rest of the text black to match
            MVII    #$243,  R3      ; Start after first JRMZ
            MVII    #7,     R0      ; Mask = 0xFFF8.  That's the 1s compl...
            COMR    R0              ; ...of 7.  (Saves an SDBD.)
            MVII    #146,   R2      ; We only need to touch 146 words.

@@titlelp:
            MVI@    R3,     R1      ; Read a word from the display
            ANDR    R0,     R1      ; Mask the foreground color
            MVO@    R1,     R3      ; Write the word back
            INCR    R3              ; Point to next word
            DECR    R2              ; Decrement our loop count
            BNEQ    @@titlelp       ; Iterate.

            ; Done.
            RETURN                  ; Return to caller

@@isr:
            ; Cyan screen
            MVII    #C_CYN, R0      ; Cyan title screen
            MVO     R0,     $28     ; Color stack 0 = Cyan
            MVO     R0,     $29     ; Color stack 1 = Cyan
            MVO     R0,     $2A     ; Color stack 2 = Cyan
            MVO     R0,     $2B     ; Color stack 3 = Cyan
            MVO     R0,     $2C     ; Border color = Cyan
 
            MVI     TEMP,   PC

            ENDP


;; ======================================================================== ;;
;;  INIT -- initialize the main program                                     ;;
;; ======================================================================== ;;
INIT:       PROC

            MVII    #$2F0,  R6      ; Reset our stack.

            ; Fill memory with zeros.
            MVII    #$25E,  R1
            MVII    #$102,  R4
            CALL    FILLZERO
    
            ; Set the ISR to the rest of our initialization
            MVII    #MAIN,  R0
            MVO     R0,     $100
            SWAP    R0
            MVO     R0,     $101    ; Set our interrupt service routine to 'ISR'
            EIS

@@spin:     DECR    PC
            ENDP
 
;; ======================================================================== ;;
;;  MAIN -- The main program                                                ;;
;; ======================================================================== ;;
MAIN:       PROC
            DIS
            MVII    #STACK, R6      ; Reset our stack.

            ; Set the ISR 
            MVII    #ISR ,  R0
            MVO     R0,     $100
            SWAP    R0
            MVO     R0,     $101    ; Set our interrupt service routine to 'ISR'
    

            CLRR    R0
            MVII    #$0028, R4      ; This is safe here because we're in
            MVO@    R0,     R4      ;    an interrupt context here.
            MVO@    R0,     R4
            MVO@    R0,     R4
            MVO@    R0,     R4
            MVO@    R0,     R4      ; Black screen.

            MVO     R4,     $20     ; Enable display

            EIS

@@test_loop:
            CALL    EXEC_CSUM
            B       @@test_loop

            ENDP

;; ======================================================================== ;;
;;  ISR                                                                     ;;
;; ======================================================================== ;;
ISR         PROC
            MVI     COUNT,  R0
            DECR    R0
            BMI     @@zero
            MVO     R0,     COUNT
@@zero: 

            MVI     $21,    R0      ; force color-stack mode
@@enable:   MVO     R5,     $20     ; enable display
@@null:     JR      R5
            ENDP

;; ======================================================================== ;;
;;  MSG_ERR    -- Print ERR! at R4                                          ;;
;;  MSG_OK     -- Print *OK* at R4                                          ;;
;;  MSG_DONE   -- Print *Done* at bottom of screen                          ;;
;;  MSG_MORE   -- Print More... at bottom of screen                         ;;
;; ======================================================================== ;;
MSG_ERR PROC
            PSHR    R5
            MVII    #C_RED, R1
            CALL    PRINT.S    
err:        STRING  "ERR!", 0
            PULR    PC
            ENDP

MSG_OK      PROC
            PSHR    R5
            MVII    #C_YEL, R1
            CALL    PRINT.S    
ok:         STRING  "*OK*", 0
            PULR    PC
            ENDP
        
MSG_DONE    PROC
            PSHR    R5
            CALL    PRINT.FLS  
            DECLE   C_BLU, $200 + 11*20 + 7
done:       STRING  "*Done*", 0
            PULR    PC
            ENDP

MSG_MORE    PROC
            PSHR    R5
            CALL    PRINT.FLS  
            DECLE   C_BLU, $200 + 11*20 + 6
more:       STRING  "More...", 0
            PULR    PC
            ENDP


;; ======================================================================== ;;
;;  WAITHAND                                                                ;;
;; ======================================================================== ;;
WAITHAND    PROC
            B       @@waithand
@@waitpress BNEQ    @@waitdone
@@waithand: MVII    #100,   R1      ; Debounce factor
@@waitloop: MVII    #$1FE,  R4
            SDBD
            MVI@    R4,     R0
            COMR    R0
            BC      @@waitpress
            BNEQ    @@waithand
@@waitdone: DECR    R1
            BNEQ    @@waitloop
            JR      R5
            ENDP


;; ======================================================================== ;;
;;  WAIT_TAP                                                                ;;
;; ======================================================================== ;;
WAIT_TAP    PROC
            PSHR    R5
            SETC
            CALL    WAITHAND
            PULR    R5
            CLRC
            B       WAITHAND
            ENDP


;; ======================================================================== ;;
;;  CRC32                                                                   ;;
;;  Performs a 32-bit CRC on a block of data.                               ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;  R4 -- Pointer to data.                                                  ;;
;;  R3 -- Length                                                            ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;  R0, R1 -- Contains checksum.                                            ;;
;; ======================================================================== ;;
CRC32       PROC
            PSHR    R5
            PSHR    R3
            PSHR    R2

            CLRR    R0
            DECR    R0
            MOVR    R0,     R1
            MVII    #$04C1, R2
            MVII    #$1DB7, R5

@@loop:     XOR@    R4,     R0

            SLLC    R0,     1
            RLC     R1,     1
            BNC     @@nc0
            XORR    R2,     R1
            XORR    R5,     R0
@@nc0

            SLLC    R0,     1
            RLC     R1,     1
            BNC     @@nc1
            XORR    R2,     R1
            XORR    R5,     R0
@@nc1

            SLLC    R0,     1
            RLC     R1,     1
            BNC     @@nc2
            XORR    R2,     R1
            XORR    R5,     R0
@@nc2

            SLLC    R0,     1
            RLC     R1,     1
            BNC     @@nc3
            XORR    R2,     R1
            XORR    R5,     R0
@@nc3

            SLLC    R0,     1
            RLC     R1,     1
            BNC     @@nc4
            XORR    R2,     R1
            XORR    R5,     R0
@@nc4

            SLLC    R0,     1
            RLC     R1,     1
            BNC     @@nc5
            XORR    R2,     R1
            XORR    R5,     R0
@@nc5

            SLLC    R0,     1
            RLC     R1,     1
            BNC     @@nc6
            XORR    R2,     R1
            XORR    R5,     R0
@@nc6

            SLLC    R0,     1
            RLC     R1,     1
            BNC     @@nc7
            XORR    R2,     R1
            XORR    R5,     R0
@@nc7

            SLLC    R0,     1
            RLC     R1,     1
            BNC     @@nc8
            XORR    R2,     R1
            XORR    R5,     R0
@@nc8

            SLLC    R0,     1
            RLC     R1,     1
            BNC     @@nc9
            XORR    R2,     R1
            XORR    R5,     R0
@@nc9

            SLLC    R0,     1
            RLC     R1,     1
            BNC     @@ncA
            XORR    R2,     R1
            XORR    R5,     R0
@@ncA

            SLLC    R0,     1
            RLC     R1,     1
            BNC     @@ncB
            XORR    R2,     R1
            XORR    R5,     R0
@@ncB

            SLLC    R0,     1
            RLC     R1,     1
            BNC     @@ncC
            XORR    R2,     R1
            XORR    R5,     R0
@@ncC

            SLLC    R0,     1
            RLC     R1,     1
            BNC     @@ncD
            XORR    R2,     R1
            XORR    R5,     R0
@@ncD

            SLLC    R0,     1
            RLC     R1,     1
            BNC     @@ncE
            XORR    R2,     R1
            XORR    R5,     R0
@@ncE

            SLLC    R0,     1
            RLC     R1,     1
            BNC     @@ncF
            XORR    R2,     R1
            XORR    R5,     R0
@@ncF

            DECR    R3
            BNEQ    @@loop

            PULR    R2
            PULR    R3
            PULR    PC
            ENDP


;; ======================================================================== ;;
;;  EXEC_CSUM                                                               ;;
;;  Takes a 32-bit CRC of the EXEC, GROM, and so on.                        ;;
;; ======================================================================== ;;
EXEC_CSUM   PROC
            PSHR    R5

            CALL    CLRSCR

            CALL    PRINT.FLS  
            DECLE   C_GRN, $200
                    ;01234567890123456789
            STRING  ">>  ROM Checksum  <<", 0

            CALL    PRINT.FLS  
            DECLE   C_WHT, $200 + 5*20
                    ;01234567890123456789
            STRING  "This will take a few"
            STRING  "seconds.  Press DISC"
            STRING  "     to start...    ", 0

            CALL    MSG_MORE
            CALL    WAIT_TAP

            CALL    CLRSCR

            CALL    PRINT.FLS  
            DECLE   C_GRN, $200
                    ;01234567890123456789
            STRING  ">>  ROM Checksum  <<", 0

            MVII    #@@irq, R0
            MVO     SP,     SSAVE
            MVO     R0,     $100
            SWAP    R0
            MVO     R0,     $101
            DECR    PC

@@irq:
            DIS
            MVII    #ISR,   R0
            MVO     R0,     $100
            SWAP    R0
            MVO     R0,     $101
            MVI     SSAVE,  SP

            ; Make sure default ECS ROMs are switched in:
            MVII    #$2FFF, R1
            MVII    #$2A51, R0
            MVO@    R0,     R1
            MVII    #$7FFF, R1
            MVII    #$7A50, R0
            MVO@    R0,     R1
            MVII    #$EFFF, R1
            MVII    #$EA51, R0
            MVO@    R0,     R1

            ; Checksum the EXEC from $0400 - $04FF.  On an Inty 1, this is
            ; all $FFFF.  On an Inty 2, this has ROM.
            MVII    #$0400, R4
            MVII    #$0100, R3
            CALL    CRC32
            PSHR    R0
            PSHR    R1

            ; Checksum the EXEC from $1000 - $1FFF.
            MVII    #$1000, R4
            MVII    #$1000, R3
            CALL    CRC32
            PSHR    R0
            PSHR    R1

            ; Checksum memory from $3000 - $37FF.
            MVII    #$3000, R4
            MVII    #$0800, R3
            CALL    CRC32
            PSHR    R0
            PSHR    R1

            ; Checksum memory from $2000 - $2FFF.  This will be all $FFFF
            ; unless we have an Inty III or an ECS.
            MVII    #$2000, R4
            MVII    #$1000, R3
            CALL    CRC32
            PSHR    R0
            PSHR    R1

            ; Checksum memory from $7000 - $7FFF.  This will be all $FFFF
            ; unless we have a Keyboard Component or an ECS.
            MVII    #$7000, R4
            MVII    #$1000, R3
            CALL    CRC32
            PSHR    R0
            PSHR    R1

            ; Checksum memory from $E000 - $EFFF.  This will be all $FFFF
            ; unless we have an ECS or something weird happens.
            MVII    #$E000, R4
            MVII    #$1000, R3
            CALL    CRC32
            PSHR    R0
            PSHR    R1
            
            CALL    PRINT.FLS  
            DECLE   C_WHT, $200 + 40
                    ;01234567890123456789
            STRING  "$0400-$04FF ........"
            STRING  "$1000-$1FFF ........"
            STRING  "$3000-$37FF ........"
            STRING  "$2000-$2FFF ........"
            STRING  "$7000-$7FFF ........"
            STRING  "$E000-$EFFF ........",0

            MVII    #$2F0,  R1
            SUBR    R4,     R1
            CALL    FILLZERO                ; clear to end of screen.

            MVII    #$200 + 140 + 12, R4
            MVII    #C_YEL, R1
            PULR    R0
            MVO     R0,     SUMS + 10
            CALL    HEX16
            PULR    R0
            MVO     R0,     SUMS + 11
            CALL    HEX16

            SUBI    #28,    R4
            PULR    R0
            MVO     R0,     SUMS + 8
            CALL    HEX16
            PULR    R0
            MVO     R0,     SUMS + 9
            CALL    HEX16

            SUBI    #28,    R4
            PULR    R0
            MVO     R0,     SUMS + 6
            CALL    HEX16
            PULR    R0
            MVO     R0,     SUMS + 7
            CALL    HEX16

            SUBI    #28,    R4
            PULR    R0
            MVO     R0,     SUMS + 4
            CALL    HEX16
            PULR    R0
            MVO     R0,     SUMS + 5
            CALL    HEX16

            SUBI    #28,    R4
            PULR    R0
            MVO     R0,     SUMS + 2
            CALL    HEX16
            PULR    R0
            MVO     R0,     SUMS + 3
            CALL    HEX16

            SUBI    #28,    R4
            PULR    R0
            MVO     R0,     SUMS + 0
            CALL    HEX16
            PULR    R0
            MVO     R0,     SUMS + 1
            CALL    HEX16

            CALL    FINDSYS

            EIS
            CALL    MSG_DONE
            CALL    WAIT_TAP

            RETURN
            ENDP

FINDSYS     PROC
            BEGIN

            MVII    #SYSTEM, R4
            B       @@start

@@o_loop:   PULR    R4
            ADDI    #33,    R4
@@start:    MVI@    R4,     R0
            TSTR    R0
            BEQ     @@unknown
            DECR    R4

            MVII    #SUMS,  R5
            MVII    #12,    R2
            PSHR    R4
@@i_loop:
            MVI@    R4,     R0
            CMP@    R5,     R0
            BNEQ    @@o_loop
            DECR    R2
            BNEQ    @@i_loop

@@print:    MOVR    R4,     R0
            MVII    #$200 + 20*9,   R4
            MVII    #C_TAN, R1
            CALL    PRINT.R    
            PULR    R4
            PULR    PC

@@unknown:  ADDI    #11,    R4
            PSHR    R4
            B       @@print

            ENDP


SYSTEM      PROC
            DECLE   $C44C, $879D, $647F, $395B
            DECLE   $561F, $A420, $8227, $7125
            DECLE   $8227, $7125, $8227, $7125
                    ;01234567890123456789
            STRING  ">> Inty 1, no ECS <<", 0

            DECLE   $C44C, $879D, $647F, $395B
            DECLE   $561F, $A420, $0B8A, $7ADD
            DECLE   $9FFB, $11AD, $6ADC, $1E6D
                    ;01234567890123456789
            STRING  ">> Inty 1, w/ ECS <<", 0

            DECLE   $E311, $1634, $FDAA, $FB68
            DECLE   $561F, $A420, $8227, $7125
            DECLE   $8227, $7125, $8227, $7125
                    ;01234567890123456789
            STRING  ">> Inty 2, no ECS <<", 0

            DECLE   $E311, $1634, $FDAA, $FB68
            DECLE   $561F, $A420, $0B8A, $7ADD
            DECLE   $9FFB, $11AD, $6ADC, $1E6D
                    ;01234567890123456789
            STRING  ">> Inty 2, w/ ECS <<", 0

            DECLE   $C44C, $879D, $64FB, $6997
            DECLE   $561F, $A420, $8227, $7125
            DECLE   $8227, $7125, $8227, $7125
                    ;01234567890123456789
            STRING  "> Sears SVA no ECS <", 0

            DECLE   $C44C, $879D, $64FB, $6997
            DECLE   $561F, $A420, $0B8A, $7ADD
            DECLE   $9FFB, $11AD, $6ADC, $1E6D
                    ;01234567890123456789
            STRING  "> Sears SVA w/ ECS <", 0

            DECLE   $0000, $0000, $0000, $0000
            DECLE   $0000, $0000, $0000, $0000
            DECLE   $0000, $0000, $0000, $0000
                    ;01234567890123456789
            STRING  ">> UNKNOWN SYSTEM <<", 0

            ENDP


;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
            INCLUDE "../library/hex16.asm"
            INCLUDE "../library/fillmem.asm"
            INCLUDE "../library/print.asm"
