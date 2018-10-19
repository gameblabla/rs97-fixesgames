;; ======================================================================== ;;
;;  Graphical keyboard display for the simple synthesizer.                  ;;
;; ======================================================================== ;;

                INCLUDE "../macro/wide_gfx.mac"


SYNGFX          PROC

                ; Cards 0 - 3 are first row
                ; Cards 4 - 7 are second row
                ; Cards 8, 9  are 49th key
                ; Cards 10, 11 are flat/sharp signs
                ; Cards 12 - 17 are small digits 1 through 6
                ; Cards 18 - 20 are for the border of the keyboard 
        
                wgfx_start  32
                        ;01234567890123456789012345678901
                wgfx    "#.##...##...##.###...##...##...#"
                wgfx    "#.##...##...##.###...##...##...#"
                wgfx    "#.##...##...##.###...##...##...#"
                wgfx    "#.##...##...##.###...##...##...#"
                wgfx    "#.##...##...##.###...##...##...#"
                wgfx    "#.##...##...##.###...##...##...#"
                wgfx    "#.##...##...##.###...##...##...#"
                wgfx    "#.##...##...##.###...##...##...#"
                                                        
                wgfx    "#.###.####.###.####.####.####.##"
                wgfx    "#.###.####.###.####.####.####.##"
                wgfx    "#.###.####.###.####.####.####.##"
                wgfx    "#.###.####.###.####.####.####.##"
                wgfx    "#.###.####.###.####.####.####.##"
                wgfx    "#.###.####.###.####.####.####.##"
                wgfx    "#.###.####.###.####.####.####.##"
                wgfx    ".#...#....#...#....#....#....#.."
                        ;01234567890123456789012345678901
                wgfx_flush

                wgfx_start  8

                wgfx    "#.####.#"
                wgfx    "#.####.#"
                wgfx    "#.####.#"
                wgfx    "#.####.#"
                wgfx    "#.####.#"
                wgfx    "#.####.#"
                wgfx    "#.####.#"
                wgfx    "#.####.#"
                                 
                wgfx    "#.####.#"
                wgfx    "#.####.#"
                wgfx    "#.####.#"
                wgfx    "#.####.#"
                wgfx    "#.####.#"
                wgfx    "#.####.#"
                wgfx    "#.####.#"
                wgfx    ".#....#."

                wgfx    "..#....."
                wgfx    "..#....."
                wgfx    "..#.#..."
                wgfx    "..##.#.."
                wgfx    "..#..#.."
                wgfx    "..#.#..."
                wgfx    "..##...."
                wgfx    "........"

                wgfx    ".....#.."
                wgfx    "..#.###."
                wgfx    ".###.#.."
                wgfx    "..#..#.."
                wgfx    "..#.###."
                wgfx    ".###.#.."
                wgfx    "..#....."
                wgfx    "........"

                wgfx    "........"
                wgfx    "........"
                wgfx    "........"
                wgfx    ".....##."
                wgfx    "......#."
                wgfx    "......#."
                wgfx    "......#."
                wgfx    ".....###"

                wgfx    "........"
                wgfx    "........"
                wgfx    "........"
                wgfx    ".....###"
                wgfx    ".......#"
                wgfx    ".....###"
                wgfx    ".....#.."
                wgfx    ".....###"

                wgfx    "........"
                wgfx    "........"
                wgfx    "........"
                wgfx    ".....###"
                wgfx    ".......#"
                wgfx    "......##"
                wgfx    ".......#"
                wgfx    ".....###"

                wgfx    "........"
                wgfx    "........"
                wgfx    "........"
                wgfx    ".....#.#"
                wgfx    ".....#.#"
                wgfx    ".....###"
                wgfx    ".......#"
                wgfx    ".......#"

                wgfx    "........"
                wgfx    "........"
                wgfx    "........"
                wgfx    ".....###"
                wgfx    ".....#.."
                wgfx    ".....###"
                wgfx    ".......#"
                wgfx    ".....###"

                wgfx    "........"
                wgfx    "........"
                wgfx    "........"
                wgfx    ".....###"
                wgfx    ".....#.."
                wgfx    ".....###"
                wgfx    ".....#.#"
                wgfx    ".....###"

                wgfx    "........"
                wgfx    "........"
                wgfx    "........"
                wgfx    "........"
                wgfx    "########"
                wgfx    "########"
                wgfx    "########"
                wgfx    "########"

                wgfx    "........"
                wgfx    "........"
                wgfx    "########"
                wgfx    "########"
                wgfx    "########"
                wgfx    "########"
                wgfx    "........"
                wgfx    "........"

                wgfx    "........"
                wgfx    "........"
                wgfx    "........"
                wgfx    "########"
                wgfx    "########"
                wgfx    "........"
                wgfx    "........"
                wgfx    "........"
                wgfx_flush

@@len           EQU     $ - SYNGFX
                ENDP

;; ------------------------------------------------------------------------ ;;
;;  DRAW_SYNTH      Draw the synthesizer keyboard on-screen.                ;;
;; ------------------------------------------------------------------------ ;;
DRAW_SYNTH      PROC

                PSHR    R5
               
                MVII    #disp_ptr(2,1), R3
               
                MVII    #4,             R4
@@o_loop       
                MVII    #$0807,         R0
                MVII    #$0827,         R1
                
                MVII    #4,             R2
@@i_loop       
                MVO@    R0,             R3
                ADDI    #20,            R3
                MVO@    R0,             R3
                ADDI    #20,            R3
                MVO@    R1,             R3
                SUBI    #39,            R3
               
                ADDI    #8,             R0
                ADDI    #8,             R1
               
                DECR    R2
                BNEQ    @@i_loop
               
                DECR    R4
                BNEQ    @@o_loop
               
                MVII    #$0847,         R0
                MVO@    R0,             R3
                ADDI    #20,            R3
                MVO@    R0,             R3
                ADDI    #8,             R0
                ADDI    #20,            R3
                MVO@    R0,             R3


                MVII    #disp_ptr(1,0), R4
                MVII    #STIC.fb_fg0 + STIC.fb_bg8, R0
                MVII    #11,            R1
                MVO@    R0,             R4
                MVII    #$800 + 19*8 + STIC.fb_fg0 + STIC.fb_bg8, R2
                MVO@    R2,             R4
                MVO@    R2,             R4
                MVO@    R2,             R4
                MVO@    R2,             R4
                NOP
                MVO@    R2,             R4
                MVO@    R2,             R4
                MVO@    R2,             R4

@@t_loop:       MVO@    R0,             R4
                DECR    R1
                BNEQ    @@t_loop

                MVO     R0,     disp_ptr(2,0)
                MVO     R0,     disp_ptr(3,0)
                MVO     R0,     disp_ptr(4,0)
                NOP
                MVO     R0,     disp_ptr(2,18)
                MVO     R0,     disp_ptr(3,18)
                MVO     R0,     disp_ptr(4,18)
               
                MVII    #disp_ptr(5,0), R4
                MVII    #$800 + 18*8 + STIC.fb_fg0 + STIC.fb_bg8, R0
                MVII    #19,            R1
@@b_loop:       MVO@    R0,             R4
                DECR    R1
                BNEQ    @@b_loop


                MVII    #16 + STIC.moby_ysize2, R0
                MVO     R0,             STICSH + 8 + 6
                MVII    #18, R0
                MVO     R0,             STICSH + 8 + 7
                MVII    #17 + STIC.mobx_xsize + STIC.mobx_visb, R0
                MVO     R0,             STICSH + 0 + 6
                MVII    #63 + STIC.mobx_visb, R0
                MVO     R0,             STICSH + 0 + 7

                MVII    #$800 + 20*8 + C_WHT,   R0
                MVO     R0,             STICSH + 16 + 6
                MVII    #$800 + 20*8 + C_RED,   R0
                MVO     R0,             STICSH + 16 + 7

                PULR    PC
                ENDP
               
KEY_X_OFS       PROC
                DECLE    1,  3,  5,  8, 10, 14, 17, 19, 22, 24, 27, 29
                ENDP
               
KEY_Y_OFS       PROC
                DECLE   16,  0, 16,  0, 16, 16,  0, 16,  0, 16,  0, 16
                ENDP

;; ------------------------------------------------------------------------ ;;
;;  GET_KEY_XY      Return the X/Y offset of the lower left corner of the   ;;
;;                  key, relative to the upper left corner of the keyboard  ;;
;; ------------------------------------------------------------------------ ;;
GET_KEY_XY      PROC
               
                MVII    #KEY_X_OFS + 12, R1
                ADDR    R0,         R1
                MVII    #-32,       R0
               
@@loop:         SUBI    #12,        R1
                ADDI    #32,        R0
                CMPI    #KEY_X_OFS + 12, R1
                BC      @@loop
               
                ADD@    R1,         R0          ; R1 is X offset
               
                ADDI    #KEY_Y_OFS - KEY_X_OFS, R1
                MVI@    R1,         R2          ; R2 is Y offset

                JR      R5
                ENDP

;; ------------------------------------------------------------------------ ;;
;;  UPD_KEY_MOBS    Move MOBS 0 through 5 to indicate what keys (if any)    ;;
;;                  are pressed.                                            ;;
;; ------------------------------------------------------------------------ ;;
UPD_KEY_MOBS    PROC
                PSHR    R5
@@x_bits        QSET    STIC.mobx_visb   + 12
@@y_bits        QSET    STIC.moby_ysize2 + 21
@@a_bits        QSET    STIC.moba_fgF + $800 + 12*8

                MVII    #ACTIVE,    R4
                MVII    #STICSH,    R3

                MVII    #@@a_bits,  R5
@@active_loop:
                MVI@    R4,         R0          ; Is this channel active?
                DECR    R0
                BPL     @@active

@@inactive      CLRR    R0                      ; Deactivate by putting  
                MVO@    R0,         R3          ; MOB in column 0.
                INCR    R3
                B       @@next
                
@@active:       CALL    GET_KEY_XY              ; X in R0, Y in R2

                ADDI    #@@x_bits,  R0          ; \_ New X position
                MVO@    R0,         R3          ; /
                ADDI    #8,         R3          ; 
                ADDI    #@@y_bits,  R2          ; \_ New Y position
                MVO@    R2,         R3          ; /
                SUBI    #7,         R3          ; 

@@next          CMPI    #ACTIVE+6,  R4
                BNEQ    @@active_loop


                MVII    #STICSH+16, R4
                MVII    #@@a_bits,  R3
                MVII    #6,         R1
@@attr_loop:
                MVO@    R3,         R4
                ADDI    #8,         R3
                DECR    R1
                BNEQ    @@attr_loop

                PULR    PC
                ENDP

