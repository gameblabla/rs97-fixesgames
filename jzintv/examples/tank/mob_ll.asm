;; ======================================================================== ;;
;;  MOB_LL.ASM:  Low level MOB manipulation code.                           ;;
;;                                                                          ;;
;;  Note:  This code does not implement animation or motion.                ;;
;;                                                                          ;;
;;  For each MOB, we maintain the following status information:             ;;
;;                                                                          ;;
;;   -- X/Y coordinate.  This coordinate pair is stored as a 16-bit         ;;
;;      8.8 quantity across two 8-bit locations for each MOB.               ;;
;;      The MOB display code ignores the fractional portion of the X/Y      ;;
;;      coordinates.                                                        ;;
;;                                                                          ;;
;;   -- Attribute index.  This is a small integer index that points to      ;;
;;      an MOB attribute table entry in ROM.  This attribute table entry    ;;
;;      sets up X/Y adjust values, flags (mirror, size, flip, color) and    ;;
;;      color.  (Some games may need to factor color out separately.)       ;;
;;                                                                          ;;
;;   -- Picture index.  This is an 8-bit value which indexes into the       ;;
;;      GFX_DATA array.  We multiply this value by 4 words prior to         ;;
;;      generating the index.  Valid indices are 0..127.  We use bit        ;;
;;      7 to indicate that the picture number has changed since the         ;;
;;      last interrupt.  (Strictly, we don't need to remember this          ;;
;;      value after we've loaded the card into GRAM, except that            ;;
;;      animation code might wish to refer to this to know what frame       ;;
;;      number is next.)  Note that we actually store TWO picture           ;;
;;      indices for each MOB, one for the upper half, and one for the       ;;
;;      lower half.                                                         ;;
;;                                                                          ;;
;;  Thus, this code stores 7 bytes for each MOB.  The X/Y data is           ;;
;;  stored interleaved in its own array.  The attribute and picture         ;;
;;  index tables are each stored separately.  The total storage devoted     ;;
;;  to the 8 MOBs by this code is 56 bytes.                                 ;;
;;                                                                          ;;
;;  Notice that we do not store the GRAM card allocated to the MOB.  The    ;;
;;  MOBs are assigned to GRAM cards 48 through 63, with two GRAM cards      ;;
;;  devoted to each MOB, thus allowing 8x16 MOBs to be displayed.  If a     ;;
;;  given MOB is only an 8x8 MOB, the neighboring GRAM card can be used     ;;
;;  for other purposes.                                                     ;;
;;                                                                          ;;
;;  In addition to the tables described above, we also store a 16-bit       ;;
;;  collision dispatch pointer.  Whenever a collision is detected between   ;;
;;  a MOB and another MOB, the background or the screen border, we will     ;;
;;  schedule the dispatch function for that MOB, if one is set.  In order   ;;
;;  to filter out unwanted collisions, we AND each collision entry with     ;;
;;  a collision mask.  A second 16-bit pointer points to the collision      ;;
;;  mask table.                                                             ;;
;;                                                                          ;;
;;  For performance reasons, we store a 32 word STIC register shadow in     ;;
;;  16-bit RAM.  This allows us to do most of our computation outside       ;;
;;  the time-critical vertical blank period.                                ;;
;;                                                                          ;;
;; ------------------------------------------------------------------------ ;;
;;  GLOBAL VARIABLES REQUIRED BY THIS MODULE                                ;;
;;                                                                          ;;
;;      Name        Length  Width   Purpose                                 ;;
;;      MOB_FLG        1    8-bit   Flag indicating what MOB updates to do  ;;
;;      MOB_PIC       16    8-bit   MOB picture table; 2 cards per MOB.     ;;
;;      MOB_ATR        8    8-bit   MOB attribute index table. 1 per MOB.   ;;
;;      MOB_XYP       32    8-bit   MOB X/Y position table, 8.8 data.       ;;
;;      MOB_IGN        1   16-bit   MOB collision ignore table ptr.         ;;
;;      MOB_DIS        1   16-bit   MOB collision dispatch table ptr.       ;;
;;      STICSH        32   16-bit   STIC MOB register shadow.               ;;
;;      TSKQHD         1    8-bit   \_ Task queue vars.  Collision disp.    ;;
;;      TSKQTL         1    8-bit   /  uses these to avoid overflows.       ;;
;;                                                                          ;;
;; ------------------------------------------------------------------------ ;;
;;  EXTERNAL STRUCTURES REFERENCED BY THIS MODULE                           ;;
;;                                                                          ;;
;;      GFX_DATA    Actual graphics picture data to load into GRAM.         ;;
;;      ATR_DATA    MOB attribute table; 3 words per record.                ;;
;;                                                                          ;;
;; ------------------------------------------------------------------------ ;;
;;  HELPER MACROS                                                           ;;
;;                                                                          ;;
;;  AtrAddr(n)      Address of Attribute Table slot for MOB #n              ;;
;;  PicAddr(n,top)  Address of upper Picture Table slot for MOB #n          ;;
;;  PicAddr(n,bot)  Address of lower Picture Table slot for MOB #n          ;;
;;  XYPAddr(n,x_lo) Address of LSB of X Position in XYP slot for MOB #n.    ;;
;;  XYPAddr(n,x_hi) Address of MSB of X Position in XYP slot for MOB #n.    ;;
;;  XYPAddr(n,y_lo) Address of LSB of Y Position in XYP slot for MOB #n.    ;;
;;  XYPAddr(n,y_hi) Address of MSB of Y Position in XYP slot for MOB #n.    ;;
;;                                                                          ;;
;; ======================================================================== ;;

;; ======================================================================== ;;
;;  AtrAddr(n) -- Computes address of attribute table entry for MOB n       ;;
;; ======================================================================== ;;
MACRO       AtrAddr(n)
            (MOB_ATR + (%n%))
ENDM

;; ======================================================================== ;;
;;  PicAddr(n,top|bot) -- Computes address of upper/lower picture for MOB   ;;
;; ======================================================================== ;;
MACRO       PicAddr(n, lh)
            (MOB_PIC + 2*(%n%) + MOB_MACRO.pic_%lh%)
ENDM

;; ======================================================================== ;;
;;  XYPAddr(n,byte) -- Computes address of specified byte in X/Y posn tbl.  ;;
;; ======================================================================== ;;
MACRO       XYPAddr(n, b)
            (MOB_XYP + 4*(%n%) + MOB_MACRO.xyp_%b%)
ENDM


MOB_MACRO   STRUCT  0
@@pic_top   EQU     0
@@pic_bot   EQU     1

@@xyp_x_lo  EQU     0
@@xyp_x_hi  EQU     1
@@xyp_y_lo  EQU     2
@@xyp_y_hi  EQU     3
            ENDS



;; ======================================================================== ;;
;;  MOB_ISR -- Update the MOB registers for the 8 MOBs.                     ;;
;; ======================================================================== ;;
MOB_ISR     PROC
            PSHR    R5

            ;; ------------------------------------------------------------ ;;
            ;;  Now for the time critical stuff.  When we get here, we      ;;
            ;;  have about 3600 cycles to do all the STIC/GRAM stuff.  We   ;;
            ;;  actually have a little more than that, but not much.        ;;
            ;; ------------------------------------------------------------ ;;


            ;; ------------------------------------------------------------ ;;
            ;;  Copy X, Y and A registers to STIC from the shadow if the    ;;
            ;;  flag says we need to.                                       ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #4,         R0      ;   8 \
            AND     MOB_FLG,    R0      ;  10  |  If bit 2 is clear, we 
            BEQ     @@skip_xya          ; 9/7  |- don't need STIC X/Y/A
            XOR     MOB_FLG,    R0      ;  10  |  copy for this frame
            MVO     R0,         MOB_FLG ;  11 /

            CLRR    R4                  ;   6
            MVII    #STICSH,    R5      ;   8
                                        ;----
                                        ;  60

            REPEAT  24
            MVI@    R5,         R0      ;   8 get a word from shadow
            MVO@    R0,         R4      ;   9 put a word to the STIC
            ENDR                        ;---- 
                                        ; 408 = (17 * 24)
                                        ;  60 (carried forward)
                                        ;----
                                        ; 468


@@skip_xya: 

            ;; ------------------------------------------------------------ ;;
            ;;  Copy collision registers out of STIC, clearing the ones     ;;
            ;;  that are there.  We actually do a merge, just in case not   ;;
            ;;  all the collisions were processed last frame.               ;;
            ;; ------------------------------------------------------------ ;;
@@c1        SET     $18                 ;   0
@@c2        SET     STICSH + 24         ;   0
            MVII    #$18,       R4      ;   8
            MVII    #STICSH+24, R5      ;   8
            CLRR    R2                  ;   6
                                        ;----
                                        ;  22
                                        ; 468 (carried forward)
                                        ;----
                                        ; 480

            REPEAT  8
            MVI     @@c1,       R0      ;  10 Read collision register
            MOVR    R0,         R1      ;   6 store a copy
            XOR     @@c2,       R1      ;  10 merge bits into shadow copy
            AND     @@c2,       R0      ;  10 some might be repeats
            XORR    R1,         R0      ;   6 make sure those stay merged
            MVO@    R2,         R4      ;   9 clear collision register
            MVO@    R0,         R5      ;   9 store merged collision to shadow
@@c1        SET     @@c1 + 1            ;   0
@@c2        SET     @@c2 + 1            ;   0
            ENDR                        ;----
                                        ; 480 = 60 * 8
                                        ; 480 (carried forward)
                                        ;----
                                        ; 960


@@pic_ok:
            ;; ------------------------------------------------------------ ;;
            ;;  Step through picture-index array, copying in GRAM cards.    ;;
            ;;  We only have enough time to copy a handful of cards each    ;;
            ;;  frame.  This means we might get some animations out of sync ;;
            ;;  if too much is moving per frame.                            ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #$3800+48*8,R5      ;   8 point to card #48 in GRAM
            MVII    #MOB_PIC,   R3      ;   8 pointer to picture number table
            MVII    #10,        R2      ;   8 maximum number of cards per frame
            MVII    #16,        R1      ;   8 up to 16 pictures to worry about
                                        ;----
                                        ;  32
                                        ; 960 (carried forward)
                                        ;----
                                        ; 992

@@pic_loop: MVI@    R3,         R0      ;   8 Get the picture number
            SUBI    #$80,       R0      ;   8 is the 'update flag' set?
            BMI     @@pic_next          ; 9/7 No?  Skip this one
            MVO@    R0,         R3      ;   9 Store picture number sans flag
                                        ;----
                                        ;  25 (branch taken)
                                        ;  32 (branch not taken)

            SLL     R0,         2       ;   8 Multiply picture number by 4
            MVII    #GFX_DATA,  R4      ;   8 
            ADDR    R0,         R4      ;   6 Index into graphics data
                                        ;----
                                        ;  22
                                        ;  32 (carried forward)
                                        ;----
                                        ;  54

            REPEAT  4
            MVI@    R4,         R0      ;   8 get 2 rows of data
            MVO@    R0,         R5      ;   9 store 1st row
            SWAP    R0                  ;   6
            MVO@    R0,         R5      ;   9 store 2nd row
            ENDR                        ;----
                                        ; 128 = 32*4
                                        ;  54 
                                        ;----
                                        ; 182

            DECR    R2                  ;   6 Have we reached our card limit?
            BEQ     @@pic_done          ; 9/7 yes:  leave
                                        ;----
                                        ;  13
                                        ; 182 (carried forward)
                                        ;----
                                        ; 195 (branch not taken)

            INCR    R3                  ;   6
            DECR    R1                  ;   6
            BNEQ    @@pic_loop          ;   9
                                        ;----
                                        ;  21 
                                        ; 195 (carried forward)
                                        ;----
                                        ; 216 per iteration w/ copy

            B       @@pic_done          ;   9

@@pic_next: ADDI    #8,         R5      ;   8
            INCR    R3                  ;   6
            DECR    R1                  ;   6
            BNEQ    @@pic_loop          ; 7/9
                                        ;----
                                        ;  29 
                                        ;  25 (carried forward)
                                        ;----
                                        ;  54 per iteration w/out copy

@@pic_done:
            ; worst case perf:  216*max_copy + 54*(16-max_copy)
            ;
            ; assume max_copy = 10:     ;2484 = 216*10 + 54*6
                                        ;   9 (lonely exit branch above)
                                        ; 997 (carried forward)
                                        ;----
                                        ;3490 we just fit.  :-)

            PULR    PC
            ENDP


;; ======================================================================== ;;
;;  MOB_STICSH -- Recalculate the STIC Shadow.  This could be called        ;;
;;                in the non-interrupt per-frame code.                      ;;
;; ======================================================================== ;;
MOB_STICSH  PROC
            PSHR    R5                  ;   9

            ;; ------------------------------------------------------------ ;;
            ;;  Step through attribute table, loading the STIC registers    ;;
            ;;  for each MOB.  We do this first, since STIC registers have  ;;
            ;;  the tightest timing constraints.  Note:  If we start to     ;;
            ;;  run out of cycles, we'll have to compute the STIC register  ;;
            ;;  image outside the ISR into a STIC shadow, and then block    ;;
            ;;  copy the shadow into the STIC.                              ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #2,         R0      ;   8 Do we need to recompute STICSH?
            AND     MOB_FLG,    R0      ;  10
            BNEQ    @@skip_sticsh       ; 9/7 If bit 1 set, then no.
            MVO     R0,         MOB_FLG ;  10 disable STICSH copying.

            MVII    #STICSH,    R2      ;   8 point R2 to STIC shadow
            MVII    #MOB_XYP+1, R3      ;   8 point R3 to MOB X/Y pos table
            MVII    #MOB_ATR,   R4      ;   8 point R4 to MOB attribute table
            MVII    #$980,      R1      ;   8 point to GRAM card #48
                                        ;----
                                        ;  76
@@sticsh_loop:                               
            MVI@    R4,         R0      ;   8 get attribute table index
            MVII    #ATR_DATA,  R5      ;   8 point to global attribute table
            ADDR    R0,         R5      ;   6 \
            ADDR    R0,         R5      ;   6  |- multiply by 3
            ADDR    R0,         R5      ;   6 /
                                             
            MVI@    R5,         R0      ;   8 get X register template
            ADD@    R3,         R0      ;   8 add in upper 8 from X position
            MVO@    R0,         R2      ;   9 store X register
            ADDI    #2,         R3      ;   8 point to upper 8 of Y position
            ADDI    #8,         R2      ;   8 point to Y register
                                             
            MVI@    R5,         R0      ;   8 get Y register template
            ADD@    R3,         R0      ;   8 add in upper 8 from Y position
            MVO@    R0,         R2      ;   9 store Y register
            ADDI    #2,         R3      ;   8 point to upper 8 of next X pos
            ADDI    #8,         R2      ;   8 point to A register
                                             
            MVI@    R5,         R0      ;   8 get A register value
            ADDR    R1,         R0      ;   6 offset to appropriate GRAM card
            MVO@    R0,         R2      ;   9 store A register
            SUBI    #15,        R2      ;   8 point to next X register
                                             
            ADDI    #16,        R1      ;   8 move by two GRAM cards per MOB
            CMPI    #STICSH+8,  R2      ;   8 are we at the end?
            BNEQ    @@sticsh_loop       ;   9
                                        ;----
                                        ;1376 = 172*8 cycles for loop
                                        ;  76 (carried forward)
            MVII    #6,         R0      ;   8 Flag: STICSH done, do STIC upd 
            MVO     R0,         MOB_FLG ;  11
                                        ;----
                                        ;1471

            
@@skip_sticsh
            PULR PC                     
            ENDP


;; ======================================================================== ;;
;;  MOB_COLDISP -- Dispatch for MOB collisions.                             ;;
;; ======================================================================== ;;
MOB_COLDISP PROC
            PSHR    R5

            ;; ------------------------------------------------------------ ;;
            ;;  Dispatch collisions that we found.  To avoid saturating     ;;
            ;;  the task queue, look at the task queue and decide how many  ;;
            ;;  collisions we can dispatch this frame.                      ;;
            ;; ------------------------------------------------------------ ;;
            MVI     TSKQHD,     R0      ;  10 \
            ADDI    #TSKQM-1,   R0      ;   8  |- find room in task queue,
            SUB     TSKQTL,     R0      ;  10 /   minus an extra slot.
            BLE     @@col_done          ; 9/7 if underflow, don't do any.
                                        ;----
                                        ;  43 (fallthru)


            MVII    #STICSH+24, R4      ;   8
            MVI     MOB_IGN,    R5      ;  10
            MVI     MOB_DIS,    R2      ;  10
            CLRR    R3                  ;   6
                                        ;----
                                        ;  34
                                        ;  43 (carried forward)
                                        ;----
                                        ;  77

@@col_loop:
            MVI@    R4,         R1      ;   8 get collision bits
            AND@    R5,         R1      ;   8 clear away ignored bits
            BEQ     @@col_next          ; 9/7 all ignored?  goto next MOB
                                        ;----
                                        ;  23 (fallthru)

            DECR    R4                  ;   6 \__ clear this MOB's
            MVO@    R3,         R4      ;   9 /   collision history

            PSHR    R0                  ;   9 \
            PSHR    R2                  ;   9  |
            PSHR    R4                  ;   9  |
            PSHR    R5                  ;   9  |
            MVI@    R2,         R0      ;   8  |  Queue dispatch for this
            JSRD    R5,         QTASK   ; 150  |- MOB's collision handler.
            PULR    R5                  ;  11  |  (cycle count approximate.)
            PULR    R4                  ;  11  |  
            PULR    R2                  ;  11  |
            PULR    R0                  ;  11 /

            DECR    R0                  ;   6 \__ make sure we don't saturate
            BEQ     @@col_done          ; 9/7 /   the task queue.
                                        ;----
                                        ; 
                                        ;  23 (carried forward)
                                        ;----
                                        ; 

@@col_next:
            INCR    R2                  ;   6
            CMPI    #STICSH+32, R4      ;   8
            BLT     @@col_loop          ; 7/9
                                        ;----
                                        ;  23 
                                        ;     (carried forward)
                                        ;----
                                        ;     = 8 *     worst case (8 iters)
                                        ;  77 (carried forward)
                                        ;----
                                        ;     worst case.
@@col_done:
            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  FLG_STICSH -- Flag that STICSH needs to be recomputed.                  ;;
;; ======================================================================== ;;
FLG_STICSH  PROC
            MVI     MOB_FLG,      R2    ; \
            ANDI    #$FFFF XOR 2, R2    ;  |- force STICSH update.
            MVO     R2,         MOB_FLG ; /   
            JR      R5
            ENDP


;; ======================================================================== ;;
;;  MOB_CREATE -- Initialize and activate a MOB                             ;;
;;                                                                          ;;
;;  INPUTS for MOB_CREATE                                                   ;;
;;      R5 -- Invocation record                                             ;;
;;              1 decle:   MOB number (0..7)                                ;;
;;              1 decle:   MOB X/Y position, packed                         ;;
;;              1 decle:   MOB attribute record.                            ;;
;;              1 decle:   MOB picture numbers, packed                      ;;
;;                                                                          ;;
;;  INPUTS for MOB_CREATE.1                                                 ;;
;;      R1 -- MOB # (0..7)                                                  ;;
;;      R5 -- Invocation record                                             ;;
;;              1 decle:   MOB X/Y position, packed                         ;;
;;              1 decle:   MOB attribute record.                            ;;
;;              1 decle:   MOB picture numbers, packed                      ;;
;;                                                                          ;;
;;  INPUTS for MOB_CREATE.2                                                 ;;
;;      R1 -- MOB # (0..7)                                                  ;;
;;      R2 -- MOB X/Y position, packed                                      ;;
;;      R5 -- Invocation record                                             ;;
;;              1 decle:   MOB attribute record.                            ;;
;;              1 decle:   MOB picture numbers, packed                      ;;
;;                                                                          ;;
;;  OUTPUT                                                                  ;;
;;      R1 -- MOB # * 4                                                     ;;
;;      R2, R3 -- trashed                                                   ;;
;;      R0, R5 -- unmodified                                                ;;
;;                                                                          ;;
;;  NOTES on packed input formats:                                          ;;
;;                                                                          ;;
;;      Packed picture numbers are stored with the 'upper half' picture     ;;
;;      number in the lower 8 bits, and the 'lower half' picture number     ;;
;;      in the upper 8 bits.  If there is no 'lower half' picture (eg.      ;;
;;      this is an 8x8 MOB), put $80 in that byte.                          ;;
;;                                                                          ;;
;;      Packed coordinates are stored w/ the X coordinate in the lower 8    ;;
;;      bits, and the Y coordinate in the upper 8 bits.  Valid range for    ;;
;;      the X coordinate is 0..255.  Valid range for the Y coordinate is    ;;
;;      0..127.                                                             ;;
;; ======================================================================== ;;
MOB_CREATE  PROC
            MVI@    R5,         R1      ; get MOB #
@@1:        MVI@    R5,         R2      ; get X, Y coordinates (packed)
@@2:        PSHR    R2
            MVI@    R5,         R2      ; get attribute #
            MVII    #MOB_ATR,   R3
            ADDR    R1,         R3      ; index into MOB attribute table
            MVO@    R2,         R3      ; store attribute #

            ADDI    #(MOB_PIC-MOB_ATR) AND $FFFF, R3  ; point to MOB pic tbl
            ADDR    R1,         R3      ; 2 bytes per MOB.
            MVI@    R5,         R2      ; get picture #s 

            XORI    #$8080,     R2      ; set 'need refresh' flag
            MVO@    R2,         R3      ; store upper card's picture #
            INCR    R3

            SWAP    R2
            BPL     @@no_lower          ; is lower card empty?

            MVO@    R2,         R3      ; store lower card's picture #
@@no_lower:
            ADDI    #(MOB_XYP-MOB_PIC-1) AND $FFFF, R3  ; point to MOB X/Y tbl
            SLL     R1,         1
            ADDR    R1,         R3      ; 4 bytes per MOB

            PULR    R2
            CLRR    R1
            MVO@    R1,         R3      ; fractional part of X is 0
            INCR    R3
            MVO@    R2,         R3      ; store integer part of X
            INCR    R3
            SWAP    R2
            MVO@    R1,         R3      ; fractional part of Y is 0
            INCR    R3
            MVO@    R2,         R3      ; store integer part of X

            B       FLG_STICSH          ; flag that STIC shadow update needed

            ENDP

;; ======================================================================== ;;
;;  MOB_DESTROY -- Destroys a given MOB.                                    ;;
;;                                                                          ;;
;;  INPUTS for MOB_DESTROY                                                  ;;
;;      R5 -- Invocation record                                             ;;
;;              1 decle:   MOB number (0..7)                                ;;
;;                                                                          ;;
;;  INPUTS for MOB_DESTROY.1                                                ;;
;;      R1 -- MOB # (0..7)                                                  ;;
;;                                                                          ;;
;;  OUTPUT                                                                  ;;
;;      R1 -- MOB # * 4                                                     ;;
;;      R2 -- trashed                                                       ;;
;;      R3 -- trashed                                                       ;;
;; ======================================================================== ;;
MOB_DESTROY PROC
            MVI@    R5,         R1      ; get MOB #

@@1:        
            MVII    #ATR.dead,  R2

            MVII    #MOB_ATR,   R3      ; point to attribute table
            ADDR    R1,         R3
            MVO@    R2,         R3      ; clear attribute

            MVII    #XYPAddr(0,x_hi),R3 ;
            SLL     R1,         2       ;
            ADDR    R1,         R3      ; 4 bytes per MOB
            MVO@    R2,         R3      ; zero out MSB of X coord only.

            B       FLG_STICSH          ; force STICSH update
            ENDP


;; ======================================================================== ;;
;;  MOB_MOVE -- Given a DX/DY, update the relative position for one MOB.    ;;
;;                                                                          ;;
;;  INPUTS for MOB_MOVE                                                     ;;
;;      R5 -- Invocation record                                             ;;
;;              1 decle:   DX (8.8 number)                                  ;;
;;              1 decle:   DY (8.8 number)                                  ;;
;;              1 decle:   MOB # (0..7)                                     ;;
;;                                                                          ;;
;;  INPUTS for MOB_MOVE.1                                                   ;;
;;      R2 -- DX (8.8 number)                                               ;;
;;      R3 -- DY (8.8 number)                                               ;;
;;      R5 -- Invocation record                                             ;;
;;              1 decle:   MOB # (0..7)                                     ;;
;;                                                                          ;;
;;  INPUTS for MOB_MOVE.2                                                   ;;
;;      R1 -- MOB # (0..7)                                                  ;;
;;      R2 -- DX (8.8 number)                                               ;;
;;      R3 -- DY (8.8 number)                                               ;;
;;                                                                          ;;
;;  OUTPUT                                                                  ;;
;; ======================================================================== ;;
MOB_MOVE    PROC
            MVI@    R5,     R2          ; get DX
            MVI@    R5,     R3          ; get DY
@@1:        MVI@    R5,     R1          ; get MOB update bitmap
@@2:        
            MVII    #MOB_XYP,R4         ; point to MOB XY position table
            SLL     R1,     2
            ADDR    R1,     R4

            SDBD                        ; \
            ADD@    R4,     R2          ;  |_ add X to DX and Y to DY.
            SDBD                        ;  |
            ADD@    R4,     R3          ; /


            SUBI    #4,     R4          ; rewind back to start of record

            MVOD@   R2,     R4          ; \
            ANDI    #$7FFF, R3          ;  |- Store updated X and Y 
            MVOD@   R3,     R4          ; /   Also, don't let Y above 127.

            B       FLG_STICSH          ; force STICSH update

            ENDP

;; ======================================================================== ;;
;;  MOB_MOVE_M -- Given a DX/DY, update the relative position for one or    ;;
;;                more MOBs.  This is useful when multiple MOBs form a      ;;
;;                single object.                                            ;;
;;                                                                          ;;
;;  INPUTS for MOB_MOVE_M                                                   ;;
;;      R5 -- Invocation record                                             ;;
;;              1 decle:   DX (8.8 number)                                  ;;
;;              1 decle:   DY (8.8 number)                                  ;;
;;              1 decle:   MOB update bitmap (1 bit per MOB#)               ;;
;;                                                                          ;;
;;  INPUTS for MOB_MOVE_M.1                                                 ;;
;;      R2 -- DX (8.8 number)                                               ;;
;;      R3 -- DY (8.8 number)                                               ;;
;;      R5 -- Invocation record                                             ;;
;;              1 decle:   MOB update bitmap (1 bit per MOB#)               ;;
;;                                                                          ;;
;;  INPUTS for MOB_MOVE_M.2                                                 ;;
;;      R1 -- MOB update bitmap (1 bit per MOB#)                            ;;
;;      R2 -- DX (8.8 number)                                               ;;
;;      R3 -- DY (8.8 number)                                               ;;
;;                                                                          ;;
;;  OUTPUT                                                                  ;;
;; ======================================================================== ;;
MOB_MOVE_M  PROC
            MVI@    R5,     R2          ; get DX
@@0:        MVI@    R5,     R3          ; get DY
@@1:        MVI@    R5,     R1          ; get MOB update bitmap
@@2:        PSHR    R5

            MOVR    R3,     R5
            MVII    #MOB_XYP-4,R4       ; point to MOB XY position table

@@loop1:    ADDI    #4,     R4
@@loop:
            SARC    R1
            BNC     @@skip

            MOVR    R2,     R0          ; copy DX to a scratch register
            MOVR    R5,     R3          ; copy DY to a scratch register

            SDBD                        ; \
            ADD@    R4,     R0          ;  |_ add X to DX and Y to DY.
            SDBD                        ;  |
            ADD@    R4,     R3          ; /

            SUBI    #4,     R4          ; rewind back to start of record

            MVOD@   R0,     R4          ; \
            ANDI    #$7FFF, R3          ;  |- Store updated X and Y 
            MVOD@   R3,     R4          ; /   Also, don't let Y above 127.

            TSTR    R1                  ; any more?
            BNEQ    @@loop
            PULR    R5
            B       FLG_STICSH          ; force STICSH update

@@skip      BNEQ    @@loop1

            PULR    R5
            B       FLG_STICSH          ; force STICSH update

            ENDP

;; ======================================================================== ;;
;;  MOB_GOTO -- Put a MOB at the specified X/Y.                             ;;
;;                                                                          ;;
;;  INPUTS for MOB_GOTO                                                     ;;
;;      R5 -- Invocation record                                             ;;
;;              1 decle:   X (8.8 number)                                   ;;
;;              1 decle:   Y (8.8 number)                                   ;;
;;              1 decle:   MOB # (0..7)                                     ;;
;;                                                                          ;;
;;  INPUTS for MOB_GOTO.1                                                   ;;
;;      R2 -- X (8.8 number)                                                ;;
;;      R3 -- Y (8.8 number)                                                ;;
;;      R5 -- Invocation record                                             ;;
;;              1 decle:   MOB # (0..7)                                     ;;
;;                                                                          ;;
;;  INPUTS for MOB_GOTO.2                                                   ;;
;;      R1 -- MOB # (0..7)                                                  ;;
;;      R2 -- X (8.8 number)                                                ;;
;;      R3 -- Y (8.8 number)                                                ;;
;;                                                                          ;;
;;  OUTPUT                                                                  ;;
;; ======================================================================== ;;
MOB_GOTO    PROC
            MVI@    R5,     R2          ; get X
            MVI@    R5,     R3          ; get Y
@@1:        MVI@    R5,     R1          ; get MOB #
@@2:        
            MVII    #MOB_XYP,R4         ; point to MOB XY position table
            SLL     R1,     2
            ADDR    R1,     R4

            MVOD@   R2,     R4          ; \
            ANDI    #$7FFF, R3          ;  |- Store updated X and Y 
            MVOD@   R3,     R4          ; /   Also, don't let Y above 127.

            B       FLG_STICSH          ; force STICSH update

            ENDP

;; ======================================================================== ;;
;;  MOB_GOTO_M -- Put one or more MOBs at a given X/Y location.             ;;
;;                This is useful when multiple MOBs form a single object.   ;;
;;                                                                          ;;
;;  INPUTS for MOB_GOTO_M                                                   ;;
;;      R5 -- Invocation record                                             ;;
;;              1 decle:   X (8.8 number)                                   ;;
;;              1 decle:   Y (8.8 number)                                   ;;
;;              1 decle:   MOB update bitmap (1 bit per MOB#)               ;;
;;                                                                          ;;
;;  INPUTS for MOB_GOTO_M.1                                                 ;;
;;      R2 -- X (8.8 number)                                                ;;
;;      R3 -- Y (8.8 number)                                                ;;
;;      R5 -- Invocation record                                             ;;
;;              1 decle:   MOB update bitmap (1 bit per MOB#)               ;;
;;                                                                          ;;
;;  INPUTS for MOB_GOTO_M.2                                                 ;;
;;      R1 -- MOB update bitmap (1 bit per MOB#)                            ;;
;;      R2 -- X (8.8 number)                                                ;;
;;      R3 -- Y (8.8 number)                                                ;;
;;                                                                          ;;
;;  OUTPUT                                                                  ;;
;; ======================================================================== ;;
MOB_GOTO_M  PROC
            MVI@    R5,     R2          ; get DX
@@0:        MVI@    R5,     R3          ; get DY
@@1:        MVI@    R5,     R1          ; get MOB update bitmap
@@2:        

            ANDI    #$7FFF, R3          ; don't let Y above 127.

            MVII    #MOB_XYP-4,R4       ; point to MOB XY position table
@@loop1:    ADDI    #4,     R4
@@loop:
            SARC    R1
            BNC     @@skip

            MVOD@   R2,     R4          ; \
            NOP                         ;  |- store updated X and Y
            MVOD@   R3,     R4          ; /

            NOP
            SWAP    R2
            SWAP    R3

            TSTR    R1                  ; any more?
            BNEQ    @@loop
            B       FLG_STICSH          ; force STICSH update

@@skip      BNEQ    @@loop1
            B       FLG_STICSH          ; force STICSH update

            ENDP

;; ======================================================================== ;;
;;  MOB_SETATR -- Change the attribute for a MOB                            ;;
;;                                                                          ;;
;;  INPUTS for MOB_SETATR                                                   ;;
;;      R5 -- Invocation record                                             ;;
;;              1 decle:   MOB # (0..7)                                     ;;
;;              1 decle:   Attribute value                                  ;;
;;                                                                          ;;
;;  INPUTS for MOB_SETATR.1                                                 ;;
;;      R1 -- MOB # (0..7)                                                  ;;
;;      R5 -- Invocation record                                             ;;
;;              1 decle:   Attribute value                                  ;;
;;                                                                          ;;
;;  INPUTS for MOB_SETATR.2                                                 ;;
;;      R1 -- MOB # (0..7)                                                  ;;
;;      R2 -- Attribute value                                               ;;
;;                                                                          ;;
;;  OUTPUT                                                                  ;;
;; ======================================================================== ;;
MOB_SETATR  PROC
            MVI@    R5,     R1          ; get MOB #
@@1:        MVI@    R5,     R2          ; get attribute #
@@2:        
            MVII    #MOB_ATR,R4         ; point to MOB XY position table
            ADDR    R1,     R4
            MVO@    R2,     R4

            B       FLG_STICSH          ; force STICSH update

            ENDP

;; ======================================================================== ;;
;;  MOB_SETATR_M -- Change the attribute for multiple MOBs                  ;;
;;                                                                          ;;
;;  INPUTS for MOB_SETATR_M                                                 ;;
;;      R5 -- Invocation record                                             ;;
;;              1 decle:   MOB update bitmap (1 bit per MOB#)               ;;
;;              N decle:   Attribute value                                  ;;
;;                                                                          ;;
;;  INPUTS for MOB_SETATR.1                                                 ;;
;;      R1 -- MOB update bitmap                                             ;;
;;      R5 -- Invocation record                                             ;;
;;              N decle:   Attribute value                                  ;;
;;                                                                          ;;
;;  OUTPUT                                                                  ;;
;; ======================================================================== ;;
MOB_SETATR_M PROC
            MVI@    R5,     R1          ; get MOB update bitmap
@@1:        

            MVII    #MOB_ATR,R4         ; point to MOB attribute bitmap
@@loop1:    ADDI    #4,     R4
@@loop:
            SARC    R1
            BNC     @@skip

            MVI@    R5,     R2          ; \__ copy attribute to MOB 
            MVO@    R2,     R4          ; /

            TSTR    R1                  ; any more?
            BNEQ    @@loop
            B       FLG_STICSH          ; force STICSH update

@@skip      BNEQ    @@loop1
            B       FLG_STICSH          ; force STICSH update

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
;* ======================================================================== *;
;*                   Copyright (c) 2003, Joseph Zbiciak                     *;
;* ======================================================================== *;
