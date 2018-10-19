;* ======================================================================== *;
;*  ANSI.SYS-style Escape-Sequence Interpreter                  J. Zbiciak  *;
;* ======================================================================== *;
;*  These routines interpret ANSI escape sequences, drawing text to the     *;
;*  Intellivision display.  This code is adapted from some old C code I     *;
;*  had written years ago.                                                  *;
;* ======================================================================== *;
;*  Exported functions defined in this module:                              *;
;*                                                                          *;
;*  A_INIT           Initializes the ANSI interpreter                       *;
;*                                                                          *;
;*  Static functions defined in this module:                                *;
;*                                                                          *;
;*  A_DISPLAY_CHAR   Send a character to the display buffer.                *;
;*  A_PROCESS_ESC    Process a completed escape sequence.                   *;
;*  A_PROCESS_CHAR   Process a single character.                            *;
;*  FB_SCROLL_UP     Scrolls some rows of the frame buffer upwards.         *;
;*  FB_SCROLL_DN     Scrolls some rows of the frame buffer downwards.       *;
;*  FB_CLEAR         Clears some range of the frame buffer to spaces,       *;
;*                   using the currently set frame-buffer attributes.       *;
;*                                                                          *;
;*  Static "state" variables maintained by this module:                     *;
;*                                                                          *;
;*  A_fb             Word-aligned frame buffer pointer                      *;
;*  A_r,  A_c        Cursor row, column                                     *;
;*  A_sr, A_sc       Saved row, column (for ESC [ s / ESC [ u)              *;
;*  A_fb_attr        Frame-buffer attributes (for drawing chars to fb.)     *;
;*  A_wrap           Word wrap mode (1=wrap, 0=no-wrap)                     *;
;*  A_esc_ptr        Pointer into escape buffer                             *;
;*  A_esc_buf[16]    Escape sequence collection buffer                      *;
;*  A_args[2]        Decoded argument buffer (overlaps esc_buf partially)   *;
;*  A_interp         Current interpreting function (used by A_process_buff) *;
;* ======================================================================== *;

;; ======================================================================== ;;
;;  ANSI INTERPRETER ENGINE DETAILS     --     SUPPORTED ESCAPE SEQUENCES   ;;
;; ======================================================================== ;;
;;                                                                          ;;
;;  Supported escape sequences are of the following form:                   ;;
;;                                                                          ;;
;;     ESC  [  ( 'arg' (; 'arg' (...) ) ) 'action'                          ;;
;;                                                                          ;;
;;  where 'arg' is a number and 'action' is a single alphabetic character.  ;;
;;  This interpreter supports argument lists up to 10 chars/10 args long.   ;;
;;  Most commands take 0 - 2 arguments (with default argument values if     ;;
;;  none are specified).                                                    ;;
;;                                                                          ;;
;;  The following 'actions' are supported by this code:                     ;;
;;                                                                          ;;
;;   Character       Action                             Args       Defaults ;;
;;       @           Insert column at cursor            # of cols  1        ;;
;;       A           Move up                            # of rows  1        ;;
;;       B           Move down                          # of rows  1        ;;
;;       C           Move right                         # of cols  1        ;;
;;       D           Move left                          # of cols  1        ;;
;;       H           'Home cursor'/cursor reposition    row, col   1,1      ;;
;;       J           Clear portion of the display       clr mode   1        ;;
;;       K           Clear portion of the line          clr mode   none     ;;
;;       L           Insert row at current cursor pos   # of rows  none     ;;
;;       M           Delete row at current cursor pos   # of rows  none     ;;
;;       P           Delete column at cursor            # of cols  1        ;;
;;       h           Set display mode*                  disp mode  none     ;;
;;       l           Reset display mode*                disp mode  none     ;;
;;       m           Set Graphics Rendition             SGR vals   none     ;;
;;       s           Saves current cursor position      none       none     ;;
;;       u           Restores saved cursor position     none       none     ;;
;;                                                                          ;;
;;   SGR Value       Attribute                                              ;;
;;       0           Reset defaults for display                             ;;
;;       1           Bold on                                                ;;
;;       4           Underline on  (not supported)                          ;;
;;       5           Blink on      (not supported)                          ;;
;;       7           Reverse video on                                       ;;
;;       8           Invisible on                                           ;;
;;      21           Bold off                                               ;;
;;      24           Underline off (not supported)                          ;;
;;      25           Blink off     (not supported)                          ;;
;;      27           Reverse video off                                      ;;
;;      28           Invisible off                                          ;;
;;   30 ... 37       Set foreground color to 0..7                           ;;
;;   40 ... 47       Set background color to 0..7                           ;;
;;                                                                          ;;
;;   Color Values    Color         Intellivision Color                      ;;
;;       0           Black               BLK 0                              ;;
;;       1           Red                 RED 2                              ;;
;;       2           Green               DGR 4                              ;;
;;       3           Yellow              YEL 6                              ;;
;;       4           Blue                BLU 1                              ;;
;;       5           Magenta             TAN 3                              ;;
;;       6           Cyan                GRN 5                              ;;
;;       7           White               WHT 7                              ;;
;;                                                                          ;;
;;   Display Mode    "Set" action                "Reset" action             ;;
;;       0           40x25 BW  (ignored)         40x25 BW  (ignored)        ;;
;;       1           40x25 clr (ignored)         40x25 clr (ignored)        ;;
;;       2           80x25 BW  (ignored)         80x25 BW  (ignored)        ;;
;;       3           80x25 clr (ignored)         80x25 clr (ignored)        ;;
;;       4           320x200 gfx (ignored)       320x200 gfx (ignored)      ;;
;;       5           320x200 gfx (ignored)       320x200 gfx (ignored)      ;;
;;       6           640x200 gfx (ignored)       640x200 gfx (ignored)      ;;
;;       7           Line wrap on                Line wrap off              ;;
;;   7 (last arg)    Required for escape sequence to be recognized.         ;;
;;                                                                          ;;
;;    * Note:  For Set and Reset display mode, I only recognize the         ;;
;;      following two sequences:  "ESC [ 7 ; 7 h"  and  "ESC [ 7 ; 7 l"     ;;
;;      which set and clear line-wrap mode.                                 ;;
;;                                                                          ;;
;;   Also, you can print most non-printing characters by preceding them     ;;
;;   simply with an ESC.  An ESC followed by a character OTHER than [       ;;
;;   results in the character being printed directly to the frame buffer    ;;
;;   with no further interpretation.  This breaks with standard behavior    ;;
;;   for an ANSI interpreter, but it was a reasonable "out".                ;;
;; ======================================================================== ;;

A_fb            EQU         $200
                BYTEVAR     A_r             ; Cursor row
                BYTEVAR     A_c             ; Cursor column
                BYTEVAR     A_sr            ; Saved row
                BYTEVAR     A_sc            ; Saved column
                BYTEVAR     A_wrap          ; Word-wrap mode
                WORDVAR     A_esc_ptr       ; Offset into Escape buffer
                BYTEARRAY   A_args,    18   ; Decoded arguments + escape buffer
A_esc_buf       EQU         A_args + 2      ; Escape buffer
A_esc_buf_end   EQU         A_esc_buf + 16
                BYTEVAR     A_cblink        ; Cursor blink timer
                WORDVAR     A_fb_attr       ; Current text format word
                WORDVAR     A_fill_attr     ; Current fill format word
                WORDVAR     A_interp        ; Next interpreter state

                BYTEVAR     A_gr_flags      ; Bold/Blink/etc flags
                BYTEVAR     A_fg_color
                BYTEVAR     A_bg_color

A_GR_BOLD       EQU         1 SHL (1 - 1)   ; ESC ] 1 m  Bold on         
A_GR_ULINE      EQU         1 SHL (4 - 1)   ; ESC ] 4 m  Underline on
A_GR_BLINK      EQU         1 SHL (5 - 1)   ; ESC ] 5 m  Blink on
A_GR_RVS        EQU         1 SHL (7 - 1)   ; ESC ] 7 m  Reverse video on
A_GR_INVIS      EQU         1 SHL (8 - 1)   ; ESC ] 8 m  Invisible on    



;* ======================================================================== *;
;*  A_INIT                                                                  *;
;* ======================================================================== *;
A_INIT          PROC
                CLRR        R0
                MVO         R0,     A_r
                MVO         R0,     A_c
                MVO         R0,     A_sr
                MVO         R0,     A_sc
                NOP
                MVO         R0,     A_esc_ptr
               
                INCR        R0
                MVO         R0,     A_wrap

                MVII        #7,     R0
                MVO         R0,     A_fb_attr
                MVO         R0,     A_fill_attr

                MVII        #A_DISPLAY_CHAR, R0
                MVO         R0,     A_interp

                JR          R5
                ENDP

;* ======================================================================== *;
;*  FB_SCROLL_UP        Scrolls some rows of the frame buffer upwards.      *;
;*                                                                          *;
;*  R0      *dst        -- Upper row of destination                         *;
;*  R4      *src        -- Upper row of source                              *;
;*  R1      rows        -- Number of rows being scrolled.                   *;
;* ======================================================================== *;
FB_SCROLL_UP    PROC
                PSHR    R5

                MOVR    R0,     R5
@@loop:         
                RPT     20
                MVI@    R4,     R0
                MVO@    R0,     R5
                ENDR

                DECR    R1
                BNEQ    @@loop

                PULR    PC
                ENDP

;* ======================================================================== *;
;*  FB_SCROLL_DN        Scrolls some rows of the frame buffer downwards     *;
;*                                                                          *;
;*  R0      *dst        -- Upper row of destination                         *;
;*  R4      *src        -- Upper row of source                              *;
;*  R1      rows        -- Number of rows being scrolled.                   *;
;* ======================================================================== *;
FB_SCROLL_DN    PROC
                PSHR    R5
                PSHR    R2
                PSHR    R1

                DECR    R1
                SLL     R1,     2
                PSHR    R1
                SLL     R1,     2
                ADD@    SP,     R1

                ADDR    R1,     R0
                ADDR    R1,     R4
                MOVR    R0,     R5

                PULR    R1

@@o_loop:       MVII    #4,     R2
@@i_loop:
                RPT     5
                MVI@    R4,     R0
                MVO@    R0,     R5
                ENDR    
                DECR    R2
                BNEQ    @@i_loop

                SUBI    #40,    R4
                SUBI    #40,    R5

                DECR    R1
                BNEQ    @@o_loop

                PULR    R2
                PULR    PC
                ENDP


;* ======================================================================== *; 
;*  FB_CLEAR            Clears some range of the frame buffer to spaces,    *; 
;*                      using the currently set frame-buffer attributes.    *; 
;*                                                                          *;
;*  R4      *fb         -- Upper left corner of linear region to clear      *; 
;*  R1       count      -- Number of characters to clear in reading-order   *; 
;*                         (left-to-right, top-to-bottom).                  *; 
;* ======================================================================== *; 
FB_CLEAR        PROC
                MVI     A_fill_attr,  R0
                B       FILLMEM
                ENDP

;* ======================================================================== *;
;*  A_DISPLAY_CHAR      Send a character to the display buffer.             *;
;*                                                                          *;
;*  R0       x          -- Character to display                             *;
;*                                                                          *;
;*  Returns:                                                                *;
;*  A_interp            -- Next state machine state processing function     *;
;*                                                                          *;
;*  This routine interprets a single character, and handles all printing    *;
;*  characters, and all control characters (not to be confused with escape  *;
;*  sequences, which are handled elsewhere).  If this routine receives an   *;
;*  escape character, it sets the ANSI-interpreting state machine into      *;
;*  escape processing mode.                                                 *;
;* ======================================================================== *;
A_DISPLAY_CHAR  PROC

                ;* -------------------------------------------------------- *;
                ;*  Peel off ESC early since it doesn't move the cursor.    *;
                ;*  27 (ESC)    -- Set state function to A_capture_esc1     *;
                ;* -------------------------------------------------------- *;
                CMPI    #27,    R0
                BNEQ    @@not_esc
                MVII    #A_CAPTURE_ESC1,    R0
;               MVII    #A_DISPLAY_CHAR,    R0
                MVO     R0,     A_interp
                JR      R5
@@not_esc:
                CMPI    #$9B,   R0              ; "ESC + 128" same as "ESC ]"
                BNEQ    @@not_csi
                MVII    #A_CAPTURE_ESC2,    R0
                MVO     R0,     A_interp
                JR      R5
@@not_csi:

                PSHR    R5
@@1:            PSHR    R1      
                PSHR    R2      
                
                MVI     A_r,        R1      ; Row
                MVI     A_c,        R2      ; Col

                ;* -------------------------------------------------------- *;
                ;*  CTRL or printing char?                                  *;
                ;* -------------------------------------------------------- *;
                SUBI    #32,        R0
                BC      @@printing_char
                ADDI    #32,        R0      ; optimize this out later.

                ;* -------------------------------------------------------- *;
                ;*  Interpret remaining control characters                  *;
                ;*                                                          *;
                ;*   8 (BS)     -- Bkspc if cursor insn't in col 0 already  *;
                ;*   9 (TAB)    -- Move cursor to next tab-stop             *;
                ;*  10 (LF)     -- Perform linefeed  (row++)                *;
                ;*  13 (CR)     -- Perform carriage return  (col=0)         *;
                ;* -------------------------------------------------------- *;

                CMPI    #8,         R0      ; Is this BS?
                BNEQ    @@not_bs    
                TSTR    R2                  ; \
                BEQ     @@done              ;  |_ Move back one column if not 
                DECR    R2                  ;  |  in column 0, and then leave.
                B       @@save_c            ; /
@@not_bs:                          
                CMPI    #9,         R0      ; Is this TAB?
                BNEQ    @@not_tab
                                   
                ADDI    #8,         R2      ; \
                PSHR    R2                  ;  |
                ANDI    #7,         R2      ;  |- Compute next tab stop
                NEGR    R2                  ;  |
                ADD@    SP,         R2      ; /
                B       @@c_check
@@not_tab:                         
                CMPI    #10,        R0      ; Is this LF?
                BNEQ    @@not_lf
                INCR    R1                  ; Move down one row
                B       @@r_check
@@not_lf:                          
                CMPI    #13,        R0      ; Is this CR?
                BNEQ    @@done              ; Not valid ctrl char?  Drop it.
                CLRR    R2                  ; 
                B       @@save_c
                
                ;* -------------------------------------------------------- *;
                ;*  Printing characters.                                    *;
                ;* -------------------------------------------------------- *;
@@printing_char:
                CMPI    #12,        R1      ; \
                BC      @@off_screen        ;  |_ Don't display if cursor 
                CMPI    #20,        R2      ;  |  was moved off screen
                BC      @@bad_column        ; /

                ; specific to fgbg mode:  Lowercase is in GRAM because FGBG
                ; limits you to 64 cards in each.
                CMPI    #64,        R0
                BNC     @@card_ok
                ADDI    #256 - 64,  R0
@@card_ok

                SLL     R0,         2
                SLL     R0,         1
                XOR     A_fb_attr,  R0

                MOVR    R1,         R5      ; Save row in R5
                SLL     R1,         2       ; R1 = 4*row
                ADDR    R5,         R1      ; R1 = 5*row
                SLL     R1,         2       ; R1 = 20*row
                ADDR    R2,         R1      ; R1 = 20*row + col
                ADDI    #A_fb,      R1      ; R1 = 20*rol + col + A_fb

                MVO@    R0,         R1

                MOVR    R5,         R1      ; Restore row in R1
                INCR    R2                  ; Move to next column
@@off_screen: 
                ;* -------------------------------------------------------- *;
                ;*  Check screen boundaries.                                *;
                ;* -------------------------------------------------------- *;
@@c_check:
                CMPI    #20,        R2      ; Cursor off the right?
                BNC     @@r_check           ; No:  Check the row
@@bad_column:   MVI     A_wrap,     R0      ; \
                TSTR    R0                  ;  |- Is wrap mode set?
                BNEQ    @@col_wrap          ; /
                MVII    #19,        R2      ; No:  Park in right column
                B       @@save_c            ;
@@col_wrap      CLRR    R2                  ; \_ Go to start of next line.
                INCR    R1                  ; /

@@r_check:      CMPI    #12,        R1
                BNC     @@save_rc

                MVII    #A_fb,      R0      ; \
                MVII    #A_fb + 20, R4      ;  |_ Scroll the display up
                MVII    #11,        R1      ;  |  one full row.
                CALL    FB_SCROLL_UP        ; /

                MVII    #A_fb +220, R4      ; \
                MVII    #20,        R1      ;  |- Clear exposed row.
                CALL    FB_CLEAR            ; /

                MVII    #11,        R1
                CLRR    R2

@@save_rc:      MVO     R1,         A_r     ; \_  Save updated row, col
@@save_c:       MVO     R2,         A_c     ; /

@@done:
                PULR    R2
                PULR    R1
                PULR    PC

                ENDP


        
;* ======================================================================== *;
;*  Escape-sequence collection state machine.                               *;
;*                                                                          *;
;*  State 0     -- No escape sequence yet observed.         A_display_char  *;
;*                      -- On ESC, go to state 1.                           *;
;*                      -- On character, display the character              *;
;*                                                                          *;
;*  State 1     -- ESC char seen.                           A_capture_esc1  *;
;*                      -- On '[', go to state 2.                           *;
;*                      -- On other character, return to state 0.           *;
;*                                                                          *;
;*  State 2..n  -- 'ESC [' seen.                            A_capture_esc2  *;
;*                      -- On '0-9' and ';' accumulate characters,          *;
;*                         while incrementing state number.  If             *;
;*                         state number overflows (goes to n+1),            *;
;*                         reset to state 0.                                *;
;*                      -- On 'A-Z', 'a-z' or '@', process escape           *;
;*                         sequence and then return to state 0.             *;
;*                      -- On other character, return to state 0.           *;
;* ======================================================================== *;

;* ------------------------------------------------------------------------ *;
;*  State 1     -- ESC char seen.                                           *;
;* ------------------------------------------------------------------------ *;
A_CAPTURE_ESC1  PROC
                ;* -------------------------------------------------------- *;
                ;*  Handle "ESC c" as a display reset.                      *;
                ;* -------------------------------------------------------- *;
                CMPI    #ASC("c",0),    R0
                BNEQ    @@not_esc_c

                PSHR    R5
                CALL    CLRSCR
                PULR    R5
                B       A_INIT
                
@@not_esc_c:
                ;* -------------------------------------------------------- *;
                ;*  Now look for a bracket.  If the next character isn't    *;
                ;*  a bracket or 'c', display the character normally.       *;
                ;*                                                          *;
                ;*  Note:  Ordinarily, control characters aren't supposed   *;
                ;*  to be interpreted, but what does that really mean?      *;
                ;* -------------------------------------------------------- *;
                CMPI    #ASC("[",0),    R0
                BNEQ    A_DISPLAY_CHAR

                ;* -------------------------------------------------------- *;
                ;*  If we got here, move to CAPTURE_ESC2.                   *;
                ;* -------------------------------------------------------- *;
                MVII    #A_esc_buf,     R0
                MVO     R0,             A_esc_ptr
                MVII    #A_CAPTURE_ESC2,R0
                MVO     R0,             A_interp

                JR      R5
                ENDP
        

;* ------------------------------------------------------------------------ *;
;*  State 2     -- 'ESC [' seen.                                            *;
;* ------------------------------------------------------------------------ *;
A_CAPTURE_ESC2  PROC

                
                ;* -------------------------------------------------------- *;
                ;*      -- On '0-9' and ';' accumulate characters           *;
                ;* -------------------------------------------------------- *;
                CMPI    #ASC(';', 0), R0
                BEQ     @@accumulate
                CMPI    #ASC('0', 0), R0
                BLT     @@no_accumulate
                CMPI    #ASC('9', 0), R0
                BGT     @@no_accumulate
@@accumulate:
                MVI     A_esc_ptr,  R4
                MVO@    R0,         R4
                MVO     R4,         A_esc_ptr

                ;* -------------------------------------------------------- *;
                ;*  See if the buffer is full.  If so, consider sequence    *;
                ;*  invalid and just dump it to the display.                *;
                ;* -------------------------------------------------------- *;
                CMPI    #A_esc_buf_end, R4
                BEQ     @@invalid

                JR      R5

@@no_accumulate:

                ;* -------------------------------------------------------- *;
                ;*  No matter what, if we get here, our next state is       *;
                ;*  A_DISPLAY_CHAR, regardless of whether this was a valid  *;
                ;*  escape sequence.                                        *;
                ;* -------------------------------------------------------- *;
                MVII    #A_DISPLAY_CHAR,R4
                MVO     R4,             A_interp

                ;* -------------------------------------------------------- *;
                ;*      -- On 'A-Z', 'a-z' or '@', process escape           *;
                ;*         sequence and then return to state 0.             *;
                ;* -------------------------------------------------------- *;
                CMPI    #ASC('@', 0),   R0
                BLT     @@invalid_esc
                CMPI    #ASC('Z', 0),   R0
                BLE     A_PROCESS_ESC
                CMPI    #ASC('a', 0),   R0
                BLT     @@invalid_esc
                CMPI    #ASC('z', 0),   R0
                BLE     A_PROCESS_ESC
@@invalid_esc:
                MVI     A_esc_ptr,  R4
                MVO@    R0,         R4
                MVO     R4,         A_esc_ptr

                ;* -------------------------------------------------------- *;
                ;*  If we have an invalid escape sequence, just write it    *;
                ;*  to the display.                                         *;
                ;* -------------------------------------------------------- *;
@@invalid:      PSHR    R5
                PSHR    R1

                MVII    #ASC("[",0),    R0
                CALL    A_DISPLAY_CHAR

                MVII    #A_esc_buf,     R1
@@inv_loop:     MVI@    R1,             R0
                CALL    A_DISPLAY_CHAR
                INCR    R1
                CMP     A_esc_ptr,      R1
                BNEQ    @@inv_loop

                PULR    R1
                PULR    PC

                ENDP

                

;* ======================================================================== *;
;*  A_PROCESS_ESC       Process a completed escape sequence.                *;
;*                                                                          *;
;*  int esc             -- Command character at end of escape sequence      *;
;*                                                                          *;
;*  This routine receives a completed ANSI/VT-100 escape sequence, parses   *;
;*  its argument list (if any), and then executes the escape sequence.  If  *;
;*  the completed escape sequence is unrecognized, then it is dropped.      *;
;* ======================================================================== *;
A_PROCESS_ESC   PROC
                PSHR    R5
                PSHR    R3
                PSHR    R2
                PSHR    R1

                ;* -------------------------------------------------------- *;
                ;*  Parse argument list.  Some commands (such as 'H') have  *;
                ;*  up to two default arguments that default to '1'.  So,   *;
                ;*  we set that up front and then loop through all the      *;
                ;*  observed characters to find any actual arguments.       *;
                ;* -------------------------------------------------------- *;
                MVII    #1,         R1
                MVO     R1,         A_args + 0
                MVO     R1,         A_args + 1

                MVII    #$8000,     R2          ; accumulated value
                MVII    #A_esc_buf, R4
                MVII    #A_args,    R5
                CMP     A_esc_ptr,  R4
                BEQ     @@argdone
@@argloop:
                MVI@    R4,         R1
                CMPI    #ASC(";", 0),   R1
                BEQ     @@emit                  ; emit argument if semicolon

                MOVR    R2,         R3          ; \
                SLL     R2,         2           ;  |_ multiply by 10
                ADDR    R3,         R2          ;  |
                SLL     R2,         1           ; /

                SUBI    #ASC('0', 0),  R1       ; Subtract off ASCII bias
                ADDR    R1,         R2          ; Accumulate new digit

                CMP     A_esc_ptr,  R4
                BNEQ    @@argloop
                MVO@    R2,         R5          ; Write out last argument.
                B       @@argdone

@@emit:         TSTR    R2                      ; Initialized argument?
                BPL     @@argok
                MVII    #1,         R2
@@argok:        MVO@    R2,         R5          ; Write out argument.

                MVII    #$8000,     R2          ; Restart accumulator

                CMP     A_esc_ptr,  R4
                BNEQ    @@argloop
@@argdone:
                MVO     R5,         A_esc_ptr   ; Now pointer to end of args

                ;* -------------------------------------------------------- *;
                ;*  Now dispatch to the handler for the specific escape.    *;
                ;* -------------------------------------------------------- *;
                MVII    #@@dispatch - ASC('@', 0), R1
                ADDR    R0,         R1          ; Dispatch based on saved esc.
                MVI@    R1,         PC
@@dispatch:
                DECLE   @@at, @@A,  @@B,  @@C,  @@D,  @@E,  @@F,  @@G
                DECLE   @@H,  @@I,  @@J,  @@K,  @@L,  @@M,  @@N,  @@O
                DECLE   @@P,  @@Q,  @@R,  @@S,  @@T,  @@U,  @@V,  @@W
                DECLE   @@X,  @@Y,  @@Z,  @@_,  @@_,  @@_,  @@_,  @@_
                DECLE   @@_,  @@a,  @@b,  @@c,  @@d,  @@e,  @@f,  @@g
                DECLE   @@h,  @@i,  @@j,  @@k,  @@l,  @@m,  @@n,  @@o
                DECLE   @@p,  @@q,  @@r,  @@s,  @@t,  @@u,  @@v,  @@w
                DECLE   @@x,  @@y,  @@z,  @@_,  @@_,  @@_,  @@_,  @@_

                ;* -------------------------------------------------------- *;
                ;*     ESC  [  (arg) A      - Move up 'arg' spaces          *;
                ;*     ESC  [  (arg) B      - Move down 'arg' spaces        *;
                ;*     ESC  [  (arg) C      - Move right 'arg' spaces       *;
                ;*     ESC  [  (arg) D      - Move left 'arg' spaces        *;
                ;*                                                          *;
                ;*  Default is to move 1 space if arg is absent.            *;
                ;* -------------------------------------------------------- *;
@@A:            MVI     A_r,    R2
                SUB     A_args, R2
                B       @@row_update
@@B:            MVI     A_r,    R2
                ADD     A_args, R2
                B       @@row_update
@@C:            MVI     A_c,    R3
                ADD     A_args, R3
                B       @@col_update
@@D:            MVI     A_c,    R3
                SUB     A_args, R3
                B       @@col_update

                ;* -------------------------------------------------------- *;
                ;*     ESC  [ H             - Home cursor (1,1)             *;
                ;*     ESC  [ arg H         - Home cursor to ('arg',1)      *;
                ;*     ESC  [ ; arg H       - Home cursor to (1,'arg')      *;
                ;*     ESC  [ arg ; arg H   - Home cursor to ('arg','arg')  *;
                ;*                                                          *;
                ;*  Note:  'f' is apparently equivalent to 'H'.             *;
                ;* -------------------------------------------------------- *;
@@f:
@@H:            MVI     A_args + 0, R2
                MVI     A_args + 1, R3
                DECR    R2
                DECR    R3
                B       @@row_col_update

                ;* -------------------------------------------------------- *;
                ;*     ESC  [ s         - Save current cursor position      *;
                ;*     ESC  [ u         - Restore saved cursor position     *;
                ;* -------------------------------------------------------- *;
@@s:            MVI     A_r,        R0
                MVO     R0,         A_sr
                MVI     A_c,        R0
                MVO     R0,         A_sc
                B       @@done

@@u:            MVI     A_sr,       R2
                MVI     A_sc,       R3
                B       @@row_col_update

                ;* -------------------------------------------------------- *;
                ;*     ESC  [ 7;7h          - Turn on word wrap             *;
                ;*     ESC  [ 7;7l          - Turn off word wrap            *;
                ;* -------------------------------------------------------- *;
@@h:            MVII    #1,         R0
                INCR    PC
@@l:            CLRR    R0
                MVII    #7,         R1
                CMP     A_args + 0, R1
                BNEQ    @@done
                CMP     A_args + 1, R1
                BNEQ    @@done
                MVO     R0,         A_wrap
                B       @@done

                ;* -------------------------------------------------------- *;
                ;*     ESC  [ J             - Clear to end of screen        *;
                ;*     ESC  [ 0 J           - Clear to end of screen        *;
                ;*     ESC  [ 1 J           - Clear top of screen to cursor *;
                ;*     ESC  [ 2 J           - Clear entire screen           *;
                ;* -------------------------------------------------------- *;
@@J:            MVI     A_r,        R1      ; \
                SLL     R1,         2       ;  |  Compute offset of 
                ADD     A_r,        R1      ;  |- the cursor
                SLL     R1,         2       ;  |
                ADD     A_c,        R1      ; / 

                CMPI    #A_args,    R5
                BEQ     @@J_to_eos

                MVI     A_args,     R5
                DECR    R5
                BMI     @@J_to_eos
                BNEQ    @@J_clear_all

@@J_to_bos:     MVII    #A_fb,      R4
                INCR    R1
                B       @@JK_do_clear
                
@@J_to_eos:     MVII    #A_fb,      R4
                ADDR    R1,         R4
                SUBI    #20*12,     R1
                NEGR    R1
                B       @@JK_do_clear

@@J_clear_all:  MVII    #A_fb,      R4
                MVII    #20*12,     R1

@@JK_do_clear   PULR    R2
                PULR    R2
                PULR    R3
                PULR    R5
                B       FB_CLEAR


                ;* -------------------------------------------------------- *;
                ;*     ESC  [ K             - Clear to end of line          *;
                ;*     ESC  [ 0 K           - Clear to end of line          *;
                ;*     ESC  [ 1 K           - Clear to beginning of line    *;
                ;*     ESC  [ 2 K           - Clear entire line             *;
                ;* -------------------------------------------------------- *;
@@K:            MVI     A_r,        R2      ; \
                SLL     R2,         2       ;  |  Compute the row containing
                ADD     A_r,        R2      ;  |- the cursor
                SLL     R2,         2       ;  |
                ADDI    #A_fb,      R2      ; /
                MOVR    R2,         R4

                CMPI    #A_args,    R5
                BEQ     @@K_to_eol

                MVI     A_args,     R5
                DECR    R5
                BMI     @@K_to_eol
                BNEQ    @@K_clear_line

@@K_to_bol:     MVI     A_c,        R1
                INCR    R1
                B       @@JK_do_clear
                
@@K_to_eol:     ADD     A_c,        R4
                MVII    #20,        R1
                SUB     A_c,        R1
                B       @@JK_do_clear

@@K_clear_line: MVII    #20,        R1
                B       @@JK_do_clear

                
                ;* -------------------------------------------------------- *;
                ;*     ESC  [ (arg) @       - Insert 'arg' chars at current *;
                ;*                            cursor position within row    *;
                ;*     ESC  [ (arg) P       - Delete 'arg' chars at current *;
                ;*                            cursor position within row    *;
                ;* -------------------------------------------------------- *;
@@at:
                MVI     A_r,        R2      ; \
                SLL     R2,         2       ;  |  Compute address of row the
                ADD     A_r,        R2      ;  |- cursor is in.  
                SLL     R2,         2       ;  |
                ADDI    #A_fb,      R2      ; /

                MOVR    R2,         R1      ; \_ exact position of the cursor
                ADD     A_c,        R1      ; /

                ADDI    #19,        R2      ; End of the line

                MOVR    R2,         R3      ; (arg) chars before end of line
                SUB     A_args,     R3      ;
@@at_move:
                MVI@    R3,         R0      ; \_ slide over a char
                MVO@    R0,         R2      ; /
                DECR    R3
                DECR    R2
                CMPR    R3,         R1      ; Did we move char at cursor?
                BC      @@at_move           ; Loop until we do
                
                MVI     A_fb_attr,  R0
                MOVR    R3,         R4
@@at_fill:      MVO@    R0,         R4      ; \_ fill void
                CMPR    R4,         R2      ; /
                BNEQ    @@at_fill

                B       @@done

@@P:
                MVI     A_r,        R2      ; \
                SLL     R2,         2       ;  |  Compute address of row the
                ADD     A_r,        R2      ;  |- cursor is in.  
                SLL     R2,         2       ;  |
                ADDI    #A_fb,      R2      ; /

                MOVR    R2,         R4      ; \_ exact position of the cursor
                ADD     A_c,        R4      ; /

                ADDI    #20,        R2      ; After end of the line

                MOVR    R5,         R5      ; (arg) chars after cursor
                ADD     A_args,     R5      ;

@@P_move:       MVI@    R5,         R0      ; \
                MVO@    R0,         R4      ;  |_ slide characters left
                CMPR    R5,         R2      ;  |
                BNEQ    @@P_move            ; /

                MVI     A_fb_attr,  R0
@@P_fill:       MVO@    R0,         R4      ; \_ fill rest of the line
                CMPR    R4,         R2      ; /
                BNEQ    @@P_fill

                B       @@done

                ;* -------------------------------------------------------- *;
                ;*     ESC  [ (arg) L       - Insert 'arg' blank rows at    *;
                ;*                            cursor position (default 1)   *;
                ;*     ESC  [ (arg) M       - Delete 'arg' blank rows at    *;
                ;*                            cursor position (default 1)   *;
                ;* -------------------------------------------------------- *;
@@L:
                MVI     A_r,        R1      ; \
                SLL     R1,         2       ;  |
                MOVR    R1,         R4      ;  |_ Compute source row pointer
                SLL     R1,         2       ;  |
                ADDR    R1,         R4      ;  |
                ADDI    #A_fb,      R4      ; /

                MVII    #A_fb + 20*12, R2   ; Default dest is "offscreen"

                MVI     A_r,        R0
                ADD     A_args + 0, R0
                CMPI    #12,        R0
                BC      @@L_clear

                MOVR    R0,         R1      ; \
                SLL     R0,         2       ;  |
                ADDR    R1,         R0      ;  |- Compute destination row 
                SLL     R0,         2       ;  |  pointer
                ADDI    #A_fb,      R0      ; /

                MOVR    R0,         R2      ; save destination row


                MVI     A_args,     R1

                PSHR    R4                  ; save source ptr
                CALL    FB_SCROLL_DN
                PULR    R4                  ; restore source ptr

@@L_clear:      MOVR    R2,         R1
                SUBR    R4,         R1
                BEQ     @@done

                CALL    FB_CLEAR

                B       @@done
                

@@M
                MVI     A_r,        R1      ; \
                SLL     R1,         2       ;  |
                MOVR    R1,         R0      ;  |_ Compute dest. row pointer
                SLL     R1,         2       ;  |
                ADDR    R1,         R0      ;  |
                ADDI    #A_fb,      R0      ; /


                MVI     A_r,        R1
                ADD     A_args + 0, R1
                CMPI    #12,        R1
                BNC     @@M_ok

                MOVR    R0,         R4
                B       @@M_clear

@@M_ok:         SLL     R1,         2       ; \
                MOVR    R1,         R4      ;  |
                SLL     R1,         2       ;  |- Compute source row pointer
                ADDR    R1,         R4      ;  |
                ADDI    #A_fb,      R4      ; /

                CALL    FB_SCROLL_UP        

                ; FB_SCROLL_UP leaves R5 pointing after last char written
                MOVR    R5,         R4
@@M_clear:
                MVII    #A_fb + 20*12, R1   ; Default source is "offscreen"
                SUBR    R4,         R1

                CALL    FB_CLEAR
                B       @@done


                ;* -------------------------------------------------------- *;
                ;*     ESC  [ m             - (same as 'ESC ] 0 m')         *;
                ;*     ESC  [ (arglist) m   - Set Graphics Rendition.       *;
                ;*                                                          *;
                ;*    SGR Value        Attribute                            *;
                ;*        0            Reset defaults for display           *;
                ;*        1            Bold on                              *;
                ;*        5            Blink on                             *;
                ;*        7            Reverse video                        *;
                ;*        8            Invisible     (sets fg == bg)        *;
                ;*       21            Bold off                             *;
                ;*       25            Blink off                            *;
                ;*       27            Reverse video off                    *;
                ;*       28            Invisible     (sets fg == bg)        *;
                ;*    30 ... 37        Set foreground color to 0..7         *;
                ;*    40 ... 47        Set background color to 0..7         *;
                ;*                                                          *;
                ;*    Color Values     Color        PC Color number         *;
                ;*        0            Black            0 / 8               *;
                ;*        1            Red              4 / 12              *;
                ;*        2            Green            2 / 10              *;
                ;*        3            Yellow           6 / 14              *;
                ;*        4            Blue             1 / 9               *;
                ;*        5            Magenta          5 / 13              *;
                ;*        6            Cyan             3 / 11              *;
                ;*        7            White            7 / 15              *;
                ;*                                                          *;
                ;* -------------------------------------------------------- *;
@@m:            CMPI    #A_args,    R5
                BNEQ    @@m_has_args
                CLRR    R0
                MVO@    R0,         R5
                MVO     R5,         A_esc_ptr
@@m_has_args:
                MVII    #A_args,    R4
@@m_loop:
                MVI@    R4,         R1
                CMPI    #47,        R1
                BGT     @@m_next
                MVII    #@@m_tbl,   R2
                ADDR    R1,         R2
                MVI@    R2,         PC

@@m_tbl     DECLE   @@m00,@@m01,@@m__,@@m__,@@m__,@@m05,@@m__,@@m07,@@m08,@@m__
            DECLE   @@m__,@@m__,@@m__,@@m__,@@m__,@@m__,@@m__,@@m__,@@m__,@@m__
            DECLE   @@m__,@@m21,@@m__,@@m__,@@m__,@@m25,@@m__,@@m27,@@m28,@@m__
            DECLE   @@m3x,@@m3x,@@m3x,@@m3x,@@m3x,@@m3x,@@m3x,@@m3x,@@m__,@@m__
            DECLE   @@m4x,@@m4x,@@m4x,@@m4x,@@m4x,@@m4x,@@m4x,@@m4x

@@m00:          CLRR    R0
                MVO     R0,         A_bg_color
                MVO     R0,         A_gr_flags
                MVII    #7,         R0
                MVO     R0,         A_fg_color
                B       @@m_next

MACRO   A_Set_Flag(x)
                MVI     A_gr_flags,     R0
                ANDI    #NOT A_GR_%x%,  R0
                XORI    #A_GR_%x%,      R0
                MVO     R0,             A_gr_flags
ENDM

MACRO   A_Clr_Flag(x)
                MVI     A_gr_flags,     R0
                ANDI    #NOT A_GR_%x%,  R0
                MVO     R0,             A_gr_flags
ENDM

@@m01:          A_Set_Flag(BOLD)
                B       @@m_next

@@m05:          A_Set_Flag(BLINK)
                B       @@m_next

@@m07:          A_Set_Flag(RVS)
                B       @@m_next

@@m08:          A_Set_Flag(INVIS)
                B       @@m_next

@@m21:          A_Clr_Flag(BOLD)
                B       @@m_next

@@m25:          A_Clr_Flag(BLINK)
                B       @@m_next

@@m27:          A_Clr_Flag(RVS)
                B       @@m_next

@@m28:          A_Clr_Flag(INVIS)
                B       @@m_next
                
@@m3x:          SUBI    #30,        R1
                MVO     R1,         A_fg_color
                B       @@m_next
                
@@m4x:          SUBI    #40,        R1
                MVO     R1,         A_bg_color
;               B       @@m_next

@@m__
@@m_next:       CMP     A_esc_ptr,  R4
                BNEQ    @@m_loop

                ;; compute new A_fb_attr here....
                MVI     A_fg_color, R1  ; \_ Read in fg, bg color #x
                MVI     A_bg_color, R2  ; /  and massage based on flags

                CLRR    R0              ; R0 holds new fb_attr

                MVI     A_gr_flags, R3

                MVII    #A_GR_RVS,  R4  ; If RVS is set, swap fg, bg
                ANDR    R3,         R4
                BEQ     @@m_notrvs
                MVI     A_fg_color, R2
                MVI     A_bg_color, R1
@@m_notrvs:

                ANDI    #A_GR_INVIS,R3  ; Copy bg to fg if invisible
                BEQ     @@m_notinvis
                MOVR    R2,         R1
@@m_notinvis:

                ADDI    #@@fg_color,R1
                ADDI    #@@bg_color,R2
                XOR@    R1,         R0
                XOR@    R2,         R0
                
                MVO     R0, A_fb_attr

                MVI     A_bg_color, R1
                ADDI    #@@bg_color,R1
                MVI@    R1,         R1
                MVO     R1,         A_fill_attr 

                B       @@done

; color table for fgbg mode. Need to add CS support.
@@fg_color:     DECLE   STIC.fb_fg0     ; black
                DECLE   STIC.fb_fg2     ; red
                DECLE   STIC.fb_fg4     ; green     => dark green
                DECLE   STIC.fb_fg6     ; yellow
                DECLE   STIC.fb_fg1     ; blue
                DECLE   STIC.fb_fg3     ; magenta   => tan
                DECLE   STIC.fb_fg5     ; cyan      => green
                DECLE   STIC.fb_fg7     ; white

@@bg_color:     DECLE   STIC.fb_bg0     ; black
                DECLE   STIC.fb_bg2     ; red
                DECLE   STIC.fb_bg4     ; green     => dark green
                DECLE   STIC.fb_bg6     ; yellow
                DECLE   STIC.fb_bg1     ; blue
                DECLE   STIC.fb_bg3     ; magenta   => tan
                DECLE   STIC.fb_bg5     ; cyan      => green
                DECLE   STIC.fb_bg7     ; white


                ;* -------------------------------------------------------- *;
                ;*  Cursor movement validation for escapes that moved it.   *;
                ;* -------------------------------------------------------- *;
@@row_col_update:
                CMPI    #0,     R3
                BGE     @@col_pos1
                CLRR    R3
                B       @@col_ok1
@@col_pos1:     CMPI    #20,    R3
                BLT     @@col_ok1
                MVII    #19,    R3
@@col_ok1:      MVO     R3,     A_c

@@row_update:   CMPI    #0,     R2
                BGE     @@row_pos
                CLRR    R2
                B       @@row_ok
@@row_pos:      CMPI    #12,    R2
                BLT     @@row_ok
                MVII    #11,    R2
@@row_ok:       MVO     R2,     A_r
                B       @@done

@@col_update:   CMPI    #0,     R3
                BGE     @@col_pos
                CLRR    R3
                B       @@col_ok
@@col_pos:      CMPI    #20,    R3
                BLT     @@col_ok
                MVII    #19,    R3
@@col_ok:       MVO     R3,     A_c
                B       @@done
                ;* -------------------------------------------------------- *;
                ;*  Unsupported escapes                                     *;
                ;* -------------------------------------------------------- *;
@@E:
@@F:
@@G:
@@I:
@@N:
@@O:
@@Q:
@@R:
@@S:
@@T:
@@U:
@@V:
@@W:
@@X:
@@Y:
@@Z:
@@a:
@@b:
@@c:
@@d:
@@e:
@@g:
@@i:
@@j:
@@k:
@@n:
@@o:
@@p:
@@q:
@@r:
@@t:
@@v:
@@w:
@@x:
@@y:
@@z:
@@_:
@@done:         PULR    R1
                PULR    R2
                PULR    R3
                PULR    PC
                ENDP
