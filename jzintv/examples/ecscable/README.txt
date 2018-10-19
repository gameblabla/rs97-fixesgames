==============================================================================
>>   WARNING:  I PROVIDE THE INSTRUCTIONS BELOW FOR EDUCATIONAL PURPOSES    <<
>>   ONLY.  IF YOU ATTEMPT TO BUILD AND OPERATE AN ECScable, YOU DO SO AT   <<
>>   YOUR OWN RISK.  I CANNOT BE RESPONSIBLE FOR ANY DAMAGE TO YOUR         <<
>>   INTELLIVISION, YOUR ECS, YOUR INTELLICART, YOUR COMPUTER OR OTHER      <<
>>   EQUIPMENT.  YOU HAVE BEEN WARNED.                                      <<
==============================================================================



This directory includes the construction information for the ECScable, as
well as the source code for the ECScable Monitor.  The PC-side utilities
are contained in the src/ tree.

The ECScable is a simple data-cable that connects an Intellivision
ECS to a PC via a parallel-port interface.  The monitor allows a
game to interact with the PC via a simple protocol.  It also allows
for reloading the game easily on-the-fly.  The current ECScable monitor
and software requires an Intellivision, an ECS, and an Intellicart.


To use the ECScable, attach the cable between a printer port on your PC
and the left/right controller connectors on an ECS.  Load the ECScable
Monitor into the Intellicart, and use the "ec_load", "ec_dump" and
"ec_watch" utilities to communicate with the Intellivision via the
ECScable.

See ECScable Monitor Features below for details on using the ECScable
Monitor itself, as well as on how to make your programs ECScable Aware.



==============================================================================
    RELIABILITY ADDENDUM:  I have not been able to get my ECScable
    to work on a machine other than my primary Linux box.  Upon reading
    the specs for the AY-3-8914, I suspect the problem is that the
    PSG isn't really designed to drive a typical PC's printer port.  I 
    will have to design and build a better electrical interface.  So, at 
    this time, I would recommend you do NOT attempt to build an ECScable 
    unless you really know what you're doing.
==============================================================================



==============================================================================
 ECScable WIRING
==============================================================================

The ECS cable has 8 lines going from PC->ECS, and 5 lines going from
ECS->PC.  One line in each direction is used as a clock line, leaving
7 data lines from PC->ECS and 4 data lines from ECS->PC.  The
wiring is as follows:


      Right hand
      controller:       PC Cable pin #
      ---------------   ---------------------------
      Bit 0   Pin 1     Pin 15 (FAULT)
      Bit 1   Pin 2     Pin 13 (SLCT)
      Bit 2   Pin 3     Pin 12 (PE)
      Bit 3   Pin 4     Pin 10 (ACK)
      Bit 4   Pin 5     Pin 11 (BUSY)
      Bit 5   Pin 6     n.c.
      Bit 6   Pin 7     n.c.
      Bit 7   Pin 8     n.c.
      GND     Pin 9     Any of Pin 18 thru 25

      Left hand
      controller:       PC Cable pin #
      ---------------   ---------------------------
      Bit 0   Pin 1     Pin 2 (Data bit 0)
      Bit 1   Pin 2     Pin 3 (Data bit 1)
      Bit 2   Pin 3     Pin 4 (Data bit 2)
      Bit 3   Pin 4     Pin 5 (Data bit 3)
      Bit 4   Pin 5     Pin 6 (Data bit 4)
      Bit 5   Pin 6     Pin 7 (Data bit 5)
      Bit 6   Pin 7     Pin 8 (Data bit 6)
      Bit 7   Pin 8     Pin 9 (Data bit 7)
      GND     Pin 9     Any of Pin 18 thru 25


==============================================================================
 PARTS LIST
==============================================================================

The ECScable does not require many parts for construction.  You can get
all the needed parts at your local Radio Shack:

    26-152B    Shielded RS-232C Cable, Female DB9 to Female DB9
    276-1536A  Shielded Metallized Hood for 25-pin connectors
    276-1429   25-Position Mail D-Sub Connector, with crimp-style pins.


==============================================================================
 SIGNALING PROTOCOL:  PC MASTER MODE
==============================================================================

For reliability reasons (primarily due to the fact that a 0->1 transition
is faster than a 1->0 transition, owing to pullup resistors on the bus),
all data transactions occur over a full "clock" period, with valid data
being read at the end of a 1->0 clock transition.

The PC always provides the clock, and the ECS echos the clock.
The following diagram shows one read/write cycle from the PC's
perspective:
                    _____
  PC Clock:  ______|     |_________
                   ________________
  PC Data:   XXXXX>>>>>>>__________<
                       ______
 ECS Clock:  _________|      |_____
                   ________________
 ECS Data:   XXXXX>>>>>>>>>>_______<
                     ^^         ^^
               Data changing   Data valid


Basically, the following code seems to reliably do a loopback:

    PC side:

    do {
        outb(i | c, base + 0);
        do {
            j = (0x80 ^ inb(base + 1)) >> 3;
        } while ((j & 1) != c);
        c = !c;
    } while (c == 0);

    INTV side:

    @@l MVI     $FF,  R0
        MVO     R0,   $FE
        B       @@l


All data transmissions are simultaneously transmits and receives.
If a given direction isn't sending or receiving something, it should
set the lines to all 1s.  (Alternately, the Inty can just echo back
the value it read -- serves as Also, when the line is in the idle state,
both directions should leave the line floating at all 1s.  (eg. 0xFF)
The Inty should set its xmit lines to the "input" direction to let the
drivers on the PSG rest.


==============================================================================
 COMMAND SEQUENCE
==============================================================================

The PC alerts the ECS that it wants to issue a command by sending
0x00 on its line, thereby coming out of idle and alerting the ECS
that it wants to do something.  It waits for the ECS to respond in
kind by responding with 0x00.  Once this short handshake completes,
the PC can send a command.

For robustness, each command is sent twice -- as the command word
and as the inverse of the command word.  The PC then sends up
0xAA and expects to read back a single nibble from the ECS.  If
it reads back a 0x0A, it assumes the Inty read the command ok.
If it reads back anything else, it assumes the Inty did not read
the command correctly.  If the PC sees that the Inty did not read
the command correctly, it returns to the idle state and then retries.
Otherwise, the PC sends the body of the command packet (if any).

Example:

      PC->ECS       ECS->PC
      11111111      11111        Idle
      00000000      11111        PC initiates handshake
      00000000      00000        ECS responds to handshake
 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      01011101      00000        PC sends a command, clock HI
      01011101      xxxx1        ECS responds by bringing clock HI
      01011100      xxxx1        PC sends a command, clock LO
      01011100      xxxx0        ECS responds by bringing clock LO
 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      10100011      xxxx0        PC sends command inverse, clock HI
      10100011      xxxx1        ECS responds by bringing clock HI
      10100010      xxxx1        PC sends command inverse, clock LO
      10100010      xxxx0        ECS responds by bringing clock LO
 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      10101011      xxxx0        PC sends a zero to finish cmd, clock HI
      10101011      01011        ECS responds with 0xA, clock HI
      10101010      01011        PC sends a zero, clock LO
      10101010      01010        ECS responds with 0xA, clock LO
 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -



==============================================================================
 COMMANDS
==============================================================================

 (Note: LSB is clock, and is shown as zero in commands below)

 0x00:  PING (just echo back "OK!")
 0x02:  Soft Reset to GAME
 0x04:  Soft Reset to MONITOR
 0x06:  Active video OFF
 0x08:  Active video ON
 0x0A:  Disable ECS ROMs
 0x0C:  Enable ECS ROMs
 0x0E:  NOP (reserved)

 0x10:  PC->ECS xfer bytes  within Inty address space
 0x12:  PC->ECS xfer decles within Inty address space
 0x14:  PC->ECS xfer words  within Inty address space
 0x16:  NOP (reserved)

 0x18:  ECS->PC xfer bytes  within Inty address space
 0x1A:  ECS->PC xfer decles within Inty address space
 0x1C:  ECS->PC xfer words  within Inty address space
 0x1E:  NOP (reserved)

 0x20:  PC->ECS xfer bytes  within Intellicart address space
 0x22:  PC->ECS xfer decles within Intellicart address space
 0x24:  PC->ECS xfer words  within Intellicart address space
 0x26:  NOP (reserved)

 0x28:  ECS->PC xfer bytes  within Intellicart address space
 0x2A:  ECS->PC xfer decles within Intellicart address space
 0x2C:  ECS->PC xfer words  within Intellicart address space
 0x2E:  NOP (reserved)


The two RESET commands have no command body.  The PC side should
go to idle after issuing the command.


==============================================================================
 DATA FORMATS FOR BLOCK TRANSFERS
==============================================================================

The remaining memory transfer commands are followed by short header
which gives the address and length.  The data is sent as follows:

  +--------+--------+--------+--------+--------+--------+--------+--------+
  |   A7   |   A6   |   A5   |   A4   |   A3   |   A2   |   A1   | (clk)  |
  +--------+--------+--------+--------+--------+--------+--------+--------+
  +--------+--------+--------+--------+--------+--------+--------+--------+
  |   A15  |   A14  |   A13  |   A12  |   A11  |   A10  |   A9   | (clk)  |
  +--------+--------+--------+--------+--------+--------+--------+--------+
  +--------+--------+--------+--------+--------+--------+--------+--------+
  |             "Length - 1"                   |   A0   |   A8   | (clk)  |
  +--------+--------+--------+--------+--------+--------+--------+--------+
  +--------+--------+--------+--------+--------+--------+--------+--------+
  |                      Header Checksum                         | (clk)  |
  +--------+--------+--------+--------+--------+--------+--------+--------+

The header checksum is a simple 2s complement arithmetic sum of the first
three bytes.  Not horribly robust, but hey.  After the header, the PC
expects to read back 0xA (as above).  If it doesn't, then it goes back
to the idle state, waits a moment, and tries sending the command again.


After the header, the PC and the ECS send data back and forth.  For
the PC->ECS the following formats are used:

  For BYTE data, up to 7 bytes in a row in this format:
  +--------+--------+--------+--------+--------+--------+--------+--------+
  |   D7a  |   D6a  |   D5a  |   D4a  |   D3a  |   D2a  |   D1a  | (clk)  |
  +--------+--------+--------+--------+--------+--------+--------+--------+
  +--------+--------+--------+--------+--------+--------+--------+--------+
  |   D7b  |   D6b  |   D5b  |   D4b  |   D3b  |   D2b  |   D1b  | (clk)  |
  +--------+--------+--------+--------+--------+--------+--------+--------+
        ....

  Followed by one byte in this format to give the LSBs:
  +--------+--------+--------+--------+--------+--------+--------+--------+
  |   D0g  |   D0f  |   D0e  |   D0d  |   D0c  |   D0b  |   D0a  | (clk)  |
  +--------+--------+--------+--------+--------+--------+--------+--------+

  If there are more than 7 bytes remaining in a packet, 7 are always
  sent, otherwise only the remaining bytes are sent.  This eliminates
  the need for length bytes in the stream.

  - - - - - - - - - - - - -

  For DECLE data, up to 7 bytes in a row in this format:
  +--------+--------+--------+--------+--------+--------+--------+--------+
  |   D9a  |   D8a  |   D7a  |   D6a  |   D5a  |   D4a  |   D3a  | (clk)  |
  +--------+--------+--------+--------+--------+--------+--------+--------+
  +--------+--------+--------+--------+--------+--------+--------+--------+
  |   D9b  |   D8b  |   D7b  |   D6b  |   D5b  |   D4b  |   D3b  | (clk)  |
  +--------+--------+--------+--------+--------+--------+--------+--------+
        ....

  Followed by one byte in this format to give bit2 for the DECLEs:
  +--------+--------+--------+--------+--------+--------+--------+--------+
  |   D2g  |   D2f  |   D2e  |   D2d  |   D2c  |   D2b  |   D2a  | (clk)  |
  +--------+--------+--------+--------+--------+--------+--------+--------+

  Followed by two bytes in this format to give bit1 and bit0 for the DECLEs:
  +--------+--------+--------+--------+--------+--------+--------+--------+
  |   D1d  |   D0c  |   D1c  |   D0b  |   D1b  |   D0a  |   D1a  | (clk)  |
  +--------+--------+--------+--------+--------+--------+--------+--------+
  +--------+--------+--------+--------+--------+--------+--------+--------+
  |   D0g  |   D1g  |   D0f  |   D1f  |   D0e  |   D1e  |   D0d  | (clk)  |
  +--------+--------+--------+--------+--------+--------+--------+--------+

  If there are more than 7 decles remaining in a packet, 7 are always
  sent, otherwise only the remaining decles are sent.  This eliminates
  the need for length bytes in the stream.

  - - - - - - - - - - - - -

  For WORD data, up to 14 bytes in a row in pairs in this format:
  +--------+--------+--------+--------+--------+--------+--------+--------+
  |   D7a  |   D6a  |   D5a  |   D4a  |   D3a  |   D2a  |   D1a  | (clk)  |
  +--------+--------+--------+--------+--------+--------+--------+--------+
  +--------+--------+--------+--------+--------+--------+--------+--------+
  |   D15a |   D14a |   D13a |   D12a |   D11a |   D10a |   D9a  | (clk)  |
  +--------+--------+--------+--------+--------+--------+--------+--------+
        ....

  Followed by two decles in this format to give the missing bits:
  +--------+--------+--------+--------+--------+--------+--------+--------+
  |   D8d  |   D0c  |   D8c  |   D0b  |   D8b  |   D0a  |   D8a  | (clk)  |
  +--------+--------+--------+--------+--------+--------+--------+--------+
  +--------+--------+--------+--------+--------+--------+--------+--------+
  |   D0g  |   D8g  |   D0f  |   D8f  |   D0e  |   D8e  |   D0d  | (clk)  |
  +--------+--------+--------+--------+--------+--------+--------+--------+

  If there are more than 7 words remaining in a packet, 7 are always
  sent, otherwise only the remaining words are sent.  This eliminates
  the need for length bytes in the stream.

For ECS->PC the following formats are used:

  For BYTE data, data is just sent in "nickle" pairs:
  +--------+--------+--------+--------+--------+
  |   D3a  |   D2a  |   D1a  |   D0a  | (clk)  |
  +--------+--------+--------+--------+--------+
  +--------+--------+--------+--------+--------+
  |   D7a  |   D6a  |   D5a  |   D4a  | (clk)  |
  +--------+--------+--------+--------+--------+

  For DECLE data, data is just sent in "nickle" triads:
  +--------+--------+--------+--------+--------+
  |   D3a  |   D2a  |   D1a  |   D0a  | (clk)  |
  +--------+--------+--------+--------+--------+
  +--------+--------+--------+--------+--------+
  |   D7a  |   D6a  |   D5a  |   D4a  | (clk)  |
  +--------+--------+--------+--------+--------+
  +--------+--------+--------+--------+--------+
  |   n/a  |   n/a  |   D9a  |   D8a  | (clk)  |
  +--------+--------+--------+--------+--------+

  For WORD data, data is just sent in "nickle" quads:
  +--------+--------+--------+--------+--------+
  |   D3a  |   D2a  |   D1a  |   D0a  | (clk)  |
  +--------+--------+--------+--------+--------+
  +--------+--------+--------+--------+--------+
  |   D7a  |   D6a  |   D5a  |   D4a  | (clk)  |
  +--------+--------+--------+--------+--------+
  +--------+--------+--------+--------+--------+
  |   D11a |   D10a |   D9a  |   D8a  | (clk)  |
  +--------+--------+--------+--------+--------+
  +--------+--------+--------+--------+--------+
  |   D15a |   D14a |   D13a |   D12a | (clk)  |
  +--------+--------+--------+--------+--------+


After each transfer (in either direction) the transfer is ACKd by the
PC sending 0xAA and the ECS sending 0xA, in what I call "the Standard
1010 Handshake."


==============================================================================
 STANDARD 1010 HANDSHAKE
==============================================================================

No, this has nothing to do with those cheesy long distance phone
companies.  The 1010 handshake is simple.  The ECS transfer protocol
actually transfers data both directions simultaneously.  This is
an artifact of how the clock protocol works.  For most things in the
ECScable Monitor protocol, we only use one direction at a time.  However,
the "handshake" that's used to terminate most of the commands below uses
both directions simultaneously.

Basically, this handshake is quite simple.  The PC and the ECS send
the following bit patterns to each other:

    +---+---+---+---+---+---+---+-----+
    | 1 | 0 | 1 | 0 | 1 | 0 | 1 |(clk)|   PC -> ECS
    +---+---+---+---+---+---+---+-----+

                +---+---+---+---+-----+
                | 0 | 1 | 0 | 1 |(clk)|   ECS -> PC
                +---+---+---+---+-----+

For some of the transfers below (such as the command header), the PC ->
ECS transfer may be a different value.  The standard ECS->PC "All is
OK!" response is the one shown above though.


==============================================================================
 ECScable MONITOR FEATURES
==============================================================================

The ECScable Monitor is a small program which provides communication
routines for the ECScable.  The Monitor implements the PC MASTER
protocol, and provides support for the commands described above.

The Monitor resides at $C000 - $CFFF.  It also places writeable memory
at $0500 - $0FFF, for monitor variables and for program loading purposes.
$0500 - $07FF is writeable but not bankswitched.  $0800 - $0FFF is both
writeable and bankswitched, and thus is used for program loading purposes.

The Monitor works by hooking the ECS's initialization sequence very
early on.  The ECS initialization jumps to location $C043, and the 
Monitor takes over from there.  

On initial powerup, or when told to "Reset to Monitor", the Monitor will
take over and display the main Monitor screen.  The Monitor can be told
to "Reset to Game," in which case it will return to the normal ECS 
initialization sequence without entering the main monitor loop.  When
set to "Reset to Game," this mode will remain the default until either:

 -- A command is received to change the default, or
 -- The [1] and [9] keys are depressed on either hand controller during 
    reset to override Reset to Game.


Once a game is active, the ECScable Monitor will by default lie dormant
until activated again by the override sequence given above.  While dormant,
the ECScable functionality cannot be used, as the ECScable Monitor itself
will not be running.

To allow the ECScable to be used while a game is active, the Monitor 
provides support for Monitor-aware programs to allow ECScable commands
to be processed in the background.  This is accomplished by inserting
a tiny sequence of code within some periodically-occurring task in the
game program:


EC_LOC  EQU     $CF00
EC_MAG  EQU     $69
EC_POLL EQU     $CF01

        MVI     EC_LOC, R0
        CMPI    #EC_MAG,R0
        BNEQ    @@no_poll
        CALL    EC_POLL
@@no_poll

By placing this code in, say, an idle loop or other non-time critical
section of code, a game program can enable ECScable Monitor functionality
with minimal effort.  (Note:  Do *NOT* call it from an interrupt service
routine.)  Because the code detects the ECScable Monitor before it
jumps to it, it is safe to leave the code in even when the Monitor is
not present.  Very cool.

Once a game is modified to poll the ECScable Monitor, it becomes possible
again to send commands to the Monitor on-the-fly.  Not all commands
are supported in polling mode, however.  Only the following commands are
supported:

 0x00:  PING (just echo back "OK!")
 0x02:  Soft Reset to GAME
 0x04:  Soft Reset to MONITOR

 0x10:  PC->ECS xfer bytes  within Inty address space
 0x12:  PC->ECS xfer decles within Inty address space
 0x14:  PC->ECS xfer words  within Inty address space
 0x16:  NOP (reserved)

 0x18:  ECS->PC xfer bytes  within Inty address space
 0x1A:  ECS->PC xfer decles within Inty address space
 0x1C:  ECS->PC xfer words  within Inty address space
 0x1E:  NOP (reserved)

 0x20:  PC->ECS xfer bytes  within Intellicart address space
 0x22:  PC->ECS xfer decles within Intellicart address space
 0x24:  PC->ECS xfer words  within Intellicart address space
 0x26:  NOP (reserved)

 0x28:  ECS->PC xfer bytes  within Intellicart address space
 0x2A:  ECS->PC xfer decles within Intellicart address space
 0x2C:  ECS->PC xfer words  within Intellicart address space
 0x2E:  NOP (reserved)

Because upload and download commands are supported, it becomes possible
to implement run-time debugging features, such as "watch windows" and
so on using the ECScable protocol.  How cool is that?  The "ec_watch"
command provides just that functionality.  It's also possible to reload
the program without touching the Intellivision:  Just issue an "ec_load",
and it'll Reset to Monitor and reload the game automagically.


