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
;;      PUTPIXELS   -- Put a pixel.  Saves and restores X, Y, and C         ;;
;;      GETPIXELS   -- Get a pixel.  Saves and restores X, Y, and C         ;;
;;                                                                          ;;
;;  This file requires the routines in colorsq.asm.                         ;;
;;==========================================================================;;

;; ======================================================================== ;;
;;  GLOBAL VARIABLES USED BY THESE ROUTINES                                 ;;
;;                                                                          ;;
;;  Note that some of these routines may use one or more global variables.  ;;
;;  If you use these routines, you will need to allocate the appropriate    ;;
;;  space in either 16-bit or 8-bit memory as appropriate.  Each global     ;;
;;  variable is listed with the routines which use it and the required      ;;
;;  memory width.                                                           ;;
;;                                                                          ;;
;;  Example declarations for these routines are shown below, commented out. ;;
;;  You should uncomment these and add them to your program to make use of  ;;
;;  the routine that needs them.  Make sure to assign these variables to    ;;
;;  locations that aren't used for anything else.                           ;;
;; ======================================================================== ;;

;; ======================================================================== ;;
;;  Note on X/Y/CSAVE:  The storage used for these only needs to be as      ;;
;;  wide as the values that you're passing into PUTPIXELS and GETPIXELS.    ;;
;; ======================================================================== ;;

                        ; Used by         Req'd Width   Description
                        ;-----------------------------------------------------
;CSAVE  EQU     $110    ; PUT/GETPIXELS   8 or 16-bit   Temp. storage
;XSAVE  EQU     $111    ; PUT/GETPIXELS   8 or 16-bit   Temp. storage
;YSAVE  EQU     $112    ; PUT/GETPIXELS   8 or 16-bit   Temp. storage
                        ;-----------------------------------------------------


;; ======================================================================== ;;
;;  GETPIXELS -- Calls GETPIXEL, but saves/restores X, Y                    ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
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
GETPIXELS       PROC
        PSHR    R5
@@chain:
        MVO     R1,     XSAVE
        MVO     R2,     YSAVE
        CALL    GETPIXEL
        MVI     XSAVE,  R1
        MVI     YSAVE,  R2
        PULR    PC
        ENDP


;; ======================================================================== ;;
;;  PUTPIXELS -- Calls PUTPIXEL, but saves/restores X, Y                    ;;
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
PUTPIXELS       PROC
        PSHR    R5
@@chain:
        MVO     R1,     XSAVE
        MVO     R2,     YSAVE
        MVO     R0,     CSAVE
        CALL    PUTPIXEL
        MVI     CSAVE,  R0
        MVI     XSAVE,  R1
        MVI     YSAVE,  R2
        PULR    PC
        ENDP

;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;
