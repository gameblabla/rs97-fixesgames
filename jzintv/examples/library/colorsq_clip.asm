;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;;==========================================================================;;
;;  Colored squares pixel-manipulation routines                             ;;
;;  Copyright 1999-2002, Joe Zbiciak.                                       ;;
;;                                                                          ;;
;;  Contained in this file:                                                 ;;
;;                                                                          ;;
;;      PIXELCLIP   -- Returns if C==1 if pixel is onscreen                 ;;
;;      PUTPIXELSC  -- Put a pixel if it's onscreen. Save/restore X/Y/C     ;;
;;      GETPIXELSC  -- Get a pixel if it's onscreen. Save/restore X/Y/C     ;;
;;                                                                          ;;
;;  This file requires the routines in colorsq.asm and colorsq_sv.asm.      ;;
;;==========================================================================;;

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
;;  GETPIXELSC                                                              ;;
;;  Calls GETPIXELS or returns R0 unchanged if pixel is off screen          ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Color to return if off-screen                                 ;;
;;      R1 -- X coordinate                                                  ;;
;;      R2 -- Y coordinate                                                  ;;
;;      R5 -- Return addr.                                                  ;;
;;                                                                          ;;
;;  OUTPUTS                                                                 ;;
;;      R0 -- Color                                                         ;;
;;      R1 -- X coordinate                                                  ;;
;;      R2 -- Y coordinate                                                  ;;
;;      R3, R4, R5 -- Clobbered.                                            ;;
;; ======================================================================== ;;
GETPIXELSC      PROC
        PSHR    R5              ; Save return address
        CALL    PIXELCLIP       ; See if pixel is onscreen
        ADCR    PC              ; If it is, then skip the return
        PULR    PC              ; Otherwise return.
        B       GETPIXELS.chain ; If onscreen chain over to GETPIXELS
        ENDP

;; ======================================================================== ;;
;;  PUTPIXELSC                                                              ;;
;;  Calls PUTPIXELS if the pixel is onscreen.                               ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Color                                                         ;;
;;      R1 -- X coordinate                                                  ;;
;;      R2 -- Y coordinate                                                  ;;
;;      R5 -- Return addr.                                                  ;;
;;                                                                          ;;
;;  OUTPUTS                                                                 ;;
;;      R0 -- Color                                                         ;;
;;      R1 -- X coordinate                                                  ;;
;;      R2 -- Y coordinate                                                  ;;
;;      R3, R4, R5 -- Clobbered.                                            ;;
;; ======================================================================== ;;
PUTPIXELSC      PROC
        PSHR    R5              ; Save return address
        CALL    PIXELCLIP       ; See if pixel is onscreen
        ADCR    PC              ; If it is, then skip the return
        PULR    PC              ; Otherwise return.
        B       PUTPIXELS.chain ; If onscreen chain over to GETPIXELS
        ENDP

;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;
