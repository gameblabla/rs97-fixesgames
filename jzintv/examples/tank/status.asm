;; ======================================================================== ;;
;;  STATUS  -- implements code to display the status bar at top/bottom      ;;
;; ======================================================================== ;;



;; ======================================================================== ;;
;;  INIT_STAT -- Initializes the status bar                                 ;;
;; ======================================================================== ;;
INIT_STAT   PROC
            PSHR    R5

            ;; ------------------------------------------------------------ ;;
            ;;  Draw the top border of the status bar.                      ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #gen_cstk_card(GFX.st_bar, GRAM, Blue, NoAdv), R0
            MVII    #20,                R1
            MVII    #disp_ptr(10, 0),   R4
            CALL    FILLMEM

            ;; ------------------------------------------------------------ ;;
            ;;  Draw the status bar itself.                                 ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #gen_cstk_card((ASC(" ",0) - $20), GROM, Blue, Adv), R0
            MVII    #disp_ptr(11, 0),   R4
            MVO@    R0,                 R4
            MVII    #19,                R1
            MVII    #gen_cstk_card((ASC(" ",0) - $20), GROM, Blue, NoAdv), R0
            CALL    FILLMEM

            ;; ------------------------------------------------------------ ;;
            ;;  Force a status bar update.                                  ;;
            ;; ------------------------------------------------------------ ;;
            CLRR    R0
            B       UPD_STATUS.1

            ENDP


;; ======================================================================== ;;
;;  UPD_STATUS -- Update the status bar, if needed.                          ;;
;; ======================================================================== ;;

UPD_STATUS  PROC
            PSHR    R5

            ;; ------------------------------------------------------------ ;;
            ;;  Is a status update necessary?                               ;;
            ;; ------------------------------------------------------------ ;;
            MVI     DO_STATS,   R0          ;\
            DECR    R0                      ; |_ if the flag's non-zero
            BMI     @@leave                 ; |  do the update, else leave
@@1:        MVO     R0,         DO_STATS    ;/   

            
            ;; ------------------------------------------------------------ ;;
            ;;  Layout:                                                     ;;
            ;;      Col 1..2    Tank 0's firing strength                    ;;
            ;;      Col 5..8    Tank 0's angle                              ;;
            ;;                                                              ;;
            ;;      Col 12..15  Tank 1's angle                              ;;
            ;;      Col 17..18  Tank 1's firing strength                    ;;
            ;; ------------------------------------------------------------ ;;
                


            MVII    #disp_ptr(11,1), R4     ; column 1, bottom row

            MVI     TNK0_POW,   R0          ; get tank 0's power
            MVII    #2,         R2          ; 2 digits
            MVII    #gen_cstk_card(0, GROM, White, NoAdv), R3
            CALL    PRNUM16.b               

            MVII    #disp_ptr(11,5), R4     ; c

            MVI     TNK0_ANG,   R0          ; get tank 0's turret angle
            SLL     R0,         1           ;\
            MOVR    R0,         R1          ; |_ Multiply by 10
            SLL     R1,         2           ; |
            ADDR    R1,         R0          ;/

            MVII    #gen_cstk_card(0, GROM, Yellow, NoAdv), R3
            CALL    PRNUM16.l

            MVII    #gen_cstk_card(GFX.degsym, GRAM, Yellow, NoAdv), R0
            MVO@    R0,         R4
            MVII    #gen_cstk_card(0, GROM, Yellow, NoAdv), R0
            MVO@    R0,         R4


            MVII    #disp_ptr(11,12), R4    ; 

            MVI     TNK1_ANG,   R0          ; get tank 1's turret angle 
            SLL     R0,         1           ;\
            MOVR    R0,         R1          ; |_ Multipy by 10
            SLL     R1,         2           ; |
            ADDR    R1,         R0          ;/ 
            MVII    #3,         R2          ; display in 3-digit field
            MVII    #gen_cstk_card(0, GROM, Yellow, NoAdv), R3
            CALL    PRNUM16.b


            MVII    #gen_cstk_card(GFX.degsym, GRAM, Yellow, NoAdv), R0
            MVO@    R0,         R4          ; draw a degrees symbol

            MVII    #disp_ptr(11,17), R4    ; 
            MVI     TNK1_POW,   R0          ; get tank 1's power
            MVII    #2,         R2          ; 2 digits
            MVII    #gen_cstk_card(0, GROM, White, NoAdv), R3
            CALL    PRNUM16.b
            ;; ------------------------------------------------------------ ;;
            ;;  Now put the scores in the upper left, right corners.        ;;
            ;; ------------------------------------------------------------ ;;
            MVI     TNK0_PTS,   R0
            MVII    #disp_ptr(0, 1), R4
            MVII    #gen_cstk_card(0, GROM, Black, NoAdv), R3
            CALL    PRNUM16.l

            MVI     TNK1_PTS,   R0
            MVII    #disp_ptr(0, 14), R4
            MVII    #gen_cstk_card(0, GROM, Black, NoAdv), R3
            MVII    #5,         R2
            CALL    PRNUM16.b


@@leave:    PULR    PC
            ENDP


;* ======================================================================== *;
;*  This program is free software; you can redistribute it and/or modify    *;
;*  it under the terms of the GNU General Public License as published by    *;
;*  the Free Software Foundation; either version 2 of the License, or       *;
;*  (at your option) any later version.                                     *;
;*                                                                          *;
;*  This program is distributed in the hope that it will be useful,         *;
;*  but WITHOUT ANY WARRANTY; without even the implied warranty of          *;
;*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *;
;*  General Public License for more details.                                *;
;*                                                                          *;
;*  You should have received a copy of the GNU General Public License       *;
;*  along with this program; if not, write to the Free Software             *;
;*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.               *;
;* ======================================================================== *;
;*                   Copyright (c) 2003, Joseph Zbiciak                     *;
;* ======================================================================== *;
