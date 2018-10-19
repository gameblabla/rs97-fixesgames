
    Welcome to Dingux CAP32

Original Author of Caprice32

  Ulrich Doewich

Author of the Dingux port version 

  Ludovic.Jacomme also known as Zx-81 (zx81.zx81@gmail.com)

1. INTRODUCTION
   ------------

  Caprice32 is one of the best emulator of the Amstrad CPC home computer series
  running on Windows and Unix.  The emulator faithfully imitates the CPC464,
  CPC664, and CPC6128 models (see http://sourceforge.net/projects/caprice32)

  Dingux CAP32 is a port on Dingux of my previous Wiz port version.

  This package is under GPL Copyright, read COPYING file for
  more information about it.

  The code of the library (cpccat) used to display the content of a 
  DSK file is not GNU, so it is not part of this package.


2. INSTALLATION
   ------------

  Unzip the zip file, and copy the content of the directory game to your
  SD card.

  Put your snapshot files generated using the Windows version of Caprice32
  on "snap" sub-directory, or disk image on "disk" sub-directory.

  For any comments or questions on this version, please visit 
  http://zx81.zx81.free.fr, http://zx81.dcemu.co.uk or 
  http://www.gp32x.com/

3. CONTROL
   ------------

3.1 - Virtual keyboard

  In the CPC emulator window, there are three different mapping 
  (standard, left trigger, and right Trigger mappings). 
  You can toggle between while playing inside the emulator using 
  the two Dingux trigger keys.

    -------------------------------------
    Dingux        CPC             (standard)
  
    Y          Backspace
    X          ENTER
    B          Joystick Fire 1
    A          Joystick Fire 2
    Up         Up
    Down       Down
    Left       Left 
    Right      Right
  
    -------------------------------------
    Dingux        CPC      (left trigger)
      
    Y          FPS
    X          Load state
    B          Save state
    A          Render mode
    Up         Up
    Down       Down
    Left       Left
    Right      Right
     
    -------------------------------------
    Dingux        CPC      (right trigger)
      
    Y          Backspace
    X          Space
    B          Auto-fire mode (on/off)
    A          Escape
    Up         Up
    Down       Down
    Left       Dec auto-fire frequency
    Right      Inc auto-fire frequency
    
    Press Menu             to enter in emulator main menu.
    Press Select           open/close the On-Screen keyboard

  In the main menu
  
  X      Go Up directory
  B      Valid
  A      Valid
  Y      Go Back to the emulator window
  
  The On-Screen Keyboard of "Danzel" and "Jeff Chen"
  
  Use digital pad to choose one of the 9 squares, and
  use X, Y, A, B to choose one of the 4 letters of the
  highlighted square.
  
  Use LTrigger and RTrigger to see other 9 squares
  figures.
    

4. LOADING CPC SNAPSHOT FILES
   ------------

  Using the Linux or Windows version of Caprice32 on your PC, you can save 
  any snapshot files of your CPC games or applications.

  You can then save those snapshot files (with .zip, .sna or .snz file extension)
  on your SD card in the 'snap' directory. Then, while inside CPC
  emulator, just press SELECT to enter in the emulator main menu, and 
  then using the file selector choose one snapshot file to load in the
  RAM of your emulator.

  Save state files are now saved using gzip compression, with SNZ as
  file extention). It's much faster to save or load states. You can use 
  gzip or 7-zip to convert old SNA to SNZ. 
  SNA file format is still supported for loading, so you convert your 
  previous saved files (or original Caprice32 files), inside the emulator.


5. LOADING CPC DISK FILES
   ------------

  If you want to load disk image in the virtual drive A of your emulator,
  you have to put your disk file (with .zip or .dsk file extension, or gzipped
  disk file with .dsz file extention) on your SD Card in the 'disk' directory.
  You proceed as previously described for snapshot files, and your disk is
  then inserted in the drive 'A' of your emulator.  

  Gzipped disk files are not writable, but they are much faster to load.
  As for SNZ file, you might use 7-zip to convert .dsk files to .dsz.

  The first method to launch a game after loading a disk file, is to set the
  "Disk startup" option to "full" and to set the "Reset on startup"
  option to true.

  When you insert a disk using the "Load disk" menu then game will start
  automatically.

  If you want to can set the "Disk startup" option to "manual", and browser
  yourself the content of the disk using the "Explore disk" menu.

  If you want to do all this manually, set the "Disk startup" option to
  "manual" and proceed as explained in the following paragraphs.

  To display the content of your drive, you have to use the virtual keyboard
  (press Select key) and type the CPC command 'CAT' followed by ENTER (Triangle).
  For example if you have loaded the disk file of the game "Green Beret" :

       CAT

  You should see something like this :

       Drive A: user    0
       
       GBERET    .     * 15K

       140K free

  You can also use directly the shortcut in the emulator menu (Command CAT option)
   
  Then if you want to run a program GBERET that is on your drive 'A', you have to
  use the CPC command 'RUN' as follow :

       RUN"GBERET

  If the filename of the .dsk (here gberet.dsk) is also the name of the 
  program you want to run (here gberet) then you can use directly the Command RUN" 
  in the emulator menu. 


  If the command to run is different from the filename then you can specify 
  the proper command to RUN in the file run.txt, for example :

    mygame=DISC

  Then, while using the Command RUM/CPM menu, the emulator will type RUN"DISC
  instead of RUN"mygame. Have a look the file run.txt for details about the syntax.

  You can specify CPM games in the run.txt file using the following syntax:

     mygame=|CPM

  Then, when using the Command RUM/CPM menu, the emulator will use the |CPM
  command (instead of RUN"mygame).

  You can use the virtual keyboard in the file requester menu to choose the
  first letter of the game you search (it might be useful when you have tons of
  games in the same folder). Entering several time the same letter let you
  choose sequentially files beginning with the given letter. You can use the
  Run key of the virtual keyboard to launch the game.

6. CHEAT CODE (.CHT)
   ----------

You can use cheat codes with Dingux-CAP32 emulator.  You can add your own cheat
codes in the cheat.txt file and then import them in the cheat menu.  

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

Let's say you're playing Prehistorik2 and you want to find the memory address
where "number lives" is stored.

. Start a new game in Prehistorik2
. Enter in the cheat menu. 
. Choose Save Ram to save initial state of the memory. 
. Specify the number of lives you want to find in
  "Scan Old Value" field.
  (for Prehistorik2 the initial lives number is 4)
. Go back to the game and loose a life.
. Enter in the cheat menu. 
. Specify the number of lives you want to find in
  "Scan New Value" field.
  (for Prehistorik2 the lives number is now 3)
. In Add Cheat you have now one matching Address
  (for Prehistorik2 it's 3F30)
. Specify the Poke value you want (for example 5) 
  and add a new cheat with this address / value.

The cheat is now activated in the cheat list and you can save it using the
"Save cheat" menu.

Restart the game and let's enjoy Prehistorik2 with infinite life !!

7. COMMENTS
   ------------

You can write your own comments for games using the "Comment" menu.  The first
line of your comments would then be displayed in the file requester menu while
selecting the given file name (snapshot, disk, keyboard, settings).


8. LOADING KEY MAPPING FILES
   ------------

  For given games, the default keyboard mapping between Dingux Keys and CPC keys,
  is not suitable, and the game can't be played on DinguxCAP32.

  To overcome the issue, you can write your own mapping file. Using notepad for
  example you can edit a file with the .kbd extension and put it in the kbd 
  directory.

  For the exact syntax of those mapping files, have a look on sample files already
  presents in the kbd directory (default.kbd etc ...).

  After writting such keyboard mapping file, you can load them using the main menu
  inside the emulator.

  If the keyboard filename is the same as the snapshot (.sna or .snz) or  
  disk filename (.dsk or .dsz) then when you load this snapshot file or this disk,
  the corresponding keyboard file is automatically loaded !

  You can now use the Keyboard menu and edit, load and save your
  keyboard mapping files inside the emulator. The Save option save the .kbd
  file in the kbd directory using the "Game Name" as filename. The game name
  is displayed on the right corner in the emulator menu.

  If you have saved the state of a game, then a thumbnail image will
  be displayed in the file requester while selecting any file (snapshot, disk,
  keyboard, settings) with game name, to help you to recognize that game later.


9. SETTINGS
   ------------

  You can modify several settings value in the settings menu of this emulator.
  The following parameters are available :

  Sound enable       : enable or disable the sound
  Display fps        : display real time fps value 
  Speed limiter      : limit the speed to a given fps value
  Skip frame         : to skip frame and increase emulator speed
  Ram size           : memory size of the CPC
  Render mode        : many render modes are available with different 
                       geometry that should covered all games requirements
  Render delta Y     : move the center of the screen vertically
  Green color        : green monochrome display
  Clock frequency    : Dingux clock frequency, by default the value is set to
                       200Mhz, and should be enough for most of all games.
  Sound tick ratio   : Increase sound tick ratio to speed up the emulator
  Disk startup       : when a new disk is inserted the emulator can try to
                       find the name of the filename to load and start it
                       automatically. This option specify what to do when
                       a new disk is inserted.
  Reset on startup   : reset emulator when loading a disk


10. JOYSTICK SETTINGS
   ------------

  You can modify several joystick settings value in the settings menu of this emulator.
  The following parameters are available :

  Swap Analog/Cursor : swap key mapping between Dingux analog pad and Dingux digital pad
  Auto fire period   : auto fire period
  Auto fire mode     : auto fire mode active or not


11. COMPILATION
   ------------

  It has been developped under Linux FC9 using gcc with DINGUX SDK. 
  All tests have been done using a Dingoo with Dingux installed
  To rebuild the homebrew run the Makefile in the src archive.

  Enjoy,

            Zx
