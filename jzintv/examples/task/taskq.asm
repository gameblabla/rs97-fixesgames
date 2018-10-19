;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;;==========================================================================;;
;; Joe Zbiciak's Task Routines:  The Task Queue.                            ;;
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
;;   -- The TSKQ must have room for TSKQM + 1 words.  That is, if TSKQM     ;;
;;      is 15, then TSKQ must have room for 16 words.                       ;;
;;                                                                          ;;
;;   -- The TSKDQ must have room for 2*(TSKQM + 1) bytes.  That is, if      ;;
;;      is 15, then TSKDQ must have room for 32 bytes.                      ;;
;;                                                                          ;;
;;  MAGIC BUILD FLAGS                                                       ;;
;;   -- RUNQ_NO_ECSCABLE should be set if you need to disable ECScable      ;;
;;      support.  This is necessary, for instance, if you are scanning      ;;
;;      for hand controllers on the ECS.                                    ;;
;; ======================================================================== ;;

                        ; Used by     Req'd Width   Description
                        ;-----------------------------------------------------
;OVRFLO EQU     $130    ; RUNQ/etc.   8-bit         Number of queue overflows
;TSKQHD EQU     $132    ; RUNQ/etc.   8-bit         Task queue head           
;TSKQTL EQU     $133    ; RUNQ/etc.   8-bit         Task queue tail           
;TSKQ   EQU     $300    ; RUNQ/etc.   16-bit        Task queue                
;TSKDQ  EQU     $140    ; RUNQ/etc.   8-bit         Task data queue           
;TSKQM  EQU     $F      ; RUNQ/etc    EQUATE        Task queue length mask    
;; The following are needed if supporting timer-based tasks
;MAXTSK EQU     4       ; RUNQ/etc.   EQUATE        Maximum # of timer tasks
;TSKACT EQU     $134    ; STOPALL     8-bit         # of active timer tasks
;TSKTBL EQU     $310    ; STOPALL     16-bit        Timer-task table


;; ======================================================================== ;;
;;  RUNQ -- Main loop for event-driven programs.                            ;;
;;                                                                          ;;
;;  The main loop runs asynchronously from the MAINISR, running tasks as    ;;
;;  they get scheduled.  There is also a background task that runs when     ;;
;;  nothing else is in the queue.                                           ;;
;;                                                                          ;;
;;   -- Grab task from task queue, if any                                   ;;
;;       -- run that task                                                   ;;
;;       -- look for next task                                              ;;
;;   -- run background task once if no tasks available                      ;;
;;                                                                          ;;
;;  CONTROLLER SCANNING SUPPORT                                             ;;
;;                                                                          ;;
;;  If the symbol SCANHAND is defined *before* INCLUDEing this file, the    ;;
;;  background task will scan hand controllers.  The background task is a   ;;
;;  good choice for scanning the hand controllers as it is reasonably       ;;
;;  periodic, and hand-controller scanning can be fairly expensive.         ;;
;;                                                                          ;;
;;  If your system is fairly busy, you may wish to scan hand controllers    ;;
;;  in a different manner, say by calling SCANHAND from an ISR.  To         ;;
;;  prevent RUNQ from calling SCANHAND, set the symbol RUNQ_NO_SCANHAND.    ;;
;;  This may be desirable if you are going scan the hand controllers        ;;
;;  elsewhere.                                                              ;;
;;                                                                          ;;
;;  RANDOM NUMBER CYCLING SUPPORT                                           ;;
;;                                                                          ;;
;;  If the symbol RAND is defined *before* INCLUDEing this file, the        ;;
;;  background task will also update the random number generator state.     ;;
;;  This helps ensure the random numbers are bit more random since the      ;;
;;  game loading varies according to what's going on.                       ;;
;;                                                                          ;;
;;  ECSCABLE POLLING SUPPORT                                                ;;
;;                                                                          ;;
;;  By default, the background task will look for the ECScable Monitor      ;;
;;  and poll for ECScable activity.  You can disable building in this       ;;
;;  support by defining the symbol RUNQ_NO_ECSCABLE.                        ;;
;; ======================================================================== ;;
RUNQ    PROC

        PSHR    R5              ; Save return address, since any
                                ; task is allowed to exit the main
                                ; game loop, say, when switching
                                ; between game phases.

@@loop:
        MOVR    PC,     R5      ; Set our return address to @@loop
        DECR    R5              ; This is 2 decles vs. 4 for SDBD/MVII

        ; Run next scheduled task from queue
        DIS                     ; Shut off interrupts
        MVI     TSKQTL, R1      ; Get the tail of the queue
        CMP     TSKQHD, R1      ; Are there tasks in the queue?
        BEQ     @@bktsk         ; No:  Do background task

        INCR    R1              ; Pop task from queue by
        ANDI    #TSKQM, R1      ; ... moving the queue tail
        MVO     R1,     TSKQTL  ; Store new task-queue tail
        MOVR    R1,     R4
        ADDR    R4,     R4
        ADDI    #TSKDQ, R4
        SDBD
        MVI@    R4,     R2      ; Load instance data for task
        ADDI    #TSKQ,  R1      ; Point to task queue
        MVI@    R1,     R0      ; Get function pointer from queue

        ; Dispatch to user's task.  Make sure interrupts are enabled again
        EIS                     ; Done with critical section
        JR      R0              ; Jump to subroutine
        ; Note that this implicitly loops back to @@oloop here

@@bktsk:
        EIS                     ; Done with critical section

        ;; ---------------------------------------------------------------- ;;
        ;;  If the random number generator exists, call it to advance the   ;;
        ;;  random state.                                                   ;;
        ;; ---------------------------------------------------------------- ;;
    IF  DEFINED RAND
        MVII    #$F,    R0
        CALL    RAND            ; Update random number state
    ENDI    

        ;; ---------------------------------------------------------------- ;;
        ;;  If the hand-controller scanner exists, call it to scan the      ;;
        ;;  controllers.  That is, unless we've been told not to.           ;;
        ;; ---------------------------------------------------------------- ;;
    IF  (DEFINED SCANHAND) = 1 AND (DEFINED RUNQ_NO_SCANHAND) = 0
        CALL    SCANHAND        ; Scan the hand controllers.
    ENDI

        ;; ---------------------------------------------------------------- ;;
        ;;  If we're allowed to, look for the ECScable Monitor and poll it. ;;
        ;; ---------------------------------------------------------------- ;;
    IF  (DEFINED RUNQ_DO_ECSCABLE) = 1

    IF  (DEFINED EC_LOC) = 0
EC_LOC  EQU     $CF00
    ENDI
    IF  (DEFINED EC_MAG) = 0
EC_MAG  EQU     $69
    ENDI
    IF  (DEFINED EC_POLL) = 0
EC_POLL EQU     $CF01
    ENDI

        MVI     EC_LOC, R0
        CMPI    #EC_MAG,R0
        BNEQ    @@loop
        CALL    EC_POLL

    ENDI

        B       @@loop          ; Run around the inner loop
        ENDP


;; ======================================================================== ;;
;;  QTASK                                                                   ;;
;;      Add a task to the task queue.                                       ;;
;;      NOTE: CALL THIS WITH INTERRUPTS OFF!!!!  (eg. use JSRD).            ;;
;;      Interrupts will be enabled before return.                           ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R0 -- Task function pointer                                         ;;
;;      R1 -- Task instance data                                            ;;
;;      R5 -- Return address.                                               ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0 -- intact.                                                       ;;
;;      R1 -- swapped                                                       ;;
;;      R2, R4 -- trashed.                                                  ;;
;; ======================================================================== ;;
QTASK   PROC
        MVI     TSKQHD, R2      ; Get head of task queue
        INCR    R2              ; Move to next queue slot
        ANDI    #TSKQM, R2      ; Stay in circular buffer
        CMP     TSKQTL, R2      ; Are we overflowing the queue?
        BEQ     @@overflow      ; Yes ... don't overflow it!
        MVO     R2,     TSKQHD  ; Store new task queue head

        MOVR    R2,     R4      ; Generate instance data pointer
        ADDR    R2,     R4      ; (remember mult by two for data ptr)
        ADDI    #TSKQ,  R2      ; Point into task queue
        MVO@    R0,     R2      ; Store function pointer to task q
; moved addi for STIC interruptibility fix
        ADDI    #TSKDQ, R4      ; Point into task data queue
        MVO@    R1,     R4      ; Store instance data to data q
        SWAP    R1              ; ....  cont'd
        MVO@    R1,     R4      ; ....  cont'd

    IF      (DEFINED OVRFLO) = 0  ; No support for overflow tracking?
@@overflow:
        EIS                     ; Re-enable ints after crit section
        JR      R5              ; Return.

    ELSE    ; separate exit path for overflow case.

        EIS                     ; Re-enable ints after crit section
        JR      R5              ; Return.

@@overflow:
        MVI     OVRFLO, R2
        INCR    R2
        MVO     R2,     OVRFLO

        EIS                     ; Re-enable ints after crit section
        JR      R5              ; Return.

    ENDI   

        ENDP

;; ======================================================================== ;;
;;  SCHEDEXIT                                                               ;;
;;      Flushes the task queue and schedules the exit task.                 ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R5 -- Return address.                                               ;;
;;                                                                          ;;
;;  OUTPUTS                                                                 ;;
;;      Does not return if scheduled as a task.  Causes RUNQ to return      ;;
;;      to its caller.                                                      ;;
;; ======================================================================== ;;
SCHEDEXIT:      PROC
        PSHR    R5              ; Push return address.
        JSRD    R5,STOPALLTASKS ; Flush the pending task queue
        MVII    #@@exit, R0     ; Schedule the exit task ...
        PULR    R5
        B       QTASK           ; ... and chain the return. (ints still DIS)
@@exit: PULR    PC              ;
        ENDP

;; ======================================================================== ;;
;;  STOPALLTASKS                                                            ;;
;;                                                                          ;;
;;  Stops all tasks by flushing the task queue (sets head/tail to 0).       ;;
;;                                                                          ;;
;;  If MAXTSK is defined (eg. timer-based tasks are being used), this       ;;
;;  code will also set all the timer-based task periods to -1, and will     ;;
;;  set number of active timer tasks (TSKACT) to 0.  It returns the         ;;
;;  previous TSKACT in R1.                                                  ;;
;;                                                                          ;;
;;  NOTE: CALL THIS WITH INTERRUPTS OFF!!!!  (eg. use JSRD)  Interrupts     ;;
;;  are NOT re-enabled by this function.  This function is usually called   ;;
;;  as part of an initialization sequence, or by SCHEDEXIT.                 ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R5 -- Return address                                                ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0 -- Previous TSKACT (if MAXTSK is defined.)                       ;;
;;      R1 -- Cleared                                                       ;;
;;      All tasks stopped.                                                  ;;
;; ======================================================================== ;;
STOPALLTASKS    PROC

        CLRR    R1
        MVO     R1,     TSKQHD          ; \__ Clear out the task queue
        MVO     R1,     TSKQTL          ; /

    IF DEFINED MAXTSK

        MVI     TSKACT, R0              ; \__ Get the previous # of active
        MVO     R1,     TSKACT          ; /   tasks and set current # to 0.

        DECR    R1

@@ofs   QSET    TSKTBL + 2
@@cnt   QSET    0
        REPEAT  MAXTSK
            IF      @@cnt = 3
                NOP                     ; Interruptible every 4th for STIC
@@cnt           QSET    0
            ELSE
@@cnt           QSET    @@cnt + 1
            ENDI
           
            MVO     R1,     @@ofs       ; Set period for task N to -1
@@ofs       QSET    @@ofs + 4
        ENDR

    ENDI
        JR      R5

        ENDP
;; ======================================================================== ;;
;;  End of File:  taskq.asm                                                 ;;
;; ======================================================================== ;;
