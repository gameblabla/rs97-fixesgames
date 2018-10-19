
This is a scrolling example which achieves its scrolling illusion by
sequencing the GRAM, rather than using the horizontal and vertical 
scroll registers.  The result is that text stays in a fixed location,
but the background moves.

In this version, the scrolling background is a simple grid with lines
spaced 16 pixels apart.  It is otherwise identical to GRAM scroll #1.
The grid moves around randomly under the displayed text.

The demo displays the current velocity and position of the grid in
hex across the top of the screen.  It also displays some fixed text
in the middle of the screen.

