
 Ball Breaker, Arkanoid like type.
 Done by Clément CORDE.
 Contact : c1702@yahoo.com


> History:

 A few years ago, I worked in the video game industry (which was a really bad experience, btw). As I never programmed a ball breaker type game, I wanted to make a little one, just for me, and that's what I did. One of my coworkers seemed interested in the project, so I asked him if he wanted to have the source code to see how I did it. This guy then suggested me to put my code online, for it might interest some people. I'm a bit sceptical on that point, but here it is. And if it may be of some use to someone, cool.

 I think there's nothing revolutionnary in this code, but it contains some principles that might guide people beginning programming (even if I don't pretend to be an expert !): How to manage sprites, animations, monsters, shots... At least, it is one way of doing things. And there are some little tricks!


> Copyrights:

 No, not mine! :)

 As I made this game for me and that I'm not an artist, I took graphics the way I could, in order to have the least possible to draw by myself. For a ball breaker, Arkanoid seemed to be an ideal model. But this means that part of the graphics are probably under copyright (Taito ?). As the goal is absolutely not to make money, let's cross fingers and hope I won't get into trouble.

 Monsters, explosions, levels' frame and the base bat are from the Atari ST version.
 Backgrounds come from the StrategyWiki website (http://strategywiki.org/wiki/Arkanoid/Walkthrough).
 The font was found on Daniel Guldkrans website (http://www.algonet.se/~guld1/freefont.htm), and slightly retouched by me to fit the game needs.
 All additional graphics by myself.

 Just a little thing I have to state: My goal was not to make a strict Arkanoid clone, even if eventually it looks like it. So there are some differences.


> Questions:

 I will answer as possible by mail (c1702@yahoo.com) to questions, but please specify a very clear subject. The reason is that I'm receiving loads of spam, and of course, I don't read it before deleting it. So if you send a mail, don't write 'viagra' as subject!


> SDL:

 The game uses SDL to the minimum. I suppose it's possible to make a lot a great things with SDL, but that was not my goal. I haven't been coding anything looking like a video game for some time, and I wanted to see if I hadn't forget everything. So basically, I use SDL to get a video buffer and blit into it, in 256 colors because that's what I wanted to, and because 256 colors modes are easy to manage and have palettes. I also use SDL to load BMP files, because I didn't want to code a BMP loader (even if it's probably easy). Otherwise, everything is done by hand.


> Sound:

 There is no sound in this game. The reason? I have absolutely nothing to create, edit or rip sound. If someone is willing to send me some and if I've got some time, I'll have a look on how to include it in the game.


> Compilation:

 I'm using Code::Blocks, on Windows XP, with gcc and SDL.

 I didn't include the project file for the following reason: There are few chances for your directories to be the same as mine. Therefore, the project would not load anyway.

 Just create a new SDL project, copy all source files and the "gfx" directory in the project folder, include all source files to the project, it should work.

 Compilation options:
 -Wall : To see all warnings.
 -O3 : Optimize code for speed.
 -DNDEBUG (= #define NDEBUG) : In release mode, for the asserts not to be compiled anymore.

 The "includes.h" file. Ok, I know, it's not really clean to do this. I think it's convenient, cause it makes me include only one ".h" file in each ".c". The side effect is that every time you modify a ".h", the whole project is compiled. But hey, the project is not that big. If you want to, feel free to make the includes by yourself. :)

 The game is programmed in C. Why not C++? Because I wanted to make things in a certain way, and there wasn't really much gain by doing it with C++. I don't want to start an endless argument about the advantages of C++ over C. C++ is cool, I just didn't feel like using it for this project. If you'd like to port the game to C++, with classes, methods and stuff, go ahead (the code is modular, so it shouldn't be too difficult).


> Code:

 * Conventions:

 Numérical variables precedeed by 'n'.
 Pointers preceded by 'p'.
 Global variables precedeed by 'g'.
 Then, variable names starting by an upper case character.

 Examples:
 u32	nMyInteger;
 u32	pMyArray[NB_ELEMENTS];
 u32	*pPtr;
 u32	**ppPtr;	< pp because pointer's pointer.
 u32	gnMyGlobalInteger;
 aso...

 Exceptions: Loop indices. More like i, j, k, ix, iy...


> Little problems:

 * Background scrollings on menus' screens:
 I can't explain why, but on some machines, the scrolls are not smooth (I have this problem on my old machine, which I'm using to program). It seems like the refresh does not wait for the vblank. I'm not a SDL expert, but from what I could get in the SDL_Flip() documentation, I guess the 320x240 256 colors mode is not supported by the hardware, and then that the SDL_Flip() acts like a SDL_UpdateRect() and doesn't wait the retrace? To remove the scrollings from the mens, comment the line "#define MENU_Bkg_Mvt 1" on the top of the "menu.cpp" file.

 * Full-screen mode (F10):
 It seems the full-screen doesn't work properly with double screen configs, which are nowadays more and more common (SDL problem?). If you only have one screen (like me), it should work.

 * Unbreakable bricks and big balls:
 This is something that doesn't mix easily. Here is the problem: When a ball becomes bigger (ref point of the ball is on the center), if a part of this ball is inside an unbreakable brick, it should be moved to be outside. It is possible to do, but it is already complicated. If in addition to this, if the ball was between 2 bricks close enough, and that when the ball gets bigger the space between the 2 bricks becomes smaller than the diameter of the ball, you've got an impossibility (graphically, it can not work). So, in the levels containing unbreakble bricks, I didn't put any "getting bigger" bonuses. Problem solved. Now you think it's a lamer's solution? So I ask you the following question: If I hadn't said it, would you have noticed it? Making games' levels in a way of avoiding problems is a solution used in the video game industry more often than the players can think.


> Levels:

 Concerning the levels, I hadn't the time nor the envy to make a level editor. But I wasn't going to fill more than 30 arrays by the hand either, as some people do. I simply used a BMP file (levels.bmp).

 The picture is made of 16x20 blocks, each block being a level. The space used in the game is 15x17. Every pixel stands for a brick: Colors 1 to 10 for the normal bricks, 16 to 18 for the special bricks.

 The 2 last lines of each block (ie a 32 elements array) are standing for the bonuses table of the level, the bonuses being picked up randomly into this table (have a look on the right of the BMP file for the meanings of the colors, colors starting from index 128 for the bonuses). With this system, we can choose which items are going to be available depending on the level. For example, we can limit the number of items in the first levels, and increase the number of bonuses while the player advances into the game.

 Levels are grabbed from left to right, and from top to bottom. If the first pixel inside a 16x20 block is not a color number 0, then the level is grabbed.

 All the tables are generated by the "brk_lev" project, in the "stdout.txt" file (this would be the standart output on a unix system, ie the console) and manually included in the "levels.h" file. The code is very easy to understand. You'll notice that I simply added a function (CreateLevels()) in the base code generated by Code::Blocks when you create a new SDL project.

 Then you just have to put the "#define LEVEL_Max xx" line in the "breaker.h" file.

 The levels are those of the first Arkanoid (thanks to the strategywiki for the layouts), adapted to fit in the game area (which is larger) and also because the ball is bigger than the Arkanoid's one, which was making some levels very/too difficult. For example, in the 3rd level, the ball would not enter un a 1 brick space interval. Therefore, the level was very difficult to complete, but also very boring.


> Game:

 I did the same kind of ball bouncing on the bat that what I saw on Arkanoid I: The ball bounces regarding the position on which it touches the bat. The movement of the bat is not taken into consideration, so don't waste time trying to put on some "effect".

 The player can select the starting level (like on the Amiga version! Amiga rules!). I limited the choice to the 12 first levels, but you can recompile with #define LEVEL_SELECT_Max (LEVEL_Max - 1) instead of #define LEVEL_SELECT_Max (12 - 1) in breaker.h, then you'll be able to start from any level.

 Note about the bricks coming back: This is something I thought about at the beginning, but that I finally made at the end. As a result, there is only one brick coming back in the level 2. But it works.


> Doh:

 Some comments here:

 I never managed to reach this level when I was playing the original game, back in the 80's, so I don't know how this monster was behaving. The way he behaves here is entierely made up.

 I didn't have any sprites for the boss. On the last screen of the strategywiki website, I took a frame where Doh had the mouth opened. I managed, not easily, to draw a frame where he has the mouth shut. But making the animation for going from open to close is too difficult for me. If someone feels like to make it, send it to me ! :)


> Sprites:

 The sprites are grabbed when the game is launched, from BMP files.

 The sprites in this game are very simple, they don't even own a collision box in addition of their bounding rectangle. The nice graphics format we were using in the video game industry was the PSD (photoshop), which allows to add as many alpha layers as you want, and which is rather easy to decode (in 256 colors mode!). It's then easy to add collision boxes and many other useful things. In this game, apart from the balls (which are pixel tested), everything is rather square and I didn't really need additional collision boxes. The use of a BMP was then sufficient, plus SDL integrates a BMP loader. (Note: Since then, I started a PSD loader, I'll use it in a future project).

 In a "real" video game, you wouldn't grab the sprites this way. You'll use a tool to generate files, which would allow, among other things, to pack the files. As the result of the sprite grabbing process will anyways always be the same, real-time sprite grabbing is useless. Using a tool would also saving the effort of declaring the "e_Spr_xxx" by the hand in the "sprites.h" file. But creating such a tool takes time, and I wanted to keep things simple, without debugging to do on external programs.

 Technically, when a sprite is grabbed, an inverse mask is generated (a rectangle of the same size than the sprite): For each sprite's pixel, 0xFF if there is nothing to draw, 0 if there is a pixel to draw. When a sprite has to be drawn, we make a "hole" with a AND at the location where pixels have to be put, then we poke those pixels with an OR.

 * How to add sprites?

 At this point, it is important to make the difference between a sprite and an animation. Of course, animations are made of sprites, but their respective management are completely separated.

 - Adding a sprite:

 To start easily, add one of your sprite into an existing BMP (it's gonna be easier for the palettes). The sprite should be put inside a rectangle of color 0 (The CGA pink). The marks on the top and left sides are marking the reference point of the sprite. The background of the graphic file should be of color 255.

 Once this is done, you now have to add a name in the list in the "sprite.h" file, at the proper location (grabbing order), and update the "chaining" (a sprite number is relative to the sprite preceeding itself).

 That's it, the sprite has been added. You can display it using the following function:
 void SprDisplay(u32 nSprNo, s32 nPosX, s32 nPosY, u32 nPrio);
 with: :
 nSprNo = Sprite number (e_Spr_<something>).
 nPosX, nPosY = Position on screen ((0,0) being the top left corner).
 nPrio = Priority, from 0 (below everything) to 255 (above everything).

 - Adding an animation:

 Once all the sprites have been added, you have to define an animation. This is done in the "anims.cpp" and "anims.h" files.

 In the "anims.cpp" file, you create a table.

 The structure is very simple:
 1 animation key (we'll speak about that later), then
 number of frames for the sprite to be displayed, sprite number to be displayed (repeated as many times as necessary).
 You can also add control codes, followed by a value (which depends on the code).

 Only 4 control codes have been defined up to now:
 e_Anm_Jump : Jump inside an animation.
 e_Anm_Goto : Initialize another anim.
 e_Anm_End : End of an anim, AnmGetImage() will return SPR_NoSprite, and the e_AnmFlag_End flag will be set (checked with AnmCheckEnd()). The animation slot is not freed.
 e_Anm_Kill : End of an anim. Returns -1 and frees slot (ex: dust).

 You have then to declare the table in the "anim.h" file: extern u32 gAnm_<Anim name>[];

 Now you can use it: You initialize an anim into a slot, and every frame you call AnmGetImage() on this slot to get the proper sprite number to display.

 Initialization:
 SlotNumber = AnmSet(gAnm_<Anim name>, SlotNumber or -1 for a new anim);
 You also have the AnmSetIfNew() function.

 The difference is that AnmSetIfNew() initializes an animation only if the animation requested is a new one AND if the animation key allows it (that is, if the priority of the requested animation is at least equal to the one of the current anim).

 But what is this animation key?

 It's a value that can be known anytime (through AnmGetKey()), so the first use we can make of it is to use it to distinguish different animations.

 Not really clear? Example: Most if the player's animation have a e_AnmKey_Null key. When the player is dying (and that it should not move anymore), the anim has then a e_AnmKey_PlyrDeath key. Inside the Brk_MovePlayer function, I test the death with the following line:
 if (AnmGetKey(gBreak.nPlayerAnmNo) == e_AnmKey_PlyrDeath) ...

 But that's not all. The animation key is made of 2 parts: A priority (the 16 higher bits) and an id (16 lower bits).

 It can be useful in situations like:
 A hero is waiting, with an idle anim. Shot button, we start the shot animation, which has a priority higher than the idle anim. That's it, from now on, if nothing happens, you can still try to set the idle anim, it will only be set when the shot anim will have ended. Only anims with a higher priority than the shot will be able to be set (a hit or the death, for example).

 This system doesn't fix all problems, but it's quite helpful.


> Monsters:

 This is how it works: The management is done in the mst.cpp file, and the code for the actual monsters is in the monsters.cpp file.

 Management: There is a table containing x slots (like for the shots, dusts...), each one corresponding to a monster.

 Each slot has a fixed size, have a look at the SMstCommon in the mst.h file. You can see in this struct that the monsters have a certain number of variables in common. For the specific variables, we use the pData[64] table, which lets you use up to 64 bytes of specific data for each monster. If 64 bytes seems few to you, I can assure you that on the contrary, it's oversized. But on PCs', the memory space is infinite!

 Then, the monsters. They've got an initialization function, and a main function (which is called every frame when the monster is active).

 When monsters need to use specific variables, we use a specific struct's pointer on the pData area. This is done like this : 
 SMstDoh *pSpe = (SMstDoh *)pMst->pData;

 Note: You have to pay extra attention to the size of the specific structures. If they are too big, there will be an overlap problem (the data at the end of one slot overlapping the next slot), and a nice mess to debug. The MstCheckStructSizes() function at the end of the monsters.cpp file is here to check. For the asserts not to be compiled in release mode, you have to specify the -DNDEBUG flag to the compiler.

 The monsters are using a phase variable, I'll let you have a look, I find it convenient.

 For other kind of games, let's say, for example, a shoot'em up, there is something to add to the monsters' management: Making ennemies appear (with the scroll), and making ennemies disappear (ennemies will do it themselves).


> Shots:


 Just a table containing slots, and a FireManage() to call every frame.

 The management of the shots going out of the screen is simplified due to the fact the the player's shots are only going up, and the boss' shots are only going down.

 What is good to know about shots is that it's not the shots which are testing monsters, it's monsters who are testing shots. And it's not the same thing. For example, the "door" monster (to go to the next level) will never test the shots. In other contexts, "asleep" (inactive) or dying monsters won't test shots either. Only the monsters "concerned" about the shots will test the shots, it's faster and much easier.

 The interesting thing to look at is the gnFireLastUsed variable in the FireGetSlot() and FireReleaseSlot() functions, which makes the search for an empty slot much faster than scanning the table from the beginning. This trick is used everytime there is a table of this nature (shots, dusts, aso...).


> Dusts:

 And now, (for something completely different... (^_^)) let's have a look at these "dusts". What is it?

 At the time I was working in the video game industry, we used to use these to manage easily many little effects. The first person to use this system used it to display "dust" at the landing of a character. The name remained.

 This list is very convenient: Dusts have their own lives, and once one is set, we never have a look anymore at it. You can use it for eveything, like smoke behind a missile for example (every x frames, a little cloud disappearing). In this game, I use dusts for the disappearing bricks, and the ennemies' explosions (which are not making any damage, so there is no need to generate a new shot)...

 The system is tunable at will. We can imagine giving a direction and a speed to the dusts, if we need these to move.


> Opacity (shadows):

 I made these the easiest possible way: The shade bob technique, that is a add (here, +6).

 It's good to think about it before making it, because the levels' palettes have to be ordered for it. Have a look at the level 1 palette (which contains all the palettes for all the ingame backgrounds, by the way). Each background uses 6 colors to max for its light part. We reserve 6 more colors for the shadows. Then, we reserve again 6 more colors for she shadows' shadows (I'll explain this later). Total: 18 colors reserved for each level.

 To display a shadow, we start like for a sprite, by making a hole on the destination at the location where the shadow has to be displayed. Then we pick up the pixels which have to be at this location on the ORIGINAL background image (not the double buffer), to which we add 6 and we poke it on the destination.

 Result:
 Normal (light) colors are now 6 colors farther in the palette, ie the shadows colors.
 Shadows colors also are 6 colors farther, ie in the shadows' shadows colors, which are the same than the shadows colors, and therefore the same colors. The trick is done.
 A light color becomes a shadow, and a shadow remains a shadow.

 With this method, we can have shadows overlapping themselves, it'll always work, for the source is always the ORIGINAL picture.

 It seems complicated, but it's done only with some AND, OR and NOT. Have a look at the SprDisplayLock() function, in the sprites.cpp file.

 The function, depending on a flag, displays either a shadow or a normal sprite.

 Then I had several options to display shadows.
 - Either, everytime I had to display a sprite, I'd display another one with an offset and with the shadow flag. Advantage: We can put a different offset on each sprite, if needed. Disadvantage: There is a double call to the SprDisplay() function, and so more sprites to sort. Shadows being always displayed behind everything, we're sorting them for nuthin'.
 - Or, and that's what I did, I'd use the shadow flag when I wanted to display a sprite and its shadow. Each sprite, shadowed or not, is sorted only one time. The SprDisplayAll() does 2 passes: The first one to display all the shadows (under the regular sprites), and a second one for the normal sprites. Disadvantage: All the sprites have the same offset, because the offset is set in the SprDisplayAll(). Advantage: Easy management, only one flag to add (with an OR) when you want a shadow under a sprite. No double call to SprDisplay(). And finally, it allowed me to add shadows in some dusts animations (brick disappearing) without touching a single thing in the dust routine. I simply added the flag in the anim table.

 You can easily go back from method 2 to method 1, just remove the shadows display loop in the SprDisplayAll() function.

 Note: For those who wish to do "real" opacity on a 256 colors image, you'd have to use a CLUT (Color Look Up Table, google it). It's not that difficult to code, but it was of no use for this project.


> How to "time" a routine:

 Nowadays, it's obviously no more possible to time a routine with the raster (as we could do a few years ago, by changing the color 0 before the routine to time and reseting it afterwards). At first, I tried to do it using SDL_GetTicks(), putting one before and one after the bit of code to time. But the bits of code I had to test were so small or so quick that most of the time, the difference was 0.

 So I found the following method (windows only), which is much more precise:

>>>
#include <windows.h>

In the function:

LARGE_INTEGER a,b,c;
QueryPerformanceFrequency(&c);
QueryPerformanceCounter(&a);

[Code to be timed]

QueryPerformanceCounter(&b);
printf("%d %d %d\n", (int)(b.QuadPart - a.QuadPart), (int)a.QuadPart, (int)b.QuadPart);
<<<

 This can be useful! :)


> Precalculations:

 I just made a sine and cosine table, 256 values.

 There are other possible precalculations:

 A SCR_Width elements table and a SCR_Height elements, to know at once which brick is at any (x,y) position. It would avoid divisions (and divisions are bad!). Note: Why didn't I choose a clever brick size, like 16 pixels for example? Because I was making the game on a PC, which are loaded with mhz, so I took a brick size that I found nice (24 pixels), even if not convenient. I thought I'd make the tables I just spoke about, but the game doesn't slow down, so I let the divisions.
 Additional note: I tried to make this table, and the results were surprising. There was almost no gain, in fact I even wonder if there was not a loss (the time is constant with 4 divs, and varying from a little better to much worse with the table). => Table removed.

 A SCR_Width elements table for the boss shots angles. There is real-time atan2() calculation, not really good for speed. I made it real time to check that the angles were correct, thinking that I'd later make a table, and it doesn't slow down, even on my old machine. So...

 The bricks table is 15 elements width. Obviously, it would be quicker if this table was 16 (with 1 column unused). I thought about expanding the ingame table to 16 to avoid a lot a multiplications by 15, but once again, it doesn't slow down...


> Frame rate:

 For the frame rate management, I got the inspiration from several bits of code found in internet tutorials. The problem is that I'm using the frame.c and frame.h from one project to another, so I really don't remember where those routines originally come from. I'd gladly credit whoever made the original tutorial, if ever this person contacts me.

 Basically, the FrameWait() function waits until a certain amount of clock ticks has passed since the last call to this function before returning. This is done like this because we can't access the vblank, especially in windowed mode! :)

 Obviously, this is a simplistic management which assumes that the game is not going to slow down. If this happens, it's not handled. What should be done is to do the game management (to compute eveything needed), but to skip the display (which takes time) once. That's why the display routines and the management routines are separated (all right, except for the monsters).

 About the SDL_Delay(3); inside the loop: That allows the operating system to handle other processes for a while, and to reduce the load on the processor. You can have a look at the processor usage with the Windows' task manager, with and without this line.

 There are other ways to manage the vblank, like an IRQ timer, for example. This makes things easier to know how many frames have been missed.


> Files:

 Files' list and their use:

sprites.cpp	Engine / Sprites management.
sprites.h

animspr.cpp	Engine / Animations management.
animspr.h

anims.cpp	Sprites animations tables.
anims.h		Tables declarations.

mst.cpp		Engine / Monsters management.
mst.h

monsters.cpp	Game monsters.
monsters.h

ctypes.h	Types declarations u8, s8, aso...

includes.h	Includes and structs definitions.

dust.cpp	Dusts management.
dust.h

fire.cpp	Shots management.
fire.h

font.cpp	Text display functions.
font.h

frame.cpp	Frame rate management.
frame.h

breaker.cpp	The game.
breaker.h

levels.h	Game's levels, this file is included in breaker.cpp.

menu.cpp	Menus and high-scores' management.
menu.h

preca.cpp	Precalculations functions.
preca.h

main.cpp	Main, main loop, menus loop, event handler.

gfx		Directory containing all graphics.

high.scr	High-scores table (binary, recreated if missing).
		It contains a small checksum, which allows to check that the file is not corrupted.


> Conclusion:

 I managed to do what I wanted to do, it went well, and I'm rather happy with the result (It doesn't look too buggy! (^_^)). Of course, it looks like a 15 years old game, but at first, I made it for me!

 I hope I didn't forget too many essential things entering into details, and that all this stuff could be of some use to some people.

 I hope as well that some will enjoy the game! :) (Even if to me, the fun part was the coding!).


--End of file--
