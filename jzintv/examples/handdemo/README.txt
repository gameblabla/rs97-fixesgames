
This demo illustrates how to use the hand-controller scanning routine
SCANHAND, along with the task-queue based event mechanism provided by
RUNQ.

The demo itself merely sets up a hand-controller dispatch table by
writing to SHDISP.  It then calls RUNQ, which receives and dispatches
to events and they come into the task queue.  See "task/README.txt"
for more information on the task routines.

The hand controllers are the only source of events in this demo.  The
events are produced by SCANHAND, which gets called from the idle loop.
Other event sources are possible.  See "timerdemo.asm" for an example.

The demo displays the following main screen:

                       L    R    L    R  
                     .123..123..123..123.
                      456  456  456  456 
                     .789..789..789..789.
                      C0E  C0E  C0E  C0E 
                      /#\  /#\  /#\  /#\ 
                      ###  ###  ###  ### 
                      \#/  \#/  \#/  \#/ 
                      ---  ---  ---  --- 
                      ---  ---  ---  --- 
                      ---  ---  ---  --- 
                       MASTER    E.C.S.  


Each of the four pictures represents a hand-controller.  The demo supports
up to four controllers -- two on the Master Component, and two on the ECS.
The controllers are labeled "L" and "R" across the top to indicate left 
and right.  They're labeled "MASTER" and "E.C.S." to indicate which unit
they're plugged into.  (Note:  If the ECS is not present, don't worry --
the scanning routines will just not show any inputs from the ECS.)

As you press various inputs on the controllers, you should see the 
corresponding portion of the hand-controller picture light up.  For the
DISC, an arrow should appear indicating the direction the disc was pushed.
In addition to the graphical depiction, you should see a number appear
below the controller. 

The displayed numbers are the numbers reported by SCANHAND.  The first
row shows numbers reported to the Keypad dispatch function.  The second
row shows numbers reported to the Action Key dispatch function.  The
third row shows numbers reported to the DISC dispatch function.  

The most-recently received event is displayed in yellow and is marked
with a '*' next to it.  The rest are displayed in green.  

To interpret the displayed numbers, see the documentation in 
"task/scanhand.asm".


