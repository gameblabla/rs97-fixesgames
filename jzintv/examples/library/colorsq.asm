;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;;==========================================================================;;
;;  Colored squares pixel-manipulation routines.                            ;;
;;  Copyright 1999-2002, Joe Zbiciak.                                       ;;
;;                                                                          ;;
;;  Contained in this file:                                                 ;;
;;                                                                          ;;
;;      CSQ_MSKTBL  -- Pixel mask lookup table                              ;;
;;      CSQ_CLRTBL  -- Pixel color lookup table                             ;;
;;      PIXCALC     -- Calculates mask and pointer info for pixel           ;;
;;      PUTPIXEL    -- Put a pixel onscreen.  Trashes X/Y/C.                ;;
;;      PUTPIXELR   -- Put a pixel onscreen given mask & pointer.           ;;
;;      GETPIXEL    -- Get (read) a pixel onscreen.                         ;;
;;      GETPIXELR   -- GEt a pixel oncreen  given mask & pointer.           ;;
;;==========================================================================;;

;; ======================================================================== ;;
;;  CSQ_MSKTBL -- Pixel mask table for all four pixel positions             ;;
;; ======================================================================== ;;
CSQ_MSKTBL:
        DECLE   $0007, $0038, $01C0, $2600

;; ======================================================================== ;;
;;  CSQ_CLRTBL -- Color-lookup for all 8 colors.                            ;;
;; ======================================================================== ;;
CSQ_CLRTBL:
        DECLE   $0000, $0249, $0492, $06DB
        DECLE   $2124, $236D, $25B6, $27FF


;; ======================================================================== ;;
;;  PIXCALC -- Pixel Calculation:  Convert x,y into addr,mask               ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R1 -- X coordinate                                                  ;;
;;      R2 -- Y coordinate                                                  ;;
;;      R5 -- Return address                                                ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R1 -- Pixel mask                                                    ;;
;;      R2 -- Address in display memory.                                    ;;
;;      R3, R4 -- Clobbered.                                                ;;
;; ======================================================================== ;;
PIXCALC         PROC
        SLL     R2,     1       ; y' = y << 1
        MOVR    R2,     R4      ; mskidx = y << 1
        ANDI    #2,     R4      ; mskidx = (y << 1) & 2
        SUBR    R4,     R2      ; y' = (y & ~1) << 1
        MOVR    R2,     R3      ; save y'
        SLL     R2,     2       ; y'' = (y & ~1) << 3
        ADDR    R3,     R2      ; y''' = y' + y'' = (y >> 1) * 20
        SARC    R1,     1       ; c = x & 1;  x' = x >> 1
        ADCR    R4              ; mskidx = ((y << 1) & 2) + (x & 1)
        ADDR    R1,     R2      ; ofs = x' + y''' = (x >> 1) + (y >> 1) * 20
        ADDI    #$200,  R2      ; ptr = ofs + $200 = &BACKTAB[ofs] ==> R2
        ADDI    #CSQ_MSKTBL,R4  ; mskptr = MSKTBL + mskidx' = &MSKTBL[mskidx]
        MVI@    R4,     R1      ; mask = *mskptr ==> R1
        JR      R5              ; return

        ENDP

;; ======================================================================== ;;
;;  PUTPIXEL -- Put pixel at x, y coordinates with color 'c' (0-7)          ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Color                                                         ;;
;;      R1 -- X coordinate                                                  ;;
;;      R2 -- Y coordinate                                                  ;;
;;      R5 -- Return address                                                ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R1 -- Word stored at R2                                             ;;
;;      R2 -- Display memory address                                        ;;
;;      R0, R3, R4, R5 -- Clobbered.                                        ;;
;; ======================================================================== ;;

PUTPIXEL        PROC
        PSHR    R5              ; Save return address
        MVII    #PUTPIXELR+1,R5 ; Return to actual pixel draw below
        B       PIXCALC         ; Convert x,y into mask,addr

;; ======================================================================== ;;
;;  PUTPIXELR -- Put pixel 'raw': accepts address, mask, and color          ;;
;;               (Alternate entry point for PUTPIXEL.)                      ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Color                                                         ;;
;;      R1 -- Pixel mask                                                    ;;
;;      R2 -- Display memory address                                        ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R1 -- Word stored at R2                                             ;;
;;      R2 -- Display memory address                                        ;;
;;      R0,R5 -- Clobbered.                                                 ;;
;; ======================================================================== ;;
PUTPIXELR:
        PSHR    R5              ; Save return address
        MOVR    R0,     R5
        ADDI    #CSQ_CLRTBL,R5  ; clrptr = CLRTBL + c' = &CLRTBL[c]
        MVI@    R5,     R0      ; color = CLRTBL[c]
        ANDR    R1,     R0      ; color = color & mask  (select desired pix)
        COMR    R1              ; invert mask
        AND@    R2,     R1      ; pix = BACKTAB & ~mask (clear pixel)
        ADDR    R0,     R1      ; pix' = pix + color    (merge pixels)
        MVO@    R1,     R2      ; BACKTAB[ofs] = pix'   (write pixels)
        PULR    PC              ; return.

        ENDP

;; ======================================================================== ;;
;;  PIXELCLIP                                                               ;;
;;  Returns with C==1 if pixel is onscreen                                  ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R1 -- X coordinate                                                  ;;
;;      R2 -- Y coordinate                                                  ;;
;;      R5 -- Return address                                                ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      Carry == 1 if onscreen, 0 if offscreen.                             ;;
;; ======================================================================== ;;

PIXELCLIP       PROC
        TSTR    R1              ; Off left?
        BMI     @@offscr        ; Yes:  Set carry and exit
        TSTR    R2              ; Off top?
        BMI     @@offscr        ; Yes:  Set carry and exit
        CMPI    #39,    R1      ; Off right?
        BGT     @@offscr        ; Yes:  Set carry and exit
        CMPI    #23,    R2      ; Off bottom?
        BGT     @@offscr        ; Yes:  Set carry and exit
        SETC                    ; No to all: Clear carry and exit
        JR      R5
@@offscr
        CLRC
        JR      R5
        ENDP

;; ======================================================================== ;;
;;  GETPIXEL -- Get pixel at x, y address                                   ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R1 -- X coordinate                                                  ;;
;;      R2 -- Y coordinate                                                  ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0 -- Color                                                         ;;
;;      R2 -- Display memory address                                        ;;
;;      R1, R3, R4, R5 -- Clobbered.                                        ;;
;; ======================================================================== ;;
GETPIXEL        PROC
        PSHR    R5              ; Save return address
        MVII    #GETPIXELR+1,R5 ; Return to actual pixel read below
        B       PIXCALC         ; Convert x,y into mask,addr


;; ======================================================================== ;;
;;  GETPIXELR -- Get pixel 'raw': accepts address, mask                     ;;
;;               (Alternate entry point for getpixel)                       ;;
;;  INPUTS:                                                                 ;;
;;      R1 -- Pixel mask                                                    ;;
;;      R2 -- Display memory address                                        ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0 -- Color                                                         ;;
;;      R2 -- Display memory address                                        ;;
;;      R1, R5 -- Clobbered.                                                ;;
;; ======================================================================== ;;
GETPIXELR:
        PSHR    R5              ; Save return address
        MVI@    R2,     R0      ; pix = BACKTAB[ofs]
        ANDR    R1,     R0      ; pix' = pix & mask
        MOVR    R0,     R1      ;
        SWAP    R0,     1       ;
        ANDI    #$20,   R0      ;
        ADDR    R1,     R0      ; merge oddball bit from lower right pixel
        SLL     R1,     2       ;
        SWAP    R1,     1       ;
        ADDR    R1,     R0      ; Fold upper, lower pairs of pixels
        MOVR    R0,     R1      ;
        SLR     R0,     1       ;
        SLR     R0,     2       ;
        ADDR    R1,     R0      ; Fold left, right pixels
        ANDI    #7,     R0      ; Select remaining pixel
        PULR    PC              ; return

        ENDP

;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;
