;; ======================================================================== ;;
;;  Graphics for 'Tank'.                                                    ;;
;; ======================================================================== ;;


; Here's what the ugly tank looks like, roughly.  It is composed of 3 MOBs
; to get 3 different colors.  The mobs will be displayed half-height.
; Two of the MOBs will be 8x16, and one will be 8x8.  This will allow the 
; turret MOB to also provide some color to the tank treads.
;
;     0   1   2   3   4   5   6   7   
;   :   :   :   :   :   :   :   :   :
; 0 :   :   :   :   :   :   :   :   :
; 1 :   :   :   :   :   :   :   :   :
; 2 :   :   :   :   :   :   :   :   :
; 3 :   :   :   :   :   :   :   :   :
; 4 :   :   :   :   :   :   :   :   :
; 5 :   :   :   :   :   :   :   :   :
; 6 :   :   ########:   :   :   :   :
; 7 :   :   ########************:   :
; 8 :   :   ########:   :   :   :   :
; 9 :   ####################:   :   :
;10 ################################:
;11 ################################:
;12 ############################:   :
;13 :   ****++++++++****++++:   :   :
;14 :   ++++############++++:   :   :
;15 :   ++++****++++++++****:   :   :
;   :   :   :   :   :   :   :   :   :
;


;; ======================================================================== ;;
;;  GFX_DATA:  The actual graphics data.                                    ;;
;; ======================================================================== ;;
GFX_DATA    PROC
            ;; ------------------------------------------------------------ ;;
            ;;  Tank graphics.                                              ;;
            ;; ------------------------------------------------------------ ;;

@@tbody:    ;; Tank body.
            gfx_start  ;01234567;
            gfx_row    "........"   ; 0
            gfx_row    "........"   ; 1
            gfx_row    "........"   ; 2
            gfx_row    "........"   ; 3
            gfx_row    "........"   ; 4
            gfx_row    "........"   ; 5
            gfx_row    "..##...."   ; 6
            gfx_row    "..##...."   ; 7
            gfx_row    "..##...."   ; 8
            gfx_row    ".#####.."   ; 9
            gfx_row    "########"   ; 10
            gfx_row    "########"   ; 11
            gfx_row    "#######."   ; 12
            gfx_row    ".#####.."   ; 13
            gfx_row    ".#####.."   ; 14
            gfx_row    ".#####.."   ; 15
            gfx_flush  ;01234567;

@@tread0:   ;; Tank tread, position 0
            gfx_start  ;01234567;
            gfx_row    "........"   ; 8
            gfx_row    "........"   ; 9
            gfx_row    "........"   ; 10
            gfx_row    "........"   ; 11
            gfx_row    "........"   ; 12
            gfx_row    "..++.+.."   ; 13
            gfx_row    ".+...+.."   ; 14
            gfx_row    ".+.++..."   ; 15
            gfx_flush  ;01234567;

@@tread1:   ;; Tank tread, position 1
            gfx_start  ;01234567;
            gfx_row    "........"   ; 8
            gfx_row    "........"   ; 9
            gfx_row    "........"   ; 10
            gfx_row    "........"   ; 11
            gfx_row    "........"   ; 12
            gfx_row    ".+.++..."   ; 13
            gfx_row    ".+...+.."   ; 14
            gfx_row    "..++.+.."   ; 15
            gfx_flush  ;01234567;

@@tread2:   ;; Tank tread, position 2
            gfx_start  ;01234567;
            gfx_row    "........"   ; 8
            gfx_row    "........"   ; 9
            gfx_row    "........"   ; 10
            gfx_row    "........"   ; 11
            gfx_row    "........"   ; 12
            gfx_row    ".++.++.."   ; 13
            gfx_row    "........"   ; 14
            gfx_row    ".++.++.."   ; 15
            gfx_flush  ;01234567;

            ;; ------------------------------------------------------------ ;;
            ;;  The turret graphic pictures are cut into two sets.  Rows    ;;
            ;;  0..7 form the upper half, and change based on the current   ;;
            ;;  aiming of the tank.  Rows 8..15 form the bottom half, and   ;;
            ;;  remain fixed.  The lower rows add color to the tank treads. ;;
            ;; ------------------------------------------------------------ ;;

@@turbot:   ;; Turret graphic, bottom half
            gfx_start  ;01234567;
            gfx_row    "........"   ; 8
            gfx_row    "........"   ; 9
            gfx_row    "........"   ; 10
            gfx_row    "........"   ; 11
            gfx_row    "........"   ; 12
            gfx_row    ".*****.."   ; 13
            gfx_row    ".*...*.."   ; 14
            gfx_row    ".*****.."   ; 15
            gfx_flush  ;01234567;


            ; 0 degrees
            ;
            ;     0   1   2   3   4   5   6   7   
            ;   :   :   :   :   :   :   :   :   :
            ; 0 :   :   :   :   :   :   :   :   :
            ; 1 :   :   :   :   :   :   :   :   :
            ; 2 :   :   :   :   :   :   :   :   :
            ; 3 :   :   :   :   :   :   :   :   :
            ; 4 :   :   :   :   :   :   :   :   :
            ; 5 :   :   :   :   :   :   :   :   :
            ; 6 :   :   ########:   :   :   :   :
            ; 7 :   :   ########************:   :
            ; 8 :   :   ########:   :   :   :   :
            ; 9 :   ####################:   :   :
            ;10 ################################:
            ;11 ################################:
            ;12 ############################:   :
            ;13 :   ****++++++++****++++:   :   :
            ;14 :   ++++############++++:   :   :
            ;15 :   ++++****++++++++****:   :   :
            ;   :   :   :   :   :   :   :   :   :
            
@@tur0:     ;; Turret graphic, upper half, 0 degrees (horizontal)
            gfx_start  ;01234567;
            gfx_row    "........"   ; 0
            gfx_row    "........"   ; 1
            gfx_row    "........"   ; 2
            gfx_row    "........"   ; 3
            gfx_row    "........"   ; 4
            gfx_row    "........"   ; 5
            gfx_row    "........"   ; 6
            gfx_row    "....***."   ; 7
            gfx_flush  ;01234567;
        

            ; 8.1 degrees
            ;
            ;     0   1   2   3   4   5   6   7   
            ;   :   :   :   :   :   :   :   :   :
            ; 0 :   :   :   :   :   :   :   :   :
            ; 1 :   :   :   :   :   :   :   :   :
            ; 2 :   :   :   :   :   :   :   :   :
            ; 3 :   :   :   :   :   :   :   :   :
            ; 4 :   :   :   :   :   :   :   :   :
            ; 5 :   :   :   :   :   :   :   :   :
            ; 6 :   :   ########:   ********:   :
            ; 7 :   :   ########****:   :   :   :
            ; 8 :   :   ########:   :   :   :   :
            ; 9 :   ####################:   :   :
            ;10 ################################:
            ;11 ################################:
            ;12 ############################:   :
            ;13 :   ****++++++++****++++:   :   :
            ;14 :   ++++############++++:   :   :
            ;15 :   ++++****++++++++****:   :   :
            ;   :   :   :   :   :   :   :   :   :
            
@@tur1:     ;; Turret graphic, upper half, 7 degrees
            gfx_start   ;01234567;
            gfx_row     "........"  ; 0
            gfx_row     "........"  ; 1
            gfx_row     "........"  ; 2
            gfx_row     "........"  ; 3
            gfx_row     "........"  ; 4
            gfx_row     "........"  ; 5
            gfx_row     ".....**."  ; 6
            gfx_row     "....*..."  ; 7
            gfx_flush   ;01234567;
           
            ; 23 degrees
            ;
            ;     0   1   2   3   4   5   6   7   
            ;   :   :   :   :   :   :   :   :   :
            ; 0 :   :   :   :   :   :   :   :   :
            ; 1 :   :   :   :   :   :   :   :   :
            ; 2 :   :   :   :   :   :   :   :   :
            ; 3 :   :   :   :   :   :   :   :   :
            ; 4 :   :   :   :   :   :   ****:   :
            ; 5 :   :   :   :   :   ****:   :   :
            ; 6 :   :   ########****:   :   :   :
            ; 7 :   :   ########:   :   :   :   :
            ; 8 :   :   ########:   :   :   :   :
            ; 9 :   ####################:   :   :
            ;10 ################################:
            ;11 ################################:
            ;12 ############################:   :
            ;13 :   ****++++++++****++++:   :   :
            ;14 :   ++++############++++:   :   :
            ;15 :   ++++****++++++++****:   :   :
            ;   :   :   :   :   :   :   :   :   :
            
@@tur2:     ;; Turret graphic, upper half, 20 degrees
            gfx_start   ;01234567;
            gfx_row     "........"  ; 0
            gfx_row     "........"  ; 1
            gfx_row     "........"  ; 2
            gfx_row     "........"  ; 3
            gfx_row     "......*."  ; 4
            gfx_row     ".....*.."  ; 5
            gfx_row     "....*..."  ; 6
            gfx_row     "........"  ; 7
            gfx_flush   ;01234567;


            ; 39 degrees
            ;
            ;     0   1   2   3   4   5   6   7   
            ;   :   :   :   :   :   :   :   :   :
            ; 0 :   :   :   :   :   :   :   :   :
            ; 1 :   :   :   :   :   :   :   :   :
            ; 2 :   :   :   :   :   ****:   :   :
            ; 3 :   :   :   :   :   ****:   :   :
            ; 4 :   :   :   :   ****:   :   :   :
            ; 5 :   :   :   :   ****:   :   :   :
            ; 6 :   :   ########:   :   :   :   :
            ; 7 :   :   ########:   :   :   :   :
            ; 8 :   :   ########:   :   :   :   :
            ; 9 :   ####################:   :   :
            ;10 ################################:
            ;11 ################################:
            ;12 ############################:   :
            ;13 :   ****++++++++****++++:   :   :
            ;14 :   ++++############++++:   :   :
            ;15 :   ++++****++++++++****:   :   :
            ;   :   :   :   :   :   :   :   :   :
            
@@tur3:     ;; Turret graphic, upper half, 39 degrees
            gfx_start   ;01234567;
            gfx_row     "........"  ; 0
            gfx_row     "........"  ; 1
            gfx_row     ".....*.."  ; 2
            gfx_row     ".....*.."  ; 3
            gfx_row     "....*..."  ; 4
            gfx_row     "....*..."  ; 5
            gfx_row     "........"  ; 6
            gfx_row     "........"  ; 7
            gfx_flush   ;01234567;


            ;
            ; 60 degrees
            ;
            ;     0   1   2   3   4   5   6   7   
            ;   :   :   :   :   :   :   :   :   :
            ; 0 :   :   :   :   :   :   :   :   :
            ; 1 :   :   :   :   ****:   :   :   :
            ; 2 :   :   :   :   ****:   :   :   :
            ; 3 :   :   :   ********:   :   :   :
            ; 4 :   :   :   ****:   :   :   :   :
            ; 5 :   :   :   ****:   :   :   :   :
            ; 6 :   :   ########:   :   :   :   :
            ; 7 :   :   ########:   :   :   :   :
            ; 8 :   :   ########:   :   :   :   :
            ; 9 :   ####################:   :   :
            ;10 ################################:
            ;11 ################################:
            ;12 ############################:   :
            ;13 :   ****++++++++****++++:   :   :
            ;14 :   ++++############++++:   :   :
            ;15 :   ++++****++++++++****:   :   :
            ;   :   :   :   :   :   :   :   :   :
            ;
            
@@tur4:     ;; Turret graphic, upper half, 60 degrees
            gfx_start   ;01234567;
            gfx_row     "........"  ; 0
            gfx_row     "....*..."  ; 1
            gfx_row     "....*..."  ; 2
            gfx_row     "...**..."  ; 3
            gfx_row     "...*...."  ; 4
            gfx_row     "...*...."  ; 5
            gfx_row     "........"  ; 6
            gfx_row     "........"  ; 7
            gfx_flush   ;01234567;

            ;
            ; 75 degrees
            ;
            ;     0   1   2   3   4   5   6   7   
            ;   :   :   :   :   :   :   :   :   :
            ; 0 :   :   :   :   ****:   :   :   :
            ; 1 :   :   :   :   ****:   :   :   :
            ; 2 :   :   :   ********:   :   :   :
            ; 3 :   :   :   ****;   :   :   :   :
            ; 4 :   :   :   ****:   :   :   :   :
            ; 5 :   :   :   ****:   :   :   :   :
            ; 6 :   :   ########:   :   :   :   :
            ; 7 :   :   ########:   :   :   :   :
            ; 8 :   :   ########:   :   :   :   :
            ; 9 :   ####################:   :   :
            ;10 ################################:
            ;11 ################################:
            ;12 ############################:   :
            ;13 :   ****++++++++****++++:   :   :
            ;14 :   ++++############++++:   :   :
            ;15 :   ++++****++++++++****:   :   :
            ;   :   :   :   :   :   :   :   :   :
            ;
            
@@tur5:     ;; Turret graphic, upper half, 75 degrees
            gfx_start   ;01234567;
            gfx_row     "....*..."  ; 0
            gfx_row     "....*..."  ; 1
            gfx_row     "...**..."  ; 2
            gfx_row     "...*...."  ; 3
            gfx_row     "...*...."  ; 4
            gfx_row     "...*...."  ; 5
            gfx_row     "........"  ; 6
            gfx_row     "........"  ; 7
            gfx_flush   ;01234567;


            ;
            ;
            ; 90 degrees
            ;
            ;     0   1   2   3   4   5   6   7   
            ;   :   :   :   :   :   :   :   :   :
            ; 0 :   :   :   ****:   :   :   :   :
            ; 1 :   :   :   ****:   :   :   :   :
            ; 2 :   :   :   ****:   :   :   :   :
            ; 3 :   :   :   ****:   :   :   :   :
            ; 4 :   :   :   ****:   :   :   :   :
            ; 5 :   :   :   ****:   :   :   :   :
            ; 6 :   :   ########:   :   :   :   :
            ; 7 :   :   ########:   :   :   :   :
            ; 8 :   :   ########:   :   :   :   :
            ; 9 :   ####################:   :   :
            ;10 ################################:
            ;11 ################################:
            ;12 ############################:   :
            ;13 :   ****++++++++****++++:   :   :
            ;14 :   ++++############++++:   :   :
            ;15 :   ++++****++++++++****:   :   :
            ;   :   :   :   :   :   :   :   :   :

@@tur6:     ;; Turret graphic, upper half, 90 degrees
            gfx_start   ;01234567;
            gfx_row     "...*...."  ; 0
            gfx_row     "...*...."  ; 1
            gfx_row     "...*...."  ; 2
            gfx_row     "...*...."  ; 3
            gfx_row     "...*...."  ; 4
            gfx_row     "...*...."  ; 5
            gfx_row     "........"  ; 6
            gfx_row     "........"  ; 7
            gfx_flush   ;01234567;


            ;; ------------------------------------------------------------ ;;
            ;;  Miscellaneous other graphics.                               ;;
            ;; ------------------------------------------------------------ ;;

@@exp0:     ;; Frame 0 of an explosion.  Explosions are normal res 8x8 MOBs.
         
            gfx_start   ;01234567;
            gfx_row     "........"  ; 0
            gfx_row     "........"  ; 1
            gfx_row     "........"  ; 2
            gfx_row     "........"  ; 3
            gfx_row     "........"  ; 4
            gfx_row     "........"  ; 5
            gfx_row     "........"  ; 6
            gfx_row     "...##..."  ; 7
            gfx_flush   ;01234567;
         
@@exp1:     ;; Frame 1 of an explosion.
         
            gfx_start   ;01234567;
            gfx_row     "........"  ; 0
            gfx_row     "........"  ; 1
            gfx_row     "........"  ; 2
            gfx_row     "........"  ; 3
            gfx_row     "........"  ; 4
            gfx_row     "........"  ; 5
            gfx_row     "...##..."  ; 6
            gfx_row     "..####.."  ; 7
            gfx_flush   ;01234567;
         
@@exp2:     ;; Frame 2 of an explosion.
         
            gfx_start   ;01234567;
            gfx_row     "........"  ; 0
            gfx_row     "........"  ; 1
            gfx_row     "........"  ; 2
            gfx_row     "........"  ; 3
            gfx_row     "........"  ; 4
            gfx_row     "..####.."  ; 5
            gfx_row     ".######."  ; 6
            gfx_row     "..#..#.."  ; 7
            gfx_flush   ;01234567;
         
@@exp3:     ;; Frame 3 of an explosion.
         
            gfx_start   ;01234567;
            gfx_row     "........"  ; 0
            gfx_row     "........"  ; 1
            gfx_row     "........"  ; 2
            gfx_row     "........"  ; 3
            gfx_row     "..####.."  ; 4
            gfx_row     ".######."  ; 5
            gfx_row     ".##..##."  ; 6
            gfx_row     "..####.."  ; 7
            gfx_flush   ;01234567;
         
@@exp4:     ;; Frame 4 of an explosion.
         
            gfx_start   ;01234567;
            gfx_row     "........"  ; 0
            gfx_row     "........"  ; 1
            gfx_row     "........"  ; 2
            gfx_row     ".#.##.#."  ; 3 
            gfx_row     "#.####.#"  ; 4
            gfx_row     ".##..##."  ; 5
            gfx_row     ".######."  ; 6
            gfx_row     "..#..#.."  ; 7
            gfx_flush   ;01234567;
         
@@exp5:     ;; Frame 5 of an explosion.
         
            gfx_start   ;01234567;
            gfx_row     "........"  ; 0
            gfx_row     "........"  ; 1
            gfx_row     "..#..#.."  ; 2
            gfx_row     "#......#"  ; 3 
            gfx_row     "..####.."  ; 4
            gfx_row     ".##..##."  ; 5
            gfx_row     ".#....#."  ; 6
            gfx_row     "........"  ; 7
            gfx_flush   ;01234567;
         
@@exp6:     ;; Frame 6 of an explosion.
         
            gfx_start   ;01234567;
            gfx_row     "........"  ; 0
            gfx_row     "........"  ; 1
            gfx_row     "...##..."  ; 2
            gfx_row     ".#....#."  ; 3 
            gfx_row     "........"  ; 4
            gfx_row     "#......#"  ; 5
            gfx_row     "........"  ; 6
            gfx_row     "........"  ; 7
            gfx_flush   ;01234567;
         
@@exp7:     ;; Frame 7 of an explosion.

            gfx_start   ;01234567;
            gfx_row     "........"  ; 0
            gfx_row     "........"  ; 1
            gfx_row     "..#..#.."  ; 2
            gfx_row     "#......#"  ; 3 
            gfx_row     "........"  ; 4
            gfx_row     "........"  ; 5
            gfx_row     "........"  ; 6
            gfx_row     "........"  ; 7
            gfx_flush   ;01234567;


@@bullet:   ;; This is the projectile we'll be firing.  For now it is an
            ;; uninspiring dot.
            gfx_start   ;01234567;
            gfx_row     "........"  ; 0
            gfx_row     "........"  ; 1
            gfx_row     "........"  ; 2
            gfx_row     "........"  ; 3
            gfx_row     "........"  ; 4
            gfx_row     "........"  ; 5
            gfx_row     "........"  ; 6
            gfx_row     ".......#"  ; 7
            gfx_flush   ;01234567;


@@gram_img: ;; These cards are loaded in the beginning of GRAM at the
            ;; start of the game


            ; degrees symbol.  Loaded in GRAM #0
@@degsym:   
            gfx_start   ;01234567;
            gfx_row     ".###...."
            gfx_row     ".#.#...."
            gfx_row     ".###...."
            gfx_row     "........"
            gfx_row     "........"
            gfx_row     "........"
            gfx_row     "........"
            gfx_flush

            ; horizontal bar.  This forms the top edge of status row.
@@st_bar:   
            gfx_start   ;01234567;
            gfx_row     "........"
            gfx_row     "........"
            gfx_row     "........"
            gfx_row     "........"
            gfx_row     "........"
            gfx_row     "........"
            gfx_row     "........"
            gfx_row     "########"
            gfx_flush

@@gram_end
            ENDP
            

;; ======================================================================== ;;
;;  GFX:  Short-hand names for referring to individual pictures in above.   ;;
;;        These labels are integer picture numbers that fit in 8-bit mem.   ;;
;; ======================================================================== ;;
GFX         PROC
@@tbody     EQU     (GFX_DATA.tbody  - GFX_DATA) / 4
@@tread0    EQU     (GFX_DATA.tread0 - GFX_DATA) / 4
@@tread1    EQU     (GFX_DATA.tread1 - GFX_DATA) / 4
@@tread2    EQU     (GFX_DATA.tread2 - GFX_DATA) / 4
@@turbot    EQU     (GFX_DATA.turbot - GFX_DATA) / 4
@@tur0      EQU     (GFX_DATA.tur0   - GFX_DATA) / 4
@@tur1      EQU     (GFX_DATA.tur1   - GFX_DATA) / 4
@@tur2      EQU     (GFX_DATA.tur2   - GFX_DATA) / 4
@@tur3      EQU     (GFX_DATA.tur3   - GFX_DATA) / 4
@@tur4      EQU     (GFX_DATA.tur4   - GFX_DATA) / 4
@@tur5      EQU     (GFX_DATA.tur5   - GFX_DATA) / 4
@@tur6      EQU     (GFX_DATA.tur6   - GFX_DATA) / 4
@@exp0      EQU     (GFX_DATA.exp0   - GFX_DATA) / 4
@@exp1      EQU     (GFX_DATA.exp1   - GFX_DATA) / 4
@@exp2      EQU     (GFX_DATA.exp2   - GFX_DATA) / 4
@@exp3      EQU     (GFX_DATA.exp3   - GFX_DATA) / 4
@@exp4      EQU     (GFX_DATA.exp4   - GFX_DATA) / 4
@@exp5      EQU     (GFX_DATA.exp5   - GFX_DATA) / 4
@@exp6      EQU     (GFX_DATA.exp6   - GFX_DATA) / 4
@@exp7      EQU     (GFX_DATA.exp7   - GFX_DATA) / 4
@@bullet    EQU     (GFX_DATA.bullet - GFX_DATA) / 4

@@none      EQU     $80

@@gram_size EQU     GFX_DATA.gram_end - GFX_DATA.gram_img

@@degsym    EQU     (GFX_DATA.degsym - GFX_DATA.gram_img) / 4
@@st_bar    EQU     (GFX_DATA.st_bar - GFX_DATA.gram_img) / 4

            ENDP
