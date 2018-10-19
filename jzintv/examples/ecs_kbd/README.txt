scan_kbd.asm  Simple driver to read the ECS QWERTY keyboard.
scan_syn.asm  Simple driver to read the ECS synthesizer keyboard.

Both use different strategies for scanning based on the unique 
characteristics of each device.  The ECS keyboard scanner, for
instance, will correctly resolve the shift key for all keys on
the keyboard, whereas the synthesizer scanner would not be able
to.

See "kbd_test" and "terminal" for examples using the QWERTY scanner.
See "syn_test" and "synth" for examples using the synthesizer scanner.


Note:  The drivers currently assume you're using "cart.mac" or similar,
as they rely on the BYTEVAR/WORDVAR/BYTEARRAY/WORDARRAY style macros
to allocate storage.  If you are not using cart.mac, you will either
need to provide compatible macros, or allocate storage for these
drivers another way.

The ECS keyboard drivers (both QWERTY and synth) are both placed in
the public domain.
