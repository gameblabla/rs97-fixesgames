/* oshw.h: Platform-specific functions that talk with the OS/hardware.
 *
 * Copyright (C) 2001-2006 by Brian Raiter, under the GNU General Public
 * License. No warranty. See COPYING for details.
 */

#ifndef	_oshw_h_
#define	_oshw_h_

#include	<stdarg.h>
#include	"gen.h"

/* This is the declaration of the top layer's main function. It is
 * called directly from the real main() inside the OS/hardware layer.
 */
extern int tworld(int argc, char *argv[]);

/* Initialize the OS/hardware interface. This function must be called
 * before any others in the oshw library. If silence is TRUE, the
 * sound system will be disabled, as if no soundcard was present. If
 * showhistogram is TRUE, then during shutdown the timer module will
 * send a histogram to stdout describing the amount of time the
 * program explicitly yielded to other processes. (This feature is for
 * debugging purposes.) soundbufsize is a number between 0 and 3 which
 * is used to scale the size of the sound buffer. A larger number is
 * more efficient, but pushes the sound effects farther out of
 * synchronization with the video.
 */
extern int oshwinitialize(int silence, int soundbufsize,
		int showhistogram, int fullscreen);

/*
 * Timer functions.
 */

/* Control the timer depending on the value of action. A negative
 * value turns off the timer if it is running and resets the counter
 * to zero. A zero value turns off the timer but does not reset the
 * counter. A positive value starts (or resumes) the timer.
 */
extern void settimer(int action);

/* Set the length (in real time) of a second of game time. A value of
 * zero selects the default of 1000 milliseconds.
 */
extern void settimersecond(int ms);

/* Return the number of ticks since the timer was last reset.
 */
extern int gettickcount(void);

/* Put the program to sleep until the next timer tick.
 */
extern int waitfortick(void);

/* Force the timer to advance to the next tick.
 */
extern int advancetick(void);

/*
 * Keyboard input functions.
 */

/* Turn keyboard repeat on or off. If enable is TRUE, the keys other
 * than the direction keys will repeat at the standard rate.
 */
extern int setkeyboardrepeat(int enable);

/* Alter the behavior of the keys used to indicate movement in the
 * game. If enable is TRUE, the direction keys repeat whenever the
 * program polls the keyboard. Otherwise, the direction keys do not
 * repeat until the program polls the keyboard three times.
 */
extern int setkeyboardarrowsrepeat(int enable);

/* Turn input mode on or off. In input mode, only the arrow and letter
 * keys are recognized.
 */
extern int setkeyboardinputmode(int enable);

/* Return the latest/current keystroke. If wait is TRUE and no
 * keystrokes are pending, the function blocks until a keystroke
 * arrives.
 */
extern int input(int wait);

/* Wait for a key to be pressed (any key, not just one recognized by
 * the program). The return value is FALSE if the key pressed is a
 * "quit" key.
 */
extern int anykey(void);

/* Return a table suitable for displaying a help screen on the
 * available keyboard commands for the given context.
 */
extern tablespec const *keyboardhelp(int context);

/* Symbolic values for requesting a specific help table.
 */
enum {
	KEYHELP_INGAME,
	KEYHELP_TWIXTGAMES,
	KEYHELP_FILELIST,
	KEYHELP_SCORELIST
};

/*
 * Resource-loading functions.
 */

//DKS - disabled
/* Extract the font stored in the given file and make it the current
 * font. FALSE is returned if the attempt was unsuccessful. If
 * complain is FALSE, no error messages will be displayed.
 */
//extern int loadfontfromfile(char const *filename, int complain);

/* Free all memory associated with the current font.
 */
extern void freefont(void);



/* Extract the tile images stored in the given file and use them as
 * the current tile set. FALSE is returned if the attempt was
 * unsuccessful. If complain is FALSE, no error messages will be
 * displayed.
 */
extern int loadtileset(char const *filename, int complain);

/* Free all memory associated with the current tile images.
 */
extern void freetileset(void);

/* The font provides special monospaced digit characters at 144-153.
 */
enum { CHAR_MZERO = 144 };

/*
 * Video output functions.
 */

/* Create a display surface appropriate to the requirements of the
 * game (e.g., sized according to the tiles and the font). FALSE is
 * returned on error.
 */
extern int creategamedisplay(void);

//DKS - disabled
/* Select the colors used for drawing the display background, normal
 * text, bold (highlighted) text, and dim (grayed) text. The color
 * values are of the form 0x00RRGGBB.
 */
//extern void setcolors(long bkgnd, long text, long bold, long dim);

/* Fill the display with the background color.
 */
extern void cleardisplay(void);

//DKS modified
///* Display the current game state. timeleft and besttime provide the
// * current time on the clock and the best time recorded for the level,
// * measured in seconds. All other data comes from the gamestate
// * structure (referred to here as an opaque pointer).
// */
//extern int displaygame(void const *state, int timeleft, int besttime);
/* Display the current game state. timeleft and besttime provide the
 * current time on the clock and the best time recorded for the level,
 * measured in seconds. All other data comes from the gamestate
 * structure (referred to here as an opaque pointer).
 */
extern int displaygame(void const *state, int timeleft, int besttime, int showhint);

//DKS added new parameters, newbesttime, and wasbesttime:
//wasbesttime is 1 if time was a new best time, 0 if not.
//newbesttime is the new best time, can be negative 
//		and this indicates no time limit and its absolute value
//		is the then the actual number of seconds level took
/* Display a short message appropriate to the end of a level's game
 * play. If the level was completed successfully, completed is TRUE,
 * and the other three arguments define the base score and time bonus
 * for the level, and the user's total score for the series; these
 * scores will be displayed to the user.
 */
//extern int displayendmessage(int basescore, int timescore, long totalscore,
//			     int completed);
extern int displayendmessage(int basescore, int timescore, long totalscore,
		int completed, int newbesttime, int wasbesttime);

/* Display a (very short) message for the given number of
 * milliseconds. bold indicates the number of milliseconds the
 * message is with highlighting. After that (if the message is
 * still visible) it is rendered as normal text.
 */
extern int setdisplaymsg(char const *msg, int msecs, int bold);

/* Display a scrollable table. title provides a title to display. The
 * table's first row provides a set of column headers which will not
 * scroll. index points to the index of the item to be initially
 * selected; upon return, the value will hold the current selection.
 * inputcallback points to a function that is called to retrieve
 * input. The function is passed a pointer to an integer. If the
 * callback returns TRUE, this integer should be set to either a new
 * index value or one of the following enum values. This value will
 * then cause the selection to be changed, whereupon the display will
 * be updated before the callback is called again. If the callback
 * returns FALSE, the table is removed from the display, and the value
 * stored in the integer will become displaylist()'s return value.
 */
extern int displaylist(char const *title, void const *table, int *index,
		int (*inputcallback)(int*));

/* Symbolic values for requesting relative movement of the selection.
 */
enum {
	SCROLL_NOP			= -1,
	SCROLL_UP			= -2,
	SCROLL_DN			= -3,
	SCROLL_PAGE_UP		= -4,
	SCROLL_PAGE_DN		= -5,
	SCROLL_HALFPAGE_UP		= -6,
	SCROLL_HALFPAGE_DN		= -7,
	SCROLL_ALLTHEWAY_UP		= -8,
	SCROLL_ALLTHEWAY_DN		= -9
};

/* Display an input prompt to the user. prompt supplies the prompt to
 * display, and input points to a buffer to hold the user's input.
 * maxlen sets a maximum length to the input that will be accepted.
 * The supplied callback function is called repeatedly to obtain
 * input. If the callback function returns a printable ASCII
 * character, the function will automatically append it to the string
 * stored in input. If '\b' is returned, the function will erase the
 * last character in input, if any. If '\f' is returned the function
 * will set input to "". If '\n' is returned, the input prompt is
 * erased and displayinputprompt() returns TRUE. If a negative value
 * is returned, the input prompt is erased and displayinputprompt()
 * returns FALSE. All other return values from the callback are
 * ignored.
 */
extern int displayinputprompt(char const *prompt, char *input, int maxlen,
		int (*inputcallback)(void));

/*
 * Sound functions.
 */

/* Activate or deactivate the sound system. The return value is TRUE
 * if the sound system is (or already was) active.
 */
extern int setaudiosystem(int active);

/* Load a wave file into memory. index indicates which sound effect to
 * associate the sound with. FALSE is returned if an error occurs.
 */
extern int loadsfxfromfile(int index, char const *filename);

/* Specify the sounds effects to be played at this time. sfx is the
 * bitwise-or of any number of sound effects. If a non-continuous
 * sound effect in sfx is already playing, it will be restarted. Any
 * continuous sound effects that are currently playing that are not
 * set in sfx will stop playing.
 */
extern void playsoundeffects(unsigned long sfx);

/* Control sound-effect production depending on the value of action.
 * A negative value turns off all sound effects that are playing. A
 * zero value temporarily suspends the playing of sound effects. A
 * positive value continues the sound effects at the point at which
 * they were suspended.
 */
extern void setsoundeffects(int action);

/* Set the current volume level. Volume ranges from 0 (silence) to 10
 * (the default). Setting the sound to zero causes sound effects to be
 * displayed as textual onomatopoeia. If display is TRUE, the new
 * volume level will be displayed to the user. FALSE is returned if
 * the sound system is not currently active.
 */
extern int setvolume(int volume, int display);

/* Alters the current volume level by delta.
 */
extern int changevolume(int delta, int display);

/* Release all memory used for the given sound effect's wave data.
 */
extern void freesfx(int index);

//DKS - added for SDL_mixer music support:
extern void freemusic(void);

/*
 * Miscellaneous functions.
 */

/* Ring the bell.
 */
extern void ding(void);

/* Set the program's subtitle. A NULL subtitle is equivalent to the
 * empty string. The subtitle is displayed in the window dressing (if
 * any).
 */
extern void setsubtitle(char const *subtitle);

/* Display a message to the user. cfile and lineno can be NULL and 0
 * respectively; otherwise, they identify the source code location
 * where this function was called from. prefix is an optional string
 * that is displayed before and/or apart from the body of the message.
 * fmt and args define the formatted text of the message body. action
 * indicates how the message should be presented. NOTIFY_LOG causes
 * the message to be displayed in a way that does not interfere with
 * the program's other activities. NOTIFY_ERR presents the message as
 * an error condition. NOTIFY_DIE should indicate to the user that the
 * program is about to shut down.
 */
extern void usermessage(int action, char const *prefix,
		char const *cfile, unsigned long lineno,
		char const *fmt, va_list args);

/* Values used for the first argument of usermessage().
 */
enum { NOTIFY_DIE, NOTIFY_ERR, NOTIFY_LOG };

/* Structure used to define text with illustrations.
 */
typedef	struct tiletablerow {
	int		isfloor;	/* TRUE if the images are floor tiles */
	int		item1;		/* first illustration */
	int		item2;		/* second illustration */
	char const *desc;		/* text */
} tiletablerow;

/* Displays a screenful of (hopefully) helpful information which
 * includes tile images. title provides the title of the display. rows
 * points to an array of tiletablerow structures. count specifies the
 * size of this array. The text of each row is displayed alongside one
 * or two tile images. completed controls the prompt that the user
 * sees at the bottom of the display. A positive value will indicate
 * that more text follows. A negative value will indicate that leaving
 * this screen will return to the prior display. A value of zero will
 * indicate that the current display is the end of a sequence.
 */
extern int displaytiletable(char const *title, tiletablerow const *rows,
		int count, int completed);

/* Displays a screenful of (hopefully) helpful information. title
 * provides the title of the display. table points to a table that
 * contains the body of the text. completed controls the prompt that
 * the user sees at the bottom of the display; see the description of
 * displaytiletable() for details.
 */
extern int displaytable(char const *title, tablespec const *table,
		int completed);

#endif
