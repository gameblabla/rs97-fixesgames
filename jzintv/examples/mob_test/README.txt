;;==========================================================================;;
;; Joe Zbiciak's Moving Object Test.   Real simple test case.               ;;
;; Copyright 2001, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

This is my "MOB Test".  The MOB Test allows you to select each of the 
eight MOBs and move them around the screen.  As you move the MOB, status
information for the MOB is updated onscreen in real time.

The main screen looks like so at startup:


                       [ Joe's 'MOB' Test ]
                                _     
                        34C 014 3800 3C00
                        34D 014 3801 3C00
                        34E 014 3802 3C00
                        34F 014 3803 3C00
                        350 014 3804 3C00
                        351 014 3805 3C00
                        352 014 3806 3C00
                        353 014 3807 3C00
                              ######
                            Clear: Off     


The four colums show the X register, Y register, A register and C register
for each of the 8 MOBs.  The X register contains the MOB X coordinate
and some flags.  The Y register contains the MOB Y coordinate and some
more flags.  The A register contains the color and card # for the MOB,
and still more flags.  And finally, the C register contains the collision
status for the MOBs.

Below the status display is a small test strip containing colored-square
pixels.  This allows for testing MOB interaction with colored-squares.  The
layout is as follows:

                           xx00224466xx
                           xx11335577xx

The locations marked 'xx' are actually a blank background card whose
background color should be cycling.  The numbered areas are colored
square pixels with the indicated color number.  The pixels with color
7 should be color-cycling alongside the two end characters.

The MOB test allows you to move the MOBs around and test some of their
behavior.  The following hand-controller inputs are defined for the
MOB Test.  Note that "Top Action" refers to both of the upper action
buttons.  Left and Right Action refer to the lower left and lower right
buttons.

       [1]     Select MOB #0
       [2]     Select MOB #1
       [3]     Select MOB #2
       [4]     Select MOB #3
       [5]     Select MOB #4
       [6]     Select MOB #5
       [7]     Select MOB #6
       [8]     Select MOB #7
       [9]     Increment the horizontal delay register for the display
       [C]     Increment the vertical delay register for the display
       [0]     Cycle through various bitmaps for the MOB
       [E]     Toggle clearing of the collision registers every frame.
   Top Action  Toggle Visibility of MOB
  Left Action  Toggle Priority of MOB
 Right Action  Toggle Interaction bit of MOB
       DISC    Move MOB around display

The row for currently selected MOB is highlighted with a blue highlight
bar.  This makes it easy to watch for changes as they happen, say as you're
maneuvering MOBs around the screen.

The following bitmap patterns are available for the cards:

    ........    ########    ####....    ....####
    ........    ########    ####....    ....####
    ........    ########    ####....    ....####
    ........    ########    ####....    ....####
    ........    ########    ........    ........
    ........    ########    ........    ........
    ........    ########    ........    ........
    .......#    ########    ........    ........

    ........    ........    #.#.#.#.    .#.#.#.#
    ........    ........    .#.#.#.#    #.#.#.#.
    ........    ........    #.#.#.#.    .#.#.#.#
    ........    ........    .#.#.#.#    #.#.#.#.
    ....####    ####....    #.#.#.#.    .#.#.#.#
    ....####    ####....    .#.#.#.#    #.#.#.#.
    ....####    ####....    #.#.#.#.    .#.#.#.#
    ....####    ####....    .#.#.#.#    #.#.#.#.


The following illustrates the format of the MOB registers, and defines
the purpose of each of the flag bits:

------------------------------------------------------------------------------
   X Register layout:                                                       
                                                                            
      13   12   11   10    9    8    7    6    5    4    3    2    1    0   
    +----+----+----+----+----+----+----+----+----+----+----+----+----+----+ 
    | ?? | ?? | ?? | X  |VISB|INTR|            X Coordinate               | 
    |    |    |    |SIZE|    |    |             (0 to 255)                | 
    +----+----+----+----+----+----+----+----+----+----+----+----+----+----+ 
                                                                            
   Y Register layout:                                                       
                                                                            
      13   12   11   10    9    8    7    6    5    4    3    2    1    0   
    +----+----+----+----+----+----+----+----+----+----+----+----+----+----+ 
    | ?? | ?? | Y  | X  | Y  | Y  |YRES|          Y Coordinate            | 
    |    |    |FLIP|FLIP|SIZ4|SIZ2|    |           (0 to 127)             | 
    +----+----+----+----+----+----+----+----+----+----+----+----+----+----+ 
                                                                            
   A Register layout:                                                       
                                                                            
      13   12   11   10    9    8    7    6    5    4    3    2    1    0   
    +----+----+----+----+----+----+----+----+----+----+----+----+----+----+ 
    |PRIO| FG |GRAM|      GRAM/GROM Card # (0 to 255)      |   FG Color   | 
    |    |bit3|GROM|     (bits 9, 10 ignored for GRAM)     |   Bits 0-2   | 
    +----+----+----+----+----+----+----+----+----+----+----+----+----+----+ 
                                                                            
   C Register layout:                                                       
                                                                            
      13   12   11   10    9    8    7    6    5    4    3    2    1    0   
    +----+----+----+----+----+----+----+----+----+----+----+----+----+----+ 
    | ?? | ?? | ?? | ?? |COLL|COLL|COLL|COLL|COLL|COLL|COLL|COLL|COLL|COLL| 
    |    |    |    |    |BORD| BG |MOB7|MOB6|MOB5|MOB4|MOB3|MOB2|MOB1|MOB0| 
    +----+----+----+----+----+----+----+----+----+----+----+----+----+----+ 


The various flag bits modify the MOBs behavior as follows:

    INTR    If set, this MOB will interact with other MOBs, background
            cards and the screen borders.  If clear, it will not interact.  
            For two MOBs to interact, both of their INTR bits must be set.

    VISB    If set, the MOB is visible onscreen.  If clear, it is not
            visible.  The MOB does not need to be visible in order to
            interact with other MOBs, background cards or the screen border.

    PRIO    If set, the MOB appears behind background cards.  If clear,
            the MOB appears in front of background cards.  This ordering
            applies to the visible pixels of the MOB only.  MOBs are first
            ordered according to their number, with MOB #0 being visible
            over MOB #1 and so on.  The PRIO bit does not affect MOB-to-MOB
            priority ordering.

    XSIZE   Controls the magnification of the MOB along the X axis. 
            If clear, the MOB is displayed at normal width (8 pixels).
            If set, the MOB is displayed at double width (16 pixels).

    YRES    These three bits controls the size and magnification of the 
    YSIZ2   MOB along the Y axis.  To explain their interaction, consult
    YSIZ4   the following table.  Note that MOBs display with half-pixels
            vertically, so the displayed height is given in terms of card
            pixels.

                YRES  YSIZ4  YSIZ2    MOB Resolution   Displayed height
                  0     0      0          8 x 8            4 pixels
                  0     0      1          8 x 8            8 pixels
                  0     1      0          8 x 8           16 pixels
                  0     1      1          8 x 8           32 pixels
                  1     0      0          8 x 16           8 pixels
                  1     0      1          8 x 16          16 pixels
                  1     1      0          8 x 16          32 pixels
                  1     1      1          8 x 16          64 pixels

            When YRES is set, the MOB takes its picture information
            from two consecutive card bitmaps.  The upper half always
            comes from an even numbered card, and the lower half always
            comes from an odd numbered card.  Bit #3 in the A registers
            (the least-significant bit of the card number) is ignored
            when YRES is set.

            When both YSIZ4 and YSIZ2 are clear, the MOB is displayed
            using half-pixels vertically.  That is, they have double
            the vertical resolution of background cards.  The displayed
            heights above are in terms of full pixels.
 
    XFLIP   Display the horizontal mirror image of the MOB.

    YFLIP   Display the vertical mirror image of the MOB.

    GRAM/   If set, the bitmap image comes from GRAM.  Otherwise, it
    GROM    comes from GROM.

    CARD #  Determines the card # displayed.  This field combines with
            the GRAM/GROM bit to create a 9-bit card # field.  This allows
            addressing all 320 pictures contained in GROM and GRAM.  Some 
            of these bits are ignored in certain circumstances, though:

            Bits 9 and 10 of the attribute register (bits 7 and 6 of
            the card #) are ignored when the bitmap comes from GRAM,
            or when the display is in Foreground/Background mode.
            (This means that only 128 of the 320 pictures are available
            when the display is in FGBG mode.  GROM cards #64 - #255
            are unavailable in FGBG mode.)

            Bit 3 of the attribute register (bit 0 of the card number)
            is ignored if the YRES bit is set.

            Bits that are ignored by the hardware may be used by
            programs to store flags or other information.

    COLL    Set when the MOB interacts with the display borders.
    BORD

    COLL    Set when the MOB interacts with a set pixel in a background 
     BG     card.  Note that colored-squares colors 0 through 6 are 
            treated as "set", and color 7 is treated as "clear".

    COLL    Set when the MOB interacts with one of the other MOBs 0 through
    MOBn    7.  Note that a MOB *never* interacts with itself, so its
            self-collision bit will *never* be set under any circumstance.

            All 10 collision bits are "sticky", meaning that once set by
            an interaction, they remain set until the CPU clears them.
            They will remain set even if the INTR bit is subsequently
            cleared.  The CPU may also artifically set collision bits by
            writing to the collision registers, although bit #N for MOB
            #N will never read as 1 even if the CPU tries to set it.
            Also, writing a new value in bit #X for MOB #Y will not
            automatically change the value for bit #Y for MOB #X.
------------------------------------------------------------------------------
