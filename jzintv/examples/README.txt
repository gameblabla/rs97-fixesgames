This directory tree contains a number of example programs and library
routines.  Many of the routines (especially those under "library") may
be useful in your own programs.

All of these programs and routines are being made available to you under
the GNU General Public License.  Please read the file "COPYING.txt" in
the top-level directory for details.

.
|-- README.txt           This file
|
|-- library              Library-related routines:
|   |-- dec16dec32.asm   DEC16 and DEC32 routines
|   |-- dec16only.asm    DEC16 (DEC32 support removed to save codesize)
|   |-- dividivu.asm     Signed and unsigned divide
|   |-- fastdivu.asm     Unsigned divide only (untested!)
|   |-- dist_fast.asm    Fast Euclidean distance estimation
|   |-- fillmem.asm      FILLMEM, FILLZERO and CLRSCR routines
|   |-- gimini.asm       Various constants and masks useful for Intellivision
|   |-- hex16.asm        HEX16 routine alone
|   |-- hexdisp.asm      HEX16 thru HEX4 and HEX12M thru HEX4M routines.
|   |-- ic_banksw.asm    Intellicart bank-switching support routines
|   |-- memcmp.asm       MEMCMP routine
|   |-- memcpy.asm       MEMCPY routine
|   |-- memset.asm       MEMSET routine (similar to FILLMEM)
|   |-- memunpk.asm      MEMUNPK routine
|   |-- print.asm        PRINT.xxx string display routines
|   |-- rand.asm         RAND random number generation routine
|   |-- sqrt.asm         SQRT square-root routine
|   |-- colorsq.asm      Baseline colored-squares routines
|   |-- colorsq_sv.asm   PUTPIXELS/GETPIXELS wrappers on PUTPIXEL/GETPIXEL
|   |-- colorsq_clip.asm Clipping versions of PUTPIXELS/GETPIXELS
|   |-- ivoice.asm       Intellivoice driver routines
|   |-- saynum16.asm     Speaks the value of a 16-bit number 
|   |-- resrom.asm       Defines symbols for the Intellivoice's RESROM
|   |-- al2.asm          SP0256 Allophone Library
|   `-- al2              \
|       |--               |__ Allophone library separated into individual
|       ...               |   assembly source files.
|       `--              /
|   
|-- macro                Macro sets intended to simplify programming
|   |-- util.mac         Loads of generic utility macros
|   |-- stic.mac         STIC-specific macros
|   |-- psg.mac          PSG-specific macros
|   `-- print.mac        PRINT.xxx specific macros
|    
|-- task                 Event-driven Multitasking routines
|   |-- README.txt       Describes the tasking model
|   |-- taskq.asm        Task-queue routines
|   |-- timer.asm        Timer-based task routines
|   |-- scanhand.asm     Hand-controller scanning / event generation
|   `-- sleep.asm        SLEEP/SPAWN advanced timer task functions
|
|-- hello                Simplest of simple:  Hello world!
|   |-- README.txt
|   `-- hello.asm 
|
|-- world                Spinning World demo.  This illustrates one way to 
|   |-- README.txt       deal with cartridges that are larger than 8K.
|   `-- world.asm 
|
|-- mazedemo             My original Maze-Demo, corrected to run on a real
|   |-- README.txt       Intellivision.
|   `-- mazedemo.asm
|
|-- bncpix               My original Bouncing Pixels demo, corrected to run
|   |-- README.txt       on a real Intellivision.
|   `-- bncpix.asm
|
|-- life                 A turbo-charged implementation of John Conway's
|   |-- README.txt       Life in colored-squares mode.
|   `-- life.asm     
|
|-- gram_scroll          Scrolls a grid w/8-pixel spacing by sequencing GRAM,
|   |-- README.txt       instead of using the horizontal/vertical delay 
|   `-- gram_scroll.asm  registers.
|
|-- gram_scroll2         Same as GRAM scroll #1, except the lines are 16
|   |-- README.txt       pixels apart.
|   `-- gram_scroll2.asm
|
|-- handdemo             Demonstrates how to use RUNQ/SCANHAND to read
|   |-- README.txt       the hand controllers.
|   `-- handdemo.asm     
|
|-- balls1               Psycho Balls:  Illustrates MOB movement and 
|   |-- README.txt       MOB interaction with an odd little demo.
|   `-- balls1.asm     
|
|-- balls2               Psycho Balls rewritten to take advantage of the
|   |-- README.txt       task-queue and timer task model.
|   `-- balls2.asm     
|
|-- sky                  Some SDK-1600 specific eye-candy
|   |-- README.txt       
|   `-- sky.asm    
|
|-- tagalong             Tag-Along Todd -- Illustrates hand controller
|   |-- README.txt       input and rudimetary AI on a timer-task.
|   `-- tagalong.asm     
|
|-- tagalong2            Tag-Along Todd #2 -- Extends Tag-Along Todd into
|   |-- README.txt       a full-blown game
|   `-- tagalong.asm     
|
|-- tagalong2v           Tag-Along Todd #2 Voice.  Adds Voice to 
|   |-- README.txt       Tag-Along Todd #2.
|   `-- tagalongv.asm    
|
|-- bankdemo             Simple Intellicart bank-switching demo
|   |-- README.txt       
|   `-- bankdemo.asm     
|
|-- bankworld            Adaptation of the Spinning World example to use
|   |-- README.txt       Intellicart bank-switching.
|   `-- bankworld.asm     
|
|-- csumexec             Simple demo that identifies which variant of 
|   |-- README.txt       the Intellivision it's running on by checksumming
|   `-- csumexec.asm     the EXEC and other ROMs in the system.
|
|-- mob_test             Moving OBject tester.  This demo allows you to
|   |-- README.txt       move the various MOBs around and play with their
|   `-- mob_test.asm     attributes.
|
|-- mem_test             Intellicart-specific Memory Tester.  
|   |-- README.txt
|   `-- mem_test.asm
|
|-- banktest             Intellicart-specific Memory Tester with
|   |-- README.txt       bankswitch support.
|   `-- banktest.asm
|
`-- ecscable             The ECScable Monitor and construction information
    |-- README.txt
    `-- ec_mon2.asm 

Each of the demos can be build with a single 'as1600' commandline.
The exact command used depends on your desired output format.  The
assembler selects the output format based on the file extension.

ROM FILE OUTPUT:
    as1600 -o program.rom -l program.lst program.asm

BIN+CFG FILE OUTPUT:
    as1600 -o program.bin -l program.lst program.asm


All of the programs in this tree are licensed under the GNU General
Public License, unless the source code for the program itself indicates
a different license.  The short description of the license is that
you're allowed to incorporate pieces of this code in your own programs.
The resulting program, though, can only be legally distributed under
the GNU GPL.  Please see the file "COPYING" in the top-level directory
for the legally binding details.

If you do not wish to be bound by the GPL, but would like to use some
of this code in your game software, please contact me.  We may be
able to work something out.

--Joseph Zbiciak <im14u2c@globalcrossing.net>

