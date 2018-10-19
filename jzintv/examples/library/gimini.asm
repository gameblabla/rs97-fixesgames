;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;;==========================================================================;;
;; GIMINI structure definitions.  Defines the memory map and peripherals.   ;;
;;==========================================================================;;

;; ======================================================================== ;;
;;  Overall Memory Map                                                      ;;
;;  MEMMAP -- Structure containing overall system memory map.               ;;
;; ======================================================================== ;;
MEMMAP          STRUCT  0
@@stic          QEQU    $0000 ;..$003F  ; Standard Television Interface Circuit
@@voice         QEQU    $0080 ;..$0081  ; Voice Synthesizer
@@psg1          QEQU    $00F0 ;..$00FF  ; Secondary Programmable Sound Gen.
@@dataram       QEQU    $0100 ;..$01EF  ; 8-bit Data RAM
@@psg0          QEQU    $01F0 ;..$01FF  ; Primary Programmable Sound Generator
@@backtab       QEQU    $0200 ;..$02EF  ; 16-bit Display RAM
@@sysram        QEQU    $02F0 ;..$035F  ; 16-bit System RAM (incl. Stack)
@@exec2         QEQU    $0400 ;..$04FF  ; Executive ROM expansion
@@exec          QEQU    $1000 ;..$1FFF  ; Executive ROM
@@grom          QEQU    $3000 ;..$37FF  ; Graphics ROM
@@gram          QEQU    $3800 ;..$39FF  ; Graphics RAM
@@gram_alias1   QEQU    $3A00 ;..$3BFF  ; Graphics RAM alias 
@@gram_alias2   QEQU    $3C00 ;..$3DFF  ; Graphics RAM alias
@@gram_alias3   QEQU    $3E00 ;..$3FFF  ; Graphics RAM alias
@@stic_alias1   QEQU    $4000 ;..$403F  ; STIC alias (incomplete decode)
@@stic_alias2   QEQU    $8000 ;..$803F  ; STIC alias (incomplete decode)
                ENDS



;;==========================================================================;;
;; AY8914 Programmable Sound Generator Register Definitions.                ;;
;;                                                                          ;;
;; PSG0 -- Primary PSG in the Gimini Console.                               ;;
;; PSG1 -- Secondary PSG, available when expansion is attached.             ;;
;; PSG  -- Constants and bitfields common to both PSG's.                    ;;
;;                                                                          ;;
;; Each PSG contains the following set of registers:                        ;;
;;                                                                          ;;
;;     Register pairs:                                                      ;;
;;                                                                          ;;
;;       7   6   5   4   3   2   1   0 | 7   6   5   4   3   2   1   0      ;;
;;     +---------------+---------------|-------------------------------+    ;;
;;  R4 |    unused     |                Channel A Period               | R0 ;;
;;     +---------------+---------------|-------------------------------+    ;;
;;  R5 |    unused     |                Channel B Period               | R1 ;;
;;     +---------------+---------------|-------------------------------+    ;;
;;  R6 |    unused     |                Channel C Period               | R2 ;;
;;     +---------------+---------------|-------------------------------+    ;;
;;  R7 |                        Envelope Period                        | R3 ;;
;;     +-------------------------------|-------+-----------------------+    ;;
;;                                                                          ;;
;;     Single registers:                                                    ;;
;;                                                                          ;;
;;         7       6       5       4       3       2       1       0        ;;
;;     +---------------+-----------------------+-----------------------+    ;;
;;     | I/O Port Dir  |    Noise Enables      |     Tone Enables      |    ;;
;;  R8 |   0   |   0   |   C   |   B   |   A   |   C   |   B   |   A   |    ;;
;;     +-------+-------+-------+-------+-------+-------+-------+-------+    ;;
;;  R9 |            unused     |              Noise Period             |    ;;
;;     +-----------------------+-------+-------+-------+-------+-------+    ;;
;;     |                               |   Envelope Characteristics    |    ;;
;; R10 |            unused             | CONT  |ATTACK | ALTER | HOLD  |    ;;
;;     +---------------+---------------+-------+-------+-------+-------+    ;;
;; R11 |    unused     | A Envl Select |    Channel A Volume Level     |    ;;
;;     +---------------+---------------+-------------------------------+    ;;
;; R12 |    unused     | B Envl Select |    Channel B Volume Level     |    ;;
;;     +---------------+---------------+-------------------------------+    ;;
;; R13 |    unused     | C Envl Select |    Channel C Volume Level     |    ;;
;;     +---------------+---------------+-------------------------------+    ;;
;;                                                                          ;;
;;==========================================================================;;

                ;;==========================================================;;
                ;; Primary PSG in the main console.                         ;;
                ;;==========================================================;;
PSG0            STRUCT  $01F0

                ;;----------------------------------------------------------;;
                ;; Register address definitions                             ;;
                ;;----------------------------------------------------------;;
@@chn_a_lo      QEQU    $ + 0   ; Channel A period, lower 8 bits of 12
@@chn_b_lo      QEQU    $ + 1   ; Channel B period, lower 8 bits of 12
@@chn_c_lo      QEQU    $ + 2   ; Channel C period, lower 8 bits of 12
@@envlp_lo      QEQU    $ + 3   ; Envelope period,  lower 8 bits of 16

@@chn_a_hi      QEQU    $ + 4   ; Channel A period, upper 4 bits of 12
@@chn_b_hi      QEQU    $ + 5   ; Channel B period, upper 4 bits of 12
@@chn_c_hi      QEQU    $ + 6   ; Channel C period, upper 4 bits of 12
@@envlp_hi      QEQU    $ + 7   ; Envelope period,  upper 8 bits of 16

@@chan_enable   QEQU    $ + 8   ; Channel enables (bits 3-5 noise, 0-2 tone)
@@noise         QEQU    $ + 9   ; Noise period (5 bits)
@@envelope      QEQU    $ + 10  ; Envelope type/trigger (4 bits)

@@chn_a_vol     QEQU    $ + 11  ; Channel A volume / Envelope select (6 bits)
@@chn_b_vol     QEQU    $ + 12  ; Channel B volume / Envelope select (6 bits)
@@chn_c_vol     QEQU    $ + 13  ; Channel C volume / Envelope select (6 bits)

@@io_port0      QEQU    $ + 14  ; I/O port 0 (8 bits)
@@io_port1      QEQU    $ + 15  ; I/O port 1 (8 bits)

                ENDS

                ;;==========================================================;;
                ;; Secondary PSG in the expansion unit.                     ;;
                ;;==========================================================;;
PSG1            STRUCT  $00F0

                ;;----------------------------------------------------------;;
                ;; Register address definitions                             ;;
                ;;----------------------------------------------------------;;
@@chn_a_lo      QEQU    $ + 0   ; Channel A period, lower 8 bits of 12
@@chn_b_lo      QEQU    $ + 1   ; Channel B period, lower 8 bits of 12
@@chn_c_lo      QEQU    $ + 2   ; Channel C period, lower 8 bits of 12
@@envlp_lo      QEQU    $ + 3   ; Envelope period,  lower 8 bits of 16

@@chn_a_hi      QEQU    $ + 4   ; Channel A period, upper 4 bits of 12
@@chn_b_hi      QEQU    $ + 5   ; Channel B period, upper 4 bits of 12
@@chn_c_hi      QEQU    $ + 6   ; Channel C period, upper 4 bits of 12
@@envlp_hi      QEQU    $ + 7   ; Envelope period,  upper 8 bits of 16

@@chan_enable   QEQU    $ + 8   ; Channel enables (bits 3-5 noise, 0-2 tone)
@@noise         QEQU    $ + 9   ; Noise period (5 bits)
@@envelope      QEQU    $ + 10  ; Envelope type/trigger (4 bits)

@@chn_a_vol     QEQU    $ + 11  ; Channel A volume / Envelope select (6 bits)
@@chn_b_vol     QEQU    $ + 12  ; Channel B volume / Envelope select (6 bits)
@@chn_c_vol     QEQU    $ + 13  ; Channel C volume / Envelope select (6 bits)

@@io_port0      QEQU    $ + 14  ; I/O port 0 (8 bits)
@@io_port1      QEQU    $ + 15  ; I/O port 1 (8 bits)

                ENDS

                ;;==========================================================;;
                ;; Useful Constants / Bit-field definitions.                ;;
                ;;==========================================================;;
PSG             STRUCT  $0000   ; Constants, etc. common to both PSGs.

                ;;----------------------------------------------------------;;
                ;; Bits to OR together for Channel Enable word              ;;
                ;;----------------------------------------------------------;;
@@tone_a_on     QEQU    00000000b   
@@tone_b_on     QEQU    00000000b
@@tone_c_on     QEQU    00000000b
@@noise_a_on    QEQU    00000000b
@@noise_b_on    QEQU    00000000b
@@noise_c_on    QEQU    00000000b

@@tone_a_off    QEQU    00000001b
@@tone_b_off    QEQU    00000010b
@@tone_c_off    QEQU    00000100b
@@noise_a_off   QEQU    00001000b
@@noise_b_off   QEQU    00010000b
@@noise_c_off   QEQU    00100000b

                ;;----------------------------------------------------------;;
                ;; Bits to OR together for Envelope Type                    ;;
                ;;----------------------------------------------------------;;
@@envl_cont     QEQU    00001000b   ; Continue
@@envl_attack   QEQU    00000100b   ; Attack (vs. Decay)
@@envl_alter    QEQU    00000010b   ; Alternate
@@envl_hold     QEQU    00000001b   ; Hold

                ;;----------------------------------------------------------;;
                ;; Volume levels                                            ;;
                ;;----------------------------------------------------------;;
@@vol_0         QEQU    0
@@vol_1         QEQU    1
@@vol_2         QEQU    2
@@vol_3         QEQU    3
@@vol_4         QEQU    4
@@vol_5         QEQU    5
@@vol_6         QEQU    6
@@vol_7         QEQU    7
@@vol_8         QEQU    8
@@vol_9         QEQU    9
@@vol_10        QEQU    10
@@vol_11        QEQU    11
@@vol_12        QEQU    12
@@vol_13        QEQU    13
@@vol_14        QEQU    14
@@vol_15        QEQU    15
@@vol_envl      QEQU    63

                ENDS

;;==========================================================================;;
;; AY8900 Standard Television Interface Circuit                             ;;
;;                                                                          ;;
;; STIC -- Register definitions, constants and bitfields                    ;;
;;                                                                          ;;
;; The Standard Television Interface Circuit provides a method for placing  ;;
;; characters and graphics on the screen.  It provides a 20x12 matrix of    ;;
;; background 'cards'.  Each card contains an 8x8 tile of pixels.  It also  ;;
;; provides 8 movable objects (refered to here as MOBs).                    ;;
;;                                                                          ;;
;; MOB controls: (Note: 'mob' is MOB number (0 - 7))                        ;;
;;  mob + $00   -- X position, and some attribute bits.                     ;;
;;  mob + $08   -- Y position, and some attribute bits.                     ;;
;;  mob + $10   -- Character definition, more attribute bits.               ;;
;;  mob + $18   -- Collision information.                                   ;;
;;                                                                          ;;
;; Display Mode controls:                                                   ;;
;;  $20         -- Display enable (must be written during vert blank.)      ;;
;;  $21         -- Graphics mode (write sets FGBG, read sets Color Stack)   ;;
;;                                                                          ;;
;; Color Stack and Border Color controls:                                   ;;
;;  $28..$2B    -- Color stack entries 0..3                                 ;;
;;  $2C         -- Screen border color                                      ;;
;;                                                                          ;;
;; Display framing controls:                                                ;;
;;  $30         -- Pixel column delay (0..7)                                ;;
;;  $31         -- Pixel row delay (0..7)                                   ;;
;;  $32         -- Edge masking (bit 0 masks left, bit 1 masks top)         ;;
;;                                                                          ;;
;; Color Set                                                                ;;
;;       Primary Color Set                Pastel Color Set                  ;;
;;    ------------------------       -----------------------------          ;;
;;     0 Black   4 Dark Green          8 Grey     12 Pink                   ;;
;;     1 Blue    5 Green               9 Cyan     13 Light Blue             ;;
;;     2 Red     6 Yellow             10 Orange   14 Yellow-Green           ;;
;;     3 Tan     7 White              11 Brown    15 Purple                 ;;
;;                                                                          ;;
;;==========================================================================;;
STIC            STRUCT  $0000

;;--------------------------------------------------------------------------;;
;; MOB Controls                                                             ;;
;;                                                                          ;;
;; X Register layout:                                                       ;;
;;                                                                          ;;
;;    13   12   11   10    9    8    7    6    5    4    3    2    1    0   ;;
;;  +----+----+----+----+----+----+----+----+----+----+----+----+----+----+ ;;
;;  | ?? | ?? | ?? | X  |VISB|INTR|            X Coordinate               | ;;
;;  |    |    |    |SIZE|    |    |             (0 to 255)                | ;;
;;  +----+----+----+----+----+----+----+----+----+----+----+----+----+----+ ;;
;;                                                                          ;;
;; Y Register layout:                                                       ;;
;;                                                                          ;;
;;    13   12   11   10    9    8    7    6    5    4    3    2    1    0   ;;
;;  +----+----+----+----+----+----+----+----+----+----+----+----+----+----+ ;;
;;  | ?? | ?? | Y  | X  | Y  | Y  |YRES|          Y Coordinate            | ;;
;;  |    |    |FLIP|FLIP|SIZ4|SIZ2|    |           (0 to 127)             | ;;
;;  +----+----+----+----+----+----+----+----+----+----+----+----+----+----+ ;;
;;                                                                          ;;
;; A Register layout:                                                       ;;
;;                                                                          ;;
;;    13   12   11   10    9    8    7    6    5    4    3    2    1    0   ;;
;;  +----+----+----+----+----+----+----+----+----+----+----+----+----+----+ ;;
;;  |PRIO| FG |GRAM|      GRAM/GROM Card # (0 to 255)      |   FG Color   | ;;
;;  |    |bit3|GROM|     (bits 9, 10 ignored for GRAM)     |   Bits 0-2   | ;;
;;  +----+----+----+----+----+----+----+----+----+----+----+----+----+----+ ;;
;;                                                                          ;;
;; C Register layout:                                                       ;;
;;                                                                          ;;
;;    13   12   11   10    9    8    7    6    5    4    3    2    1    0   ;;
;;  +----+----+----+----+----+----+----+----+----+----+----+----+----+----+ ;;
;;  | ?? | ?? | ?? | ?? |COLL|COLL|COLL|COLL|COLL|COLL|COLL|COLL|COLL|COLL| ;;
;;  |    |    |    |    |BORD| BG |MOB7|MOB6|MOB5|MOB4|MOB3|MOB2|MOB1|MOB0| ;;
;;  +----+----+----+----+----+----+----+----+----+----+----+----+----+----+ ;;
;;--------------------------------------------------------------------------;;

@@mob0_x        QEQU    $00 + 0     ; MOB 0 X position, XSIZE/VIS attributes
@@mob1_x        QEQU    $00 + 1     ; MOB 1 X position, XSIZE/VIS attributes
@@mob2_x        QEQU    $00 + 2     ; MOB 2 X position, XSIZE/VIS attributes
@@mob3_x        QEQU    $00 + 3     ; MOB 3 X position, XSIZE/VIS attributes
@@mob4_x        QEQU    $00 + 4     ; MOB 4 X position, XSIZE/VIS attributes
@@mob5_x        QEQU    $00 + 5     ; MOB 5 X position, XSIZE/VIS attributes
@@mob6_x        QEQU    $00 + 6     ; MOB 6 X position, XSIZE/VIS attributes
@@mob7_x        QEQU    $00 + 7     ; MOB 7 X position, XSIZE/VIS attributes

@@mob0_y        QEQU    $08 + 0     ; MOB 0 Y pos'n, YRES/YSIZE/XFLIP/YFLIP
@@mob1_y        QEQU    $08 + 1     ; MOB 1 Y pos'n, YRES/YSIZE/XFLIP/YFLIP
@@mob2_y        QEQU    $08 + 2     ; MOB 2 Y pos'n, YRES/YSIZE/XFLIP/YFLIP
@@mob3_y        QEQU    $08 + 3     ; MOB 3 Y pos'n, YRES/YSIZE/XFLIP/YFLIP
@@mob4_y        QEQU    $08 + 4     ; MOB 4 Y pos'n, YRES/YSIZE/XFLIP/YFLIP
@@mob5_y        QEQU    $08 + 5     ; MOB 5 Y pos'n, YRES/YSIZE/XFLIP/YFLIP
@@mob6_y        QEQU    $08 + 6     ; MOB 6 Y pos'n, YRES/YSIZE/XFLIP/YFLIP
@@mob7_y        QEQU    $08 + 7     ; MOB 7 Y pos'n, YRES/YSIZE/XFLIP/YFLIP

@@mob0_a        QEQU    $10 + 0     ; MOB 0 Color, Card #, Priority
@@mob1_a        QEQU    $10 + 1     ; MOB 1 Color, Card #, Priority
@@mob2_a        QEQU    $10 + 2     ; MOB 2 Color, Card #, Priority
@@mob3_a        QEQU    $10 + 3     ; MOB 3 Color, Card #, Priority
@@mob4_a        QEQU    $10 + 4     ; MOB 4 Color, Card #, Priority
@@mob5_a        QEQU    $10 + 5     ; MOB 5 Color, Card #, Priority
@@mob6_a        QEQU    $10 + 6     ; MOB 6 Color, Card #, Priority
@@mob7_a        QEQU    $10 + 7     ; MOB 7 Color, Card #, Priority

@@mob0_c        QEQU    $18 + 0     ; MOB 0 Collision detect
@@mob1_c        QEQU    $18 + 1     ; MOB 1 Collision detect
@@mob2_c        QEQU    $18 + 2     ; MOB 2 Collision detect
@@mob3_c        QEQU    $18 + 3     ; MOB 3 Collision detect
@@mob4_c        QEQU    $18 + 4     ; MOB 4 Collision detect
@@mob5_c        QEQU    $18 + 5     ; MOB 5 Collision detect
@@mob6_c        QEQU    $18 + 6     ; MOB 6 Collision detect
@@mob7_c        QEQU    $18 + 7     ; MOB 7 Collision detect

                ;;----------------------------------------------------------;;
                ;; Display Mode Controls                                    ;;
                ;;----------------------------------------------------------;;
@@viden         QEQU    $20         ; Display Enable (write during vblank)
@@mode          QEQU    $21         ; Mode select 

                ;;----------------------------------------------------------;;
                ;; Color Stack and Display Border Color Controls            ;;
                ;;----------------------------------------------------------;;
@@colstack      QEQU    $28         ; Base of the color stack
@@cs0           QEQU    $28 + 0     ; Color Stack 0
@@cs1           QEQU    $28 + 1     ; Color Stack 1
@@cs2           QEQU    $28 + 2     ; Color Stack 2
@@cs3           QEQU    $28 + 3     ; Color Stack 3
@@bord          QEQU    $2C         ; Border color

                ;;----------------------------------------------------------;;
                ;; Display Framing Controls                                 ;;
                ;;----------------------------------------------------------;;
@@h_delay       QEQU    $30         ; Horizontal delay (0 - 7 pixels)
@@v_delay       QEQU    $31         ; Vertical delay (0 - 7 pixels)
@@edgemask      QEQU    $32         ; Edge masking

                ;;----------------------------------------------------------;;
                ;; Useful bitfields/constants/masks for MOBS                ;;
                ;;----------------------------------------------------------;;
@@mobx_xpos     QEQU    00000011111111b     ; MOB XREG: X position
@@mobx_intr     QEQU    00000100000000b     ; MOB XREG: Interaction
@@mobx_visb     QEQU    00001000000000b     ; MOB XREG: Visibility
@@mobx_xsize    QEQU    00010000000000b     ; MOB XREG: Horiz 2x magnification

@@moby_ypos     QEQU    00000001111111b     ; MOB YREG: Y position
@@moby_yres     QEQU    00000010000000b     ; MOB YREG: Y res. (8 or 16 rows)
@@moby_ysize2   QEQU    00000100000000b     ; MOB YREG: Vert 2x magnification
@@moby_ysize4   QEQU    00001000000000b     ; MOB YREG: Vert 4x magnification
@@moby_ysize8   QEQU    00001100000000b     ; MOB YREG: 8x mag (sets 4x & 2x)
@@moby_xflip    QEQU    00010000000000b     ; MOB YREG: Flip horizontally
@@moby_yflip    QEQU    00100000000000b     ; MOB YREG: Flip vertically

@@moba_fg0      QEQU    00000000000000b     ; MOB AREG: Foreground color =  0
@@moba_fg1      QEQU    00000000000001b     ; MOB AREG: Foreground color =  1
@@moba_fg2      QEQU    00000000000010b     ; MOB AREG: Foreground color =  2
@@moba_fg3      QEQU    00000000000011b     ; MOB AREG: Foreground color =  3
@@moba_fg4      QEQU    00000000000100b     ; MOB AREG: Foreground color =  4
@@moba_fg5      QEQU    00000000000101b     ; MOB AREG: Foreground color =  5
@@moba_fg6      QEQU    00000000000110b     ; MOB AREG: Foreground color =  6
@@moba_fg7      QEQU    00000000000111b     ; MOB AREG: Foreground color =  7
@@moba_fg8      QEQU    01000000000000b     ; MOB AREG: Foreground color =  8
@@moba_fg9      QEQU    01000000000001b     ; MOB AREG: Foreground color =  9
@@moba_fgA      QEQU    01000000000010b     ; MOB AREG: Foreground color = 10
@@moba_fgB      QEQU    01000000000011b     ; MOB AREG: Foreground color = 11
@@moba_fgC      QEQU    01000000000100b     ; MOB AREG: Foreground color = 12
@@moba_fgD      QEQU    01000000000101b     ; MOB AREG: Foreground color = 13
@@moba_fgE      QEQU    01000000000110b     ; MOB AREG: Foreground color = 14
@@moba_fgF      QEQU    01000000000111b     ; MOB AREG: Foreground color = 15
@@moba_card     QEQU    00000111111000b     ; MOB AREG: Card # mask
@@moba_gram     QEQU    00100000000000b     ; MOB AREG: GRAM card select
@@moba_prio     QEQU    10000000000000b     ; MOB AREG: Priority 
                                            ;           (above/below bkgnd)

@@mobc_coll0    QEQU    00000000000001b     ; MOB CREG: Collision w/ MOB #0
@@mobc_coll1    QEQU    00000000000010b     ; MOB CREG: Collision w/ MOB #1
@@mobc_coll2    QEQU    00000000000100b     ; MOB CREG: Collision w/ MOB #2
@@mobc_coll3    QEQU    00000000001000b     ; MOB CREG: Collision w/ MOB #3
@@mobc_coll4    QEQU    00000000010000b     ; MOB CREG: Collision w/ MOB #4
@@mobc_coll5    QEQU    00000000100000b     ; MOB CREG: Collision w/ MOB #5
@@mobc_coll6    QEQU    00000001000000b     ; MOB CREG: Collision w/ MOB #6
@@mobc_coll7    QEQU    00000010000000b     ; MOB CREG: Collision w/ MOB #7
@@mobc_collmob  QEQU    00000011111111b     ; MOB CREG: Coll w/ any MOB (mask)
@@mobc_collbg   QEQU    00000100000000b     ; MOB CREG; Coll w/ background
@@mobc_collbord QEQU    00001000000000b     ; MOB CREG; Coll w/ background

                ;;----------------------------------------------------------;;
                ;; Useful bits for Edge Masking.                            ;;
                ;;----------------------------------------------------------;;
@@mask_left     QEQU    00000000000001b     ; Edge mask: Mask leftmost 8 pixels
@@mask_top      QEQU    00000000000010b     ; Edge mask: Mask topmost 8 pixels

;;--------------------------------------------------------------------------;;
;; Useful bits for Color Stack Mode                                         ;;
;;                                                                          ;;
;; Display format word layout in Color Stack Mode:                          ;;
;;   13   12    11   10    9    8    7    6    5    4    3    2    1    0   ;;
;; +----+-----+----+----+----+----+----+----+----+----+----+----+----+----+ ;;
;; |Adv.|FG   |GRAM|           GRAM/GROM Card #            |   FG Color   | ;;
;; |col |Bit3/|GROM|    (0-255 for GROM, 0-63 for GRAM)    |   Bits 0-2   | ;;
;; |stck|----------|                                       |              | ;;
;; |    |col. sqr. |                                       |              | ;;
;; |    |mode slct.|                                       |              | ;;
;; +----+-----+----+----+----+----+----+----+----+----+----+----+----+----+ ;;
;;                                                                          ;;
;; Color Stack Notes:                                                       ;;
;;  -- If GRAM card, two MSBs of Card # are ignored, and may be used by     ;;
;;     the program to store other information.                              ;;
;;                                                                          ;;
;;  -- Bit 12 is set and Bit 11 is cleared, this word is treated as a       ;;
;;     colored-square-mode display word, in the format below.               ;;
;;                                                                          ;;
;;  -- The color stack is reset to offset 0 at the start of the display.    ;;
;;     Setting the 'advance' bit advances the color stack by one for that   ;;
;;     card and all cards after it in normal left-to-right scanning order.  ;;
;;                                                                          ;;
;;  -- Bits 14 and 15 of the display format word are ignored and may be     ;;
;;     used by the program to store status bits, etc.                       ;;
;;                                                                          ;;
;; Display format word layout in Colored Squares Mode:                      ;;
;;                                                                          ;;
;;   13   12   11   10    9    8    7    6    5    4    3    2    1    0    ;;
;; +----+----+----+----+----+----+----+----+----+----+----+----+----+----+  ;;
;; |Pix3|  1 |  0 | Pix. 3  | Pix. 2 color | Pix. 1 color | Pix. 0 color |  ;;
;; |Bit2|    |    | bit 0-1 |    (0-7)     |    (0-7)     |    (0-7)     |  ;;
;; +----+----+----+----+----+----+----+----+----+----+----+----+----+----+  ;;
;;                                                                          ;;
;; Pixels are display like so:  +-----+-----+                               ;;
;;                              |Pixel|Pixel|                               ;;
;;                              |  0  |  1  |                               ;;
;;                              +-----+-----+                               ;;
;;                              |Pixel|Pixel|                               ;;
;;                              |  2  |  3  |                               ;;
;;                              +-----+-----+                               ;;
;;                                                                          ;;
;; Colored Square Mode Notes:                                               ;;
;;                                                                          ;;
;;  -- It is not possible to advance the color stack with a card that is    ;;
;;     displayed in color-stack mode.                                       ;;
;;                                                                          ;;
;;  -- Color 7 in colored squares mode instead shows the current color on   ;;
;;     the color stack.                                                     ;;
;;                                                                          ;;
;;  -- Colors 0 through 6 in a colored-square card will interact with       ;;
;;     MOBs, but color 7 will not.                                          ;;
;;                                                                          ;;
;;  -- Bits 14 and 15 of the display format word are ignored and may be     ;;
;;     used by the program to store status bits, etc.                       ;;
;;--------------------------------------------------------------------------;;
@@cs_fg0        QEQU    00000000000000b     ; foreground ==  0
@@cs_fg1        QEQU    00000000000001b     ; foreground ==  1
@@cs_fg2        QEQU    00000000000010b     ; foreground ==  2
@@cs_fg3        QEQU    00000000000011b     ; foreground ==  3
@@cs_fg4        QEQU    00000000000100b     ; foreground ==  4
@@cs_fg5        QEQU    00000000000101b     ; foreground ==  5
@@cs_fg6        QEQU    00000000000110b     ; foreground ==  6
@@cs_fg7        QEQU    00000000000111b     ; foreground ==  7
@@cs_fg8        QEQU    01000000000000b     ; foreground ==  8
@@cs_fg9        QEQU    01000000000001b     ; foreground ==  9
@@cs_fgA        QEQU    01000000000010b     ; foreground == 10
@@cs_fgB        QEQU    01000000000011b     ; foreground == 11
@@cs_fgC        QEQU    01000000000100b     ; foreground == 12
@@cs_fgD        QEQU    01000000000101b     ; foreground == 13
@@cs_fgE        QEQU    01000000000110b     ; foreground == 14
@@cs_fgF        QEQU    01000000000111b     ; foreground == 15
@@cs_card       QEQU    00011111111000b     ; Card # mask (GRAM/GROM index #)
@@cs_gram       QEQU    00100000000000b     ; Selects cards from GRAM if set
@@cs_advance    QEQU    10000000000000b     ; Advances color stack.
@@cs_colsqr     QEQU    01000000000000b     ; Selects 'colored square mode'

@@cs_pix0       QEQU    00000000000111b     ; ColSqr Pixel 0 mask
@@cs_pix1       QEQU    00000000111000b     ; ColSqr Pixel 1 mask
@@cs_pix2       QEQU    00000111000000b     ; ColSqr Pixel 2 mask
@@cs_pix3       QEQU    10011000000000b     ; ColSqr Pixel 3 mask

@@cs_pix0_0     QEQU    00000000000000b     ; ColSqr Pixel 0, color == 0
@@cs_pix0_1     QEQU    00000000000001b     ; ColSqr Pixel 0, color == 1
@@cs_pix0_2     QEQU    00000000000010b     ; ColSqr Pixel 0, color == 2
@@cs_pix0_3     QEQU    00000000000011b     ; ColSqr Pixel 0, color == 3
@@cs_pix0_4     QEQU    00000000000100b     ; ColSqr Pixel 0, color == 4
@@cs_pix0_5     QEQU    00000000000101b     ; ColSqr Pixel 0, color == 5
@@cs_pix0_6     QEQU    00000000000110b     ; ColSqr Pixel 0, color == 6
@@cs_pix0_7     QEQU    00000000000111b     ; ColSqr Pixel 0, color == 7

@@cs_pix1_0     QEQU    00000000000000b     ; ColSqr Pixel 1, color == 0
@@cs_pix1_1     QEQU    00000000001000b     ; ColSqr Pixel 1, color == 1
@@cs_pix1_2     QEQU    00000000010000b     ; ColSqr Pixel 1, color == 2
@@cs_pix1_3     QEQU    00000000011000b     ; ColSqr Pixel 1, color == 3
@@cs_pix1_4     QEQU    00000000100000b     ; ColSqr Pixel 1, color == 4
@@cs_pix1_5     QEQU    00000000101000b     ; ColSqr Pixel 1, color == 5
@@cs_pix1_6     QEQU    00000000110000b     ; ColSqr Pixel 1, color == 6
@@cs_pix1_7     QEQU    00000000111000b     ; ColSqr Pixel 1, color == 7

@@cs_pix2_0     QEQU    00000000000000b     ; ColSqr Pixel 2, color == 0
@@cs_pix2_1     QEQU    00000001000000b     ; ColSqr Pixel 2, color == 1
@@cs_pix2_2     QEQU    00000010000000b     ; ColSqr Pixel 2, color == 2
@@cs_pix2_3     QEQU    00000011000000b     ; ColSqr Pixel 2, color == 3
@@cs_pix2_4     QEQU    00000100000000b     ; ColSqr Pixel 2, color == 4
@@cs_pix2_5     QEQU    00000101000000b     ; ColSqr Pixel 2, color == 5
@@cs_pix2_6     QEQU    00000110000000b     ; ColSqr Pixel 2, color == 6
@@cs_pix2_7     QEQU    00000111000000b     ; ColSqr Pixel 2, color == 7

@@cs_pix3_0     QEQU    00000000000000b     ; ColSqr Pixel 3, color == 0
@@cs_pix3_1     QEQU    00001000000000b     ; ColSqr Pixel 3, color == 1
@@cs_pix3_2     QEQU    00010000000000b     ; ColSqr Pixel 3, color == 2
@@cs_pix3_3     QEQU    00011000000000b     ; ColSqr Pixel 3, color == 3
@@cs_pix3_4     QEQU    10000000000000b     ; ColSqr Pixel 3, color == 4
@@cs_pix3_5     QEQU    10001000000000b     ; ColSqr Pixel 3, color == 5
@@cs_pix3_6     QEQU    10010000000000b     ; ColSqr Pixel 3, color == 6
@@cs_pix3_7     QEQU    10011000000000b     ; ColSqr Pixel 3, color == 7

;;--------------------------------------------------------------------------;;
;; Useful bits for Foreground/Background Mode                               ;;
;;                                                                          ;;
;; Display format word layout in Color Stack Mode:                          ;;
;;   13   12   11   10    9    8    7    6    5    4    3    2    1    0    ;;
;; +----+----+----+----+----+----+----+----+----+----+----+----+----+----+  ;;
;; |BG  |BG  |GRAM|BG  |BG  |      GRAM/GROM Card #       |   FG Color   |  ;;
;; |Bit2|Bit3|GROM|Bit1|Bit0|          (0 - 63)           |   Bits 0-2   |  ;;
;; +----+----+----+----+----+----+----+----+----+----+----+----+----+----+  ;;
;;--------------------------------------------------------------------------;;

@@fb_fg0        QEQU    00000000000000b     ; foreground ==  0
@@fb_fg1        QEQU    00000000000001b     ; foreground ==  1
@@fb_fg2        QEQU    00000000000010b     ; foreground ==  2
@@fb_fg3        QEQU    00000000000011b     ; foreground ==  3
@@fb_fg4        QEQU    00000000000100b     ; foreground ==  4
@@fb_fg5        QEQU    00000000000101b     ; foreground ==  5
@@fb_fg6        QEQU    00000000000110b     ; foreground ==  6
@@fb_fg7        QEQU    00000000000111b     ; foreground ==  7

@@fb_bg0        QEQU    00000000000000b     ; background ==  0
@@fb_bg1        QEQU    00001000000000b     ; background ==  1
@@fb_bg2        QEQU    00010000000000b     ; background ==  2
@@fb_bg3        QEQU    00011000000000b     ; background ==  3
@@fb_bg4        QEQU    10000000000000b     ; background ==  4
@@fb_bg5        QEQU    10001000000000b     ; background ==  5
@@fb_bg6        QEQU    10010000000000b     ; background ==  6
@@fb_bg7        QEQU    10011000000000b     ; background ==  7
@@fb_bg8        QEQU    01000000000000b     ; background ==  8
@@fb_bg9        QEQU    01001000000000b     ; background ==  9
@@fb_bgA        QEQU    01010000000000b     ; background == 10
@@fb_bgB        QEQU    01011000000000b     ; background == 11
@@fb_bgC        QEQU    11000000000000b     ; background == 12
@@fb_bgD        QEQU    11001000000000b     ; background == 13
@@fb_bgE        QEQU    11010000000000b     ; background == 14
@@fb_bgF        QEQU    11011000000000b     ; background == 15

@@fb_card       QEQU    00000111111000b     ; Card # mask (GRAM/GROM index #)
@@fb_gram       QEQU    00100000000000b     ; Selects cards from GRAM if set

                ENDS

;;==========================================================================;;
;;  STIC COLOR-NAMES                                                        ;;
;;                                                                          ;;
;;  These are easier to remember short mnemonics for the STIC's colors.     ;;
;;  You can use the C_xxx colors for color-stack registers, border colors   ;;
;;  and so on where the color is stored in a contiguous field.  Use the     ;;
;;  X_xxx color names for the foreground color on display cards in color-   ;;
;;  stack mode, or for MOB attribute words.                                 ;;
;;                                                                          ;;
;;  Note that for the primary color set, C_xxx and X_xxx are identical.     ;;
;;==========================================================================;;

C_BLK   QEQU    $0              ; Black
C_BLU   QEQU    $1              ; Blue
C_RED   QEQU    $2              ; Red
C_TAN   QEQU    $3              ; Tan
C_DGR   QEQU    $4              ; Dark Green
C_GRN   QEQU    $5              ; Green
C_YEL   QEQU    $6              ; Yellow
C_WHT   QEQU    $7              ; White
C_GRY   QEQU    $8              ; Grey
C_CYN   QEQU    $9              ; Cyan
C_ORG   QEQU    $A              ; Orange
C_BRN   QEQU    $B              ; Brown
C_PNK   QEQU    $C              ; Pink
C_LBL   QEQU    $D              ; Light Blue
C_YGR   QEQU    $E              ; Yellow-Green
C_PUR   QEQU    $F              ; Purple

X_BLK   QEQU    $0              ; Black
X_BLU   QEQU    $1              ; Blue
X_RED   QEQU    $2              ; Red
X_TAN   QEQU    $3              ; Tan
X_DGR   QEQU    $4              ; Dark Green
X_GRN   QEQU    $5              ; Green
X_YEL   QEQU    $6              ; Yellow
X_WHT   QEQU    $7              ; White
X_GRY   QEQU    $1000           ; Grey
X_CYN   QEQU    $1001           ; Cyan
X_ORG   QEQU    $1002           ; Orange
X_BRN   QEQU    $1003           ; Brown
X_PNK   QEQU    $1004           ; Pink
X_LBL   QEQU    $1005           ; Light Blue
X_YGR   QEQU    $1006           ; Yellow-Green
X_PUR   QEQU    $1007           ; Purple

;; ======================================================================== ;;
;;  End of File:  gimini.asm                                                ;;
;; ======================================================================== ;;
