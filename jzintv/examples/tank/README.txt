Simple tank demo.  Meant to demonstrate simple animation, collision
detection, and simple ballistics computation.

This game could be turned into a Scorched Earth type of game with some
work.  I haven't taken it nearly that far.  It keeps a simple form of 
score -- namely, the number of display frames each tank is in contact 
with an explosion.  There's only one 'warhead' type:  a simple bullet.

Game controls:

    Each controller controls one tank.  Left controller controls the
    left tank, right controller controls the right tank.

    The disc moves the tank left and right.  The tanks can push each
    other but cannot travel offscreen.

    The number pad selects the firing strength.  [1] thru [9] select
    firing strengths 1 through 9.  [C], [0] and [E] all select a firing
    strength of 10.

    The lower action buttons rotate the gun turret.  Lower right rotates
    clockwise, lower left rotates counter-clockwise.  The upper action
    button fires.


Source files:

    tank.asm        Top level game file.  Contains all vars and the ROM hdr.
    mob_ll.asm      Low level MOB manipulation routines
    objects.asm     Object management (tanks and bullets)
    status.asm      Status bar display
    gfx_data.asm    GRAM pictures for MOBs and otherwise
    atr_data.asm    Attribute data tables for MOBs
    mob_data.asm    Other miscellaneous MOB information
    util.asm        Utility functions and macros.
