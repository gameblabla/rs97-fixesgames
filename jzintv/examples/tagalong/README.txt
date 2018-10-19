Tag-along Todd

This is a simple game-like demo.  It'd be a game, except there's no
scoring, and no point.


Tag-along Todd is the annoying kid next door that won't go away.  No
matter where you go, Todd comes lumbering after you.  Todd's not
too swift, so he kinda lumbers around.


This program uses a mix of timer-based tasks and hand-controller
input.  In this case, one timer task ticks twice per second to
update Todd's velocity.  The player's velocity is set by hand
controller input.  A third task is scheduled every frame to update
the MOBs.

To smooth out the player's motion, I separate out target velocity
from actual velocity, and I smoothly update actual velocity towards
target velocity.
