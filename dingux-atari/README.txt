
    Welcome to Dingux-ATARI

Original Authors of Atari800

  David Firth, Petr Stehlik and Atari800 Development Team 

Author of the PSP, GP2X-F100, GP2X-Wiz and Dingoo port versions

  Ludovic.Jacomme also known as Zx-81 (zx81.zx81@gmail.com)


1. INTRODUCTION
   ------------

  Atari800 is an emulator for the 800, 800XL, 130XE and 5200 models of
  the Atari personal computer. It can be used on console, FrameBuffer or X11.
  See http://atari800.sourceforge.net/ for further informations.

  PSPATARI is a port on PSP of the version 2.0.2 (April 08 2006) of Atari800
  from Petr Stehlik.

  Dingux-ATARI is a port of the PSP version to Dingoo/Dingux.

  This software is under GPL Copyright, read COPYING file for more 
  information about it.


2. INSTALLATION
   ------------

  Unzip the zip file, and copy the content of the directory game to your SD
  memory.

  Put your rom image files on "roms" sub-directory.

  For any comments or questions on this version, please visit 
  http://zx81.zx81.free.fr, http://zx81.dcemu.co.uk or 
  http://www.gp32x.com/

3. CONTROL
   ------------

  In the original Atari menu :

    LTrigger   Escape
    RTrigger   Start
    Y          Return
    B          Space 

    Joystick   Joystick

  In the ATARI emulator window 
  
  Normal mapping :
  
  Dingoo        Atari                    
  
  B          Fire 1
  Y          Fire 2
  X          Start
  A          Select

  Joystick   Joystick 
  
  LTrigger mapping :
  
  Dingoo        Atari                    
  
  B          Save state
  Y          FPS
  X          Load state
  A          Joystick

  Up         Console Start
  Down       Console Select
  Left/Right Render mode
  
  
  RTrigger mapping :
  
  Dingoo        Atari                    
  
  B          Auto-fire
  Y          Fire 2
  X          Return
  A          Space

  Left/Right Dec/Inc fire
  Up         Console Option 
  Down       Console Cold start
  
  LTrigger   Toogle with L keyboard mapping
  RTrigger   Toggle with R keyboard mapping
  
  
  Press Menu      to enter in emulator main menu.
  Press Select    open/close the virtual keyboard

  In the main menu
  
  RTrigger   Reset the emulator
  
  X   Go Up directory
  B   Valid
  A   Valid
  Y   Go Back to the emulator window
  
  The On-Screen Keyboard of "Danzel" and "Jeff Chen"
  
  Use the stick to choose one of the 9 squares, and use
  A, B, X, Y to choose one of the 4 letters of the
  highlighted square.
  
  Use LTrigger and RTrigger to see other 9 squares
  figures.


4. LOADING ATARI ROM FILES
   ------------

  If you want to load rom image in your emulator, you have to put your rom or
  cartridge files on your SD memory in the 'roms' directory. 

  The .zip, .rom, .atr or .bin file extension are usable with the Atari
  800-XE/XL Rom menu.  The .zip, .a52 file extension are usable with
  the Atari 5200 Cartridge menu.

  While inside Dingoo-Atari  emulator, just press Select to enter in the
  emulator main menu, and then using the file selector choose the rom or
  cartridge file to load in your emulator.

  Back to the emulator window, the rom should stard automatically.

  In the main emulator window you can only load Atari 800 roms and Atari 5200
  cartridge ...  If you want to load other kind of roms or cartridge, please use
  the "Original" Atari Menu menu (the white option in the emulator menu). 

  Then select "Cartridge management" then "Insert cartridge" and specify your
  disk image file, then choose the cartridge type 5200 (for exemple), and go
  back to the emulator using LTrigger.


5. CHEAT CODE (.CHT)
   ----------

You can use cheat codes with this emulator.  You can add your own cheat codes
in the cheat.txt file and then import them in the cheat menu.  

All cheat codes you have specified for a game can be save in a CHT file in
'cht' folder.  Those cheat codes would then be automatically loaded when you
start the game.

The CHT file format is the following :
#
# Enable, Address, Value, Comment
#
1,36f,3,Cheat comment

Using the Cheat menu you can search for modified bytes in RAM between current
time and the last time you saved the RAM. It might be very usefull to find
"poke" address by yourself, monitoring for example life numbers.

To find a new "poke address" you can proceed as follow :

Let's say you're playing Defend or Die and you want to find the memory address
where "number lives" is stored.

. Start a new game 
. Enter in the cheat menu. 
. Choose Save Ram to save initial state of the memory. 
. Specify the number of lives you want to find in
  "Scan Old Value" field.
  (for Defend or Die the initial lives number is 3)
. Go back to the game and loose a life.
. Enter in the cheat menu. 
. Specify the number of lives you want to find in
  "Scan New Value" field.
  (for Defend or Die the lives number is now 2)
. In Add Cheat you have now one matching Address
  (for Defend or Die it's 9E)
. Specify the Poke value you want (for example 3) 
  and add a new cheat with this address / value.

The cheat is now activated in the cheat list and you can save it using the
"Save cheat" menu.

Let's enjoy your game with infinite life !!

6. COMMENTS
   ------------

You can write your own comments for games using the "Comment" menu.  The first
line of your comments would then be displayed in the file requester menu while
selecting the given file name (roms, keyboard, settings).


7. SETTINGS
   ------------

You can modify several settings value in the settings
menu of this emulator.  The following parameters are
available :

  Sound enable : 
    enable or disable the sound

  Display fps : 
    display real time fps value 

  Speed limiter :
    limit the speed to a given fps value

  Skip frame : 
    to skip frame and increase emulator speed

  Virtual keyboard :
    enable or disable transparency of virtual keyboard

  Vsync : 
    wait for vertical signal between each frame displayed

  Clock frequency : 
    Dingoo clock frequency, by default the value is set
    to 200Mhz, and should be enough for most of all games.


8. JOYSTICK SETTINGS
------------

  You can modify several joystick settings value in the settings menu of this
emulator.  The following parameters are available :

  Swap Analog/Cursor : 
    swap key mapping between Dingoo analog pad and Dingoo digital pad

  Auto fire period : 
    auto fire period

  Auto fire mode : 
    auto fire mode active or not


9. LOADING KEY MAPPING FILES
   ------------

  For given games, the default keyboard mapping between Dingoo Keys and Atari
  keys, is not suitable, and the game can't be played on Dingoo-Atari.

  To overcome the issue, you can write your own mapping file. Using notepad for
  example you can edit a file with the .kbd extension and put it in the kbd 
  directory.

  For the exact syntax of those mapping files, save the keyboard settings
  inside the emulator and have a look on file presents in the kbd directory 
  (default.kbd etc ...).

  After writting such keyboard mapping file, you can load them using 
  the main menu inside the emulator.

  If the keyboard filename is the same as the rom file then when you load this
  rom file, the corresponding keyboard file is automatically loaded !

  You can now use the Keyboard menu and edit, load and save your
  keyboard mapping files inside the emulator. The Save option save the .kbd
  file in the kbd directory using the "Game Name" as filename. The game name
  is displayed on the right corner in the emulator menu.

  
10. COMPILATION
   ------------

  It has been developped under Linux FC9 using gcc with DINGUX SDK. 
  All tests have been done using a Dingoo with Dingux installed
  To rebuild the homebrew run the Makefile in the src archive.


    Enjoy,

               Zx
