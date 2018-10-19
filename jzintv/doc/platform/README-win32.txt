 jzIntv 1.0:  Setting up jzIntv under Windows NT and Windows XP
==============================================================================

The following instructions should work for most Windows NT-based Windows
variants, including Windows NT 4.0, Windows 2000 and Windows XP.  They will 
not work with Windows 9x.

jzIntv is not really a Windows program so much as it is a Linux / UNIX program 
that is compiled for Windows.  Therefore, jzIntv does not work like most 
Windows applications.

jzIntv controls itself through two components:  Environment variables and 
command line flags.  Windows let you set up environment variables using the 
"System" control panel.  Windows lets you set command line flags when you run 
jzIntv.


Environment Variables
---------------------

If this is your first time installing jzIntv, the following steps will set you 
up with the configuration I currently recommend:


1.  Unpack the jzIntv Zip file under C:\Program Files\.  This should give you 
    a folder named C:\Program Files\jzintv-1.0\.

    (Note:  The exact name of the folder will change with each release of 
    jzIntv.  Verify the name of the folder that jzIntv unpacks to.)

2.  Copy all of your game ROMs into C:\Program Files\jzintv-1.0\rom\.  This 
    includes "exec.bin" and "grom.bin".

3.  Go to the System Control Panel, and perform the following steps:

    a.  Tell Windows where jzIntv is installed.
    
        (1) Click on the "Advanced" tab at the top.

        (2) Click on the "Environment Variables" button near the bottom

        (3) Click "New."

        (4) Under "Variable Name", put "JZINTV_DIR".

        (5) Under "Variable Value", put "C:\Program Files\jzintv-1.0".
            (Note:  If you installed jzIntv in a different location,
            put that directory here.)

        (6) Click "ok," but leave the Environment Variables window open.

    b.  Add jzIntv to your PATH as follows:


        (1) In the Environment Variables window, scroll through the "System 
            Variables" box (the lower box), and locate "Path."  Highlight 
            "Path" by clicking on it, and then click "Edit."

        (2) Do NOT delete the current variable value.  Append the following
            to the variable value:  ";%JZINTV_DIR%\bin"

        (3) Click "ok," but leave the Environment Variables window open.

    c.  Tell Windows to tell jzIntv where to find your ROMs:

        (1) Click "New."

        (2) Under "Variable Name", put "JZINTV_ROM_PATH".

        (3) Under "Variable Value", put "%JZINTV_DIR%\rom".

    d.  If you wish to also use SDK-1600, tell Windows to tell SDK-1600
        where to find SDK-1600's demo and library files:

        (1) Click "New."

        (2) Under "Variable Name", put "AS1600_PATH".

        (3) Under "Variable Value", put "%JZINTV_DIR%\examples".

    e.  Close the System Control Panel.

4.  Fire up jzintv.  Click "Run..." under the start menu, and type
    "jzintv gamename".  jzIntv should launch the requested game.

    Note:  If the game filename has space characters, you need to put quotes 
    around the name.  For example, suppose the game is named Space Armada.bin.
    You would have to type the following:

        jzintv "Space Armada"

    jzIntv will automatically try to add ".rom", ".bin", ".int", and ".itv" 
    when it searches for the game.  Therefore, you need not type this part of 
    the name.

    To run jzIntv with additional flags, such as to specify full screen display
    or a different display resolution, simply add the flags anywhere on the
    command line.  I recommend putting the flags before the game name:

        jzintv [flags] "Game Name"


If you are upgrading jzIntv, and have previously followed the instructions
above, then you only need to do two minor steps to point jzIntv to the new 
version:

1.  Use the System Control Panel to change JZINTV_DIR to point to the new 
    folder.  (See the directions above for how to change or add environment 
    variables using the System Control Panel.)

2.  Either:
    a.  Copy your ROMs to the new installation folder.  This is appropriate 
        if you're deleting the previous installation.  It's also the easiest, 
        but uses the most disk space.

    b.  Change JZINTV_ROM_PATH so it points to the ROMs in the older
        installation.  To do this, use the System Control Panel to edit the 
        JZINTV_ROM_PATH variable as follows:

        (1) Delete the existing value.

        (2) Type in the full path to where the ROMs are stored.




Default Flags
-------------

jzIntv currently does not have a mechanism to change the default flags it uses 
when the user invokes it.  The easiest way to specify a default set of flags 
to jzIntv is to write a batch file that "wraps" jzIntv as follows.


1.  Configure jzIntv's environement variables as described above.

2.  Open up Notepad, and enter the following into the buffer:

        jzintv [flags] %1 %2 %3 %4 %5 %6 %7 %8 %9

    where [flags] includes the list of flags you wish to pass to jzIntv by 
    default.  For example, if you always want to run jzIntv in full-screen mode
    at 640x480 with the Intellivoice enabled, you might write:

        jzintv -z1 -f1 -v1 %1 %2 %3 %4 %5 %6 %7 %8 %9


3.  Save this file as "C:\Program Files\jzintv-1.0\bin\go-jzintv.bat".
    (If you installed jzIntv under a different location than 
    "C:\Program Files\jzintv-1.0", change the above path accordingly.)

4.  To use the default flags, start jzIntv by typing "go-jzintv" instead
    of "jzintv".  



