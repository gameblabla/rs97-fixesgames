==============================================================================
  THE TASK QUEUE EVENT-DRIVEN PROGRAMMING MODEL:  taskq.asm
==============================================================================

RUNQ provides an event-driven programming model.  The model is built
around a task queue, and a main event-handling loop.  The purpose
of the event-handling loop is to look for events to get queued, and
to process those events as they come in.  When there's nothing else
to do, the event loop executes an "idle task."  The "idle task" may
subsequently generate new events.

The "idle task" is embedded in RUNQ.  It's exact contents depend on
build flags.  Look at "taskq.asm" for details.  By default, the idle
task will do the following:

 -- Scan the hand controllers looking for new inputs.  This can be
    a source of new events to process.

 -- Update the random number generator.  The goal here is to let
    system loading improve the randomness of the random number
    generator.  (We hope.)

 -- Poll the ECScable (if we're allowed to).


The task queue is fairly simple:  It contains a queue of function
pointers, and a parallel queue of arguments to pass to those functions.
A single 16-bit argument is provided.

This simple queue structure is sufficient for reporting input events,
collision events, timed events and so on.  Events can come from anywhere.
Any routine can call QTASK to queue a new event to be processed.  This
provides a fairly flexible method for queuing up things to do.

Another reason for queuing tasks is to break up large operations into
small chunks, so that many parallel operations can interleave.  The
game 4-Tris does this, for instance.  

In 4-Tris, the score is updated by "rolling" the displayed value towards
the actual score.  Likewise, the well is cleared with an animated
sequence.  We would like each of these actions to occur in parallel.
4-Tris handles this by breaking up both operations into tiny events.
For the score, each event call causes the score to do one update as part
of its "rolling" process.  For the well-clearing, one event call clears
one row of the area being cleared.  Because these streams of events
are being scheduled simultaneously, the two streams can interleave,
giving the illusion of simultaneity.

In general, I'd recommend that you keep event handlers short.  Anything
that takes a "long time" to process should be broken up into small
pieces.  This allows the process to "multitask" with the rest of the
system.

API Reference:

    RUNQ         -- Call to turn on the event handling loop.  Returns only
                    when SCHEDEXIT is called.
                
    QTASK        -- Queues a task to be handled by the event handling loop.
               
    SCHEDEXIT    -- Causes RUNQ to exit.  This is useful when you have a
                    top-level function which transitions game phases between
                    calls to RUNQ.
   
    STOPALLTASKS -- Flushes the tasks queue, and if there are any timer-based
                    tasks, disabled them as well.

==============================================================================
  TIMER-BASED TASKS:  timer.asm
==============================================================================

The routines in timer.asm are intended to allow programs to generate
periodic tasks -- tasks with fixed or nearly-fixed periods. 

The timer routines maintain a set of task structures in memory.  These
structures contain pointers to each of the timer-triggered event
handlers, the period of each task, and the argument to pass to the
task when the task is triggered.

Timer tasks can be either one-shot or repeating tasks.  A one-shot
timer task gets queued once when its timer expires.  That's it.
A repeating task will reinitialize its timer each time its triggered.

The timers are updated by a single function called DOTIMER.  DOTIMER
should be called from an ISR, although it can also be queued as
a task.   On each invocation of DOTIMER, all active task timers are
updated, and events are queued for timers that expire.  

Additional functions are provided for starting and stopping timed tasks.
The variable "TSKACT" gives the total number of active tasks.  DOTIMER
only updates tasks 0 .. TSKACT-1.

API Reference:

    DOTIMER          -- Call periodically to update the task timers

    STARTTASK        -- Initializes a task timer record.  User must still
                        set TSKACT if needed.

    STOPTASK         -- Disabled a task timer.

    RETRIGGERTASK    -- Copies a tasks timer reinitialization field to
                        its period.  This will retrigger a one-shot
                        or restart a period task that had been stopped.

    STOPALLTASKS     -- Flushes the tasks queue and disables all timer-based
                        tasks.

==============================================================================
  SLEEP and SPAWN:  sleep.asm
==============================================================================

SLEEP and SPAWN make use a timer slot to allow a function to "sleep"
for a specified period while other events are handled.  This function is
a somewhat tricky function to use correctly.  Most programs can probably
live without as well, which is why it's broken out to a separate file.

SLEEP and SPAWN are actually the same function.  So from here, when
I refer to SLEEP, I'm referring to both unless noted otherwise.

SLEEP works by saving the caller's registers to a special register-save
area.  It then sets up a task timer record, and returns to the caller's
caller.  That's right, rather than return to its caller, it looks for
a return address on the top of stack, and returns to that.  This is
important:  You *must* have your return address on the top of stack
when you call SLEEP.  You *cannot* have other information stored on
the stack when you call SLEEP.

When the task timer expires, it will kick off a special "resume" task.
This task will restore the registers from the register-save area and
jump to the point just after SLEEP was called.  

SLEEP and SPAWN differ only in the period argument passed to either.
One can specify a one-shot period which results in a SLEEP.  Alternately,
one can specify a repeating period, which results in a SPAWN.  SPAWN
effectively "spawns" a new repeating task a the point where the call to
SPAWN is made.  This can be useful, but tricky to get right.  Look
at 4-Tris for an example of how SPAWN is used.

==============================================================================
  SCANHAND -- Reading the hand controllers
==============================================================================

The RUNQ routine will scan the hand controller for you, unless either
SCANHAND is missing, or you build it with the symbol "RUNQ_NO_SCANHAND" 
set.

The file "scanhand.asm" provides a fairly complete and comprehensive
hand-controller scanning routine.  It may be overkill for your application.
Please read the docs in "scanhand.asm" for details.

You're welcome to provide your own hand-controller scanning routine.
If you want RUNQ to call it from the background task, then name it
SCANHAND and define it before you INCLUDE "taskq.asm".


==============================================================================
  HOW DOES THIS RELATE TO 4-TRIS?
==============================================================================

These routines were developed as part of 4-Tris.  The routines
"MAINISR" and "MAINLOOP have been renamed "DOTIMER" and "RUNQ", 
respectively.

The hand-controller scanning routine in "scanhand.asm" is very different
from the SCANHAND that appeared in 4-Tris.  It uses a different dispatch
table format, requires slightly more RAM, and provides significantly 
different functionality.

