;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;;==========================================================================;;
;; Joe Zbiciak's Task Routines:  Timer-based Tasks                          ;;
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
;;   -- TSKQHD and TSKQTL must be initialized to 0.                         ;;
;;                                                                          ;;
;;   -- The symbol OVRFLO isn't required.  If it's not available, then      ;;
;;      the code won't count overflows.                                     ;;
;;                                                                          ;;
;;   -- The symbol WTIMER isn't required.  If it's not available, then      ;;
;;      the function WAIT will not be included.                             ;;
;;                                                                          ;;
;;   -- The TSKQ must have room for TSKQM + 1 words.  That is, if TSKQM     ;;
;;      is 15, then TSKQ must have room for 16 words.                       ;;
;;                                                                          ;;
;;   -- The TSKDQ must have room for 2*(TSKQM + 1) bytes.  That is, if      ;;
;;      is 15, then TSKDQ must have room for 32 bytes.                      ;;
;;                                                                          ;;
;;   -- The TSKTBL must have room for MAXTSK*4 words.                       ;;
;; ======================================================================== ;;

                       ; Used by       Req'd Width  Description
                       ;------------------------------------------------------
;MAXTSK EQU     4      ; DOTIMER/etc   EQUATE       Maximum # of timer tasks
;OVRFLO EQU     $130   ; DOTIMER       8-bit        Number of queue overflows
;WTIMER EQU     $131   ; DOTIMER/WAIT  8-bit        Number of queue overflows
;TSKQHD EQU     $132   ; DOTIMER/etc   8-bit        Task queue head
;TSKQTL EQU     $133   ; DOTIMER/etc   8-bit        Task queue tail
;TSKQ   EQU     $300   ; DOTIMER/etc   16-bit       Task queue
;TSKDQ  EQU     $140   ; DOTIMER/etc   8-bit        Task data queue
;TSKQM  EQU     $F     ; DOTIMER/etc   EQUATE       Task queue length mask
;TSKACT EQU     $134   ; DOTIMER/etc   8-bit        # of active timer tasks
;TSKTBL EQU     $310   ; DOTIMER/etc   16-bit       Timer-task table


;; ======================================================================== ;;
;;  DOTIMER                                                                 ;;
;;                                                                          ;;
;;  This function handles all the timer-based tasks.  It provides support   ;;
;;  for periodically-occuring events.                                       ;;
;;                                                                          ;;
;;  >>  NOTE:  Call this early in your ISR, or call it with interrupts  <<  ;;
;;  >>  disabled!  This code is NON-REENTRANT.                          <<  ;;
;;                                                                          ;;
;;  This routine counts down task timers and schedules tasks.  We have up   ;;
;;  to MAXTSK active timer-based tasks at one time.  The tasks may be a     ;;
;;  mixture of one-shot and periodic tasks.  The following describes the    ;;
;;  timer evaluation logic that is applied to each task.                    ;;
;;                                                                          ;;
;;   -- If the count is initially <= 0, the task is inactive and            ;;
;;      is skipped.                                                         ;;
;;                                                                          ;;
;;   -- The counter value is decremented by 2.  If the count goes to        ;;
;;      zero, the task is triggered and its count is reinitialized.         ;;
;;      Thus, tasks w/ even periods are periodic tasks.                     ;;
;;                                                                          ;;
;;   -- If the count goes negative, the task is triggered and its           ;;
;;      count is not reinitialized.  Thus, tasks w/ odd periods are         ;;
;;      one-shot tasks.                                                     ;;
;;                                                                          ;;
;;  This arrangement allows us to have one-shot and repeating tasks with    ;;
;;  minimal overhead.  Repeating tasks clear bit 0 of their period count,   ;;
;;  and one-shot tasks set bit 0.                                           ;;
;;                                                                          ;;
;;  When tasks are triggered, their procedure address is written to the     ;;
;;  task queue.  The task queue is drained by RUNQ routine in "taskq.asm".  ;;
;;                                                                          ;;
;;  The task queue is a small circular buffer in 16-bit memory with head    ;;
;;  and tail pointers in 8-bit memory.  The circular buffer is 16 entries   ;;
;;  large by default.  You can make it larger or smaller by changing the    ;;
;;  macro TSKQM.  This code queues the task directly.  It does not call     ;; 
;;  QTASK.                                                                  ;;
;;                                                                          ;;
;;  If the task queue overflows, the overflowing task is dropped.  If the   ;;
;;  symbol OVRFLO is defined, overflows are tallied to that variable.       ;;
;;  Also, a task-queue overflow will cause DOTIMER to not count-down        ;;
;;  the rest of the tasks in the task table during this tick.               ;;
;;                                                                          ;;
;;                                                                          ;;
;;  WAIT TIMER SUPPORT                                                      ;;
;;                                                                          ;;
;;  If WTIMER is defined, this routine will also count down the busy-       ;;
;;  wait timer used by WAIT.                                                ;;
;;                                                                          ;;
;;                                                                          ;;
;;  TASK CONTROL TABLE ENTRY LAYOUT                                         ;;
;;                                                                          ;;
;;      Word 0:  Function Pointer for task                                  ;;
;;      Word 1:  Instance data (passed in R1 to task)                       ;;
;;      Word 2:  Current down-count                                         ;;
;;      Word 3:  Counter reinitialization value.                            ;;
;;                                                                          ;;
;;==========================================================================;;
DOTIMER     PROC
            PSHR    R5              ; Save return address

            MVI     TSKACT, R0      ; Iterate over active tasks
            TSTR    R0
            BEQ     @@notasks
            MVII    #TSKTBL+2, R3   ; Point to the task control table

@@taskloop: MVI@    R3,     R1      ; Get tasks's current tick count
            TSTR    R1              ; Is it <= 0?
            BLE     @@nexttask      ; ... yes? Skip it.

    
            SUBI    #2,     R1      ; Count it down.
            BNEQ    @@noreinit      ; If it went to zero, reinit count
    
            INCR    R3
            MVI@    R3,     R1      ; Get new period for task
            DECR    R3
            SUBR    R2,     R2      ; Z=1, S=0 (causes the BGT to not be taken)

@@noreinit: MVO@    R1,     R3      ; Store new period.
            BGT     @@nexttask      ; If this count didn't expire, ...
                                    ;  ... go to the next task
@@q:
            ; Schedule this task by adding it to the task queue
            MOVR    R3,     R4
            SUBI    #2,     R4
            MVI@    R4,     R2      ; Get task function pointer
            MVI     TSKQHD, R1      ; Get task queue head
            INCR    R1              ; Move to next slot
            ANDI    #TSKQM, R1      ; Stay within circ buffer
            CMP     TSKQTL, R1
            BEQ     @@overflow      ; Drop this task if queue overflows. (!)

            MOVR    R1,     R5
            ADDR    R1,     R5      ; R5 is for task data queue
            ADDI    #TSKDQ, R5      ; Point to task data queue

            MVO     R1,     TSKQHD  ; Store updated task queue head

            ADDI    #TSKQ,  R1      ; Point to actual task queue
            MVO@    R2,     R1      ; Store the function pointer
            MVI@    R4,     R2      ; Get task instance data
            MVO@    R2,     R5      ; Store low half of instance data
            SWAP    R2,     1
            MVO@    R2,     R5      ; Store high half of instance data

@@nexttask: ADDI    #4,     R3      ; Point to next task struct
            DECR    R0
            BNEQ    @@taskloop      ; Loop until done with all tasks
        IF  DEFINED OVRFLO
            B       @@notasks

@@overflow:
            MVI     OVRFLO, R0
            INCR    R0
            MVO     R0,     OVRFLO
        ELSE
@@overflow:
        ENDI

@@notasks:

        IF  DEFINED WTIMER
@@wtimer:
            ;; Count down the wait-timer, if there is one
            MVI     WTIMER, R5
            DECR    R5
            BMI     @@wt_expired
            MVO     R5,     WTIMER
@@wt_expired:
        ENDI

            PULR    PC
            ENDP




;; ======================================================================== ;;
;;  STARTTASK                                                               ;;
;;      Puts a task into a task slot for general timer-based scheduling.    ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R5 -- Invocation record:                                            ;;
;;              DECLE -- Task number                                        ;;
;;                                                                          ;;
;;              DECLE -- Task function pointer                              ;;
;;                                                                          ;;
;;              DECLE -- Task initial period * 2.                           ;;
;;                       If bit 0==1, this is one-shot task.                ;;
;;                                                                          ;;
;;              DECLE -- Task period re-init.  Useful if the task's first   ;;
;;                       delay != recurring delay, or for one-shots that    ;;
;;                       will be retriggered.                               ;;
;;                                                                          ;;
;;      R2 -- Task instance data (optional)                                 ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      Task record is set up.  (User still needs to update TSKACT to       ;;
;;      make the task records active, if necessary.)                        ;;
;;                                                                          ;;
;;  Registers R0, R4 are trashed.                                           ;;
;; ======================================================================== ;;
STARTTASK   PROC
            MVI@    R5,     R0
            SLL     R0,     2       ; R0 = R0 * 4 (four words/entry)
            MVII    #TSKTBL,R4      ; R4 = &TSKTBL[0]
            ADDR    R0,     R4      ; R4 = &TSKTBL[n]
            DIS                     ; Entering critical section.
            MVI@    R5,     R0      ; Get Function pointer
            MVO@    R0,     R4      ; ... and write it
            MVO@    R2,     R4      ; Write task instance data
            MVI@    R5,     R0      ; Get task period
            MVO@    R0,     R4      ; ... and write it
            MVI@    R5,     R0      ; Get task period reinit
            MVO@    R0,     R4      ; ... and write it
            EIS
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  STOPTASK                                                                ;;
;;      Stops a given task by setting its period to -1.  Task can be        ;;
;;      retriggered/restarted by calling RETRIGGERTASK.                     ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R3 -- Task number.                                                  ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      Task is disabled.                                                   ;;
;;      R0 == -1, R3 points to task delay count.                            ;;
;; ======================================================================== ;;
STOPTASK    PROC
            SLL     R3,     2       ; R3 = R3 * 4 (four words/entry)
            ADDI    #TSKTBL+2, R3   ; R3 = &TSKTBL[n].delay
            CLRR    R0
            DECR    R0              ; R0 == -1.
            MVO@    R0,     R3      ; TSKTBL[n].delay = -1
            JR      R5
            ENDP

;; ======================================================================== ;;
;;  RETRIGGERTASK                                                           ;;
;;      Restarts a task by copying its delay re-init field to its delay     ;;
;;      field.  Can be used on one-shot tasks or tasks which have been      ;;
;;      stopped with 'STOPTASK'.                                            ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R3 -- Task number.                                                  ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0 -- Task delay                                                    ;;
;;      R3 -- points to task delay count                                    ;;
;; ======================================================================== ;;

RETRIGGERTASK:  PROC
            SLL     R3,     2       ; R3 = R3 * 4 (four words/entry)
            ADDI    #TSKTBL+3, R3   ; R3 = &TSKTBL[n].reload
            DIS
            MVI@    R3,     R0      ; Get reload value
@@chain:    DECR    R3              ; Offset 2 is delay count
            MVO@    R0,     R3
            EIS
            JR      R5

            ENDP

;; ======================================================================== ;;
;;  WAIT                                                                    ;;
;;      Busy-waits for the number of ticks specified in R0.                 ;;
;;                                                                          ;;
;;      This function is build only if WTIMER is defined.                   ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Number of ticks to wait                                       ;;
;;      R5 -- Return address                                                ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0 -- cleared                                                       ;;
;; ======================================================================== ;;
        IF  DEFINED WTIMER
WAIT        PROC
            MVI@    R5,     R0
            MVO     R0,     WTIMER
            CLRR    R0
@@loop:     CMP     WTIMER, R0
            BNEQ    @@loop
            JR      R5
            ENDP
        ENDI
 
;;==========================================================================;;
;;  RATELIMIT                                                               ;;
;;      Like "WAIT", except it waits for the count from the previous call   ;;
;;      to expire.  The idea is that you set a timer now, and later wait    ;;
;;      for it to go off.                                                   ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R5 -- Number of ticks to wait next time, followed by return address ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0 -- Number of ticks to wait next time                             ;;
;;==========================================================================;;
        IF  DEFINED WTIMER
RATELIMIT   PROC
            CLRR    R0
@@loop:
            CMP     WTIMER, R0
            BNEQ    @@loop

            MVI@    R5,     R0
            MVO     R0,     WTIMER
            JR      R5
            ENDP
        ENDI

;; ======================================================================== ;;
;;  End of File:  timer.asm                                                 ;;
;; ======================================================================== ;;
