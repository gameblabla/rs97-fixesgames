;;==========================================================================;;
;; Joe Zbiciak's Moving Object Test.   Real simple test case.               ;;
;; Copyright 2002, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;; This software is licensed under the GNU GPL.  See COPYING.txt for info.  ;;
;;==========================================================================;;

;* ======================================================================== *;
;*  TO BUILD IN BIN+CFG FORMAT:                                             *;
;*      as1600 -o mob_test.bin -l mob_test.lst mob_test.asm                 *;
;*                                                                          *;
;*  TO BUILD IN ROM FORMAT:                                                 *;
;*      as1600 -o mob_test.rom -l mob_test.lst mob_test.asm                 *;
;* ======================================================================== *;

            ROMW    16              ; Use 16-bit ROM

;------------------------------------------------------------------------------
; Include system information
;------------------------------------------------------------------------------
            INCLUDE "../library/gimini.asm"

;------------------------------------------------------------------------------
; Global constants and configuration.
;------------------------------------------------------------------------------
GROM        EQU     $3000           ; GROM base address
GRAM        EQU     $3800           ; GRAM base address
TSKQM       EQU     $7              ; Task queue is 8 entries large
MAXTSK      EQU     4               ; Right now allow 4 active tasks
COLSTK      EQU     STIC.mode
VBLANK      EQU     STIC.viden
CS0         EQU     STIC.cs0
CS1         EQU     STIC.cs1
CS2         EQU     STIC.cs2
CS3         EQU     STIC.cs3
CB          EQU     STIC.bord


;------------------------------------------------------------------------------
; Allocate 8-bit variables in Scratch RAM
;------------------------------------------------------------------------------
SCRATCH     ORG     $100, $100, "-RWBN"

ISRVEC      RMB     2               ; Always at $100 / $101

            ; Task-oriented 8-bit variables
TSKQHD      RMB     1               ; Task queue head
TSKQTL      RMB     1               ; Task queue tail
TSKDQ       RMB     2*(TSKQM+1)     ; Task data queue
WTIMER      RMB     1               ; Wait timer
OVRFLO      RMB     1               ; Number of overflows observed
TSKACT      RMB     1               ; Number of active tasks

            ; Hand-controller 8-bit variables
SH_TMP      RMB     1               ; Temp storage.
SH_LR0      RMB     3               ;\
SH_FL0      EQU     SH_LR0 + 1      ; |-- Three bytes for left controller
SH_LV0      EQU     SH_LR0 + 2      ;/
SH_LR1      RMB     3               ;\
SH_FL1      EQU     SH_LR1 + 1      ; |-- Three bytes for right controller
SH_LV1      EQU     SH_LR1 + 2      ;/

            ; Demo-specific variables
CURMOB      RMB     1               ; Currently selected MOB
HDLYSH      RMB     1               ; Horizontal delay (shadowed)
VDLYSH      RMB     1               ; Vertical delay (shadowed)
DOCLR       RMB     1               ; Flag:  Clear collision bits?


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

_SYSTEM     EQU     $               ; end of system area



;------------------------------------------------------------------------------
; EXEC-friendly ROM header.
;------------------------------------------------------------------------------
            ORG     $5000           ; Use default memory map
ROMHDR:     BIDECLE ZERO            ; MOB picture base   (points to NULL list)
            BIDECLE ZERO            ; Process table      (points to NULL list)
            BIDECLE START           ; Program start address
            BIDECLE ZERO            ; Bkgnd picture base (points to NULL list)
            BIDECLE ONES            ; GRAM pictures      (points to NULL list)
            BIDECLE TITLE           ; Cartridge title/date
            DECLE   $03C0           ; No ECS title, run code after title,
                                    ; ... no clicks
ZERO:       DECLE   $0000           ; Screen border control
            DECLE   $0000           ; 0 = color stack, 1 = f/b mode
            DECLE   C_BLK, C_BLK    ; Initial color stack 0 and 1: Black
            DECLE   C_BLK, C_BLK    ; Initial color stack 2 and 3: Black
            DECLE   C_BLK           ; Initial border color: Black
ONES:       DECLE   $1, $0
;------------------------------------------------------------------------------




;; ======================================================================== ;;
;;  TITLE / START                                                           ;;
;;                                                                          ;;
;;  This contains the title string and the startup code.  We pre-empt the   ;;
;;  EXEC's initialization sequence by setting the "Special Copyright" bit   ;;
;;  in location $500C.  This causes the code at 'START' to run before the   ;;
;;  built-in title screen is completely displayed.                          ;;
;;                                                                          ;;
;;  The Startup code does very little.  Mainly, it sets the Interrupt       ;;
;;  Service Routine vector to point to our _real_ initialization routine,   ;;
;;  INIT.  This is done because we can only get to GRAM and STIC registers  ;;
;;  during the vertical retrace, and vertical retrace is signaled by an     ;;
;;  interrupt.  (Actually, we can have access to GRAM/STIC for longer       ;;
;;  if we don't hit the STIC 'handshake' at location $20, but then the      ;;
;;  display blanks.  During INIT, the display does blank briefly.)          ;;
;; ======================================================================== ;;
TITLE:      BYTE    100, "MOB Test", 0 ;
            ; Intercept/preempt EXEC initialization and just do our own.
            ; We call no EXEC routines in this program.
START:     
            CLRR    R4              ; Zero all system RAM, PSG0,& STIC.
            MVII    #$20,   R1      ; $00...$1F. (The STIC)
            JSRD    R5,     FILLZERO
           
            ADDI    #8,     R4      ; $28...$32. (The rest of the STIC)
            MVII    #11,    R1
            CALL    FILLZERO
                             
            MVII    #$F0,   R4      ; $F0...$35D. We spare the rand seed values
            MVII    #$26D,  R1      ; in $35E..$35F to add some randomness.
            CALL    FILLZERO
           
            MVII    #INIT,   R0     ; Our initialization routine
            MVII    #ISRVEC, R4     ; ISR vector
            MVO@    R0,     R4      ; Write low half
            SWAP    R0              ;
            MVO@    R0,     R4      ; Write high half
           
            MOVR    PC,     R5      ; Loop starting at the next instruction.
@@spin:     EIS
            ; This falls through to STUB which causes
            ; our loop (saves a couple DECLEs.)

;; ======================================================================== ;;
;;  STUB                                                                    ;;
;;  Null routine used in dispatchers where no behavior is defined/desired.  ;;
;; ======================================================================== ;;
STUB:   JR      R5              ; Stub routine


;; ======================================================================== ;;
;;  INIT                                                                    ;;
;;  Initializes the ISR, etc.  Gets everything ready to run.                ;;
;;  This is called via the ISR dispatcher, so it's safe to bang GRAM from   ;;
;;  here, too.                                                              ;;
;;                                                                          ;;
;;   -- Zero out memory to get started                                      ;;
;;   -- Set up variables that need to be set up here and there              ;;
;;   -- Set up GRAM image                                                   ;;
;;   -- Drop into the main state-machine                                    ;;
;; ======================================================================== ;;
INIT:   PROC
        DIS
        MVII    #$2F0,  R6              ; Reset the stack pointer


        ; Stub out all of the task hooks

        MVII    #MY_ISR,R0              ; Point ISR vector to our ISR
        MVO     R0,     ISRVEC          ; store low half of ISR vector
        SWAP    R0                      ;
        MVO     R0,     ISRVEC+1        ; store high half of ISR vector

        ; Default the GRAM image to be same as GROM.
        MVII    #GROM,  R5              ; Point R5 at GROM
        MVII    #GRAM,  R4              ; Point R4 at GRAM
        MVII    #$200,  R0
@@gromcopy:
        MVI@    R5,     R1
        MVO@    R1,     R4
        DECR    R0
        BNEQ    @@gromcopy

        ; Copy our GRAM font into GRAM overtop of default
        ;
        ; Font data is broken up into spans of characters that are copied
        ; into GRAM.  Each span is defined as follows.
        ;
        ;   Span Header:  2 Decles
        ;       DECLE   Skip Length (in bytes of GRAM memory)
        ;       DECLE   Span Length (in bytes of GRAM memory)
        ;   Span Data -- up to Span Length Decles.
        ;
        ; Span Data is run-length encoded using the upper two bits of the
        ; decle to specify the run length.  Valid run lengths are 0..3,
        ; with 0 meaning "just copy this byte to the GRAM", and 3 meaning
        ; "copy this byte to GRAM and make three more copies in the locations
        ; afterwards".  To see what I mean, look at the font data in
        ; "font.asm".
        ;
        ; The run length encoding does not change the value used for
        ; 'span length'.  The span length is always given in terms of
        ; # of GRAM locations, and not number of decles in the FONT data.
        ;
        ; The font is terminated with a span of length 0.

        CALL    LOADFONT
        DECLE   FONT

        ; Ok, everything's ready to roll now.
        EIS

        ;; ================================================================ ;;
        ;;  Lets do it!                                                     ;;
        ;; ================================================================ ;;
        CALL    MOBTEST         ; Set up object test's tasks.
        CALL    RUNQ            ; Let all the tasks run.
        DECR    PC              ; Should never get here...
        ENDP

;; ======================================================================== ;;
;;  LOADFONT  -- Loads our set of pictures into GRAM                        ;;
;; ======================================================================== ;;
LOADFONT PROC
        MVI@    R5,     R0
        PSHR    R5
        MOVR    R0,     R5

        MVII    #GRAM,  R4              ; Point R4 at GRAM

@@gramloop:
        MVI@    R5,     R0              ; Get skip & span len. (in GRAM bytes)
        TSTR    R0                      ; Quit if skip/span == 0.
        BEQ     @@gramdone

        MOVR    R0,     R1
        ANDI    #$7F8,  R0              ; Extract span length.
        XORR    R0,     R1              ; Clear away span bits from word
        SWAP    R1                      ; Extrack skip value.
        ADDR    R1,     R4              ; Skip our output pointer.
        SLR     R0,     1               ; Divide count by 2.

@@charloop:
        MVI@    R5,     R1              ; Get two bytes
        MVO@    R1,     R4              ; Put the first byte
        SWAP    R1                      ; Put the other byte into position
        MVO@    R1,     R4              ; Put the second byte
        DECR    R0                      ; Sheesh, do I have to spell this out?
        BNEQ    @@charloop              ; inner loop
        B       @@gramloop              ; outer loop

@@gramdone:
        PULR    PC
        ENDP



;; ======================================================================== ;;
;;  MOBTEST                                                                 ;;
;;  This sets up the main MOBTEST screen and all the event handlers.  Then  ;;
;;  it sits back and lets it all happen.                                    ;;
;; ======================================================================== ;;
MOBTEST PROC
        PSHR    R5

        CALL    CLRSCR                  ; Clear the display

        ;; Display the MOB-test screen.
        ;; It's pretty ugly, but who cares?

        CALL    PRINT.FLS  
        DECLE   $007, $200
                ;01234567890123456789
        DECLE   "[ Joe's 'MOB' Test ]", 0

        ;; Start a MARQUEE task for the title string (cheesy, I know)
        MVII    #2 + 256*16, R2         ; Length 16, Offset 2
        CALL    STARTTASK
        DECLE   0                       ; Task #0
        DECLE   MARQUEE                 ; MARQUEE task
        DECLE   20,     20              ; Period: every 10 ticks

        CALL    STARTTASK
        DECLE   2                       ; Task #2
        DECLE   SHOWMOBS                ; SHOWMOBS task
        DECLE   8,      8               ; Period: every 4 ticks

        MVII    #3,     R0              ; Enable 3 tasks.  Task 1 is key rpt
        MVO     R0,     TSKACT

        ;; Set our hand controller dispatcher
        MVII    #MOBHAND, R0
        MVO     R0,     SHDISP

        ;; Clear our STIC shadow
        MVII    #$20,   R1      ; Zero out the STIC shadow
        MVII    #STICSH,R4
        CALL    FILLZERO

        ;; Set our initial MOB state up by writing to the STIC shadow
        MVII    #$3800,  R0           ; PRIO=1, FG bit3 = 1, GRAM = 1
        MVII    #STICSH+$10, R4       ; Point to shadow copy of A regs.
        MVII    #8,     R1            ; All 8 MOBs
@@ml0:
        MVO@    R0,     R4            ;\
        INCR    R0                    ; |__ Set the 'A' register for all
        DECR    R1                    ; |   8 MOBs such that PRIO=1, bit3
        BNEQ    @@ml0                 ;/    of FG=1 and GRAM=1.
        
        MVII    #8,     R1            ; 8 MOBs
        MVII    #STICSH+$00, R4       ; Point R4 to shadow copy of X regs
        MVII    #STICSH+$08, R5       ; Point R5 to shadow copy of Y regs
        MVII    #$300 + 76, R2        ; Set VISB + INTR and center onscreen
        MVII    #20,        R3        ; Middle of third row of cards
@@ml1:
        MVO@    R2,     R4            ;\
        INCR    R2                    ; |   Line up all 8 MOBs in a thin
        MVO@    R3,     R5            ; |-- line in middle of 3rd row.
        DECR    R1                    ; |   Set VISB=1 and INTR=1 on each.
        BNEQ    @@ml1                 ;/

        ;; Show initial MOB state
        CALL    SHOWMOBS        ; Show updated MOB status.
        CLRR    R2
        INCR    R2
        MVO     R2,     DOCLR   ; clear collision info by default
        CALL    PICK            ; Select MOB 0
        
        PULR    PC
        ENDP


;; ======================================================================== ;;
;;  MY_ISR  -- Shadows the STIC registers in 16-bit RAM.                    ;;
;;                                                                          ;;
;;  This routine is called from an interrupt context.  It makes a           ;;
;;  "shadow" copy of the STIC registers from RAM to the STIC.  It also      ;;
;;  reads out a copy of the MOB collision registers and places their        ;;
;;  values in the shadow copy.  This allows our main program to access      ;;
;;  a copy of the STIC registers at any time, not just during vertical      ;;
;;  retrace.                                                                ;;
;; ======================================================================== ;;
MY_ISR  PROC
        PSHR    R5

        ;; Force color-stack mode, enable display
        MVI     COLSTK, R0
        MVO     R0,     VBLANK

        ;; Poke in the hdly/vdly shadow amounts
        MVI     HDLYSH, R0
        MVO     R0,     $30
        MVI     VDLYSH, R0
        MVO     R0,     $31

;;      MVI     VDLYSH, R0
;;      MVO     R0,     $22
        MVII    #0,     R0
        MVO     R0,     $32

        ;; Set up color stack, bgcolor:
        CLRR    R0
        MVO     R0,     CS0     ; CS0 == Black
        MVO     R0,     CS2     ; CS2 == Black
        INCR    R0
        MVO     R0,     CS1     ; CS1 == Blue (for selection bar)


        MVII    #9,     R0
        MVO     R0,     CB      ; Border == Cyan
        
        MVI     CS3,    R0      
        INCR    R0
        MVO     R0,     CS3     ; CS3 == prev_CS3 + 1

        ;; Point to RAM mirror and to STIC
        MVII    #STICSH,R4
        CLRR    R5

        ;; First, copy MOB parameters from RAM to STIC ($00..$17)
        MVII    #$18,   R1
@@l1:
        MVI@    R4,     R0      ; Read from RAM
        MVO@    R0,     R5      ; Write to STIC
        DECR    R1
        BNEQ    @@l1

        ;; Next, copy MOB interactions from STIC to RAM
        ;; If DOCLR is set, write zeros back to collision registers
        MVII    #$8,    R1
        MVI     DOCLR,  R0
        SARC    R0,     1
        BNC     @@l3
@@l2:
        MVI@    R5,     R0      ; Read from STIC
        DECR    R5
        MVO@    R0,     R4      ; Write to RAM
        CLRR    R0
        MVO@    R0,     R5      ; Write to STIC
        DECR    R1
        BNEQ    @@l2

        B       @@leave

@@l3:
        MVI@    R5,     R0      ; Read from STIC
        MVO@    R0,     R4      ; Write to RAM
        DECR    R1
        BNEQ    @@l3


        ;; Lastly, return via DOTIMER to update timer-based tasks
@@leave PULR    R5
        B       DOTIMER

        ENDP

;; ======================================================================== ;;
;;  MOBHAND                                                                 ;;
;;  Table of keypad dispatches                                              ;;
;; ======================================================================== ;;
MOBHAND PROC
        DECLE   HIT_KEYPAD      ; Process keypad
        DECLE   HIT_ACTION      ; Process action-keys
        DECLE   HIT_DISC        ; Process DISC inputs
        ENDP

;; ======================================================================== ;;
;;  HIT_KEYPAD -- handle keypad dispatching.                                ;;
;; ======================================================================== ;;
HIT_KEYPAD PROC
        ANDI    #$FF,   R2      ; Ignore controller number
        CMPI    #$01,   R2      ; 
        BLT     INCCARD         ; key == 0      ==> Increment card #
        CMPI    #$09,   R2      ;
        BLT     PICK            ; 1 <= key < 9  ==> Pick Object
        BEQ     INCHDLY         ; key == 9      ==> Incr. horiz delay
        CMPI    #$0B,   R2      ; 
        BLT     INCVDLY         ; key == 10     ==> Incr. vert delay
        BEQ     TOG_DOCLR       ; key == 11     ==> Toggle do-clear.
                                ;
        JR      R5              ; key > 11 (release event?) ==> Return.
        ENDP

;; ======================================================================== ;;
;;  HIT_ACTION -- handle action-key dispatching                             ;;
;; ======================================================================== ;;
HIT_ACTION PROC
        ANDI    #$FF,   R2      ; Ignore controller number
        DECR    R2              ;
        BEQ     TOG_VIS         ; Top:          Toggle VISB bit for MOB
        DECR    R2              ;
        BEQ     TOG_PRI         ; Lower-left:   Toggle PRIO bit for MOB
        DECR    R2              ;
        BEQ     TOG_INT         ; Lower-right:  Togglie INTR bit for MOB
        JR      R5              ; Key-release:  Ignore
        ENDP

;; ======================================================================== ;;
;;  HIT_DISC   -- handle disc dispatching.                                  ;;
;;                This also handles the "repeating" nature of the DISC.     ;;
;; ======================================================================== ;;
HIT_DISC PROC
        ANDI    #$FF,   R2      ; Ignore controller number

        CMPI    #$80,   R2      ; \__ shut off keyrepeat on release
        BGE     @@release       ; /

        ADDI    #2,     R2      ; \__ Rotate by 2 to make decoding easier
        ANDI    #$F,    R2      ; /

        MVII    #MOVE.d,R3      ; 10, 11, 12, 13 ==> Move down 

        CMPI    #4,     R2      ;
        BGE     @@not_r         ; 
        MVII    #MOVE.r,R3      ; 14, 15, 0, 1   ==> Move right
        B       @@got_it

@@not_r CMPI    #8,     R2      ;
        BGE     @@not_u         ; 
        MVII    #MOVE.u,R3      ; 2, 3, 4, 5     ==> Move up
        B       @@got_it

@@not_u CMPI    #12,    R2      ;
        BGE     @@got_it        ; 
        MVII    #MOVE.l,R3      ; 6, 7, 8, 9     ==> Move left


@@got_it:
        PSHR    R5

        MOVR    R3,     R2
        CALL    STARTTASK       ; Set up key-repeating.
        DECLE   1               ; Task #1
        DECLE   @@repeat        ; KEYREPEAT task
        DECLE   30,     4       ; First repeat 250ms, rest @ 30Hz

        PULR    R5
@@repeat:
        JR      R2              ; Jump to the movement routine.

@@release:
        MVII    #1,     R3
        B       STOPTASK

        ENDP


;; ======================================================================== ;;
;;  PICK   -- Pick a MOB to make "current"                                  ;;
;; ======================================================================== ;;
PICK    PROC
        ;; Remember the newly selected MOB
        DECR    R2
        MVO     R2,     CURMOB
        
        ;; Update the highlight bar.  We do this by stepping through all 8
        ;; rows, de-highlighting the rows that aren't current, and 
        ;; highlighting the one row that IS current.
        MVII    #$200 + 40, R4
        CLRR    R0
@@l:
        CLRR    R1
        CMPR    R0,     R2    ; Is this the row?
        BNEQ    @@not         ; No:  De-highlight it
        MVII    #$2000, R1    ; Yes: Highlight it by advancing color-stack
@@not:
        MVO@    R1,     R4    ; Set/clear colorstack bit at left edge
        ADDI    #18,    R4    ;
        MVO@    R1,     R4    ; Set/clear colorstack bit at right edge

        INCR    R0            ;\
        CMPI    #8,     R0    ; |-- Have we done all 8 yet?
        BLT     @@l           ;/

        JR      R5
        ENDP

;; ======================================================================== ;;
;;  MOVE   -- Move a MOB.                                                   ;;
;;  This just adds or subtracts 1 from either the X or Y coord. depending   ;;
;;  on the direction pressed on the DISC.                                   ;;
;; ======================================================================== ;;
MOVE    PROC

@@u:    CLRR    R0
        MVII    #$FFFF, R1
        B       @@move

@@d:    CLRR    R0
        MVII    #1,     R1
        B       @@move

@@l:    MVII    #$FFFF, R0
        CLRR    R1
        B       @@move

@@r:    MVII    #1,     R0
        CLRR    R1

@@move:
        ;; When we get to this point, R0 has our X offset, and
        ;; R1 has our Y offset.

        MVI     CURMOB, R2      ; Get our current MOB #
        ADDI    #STICSH,R2      ; R2 points to X coord
        MVII    #8,     R3
        ADDR    R2,     R3      ; R3 points to Y coord

        ADD@    R2,     R0      ; Add X offset to X coordinate
        MVI@    R2,     R4      ; Get original X register value
        ANDI    #$00FF, R0      ; Keep only X coord part of updated X reg.
        ANDI    #$FF00, R4      ; Keep the non-coord parts of original X reg.
        ADDR    R4,     R0      ; Merge the two halves together
        MVO@    R0,     R2      ; Store out the updated X register.

        ADD@    R3,     R1      ; Add Y offset to Y coordinate
        MVI@    R3,     R4      ; Get original Y register value
        ANDI    #$007F, R1      ; Keep only Y coord part of update Y reg.
        ANDI    #$FF80, R4      ; Keep non-coord parts of original Y reg.
        ADDR    R4,     R1      ; Merge the two halves together
        MVO@    R1,     R3      ; Store out the updated Y register

        PSHR    R5              ; \
        CALL    RATELIMIT       ;  |__ Don't let this puppy move too fast.
        DECLE   1               ;  |
        PULR    PC              ; /

        ENDP

;; ======================================================================== ;;
;;  TOG_DOCLR -- Toggle the "clear all collisions" mode                     ;;
;; ======================================================================== ;;
TOG_DOCLR PROC
        MVI     DOCLR,  R0
        XORI    #1,     R0
        MVO     R0,     DOCLR
        JR      R5
        ENDP

;; ======================================================================== ;;
;;  TOG_VIS -- Toggle visibility for a MOB                                  ;;
;; ======================================================================== ;;
TOG_VIS PROC
        MVI     CURMOB, R2
        ADDI    #STICSH,R2
        MVII    #$0200, R1
        XOR@    R2,     R1
        MVO@    R1,     R2
        MVI     CURMOB, R0
        B       SHOWMOB
        ENDP

;; ======================================================================== ;;
;;  TOG_INT -- Toggle interaction bit for a MOB                             ;;
;; ======================================================================== ;;
TOG_INT PROC
        MVI     CURMOB, R2
        ADDI    #STICSH,R2
        MVII    #$0100, R1
        XOR@    R2,     R1
        MVO@    R1,     R2
        MVI     CURMOB, R0
        B       SHOWMOB
        ENDP

;; ======================================================================== ;;
;;  TOG_PRI -- Toggle priority for a MOB                                    ;;
;; ======================================================================== ;;
TOG_PRI PROC
        MVI     CURMOB, R2
        ADDI    #STICSH + $10,R2
        MVII    #$2000, R1
        XOR@    R2,     R1
        MVO@    R1,     R2
        MVI     CURMOB, R0
        B       SHOWMOB
        ENDP

;; ======================================================================== ;;
;;  INCHDLY -- Increase horizontal delay                                    ;;
;; ======================================================================== ;;
INCHDLY PROC
        MVI     HDLYSH, R0
        INCR    R0
        MVO     R0,     HDLYSH
        MVI     CURMOB, R0
        B       SHOWMOB
        ENDP
;; ======================================================================== ;;
;;  INCVDLY -- Increase vertical delay                                      ;;
;; ======================================================================== ;;
INCVDLY PROC
        MVI     VDLYSH, R0
        INCR    R0
        MVO     R0,     VDLYSH
        MVI     CURMOB, R0
        B       SHOWMOB
        ENDP

;; ======================================================================== ;;
;;  INCCARD -- Set the card for a MOB.                                      ;;
;; ======================================================================== ;;
INCCARD PROC
        MVI     CURMOB, R3
        ADDI    #STICSH + $10,R3
        MVI@    R3,     R2
        ADDI    #8,     R2
        ANDI    #$38,   R2
        MVII    #$FE07, R1
        AND@    R3,     R1
        ADDR    R2,     R1
        MVO@    R1,     R3
        JR      R5
        ENDP

;; ======================================================================== ;;
;;  SHOWMOB -- Show MOB info for MOB # given in R0.                         ;;
;; ======================================================================== ;;
SHOWMOB PROC
        PSHR    R5
        PSHR    R4
        PSHR    R3
        NOP                     ; needed to prevent display glitch
        PSHR    R2
        PSHR    R1
        PSHR    R0

        ;; Move to row "MOB# + 2"
        MOVR    R0,     R1
        SLL     R1,     2
        ADDR    R0,     R1
        SLL     R1,     2       ; R1 = R0 * 5.
        MVII    #$200+41, R4
        ADDR    R1,     R4      ; R4 = ptr to row "MOB# + 2", col #1

        ;; Get STIC Shadow ptr
        MVII    #STICSH, R5
        ADDR    R0,     R5

        ;; Now display the X/Y/A/C registers
        MVII    #7,     R1
        MVI@    R5,     R0
        PSHR    R5
        CALL    HEX12 
        PULR    R5

        INCR    R4
        ADDI    #7,     R5
        MVI@    R5,     R0
        PSHR    R5
        CALL    HEX12 
        PULR    R5
        
        INCR    R4
        ADDI    #7,     R5
        MVI@    R5,     R0
        PSHR    R5
        CALL    HEX16
        PULR    R5
        
        INCR    R4
        ADDI    #7,     R5
        MVI@    R5,     R0
        CALL    HEX16
        
        PULR    R0
        PULR    R1
        PULR    R2
        PULR    R3
        PULR    R4
        PULR    PC
        ENDP

;; ======================================================================== ;;
;;  SQUARES  -- Colored square "string".                                    ;;
;; ======================================================================== ;;
SQUARES PROC
@@sqr           EQU     01000000000000b     ; Selects 'colored square mode'

@@cs_pix0_0     EQU     00000000000000b     ; ColSqr Pixel 0, color == 0
@@cs_pix0_1     EQU     00000000000001b     ; ColSqr Pixel 0, color == 1
@@cs_pix0_2     EQU     00000000000010b     ; ColSqr Pixel 0, color == 2
@@cs_pix0_3     EQU     00000000000011b     ; ColSqr Pixel 0, color == 3
@@cs_pix0_4     EQU     00000000000100b     ; ColSqr Pixel 0, color == 4
@@cs_pix0_5     EQU     00000000000101b     ; ColSqr Pixel 0, color == 5
@@cs_pix0_6     EQU     00000000000110b     ; ColSqr Pixel 0, color == 6
@@cs_pix0_7     EQU     00000000000111b     ; ColSqr Pixel 0, color == 7

@@cs_pix1_0     EQU     00000000000000b     ; ColSqr Pixel 1, color == 0
@@cs_pix1_1     EQU     00000000001000b     ; ColSqr Pixel 1, color == 1
@@cs_pix1_2     EQU     00000000010000b     ; ColSqr Pixel 1, color == 2
@@cs_pix1_3     EQU     00000000011000b     ; ColSqr Pixel 1, color == 3
@@cs_pix1_4     EQU     00000000100000b     ; ColSqr Pixel 1, color == 4
@@cs_pix1_5     EQU     00000000101000b     ; ColSqr Pixel 1, color == 5
@@cs_pix1_6     EQU     00000000110000b     ; ColSqr Pixel 1, color == 6
@@cs_pix1_7     EQU     00000000111000b     ; ColSqr Pixel 1, color == 7

@@cs_pix2_0     EQU     00000000000000b     ; ColSqr Pixel 2, color == 0
@@cs_pix2_1     EQU     00000001000000b     ; ColSqr Pixel 2, color == 1
@@cs_pix2_2     EQU     00000010000000b     ; ColSqr Pixel 2, color == 2
@@cs_pix2_3     EQU     00000011000000b     ; ColSqr Pixel 2, color == 3
@@cs_pix2_4     EQU     00000100000000b     ; ColSqr Pixel 2, color == 4
@@cs_pix2_5     EQU     00000101000000b     ; ColSqr Pixel 2, color == 5
@@cs_pix2_6     EQU     00000110000000b     ; ColSqr Pixel 2, color == 6
@@cs_pix2_7     EQU     00000111000000b     ; ColSqr Pixel 2, color == 7

@@cs_pix3_0     EQU     00000000000000b     ; ColSqr Pixel 3, color == 0
@@cs_pix3_1     EQU     00001000000000b     ; ColSqr Pixel 3, color == 1
@@cs_pix3_2     EQU     00010000000000b     ; ColSqr Pixel 3, color == 2
@@cs_pix3_3     EQU     00011000000000b     ; ColSqr Pixel 3, color == 3
@@cs_pix3_4     EQU     10000000000000b     ; ColSqr Pixel 3, color == 4
@@cs_pix3_5     EQU     10001000000000b     ; ColSqr Pixel 3, color == 5
@@cs_pix3_6     EQU     10010000000000b     ; ColSqr Pixel 3, color == 6
@@cs_pix3_7     EQU     10011000000000b     ; ColSqr Pixel 3, color == 7

        DECLE   $2000
        DECLE   @@cs_pix0_0 + @@cs_pix1_0 + @@cs_pix2_1 + @@cs_pix3_1 + @@sqr
        DECLE   @@cs_pix0_2 + @@cs_pix1_2 + @@cs_pix2_3 + @@cs_pix3_3 + @@sqr
        DECLE   @@cs_pix0_4 + @@cs_pix1_4 + @@cs_pix2_5 + @@cs_pix3_5 + @@sqr
        DECLE   @@cs_pix0_6 + @@cs_pix1_6 + @@cs_pix2_7 + @@cs_pix3_7 + @@sqr
        DECLE   0, $2000
        ENDP

;; ======================================================================== ;;
;;  SHOWMOBS -- Show info for all 8 MOBs, as well as our clear-toggle mode. ;;
;; ======================================================================== ;;
SHOWMOBS    PROC
        PSHR    R5
        CLRR    R0
@@l:
        CALL    SHOWMOB
        INCR    R0
        CMPI    #8,     R0
        BLT     @@l

        MVII    #$200 + 10*20 + 7, R4
        MVII    #7, R1
        MVII    #SQUARES, R5
@@l2:   MVI@    R5,     R0
        MVO@    R0,     R4
        DECR    R1
        BNEQ    @@l2

        CALL    PRINT.FLS  
        DECLE   $003, $200 + 20*11 + 5
        DECLE   "Clear: ", 0

        MVI     DOCLR,  R0
        SARC    R0,     1
        BNC     @@off

        CALL    PRINT.FLS  
        DECLE   $006, $200 + 20*11 + 12
        DECLE   "ON!", 0
        PULR    PC

@@off:
        CALL    PRINT.FLS  
        DECLE   $003, $200 + 20*11 + 12
        DECLE   "Off", 0

        PULR    PC
        ENDP

;; ======================================================================== ;;
;;  FONT data                                                               ;;
;; ======================================================================== ;;
FONT:   PROC
;; Skipped 0 indices.
;; Encoding span of 8 entries
        DECLE   $0040

        DECLE  $0000  ;........
;       - - -         ;........
        DECLE  $0000  ;........
;       - - -         ;........
        DECLE  $0000  ;........
;       - - -         ;........
        DECLE  $0100  ;........
;       - - -         ;.......#

        DECLE  $FFFF  ;########
;       - - -         ;########
        DECLE  $FFFF  ;########
;       - - -         ;########
        DECLE  $FFFF  ;########
;       - - -         ;########
        DECLE  $FFFF  ;########
;       - - -         ;########

        DECLE  $F0F0  ;####....
;       - - -         ;####....
        DECLE  $F0F0  ;####....
;       - - -         ;####....
        DECLE  $0000  ;........
;       - - -         ;........
        DECLE  $0000  ;........
;       - - -         ;........

        DECLE  $0F0F  ;....####
;       - - -         ;....####
        DECLE  $0F0F  ;....####
;       - - -         ;....####
        DECLE  $0000  ;........
;       - - -         ;........
        DECLE  $0000  ;........
;       - - -         ;........

        DECLE  $0000  ;........
;       - - -         ;........
        DECLE  $0000  ;........
;       - - -         ;........
        DECLE  $0F0F  ;....####
;       - - -         ;....####
        DECLE  $0F0F  ;....####
;       - - -         ;....####

        DECLE  $0000  ;........
;       - - -         ;........
        DECLE  $0000  ;........
;       - - -         ;........
        DECLE  $F0F0  ;####....
;       - - -         ;####....
        DECLE  $F0F0  ;####....
;       - - -         ;####....

 IF 0
        DECLE  $55AA  ;#.#.#.#.
;       - - -         ;.#.#.#.#
        DECLE  $55AA  ;#.#.#.#.
;       - - -         ;.#.#.#.#
        DECLE  $55AA  ;#.#.#.#.
;       - - -         ;.#.#.#.#
        DECLE  $55AA  ;#.#.#.#.
;       - - -         ;.#.#.#.#

        DECLE  $AA55  ;.#.#.#.#
;       - - -         ;#.#.#.#.
        DECLE  $AA55  ;.#.#.#.#
;       - - -         ;#.#.#.#.
        DECLE  $AA55  ;.#.#.#.#
;       - - -         ;#.#.#.#.
        DECLE  $AA55  ;.#.#.#.#
;       - - -         ;#.#.#.#.
 ELSE
        DECLE  $0080, 0, 0, 0
        DECLE  $8000, 0, 0, 0
 ENDI


;; End of font.
        DECLE   $0000

;; Total chars:           4 characters
;; Total length:         18 decles
;; Decles/char:       4.500 decles/character
        ENDP

;; ======================================================================== ;;
;;  MARQUEE                                                                 ;;
;;  Cycles colors on a string of text.                                      ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R2 -- Task instance data:                                           ;;
;;            Low byte of instance data gives screen offset.                ;;
;;            High byte of instance data gives length.                      ;;
;; ======================================================================== ;;
MARQUEE PROC

        MOVR    R2,     R3
        ANDI    #$FF,   R3      ; R3 == screen offset.
        XORR    R3,     R2
        SWAP    R2              ; R2 == length.
        ADDI    #$200,  R3      ; Convert screen offset into actual address.

        MVI@    R3,     R0      ; Get first display word from string.
        MVII    #$7,    R1      ; Color mask for three LSBs.
        ANDR    R1,     R0      ; Get color from leading character in string.
        COMR    R1              ; Invert our mask, so we can replace colors.
@@loop:
        DECR    R0              ; Cycle color by decrementing
        BNEQ    @@ok            ; Make sure it didn't go to zero!
        MVII    #$7,    R0      ; If it did, set it back to 7.
@@ok:
        MVI@    R3,     R4      ; Get a character from the display.
        ANDR    R1,     R4      ; Mask away its color bits.
        ADDR    R0,     R4      ; Replace them with our new color bits.
        MVO@    R4,     R3      ; Write the modified character to the display.
        INCR    R3              ; Move to next character.
        DECR    R2              ; Loop until done.
        BNEQ    @@loop

        JR      R5              ; Return.
        ENDP


;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
        INCLUDE "../library/hexdisp.asm"    ; for HEX16, HEX12
        INCLUDE "../library/print.asm"      ; for PRINT.xxx  
        INCLUDE "../library/fillmem.asm"    ; for FILLZERO, FILLMEM, CLRSCR
        INCLUDE "../task/scanhand.asm"      ; SCANHAND (include before taskq!)
        INCLUDE "../task/timer.asm"         ; timer-based tasks
        INCLUDE "../task/taskq.asm"         ; task RUNQ
        

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
