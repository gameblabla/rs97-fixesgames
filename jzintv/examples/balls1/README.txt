Ok, this demo is just a tad bizarre.  I wanted a short demo to illustrate
putting MOBs onscreen and moving them using fixed-point arithmetic.  So
here's the demo.

This demo keeps a 16-bit fixed point velocity around for each of the
8 MOBs.  The velocity is divided into 8-bit fraction, 8-bit integer.
It also keeps the X and Y coordinates in this same format.  (Note:
For most games, such a large format is overkill.)

The demo updates the position of all 8 MOBs.  It randomizes the velocity
of the MOBs whenever they interact with anything.  The result is Psycho 
Balls.

--Joe
