;;==========================================================================;;
;; Spinning World with Intellicart Bankswitching                            ;;
;; Copyright 2002, Joe Zbiciak.                                             ;;
;;                                                                          ;;
;; This demonstration combines the concepts of bankswitching with the       ;;
;; spinning-world demo.  The frames of the spinning world are stored in     ;;
;; the lower part of the Intellicart's address space and are accessed       ;;
;; via a small bankswitched window.                                         ;;
;;                                                                          ;;
;; You may wish to compare this demo to the "world" example as well.        ;;
;; The two are nearly identical, except that this one reads the picture     ;;
;; data via bankswitched window.  This one also is set to build as a 16-bit ;;
;; ROM image (I was feeling lazy).                                          ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  TO BUILD IN BIN+CFG FORMAT:                                             *;
;*      as1600 -o bankworld.bin -l bankworld.lst bankworld.asm              *;
;*                                                                          *;
;*  TO BUILD IN ROM FORMAT:                                                 *;
;*      as1600 -o bankworld.rom -l bankworld.lst bankworld.asm              *;
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
;*                   Copyright (c) 2001, Joseph Zbiciak                     *;
;* ======================================================================== *;

        ROMW    16

; Magic memory locations
VBLANK  EQU     $20             ; Vertical-blank Handshake
COLSTK  EQU     $21             ; Color-stack/FGBG switch
CS0     EQU     $28             ; Color Stack 0
CS1     EQU     $29             ; Color Stack 1
CS2     EQU     $2A             ; Color Stack 2
CS3     EQU     $2B             ; Color Stack 3
CB      EQU     $2C             ; Color for border
ISRVEC  EQU     $100            ; ISR jump vector
STATE   EQU     $110            ; State word for animation
FRAME   EQU     $111            ; Next frame number * 2
GROFS   EQU     $112            ; GRAM offset
RATE    EQU     $6              ; Animation rate in ticks/frame
GRAM    EQU     $3800           ; GRAM address

;; ======================================================================== ;;
;;  BANKSWITCHED WINDOW                                                     ;;
;;                                                                          ;;
;;  This gives us a bankswitched window through which we can see the        ;;
;;  picture data for the spinning globe.  We will be using the bankswitched ;;
;;  range as a "sliding window", moving the window by 256 word incremenets  ;;
;;  as needed.                                                              ;;
;;                                                                          ;;
;;  The bankswitched window is mapped at $5000 - $5FFF.                     ;;
;; ======================================================================== ;;
WINDOW  ORG     $5800, $5800, "+RB"
        RMB     $0800
WBANK   EQU     $00FF AND (WINDOW SHR 8)


        ORG     $5000
;------------------------------------------------------------------------------
; EXEC-friendly ROM header.
;------------------------------------------------------------------------------
ROMHDR: BIDECLE ZERO            ; MOB picture base   (points to NULL list)
        BIDECLE ZERO            ; Process table      (points to NULL list)
        BIDECLE 0               ; Program start address (unused)
        BIDECLE ZERO            ; Bkgnd picture base (points to NULL list)
        BIDECLE ONES            ; GRAM pictures      (points to NULL list)
        BIDECLE TITLE           ; Cartridge title/date
        DECLE   $03C0           ; No ECS title, run code after title,
                                ; ... no clicks
ZERO:   DECLE   $0000           ; Screen border control
        DECLE   $0000           ; 0 = color stack, 1 = f/b mode
        DECLE   0, 0, 0, 0, 0   ; Initial color stack / border color: Black
ONES:   DECLE   1
;------------------------------------------------------------------------------


TITLE:  BYTE    102, "Bank-switched World", 0
INIT:   PROC
        DIS
        ; Intercept EXEC initialization and just
        ; do our own.  We use no EXEC routines.

        ; Zero most of memory
        MVII    #$100,  R4      
        MVII    #$260,  R1      ; $100...$35F. 
        CALL    FILLZERO        

        
        ; Draw a 6x6 tiled area in the middle of the screen in white.
        MVII    #$807,  R0      ; White, with GRAM bit set.
        CALL    MAKETILE

        MOVR    PC,     R0      ; (generate PC-relative address)
        ADDI    #ISR -$, R0     ; Our demo code.
        MVII    #ISRVEC, R4     ; ISR vector
        MVO@    R0,     R4      ; Write low half
        SWAP    R0              ; 
        MVO@    R0,     R4      ; Write high half

        EIS

@@spin: DECR    PC              ; spin on this instr.

        ENDP

;; ======================================================================== ;;
;;  MAKETILE                                                                ;;
;;  Makes a 6x6 tile on the screen with increasing GRAM index numbers.      ;;
;;                                                                          ;;
;;  INPUTS                                                                  ;;
;;      R0  Starting attribute word                                         ;;
;;      R5  Return address                                                  ;;
;;                                                                          ;;
;;  OUTPUTS                                                                 ;;
;;      R0, R1, R2, R4 -- Trashed                                           ;;
;;                                                                          ;;
;; ======================================================================== ;;

MAKETILE PROC
        MVII    #$200 + 3*20 + 7, R4    ; Centered on the screen
        MVII    #6,     R2      ; 6 rows in tiled area
@@rowlp:
        MVII    #6,     R1      ; 6 columns in tiled area
@@collp:
        MVO@    R0,     R4      ; Write out a word of tile
        ADDI    #8,     R0      ; Point to next GRAM card
        DECR    R1              ; Loop over columns
        BNEQ    @@collp

        ADDI    #14,    R4      ; Skip to next row
        DECR    R2              ; Loop over rows
        BNEQ    @@rowlp

        JR      R5
        ENDP

;; ======================================================================== ;;
;;  ISR -- Our main demo code.                                              ;;
;;                                                                          ;;
;;  This ISR forms an 16 state state-machine.  The even numbered states     ;;
;;  do no work -- they exist for timing purposes only.  The odd numbered    ;;
;;  states copy chunks of data to the GRAM for updating the animation.      ;;
;;                                                                          ;;
;;  The onscreen animation consists of a 6x6 tile of cards which display    ;;
;;  36 unique GRAM definitions onscreen.  This gives us a 48x48 pixel area  ;;
;;  for the animation.  We update the animation by copying new GRAM         ;;
;;  definitions to memory during the vertical blanking interval.            ;;
;;                                                                          ;;
;;  If we had sufficient bandwidth to the GRAM, we could update the entire  ;;
;;  GRAM definition during one VBLANK, and could get away with using only   ;;
;;  36 card slots for our animation and leave it at that.  Unfortunately,   ;;
;;  this isn't the case.                                                    ;;
;;                                                                          ;;
;;  Instead, we break our GRAM updates into strips of 16 cards, and upload  ;;
;;  those strips one at a time to an unused portion of GRAM.                ;;
;; ======================================================================== ;;

ISR:    PROC

        ; Set color stack/border to black.
        CLRR    R0
        MVO     R0,     CS0
        MVO     R0,     CS1
        MVO     R0,     CS2
        MVO     R0,     CS3
        MVO     R0,     CB

        MVI     STATE,  R0      ; Get our current state word.
        INCR    R0
        ANDI    #15,    R0      ; We have 16 states.
        MVO     R0,     STATE   ; Save updated state.

        SARC    R0,     1       ; LSB says whether we do something this state
        BC      @@do_work
@@do_nothing:
        MVO     R0,     VBLANK  ; Just hit STIC enable otherwise.
        JR      R5

@@do_work:
        ANDI    #3,     R0      ; R1 will be our index into the frame.

        CLRR    R3              ; Default to not needing new tiles.

        CMPI    #3,     R0      ; If it's exactly 3, prepare for new tiles
        BNEQ    @@not_new
        INCR    R3              ; Set flag saying we need new tiles.
@@not_new:
        
        PSHR R5                 ; At this point we know to save ret addr.

        ; Turn our frame index into an actual byte offset into the frame.
        ; R0 = (R0 * 9) << 3 = (R0 << 6) + (R0 << 3)
        MOVR    R0,     R1
        SWAP    R0,     1
        SLR     R0,     2 
        SLL     R1,     2
        ADDR    R1,     R0
        ADDR    R1,     R0      ; R0 = (R0 * 9) << 3

        ; Copy 16 card definitions to GRAM.
@@do_strip:
        MVI     FRAME,  R4      ; Get our current frame #
        TSTR    R0              ; Is our offset into the frame 0?
        BNEQ    @@sameframe     ; No... same frame as last interrupt.
        INCR    R4              ; Yes... advance to next frame.
        CMPI    #30,    R4      ; Did we go past last frame?
        BLT     @@nowrap        ; No... do nothing.
        CLRR    R4              ; Yes... go back to frame 0.
@@nowrap:
        MVO     R4,     FRAME   ; Store new frame number.
@@sameframe:
        ADDI    #FRAMES,R4      ; Point to frame index table for WORLD
        MVI@    R4,     R5      ; Get frame address into R5
        ADDR    R0,     R5      ; Add offset into the frame.

        ; The pointer in R5 now points to the address of our frame in
        ; Intellicart address space.  We take the upper half of that and 
        ; use it as our Intellicart bank selector.  This will point our
        ; window at the desired data within a 256-word granularity.
        ; We take the lower half of that and add to our bank-switched 
        ; window address to generate our final pointer.  Got that?

        PSHR    R5
        MOVR    R5,     R2      ;\
        SWAP    R2              ; |__ R2 has addr. we want window to 
        ANDI    #$FF,   R2      ; |   point to.  R1 has addr of window.
        MVII    #WBANK, R1      ;/
        CALL    IC_SETBANK      ; Set the bank selector for our window

        PULR    R5
        ANDI    #$00FF, R5      ; Now get offset portion of address
        ADDI    #WINDOW, R5     ; ... and point into our bankswitched window.

        ; Get our GRAM offset and add 9 cards.  Add offset to GRAM base.
        MVI     GROFS,  R4
        ADDI    #$1C24, R4
        MVO     R4,     GROFS   ; Note that the high byte is not stored.
        ADDR    R4,     R4
        MVII    #9,     R2      ; Copy 9 cards to GRAM
@@copylp:
        MVI@    R5,     R1      ; Copy one complete card image to GRAM
        MVO@    R1,     R4
        MVI@    R5,     R1
        MVO@    R1,     R4
        MVI@    R5,     R1
        MVO@    R1,     R4
        MVI@    R5,     R1
        MVO@    R1,     R4
        MVI@    R5,     R1
        MVO@    R1,     R4
        MVI@    R5,     R1
        MVO@    R1,     R4
        MVI@    R5,     R1
        MVO@    R1,     R4
        MVI@    R5,     R1
        MVO@    R1,     R4
        DECR    R2              ; Iterate
        BNEQ    @@copylp

        MVO     R2,     VBLANK  ; Hit the STIC video enable.
        
        TSTR    R3              ; Do we need new tiles?
        BEQ     @@return        ; R3 == 0 means no we don't


        SUBI    #$0120, R4      ; Figure out the start of the GRAM 
                                ; region that we just finished uploading
        ANDI    #$01FF, R4
        MOVR    R4,     R0
        XORI    #$807,  R0      ; Mask in color. (GRAM bit left set by SUBI)
        PULR    R5              ; Chain the return.
        B       MAKETILE        ; Make the tile array

@@return
        PULR    PC              
        ENDP


;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
        INCLUDE "../library/fillmem.asm"    ; for FILLZERO/FILLMEM/CLRSCR
        INCLUDE "../library/ic_banksw.asm"  ; Bankswitch code

;; ======================================================================== ;;
;;  FRAMES -- Indices of each of the frames of the WORLD.                   ;;
;;  This is separate from "WORLD" because we want it always available.      ;;
;; ======================================================================== ;;
FRAMES  PROC
        DECLE   WORLD.frame_00
        DECLE   WORLD.frame_01
        DECLE   WORLD.frame_02
        DECLE   WORLD.frame_03
        DECLE   WORLD.frame_04
        DECLE   WORLD.frame_05
        DECLE   WORLD.frame_06
        DECLE   WORLD.frame_07
        DECLE   WORLD.frame_08
        DECLE   WORLD.frame_09
        DECLE   WORLD.frame_10
        DECLE   WORLD.frame_11
        DECLE   WORLD.frame_12
        DECLE   WORLD.frame_13
        DECLE   WORLD.frame_14
        DECLE   WORLD.frame_15
        DECLE   WORLD.frame_16
        DECLE   WORLD.frame_17
        DECLE   WORLD.frame_18
        DECLE   WORLD.frame_19
        DECLE   WORLD.frame_20
        DECLE   WORLD.frame_21
        DECLE   WORLD.frame_22
        DECLE   WORLD.frame_23
        DECLE   WORLD.frame_24
        DECLE   WORLD.frame_25
        DECLE   WORLD.frame_26
        DECLE   WORLD.frame_27
        DECLE   WORLD.frame_28
        DECLE   WORLD.frame_29
    ENDP

;; ======================================================================== ;;
;;  WORLD -- Pictures of a rotating planet.                                 ;;
;;                                                                          ;;
;;  We set this to load into the Intellicart at $0000.  Since we're         ;;
;;  interested only in the Intellicart addresses for the world's frames,    ;;
;;  we set both addresses in the ORG statement to $0000.  We set the        ;;
;;  memory attributes to "-RWBN" to prevent these from getting mapped in    ;;
;;  the Intellivision address space.                                        ;;
;; ======================================================================== ;;

        ORG     $0000,  $0000,  "-RWBN"
WORLD   PROC
@@frame_00:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $01
        BYTE    $00, $01, $0F, $1E, $7D, $78, $FC, $F0
        BYTE    $FD, $FF, $FD, $E0, $84, $00, $00, $00
        BYTE    $FF, $FF, $FF, $77, $C1, $00, $00, $00
        BYTE    $00, $80, $F0, $F8, $FE, $7F, $1F, $1F
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $03, $07, $0F, $0C, $18, $18, $30, $30
        BYTE    $60, $C0, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $08, $08
        BYTE    $0F, $07, $03, $01, $00, $00, $00, $00
        BYTE    $E0, $E0, $F0, $F0, $F8, $78, $1C, $1C
        BYTE    $70, $71, $70, $70, $60, $40, $70, $7C
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $0E, $06, $02, $02, $02, $02, $02, $02
        BYTE    $7F, $6F, $77, $4E, $5F, $1F, $3F, $3F
        BYTE    $C0, $20, $B8, $80, $00, $02, $C2, $C2
        BYTE    $00, $00, $02, $00, $02, $30, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $02, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $02, $02, $00, $02, $02, $00, $04, $04
        BYTE    $1F, $1F, $0F, $07, $04, $02, $00, $01
        BYTE    $E0, $F0, $F0, $F0, $F0, $30, $18, $00
        BYTE    $00, $00, $00, $00, $70, $30, $70, $60
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $08, $10, $00, $20, $40, $80, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $80, $20, $08, $02, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $FC, $7F, $00, $00
        BYTE    $00, $00, $00, $00, $7F, $F8, $00, $00
        BYTE    $01, $06, $10, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_01:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $03
        BYTE    $00, $01, $0F, $1F, $7F, $7E, $FF, $F6
        BYTE    $7F, $FF, $FF, $F8, $30, $00, $00, $00
        BYTE    $FF, $FF, $7F, $1D, $90, $00, $00, $00
        BYTE    $00, $80, $F0, $F8, $3E, $1F, $07, $03
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $03, $07, $0F, $0E, $1E, $1E, $38, $34
        BYTE    $D8, $F8, $40, $00, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $03, $01, $00, $00, $00, $00, $80, $40
        BYTE    $E0, $E0, $70, $30, $38, $18, $0C, $04
        BYTE    $6C, $6C, $6C, $6C, $5C, $70, $7C, $7F
        BYTE    $10, $00, $00, $00, $00, $00, $00, $C0
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $06, $00, $02, $00, $02, $02, $00, $02
        BYTE    $7F, $7A, $5C, $43, $43, $07, $27, $2F
        BYTE    $FC, $F2, $F9, $A8, $B0, $F0, $F8, $FC
        BYTE    $00, $00, $80, $00, $00, $21, $00, $30
        BYTE    $00, $00, $10, $00, $10, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $02, $00, $02, $00, $02, $04, $00, $04
        BYTE    $0F, $17, $17, $0B, $05, $02, $00, $01
        BYTE    $FE, $FE, $FE, $FE, $1E, $0E, $02, $00
        BYTE    $00, $00, $00, $00, $06, $03, $06, $0C
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $08, $10, $10, $00, $20, $40, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $80, $20, $08, $02, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $FE, $7F, $00, $00
        BYTE    $00, $00, $00, $00, $3F, $F0, $00, $00
        BYTE    $01, $06, $10, $60, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_02:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $03
        BYTE    $00, $01, $0F, $1F, $7F, $7F, $FF, $FE
        BYTE    $7F, $FF, $FF, $FE, $E6, $C8, $E0, $C0
        BYTE    $FF, $FF, $DF, $07, $12, $00, $00, $00
        BYTE    $00, $80, $F0, $78, $1E, $07, $03, $01
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $03, $07, $0F, $0F, $1F, $1F, $3E, $3C
        BYTE    $FB, $FF, $CC, $C0, $A0, $80, $80, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $18, $00
        BYTE    $E0, $60, $30, $10, $18, $08, $04, $04
        BYTE    $79, $39, $79, $73, $66, $76, $7F, $3F
        BYTE    $00, $81, $80, $80, $80, $00, $E0, $38
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $02, $00, $02, $02, $02, $00, $02
        BYTE    $5F, $5E, $0F, $40, $40, $00, $21, $23
        BYTE    $FF, $1F, $8F, $32, $7B, $FF, $FF, $FF
        BYTE    $40, $F0, $8C, $40, $00, $01, $80, $C3
        BYTE    $00, $00, $01, $00, $01, $08, $10, $00
        BYTE    $00, $00, $00, $00, $00, $02, $00, $00
        BYTE    $02, $00, $02, $00, $02, $04, $00, $04
        BYTE    $03, $11, $08, $00, $04, $02, $00, $01
        BYTE    $FF, $FF, $FF, $FF, $47, $01, $00, $00
        BYTE    $E0, $E0, $E0, $E0, $E0, $C0, $40, $01
        BYTE    $00, $00, $00, $00, $60, $30, $E0, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $08, $00, $08, $10, $20, $20, $40, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $40, $20, $08, $04, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $7F, $00, $00
        BYTE    $00, $00, $00, $00, $9D, $F2, $00, $00
        BYTE    $01, $04, $18, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_03:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $03
        BYTE    $00, $01, $0F, $1F, $7F, $7F, $FF, $FF
        BYTE    $FF, $FF, $FF, $FD, $F8, $F9, $EC, $D8
        BYTE    $F7, $FF, $FF, $C1, $80, $00, $00, $00
        BYTE    $00, $80, $F0, $E8, $8E, $00, $01, $00
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $03, $07, $0F, $0F, $1F, $1F, $3F, $3F
        BYTE    $FF, $FF, $F9, $F9, $FA, $F0, $D0, $90
        BYTE    $30, $E0, $C0, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $01, $00
        BYTE    $40, $20, $10, $10, $08, $00, $04, $84
        BYTE    $6F, $67, $6F, $4E, $4C, $4D, $4D, $0F
        BYTE    $10, $30, $38, $18, $68, $C0, $F4, $F7
        BYTE    $00, $00, $00, $00, $00, $00, $00, $C0
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $02, $00, $02, $02, $02, $02, $00
        BYTE    $47, $43, $41, $00, $40, $40, $20, $00
        BYTE    $B7, $81, $F8, $13, $0F, $1F, $3F, $7F
        BYTE    $F6, $79, $FC, $A4, $B0, $F0, $FC, $FC
        BYTE    $00, $00, $60, $00, $08, $00, $10, $18
        BYTE    $08, $00, $00, $00, $10, $80, $00, $01
        BYTE    $02, $02, $00, $02, $02, $24, $00, $04
        BYTE    $10, $00, $10, $08, $04, $02, $01, $00
        BYTE    $7F, $3F, $3F, $1F, $08, $00, $00, $00
        BYTE    $FE, $FE, $FF, $FE, $7E, $1C, $0C, $00
        BYTE    $00, $00, $00, $00, $06, $06, $1C, $18
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $08, $08, $10, $00, $20, $20, $40, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $C0, $00, $10, $04, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $7F, $00, $00
        BYTE    $00, $00, $00, $00, $C5, $FA, $00, $00
        BYTE    $01, $06, $10, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_04:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $03
        BYTE    $00, $01, $0F, $1F, $7F, $7F, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FB
        BYTE    $FB, $FF, $FD, $F8, $30, $00, $40, $80
        BYTE    $00, $80, $E0, $F8, $24, $02, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $80
        BYTE    $03, $07, $0F, $0F, $1F, $1F, $3F, $3D
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FD, $F1
        BYTE    $F3, $BE, $9C, $80, $20, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $40, $20, $00, $10, $00, $08, $64, $14
        BYTE    $39, $50, $51, $11, $53, $41, $41, $00
        BYTE    $E3, $F1, $F3, $C1, $86, $9C, $FF, $DF
        BYTE    $00, $80, $80, $80, $80, $00, $D0, $7E
        BYTE    $00, $80, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $02, $00, $02, $02, $02, $00, $02
        BYTE    $40, $40, $40, $00, $40, $40, $20, $20
        BYTE    $FF, $7A, $1F, $00, $00, $00, $03, $07
        BYTE    $7F, $07, $C7, $39, $F9, $FF, $FF, $FF
        BYTE    $B0, $C4, $E2, $20, $80, $80, $C0, $E1
        BYTE    $00, $00, $01, $00, $01, $88, $08, $80
        BYTE    $02, $00, $02, $02, $02, $0C, $00, $04
        BYTE    $00, $10, $08, $00, $04, $02, $01, $00
        BYTE    $07, $07, $03, $03, $01, $00, $00, $80
        BYTE    $FF, $FF, $FF, $FF, $07, $01, $00, $00
        BYTE    $E0, $F0, $F0, $E0, $E0, $C0, $C0, $02
        BYTE    $00, $00, $00, $00, $40, $60, $80, $00
        BYTE    $08, $08, $10, $10, $20, $20, $40, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $40, $20, $10, $04, $03, $00, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $7F, $00, $00
        BYTE    $00, $00, $00, $00, $F1, $FE, $00, $00
        BYTE    $01, $06, $10, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_05:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $03
        BYTE    $00, $01, $0F, $1F, $7F, $7F, $FF, $FF
        BYTE    $7F, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $FE, $E6, $E0, $EC, $A0
        BYTE    $00, $80, $F0, $38, $04, $02, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $80
        BYTE    $03, $07, $0F, $0F, $1F, $1F, $37, $27
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $3F
        BYTE    $FE, $FB, $F9, $F8, $F2, $F8, $D0, $18
        BYTE    $20, $E0, $80, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $40, $20, $00, $10, $00, $08, $14, $08
        BYTE    $26, $46, $46, $46, $46, $40, $00, $40
        BYTE    $3E, $1F, $16, $14, $38, $38, $1D, $1D
        BYTE    $18, $18, $2C, $2C, $6C, $E0, $F6, $D3
        BYTE    $00, $04, $00, $00, $00, $00, $80, $E0
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $02, $00, $02, $02, $02, $02, $02, $02
        BYTE    $40, $40, $00, $40, $40, $20, $20, $20
        BYTE    $0F, $07, $01, $00, $00, $00, $00, $00
        BYTE    $FF, $90, $FD, $09, $07, $0F, $3F, $7F
        BYTE    $FD, $BF, $3E, $89, $D8, $FC, $FC, $FE
        BYTE    $80, $40, $30, $10, $00, $08, $01, $18
        BYTE    $12, $02, $02, $02, $22, $84, $04, $04
        BYTE    $30, $00, $10, $08, $04, $02, $00, $01
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $7F, $7F, $3F, $3F, $10, $00, $00, $00
        BYTE    $FF, $FF, $FF, $FE, $7C, $38, $08, $00
        BYTE    $00, $00, $00, $00, $08, $08, $30, $60
        BYTE    $08, $08, $10, $10, $20, $20, $40, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $80, $60, $10, $02, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $3F, $00, $00
        BYTE    $00, $00, $00, $00, $F9, $FE, $00, $00
        BYTE    $01, $06, $10, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_06:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $03
        BYTE    $00, $01, $0F, $1F, $7F, $7F, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FB, $FF, $FF, $FF, $FC, $FC, $FB, $F6
        BYTE    $00, $80, $E0, $98, $8C, $82, $80, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $80
        BYTE    $03, $07, $0F, $0F, $1F, $1F, $3D, $38
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $F3
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FC, $E8
        BYTE    $E6, $AE, $9C, $80, $A0, $80, $80, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $40, $20, $00, $10, $00, $08, $04, $04
        BYTE    $71, $60, $60, $60, $60, $40, $40, $40
        BYTE    $E3, $C1, $C1, $C1, $43, $03, $01, $01
        BYTE    $F0, $F0, $71, $A2, $C2, $CE, $DF, $FF
        BYTE    $80, $C0, $E0, $60, $60, $00, $BA, $9F
        BYTE    $00, $40, $00, $00, $00, $00, $00, $00
        BYTE    $00, $02, $00, $02, $02, $02, $02, $02
        BYTE    $40, $40, $40, $40, $60, $70, $30, $30
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $FF, $3D, $1F, $00, $00, $00, $01, $07
        BYTE    $F7, $01, $E9, $0C, $3E, $7F, $FF, $FF
        BYTE    $F8, $E4, $F6, $91, $C0, $C0, $E1, $E1
        BYTE    $06, $02, $02, $02, $0A, $90, $24, $04
        BYTE    $10, $10, $10, $08, $04, $02, $00, $01
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $07, $07, $03, $03, $02, $00, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $07, $03, $01, $00
        BYTE    $F0, $F0, $E0, $E0, $C1, $83, $0E, $18
        BYTE    $04, $08, $10, $10, $00, $20, $40, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $80, $40, $18, $02, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $7F, $00, $00
        BYTE    $00, $00, $00, $00, $F9, $FA, $00, $00
        BYTE    $01, $06, $10, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_07:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $03
        BYTE    $00, $01, $0F, $1F, $7F, $7F, $FF, $FF
        BYTE    $3F, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $00, $80, $E0, $F0, $E4, $A2, $E0, $60
        BYTE    $00, $00, $00, $00, $00, $00, $00, $80
        BYTE    $03, $07, $0F, $0F, $1F, $1F, $3F, $3F
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $3F, $1F
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $3F
        BYTE    $FE, $FF, $F9, $F8, $F9, $F8, $E0, $8C
        BYTE    $40, $C0, $80, $00, $00, $10, $00, $00
        BYTE    $40, $20, $00, $10, $08, $08, $04, $04
        BYTE    $7E, $78, $7C, $78, $78, $70, $70, $60
        BYTE    $1E, $1C, $0C, $0C, $04, $00, $00, $00
        BYTE    $3F, $0F, $0B, $0A, $1C, $1E, $0E, $0F
        BYTE    $8C, $8E, $8E, $17, $3F, $70, $FF, $FF
        BYTE    $00, $04, $00, $00, $00, $00, $C0, $78
        BYTE    $00, $02, $00, $02, $02, $02, $02, $00
        BYTE    $60, $60, $60, $70, $78, $3C, $3C, $3D
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $07, $03, $00, $00, $00, $00, $00, $00
        BYTE    $FD, $E8, $7F, $00, $03, $07, $1F, $3F
        BYTE    $FE, $3E, $5E, $E9, $EC, $FC, $FC, $FE
        BYTE    $82, $40, $66, $22, $02, $1A, $04, $24
        BYTE    $14, $18, $08, $08, $00, $04, $02, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $3F, $3F, $3F, $3F, $20, $00, $00, $00
        BYTE    $FE, $FE, $FE, $FC, $78, $30, $33, $04
        BYTE    $08, $00, $08, $10, $60, $C0, $00, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $80, $40, $18, $02, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $7F, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $FC, $00, $00
        BYTE    $02, $04, $18, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_08:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $03
        BYTE    $00, $01, $0F, $1F, $7F, $7F, $FF, $FF
        BYTE    $5F, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $00, $80, $E0, $F8, $FC, $F2, $FC, $F8
        BYTE    $00, $00, $00, $00, $00, $00, $00, $80
        BYTE    $03, $07, $0F, $0F, $1F, $1F, $3F, $3F
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $F3, $E1
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $F9
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FE, $F4
        BYTE    $C8, $7C, $B0, $80, $A0, $C0, $00, $40
        BYTE    $40, $20, $00, $10, $00, $08, $04, $04
        BYTE    $7F, $7F, $7F, $7F, $7F, $7E, $7C, $7C
        BYTE    $C1, $01, $00, $00, $00, $00, $00, $00
        BYTE    $E1, $C0, $C0, $E0, $61, $00, $00, $00
        BYTE    $F8, $7C, $78, $D0, $E1, $E3, $F7, $77
        BYTE    $40, $60, $70, $B0, $B2, $80, $FC, $EF
        BYTE    $00, $02, $00, $02, $02, $02, $02, $02
        BYTE    $78, $78, $78, $7C, $7D, $3F, $3F, $3D
        BYTE    $00, $00, $00, $00, $00, $00, $80, $A0
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $3F, $1E, $07, $00, $00, $00, $01, $03
        BYTE    $7F, $03, $F1, $2C, $1E, $3F, $FF, $FF
        BYTE    $F2, $FA, $EA, $A2, $82, $8E, $80, $CC
        BYTE    $1D, $1C, $0C, $0C, $04, $02, $00, $01
        BYTE    $80, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $03, $03, $03, $03, $02, $00, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $0F, $0C, $0D, $01
        BYTE    $C8, $C8, $80, $90, $20, $40, $80, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $40, $20, $08, $02, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $7F, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $FC, $00, $00
        BYTE    $02, $04, $08, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_09:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $01
        BYTE    $00, $01, $0F, $1F, $7F, $FF, $FF, $FF
        BYTE    $5F, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $00, $80, $F0, $E8, $FC, $FE, $FE, $FA
        BYTE    $00, $00, $00, $00, $00, $00, $00, $80
        BYTE    $03, $07, $0F, $0F, $1F, $1F, $3F, $3F
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FE, $FE
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $3F, $1F
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $9F
        BYTE    $FB, $FF, $F2, $F8, $FA, $F8, $E0, $C4
        BYTE    $40, $20, $00, $10, $00, $08, $04, $04
        BYTE    $7F, $7F, $7F, $7F, $7F, $7F, $7F, $7F
        BYTE    $F8, $F0, $E8, $E0, $E0, $E0, $C0, $80
        BYTE    $0F, $0E, $0E, $06, $02, $00, $00, $00
        BYTE    $0F, $07, $05, $06, $0F, $0F, $07, $03
        BYTE    $C4, $C6, $C6, $8B, $0B, $38, $7F, $BE
        BYTE    $00, $12, $02, $02, $42, $02, $C2, $E2
        BYTE    $7F, $7F, $7F, $3F, $7F, $3F, $3F, $3F
        BYTE    $00, $00, $00, $80, $B0, $F0, $30, $32
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $01, $00, $00, $00, $00, $00, $00, $00
        BYTE    $BF, $E4, $3F, $00, $03, $07, $0F, $3F
        BYTE    $FC, $7E, $BA, $DA, $D2, $F2, $F4, $F4
        BYTE    $0F, $1F, $0F, $03, $04, $02, $01, $00
        BYTE    $B0, $00, $80, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $3F, $3F, $3F, $3F, $23, $01, $03, $00
        BYTE    $F0, $F8, $F0, $F0, $E0, $C0, $80, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $80, $20, $10, $04, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $3F, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $FC, $00, $00
        BYTE    $01, $04, $08, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_10:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $01
        BYTE    $00, $01, $0F, $1F, $3F, $FF, $FF, $FF
        BYTE    $AF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $00, $80, $F0, $F8, $FC, $FF, $FF, $FF
        BYTE    $00, $00, $00, $00, $00, $00, $00, $C0
        BYTE    $03, $07, $0F, $0F, $1F, $1F, $3F, $3F
        BYTE    $FF, $BF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $F1, $E0
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FC
        BYTE    $FF, $FD, $FE, $FF, $FF, $FF, $FE, $FC
        BYTE    $C0, $C0, $D0, $00, $08, $88, $04, $84
        BYTE    $7F, $7F, $7F, $7F, $7F, $5F, $5F, $1F
        BYTE    $FF, $FF, $FF, $FF, $FF, $FE, $FC, $F8
        BYTE    $C0, $00, $00, $00, $00, $00, $00, $00
        BYTE    $F0, $70, $70, $70, $10, $00, $00, $00
        BYTE    $FC, $3C, $3C, $28, $31, $33, $3B, $1B
        BYTE    $C0, $C2, $E2, $62, $A8, $82, $FA, $FE
        BYTE    $5F, $4F, $0F, $4F, $4F, $0F, $27, $27
        BYTE    $F0, $F0, $F0, $F8, $FB, $FF, $F7, $F7
        BYTE    $00, $00, $00, $00, $00, $00, $00, $20
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $1F, $0F, $07, $00, $00, $00, $01, $03
        BYTE    $FE, $4E, $EE, $14, $3E, $7C, $FC, $FC
        BYTE    $03, $13, $09, $08, $04, $02, $00, $01
        BYTE    $F3, $E0, $E0, $E0, $40, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $03, $07, $07, $07, $04, $00, $00, $00
        BYTE    $F8, $F8, $F0, $F0, $60, $40, $80, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $40, $20, $08, $04, $01, $00, $00, $00
        BYTE    $01, $00, $00, $00, $FF, $3F, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $FC, $00, $00
        BYTE    $01, $02, $08, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_11:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $00
        BYTE    $00, $01, $07, $0F, $3F, $7F, $FF, $FF
        BYTE    $EF, $7F, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $00, $80, $F0, $F8, $FE, $FF, $FF, $FF
        BYTE    $00, $00, $00, $00, $00, $00, $80, $80
        BYTE    $03, $07, $07, $0F, $1F, $1F, $3F, $3F
        BYTE    $FF, $EF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $7F
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $1F, $0F
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $CF
        BYTE    $E0, $E0, $F0, $D0, $C8, $E8, $C4, $90
        BYTE    $3F, $7F, $7F, $3F, $7F, $27, $43, $43
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FC, $F0, $FC, $F8, $F0, $E0, $C0, $80
        BYTE    $07, $07, $03, $03, $00, $00, $00, $00
        BYTE    $8F, $07, $03, $82, $87, $07, $03, $01
        BYTE    $92, $9A, $C8, $9A, $1A, $32, $7E, $FE
        BYTE    $03, $41, $41, $01, $41, $41, $21, $20
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FE
        BYTE    $80, $00, $00, $80, $B8, $B8, $30, $71
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $01, $00, $00, $00, $00, $00, $00, $00
        BYTE    $BE, $E2, $7E, $06, $0E, $1C, $3C, $7C
        BYTE    $00, $10, $08, $00, $04, $02, $01, $00
        BYTE    $7F, $7E, $3E, $1C, $10, $00, $00, $00
        BYTE    $30, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $01, $00, $00, $01
        BYTE    $78, $F8, $F0, $F0, $20, $40, $40, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $80, $20, $10, $04, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $7F, $3F, $00, $00
        BYTE    $20, $00, $00, $00, $FF, $FC, $00, $00
        BYTE    $02, $04, $10, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_12:
        BYTE    $00, $00, $00, $00, $00, $00, $00, $02
        BYTE    $00, $01, $03, $1E, $0F, $DF, $3F, $7F
        BYTE    $3F, $DF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $00, $80, $F0, $F8, $FE, $FF, $FF, $FF
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $02, $04, $09, $01, $17, $07, $2F, $2F
        BYTE    $FF, $FC, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $F7
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $F9, $F0
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $7D
        BYTE    $E0, $E0, $F0, $F0, $F8, $F8, $F4, $F4
        BYTE    $5F, $1F, $5F, $4F, $0F, $48, $40, $00
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $7F, $7F
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FE, $FC
        BYTE    $E0, $80, $E0, $80, $80, $00, $00, $00
        BYTE    $78, $30, $31, $18, $08, $00, $00, $00
        BYTE    $F4, $76, $76, $56, $66, $6E, $7E, $3E
        BYTE    $40, $40, $40, $40, $40, $40, $20, $20
        BYTE    $7F, $3F, $1F, $1F, $1F, $1F, $1F, $1F
        BYTE    $F8, $F8, $F8, $F8, $FD, $FB, $F3, $F3
        BYTE    $00, $00, $00, $00, $80, $80, $80, $20
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $3E, $1A, $0E, $02, $02, $06, $0C, $1C
        BYTE    $00, $10, $08, $00, $04, $02, $01, $00
        BYTE    $0F, $0F, $07, $03, $01, $00, $00, $00
        BYTE    $F3, $E0, $E0, $C0, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $01
        BYTE    $18, $38, $30, $70, $40, $20, $40, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $80, $60, $08, $04, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $BF, $7F, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $FC, $00, $00
        BYTE    $02, $04, $10, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_13:
        BYTE    $00, $00, $00, $00, $00, $00, $00, $02
        BYTE    $00, $01, $05, $19, $07, $C3, $0F, $0F
        BYTE    $FF, $8F, $BF, $BF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $00, $80, $F0, $F8, $FE, $FF, $FF, $FF
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $01, $04, $08, $00, $11, $01, $23, $23
        BYTE    $1F, $1F, $3F, $7F, $FF, $FF, $FF, $FF
        BYTE    $FF, $5F, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $BF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $9F, $07
        BYTE    $E0, $E0, $F0, $F0, $F8, $F8, $FC, $BC
        BYTE    $03, $43, $43, $43, $01, $40, $40, $40
        BYTE    $FF, $FF, $FF, $FF, $FF, $8F, $07, $07
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FE, $F8, $FE, $FC, $FC, $F8, $F0, $E0
        BYTE    $07, $03, $03, $01, $00, $00, $00, $00
        BYTE    $1A, $1E, $0E, $8E, $9A, $0E, $0E, $0E
        BYTE    $40, $60, $60, $60, $60, $20, $20, $30
        BYTE    $03, $03, $01, $01, $01, $03, $01, $01
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $C0, $C0, $C0, $C0, $CC, $DC, $98, $38
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $0E, $06, $06, $00, $02, $02, $04, $04
        BYTE    $10, $10, $10, $08, $04, $02, $00, $01
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $FF, $FE, $7E, $3C, $20, $00, $00, $00
        BYTE    $38, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $01
        BYTE    $08, $08, $10, $30, $00, $20, $40, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $40, $10, $0C, $04, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $9F, $7F, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $FC, $00, $00
        BYTE    $02, $04, $10, $60, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_14:
        BYTE    $00, $00, $00, $00, $00, $00, $00, $02
        BYTE    $00, $01, $06, $18, $20, $40, $81, $03
        BYTE    $DF, $C3, $EF, $6F, $FF, $FF, $FF, $FF
        BYTE    $F7, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $00, $80, $F0, $F8, $FE, $FF, $FF, $FF
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $00, $04, $08, $00, $10, $00, $20, $20
        BYTE    $43, $03, $07, $0F, $3F, $3F, $3F, $7F
        BYTE    $FF, $F9, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $F9, $F8
        BYTE    $E0, $E0, $F0, $F0, $F8, $F8, $FC, $EC
        BYTE    $40, $40, $40, $40, $40, $40, $40, $60
        BYTE    $7F, $7F, $7F, $3F, $3F, $08, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $FF, $7F, $3F, $3F
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FE
        BYTE    $F0, $C0, $E0, $E0, $C0, $C0, $80, $00
        BYTE    $EE, $66, $66, $36, $12, $02, $02, $02
        BYTE    $70, $78, $78, $78, $78, $38, $38, $38
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $3F, $1F, $1F, $0F, $1F, $1F, $1F, $0F
        BYTE    $FE, $FE, $FC, $FE, $FE, $FF, $F9, $F9
        BYTE    $00, $00, $00, $00, $C0, $C0, $C0, $80
        BYTE    $02, $02, $02, $00, $02, $02, $04, $00
        BYTE    $18, $18, $08, $0C, $04, $02, $02, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $0F, $0F, $07, $03, $02, $00, $00, $00
        BYTE    $F1, $F0, $E0, $C0, $00, $00, $00, $00
        BYTE    $80, $00, $00, $00, $00, $00, $00, $00
        BYTE    $04, $08, $10, $00, $20, $40, $80, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $80, $68, $00, $0E, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $9F, $5F, $00, $00
        BYTE    $00, $00, $00, $00, $FD, $FC, $00, $00
        BYTE    $01, $06, $10, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_15:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $01
        BYTE    $00, $01, $0F, $10, $70, $60, $80, $00
        BYTE    $FF, $F0, $3B, $0B, $3F, $1F, $3F, $7F
        BYTE    $EF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $00, $80, $F0, $F8, $FE, $FF, $FF, $FF
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $02, $04, $08, $10, $10, $00, $20, $20
        BYTE    $00, $00, $00, $01, $03, $03, $07, $07
        BYTE    $7F, $3F, $7F, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $9F, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $E0, $E0, $F0, $F0, $F8, $F8, $3C, $1C
        BYTE    $20, $40, $60, $60, $60, $70, $70, $78
        BYTE    $07, $07, $07, $03, $03, $00, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $FF, $87, $03, $03
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FE, $FC, $FE, $FE, $FC, $FC, $F8, $F0
        BYTE    $1A, $1A, $0A, $0E, $06, $00, $02, $02
        BYTE    $7E, $7F, $7F, $7E, $7E, $7E, $3E, $3E
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $01, $00, $00, $00, $00, $01, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $E0, $E0, $E0, $E0, $EC, $FC, $9C, $99
        BYTE    $02, $00, $02, $00, $02, $02, $04, $04
        BYTE    $1E, $1C, $0E, $0E, $06, $02, $03, $01
        BYTE    $00, $00, $00, $00, $00, $00, $00, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $FF, $7F, $7E, $3C, $20, $00, $00, $00
        BYTE    $98, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $08, $10, $00, $20, $20, $40, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $E0, $24, $10, $06, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $C7, $6F, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $FE, $00, $00
        BYTE    $02, $04, $18, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_16:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $03
        BYTE    $00, $01, $0F, $1C, $78, $F0, $D0, $00
        BYTE    $FF, $7C, $CC, $03, $07, $01, $07, $07
        BYTE    $FF, $3F, $7F, $7F, $FF, $FF, $FF, $FF
        BYTE    $00, $80, $F0, $F8, $FE, $FF, $FF, $FF
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $02, $04, $08, $08, $10, $10, $30, $38
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $07, $07, $07, $0F, $3F, $3F, $7F, $7F
        BYTE    $E7, $F1, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $E0, $E0, $F0, $F0, $F8, $F8, $FC, $EC
        BYTE    $40, $40, $78, $78, $7C, $7C, $7E, $7F
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $7F, $7F, $7F, $3F, $1F, $04, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $FF, $3F, $1F, $1F
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $C4, $86, $E6, $C2, $C2, $80, $82, $02
        BYTE    $7F, $7F, $7F, $7F, $7F, $3F, $3F, $3F
        BYTE    $E0, $E0, $E0, $E0, $C0, $C0, $E0, $C0
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $0F, $07, $07, $07, $0F, $0F, $0F, $07
        BYTE    $FE, $FE, $FE, $FE, $FE, $FF, $F9, $FB
        BYTE    $00, $02, $02, $00, $C2, $80, $84, $C4
        BYTE    $0F, $1F, $0F, $0F, $03, $03, $01, $00
        BYTE    $80, $80, $80, $80, $80, $80, $80, $C0
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $07, $07, $07, $03, $02, $00, $00, $00
        BYTE    $FB, $F0, $E0, $C0, $00, $00, $00, $01
        BYTE    $00, $08, $10, $00, $20, $40, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $40, $11, $00, $05, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $E3, $1F, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $FC, $00, $00
        BYTE    $02, $06, $10, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_17:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $01
        BYTE    $00, $01, $07, $1E, $7E, $7C, $F4, $C0
        BYTE    $FF, $BF, $73, $00, $00, $00, $00, $00
        BYTE    $7F, $1F, $DF, $7F, $FF, $7F, $7F, $7F
        BYTE    $00, $80, $F0, $F8, $FE, $FF, $FF, $FF
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $03, $07, $0E, $0C, $1C, $18, $3C, $2E
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $20, $00, $00, $01, $03, $03, $03, $03
        BYTE    $FF, $7F, $7F, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $9F, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $E0, $E0, $F0, $F0, $F8, $F8, $FC, $FC
        BYTE    $61, $20, $7E, $7E, $5F, $5F, $7F, $7F
        BYTE    $00, $00, $00, $00, $80, $C0, $E0, $F0
        BYTE    $03, $03, $03, $01, $00, $00, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $FF, $41, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FA, $F2, $F8, $FA, $F2, $F0, $F2, $E2
        BYTE    $7F, $7F, $1F, $5F, $5F, $0F, $27, $27
        BYTE    $FE, $FE, $FF, $FE, $FC, $FC, $FC, $FC
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $FF, $7F, $7F, $7F, $7F, $7F, $7F, $7F
        BYTE    $E0, $C2, $C2, $C0, $D2, $F2, $B4, $64
        BYTE    $03, $13, $0B, $09, $05, $02, $01, $00
        BYTE    $F8, $F0, $F0, $F0, $E0, $E0, $E0, $E0
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $7F, $7E, $7C, $78, $00, $00, $00, $01
        BYTE    $68, $00, $08, $10, $00, $20, $40, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $70, $1C, $10, $04, $01, $00, $00, $00
        BYTE    $00, $20, $00, $80, $A1, $3F, $00, $00
        BYTE    $00, $00, $00, $00, $FD, $FC, $00, $00
        BYTE    $02, $04, $18, $20, $C0, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_18:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $03
        BYTE    $00, $01, $07, $1F, $7F, $7F, $FF, $F0
        BYTE    $FF, $FF, $D8, $80, $80, $80, $80, $00
        BYTE    $F7, $C7, $DF, $1F, $1F, $0F, $07, $0F
        BYTE    $00, $80, $F0, $F8, $FE, $FF, $FF, $FF
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $03, $07, $0F, $0F, $1B, $13, $3F, $3B
        BYTE    $E0, $C0, $80, $00, $00, $00, $00, $C0
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $0F, $07, $0F, $0F, $3F, $1F, $3F, $1F
        BYTE    $FF, $F7, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $E0, $E0, $F0, $F0, $F8, $F8, $FC, $FC
        BYTE    $78, $18, $4F, $4F, $07, $47, $47, $2F
        BYTE    $20, $00, $E0, $E0, $F8, $FC, $FE, $FF
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $3F, $3F, $1F, $0F, $0F, $02, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $FF, $1F, $0F, $0F
        BYTE    $FE, $FE, $FC, $FE, $FC, $FC, $FE, $FA
        BYTE    $4F, $4F, $07, $47, $43, $03, $20, $20
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $F0, $F0, $F0, $E0, $E0, $E0, $E0, $C0
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $07, $07, $07, $07, $07, $07, $0F, $07
        BYTE    $FA, $FA, $FA, $FC, $FE, $FE, $E8, $FC
        BYTE    $00, $10, $08, $00, $04, $02, $01, $00
        BYTE    $FF, $7F, $7F, $7E, $3C, $3C, $3C, $9C
        BYTE    $C0, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $07, $07, $07, $07, $00, $00, $00, $00
        BYTE    $F8, $C0, $90, $00, $20, $00, $40, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $4C, $27, $1C, $04, $01, $00, $00, $00
        BYTE    $00, $08, $00, $60, $EC, $6F, $00, $00
        BYTE    $00, $00, $00, $00, $FF, $FC, $00, $00
        BYTE    $01, $04, $18, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_19:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $01
        BYTE    $00, $01, $0F, $1D, $7F, $FF, $FF, $FE
        BYTE    $FF, $F7, $F7, $E0, $F0, $F0, $F0, $00
        BYTE    $FD, $D3, $37, $03, $03, $01, $00, $01
        BYTE    $00, $80, $F0, $F8, $FE, $FF, $FF, $EF
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $03, $07, $0F, $0F, $1C, $1C, $1F, $3E
        BYTE    $F8, $F0, $E0, $C0, $60, $A0, $F0, $7C
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $03, $01, $01, $01
        BYTE    $FF, $7F, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $E0, $E0, $F0, $F0, $F8, $F8, $FC, $FC
        BYTE    $4F, $47, $03, $41, $40, $40, $00, $45
        BYTE    $02, $00, $FE, $FE, $FF, $FF, $FF, $FF
        BYTE    $00, $00, $00, $00, $C0, $C0, $F0, $F8
        BYTE    $01, $01, $01, $00, $00, $00, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $7F, $11, $00, $00
        BYTE    $FE, $FE, $FE, $FE, $FE, $FE, $FE, $FE
        BYTE    $41, $01, $40, $40, $40, $00, $20, $20
        BYTE    $FF, $FF, $FF, $FF, $7F, $3F, $1F, $0F
        BYTE    $FF, $FF, $FF, $FF, $FE, $FE, $FE, $FE
        BYTE    $00, $80, $80, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $FE, $7E, $7C, $7E, $FE, $FC, $FC, $FC
        BYTE    $00, $10, $08, $00, $04, $02, $00, $01
        BYTE    $1F, $0F, $0F, $0F, $0F, $0F, $07, $07
        BYTE    $F8, $F0, $F0, $E0, $C0, $C0, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $01, $00, $00, $00, $00
        BYTE    $F8, $F0, $F0, $C0, $20, $40, $80, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $83, $21, $08, $04, $01, $00, $00, $00
        BYTE    $80, $E1, $00, $18, $DC, $7F, $00, $00
        BYTE    $00, $00, $00, $00, $39, $F8, $00, $00
        BYTE    $01, $06, $10, $20, $C0, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_20:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $01
        BYTE    $00, $01, $0F, $1F, $7F, $7F, $FF, $FF
        BYTE    $FF, $FD, $F9, $7C, $FC, $FE, $FE, $C0
        BYTE    $FF, $F9, $CC, $00, $00, $00, $00, $00
        BYTE    $00, $80, $F0, $F8, $FE, $3F, $1F, $3F
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $03, $07, $0F, $07, $17, $17, $03, $27
        BYTE    $FF, $FF, $FC, $F8, $8C, $0A, $7F, $E7
        BYTE    $00, $00, $00, $00, $00, $00, $00, $C0
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $1F, $0F, $0F, $0F, $3F, $1F, $1F, $1F
        BYTE    $E0, $E0, $F0, $F0, $F8, $F8, $FC, $FC
        BYTE    $43, $00, $40, $40, $40, $00, $40, $40
        BYTE    $E0, $E0, $3F, $3F, $0F, $0F, $1F, $9F
        BYTE    $20, $20, $E0, $F0, $FE, $FE, $FF, $FF
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $1F, $1F, $0F, $0F, $07, $01, $00, $00
        BYTE    $FE, $FE, $FE, $FE, $FE, $3E, $1E, $1E
        BYTE    $00, $40, $40, $00, $40, $40, $20, $00
        BYTE    $1F, $1F, $0F, $0F, $07, $03, $01, $01
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $F8, $FC, $FC, $F8, $F0, $F0, $F0, $E0
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $1E, $0E, $1E, $0E, $1E, $1C, $3C, $1C
        BYTE    $10, $10, $08, $00, $04, $02, $00, $01
        BYTE    $01, $00, $00, $00, $00, $00, $00, $00
        BYTE    $FF, $FF, $FF, $FE, $FC, $F8, $F0, $E0
        BYTE    $C0, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $01
        BYTE    $38, $38, $30, $70, $20, $20, $40, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $40, $20, $08, $02, $01, $00, $00, $00
        BYTE    $70, $7C, $00, $06, $9E, $4F, $00, $00
        BYTE    $00, $00, $00, $00, $79, $F8, $00, $00
        BYTE    $02, $04, $10, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_21:
        BYTE    $00, $00, $00, $00, $00, $00, $00, $02
        BYTE    $00, $01, $0F, $1F, $3F, $FF, $7F, $FF
        BYTE    $FF, $FF, $FF, $EF, $FF, $FF, $FF, $F8
        BYTE    $FC, $FB, $72, $00, $C0, $C0, $40, $00
        BYTE    $00, $80, $70, $68, $3E, $0F, $07, $07
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $01, $05, $09, $01, $11, $01, $20, $20
        BYTE    $FF, $FF, $FF, $FF, $F0, $F0, $F7, $FC
        BYTE    $F0, $E0, $C0, $80, $80, $E0, $D8, $5C
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $07, $01, $01, $01, $03, $03, $03, $01
        BYTE    $E0, $E0, $F0, $F0, $F8, $F8, $FC, $FC
        BYTE    $00, $40, $40, $40, $00, $40, $40, $00
        BYTE    $7E, $0E, $03, $03, $00, $00, $00, $09
        BYTE    $01, $00, $BF, $FF, $7F, $7F, $FF, $FF
        BYTE    $00, $00, $00, $80, $F0, $F0, $F8, $FE
        BYTE    $01, $01, $01, $00, $00, $00, $00, $00
        BYTE    $FE, $FE, $FE, $FE, $7E, $0E, $06, $06
        BYTE    $40, $40, $40, $00, $40, $40, $20, $20
        BYTE    $01, $01, $00, $00, $00, $00, $00, $00
        BYTE    $FF, $FF, $FF, $7F, $7F, $3F, $0F, $0F
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $C0, $C0, $C0, $80, $80, $00, $00, $00
        BYTE    $06, $06, $06, $06, $06, $04, $0C, $0C
        BYTE    $00, $10, $10, $08, $04, $02, $00, $01
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $0F, $0F, $0F, $0F, $1F, $1F, $1E, $0E
        BYTE    $FC, $F8, $F0, $E0, $C0, $80, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $01
        BYTE    $08, $18, $10, $30, $00, $20, $40, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $40, $20, $10, $04, $01, $00, $00, $00
        BYTE    $0E, $0E, $00, $01, $AF, $4F, $00, $00
        BYTE    $00, $84, $00, $80, $0B, $F8, $00, $00
        BYTE    $02, $04, $10, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_22:
        BYTE    $00, $00, $00, $00, $00, $00, $00, $02
        BYTE    $00, $01, $0F, $1F, $0F, $DF, $1F, $3F
        BYTE    $FF, $FF, $FF, $FB, $FF, $FF, $FF, $FF
        BYTE    $FF, $FD, $DD, $E0, $F0, $F8, $F8, $80
        BYTE    $00, $80, $F0, $18, $1A, $03, $01, $01
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $00, $04, $08, $00, $10, $00, $20, $20
        BYTE    $3F, $7F, $3F, $3F, $3F, $3E, $0E, $0F
        BYTE    $FF, $FE, $FC, $F8, $0E, $0E, $7F, $E7
        BYTE    $00, $00, $00, $00, $00, $00, $80, $E0
        BYTE    $01, $00, $00, $00, $00, $00, $00, $00
        BYTE    $E0, $60, $70, $70, $F8, $78, $7C, $3C
        BYTE    $00, $40, $40, $00, $40, $40, $00, $40
        BYTE    $07, $00, $00, $00, $00, $00, $00, $00
        BYTE    $F0, $F0, $37, $3F, $07, $07, $07, $8F
        BYTE    $10, $00, $F8, $F8, $FF, $FF, $FF, $FF
        BYTE    $00, $00, $00, $00, $00, $80, $C0, $E0
        BYTE    $3E, $3E, $3E, $1E, $1E, $02, $02, $02
        BYTE    $40, $40, $40, $40, $40, $20, $20, $20
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $0F, $0F, $07, $07, $03, $01, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FC, $FC, $FC, $FC, $F8, $F8, $F0, $F0
        BYTE    $02, $02, $02, $02, $02, $04, $00, $04
        BYTE    $10, $00, $10, $08, $04, $02, $00, $01
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $01, $01, $01, $01
        BYTE    $FF, $FF, $FF, $FE, $FC, $F8, $E0, $C0
        BYTE    $C0, $80, $00, $00, $00, $00, $00, $01
        BYTE    $08, $08, $10, $00, $20, $00, $40, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $40, $20, $10, $04, $01, $00, $00, $00
        BYTE    $01, $01, $00, $00, $9F, $6F, $00, $00
        BYTE    $C0, $E0, $20, $60, $CF, $F2, $00, $00
        BYTE    $02, $04, $10, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_23:
        BYTE    $00, $00, $00, $00, $00, $00, $00, $02
        BYTE    $00, $01, $0F, $1F, $27, $C7, $07, $0F
        BYTE    $7F, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FE, $FF, $FF, $78, $FE, $FF, $FF, $F0
        BYTE    $00, $80, $E0, $18, $0E, $02, $01, $00
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $00, $04, $08, $00, $10, $10, $10, $30
        BYTE    $0F, $0F, $07, $07, $03, $03, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $F0, $E0, $F6, $FE
        BYTE    $E0, $E0, $C0, $80, $E0, $B0, $FC, $2E
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $60, $60, $30, $10, $18, $18, $1C, $0C
        BYTE    $40, $00, $40, $40, $40, $40, $40, $40
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $3F, $07, $01, $01, $00, $00, $00, $04
        BYTE    $00, $80, $BF, $FF, $3F, $3F, $7F, $7F
        BYTE    $80, $80, $80, $C0, $F0, $F8, $FC, $FE
        BYTE    $0E, $0E, $06, $06, $06, $02, $02, $00
        BYTE    $40, $40, $40, $40, $40, $50, $20, $20
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $7F, $7F, $3F, $3F, $1F, $1F, $07, $07
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FE
        BYTE    $82, $C2, $C0, $82, $02, $04, $00, $04
        BYTE    $10, $00, $10, $08, $04, $02, $01, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $0F, $0F, $0F, $0F, $1F, $1F, $3C, $38
        BYTE    $FC, $F0, $F0, $E0, $80, $00, $00, $00
        BYTE    $00, $08, $10, $00, $20, $20, $40, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $80, $40, $10, $04, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $9F, $4F, $00, $00
        BYTE    $38, $36, $00, $18, $F5, $FA, $00, $00
        BYTE    $01, $46, $10, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_24:
        BYTE    $00, $00, $00, $00, $00, $00, $00, $02
        BYTE    $00, $01, $07, $0E, $30, $80, $00, $01
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $FF, $FF, $FF, $DF, $FF, $FF, $FF, $FF
        BYTE    $00, $80, $C0, $10, $C4, $E2, $E0, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $80
        BYTE    $00, $04, $08, $00, $10, $10, $2C, $20
        BYTE    $01, $00, $00, $00, $00, $00, $00, $00
        BYTE    $FF, $FF, $FF, $7F, $7F, $3F, $0F, $0F
        BYTE    $FE, $FE, $FC, $FC, $0E, $06, $3F, $F1
        BYTE    $00, $00, $00, $00, $00, $00, $C0, $F0
        BYTE    $C0, $20, $10, $10, $08, $08, $04, $04
        BYTE    $00, $40, $40, $40, $00, $40, $40, $40
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $03, $00, $00, $00, $00, $00, $00, $00
        BYTE    $F8, $78, $1E, $0F, $01, $01, $03, $23
        BYTE    $08, $00, $F8, $FC, $FF, $FF, $FF, $FF
        BYTE    $06, $06, $02, $02, $02, $82, $C2, $C0
        BYTE    $40, $00, $40, $40, $60, $24, $20, $20
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $03, $07, $03, $01, $01, $00, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $7F, $7F
        BYTE    $F2, $F2, $F0, $F2, $E2, $E0, $E4, $C4
        BYTE    $10, $00, $18, $08, $04, $02, $02, $01
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $01, $03, $03, $07
        BYTE    $7F, $FF, $FE, $FC, $F0, $F0, $C0, $81
        BYTE    $80, $08, $10, $00, $20, $40, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $C0, $20, $08, $04, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $8F, $5F, $00, $00
        BYTE    $07, $06, $00, $06, $F9, $FE, $00, $00
        BYTE    $02, $84, $10, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_25:
        BYTE    $00, $00, $00, $00, $00, $00, $00, $02
        BYTE    $00, $01, $07, $1F, $28, $C0, $00, $00
        BYTE    $FF, $FF, $FF, $3F, $3F, $1F, $1F, $1F
        BYTE    $FF, $FF, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $00, $80, $E0, $D0, $F4, $FA, $F0, $E0
        BYTE    $00, $00, $00, $00, $00, $00, $00, $80
        BYTE    $00, $04, $08, $00, $10, $00, $21, $20
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $1F, $1F, $0F, $07, $07, $03, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $F0, $F0, $7B, $7F
        BYTE    $C0, $C0, $C0, $C0, $60, $70, $FC, $1F
        BYTE    $40, $20, $00, $10, $00, $08, $04, $04
        BYTE    $40, $00, $40, $40, $40, $40, $40, $40
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $1F, $03, $00, $00, $00, $00, $00, $02
        BYTE    $C0, $C0, $DF, $7F, $1F, $1F, $1F, $3F
        BYTE    $02, $80, $80, $C2, $E2, $F2, $F2, $F8
        BYTE    $50, $40, $40, $44, $50, $60, $20, $20
        BYTE    $00, $00, $00, $00, $00, $00, $40, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $3F, $3F, $1F, $1F, $1F, $0F, $07, $07
        BYTE    $FE, $FC, $FE, $FC, $FA, $F8, $FC, $F4
        BYTE    $10, $00, $18, $08, $06, $02, $01, $01
        BYTE    $00, $00, $00, $00, $00, $00, $00, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $0F, $0F, $1F, $1F, $3E, $3C, $70, $E0
        BYTE    $E8, $C0, $C8, $90, $00, $20, $40, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $40, $20, $10, $04, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $8F, $5F, $00, $00
        BYTE    $00, $01, $00, $03, $FD, $FA, $00, $00
        BYTE    $E1, $86, $18, $60, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_26:
        BYTE    $00, $00, $00, $00, $00, $00, $00, $02
        BYTE    $00, $01, $07, $19, $26, $40, $80, $00
        BYTE    $BF, $FF, $FF, $CF, $07, $03, $03, $03
        BYTE    $FC, $FD, $FF, $FF, $FF, $FF, $FF, $FF
        BYTE    $00, $80, $E0, $F0, $FC, $FE, $FE, $F8
        BYTE    $00, $00, $00, $00, $00, $00, $00, $80
        BYTE    $00, $04, $08, $00, $10, $00, $20, $20
        BYTE    $00, $00, $00, $00, $00, $00, $30, $10
        BYTE    $03, $01, $00, $00, $00, $00, $00, $00
        BYTE    $FF, $FF, $FF, $7F, $7F, $3F, $07, $07
        BYTE    $F8, $F8, $F8, $F8, $0A, $02, $BF, $F1
        BYTE    $40, $20, $00, $10, $00, $08, $84, $E4
        BYTE    $40, $00, $40, $40, $60, $00, $40, $40
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $01, $00, $00, $00, $00, $00, $00, $00
        BYTE    $F8, $3C, $0D, $07, $01, $01, $03, $13
        BYTE    $20, $02, $F0, $F2, $FA, $FE, $FC, $FE
        BYTE    $44, $40, $40, $40, $62, $08, $28, $30
        BYTE    $00, $00, $00, $00, $00, $00, $04, $20
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $03, $03, $01, $01, $01, $01, $00, $00
        BYTE    $FE, $FE, $FE, $FE, $FE, $FC, $FC, $FC
        BYTE    $10, $10, $08, $08, $07, $03, $00, $00
        BYTE    $00, $00, $00, $00, $00, $80, $80, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $01, $03, $03, $07, $07, $0E, $1C
        BYTE    $F8, $F8, $F0, $F0, $80, $20, $40, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $40, $20, $10, $04, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $87, $6F, $00, $00
        BYTE    $00, $00, $00, $00, $FB, $F8, $00, $00
        BYTE    $39, $2E, $10, $E0, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00

@@frame_27:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $02
        BYTE    $00, $01, $0F, $1C, $15, $C0, $80, $00
        BYTE    $EF, $FF, $FF, $73, $81, $00, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $FF, $7F, $3F, $3F
        BYTE    $00, $80, $F0, $F8, $FE, $FF, $FF, $FF
        BYTE    $00, $00, $00, $00, $00, $00, $00, $80
        BYTE    $04, $04, $00, $08, $10, $00, $20, $20
        BYTE    $00, $00, $00, $00, $00, $00, $03, $01
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $3F, $1F, $0F, $07, $03, $01, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $F1, $F0, $7B, $3F
        BYTE    $40, $20, $00, $10, $80, $C8, $E4, $7C
        BYTE    $20, $40, $40, $40, $40, $40, $40, $48
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $1F, $03, $00, $00, $00, $00, $00, $01
        BYTE    $88, $82, $FC, $FE, $3E, $3E, $3E, $3E
        BYTE    $40, $60, $60, $50, $40, $69, $21, $3C
        BYTE    $40, $00, $00, $00, $40, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $7E, $3E, $3E, $3E, $3E, $3C, $1C, $3C
        BYTE    $10, $18, $08, $0C, $04, $06, $03, $00
        BYTE    $00, $00, $00, $00, $C0, $60, $68, $20
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $01, $03, $06
        BYTE    $38, $78, $70, $F0, $E0, $C0, $80, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $80, $40, $18, $06, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $E3, $1F, $00, $00
        BYTE    $00, $00, $00, $00, $7F, $F8, $00, $00
        BYTE    $0D, $1C, $10, $60, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_28:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $01
        BYTE    $00, $01, $07, $1E, $78, $70, $C0, $00
        BYTE    $6F, $FF, $FF, $0E, $B0, $00, $00, $00
        BYTE    $FF, $FF, $FF, $FF, $3F, $0F, $07, $07
        BYTE    $00, $80, $F0, $F8, $FE, $FF, $FF, $FF
        BYTE    $00, $00, $00, $00, $00, $00, $80, $C0
        BYTE    $02, $06, $08, $08, $10, $00, $20, $20
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $30, $10
        BYTE    $03, $01, $00, $00, $00, $00, $00, $00
        BYTE    $FF, $FF, $FF, $7F, $3E, $1F, $07, $07
        BYTE    $C0, $E0, $D0, $C0, $28, $18, $78, $EC
        BYTE    $40, $40, $00, $40, $40, $40, $40, $40
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $01, $00, $00, $00, $00, $00, $00, $00
        BYTE    $F6, $72, $1E, $1E, $06, $06, $0E, $2E
        BYTE    $70, $78, $6C, $70, $60, $31, $31, $39
        BYTE    $00, $00, $04, $00, $04, $20, $20, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $0E, $0E, $0E, $0E, $0E, $0C, $0C, $0C
        BYTE    $1C, $1C, $0E, $0E, $06, $07, $01, $00
        BYTE    $00, $00, $00, $00, $18, $08, $98, $98
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $01, $01
        BYTE    $08, $18, $30, $30, $60, $C0, $80, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $40, $20, $10, $0E, $01, $00, $00, $00
        BYTE    $00, $00, $00, $00, $E0, $3F, $00, $00
        BYTE    $00, $00, $00, $00, $79, $F2, $00, $00
        BYTE    $07, $0C, $10, $60, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
@@frame_29:
        BYTE    $00, $00, $00, $00, $00, $00, $01, $03
        BYTE    $00, $01, $0F, $1F, $76, $74, $E0, $C0
        BYTE    $7B, $FF, $F7, $83, $22, $00, $00, $00
        BYTE    $FF, $FF, $FF, $9F, $07, $01, $00, $00
        BYTE    $00, $80, $E0, $F8, $FE, $FF, $FF, $7F
        BYTE    $00, $00, $00, $00, $00, $00, $80, $80
        BYTE    $03, $07, $0C, $08, $18, $10, $20, $20
        BYTE    $80, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $03, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $80
        BYTE    $7F, $3F, $1F, $07, $07, $03, $00, $00
        BYTE    $E0, $E0, $F0, $F0, $D8, $C8, $FC, $FC
        BYTE    $24, $40, $40, $40, $40, $60, $40, $70
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $38, $0E, $06, $06, $02, $02, $02, $0A
        BYTE    $7C, $5E, $5D, $7C, $38, $3C, $3E, $3E
        BYTE    $00, $00, $80, $80, $00, $22, $02, $30
        BYTE    $00, $00, $40, $00, $40, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $20, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $02, $02, $02, $02, $02, $04, $00, $04
        BYTE    $1F, $1F, $0F, $0F, $07, $02, $00, $01
        BYTE    $00, $80, $80, $80, $83, $81, $43, $03
        BYTE    $00, $00, $00, $00, $00, $80, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $0C, $08, $00, $30, $20, $40, $80, $80
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        BYTE    $40, $00, $10, $04, $03, $00, $00, $00
        BYTE    $00, $00, $00, $00, $F8, $3F, $00, $00
        BYTE    $00, $00, $00, $00, $3F, $F2, $00, $00
        BYTE    $03, $04, $10, $20, $80, $00, $00, $00
        BYTE    $00, $00, $00, $00, $00, $00, $00, $00
        ENDP

