;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2010                                     *;
;* ======================================================================== *;

;;==========================================================================;;
;; Joe Zbiciak's Task Routines:  Pausing/unpausing timer-based tasks        ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

;; ======================================================================== ;;
;;  PAUSE_TASK   -- Pauses a timer based task                               ;;
;;  UNPAUSE_TASK -- Unpauses a timer based task                             ;;
;;                                                                          ;;
;;  The timer code treats negative timer values as inactive tasks.  So,     ;;
;;  to pause a task, we merely need to make its count look negative.        ;;
;;  That could be as simple as just manipulating bit 15.                    ;;
;;                                                                          ;;
;;  But what about tasks that are already inactive?  Pausing an inactive    ;;
;;  task should leave it inactive, and unpausing it should not reactivate   ;;
;;  it.  (That's what "RETRIGGERTASK" is for.)                              ;;
;;                                                                          ;;
;;  So, this code takes a unique approach:  It copies bit 15 to bit 14      ;;
;;  in the timer period when pausing, and then copies bit 14 back to        ;;
;;  bit 15 when unpausing.  This is perfectly fine for period values up     ;;
;;  to $3FFF.  That's about 136 seconds (just over 2 minutes).              ;;
;;                                                                          ;;
;;  PAUSE_TASK                                                              ;;
;;                                                                          ;;
;;  INPUTS                                                                  ;;
;;      R3 -- Task number                                                   ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      Task is paused (bit 15 -> bit 14; 1 -> bit 15)                      ;;
;;      R0 -- Resulting delay value for task                                ;;
;;      R3 -- Points to task delay value                                    ;;
;;                                                                          ;;
;; ======================================================================== ;;
PAUSE_TASK      PROC
                SLL     R3,     2       ; R3 = R3 * 4 (four words/entry)
                ADDI    #TSKTBL+2, R3   ; R3 = &TSKTBL[n].delay
                DIS                     ; --> Begin critical section
                MVI@    R3,     R0
                SLLC    R0,     2       ; \
                RRC     R0,     1       ;  |_ Move bit 15 to bit 14.
                SETC                    ;  |  Put 1 in bit 15.
                RRC     R0,     1       ; /
                MVO@    R0,     R3
                EIS                     ; <-- end critical section
                JR      R5
                ENDP

;; ======================================================================== ;;
;;  UNPAUSE_TASK                                                            ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R3 -- Task number                                                   ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      Task is unpaused (but may still be inactive)                        ;;
;;          (bit 14 -> bit 15; 0 -> bit 14)                                 ;;
;;      R0 -- Resulting delay value for task                                ;;
;;      R3 -- Points to task delay value                                    ;;
;; ======================================================================== ;;
UNPAUSE_TASK    PROC
                SLL     R3,     2
                ADDI    #TSKTBL+2, R3   ; R3 = &TSKTBL[n].delay
                MVII    #$7FFF, R0      ; 0 -> Bit 15
                DIS                     ; --> Begin critical section
                AND@    R3,     R0
                SLLC    R0,     2       ; \_ Swap bits 14, 15
                RRC     R0,     2       ; /
                MVO@    R0,     R3
                EIS                     ; <-- End critical section
                JR      R5
                ENDP

;; ======================================================================== ;;
;;  End of File:  pause.asm                                                 ;;
;; ======================================================================== ;;
