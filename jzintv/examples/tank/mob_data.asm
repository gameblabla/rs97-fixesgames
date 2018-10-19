;; ======================================================================== ;;
;;  DISPATCH -- dispatch table for MOB collision detection.                 ;;
;; ======================================================================== ;;
DISPATCH    PROC
            DECLE   STUB
            DECLE   STUB
            DECLE   STUB      
            DECLE   STUB
            DECLE   STUB
            DECLE   STUB      
            DECLE   HIT_BULLET.0
            DECLE   HIT_BULLET.1
            ENDP

;; ======================================================================== ;;
;;  IGNORE -- collision ignore table.  These collisions never dispatch.     ;;
;; ======================================================================== ;;
IGNORE      PROC
            DECLE   0,0,0       ; no direct dispatching for Tank 0
            DECLE   0,0,0       ; no direct dispatching for Tank 1
            DECLE   $3FF        ;\__ bullets can collide with tank 0, tank 1
            DECLE   $3FF        ;/   border, each other and background cards.
            ENDP


;; ======================================================================== ;;
;;  GRAVITY -- Acceration due to gravity.                                   ;;
;; ======================================================================== ;;
GRAVITY     EQU     $0002       


;; ======================================================================== ;;
;;  MNUM -- MOB number assignments for tank 0 and tank 1.                   ;;
;; ======================================================================== ;;
MNUM        PROC
@@t0        EQU     0
@@t0_body   EQU     2
@@t0_tur    EQU     1
@@t0_tread  EQU     0

@@t1        EQU     3
@@t1_body   EQU     5
@@t1_tur    EQU     4
@@t1_tread  EQU     3

@@b0        EQU     6
@@b1        EQU     7
            ENDP


;; ======================================================================== ;;
;;  TURTBL   -- Turret angle table.  Also, left/right flag in bits 0, 1     ;;
;; ======================================================================== ;;
TURTBL      PROC
@@na        EQU     2
@@lft       EQU     1
@@rgt       EQU     0

@@tur0      EQU     ($80 + GFX.tur0) * 4
@@tur1      EQU     ($80 + GFX.tur1) * 4
@@tur2      EQU     ($80 + GFX.tur2) * 4
@@tur3      EQU     ($80 + GFX.tur3) * 4
@@tur4      EQU     ($80 + GFX.tur4) * 4
@@tur5      EQU     ($80 + GFX.tur5) * 4
@@tur6      EQU     ($80 + GFX.tur6) * 4

            DECLE   @@tur0 + @@na           ;   0 degrees
            DECLE   @@tur1 + @@na           ;  10 degrees
            DECLE   @@tur2 + @@na           ;  20 degrees
            DECLE   @@tur3 + @@na           ;  30 degrees
            DECLE   @@tur3 + @@na           ;  40 degrees
            DECLE   @@tur4 + @@na           ;  50 degrees
            DECLE   @@tur4 + @@na           ;  60 degrees
            DECLE   @@tur5 + @@na           ;  70 degrees
            DECLE   @@tur5 + @@rgt          ;  80 degrees
            DECLE   @@tur6 + @@na           ;  90 degrees
            DECLE   @@tur5 + @@lft          ; 100 degrees
            DECLE   @@tur5 + @@na           ; 110 degrees
            DECLE   @@tur4 + @@na           ; 120 degrees
            DECLE   @@tur4 + @@na           ; 130 degrees
            DECLE   @@tur3 + @@na           ; 140 degrees
            DECLE   @@tur3 + @@na           ; 150 degrees
            DECLE   @@tur2 + @@na           ; 160 degrees
            DECLE   @@tur1 + @@na           ; 170 degrees
            DECLE   @@tur0 + @@na           ; 180 degrees

            ENDP

;; ======================================================================== ;;
;;  FACE -- Attribute records for a tank facing left or right.              ;;
;; ======================================================================== ;;
FACE        PROC
@@lft       DECLE   ATR.l_tread, ATR.l_tur, ATR.l_tbody
@@rgt       DECLE   ATR.r_tread, ATR.r_tur, ATR.r_tbody
            ENDP

;; ======================================================================== ;;
;;  SINCOS -- Sine/Cosine values for initial bullet velocity.               ;;
;; ======================================================================== ;;
SINCOS      PROC

            DECLE   $0058,  $0000
            DECLE   $0056,  $FFF1
            DECLE   $0053,  $FFE2
            DECLE   $004C,  $FFD4
            DECLE   $0043,  $FFC8
            DECLE   $0038,  $FFBD
            DECLE   $002C,  $FFB4
            DECLE   $001E,  $FFAD
            DECLE   $000F,  $FFAA
            DECLE   $0000,  $FFA8
            DECLE   $FFF1,  $FFAA
            DECLE   $FFE2,  $FFAD
            DECLE   $FFD4,  $FFB4
            DECLE   $FFC8,  $FFBD
            DECLE   $FFBD,  $FFC8
            DECLE   $FFB4,  $FFD4
            DECLE   $FFAD,  $FFE2
            DECLE   $FFAA,  $FFF1
            DECLE   $FFA8,  $0000

            ENDP

;; ======================================================================== ;;
;;  BULOFS -- X/Y offsets for bullet position based on turret angle.        ;;
;; ======================================================================== ;;
BULOFS      PROC

            ; offsets for right-facing tanks
            DECLE   pack($04, $00)      ; turret pos 0;   0 degrees
            DECLE   pack($04, $00)      ; turret pos 1;  10 degrees
            DECLE   pack($05, $00)      ; turret pos 2;  20 degrees
            DECLE   pack($06, $01)      ; turret pos 3;  30 degrees
            DECLE   pack($06, $01)      ; turret pos 3;  40 degrees
            DECLE   pack($07, $02)      ; turret pos 4;  50 degrees
            DECLE   pack($07, $02)      ; turret pos 4;  60 degrees
            DECLE   pack($07, $03)      ; turret pos 5;  70 degrees
            DECLE   pack($07, $03)      ; turret pos 5;  80 degrees
            DECLE   pack($07, $04)      ; turret pos 6;  90 degrees
                                 
            ; offsets for left-facing tanks
            DECLE   pack($07, $03)      ; turret pos 6;  90 degrees
            DECLE   pack($07, $03)      ; turret pos 5; 100 degrees
            DECLE   pack($07, $03)      ; turret pos 5; 110 degrees
            DECLE   pack($07, $04)      ; turret pos 4; 120 degrees
            DECLE   pack($07, $04)      ; turret pos 4; 130 degrees
            DECLE   pack($06, $05)      ; turret pos 3; 140 degrees
            DECLE   pack($06, $05)      ; turret pos 3; 150 degrees
            DECLE   pack($05, $06)      ; turret pos 2; 160 degrees
            DECLE   pack($04, $06)      ; turret pos 1; 170 degrees
            DECLE   pack($04, $06)      ; turret pos 0; 180 degrees
            ENDP
