;; ======================================================================== ;;
;;  Lines Demo                                                              ;;
;;                                                                          ;;
;;  This demo draws 8 parallel lines that go from the top of the screen     ;;
;;  to the bottom at pseudo random rates.  The lines are drawn as bitmaps   ;;
;;  from a pool of 48 cards that get reallocated in LRU order.  If a given  ;;
;;  card gets recycled, its old instance on the display gets replaced with  ;;
;;  a blank.                                                                ;;
;; ======================================================================== ;;


        INCLUDE "../macro/cart.mac"



;; ======================================================================== ;;
;;  ROM Header                                                              ;;
;; ======================================================================== ;;
        ROMSETUP    16K, 2010, "Lines Demo", START, 32

;; ======================================================================== ;;
;;  Global variables and settings.                                          ;;
;; ======================================================================== ;;
BPOOL   EQU         48
MAXLN   EQU         8

        WORDVAR     RSEED

        BYTEARRAY   REVMAP,     BPOOL

        BYTEARRAY   LRULIST,    2*BPOOL ; Least Recently Used list 
        BYTEVAR     LRUHEAD             ; Most Recently Used GRAM tile
        BYTEVAR     LRUTAIL             ; Least Recently Used GRAM tile

        BYTEVAR     CLRGCNT            
        BYTEARRAY   CLRGLST,    MAXLN   ; GRAM slots to clear

        BYTEVAR     CLRBCNT
        BYTEARRAY   CLRBLST,    MAXLN   ; BTAB locations to clear

        BYTEVAR     GRAMUCNT
        WORDARRAY   GRAMUPD,    2*MAXLN ; GRAM slots to update
        
        BYTEVAR     BTABUCNT
        WORDARRAY   BTABUPD,    2*MAXLN ; BTAB locations to update

        BYTEVAR     VBLFLAG             ; VBlank occurred counter.

        WORDARRAY   LINEPOS,    2*MAXLN ; Positions of the incoming lines
        BYTEARRAY   LINEVEL,    2*MAXLN ; Velocities of the incoming lines

;; ======================================================================== ;;
;;  Library includes                                                        ;;
;; ======================================================================== ;;
        INCLUDE     "../library/randfast.asm"

SHRTBL  PROC
@@_     SET         0
        REPEAT      16
        DECLE       1 SHL (15 - @@_)
@@_     SET         @@_ + 1
        ENDR
        ENDP

;; ------------------------------------------------------------------------ ;;
;;  BMPISR                                                                  ;;
;;                                                                          ;;
;;  This is where all the magic happens, updating the GRAM and BTAB based   ;;
;;  on the various queues that got set up outside the ISR.                  ;;
;; ------------------------------------------------------------------------ ;;
BMPISR  PROC

        MVO         R0,         $20     ; \_ enable display
        MVI         $21,        R0      ; /

        MVI         VBLFLAG,    R0      ; \
        INCR        R0                  ;  |- Signal ISR completion.
        MVO         R0,         VBLFLAG ; /

        ;; ---------------------------------------------------------------- ;;
        ;;  Clear out cards that got re-allocated.                          ;;
        ;; ---------------------------------------------------------------- ;;
        MVI         CLRGCNT,    R1
        TSTR        R1
        BEQ         @@skipcg
        CLRR        R0
        MVII        #CLRGLST,   R4
@@cgloop:
        MVI@        R4,         R2
        SLL         R2,         2
        SLL         R2,         1
        ADDI        #$3800,     R2
        MOVR        R2,         R5
        REPEAT      8
        MVO@        R0,         R5
        ENDR
        DECR        R1
        BNEQ        @@cgloop
@@skipcg


        ;; ---------------------------------------------------------------- ;;
        ;;  Update GRAM pixels for active lines                             ;;
        ;; ---------------------------------------------------------------- ;;

        MVI         GRAMUCNT,   R2
        MVII        #GRAMUPD,   R4
        TSTR        R2
        BEQ         @@skipgupd
@@gupd:
        MVI@        R4,         R3      ; get address to change
        MVI@        R4,         R0      ; get pixel to change
        MOVR        R0,         R1
        COMR        R1


        AND@        R3,         R1      ; \
        XORR        R0,         R1      ;  |- Merge in new pixel.
        MVO@        R1,         R3      ; /

        DECR        R2
        BNEQ        @@gupd
@@skipgupd

        ;; ---------------------------------------------------------------- ;;
        ;;  Clear BACKTAB locations that need to be cleared.                ;;
        ;; ---------------------------------------------------------------- ;;
        MVI         CLRBCNT,    R1
        MVII        #CLRBLST,   R4
        TSTR        R1
        BEQ         @@skipcb
        ;; R2 should be 0 here.
@@cbloop:
        MVI@        R4,         R3      ; Get BTAB offset
        ADDI        #$200,      R3      ; Offset => Screen address
        MVO@        R2,         R3      ; Clear BTAB location
        DECR        R1
        BNEQ        @@cbloop
@@skipcb

        ;; ---------------------------------------------------------------- ;;
        ;;  Update BACKTAB cards for reallocated cards.                     ;;
        ;; ---------------------------------------------------------------- ;;
        MVI         BTABUCNT,   R1
        MVII        #BTABUPD,   R4
        TSTR        R1
        BEQ         @@skipbupd
@@bupd:
        
        MVI@        R4,         R3      ; get location of update
        MVI@        R4,         R0      ; get value to write
        MVO@        R0,         R3      ; make update
        DECR        R1
        BNEQ        @@bupd
@@skipbupd

        ;; ---------------------------------------------------------------- ;;
        ;;  Reset update queues                                             ;;
        ;; ---------------------------------------------------------------- ;;
        ;; R2 should be 0 here.

        MVO         R2,         CLRGCNT
        MVO         R2,         CLRBCNT
        MVO         R2,         GRAMUCNT
        MVO         R2,         BTABUCNT


        ;; ---------------------------------------------------------------- ;;
        ;;  Increment VBlank flag.                                          ;;
        ;; ---------------------------------------------------------------- ;;
        MVI         VBLFLAG,    R0
        INCR        R0
        MVO         R0,         VBLFLAG

        B           $1014
        ENDP


;; ------------------------------------------------------------------------ ;;
;;  BMPINIT                                                                 ;;
;;                                                                          ;;
;;  This initializes all the bitmap-related structures.  For now, all of    ;;
;;  the cards in the pool get allocated to the first couple rows of the     ;;
;;  screen, filled with blanks.  This routine is intended to be called      ;;
;;  as an ISR.  It'll set up BMPISR as the ISR when it's done.              ;;
;; ------------------------------------------------------------------------ ;;
BMPINIT PROC

        ;; ---------------------------------------------------------------- ;;
        ;;  Set the STIC to color stack mode w/ black background, no bord.  ;;
        ;;  extension, etc., and no MOBs on display.                        ;;
        ;; ---------------------------------------------------------------- ;;
        MVI         $21,        R0          ; Color stack mode
        CLRR        R0
        MVII        #$28,       R4          ; \
        MVO@        R0,         R4          ;  |
        MVO@        R0,         R4          ;  |- Black color stack, border
        MVO@        R0,         R4          ;  |
        MVO@        R0,         R4          ;  |
        MVO@        R0,         R4          ; / 
        MVII        #$30,       R4          ; \
        MVO@        R0,         R4          ;  |_ No hdly/vdly/bord extension
        MVO@        R0,         R4          ;  |
        MVO@        R0,         R4          ; /
        CLRR        R4
        MVII        #32,        R1
@@mobclr:
        MVO@        R0,         R4          ; No MOBs
        DECR        R1
        BNEQ        @@mobclr

        ;; ---------------------------------------------------------------- ;;
        ;;  Reset the first BPOOL GRAM cards.                               ;;
        ;; ---------------------------------------------------------------- ;;
        MVII        #$3800,     R4
        MVII        #BPOOL,     R1
        CLRR        R0
@@gclr:
        REPEAT      8
        MVO@        R0,         R4
        ENDR
        DECR        R1
        BNEQ        @@gclr

        ;; ---------------------------------------------------------------- ;;
        ;;  Fill the first BPOOL BTAB positions with the first BPOOL GRAM   ;;
        ;;  cards.  Also, set up the reverse map.                           ;;
        ;; ---------------------------------------------------------------- ;;
        MVII        #$807,      R0
        MVII        #BPOOL,     R1
        MVII        #$200,      R4
        MVII        #REVMAP,    R5
@@bfill:
        MVO@        R4,         R5      ; initialize reverse map
        MVO@        R0,         R4      ; put card in BTAB
        ADDI        #8,         R0      ; update pattern for BTAB
        DECR        R1                  ; wash, rinse, repeat
        BNEQ        @@bfill

        ;; ---------------------------------------------------------------- ;;
        ;;  Clear the rest of BTAB.                                         ;;
        ;; ---------------------------------------------------------------- ;;
        CLRR        R0
        MVII        #240-BPOOL, R1
@@bclr:
        MVO@        R0,         R4
        DECR        R1
        BNEQ        @@bclr


        ;; ---------------------------------------------------------------- ;;
        ;;  Initialize the LRU so that 0 is MRU, 47 is LRU.                 ;;
        ;;  Each LRU has a prev/next pointer, except first and last         ;;
        ;;  entries, for which prev (first) and next (last) aren't          ;;
        ;;  meaningful.  For those, any old garbage will do.                ;;
        ;; ---------------------------------------------------------------- ;;
        MVII        #$FFFF,     R0      ; prev ptr
        MVII        #$0001,     R1      ; next ptr
        MVII        #LRULIST,   R4
        MVII        #BPOOL,     R2
@@lruinit:
        MVO@        R0,         R4      ; write prev ptr
        MVO@        R1,         R4      ; write next ptr
        INCR        R0
        INCR        R1
        DECR        R2
        BNEQ        @@lruinit

        MVII        #BPOOL-1,   R0      ; Set LRU to last element.
        MVO         R0,         LRUTAIL
        MVO         R2,         LRUHEAD

        ;; ---------------------------------------------------------------- ;;
        ;;  Clear all the update queues.                                    ;;
        ;; ---------------------------------------------------------------- ;;
        ;; R2 should be 0 here
        MVO         R2,         CLRGCNT
        MVO         R2,         CLRBCNT
        MVO         R2,         GRAMUCNT
        MVO         R2,         BTABUCNT


        ;; ---------------------------------------------------------------- ;;
        ;;  Set up bitmap ISR as our current ISR.                           ;;
        ;; ---------------------------------------------------------------- ;;
        MVII        #BMPISR,    R0
        MVO         R0,         $100
        SWAP        R0
        MVO         R0,         $101

        B           $1014
        ENDP


;; ------------------------------------------------------------------------ ;;
;;  PLOT                                                                    ;;
;;                                                                          ;;
;;  Given an X/Y coordinate, schedule a pixel to be plotted.  This does     ;;
;;  all the work of figuring out the card that needs updating allocating    ;;
;;  a card if necessary, etc. etc.                                          ;;
;;                                                                          ;;
;;  INPUTS                                                                  ;;
;;      R0  X coordinate                                                    ;;
;;      R1  Y coordinate                                                    ;;
;;                                                                          ;;
;; ------------------------------------------------------------------------ ;;
        BYTEVAR     XTMP
        BYTEVAR     YTMP
        WORDVAR     GTMP

PLOT    PROC
        PSHR        R5
        NOP
        MVO         R0,         XTMP
        MVO         R1,         YTMP

        ;; ---------------------------------------------------------------- ;;
        ;;  Look up what GRAM card we're updating, or allocate one.         ;;
        ;; ---------------------------------------------------------------- ;;
        SLR         R0,         2       ; \_ X / 8 to get card column
        SLR         R0,         1       ; /

        ANDI        #NOT 7,     R1      ; Clamp Y to card row => Row * 8
        SLL         R1,         1       ; Row * 16
        ADDR        R1,         R0      ;
        SLR         R1,         2       ; Row * 4
        ADDR        R0,         R1
        ADDI        #$200,      R1      ; BTAB index

        CMPI        #$2F0,      R1      ; Refuse to plot off-screen pixels
        BC          @@leave

        MVI@        R1,         R0      ; Get BTAB word
        ANDI        #$FF8,      R0

        CMPI        #$800,      R0      ; \   Determine if we need to alloc    
        BLT         @@alloc_gram        ;  |_ a new GRAM card, or just update
        CMPI        #$800 + BPOOL*8, R0 ;  |  the LRU.
        BLT         @@update_lru        ; /   

@@alloc_gram:
        ;; R0 = garbage
        ;; R1 = BTAB address ($200 - $2EF)

        MVI         LRUTAIL,    R2      ; Who's the unlucky sod?

        ; Reverse map to BTAB
        MVII        #REVMAP,    R3      ; \   
        ADDR        R2,         R3      ;  |- use the reverse map to find 
        MVI@        R3,         R5      ; /   reaped card so we can clear it
        MVO@        R1,         R3      ; Update reverse map

        ; Schedule removing old card from BTAB
        MVI         CLRBCNT,    R4      ; \
        INCR        R4                  ;  |
        MVO         R4,         CLRBCNT ;  |- Add BTAB card to BTAB clear list
        ADDI        #CLRBLST-1, R4      ;  |  
        MVO@        R5,         R4      ; /

        ; Mark card as reaped in BTAB
        ADDI        #$200,      R5
        MVI@        R5,         R4
        ADDI        #$200,      R4
        DECR        R5
        MVO@        R4,         R5

        ; Schedule removing old tile from GRAM
        MVI         CLRGCNT,    R4
        INCR        R4
        MVO         R4,         CLRGCNT
        ADDI        #CLRGLST-1, R4
        MVO@        R2,         R4
        
        ; Turn newly allocated GRAM card # into GRAM offset
        MOVR        R2,         R0      ; \
        SLL         R0,         2       ;  |_ Turn GRAM card # into GRAM
        SLL         R0,         1       ;  |  offset.
        ADDI        #$800,      R0      ; /

        ; Merge in display attributes
        MOVR        R0,         R5      ; \_ XXX: For now draw all the pixels
        ADDI        #$7,        R5      ; /  in white.

        ; Schedule drawing new card in BTAB
        MVI         BTABUCNT,   R3      ; \
        INCR        R3                  ;  |
        MVO         R3,         BTABUCNT;  |- Add new card to BTAB update list
        SLL         R3                  ;  |
        ADDI        #BTABUPD-2, R3      ;  |
        MVO@        R1,         R3      ;  |  (screen address)
        INCR        R3                  ;  |
        MVO@        R5,         R3      ; /   (display word)

        ; Mark it allocated but leave it black in BTAB
        MVO@        R0,         R1

        ; Update the LRU list so that newly alloc'd card is MRU 
        MVI         LRUHEAD,    R1      ; \
        MOVR        R1,         R4      ;  |_ Make the new MRU the prev of the
        SLL         R1                  ;  |  old MRU
        ADDI        #LRULIST,   R1      ;  |
        MVO@        R2,         R1      ; /
        MVO         R2,         LRUHEAD ; Make us the new MRU

        MOVR        R2,         R3      ; \
        SLL         R3                  ;  |  
        ADDI        #LRULIST,   R3      ;  |- Make new MRU's old prev the new
        MVI@        R3,         R1      ;  |  LRU.
        MVO         R1,         LRUTAIL ; /
        INCR        R3                  ; \__ ...and make old MRU the new
        MVO@        R4,         R3      ; /   MRU's next

        B           @@done_alloc


@@update_lru:
        ;; R0 = ((GRAM card #) SHL 3) + $800
        ;; R1 = BTAB address ($200 - $2EF)

        MOVR        R0,         R2
        ANDI        #$7F8,      R2

        SLR         R2,         2       ;  
        MVII        #LRULIST,   R4      ; \_ R4 points to this card's LRU
        ADDR        R2,         R4      ; /  entry
        SLR         R2,         1       ; R2 = GRAM card #

        MVI         LRUHEAD,    R1      ; \
        CMPR        R1,         R2      ;  |- Skip LRU update if already MRU
        BEQ         @@skip_lru          ; /

        MOVR        R1,         R5      ; remember old MRU

        SLL         R1,         1       ; \ 
        ADDI        #LRULIST,   R1      ;  |- Make new MRU the prev of 
        MVO@        R2,         R1      ; /   the old MRU
        MVO         R2,         LRUHEAD ; Make R2 the new MRU

        MVI@        R4,         R1      ; \__ Get prev/next pointers for 
        MVI@        R4,         R3      ; /   new MRU

        DECR        R4
        MVO@        R5,         R4      ; Make old MRU next of new MRU

        CMP         LRUTAIL,    R2      ; Were we the current tail?
        BNEQ        @@not_tail

        MVO         R1,         LRUTAIL ; Yes:  Make curr->prev new tail
        B           @@done_alloc

@@not_tail                              ; No:  Unhook us from middle of list
        MOVR        R1,         R4
        SLL         R1                  ; \
        ADDI        #LRULIST+1, R1      ;  |- curr->prev->next = curr->next
        MVO@        R3,         R1      ; /

        SLL         R3                  ; \
        ADDI        #LRULIST,   R3      ;  |- curr->next->prev = curr->prev
        MVO@        R4,         R3      ; /

@@skip_lru:
@@done_alloc:

        ;; ---------------------------------------------------------------- ;;
        ;;  Now that a GRAM card is looked up or allocated, convert to a    ;;
        ;;  pointer to the row to update and an update mask.                ;;
        ;;                                                                  ;;
        ;;  R0 should have the GRAM offset.                                 ;;
        ;; ---------------------------------------------------------------- ;;
        MVI         YTMP,       R1      ; Get Y offset
        ANDI        #7,         R1      ; Get just the 3 LSBs for row offset
        ADDR        R1,         R0      ; Add it to GRAM offset
        ADDI        #$3000,     R0      ; Turn it to GRAM pointer

        MVI         GRAMUCNT,   R1
        INCR        R1
        MVO         R1,         GRAMUCNT
        SLL         R1
        ADDI        #GRAMUPD-2, R1
        MVO@        R0,         R1      ; Write out GRAM update address
        INCR        R1

        MVI         XTMP,       R2
        ANDI        #7,         R2
        ADDI        #SHRTBL+8,  R2
        MVI@        R2,         R2
        MVO@        R2,         R1      ; Write out GRAM update mask.
@@leave:
        PULR        PC
        ENDP

;; ------------------------------------------------------------------------ ;;
;;  WAITVBL                                                                 ;;
;; ------------------------------------------------------------------------ ;;
MACRO   WAITVBL
        MVI         VBLFLAG,    R0
        CMP         VBLFLAG,    R0
        BEQ         $ - 2
ENDM

;; ------------------------------------------------------------------------ ;;
;;  START                                                                   ;;
;; ------------------------------------------------------------------------ ;;
START   PROC
        EIS 

        ;; ---------------------------------------------------------------- ;;
        ;;  Start all the lines off the bottom of the screen.               ;;
        ;; ---------------------------------------------------------------- ;;
        MVII        #$7FFF,     R0
        MVII        #LINEPOS,   R4
        MVII        #MAXLN*2,   R1
@@lineinit:
        MVO@        R0,         R4
        DECR        R1
        BNEQ        @@lineinit

        ;; ---------------------------------------------------------------- ;;
        ;;  Initialize the bitmap engine.                                   ;;
        ;; ---------------------------------------------------------------- ;;
        MVII        #BMPINIT,   R0
        MVO         R0,         $100
        SWAP        R0 
        MVO         R0,         $101


        ;; ---------------------------------------------------------------- ;;
        ;;  Main loop:                                                      ;;
        ;;   -- Advance all the lines.  If any line goes off-screen,        ;;
        ;;      re-initialize it with a random X position and vel.          ;;
        ;;   -- Plot the lines whose integral position changed.             ;;
        ;;      (Ok, for now just plot them all always.)                    ;;
        ;;   -- Wait for VBlank.                                            ;;
        ;; ---------------------------------------------------------------- ;;
@@main_loop:
        WAITVBL

        MVII        #LINEPOS,   R4
        MVII        #LINEVEL,   R5
        MVII        #MAXLN,     R2
@@vel_loop:
        MVI@        R5,         R0
        SLL         R0
        ADD@        R4,         R0
        SUBI        #$100,      R0
        CMPI        #160 SHL 8, R0
        BC          @@realloc_x
        DECR        R4
        MVO@        R0,         R4

        MVI@        R5,         R0
        ADD@        R4,         R0
        CMPI        #96 SHL 8,  R0
        BNC         @@ok
@@realloc_y:
        DECR        R4
        DECR        R5
@@realloc_x:
        DECR        R4
        DECR        R5

        PSHR        R5
        CALL        RANDFAST
        CALL        RANDFAST
        CALL        RANDFAST
        CALL        RANDFAST
        ANDI        #$7FFF,     R0
        ADDI        #$1000,     R0
        MVO@        R0,         R4      ; new random X (16 .. 143)
        CLRR        R0
        MVO@        R0,         R4      ; Y == 0
        
        MOVR        R4,         R3
        PULR        R4                  ; Get velocity pointer
        CALL        RANDFAST
        CALL        RANDFAST

        MVO@        R0,         R4      ; new random X velocity

        CALL        RANDFAST
        CALL        RANDFAST

        ANDI        #$3F,       R0
        ADDI        #$C0,       R0
        MVO@        R0,         R4      ; new, positive Y velocity

        MOVR        R4,         R5      ; put vel pointer back
        MOVR        R3,         R4      ; put pos pointer back

        B           @@next

@@ok    DECR        R4
        MVO@        R0,         R4
@@next: DECR        R2
        BNEQ        @@vel_loop


        MVII        #LINEPOS,   R4
        MVII        #MAXLN,     R2
@@plot_loop:
        MVI@        R4,         R0
        MVI@        R4,         R1

        
        SWAP        R0
        SWAP        R1

        ANDI        #$FF,       R0
        ANDI        #$FF,       R1

        PSHR        R4
        PSHR        R2
        JSRD        R5,         PLOT
        EIS
        PULR        R2
        PULR        R4

        DECR        R2
        BNEQ        @@plot_loop

        B           @@main_loop


        ENDP

