;;==========================================================================;;
;; Joe Zbiciak's LIFE DEMO                                                  ;;
;; Copyright 2002, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  TO BUILD IN BIN+CFG FORMAT:                                             *;
;*      as1600 -o life.bin -l life.lst life.asm                             *;
;*                                                                          *;
;*  TO BUILD IN ROM FORMAT:                                                 *;
;*      as1600 -o life.rom -l life.lst life.asm                             *;
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


            ROMW    16              ; Use 16-bit ROM width

;------------------------------------------------------------------------------
; Include system information
;------------------------------------------------------------------------------
            INCLUDE "../library/gimini.asm"

;------------------------------------------------------------------------------
; Set up our variables in scratch and system memory.  
;------------------------------------------------------------------------------
SCRATCH     ORG     $100,   $100,   "-RWBN"
ISRVEC      RMB     2
GEN0        RMB     96
GEN1        RMB     96
ROWB        RMB     32
_EOSCR      EQU     $
           
           
SYSTEM      ORG     $2F0,   $2F0,   "-RWBN"
STACK       RMB     32
TICKS       RMB     1
RNDLO       RMB     1
RNDHI       RMB     1
CURGEN      RMB     1
NXTGEN      RMB     1
CLTMP       RMB     4               ; temp space for CALCLIFE
XSAVE       RMB     1               ; \
YSAVE       RMB     1               ;  |-- for colored-square routines
CSAVE       RMB     1               ; /
LOOP        RMB     1
GENCOL      RMB     1
GCNUM       RMB     1
_EOSYS      EQU     $

;------------------------------------------------------------------------------
; Build options:  Set HGLIDER or VGLIDER to 1 to test with the gliders.
;                 Otherwise the screen will be randomized at start.
;------------------------------------------------------------------------------
HGLIDER     EQU     0
VGLIDER     EQU     0

;------------------------------------------------------------------------------
; Magic numbers for ECScable polling.
;------------------------------------------------------------------------------
EC_LOC      EQU     $CF00
EC_MAG      EQU     $69
EC_POLL     EQU     $CF01
           
            ORG     $5000           ; Use default memory map
;------------------------------------------------------------------------------
; EXEC-friendly ROM header.
;------------------------------------------------------------------------------
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
            BYTE    102, 'Life Demo', 0
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

            ; ------------------------------------------------------------- ;
            ;  Prepare to jettison EXEC support.  Also, initialize random   ;
            ;  number generator from everything in memory.                  ;
            ; ------------------------------------------------------------- ;
            DIS
            MVII    #$100,  R4
            MVII    #$130,  R3
@@rnd:     
            ADD@    R4,     R0
            ADD@    R4,     R1
            DECR    R3
            BNEQ    @@rnd
           
            TSTR    R0
            BNEQ    @@r0_ok
            MOVR    PC,     R0
@@r0_ok:
            TSTR    R1
            BNEQ    @@r1_ok
            MOVR    PC,     R1
@@r1_ok:
            MVO     R0,     RNDLO
            MVO     R1,     RNDHI
           
            MVII    #STACK, R6
           
            MVII    #MYISR, R0
            MVO     R0,     ISRVEC
            SWAP    R0
            MVO     R0,     ISRVEC+1
           
           
            EIS
           
            ; ------------------------------------------------------------- ;
            ;  Initialize the display to colored-squares mode.              ;
            ; ------------------------------------------------------------- ;
            MVII    #$00F0, R1      ; Initialize by filling the screen.
            MVII    #$0200, R4      ;
            MVII    #$37FF, R0      ;
            CALL    FILLMEM         ; Fill screen w/ grey colored squares
           
            IF      (HGLIDER OR VGLIDER) = 0 ; No gliders?  Do random.
            ; ------------------------------------------------------------- ;
            ;  Fill our GEN buffers with random data.                       ;
            ; ------------------------------------------------------------- ;
            MVII    #192,   R1
            MVII    #GEN0,  R4
@@rfill:
            MVII    #8,     R0
            CALL    RAND
            MVO@    R0,     R4
            DECR    R1
            BNEQ    @@rfill
         
            ELSE    ; do either horizontal glider or vertical glider
         
            MVII    #192,   R1
            MVII    #GEN0,  R4
            CALL    FILLZERO
         
            IF      HGLIDER NE 0
            ; ------------------------------------------------------------- ;
            ;  Put a horizontal glider near top right.                      ;
            ; ------------------------------------------------------------- ;
         
            MVII    #%0100100, R0
            MVO     R0,     GEN0 + 1 + 4*2
            MVII    #%0000010, R0
            MVO     R0,     GEN0 + 1 + 4*3
            MVII    #%0100010, R0
            MVO     R0,     GEN0 + 1 + 4*4
            MVII    #%0011110, R0
            MVO     R0,     GEN0 + 1 + 4*5
         
            ENDI    ;HGLIDER
         
            IF      VGLIDER NE 0
            ; ------------------------------------------------------------- ;
            ;  Put a vertical glider near top left.                         ;
            ; ------------------------------------------------------------- ;
         
            MVII    #%0101000, R0
            MVO     R0,     GEN0 + 3 + 4*5
            MVII    #%0000100, R0
            MVO     R0,     GEN0 + 3 + 4*6
            MVII    #%0000100, R0
            MVO     R0,     GEN0 + 3 + 4*7
            MVII    #%0100100, R0
            MVO     R0,     GEN0 + 3 + 4*8
            MVII    #%0011100, R0
            MVO     R0,     GEN0 + 3 + 4*9
         
            ENDI    ;VGLIDER
         
            ENDI    ; VGLIDER OR HGLIDER
         
            ; ------------------------------------------------------------- ;
            ;  Set up the initial "life" state.                             ;
            ; ------------------------------------------------------------- ;
            MVII    #GEN0,  R0
            MVO     R0,     CURGEN
            MVII    #GEN1,  R0
            MVO     R0,     NXTGEN
         
            MVII    #1,     R0    
            MVO     R0,     GCNUM
         
            ; ------------------------------------------------------------- ;
            ;  Decide if we will be polling the ECScable every iteration.   ;
            ; ------------------------------------------------------------- ;
            MVII    #@@main_loop, R1
            MVI     EC_LOC, R0
            CMPI    #EC_MAG,R0
            BNEQ    @@no_poll
            MVII    #@@ec_poll,   R1
@@no_poll:  MVO     R1,     LOOP

            JR      R1    ; Start first loop iteration.


            ; ------------------------------------------------------------- ;
            ;  If ECScable Monitor is found, we'll call EC_POLL after       ;
            ;  every generation.  Otherwise, we won't.                      ;
            ; ------------------------------------------------------------- ;
@@ec_poll:  CALL    EC_POLL

            ; ------------------------------------------------------------- ;
            ;  Pause between generations if controller is pressed.          ;
            ; ------------------------------------------------------------- ;
@@main_loop:
            MVI     $1FE,   R0
            AND     $1FF,   R0
            XORI    #$FF,   R0
            BNEQ    @@main_loop

            ; ------------------------------------------------------------- ;
            ;  Benchmarking:  Reset the tick counter.                       ;
            ; ------------------------------------------------------------- ;
            CLRR    R0
            MVO     R0,     TICKS

            ; ------------------------------------------------------------- ;
            ;  Increment our color selection.  Cycle from 1 thru 6.         ;
            ; ------------------------------------------------------------- ;
            MVI     GCNUM,  R1
            INCR    R1
            CMPI    #6,     R1
            BLE     @@gcok
            MVII    #1,     R1
@@gcok:     MVO     R1,     GCNUM
            ADDI    #COLORS,R1
            MVI@    R1,     R1
            MVO     R1,     GENCOL
           
            ; ------------------------------------------------------------- ;
            ;  Show current generation and calculate the next one.          ;
            ; ------------------------------------------------------------- ;
            CALL    SHOWLIFE
            CALL    CALCLIFE
           
            ; ------------------------------------------------------------- ;
            ;  Swap our double-buffer pointers.                             ;
            ; ------------------------------------------------------------- ;
            MVI     CURGEN, R0
            MVI     NXTGEN, R1
            MVO     R1,     CURGEN
            MVO     R0,     NXTGEN
           
            ; ------------------------------------------------------------- ;
            ;  Display the # of ticks it took to calculate in lower right.  ;
            ; ------------------------------------------------------------- ;
            MVI     TICKS,  R0
            MVII    #C_BLK, R1
            MVII    #$2EE,  R4
            CALL    HEX8
           
            ; ------------------------------------------------------------- ;
            ;  Loop back to either @@main_loop or @@ec_poll.                ;
            ; ------------------------------------------------------------- ;
            MVI     LOOP,   PC
           
@@spin      DECR    PC          ; can't get here.
            ENDP

;; ======================================================================== ;;
;;  MYISR -- Just keep the display on and increment the tick counter.       ;;
;; ======================================================================== ;;
MYISR       PROC
            PSHR    R5
           
            MVO     R0,     $20     ; Keep display on
            MVI     $21,    R0      ; Keep it in color-stack mode
           
            MVII    #C_GRY, R0
            MVO     R0,     $28     ; Set color-stack, border to grey
            MVO     R0,     $2C
           
            MVI     TICKS,  R0
            INCR    R0
            MVO     R0,     TICKS
           
            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  LIFE RULES:                                                             ::
;;                                                                          ;;
;;   -- A dead cell with exactly three live neighbors becomes a live        ;;
;;      cell (birth).                                                       ;:
;;                                                                          ;;
;;   -- A live cell with two or three live neighbors stays alive            ;;
;;      (survival).                                                         ;;
;;                                                                          ;;
;;   -- In all other cases, a cell dies or remains dead (overcrowding       ;;
;;      or loneliness).                                                     ;;
;;                                                                          ;;
;;  OUR WORLD                                                               ;;
;;      We display a 32 x 24 "world" for the cells to live in.  The world   ;;
;;      is "toroidal", in that the top connects to the bottom, and the      ;;
;;      left connects to the right.  This world holds 768 cells.            ;;
;;                                                                          ;;
;;      The cell information for the current and next generations are       ;;
;;      held in two bitmaps in 8-bit scratch RAM.  We store 1 bit per       ;;
;;      cell, so each bitmap occupies 96 bytes.  The bitmap is stored in    ;;
;;      "little endian" order -- the leftmost cell onscreen is in bit 0     ;;
;;      of the first byte, not bit 7 as you may expect.                     ;;
;;                                                                          ;;
;;      The engine works by calculating the new generation from the last    ;;
;;      generation's state according to the rules above.  It then swaps     ;;
;;      the two generations.  We don't actually move anything in memory     ;;
;;      when we swap -- we keep track of which world is which with          ;;
;;      pointers.                                                           ;;
;;                                                                          ;;
;;      Once a new generation is calculated, we display it.  That's it.     ;;
;; ======================================================================== ;;


;; ======================================================================== ;;
;;  LDTBL -- Live/Dead decision table.                                      ;;
;;                                                                          ;;
;;  This lookup table implements the rules for deciding whether a cell      ;;
;;  lives or dies.  A 0 means that the cell is dead in the next generation  ;;
;;  and a $8000 means that it is alive.  The table is indexed by a 4-bit    ;;
;;  value.  Bits 3 thru 1 hold the neighbor+self count, and bit 0 holds     ;;
;;  whether the cell is alive in the present generation.                    ;;
;; ======================================================================== ;;
LDTBL       PROC
            DECLE   0       ; 0 neighbors, dead  -> dead
            DECLE   0       ; na

            DECLE   0       ; 1 neighbors, dead  -> dead
            DECLE   0       ; 0 neighbors, live  -> dead

            DECLE   0       ; 2 neighbors, dead  -> dead
            DECLE   0       ; 1 neighbors, live  -> dead

            DECLE   $8000   ; 3 neighbors, dead  -> live
            DECLE   $8000   ; 2 neighbors, live  -> live

            DECLE   0       ; 4 neighbors, dead  -> dead
            DECLE   $8000   ; 3 neighbors, live  -> live

            DECLE   0       ; 5 neighbors, dead  -> dead
            DECLE   0       ; 4 neighbors, live  -> dead

            DECLE   0       ; 6 neighbors, dead  -> dead
            DECLE   0       ; 5 neighbors, live  -> dead

            DECLE   0       ; 7 neighbors, dead  -> dead
            DECLE   0       ; 6 neighbors, live  -> dead

            DECLE   0       ; 8 neighbors, dead  -> dead
            DECLE   0       ; 7 neighbors, live  -> dead

            DECLE   0       ; na
            DECLE   0       ; 8 neighbors, live  -> dead
            ENDP


;; ======================================================================== ;;
;;  EXP -- expands an 8 1-bit fields into 8 2-bit fields.                   ;;
;;                                                                          ;;
;;  This lookup table takes an 8-bit value and expands it to a 16 bit       ;;
;;  value by inserting 0s between each bit.  This effectively takes the     ;;
;;  8 1-bit fields of the original number and turns them into 8 2-bit       ;;
;;  fields.                                                                 ;;
;;                                                                          ;;
;;  We use this in two places.  The first is where we add across three      ;;
;;  rows of cells.  We load a single byte containing 8 cells, ane expand    ;;
;;  that to 8 2-bit fields.  We add the expanded bytes for all three rows   ;;
;;  together, yeilding 8 sums that range from 00b through 11b (0 to 3).     ;;
;;  Those 8 fields are then unpacked to bytes.                              ;;
;;                                                                          ;;
;;  The second place is in the display routine.  We load pixel information  ;;
;;  for two rows, and then interleave the pixels for those rows.  This      ;;
;;  places pixels that will be displayed vertically relative to each other  ;;
;;  in adjacent bit positions.  This makes it easy to then generate the     ;;
;;  colored-square information, since colored-squares mode stores 2x2       ;;
;;  in each card and thus needs pixel information from two rows.            ;;
;; ======================================================================== ;;
EXP PROC
    DECLE   $0000,  $0001,  $0004,  $0005,  $0010,  $0011,  $0014,  $0015
    DECLE   $0040,  $0041,  $0044,  $0045,  $0050,  $0051,  $0054,  $0055
    DECLE   $0100,  $0101,  $0104,  $0105,  $0110,  $0111,  $0114,  $0115
    DECLE   $0140,  $0141,  $0144,  $0145,  $0150,  $0151,  $0154,  $0155
    DECLE   $0400,  $0401,  $0404,  $0405,  $0410,  $0411,  $0414,  $0415
    DECLE   $0440,  $0441,  $0444,  $0445,  $0450,  $0451,  $0454,  $0455
    DECLE   $0500,  $0501,  $0504,  $0505,  $0510,  $0511,  $0514,  $0515
    DECLE   $0540,  $0541,  $0544,  $0545,  $0550,  $0551,  $0554,  $0555
    DECLE   $1000,  $1001,  $1004,  $1005,  $1010,  $1011,  $1014,  $1015
    DECLE   $1040,  $1041,  $1044,  $1045,  $1050,  $1051,  $1054,  $1055
    DECLE   $1100,  $1101,  $1104,  $1105,  $1110,  $1111,  $1114,  $1115
    DECLE   $1140,  $1141,  $1144,  $1145,  $1150,  $1151,  $1154,  $1155
    DECLE   $1400,  $1401,  $1404,  $1405,  $1410,  $1411,  $1414,  $1415
    DECLE   $1440,  $1441,  $1444,  $1445,  $1450,  $1451,  $1454,  $1455
    DECLE   $1500,  $1501,  $1504,  $1505,  $1510,  $1511,  $1514,  $1515
    DECLE   $1540,  $1541,  $1544,  $1545,  $1550,  $1551,  $1554,  $1555
    DECLE   $4000,  $4001,  $4004,  $4005,  $4010,  $4011,  $4014,  $4015
    DECLE   $4040,  $4041,  $4044,  $4045,  $4050,  $4051,  $4054,  $4055
    DECLE   $4100,  $4101,  $4104,  $4105,  $4110,  $4111,  $4114,  $4115
    DECLE   $4140,  $4141,  $4144,  $4145,  $4150,  $4151,  $4154,  $4155
    DECLE   $4400,  $4401,  $4404,  $4405,  $4410,  $4411,  $4414,  $4415
    DECLE   $4440,  $4441,  $4444,  $4445,  $4450,  $4451,  $4454,  $4455
    DECLE   $4500,  $4501,  $4504,  $4505,  $4510,  $4511,  $4514,  $4515
    DECLE   $4540,  $4541,  $4544,  $4545,  $4550,  $4551,  $4554,  $4555
    DECLE   $5000,  $5001,  $5004,  $5005,  $5010,  $5011,  $5014,  $5015
    DECLE   $5040,  $5041,  $5044,  $5045,  $5050,  $5051,  $5054,  $5055
    DECLE   $5100,  $5101,  $5104,  $5105,  $5110,  $5111,  $5114,  $5115
    DECLE   $5140,  $5141,  $5144,  $5145,  $5150,  $5151,  $5154,  $5155
    DECLE   $5400,  $5401,  $5404,  $5405,  $5410,  $5411,  $5414,  $5415
    DECLE   $5440,  $5441,  $5444,  $5445,  $5450,  $5451,  $5454,  $5455
    DECLE   $5500,  $5501,  $5504,  $5505,  $5510,  $5511,  $5514,  $5515
    DECLE   $5540,  $5541,  $5544,  $5545,  $5550,  $5551,  $5554,  $5555
    ENDP
        
          

;; ======================================================================== ;;
;;  CALCLIFE                                                                ;;
;;  Scan CURGEN and generate NXTGEN.  Treats screen as a torus.             ;;
;;  Requires 4 words of temporary storage for pointers, and a 32-byte       ;;
;;  temporary buffer (ROWB).                                                ;;
;; ======================================================================== ;;
CALCLIFE    PROC
            PSHR    R5

@@prev      EQU     CLTMP + 0   ; pointer to row - 1  (modulo screen height)
@@curr      EQU     CLTMP + 1   ; pointer to current row
@@next      EQU     CLTMP + 2   ; pointer to row + 1  (modulo screen height)
@@outp      EQU     CLTMP + 3   ; output pointer

            MVI     CURGEN, R0      ; Get pointer to current generation
            MVO     R0,     @@curr  ; Make top row "current" row
            ADDI    #4,     R0      ;
            MVO     R0,     @@next  ; Make second row "next" row
            ADDI    #4*22,  R0      ;
            MVO     R0,     @@prev  ; Make last row "previous" row

            MVI     NXTGEN, R1      ; Get pointer to next generation
            MVO     R1,     @@outp  ; Make that the output pointer


            ; A B C
            ; D E F
            ; G H I

            ; ------------------------------------------------------------- ;
            ;  The row loop iterates 24 times, once for each row's worth    ;
            ;  of output.  We tally up neighbor counts on a per-row basis   ;
            ;  and then generate output for that row.                       ;
            ; ------------------------------------------------------------- ;
            MVII    #24,    R5
@@row_loop:
            PSHR    R5

            ; ------------------------------------------------------------- ;
            ;  Tally neighbors vertically.  No side-to-side summing is      ;
            ;  done here.   This loop generates 32 2-bit sums in a packed   ;
            ;  manner, and expands them to 8-bit sums.                      ;
            ; ------------------------------------------------------------- ;
            MVII    #4,     R2
            MVII    #ROWB,  R5
            MVII    #$0303, R0          ;       Mask for pairs of sums
@@clp1: 
            ; ------------------------------------------------------------- ;
            ;  Get 8 cells from prev row and use that to start the sum.     ;
            ; ------------------------------------------------------------- ;
            MVI     @@prev, R4          ;  10   Get prev ptr
            MVI@    R4,     R3          ;   8   Read 8 bits
            MVO     R4,     @@prev      ;  11   Save updated ptr
            ADDI    #EXP,   R3          ;   8   Index into EXP table
            MVI@    R3,     R1          ;   8   Expand to 2-bit fields
                                        ;----
                                        ;  45

            ; ------------------------------------------------------------- ;
            ;  Get 8 cells from curr row and add that to the sum.           ;
            ; ------------------------------------------------------------- ;
            MVI     @@curr, R4          ;  10   Get curr ptr
            MVI@    R4,     R3          ;   8   Read 8 bits
            MVO     R4,     @@curr      ;  11   Save updated ptr
            ADDI    #EXP,   R3          ;   8   Index into EXP table
            ADD@    R3,     R1          ;   8   Accumulate 2-bit sums
                                        ;----
                                        ;  45

            ; ------------------------------------------------------------- ;
            ;  Get 8 cells from next row and add that to the sum.           ;
            ; ------------------------------------------------------------- ;
            MVI     @@next, R4          ;  10   Get next ptr
            MVI@    R4,     R3          ;   8   Read 8 bits
            MVO     R4,     @@next      ;  11   Save updated ptr
            ADDI    #EXP,   R3          ;   8   Index into EXP table
            ADD@    R3,     R1          ;   8   Accumulate 2-bit sums
                                        ;----
                                        ;  45

            MOVR    R5,     R4          ;   6   R4 used for sums 0..3
            ADDI    #4,     R5          ;   8   R5 used for sums 4..7
                                        ;----
                                        ;  14

            MOVR    R1,     R3          ;   6   \__ Mask out sums 4 and 0
            ANDR    R0,     R3          ;   6   /
            MVO@    R3,     R4          ;   9   Write out sum 0
            SWAP    R3,     1           ;   6
            MVO@    R3,     R5          ;   9   Write out sum 4
            SLR     R1,     2           ;   8   Advance to next 2 sums
                                        ;----
                                        ;  44

            MOVR    R1,     R3          ;   6   \__ Mask out sums 5 and 1
            ANDR    R0,     R3          ;   6   /
            MVO@    R3,     R4          ;   9   Write out sum 1
            SWAP    R3,     1           ;   6
            MVO@    R3,     R5          ;   9   Write out sum 5
            SLR     R1,     2           ;   8   Advance to next 2 sums
                                        ;----
                                        ;  44

            MOVR    R1,     R3          ;   6   \__ Mask out sums 6 and 2
            ANDR    R0,     R3          ;   6   /
            MVO@    R3,     R4          ;   9   Write out sum 2
            SWAP    R3,     1           ;   6
            MVO@    R3,     R5          ;   9   Write out sum 6
            SLR     R1,     2           ;   8   Advance to next 2 sums
                                        ;----
                                        ;  44

            ANDR    R0,     R1          ;   6   Mask out sums 7 and 3
            MVO@    R1,     R4          ;   9   Write out sum 3
            SWAP    R1,     1           ;   6
            MVO@    R1,     R5          ;   9   Write out sum 7
                                        ;----
                                        ;  30

                                        ;====
                                        ; 311 = 45*3 + (36*3+30) + 14

            DECR    R2                  ;   6
            BNEQ    @@clp1              ; 9/7
                                        ;----
                                        ;1304 total

            MVI     @@curr, R0          ; \
            SUBI    #4,     R0          ;  |-- Rewind current pointer
            MVO     R0,     @@curr      ; /

            ; ------------------------------------------------------------- ;
            ;  The neighbor count for a given cell is the sum of the vert.  ;
            ;  sums for the previous column, current column, and next       ;
            ;  column.  (Note that this includes the cell itself in the     ;
            ;  count.  We adjust for this later.)                           ;
            ;                                                               ;
            ;  In this loop, we compute the horizontal sums by running a    ;
            ;  3-entry "sliding window" over the row buffer.  We start off  ;
            ;  with R0 and R1 holding the "prev" and "curr" relative to     ;
            ;  our output, and the loop starts by reading "next" into R2.   ;
            ;  The sum of these three is written to the output.             ;
            ;                                                               ;
            ;  The loop proceeds to allow "prev", "curr" and "next"         ;
            ;  rotate through R0, R1, and R2.  The loop is unrolled 3x so   ;
            ;  that we never explicitly move values around.  We peel off    ;
            ;  two loop iterations because 32 MOD 3 == 2.                   ;
            ;                                                               ;
            ;  Notice how "prev" for the first pixel is the last pixel,     ;
            ;  and "next" for the last pixel is the first pixel.  Since     ;
            ;  we overwrite pixels as we go, we need to save the first      ;
            ;  pixels initial value on the stack prior to the loop.         ;
            ; ------------------------------------------------------------- ;

            MVI     ROWB+31,  R0        ; Read last pixel as 'prev'
            MVII    #ROWB,    R4        ; Point to first pixel (ROWB[0])
            MOVR    R4,       R5        ; R5 is output pointer.
            MVI@    R4,       R1        ; Read first pixel into 'curr'
            PSHR    R1                  ; will ROWB[0] for last pixel
            MVII    #10,      R3        ; Iterate 10 times (30 pixels)
@@clp2: 
            ; prev=R0 curr=R1 next=R2
            MVI@    R4,     R2          ;   8
            ADDR    R1,     R0          ;   6
            ADDR    R2,     R0          ;   6
            MVO@    R0,     R5          ;   9
                                        ;----
                                        ;  29

            ; prev=R1 curr=R2 next=R0
            MVI@    R4,     R0          ;   8
            ADDR    R2,     R1          ;   6
            ADDR    R0,     R1          ;   6
            MVO@    R1,     R5          ;   9
                                        ;----
                                        ;  29

            ; prev=R2 curr=R0 next=R1
            MVI@    R4,     R1          ;   8
            ADDR    R0,     R2          ;   6
            ADDR    R1,     R2          ;   6
            MVO@    R2,     R5          ;   9
                                        ;----
                                        ;  29

            DECR    R3                  ;   6
            BNEQ    @@clp2              ; 9/7
                                        ;----
                                        ;1018 = 10 * (29*3 + 15) - 2

            ; do 2nd to last pixel.
            ; prev=R0 curr=R1 next=R2
            MVI@    R4,     R2          ;   8
            ADDR    R1,     R0          ;   6
            ADDR    R2,     R0          ;   6
            MVO@    R0,     R5          ;   9

            ; do last pixel on row.  
            ; prev=R1 curr=R2 next=@SP
            ADDR    R2,     R1          ;
            ADD@    SP,     R1          ; add w/ first pixel on row
            MVO@    R1,     R5          ;


            ; ------------------------------------------------------------- ;
            ;  Now, make live/die decision for each pixel in row.           ;
            ;                                                               ;
            ;  We do this by taking the "neighbor + self" count for each    ;
            ;  cell, and merging it with the cell's live/dead status.       ;
            ;  We then look this up in a lookup-table to decide whether     ;
            ;  the cell lives or dies.                                      ;
            ;                                                               ;
            ;  Because each row only has 32 cells, we store the entire row  ;
            ;  bitmap in two registers.  Also, because we produce the new   ;
            ;  generation as we consume the old, we can fill in the new     ;
            ;  generation in the same two registers as we go.               ;
            ;                                                               ;
            ;  NOTE:  Leftmost cell on screen is in bit 0.  That is, the    ;
            ;  bitmap is the mirror image horizontally of what displayed    ;
            ;  on the screen.                                               ;
            ;                                                               ;
            ;  This loop is unrolled 2x for speed.                          ;
            ;                                                               ;
            ;  Optimization note:  The CLRC doesn't need to be inside the   ;
            ;  loop because none of our operations in the loop will set C.  ;
            ; ------------------------------------------------------------- ;
            MVI     @@curr, R4
            SDBD
            MVI@    R4,     R0
            SDBD
            MVI@    R4,     R1

            MVII    #LDTBL, R5
            MVII    #ROWB,  R4
            MVII    #16,    R2
            CLRC                        ; clear out the carry
@@clp3:
            RRC     R1,     1           ; \__ Advance one cell. 0 to R1's MSB,
            RRC     R0,     1           ; /   leftmost cell to Carry bit
            MVI@    R4,     R3          ; Get neighbor count (includes self)
            RLC     R3,     1           ; LSB of index contains curr cell.
            ADDR    R5,     R3          ; Index into live/die table
            XOR@    R3,     R1          ; Generate new cell in MSB of R1.

            RRC     R1,     1           ; \__ Advance one cell. 0 to R1's MSB,
            RRC     R0,     1           ; /   leftmost cell to Carry bit
            MVI@    R4,     R3          ; Get neighbor count (includes self)
            RLC     R3,     1           ; LSB of index contains curr cell.
            ADDR    R5,     R3          ; Index into live/die table
            XOR@    R3,     R1          ; Generate new cell in MSB of R1.

            DECR    R2
            BNEQ    @@clp3

            ; ------------------------------------------------------------- ;
            ;  Write out the 32 new cells.                                  ;
            ; ------------------------------------------------------------- ;
            MVI     @@outp, R4
            MVO@    R0,     R4
            SWAP    R0
            MVO@    R0,     R4
            NOP
            MVO@    R1,     R4
            SWAP    R1
            MVO@    R1,     R4
            MVO     R4,     @@outp

            ; ------------------------------------------------------------- ;
            ;  Move down a row.  On the last row, make the "next" pointer   ;
            ;  point to the topmost row of the current generation.          ;
            ; ------------------------------------------------------------- ;
            PULR    R5

            MVI     @@curr, R0          ; Get current pointer
            MVO     R0,     @@prev      ; ... and make it the new "previous"

            ADDI    #4,     R0          ; Move current down one row
            MVO     R0,     @@curr      ; ... and make it the new "current"

            ADDI    #4,     R0          ; Move current down one more row
            CMPI    #2,     R5          ; Are we starting last iteration?
            BNEQ    @@not_last          ; No:  Then make current+2 new "next"
            MVI     CURGEN, R0          ; Yes: Make top of generation "next"

@@not_last  MVO     R0,     @@next

            DECR    R5                  ; Do next iteration if there is one.
            BNEQ    @@row_loop

            PULR    PC
            ENDP


;; ======================================================================== ;;
;;  PMASK -- Pixel masks for each of the four colored square pixels.        ;;
;; ======================================================================== ;;
PMASK       PROC
@@0         EQU     $0007 
@@1         EQU     $0038
@@2         EQU     $01C0
@@3         EQU     $2600
            ENDP
           
;; ======================================================================== ;;
;;  COLORS -- Color masks for the four pixels.  Includes "color-squares"    ;;
;;            bit set in bit 12.                                            ;;
;; ======================================================================== ;;
COLORS:     PROC
            DECLE   $0000 + $1000
            DECLE   $0249 + $1000
            DECLE   $0492 + $1000
            DECLE   $06DB + $1000
            DECLE   $2124 + $1000
            DECLE   $236D + $1000
            DECLE   $25B6 + $1000
            DECLE   $27FF + $1000
            ENDP

;; ======================================================================== ;;
;;  CSQDECODE -- Decodes the "interleaved bitmap" we generate into a        ;;
;;               colored-squares pixel AND-mask.  AND this with a color to  ;;
;;               generate the inforation you need for BACKTAB.              ;;
;;                                                                          ;;
;;               Pixel order in table index:  1 3 0 2.                      ;;
;; ======================================================================== ;;
CSQDECODE   PROC
@@0000      DECLE   $1000 + 0          
@@0001      DECLE   $1000 +                               PMASK.2
@@0010      DECLE   $1000 +           PMASK.0           
@@0011      DECLE   $1000 +           PMASK.0           + PMASK.2
@@0100      DECLE   $1000 +                     PMASK.3 
@@0101      DECLE   $1000 +                     PMASK.3 + PMASK.2
@@0110      DECLE   $1000 +           PMASK.0 + PMASK.3 
@@0111      DECLE   $1000 +           PMASK.0 + PMASK.3 + PMASK.2
@@1000      DECLE   $1000 + PMASK.1    
@@1001      DECLE   $1000 + PMASK.1                     + PMASK.2
@@1010      DECLE   $1000 + PMASK.1 + PMASK.0           
@@1011      DECLE   $1000 + PMASK.1 + PMASK.0           + PMASK.2
@@1100      DECLE   $1000 + PMASK.1           + PMASK.3 
@@1101      DECLE   $1000 + PMASK.1           + PMASK.3 + PMASK.2
@@1110      DECLE   $1000 + PMASK.1 + PMASK.0 + PMASK.3 
@@1111      DECLE   $1000 + PMASK.1 + PMASK.0 + PMASK.3 + PMASK.2
            ENDP


;; ======================================================================== ;;
;;  SHOWLIFE                                                                ;;
;;  Go through the current generation's bitmap and display using colored-   ;;
;;  squares mode.                                                           ;;
;; ======================================================================== ;;
SHOWLIFE    PROC
            PSHR    R5

            MVI     CURGEN, R4      ; Get pointer to current generation
            MVII    #$202,  R5      ; Point to top-left 2x2 block
            MVI     GENCOL, R1      ; Get color for this generation

            ; ------------------------------------------------------------- ;
            ;  ROW_LP generates two rows of pixels each iteration.          ;
            ; ------------------------------------------------------------- ;
@@row_lp:
            MVII    #4,     R0      ; Iterate 4 times per row.

            ; ------------------------------------------------------------- ;
            ;  COL_LP outputs four  rows of pixels each iteration.          ;
            ; ------------------------------------------------------------- ;
@@col_lp:   MVI@    R4,     R3
            ADDI    #EXP,   R3
            MVI@    R3,     R3
            SLL     R3,     1

            ADDI    #3,     R4
            MVI@    R4,     R2
            SUBI    #4,     R4
            ADDI    #EXP,   R2
            ADD@    R2,     R3

            ; ------------------------------------------------------------- ;
            ;  At this point, R3 contains 16 pixels, 8 pixels in 2 rows.    ;
            ;  Let's display them.  Recall LSBs are leftmost pixels.        ;
            ; ------------------------------------------------------------- ;

            MOVR    R3,     R2      ; Copy pixels to R2
            ANDI    #$F,    R2      ; Get first 2x2 block
            ADDI    #CSQDECODE, R2  ;\__ Get proper 2x2 mask
            MVI@    R2,     R2      ;/
            ANDR    R1,     R2      ; Convert to desired color
            MVO@    R2,     R5      ; Store to display

            SLR     R3,     2       ;\__ Shift to next 2x2 pixel set
            SLR     R3,     2       ;/

            MOVR    R3,     R2      ; Copy pixels to R2
            ANDI    #$F,    R2      ; Get 2nd 2x2 block
            ADDI    #CSQDECODE, R2  ;\__ Get proper 2x2 mask
            MVI@    R2,     R2      ;/
            ANDR    R1,     R2      ; Convert to desired color
            MVO@    R2,     R5      ; Store to display

            SLR     R3,     2       ;\__ Shift to next 2x2 pixel set
            SLR     R3,     2       ;/

            MOVR    R3,     R2      ; Copy pixels to R2
            ANDI    #$F,    R2      ; Get 3rd 2x2 block
            ADDI    #CSQDECODE, R2  ;\__ Get proper 2x2 mask
            MVI@    R2,     R2      ;/
            ANDR    R1,     R2      ; Convert to desired color
            MVO@    R2,     R5      ; Store to display

            SLR     R3,     2       ;\__ Shift to next 2x2 pixel set
            SLR     R3,     2       ;/

            ADDI    #CSQDECODE, R3  ;\__ Get proper 2x2 mask
            MVI@    R3,     R2      ;/
            ANDR    R1,     R2      ; Convert to desired color
            MVO@    R2,     R5      ; Store to display

            DECR    R0
            BNEQ    @@col_lp

            ADDI    #4,     R4      ; advance one row on input
            ADDI    #4,     R5      ; skip border stuff on output
            CMPI    #$2EE,  R5      ; At end of screen yet?
            BLT     @@row_lp

            PULR    PC
            ENDP


;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
            INCLUDE "../library/print.asm"       ; PRINT.xxx routines
            INCLUDE "../library/fillmem.asm"     ; CLRSCR/FILLZERO/FILLMEM
            INCLUDE "../library/rand.asm"        ; RAND
            INCLUDE "../library/hexdisp.asm"     ; HEX8

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
