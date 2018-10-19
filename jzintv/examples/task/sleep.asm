;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;;==========================================================================;;
;; Joe Zbiciak's Task Routines:  Timer-based Task SLEEP/SPAWN support       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
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
;;                                                                          ;;
;;  NOTES                                                                   ;;
;;   -- TSKTBL is formatted as described in 'timer.asm'                     ;;
;;   -- SLEEP_REGSV needs room for 5 words.                                 ;;
;;                                                                          ;;
;; ======================================================================== ;;

                       ; Used by       Req'd Width  Description
                       ;------------------------------------------------------
;TSKTBL      EQU $310  ; DOTIMER/etc   16-bit       Timer-task table
;SLEEP_REGSV EQU $310  ; SLEEP/SPAWN   16-bit       Timer-task table


;; ======================================================================== ;;
;;  SLEEP                                                                   ;;
;;      Causes a one-shot process to sleep by scheduling a second one-shot  ;;
;;      process to reawake it.  This is VERY TRICKY TO USE, and can be      ;;
;;      used by ONE PROCESS ONLY at a time, because there is only one       ;;
;;      register save area (SLEEP_REGSV).                                   ;;
;;                                                                          ;;
;;  SPAWN                                                                   ;;
;;      Same as sleep, only the sleeping task isn't a one-shot.  It keeps   ;;
;;      retriggering automatically at the designated interval, or can be    ;;
;;      retriggered manually if the period is odd.                          ;;
;;                                                                          ;;
;;      (NOTE: SLEEP and SPAWN point to the same place.  The different      ;;
;;      names are provided to make your code easier to read.)               ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R5 -- Invocation record, in following format:                       ;;
;;            Period/sleep time  1 DECLE  (ODD for SLEEP, EVEN for SPAWN)   ;;
;;            Process # to use   1 DECLE                                    ;;
;;      Top of Stack -- Return addr from _this_ process back to scheduler.  ;;
;;      Task will wake at address after invocation record.                  ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0, R1, R2, R3, R4 restored                                         ;;
;;      R5 points to your return address                                    ;;
;;      Top of stack has return address for scheduler                       ;;
;; ======================================================================== ;;
SLEEP:      PROC
SPAWN:     
            MVO     R4,     SLEEP_REGSV+4 ; Save R4
            MVII    #SLEEP_REGSV, R4
            MVO@    R0,     R4      ; Save R0
            MVO@    R1,     R4      ; Save R1
            MVO@    R2,     R4      ; Save R2
            MVO@    R3,     R4      ; Save R3
           
            MVI@    R5,     R2      ; Get sleep time
            MVI@    R5,     R3      ; Get PID
           
            SLL     R3,     2       ; R3 = R3 * 4 (four words/entry)
            MVII    #TSKTBL,R4      ; R4 = &TSKTBL[0]
            ADDR    R3,     R4      ; R4 = &TSKTBL[n]
            MVII    #@@wake,R0
            MVO@    R0,     R4      ; Address to wake up at
            MVO@    R5,     R4      ; Instance data (R5 restore)
            MVO@    R2,     R4
            MVO@    R2,     R4
           
            PULR    PC
           
@@wake:     PSHR    R5              ; Remember return address
            MOVR    R2,     R5
            MVII    #SLEEP_REGSV,R4 ; Point to register save area
            MVI@    R4,     R0      ; Restore R0
            MVI@    R4,     R1      ; Restore R1
            MVI@    R4,     R2      ; Restore R2
            MVI@    R4,     R3      ; Restore R3
            MVI@    R4,     R4      ; Restore R4
           
            JR      R5              ; Go there
           
            ENDP

;; ======================================================================== ;;
;;  End of File:  sleep.asm                                                 ;;
;; ======================================================================== ;;
