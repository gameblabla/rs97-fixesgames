
This demo is very similar to "balls1", except that it has been modified
to use the task-queue routines to schedule everything.  

Note that the processing for the MOBs wants to run every 'tick', but
it's sometimes too slow.  This code solves that problem by keeping a 
"busy" flag for the MOB updates.  When the ISR runs, it will only queue 
MOB updates when it knows the previous MOB update has completed.

The sweeper is handled by a pair of timer-based tasks.  A long-period
task restarts the sweeper every few seconds.  A short-period task
updates the sweeper when its active.

You'll notice that the sweeper's updates are a little smoother in this
version.  This is due to the effects of rate-limiting the MOB updates
and queueing the sweeper updates.

To change the speed of the sweeper in this version, you need only to
change its task period in "SW_START".
