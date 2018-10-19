These routines require a Cuttle Cart 3.  They provide basic serial
I/O access via the CC3's 16550-compatible UART.

The file "cc3serial.asm" contains a simple CC3 serial driver with
synchronous (ie. blocking) send and receive primitives.

The file "cc3ser2.asm" contains a much more involved driver that
is also somewhat slower.

The file "loopback.asm" contains a simple loopback demo that is
useful for testing purposes.  Anything the Intellivision receives,
it'll turn around and retransmit back.  It uses 19200-8-N-1 by
default.


