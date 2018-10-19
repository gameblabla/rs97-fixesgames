;;==========================================================================;;
;; Joe Zbiciak's ECScable Monitor ROM.                                      ;;
;; Copyright 2001, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

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
;* ------------------------------------------------------------------------ *;
;*                   Copyright (c) 2001, Joseph Zbiciak                     *;
;* ======================================================================== *;

            ROMW 16

P1CTL       EQU     $F8
P1OUT       EQU     $FE
P1IN        EQU     $FF
TIMEOUT     EQU     $FF

;; ======================================================================== ;;
;;  Include system description 
;; ======================================================================== ;;
            INCLUDE "../library/gimini.asm"

;; ======================================================================== ;;
;;  Here are the variables local to the ECS monitor:                        ;;
;; ======================================================================== ;;
            ORG $0500, $0500, "=RW"
GAME        DECLE   $0              ; Magic flag which says "Go To Cart"
ECSDIS      DECLE   $0              ; Magic flag which says "Disable ECS"
ADR         RMB     1               ; Address for command
LEN         RMB     1               ; Length for command
CMD         RMB     1               ; Actual command #
VFLAG       RMB     1               ; Actual command #
R0SAV       RMB     1
R1SAV       RMB     1
R2SAV       RMB     1
R3SAV       RMB     1
R4SAV       RMB     1
CTSAV       RMB     1
BUSY        RMB     1
IDLECNT     RMB     1
INITWAIT    DECLE   $0
BOOT        DECLE   $1
;           RMB     $800 - $

;; ======================================================================== ;;
;;  Open a bankswitched window at $0E00 so we can program the Intellicart.  ;;
;; ======================================================================== ;;
            ORG $0E00, $0E00, "=RWB"
WINDOW      RMB $0200               ; 512-word window on memory
WBANK       EQU WINDOW SHR 8

;; ======================================================================== ;;
;;  We need a single DECLE at $CFFF so the ECS will jump to us.             ;;
;; ======================================================================== ;;
            ORG $CFFF, $CFFF, "=R"
            DECLE $1C1

;; ======================================================================== ;;
;;  Set up a "superset" memory map that will handle most cartridges we'll   ;;
;;  want to download via the ECScable interface.  If cartridges want some   ;;
;;  RAM or more ROM, we'll have to expand this.  But for now, this should   ;;
;;  do for most purposes.                                                   ;;
;; ======================================================================== ;;

            ORG $5000, $5000, "=R"
            RMB $2000               ; Mark $5000 - $6FFF readable

            ORG $8800, $8800, "=RW" 
            RMB $0800               ; Mark $8800 - $8FFF read/write 

            ORG $9000, $9000, "=R"
            RMB $3000               ; Mark $9000 - $BFFF readable

            ORG $D000, $D000, "=R"
            RMB $1000               ; Mark $D000 - $DFFF readable

            ORG $F000, $F000, "=R"
            RMB $1000               ; Mark $F000 - $FFFF readable


;; ======================================================================== ;;
;;  Stub out $C040 so we don't totally kill ECS BASIC performance:          ;;
;; ======================================================================== ;;
            ORG $C040, $C040, "=R"
_C040       JR  R5
            JR  R5
            JR  R5

;; ======================================================================== ;;
;;  START -- This is the main monitor code right here.  This gets called    ;;
;;           before the ECS even finishes the ECS-specific initialization.  ;;
;; ======================================================================== ;;
START:      PROC

            PSHR    R0                  ; Save R0 (in case we skip monitor)

            CLRR    R0
            MVO     R0,     BUSY        ; Clear the "BUSY" flag.

            MVI     GAME,   R0          ; Check our "reset to GAME flag"
            CMPI    #$AA55, R0          ; If != AA55, go to monitor.
            BNEQ    @@nret

            MVI     $1FE,   R0          ; If the user's holding 1+9 on the
            AND     $1FF,   R0          ; controller, go to monitor
            CMPI    #$5A,   R0
            BEQ     @@nret

            MVI     ECSDIS, R0          ; If the ECS is supposed to be
            TSTR    R0                  ; enabled, return to ECS startup.
            BEQ     @@ret               ;

            MVII    #RESET_GAME, R5     ; Return via GAME RESET routine
            B       ECS_DISABLE         ; Disable the ECS and get outta here.

@@ret       PULR    R0                  ; Return to startup.  Restore R0
            JR      R5                  ; on our way out.

            
@@nret:     PULR    R0
            DIS

            CLRR    R0
            MVO     R0,     GAME        ; Disable "RESET to GAME" flag.
            MVO     R0,     BUSY        ; Clear our "BUSY" flag.

            MVII    #WBANK, R1          ; Reset our bankswitch on $0E00.
            MVII    #WBANK, R2
            CALL    IC_SETFBANK

            CALL    CLRSCR              ; Clear the screen for the monitor

            MVII    #201*8 + 2, R0      ; \
            MVII    #20,    R1          ;  |__ Red stripe
            MVII    #$200 + 5*20, R4    ;  |
            CALL    FILLMEM             ; /
            MVII    #209*8, R0          ; \
            MVII    #20,    R1          ;  |
            MVII    #$200 + 2*20, R4    ;  |
            CALL    FILLMEM             ;  |__ Black bar
            MVII    #208*8, R0          ;  |
            MVII    #20,    R1          ;  |
            MVII    #$200 + 4*20, R4    ;  |
            CALL    FILLMEM             ; /


            CALL    PRINT.FLS                   ; \
            DECLE   X_WHT, $200 + 3*20          ;  |
                    ;01234567890123456789       ;  |
            DECLE   $0420                       ;  |
            STRING   "ECScable", 0              ;  |___ Title screen.
                                                ;  |
            CALL    PRINT.FLS                   ;  |
            DECLE   X_RED, $200 + 3*20 + 12     ;  |
                    ;01234567890123456789       ;  |
            STRING              "Monitor ",0    ; /

            MVII    #$2000, R0
            XOR@    R4,     R0
            DECR    R4
            MVO@    R0,     R4

            MVI     BOOT,   R0
            TSTR    R0
            BEQ     @@nboot

            CALL    PRINT.FLS   
            DECLE   X_BLK, $200 + 7*20 + 8      ;
            STRING          "v1.2", 0           ;
            CALL    PRINT.FLS   
            DECLE   X_BLK, $200 + 9*20 + 1      ;
                    ;01234567890123456789       ;
            STRING  "Copyright 2002 J.Z.",0     ;
@@nboot:
            CALL    INITISR             ; This enables interrupts, too

@@loop:
            MVII    #$49*8, R0          ; put a small 'i' in corner
            MVO     R0,     $201        ; to signify we're idle.

            CALL    EC_SPINIDLE         ; Spin while being idle.

            CLRR    R0                  ; Delete the 'i' when we're not idle.
            MVO     R0,     $201        ; 

            CALL    EC_GETCMD           ; Get the command from the PC.
            BC      @@fail              ; If there's an error, fail out.

            CMPI    #$17,   R1          ; Make sure command # is in range.
            BGE     @@cmderr            ; Signal an error if it isn't.

            CMPI    #$08,   R1
            BGT     @@xfer_cmd

            ADDI    #CMDTBL,R1
            MVII    #@@loop,R5          ; Return to top-of-loop from command.
            MVI@    R1,     PC          ; Jump to command.

@@xfer_cmd: MVO     R1,     CMD         ; Save the command number

            CALL    EC_GETHDR           ; Read the command header
            BC      @@fail              ; Fail on error.

            MVO     R1,     ADR         ; Save address and length
            MVO     R0,     LEN

            MVII    #$200+8*20+5, R4    ; \
            MVI     ADR,    R0          ;  |
            MVII    #X_BLU, R1          ;  |
            CALL    HEX16               ;  |
                                        ;  |__ Display range of addresses
            MVI     ADR,    R0          ;  |   on the screen during command.
            ADD     LEN,    R0          ;  |
            DECR    R0                  ;  |
            ADDI    #2,     R4          ;  |
            CALL    HEX16               ; /

            MVI     LEN,    R0          ; \
            MVI     ADR,    R1          ;  |-- Restore args to command
            MVI     CMD,    R2          ; /
            SUBI    #$10,   R2          ; 
            BPL     @@xfer_icart        ; Dispatch to approp. XFER routine.

            ADDI    #$8,    R2          ; \
            CALL    EC_XFER_INTY        ;  |__ Commands 8..F transfer to
            BC      @@fail              ;  |   Intellivision address space.
            B       @@loop              ; /

@@xfer_icart
            CALL    EC_XFER_ICART       ; \
            BC      @@fail              ;  |-- Commands 10..17 transfer to
            B       @@loop              ; /    Intellicart address space.


@@cmderr:   PSHR    R1                      ; Remember command number
            CALL    PRINT.FLS               ; \
            DECLE   X_RED,  $200 + 8*20     ;  |
                    ;01234567890123456789   ;  |
            STRING  " Bad Command: $", 0    ;  |-- Complain about invalid
            PULR    R0                      ;  |   command numbers.
            MVII    #X_RED, R1              ;  |
            CALL    HEX16                   ; /
            B       @@loop

@@fail:     CALL    PRINT.FLS                   ; \
            DECLE   X_RED,  $200 + 8*20         ;  |__ Report errors 
                    ;01234567890123456789       ;  |   (fairly vague.)
            STRING  "   Transfer Error   ", 0   ; /

            B       @@loop
            ENDP

;; ======================================================================== ;;
;;  Command jump table for various low-numbered commands.                   ;;
;; ======================================================================== ;;
CMDTBL      PROC
            DECLE   NOP             ; 0x00
            DECLE   RESET_GAME      ; 0x01
            DECLE   RESET_MONITOR   ; 0x02
            DECLE   VIDEO_OFF       ; 0x03
            DECLE   VIDEO_ON        ; 0x04
            DECLE   ECS_DISABLE     ; 0x05
            DECLE   ECS_ENABLE      ; 0x06
            DECLE   ECS_SETPAGE     ; 0x07
            ENDP

;; ======================================================================== ;;
;;  NOP  -- A No-Operation command.                                         ;;
;; ======================================================================== ;;
NOP         PROC
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  RESET_GAME -- Reset and go to GAME, not MONITOR.                        ;;
;; ======================================================================== ;;
RESET_GAME  PROC
            DIS

            MVII    #WBANK, R1      ; Reset our bankswitch on $0E00.
            MVII    #WBANK, R2
            CALL    IC_SETFBANK

            MVII    #$AA55, R0      ; Set GAME flag to go to the GAME
            MVO     R0,     GAME    ;
            MVO     R0,     BOOT    ; Set BOOT flag to show boot message ltr.
            CLRR    R0              ;
            MVO     R0,     BUSY    ; Clear global busy flag
            MVII    #$1000, R0      ; \
            MVO     R0,     $100    ;  |__ Set interrupt vector to the RESET
            SWAP    R0              ;  |   vector
            MVO     R0,     $101    ; /
            EIS
            DECR    PC              ; Spin until interrupt.
            ENDP

;; ======================================================================== ;;
;;  RESET_MONITOR -- Reset and go to MONITOR, not GAME.                     ;;
;; ======================================================================== ;;
RESET_MONITOR   PROC
            DIS
            CLRR    R0              ; Clear GAME flag to go to the MONITOR
            MVO     R0,     GAME    ;
            MVO     R0,     BUSY    ; Clear global busy flag
            MVO     R0,     BOOT    ; Clear "boot message" flag
            MVII    #$1000, R0      ; \
            MVO     R0,     $100    ;  |__ Set interrupt vector to the RESET
            SWAP    R0              ;  |   vector
            MVO     R0,     $101    ; /
            EIS
            DECR    PC              ; Spin until interrupt.
            ENDP

;; ======================================================================== ;;
;;  VIDEO_OFF     -- Disable video, and wait until interrupt happens.       ;;
;; ======================================================================== ;;
VIDEO_OFF   PROC
            MVII    #ISR_VOFF_X, R0
            MVO     R0,     $100
            SWAP    R0
            MVO     R0,     $101
            CLRR    R0
            MVO     R0,     VFLAG
@@loop      CMP     VFLAG,  R0
            BEQ     @@loop
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  VIDEO_ON      -- Enable video.  Don't bother waiting for interrupt.     ;;
;; ======================================================================== ;;
VIDEO_ON    PROC
            MVII    #ISR_VON,  R0
            MVO     R0,     $100
            SWAP    R0
            MVO     R0,     $101
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  ECS_ENABLE    -- Enable the ECS ROMs.                                   ;;
;; ======================================================================== ;;
ECS_ENABLE  PROC
            CLRR    R0
            MVO     R0,     ECSDIS
            MVII    #$2A51, R0
            MVO     R0,     $2FFF
            MVII    #$7A50, R0
            MVO     R0,     $7FFF
            MVII    #$EA51, R0
            MVO     R0,     $EFFF
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  ECS_DISABLE   -- Disable the ECS ROMs.                                  ;;
;; ======================================================================== ;;
ECS_DISABLE PROC
            MVII    #$2A5F, R0
            MVO     R0,     $2FFF
            MVII    #$7A5F, R0
            MVO     R0,     $7FFF
            MVII    #$EA5F, R0
            MVO     R0,     $EFFF
            MVO     R0,     ECSDIS
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  ECS_SETPAGE   -- Set a bank on an ECS Paged ROM.                        ;;
;; ======================================================================== ;;
ECS_SETPAGE PROC
            PSHR    R5
            PSHR    R4


            CALL    EC_XFER         ; Read address range # 
            MOVR    R0,     R1      ; (save it)

            CALL    EC_XFER         ; Read page number to select in.

            SWAP    R1              ; \   Put address range in upper half
            SLL     R1,     2       ;  |- Align it into upper nibble.
            SLL     R1,     2       ; /
            MOVR    R1,     R4      ; Save address range in R4.
            ADDR    R0,     R1      ; Merge address range w/ page # in R1
            ANDI    #$F00F, R1      ; Mask to make sure it's a valid #.
            XORI    #$0A50, R1      ; Make R1 the xA5y value.
            XORI    #$0FFF, R4      ; Make R4 the xFFF value
            MVO@    R1,     R4      ; Write xA5y to xFFF.

            MVII    #$A,    R1      ; Send 'ACK' to PC.
            CALL    EC_XFER

            PULR    R4
            PULR    PC
            ENDP


;; ======================================================================== ;;
;;  INITISR -- Initializes the Monitor's desired state of universe.         ;;
;;             Actually, all the work is done in INITISR2.                  ;;
;; ======================================================================== ;;
INITISR     PROC
            MVII    #INITISR2,  R0
            MVO     R0,     $100
            SWAP    R0
            MVO     R0,     $101
            MVO     R0,     INITWAIT
            EIS
@@spin      CMP     INITWAIT, R0
            BEQ     @@spin
            JR      R5
            ENDP

INITISR2    PROC
            CLRR    R0                  ; \
            CLRR    R4                  ;  |
            MVII    #$20,   R1          ;  |__ Clear the STIC MOB registers
@@loop      MVO@    R0,     R4          ;  |
            DECR    R1                  ;  |
            BNEQ    @@loop              ; / 

            ADDI    #8,     R4          ; \
            MVII    #$18,   R1          ;  |
@@loop2     MVO@    R0,     R4          ;  |-- Clear $28 .. $3F.
            DECR    R1                  ;  |
            BNEQ    @@loop2             ; /

            MVII    #8,     R0          ; \
            CLRR    R1                  ;  |
            MVII    #$28,   R4          ;  |
            MVO@    R0,     R4          ;  |__ Set up the color stack the
            MVO@    R1,     R4          ;  |   way we would like it.
            MVO@    R0,     R4          ;  |
            MVO@    R0,     R4          ;  |
            MVO@    R0,     R4          ; /
            
            MVI     $21,    R0          ; Color stack display mode
            MVO     R0,     $20         ; Enable video display.

            MVII    #ISR_VON, R0        ; \
            MVO     R0,     $100        ;  |__ Point to our real ISR.
            SWAP    R0                  ;  |
            MVO     R0,     $101        ; /

            CLRR    R0
            MVO     R0,     INITWAIT
            JR      R5                  ; Return.
            ENDP

;; ======================================================================== ;;
;;  ISR_VON -- Does a very minimal amount of work.                          ;;
;; ======================================================================== ;;
ISR_VON     PROC
            MVO     R5,     $20
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  ISR_VOFF -- Does a very minimal amount of work.                         ;;
;; ======================================================================== ;;
ISR_VOFF    EQU     $1014 
ISR_VOFF_X  PROC
            MVII    #ISR_VOFF,R0
            MVO     R0,     $100
            SWAP    R0
            MVO     R0,     $101
            MVO     R5,     VFLAG
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  EC_SPINIDLE      -- Idle & wait for PC to take us un-idle.              ;;
;; ======================================================================== ;;
EC_SPINIDLE PROC
            PSHR    R5
            PSHR    R1
            PSHR    R0

            MVI     P1CTL,  R0      ; Get current I/O direction bits
            ANDI    #$3F,   R0      ; Clear direction flags
            PSHR    R0

            CLRR    R5
            B       @@spin

@@oloop:
            PULR    R0
            MVO     R0,     P1CTL   ; After suitable timeout, shut off I/O
                                    ; drivers on PSG.
            PSHR    R0
            MVII    #$200 + 6*20, R4
            MVII    #$2F0 - ($200 + 6*20), R1
            CALL    FILLZERO

@@spin:     DECR    R5
            BEQ     @@oloop
            CLRR    R0
            CMP     P1IN,   R0      ; Spin as long as P1IN != 0x00
            BEQ     @@nspin
            CMP     P1IN,   R0      ; Spin as long as P1IN != 0x00
            BEQ     @@nspin
            CMP     P1IN,   R0      ; Spin as long as P1IN != 0x00
            BEQ     @@nspin
            CMP     P1IN,   R0      ; Spin as long as P1IN != 0x00
            BEQ     @@nspin
            CMP     P1IN,   R0      ; Spin as long as P1IN != 0x00
            BNEQ    @@spin
@@nspin

            PULR    R0
            XORI    #$40,   R0      ; Set our output port to output
            MVO     R0,     P1CTL
            CLRR    R0
            MVO     R0,     P1OUT   ; Acknowledge by sending a zero.

@@nidle:
            PULR    R0
            PULR    R1
            PULR    PC              ; Return.  We're un-idle.
            ENDP

;; ======================================================================== ;;
;;  EC_IDLE          -- Set cable to idle.                                  ;;
;; ======================================================================== ;;
EC_IDLE     PROC
            PSHR    R0

            MVI     P1CTL,  R0      ; Get current I/O direction bits
            ANDI    #$3F,   R0      ; Set both ports to "input"
            MVO     R0,     P1CTL
            
            PULR    R0
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  EC_XFER         -- Read a 7-bit quanitity while sending a 4-bit qty.    ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R1 -- Data to send.                                                 ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0 -- Data read (bits 7..1)                                         ;;
;;      R1 -- Sent data, masked by 0xFE.                                    ;;
;;      R2 ... R4 -- untouched.                                             ;;
;;      R5 -- trashed.                                                      ;;
;;                                                                          ;;
;;      C==0:  Operation successful                                         ;;
;;      C==1:  Operation timed out.  Also, sets cable to idle.              ;;
;; ======================================================================== ;;
EC_XFER     PROC
            
            ANDI    #$FFFE, R1

            ; Wait for clock to go high.  When it does, respond with our
            ; data and set our clock high.
@@clkhi:    MVI     P1IN,   R0
            SARC    R0
            BC      @@clkhi_done
            MVI     P1IN,   R0
            SARC    R0
            BC      @@clkhi_done
            MVI     P1IN,   R0
            SARC    R0
            BC      @@clkhi_done
            MVI     P1IN,   R0
            SARC    R0
            BNC     @@clkhi
@@clkhi_done:
            INCR    R1
            MVO     R1,     P1OUT

            ; Wait for clock to go low.  When it does, respond with our
            ; data and set our clock low.
@@clklo:    MVI     P1IN,   R0
            SARC    R0
            BNC     @@clklo_done
            MVI     P1IN,   R0
            SARC    R0
            BNC     @@clklo_done
            MVI     P1IN,   R0
            SARC    R0
            BNC     @@clklo_done
            MVI     P1IN,   R0
            SARC    R0
            BC      @@clklo
@@clklo_done:
            DECR    R1
            MVO     R1,     P1OUT

            JR      R5
            ENDP

;; ======================================================================== ;;
;;  EC_GETCMD       -- Get a command from the PC.  Returns with C==1 on err ;;
;;                     Command in R1.                                       ;;
;; ======================================================================== ;;
EC_GETCMD   PROC
            PSHR    R5
            PSHR    R0
            PSHR    R2

            MVI     P1CTL,  R0
            ANDI    #$3F,   R0
            XORI    #$40,   R0
            MVO     R0,     P1CTL

            CALL    EC_XFER         ; Get command byte
            MOVR    R0,     R2
            CALL    EC_XFER         ; Get inverse command byte
            XORR    R2,     R0
            CMPI    #$7F,   R0      ; Make sure they're inverses.
            BNEQ    @@fail

            MVII    #$A,    R1
            CALL    EC_XFER         ; Send $A to say "OK"
            MOVR    R2,     R1

@@exit:     PULR    R2
            PULR    R0
            PULR    PC

@@fail:     CALL    EC_IDLE         ; Failure.  Idle the cable and leave
            SETC
            B       @@exit

            ENDP

;; ======================================================================== ;;
;;  EC_GETHDR        -- Get a command header from PC.  C==1 means error.    ;;
;;                      Address in R1, Length in R0.                        ;;
;; ======================================================================== ;;
EC_GETHDR   PROC
            PSHR    R5
            PSHR    R4

            CLRR    R4

            CALL    EC_XFER
            ADDR    R0,     R4      ; update checksum
            MOVR    R0,     R2      ; Save A7...A1
            SWAP    R2              ; swap halves around for a moment.

            CALL    EC_XFER
            ADDR    R0,     R4      ; update checksum
            ADDR    R0,     R2      ; Merge in A15...A9

            CALL    EC_XFER
            ADDR    R0,     R4      ; update checksum

            SARC    R0              ; \ Merge bit 8 into address
            RLC     R2              ; /
            SWAP    R2              ; swap halves back
            SARC    R0              ; \ Merge bit 0 into address
            ADCR    R2              ; /

            INCR    R0              ; R0 == length
            PSHR    R0

            CALL    EC_XFER         
            ANDI    #$7F,   R4
            CMPR    R0,     R4
            BNEQ    @@fail1

            MVII    #$A,    R1      ; Tell PC we received the header OK.
            CALL    EC_XFER

            MOVR    R2,     R1

            CLRC

            PULR    R0
@@exit:     PULR    R4
            PULR    PC

@@fail1:    PULR    R1
            PULR    R0
@@fail:     CALL    EC_IDLE         ; Error:  Idle the cable and leave.
            SETC
            PULR    R4
            PULR    PC

            ENDP

;; ======================================================================== ;;
;;  EC_XFER_ICART   -- Receive an array to Intellicart address space        ;;
;;  EC_XFER_INTY    -- Receive an array to Intellivision address space      ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Number of bytes                                               ;;
;;      R1 -- Address                                                       ;;
;;      R2 -- Transfer:                                                     ;;
;;             0 -- Receive Byte                                            ;;
;;             1 -- Receive Decle                                           ;;
;;             2 -- Receive Word                                            ;;
;;             3 -- NOP                                                     ;;
;;             4 -- Send Byte                                               ;;
;;             5 -- Send Decle                                              ;;
;;             6 -- Send Word                                               ;;
;;             7 -- NOP                                                     ;;
;;      R5 -- Return address                                                ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0..R3 -- trashed                                                   ;;
;;      R4 -- Points after array                                            ;;
;; ======================================================================== ;;
EC_XFER_ICART PROC
            PSHR    R5
            PSHR    R0
            PSHR    R1
            PSHR    R2

            ; Adjust the bankswitched memory at $0E00 to point to the
            ; correct address range in the Intellicart's address space.
            MOVR    R1,     R2
            SWAP    R2
            ANDI    #$FF,   R2      ; Get exact 256-byte page address.
            MVII    #WBANK, R1      ; Adjust bankswitch setting on $0E00
            CALL    IC_SETFBANK     ; Set it.

            PULR    R2
            PULR    R1
            PULR    R0
            ANDI    #$FF,   R1
            ADDI    #WINDOW,R1      ; Intellicart page is now mapped @$0E00.
            PULR    R5              ; Chain return through selected routine.
EC_XFER_INTY
            INCR    R2
            ADDR    PC,     R2
            MVI@    R2,     PC      ; Jump table!
            DECLE   EC_RCVBYTE
            DECLE   EC_RCVDECLE
            DECLE   EC_RCVWORD
            DECLE   EC_STUB
            DECLE   EC_SNDBYTE
            DECLE   EC_SNDDECLE
            DECLE   EC_SNDWORD
            DECLE   EC_STUB
EC_STUB     JR      R5
            ENDP


;; ======================================================================== ;;
;;  EC_RCVBYTE      -- Receive an array of bytes.                           ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Number of bytes                                               ;;
;;      R1 -- Address                                                       ;;
;;      R5 -- Return address                                                ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0..R3 -- trashed                                                   ;;
;;      R4 -- Points after array                                            ;;
;; ======================================================================== ;;
EC_RCVBYTE  PROC
            PSHR    R5
            MOVR    R1,     R4
            MOVR    R0,     R2


@@olp:      MVII    #7,     R3      ; We read in packets of 7.
            CMPR    R3,     R2      ; However last packet may be shorter.
            BPL     @@do7
            MOVR    R2,     R3      ; Clamp packet size to remaining bytes

@@do7:      SUBR    R3,     R2      ; Subtract packet length from byte count

            MOVR    R3,     R1
            ADDR    R1,     R1

@@ilp:      CALL    EC_XFER         ; Get the next byte
            MVO@    R0,     R4      ; Store it out
            SUBI    #2,     R1      ; Decrement our packet count.
            BNEQ    @@ilp           ; If it doesn't expire, keep going.

            
@@done:     CALL    EC_XFER         ; Get one more byte 

            SUBR    R3,     R4

@@xlp:      MVI@    R4,     R1      ; Now read in original byte
            DECR    R4              ; back up the ptr.
            SARC    R0              ; Get a bit to merge onto byte
            RLC     R1              ; Merge it onto the byte
            MVO@    R1,     R4      ; Store it back out.
            DECR    R3              ; Loop until done.
            BNEQ    @@xlp

            TSTR    R2
            BNEQ    @@olp           ; Loop as long as we have bytes left.
            
            MVII    #$A,    R1
            CALL    EC_XFER         ; Send an $A to say we're OK.
            CMPI    #$55,   R0
            BNEQ    @@fail

            CLRC
            PULR    PC

@@fail:     SETC
            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  EC_RCVDECLE     -- Receive an array of decles.                          ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Number of bytes                                               ;;
;;      R1 -- Address                                                       ;;
;;      R5 -- Return address                                                ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0..R3 -- trashed                                                   ;;
;;      R4 -- Points after array                                            ;;
;; ======================================================================== ;;
EC_RCVDECLE PROC
            PSHR    R5
            MOVR    R1,     R4
            MOVR    R0,     R2


@@olp:      MVII    #7,     R3      ; We read in packets of 7.
            CMPR    R3,     R2      ; However last packet may be shorter.
            BPL     @@do7
            MOVR    R2,     R3      ; Clamp packet size to remaining bytes

@@do7:      SUBR    R3,     R2      ; Subtract packet length from byte count

            PSHR    R2

            MOVR    R3,     R1
            ADDR    R1,     R1
@@ilp:      CALL    EC_XFER         ; Get the next byte
            MVO@    R0,     R4      ; Store it out
            SUBI    #2,     R1      ; Decrement our packet count.
            BNEQ    @@ilp           ; If it doesn't expire, keep going.

            
@@done:     MVII    #$10,   R1
            CALL    EC_XFER         ; Get one more byte for bit 2.
            MOVR    R0,     R2      ; the byte w/ bit2 goes in R2

            CALL    EC_XFER         ; get first byte of bit0/bit1 word
            MOVR    R0,     R1
            ADDR    R1,     R1
            CALL    EC_XFER         ; get second byte of bit0/bit1 word
            SWAP    R0              ; actually put value *in* upper half
            ADDR    R1,     R0      ; merge two halves
            SLR     R0              ; R0 has bit1/bit0

            SUBR    R3,     R4

@@xlp:      MVI@    R4,     R1      ; Now read in original decle
            SARC    R2              ; Get bit2 to merge onto decle
            RLC     R1              ; Merge it onto the decle
            DECR    R4              ; back up the ptr.
            RRC     R0,     2       ; Get bit0/bit1 to merge onto decle
            RLC     R1,     2       ; Merge bit1/bit0 onto decle
            MVO@    R1,     R4      ; Store it back out.
            DECR    R3              ; Loop until done.
            BNEQ    @@xlp

            PULR    R2
            TSTR    R2
            BNEQ    @@olp           ; Loop as long as we have bytes left.
            
            MVII    #$A,    R1
            CALL    EC_XFER         ; Send an $A to say we're OK.
            CMPI    #$55,   R0
            BNEQ    @@fail

            CLRC
            PULR    PC

@@fail:     SETC
            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  EC_RCVWORD      -- Receive an array of 16-bit words.                    ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Number of bytes                                               ;;
;;      R1 -- Address                                                       ;;
;;      R5 -- Return address                                                ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0..R3 -- trashed                                                   ;;
;;      R4 -- Points after array                                            ;;
;; ======================================================================== ;;
EC_RCVWORD  PROC
            PSHR    R5
            MOVR    R1,     R4
            MOVR    R0,     R2


@@olp:      MVII    #7,     R3      ; We read in packets of 7.
            CMPR    R3,     R2      ; However last packet may be shorter.
            BPL     @@do7
            MOVR    R2,     R3      ; Clamp packet size to remaining bytes

@@do7:      SUBR    R3,     R2      ; Subtract packet length from byte count

            PSHR    R2
            MOVR    R3,     R1

            SLL     R1,     1 
@@ilp:      CALL    EC_XFER         ; Get the lo-half byte
            MOVR    R0,     R2
            CALL    EC_XFER         ; Get the hi-half byte
            SWAP    R0              ; put it in the high half
            ADDR    R2,     R0      ; merge w/ lo half
            MVO@    R0,     R4      ; Store it out
            SUBI    #2,     R1      ; Decrement our packet count.
            BNEQ    @@ilp           ; If it doesn't expire, keep going.

            
@@done:     MVII    #$10,   R1
            CALL    EC_XFER         ; Get bit8/bit0 word (lo half)
            MOVR    R0,     R1
            SLL     R1,     1 
            CALL    EC_XFER         ; Get bit8/bit0 word (hi half)
            SWAP    R0              ; put in hi half
            ADDR    R1,     R0      ; merge with lo half
            SLR     R0              ; 

            SUBR    R3,     R4

@@xlp:      MVI@    R4,     R1      ; Now read in original byte
            DECR    R4              ; back up the ptr.
            SARC    R0              ; Get bit 8 to merge into word
            SWAP    R1
            RLC     R1              ; Merge in bit 8
            SARC    R0              ; Get bit 0 to merge into word
            SWAP    R1
            ADCR    R1              ; Merge in bit 0
            MVO@    R1,     R4      ; Store it back out.
            DECR    R3              ; Loop until done.
            BNEQ    @@xlp

            PULR    R2
            TSTR    R2
            BNEQ    @@olp           ; Loop as long as we have bytes left.
            
            MVII    #$A,    R1
            CALL    EC_XFER         ; Send an $A to say we're OK.
            CMPI    #$55,   R0
            BNEQ    @@fail

            CLRC
            PULR    PC

@@fail:     SETC
            PULR    PC
            ENDP

            
;; ======================================================================== ;;
;;  EC_SNDBYTE      -- Send an array of bytes.                              ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Number of bytes                                               ;;
;;      R1 -- Address                                                       ;;
;;      R5 -- Return address                                                ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0..R3 -- trashed                                                   ;;
;;      R4 -- Points after array                                            ;;
;; ======================================================================== ;;
EC_SNDBYTE  PROC
            PSHR    R5

            MOVR    R0,     R2

            MOVR    R1,     R4
@@loop:     MVI@    R4,     R1
            SLL     R1,     1
            CALL    EC_XFER
            SLR     R1,     2
            SLR     R1,     2
            CALL    EC_XFER
            
            DECR    R2
            BNEQ    @@loop

            MVII    #$A,    R1
            CALL    EC_XFER
            CMPI    #$55,   R0
            BNEQ    @@fail
            CLRC
            INCR    PC

@@fail:     SETC
            PULR    PC
            ENDP
            
;; ======================================================================== ;;
;;  EC_SNDDECLE     -- Send an array of 10-bit values.                      ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Number of bytes                                               ;;
;;      R1 -- Address                                                       ;;
;;      R5 -- Return address                                                ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0..R3 -- trashed                                                   ;;
;;      R4 -- Points after array                                            ;;
;; ======================================================================== ;;
EC_SNDDECLE PROC
            PSHR    R5

            MOVR    R0,     R2

            MOVR    R1,     R4
@@loop:     MVI@    R4,     R1
            ADDR    R1,     R1
            CALL    EC_XFER
            SLR     R1,     2
            SLR     R1,     2
            CALL    EC_XFER
            SLR     R1,     2
            SLR     R1,     2
            CALL    EC_XFER
            
            DECR    R2
            BNEQ    @@loop

            MVII    #$A,    R1
            CALL    EC_XFER
            CMPI    #$55,   R0
            BNEQ    @@fail
            CLRC
            INCR    PC

@@fail:     SETC
            PULR    PC
            ENDP
            
;; ======================================================================== ;;
;;  EC_SNDWORD      -- Send an array of 16-bit words                        ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Number of bytes                                               ;;
;;      R1 -- Address                                                       ;;
;;      R5 -- Return address                                                ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0..R3 -- trashed                                                   ;;
;;      R4 -- Points after array                                            ;;
;; ======================================================================== ;;
EC_SNDWORD  PROC
            PSHR    R5

            MOVR    R0,     R2

            MOVR    R1,     R4
@@loop:     MVI@    R4,     R1
            MOVR    R1,     R3
            ADDR    R1,     R1
            CALL    EC_XFER
            MOVR    R3,     R1
            SLR     R1,     1
            SLR     R1,     2
            CALL    EC_XFER
            SLR     R1,     2
            SLR     R1,     2
            CALL    EC_XFER
            SLR     R1,     2
            SLR     R1,     2
            CALL    EC_XFER
            DECR    R2
            BNEQ    @@loop

            MVII    #$A,    R1
            CALL    EC_XFER
            CMPI    #$55,   R0
            BNEQ    @@fail
            CLRC
            INCR    PC

@@fail:     SETC
            PULR    PC
            ENDP


;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
            INCLUDE "../library/fillmem.asm"    ; FILLZERO/FILLMEM/CLRSCR
            INCLUDE "../library/ic_banksw.asm"  ; Intellicart bankswitch rtns
            INCLUDE "../library/print.asm"      ; PRINT.xxx routines
            INCLUDE "../library/hex16.asm"      ; HEX16 


;; ======================================================================== ;;
;;  EC_POLL  -- Check to see if ECS cable is non-idle, and if so, execute   ;;
;;              a single ECS cable command.  Only a subset of the commands  ;;
;;              are supported.                                              ;;
;; ======================================================================== ;;

            ORG     $CF00
            DECLE   $69

EC_POLL     PROC
            PSHR    R5
            MVO     R0,     R0SAV
            DIS
            MVI     BUSY,   R0
            TSTR    R0
            BNEQ    @@leave


            MVI     P1IN,   R0
            TSTR    R0
            BEQ     @@doit

@@leave:    MVI     IDLECNT, R0
            DECR    R0
            ANDI    #$3FF,  R0
            MVO     R0,     IDLECNT
            BNEQ    @@noidle

            MVI     P1CTL,  R0
            ANDI    #$3F,   R0
            XORI    #$40,   R0
            MVO     R0,     P1CTL
            CLRR    R0
            DECR    R0
            MVO     R0,     P1OUT

@@noidle:   EIS
            MVI     R0SAV,  R0
            PULR    PC

@@doit:     
            MVO     R1,     R1SAV
            MVO     R2,     R2SAV
            MVO     R3,     R3SAV
            MVI     P1CTL,  R0
            MVO     R4,     R4SAV
            MVO     PC,     BUSY
            EIS

            ANDI    #$3F,   R0
            XORI    #$40,   R0
            MVO     R0,     P1CTL

            CLRR    R0
            MVO     R0,     P1OUT

            CALL    EC_GETCMD           ; Get the command from the PC.
            BC      @@exit              ; If there's an error, fail out.

            CMPI    #$17,   R1          ; Make sure command # is in range.
            BGE     @@exit              ; Signal an error if it isn't.

            CMPI    #$08,   R1
            BGT     @@xfer_cmd

            ADDI    #PCMDTBL,R1
            MVII    #@@exit,R5          ; Exit after command
            MVI@    R1,     PC          ; Jump to command.

@@xfer_cmd: PSHR    R1                  ; Save the command number

            CALL    EC_GETHDR           ; Read the command header
            BC      @@exit              ; Fail on error.

            PULR    R2                  ; 
            SUBI    #$10,   R2          ; 
            BPL     @@xfer_icart        ; Dispatch to approp. XFER routine.

            ADDI    #$8,    R2          ; \
            CALL    EC_XFER_INTY        ;  |__ Commands 8..F transfer to
            B       @@exit              ; /

@@xfer_icart
            CALL    EC_XFER_ICART       ; \___ Intellicart address space

@@exit      
            DIS
            CLRR    R0
            MVO     R0,     BUSY
            DECR    R0
            MVO     R0,     IDLECNT
            EIS
            MVI     R0SAV,  R0
            MVI     R1SAV,  R1
            MVI     R2SAV,  R2
            MVI     R3SAV,  R3
            MVI     R4SAV,  R4

            PULR    PC

            ENDP

;; ======================================================================== ;;
;;  Command jump table for various low-numbered commands.                   ;;
;; ======================================================================== ;;
PCMDTBL     PROC
            DECLE   NOP             ; 0x00
            DECLE   RESET_GAME      ; 0x01
            DECLE   RESET_MONITOR   ; 0x02
            DECLE   NOP             ; 0x03
            DECLE   NOP             ; 0x04
            DECLE   NOP             ; 0x05
            DECLE   NOP             ; 0x06
            DECLE   NOP             ; 0x07
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
;* ------------------------------------------------------------------------ *;
;*                   Copyright (c) 2001, Joseph Zbiciak                     *;
;* ======================================================================== *;
