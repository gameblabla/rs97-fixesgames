;;==========================================================================;;
;; Joe Zbiciak's LIFE DEMO                                                  ;;
;; Copyright 2002, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

This demo implements John Conway's "Life."  In case you're not familiar with
Life, do a quick Google search.  There are about 1000 pages which will
describe Life better than I can.  

Here's some summary infomation though:

LIFE RULES:                                                            
                                                                       
 -- A dead cell with exactly three live neighbors becomes a live       
    cell (birth).                                                      
                                                                       
 -- A live cell with two or three live neighbors stays alive           
    (survival).                                                        
                                                                       
 -- In all other cases, a cell dies or remains dead (overcrowding      
    or loneliness).                                                    
                                                                       
OUR WORLD                                                              
    We display a 32 x 24 "world" for the cells to live in.  The world  
    is "toroidal", in that the top connects to the bottom, and the     
    left connects to the right.  This world holds 768 cells.           
                                                                       
    The cell information for the current and next generations are      
    held in two bitmaps in 8-bit scratch RAM.  We store 1 bit per      
    cell, so each bitmap occupies 96 bytes.  The bitmap is stored in   
    "little endian" order -- the leftmost cell onscreen is in bit 0    
    of the first byte, not bit 7 as you may expect.                    
                                                                       
    The engine works by calculating the new generation from the last
    generation's state according to the rules above.  It then swaps    
    the two generations.  We don't actually move anything in memory    
    when we swap -- we keep track of which world is which with         
    pointers.                                                          
                                                                       
    Once a new generation is calculated, we display it.  That's it.    

BUILD OPTIONS
    You can rebuild Life to start with a horizontal or vertical
    glider instead of a random world.  See the source for details.

BENCHMARKING
    I decided to code this demo for speed -- I wanted to see how
    fast the Inty could calculate the Life world.  Therefore, I display
    performance information onscreen.  In the lower right, you'll see
    2-digit hex number.  This number is the number of VBLANK interrupts
    (ticks) that occur during calculation and display of the screen.

ECSCABLE
    This demo supports the ECScable, by calling EC_POLL if the ECScable
    Monitor is detected.

IDEAS FOR TWEAKS -- THINGS TO TRY
    You might consider modifying the demo to allow the user to 
    insert one of the classic Life shapes into the life world,
    such as the gliders that are already in there, or one of 
    several other interesting patterns.

    Also, you might add support for inserting, say, a glider at
    random after the world's been going for awhile.

    And, do you think you could make it go any faster?  <:-)

