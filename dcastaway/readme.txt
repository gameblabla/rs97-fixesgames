                                                                                  
 _____       .____            ____. .________                                     
 NF"4NNN   .NNN"4NN         .NNF"NN `"""NF"""              
 N)   `NL  NN`    ` (NNNNL  (N.         N)    NNNNN. (N) .N) (N) NNNNN. (N)  (N)
 N)    4N  NF       `"  4N) (NNNN_.     N)    "` `NN `N) JNN (N` "` `NN  NN  NN
 N)    JN  NL       JNNNNN)   "4NNN)    N)   .NNNNNN  NN.N"N)NN .NNNNNN  (N)(N)
 N)   .NF  NN.    . NF   N) .     N)    N)   (N`  (N  (NJF 4NN) (N`  (N   4NNF 
 NL_JNNN   `NNN_JNN NNLJNN) (NN_JNN`    N)   (NN_NNN   NN` `NN  (NN_NNN   `NN`  For Dingux
 """""       `""""   """ "`  `""""      "`    `""``"   ""   ""   `""``"   (N)  
                                                                         .NN      

DCaSTaway is an opensource Atari ST emulator for the Dingux operating system on the Dingoo A320 system. 
It started life as Atari ST emulator for the Dreamcast console and has now been ported to the Dingoo.

DcaSTaway is a combination of two opensource emulator projects:

Hatari ------> 	http://hatari.sourceforge.net
Castaway ---->	http://castaway.sourceforge.net

The Castaway project includes an MC68000(tm) emulator and an Atari ST (tm) emulator. The user interface 
is based on the SDL library, thus the ST emulator is adaptable to many platforms.

Hatari is an Atari ST emulator for Linux, BSD, BeOS, Mac OS X and other systems that are supported by the
SDL library.

All code parts from both emulaltors were re-written and optimized for the Dreamcast, and DCaSTaway is the 
results of all of this effort. It is now aviable for the Dreamcast, PC - Windows and Linux, and Dingux on
the Dingoo A320.

Additonal details about this project can be found at http://chui.dcemu.co.uk/DCaSTaway.html


Index

1. Setup
2. Controls
3. Main Features
4. FAQ

------------------------------------

1. Setup

In order to use DcaSTaway you need an Atari ST Bios image (TOS). 

Other versions of the TOS rom can be used including EMUTOS, which is an open source project ----> http://emutos.sourceforge.net 

The TOS bios needs to be placed in the same directory as "DcaSTaway.dge" and named "rom". Note: make sure there is no file extension.

DcaStaway has its own GIU and disk selector so there is no need to use the browser contained in Dmenu or Gx2menu.

------------------------------------

2. Controls

A: Joystick fire button
B: can be used a key map button, is mapped to UP by default
X: Mouse Right button  
Y: Mouse Left button
L: Switches between joystick and mouse control (when using mouse press R to chnage the mouse speed)
R: Virtual keyboard, use the A button to press a key (B, X & Y) can be used to map a key to, return to the main menu to reset keymaps).
SELECT + START: Brings up the UAE4all menu
SELECT + L: Quick load state.
SELECT + R: Quick save state.
SELECT + UP: Brightness up.
SELECT + DOWN: Brightness down.
SELECT + LEFT: Volume down.
SELECT + RIGHT: Volume up.
SELECT + A: Increase Dingoo overclocking
SELECT + B: Decrease Dingoo overclocking
SELECT + X: Move up to next save state
SELECT + Y: Move down to next save state
START: SuperThrottle on/off

------------------------------------

3. Main Features

------------------------------------

Disk Images

Two drives are emulated in DcaSTaway, Disk images can be placed in any directory on your SD card. Selecting a disk image using the A button 
mounts that disk in Drive 1 (main drive), you can use the Y button to mount a disk in Drive 2 (second drive). Pressing R jumps to the top of 
the disk image menu and pressing L jumps to the bottom.

During emualtion the drive display at the bottom of the screen shows a spinning disk to indicate when the drive is being accessed.

Disk images need to be in ST. (Atari ST disk image) or zip. (zipped) format.

Super Thottle 

Pressing start switches on the Super Throttle mode, This instantly turnsframe skip to maximum and switches off the sound. This utility is 
useful if you want to skip long loading times or intros.

Frameskip

Frameskip can be set between 0-5 or Auto, frameskip helps to speed up emulation by missing the number of frames specified. Most games tested 
run fine on the default of Auto, but in some cases gfx and sound issues can occur. 

Where there are issues it is best to experiment to find the setting that suits best. 


Save States 

You can save your progress at any point by using the save state feature. There are four save states slotes available. Press Y under the main 
menu to bring up the save state menu. 

------------------------------------

4. Frequently Asked Questions.

Q.) Why won't DcaSTaway start?  

A.) Make sure you have put the "rom" image in the same directory as "DcaSTaway.dge" and "data" directory. Make sure the "rom" file does not have
a file extension.

Q.) Why can't I get xxxxxx game to work.

A.) First make sure the disk image in ST format, STX images do not work. If that doesn't work then it could be a bad disk image. Some disk images 
with cracks or trainers may well be incompitable.

Q) Why do I get gfx or sound problems with xxxxxx game.

A.) This could be due to compatbility problems. Try different frameskip settings.
 
Q.) Are compressed disk images supported?

A) Yes, in zip format.

Q.) Does the Emulator include support for the STE and Falcon?

A) There is partial support for the STE, so some games/software may work. There is no support for the Falcon.

Q.) My savestate won't load, why is this?

A.) You need to make sure that you have the disk you used to save the state in Drive 1. For example, if you saved the state with Disk 1 in Drive you
will need to insert Disk 1 in the Drive 1 before the save state will be recognised.

-----------------
Thanks to Scott Ralph
