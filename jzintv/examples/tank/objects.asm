;; ======================================================================== ;;
;;  OBJECTS -- implements code to display the tanks and bullets.            ;;
;;                                                                          ;;
;;  This code implements all the movement, animation and physics for the    ;;
;;  tanks and bullets.                                                      ;;
;;                                                                          ;;
;;  This code isn't as clean as it could be.  For instance, MOVE_TANK0      ;;
;;  and MOVE_TANK1 could be one function with tank number as an argument    ;;
;;  if I were perhaps a little more careful.  (An indexed indirect          ;;
;;  addressing mode would also be quite helpful for that as well.)          ;;
;;                                                                          ;;
;;  This code also has too many "manifest constants" that should be broken  ;;
;;  out to equates or something.                                            ;;
;;                                                                          ;;
;; ------------------------------------------------------------------------ ;;
;;  GLOBAL VARIABLES REQUIRED BY THIS MODULE                                ;;
;;                                                                          ;;
;;  Name        Width   Purpose                                             ;;
;;  TNKx_VEL    8-bit   Desired velocity of tank * 8.  -2, 0 or 2.          ;;
;;  TNKx_MOV    8-bit   Actual movement of tank * 8.  -2, -1, 0, 1, or 2.   ;;
;;  TNKx_AVL    8-bit   Angular velocity of tank's gun turret.  -1, 0, or 1 ;;
;;  TNKx_ANG    8-bit   Angle of tank's turret divided by 10.  0 thru 18.   ;;
;;  TNKx_POW    8-bit   Firing strength configured for tank.                ;;
;;  TNKx_PTS    16-bit  Number of points scored by tank (up to 65535).      ;;
;;  TNKx_FLP    8-bit   Flag:  Tank now faces opposite direction.           ;;
;;  DO_STATS    8-bit   Flag:  If non-zero, update stats next frame.        ;;
;;  B0_TICK     8-bit   Count:  Ignore bullet 0 collisions while != 0.      ;;
;;  B1_TICK     8-bit   Count:  Ignore bullet 1 collisions while != 0.      ;;
;;  AVL_TICK    8-bit   Count:  Apply angular vel. to turrets every N ticks ;;
;;  BULx_XVL    16-bit  X velocity (8.8) of bullet                          ;;
;;  BULx_YVL    16-bit  Y velocity (8.8) of bullet                          ;;
;;  MOB_DIS     16-bit  MOB collision table dispatch pointer (from MOB_LL)  ;;
;;  MOB_IGN     16-bit  MOB collision ignore table pointer (from MOB_LL)    ;;
;;                                                                          ;;
;; ------------------------------------------------------------------------ ;;
;;  EXTERNAL STRUCTURES REFERENCED BY THIS MODULE                           ;;
;;                                                                          ;;
;;      MOB_PIC     MOB picture table (from MOB_LL)                         ;;
;;      MOB_ATR     MOB attribute table (from MOB_LL)                       ;;
;;      MOB_XYP     MOB X/Y position table (from MOB_LL)                    ;;
;;      GFX.xxx     Graphics picture numbers associated with game elements  ;;
;;      ATR.xxx     MOB attribute numbers associated w/ game elements       ;;
;;      MNUM.xxx    MOB numbers assigned to individual game elements        ;;
;;      SINCOS      Sine/Cosine table for initial bullet velocity           ;;
;;      TURTBL      Turret picture-number lookup table                      ;;
;;      BULOFS      Bullet pixel offset by turret angle lookup table        ;;
;;      FACE        MOB attribute lookup for tank facing left/right         ;;
;;                                                                          ;;
;; ------------------------------------------------------------------------ ;;
;;  FUNCTIONS DEFINED IN THIS MODULE                                        ;;
;;                                                                          ;;
;;      INIT_TANK   Initialize the two tanks                                ;;
;;      MOVE_TANK0  Move tank 0 by the amount in R2                         ;;
;;      MOVE_TANK1  Move tank 1 by the amount in R2                         ;;
;;      MOVE_TUR    Move turret for tank in R0 by amount in R2              ;;
;;      UPD_TANKS   Do per-frame updates for tanks (movement etc.)          ;;
;;      UPD_BULLET  Do per-frame updates for bullets (movement etc.)        ;;
;;      HIT_BULLET  Handle bullet collisions                                ;;
;;      FIRE_BULLET Make a bullet airborne for a tank.                      ;;
;; ======================================================================== ;;


;; ======================================================================== ;;
;;  INIT_TANK -- Initializes the two tanks.                                 ;;
;; ======================================================================== ;;
INIT_TANK   PROC
            PSHR    R5

            MVII    #DISPATCH,  R0
            MVOD    R0,         MOB_DIS
            MVII    #IGNORE,    R0
            MVOD    R0,         MOB_IGN

            ; MOBs 0..2 are first tank

            ; tank body
            CALL    MOB_CREATE
            DECLE   MNUM.t0_body
            DECLE   pack(87, 10)
            DECLE   ATR.r_tbody
            DECLE   pack(GFX.tbody+1, GFX.tbody)

            ; tank turret
            CALL    MOB_CREATE
            DECLE   MNUM.t0_tur
            DECLE   pack(87, 10)
            DECLE   ATR.r_tur
            DECLE   pack(GFX.turbot, GFX.tur0)


            ; tank tread
            CALL    MOB_CREATE
            DECLE   MNUM.t0_tread
            DECLE   pack(87, 10)
            DECLE   ATR.r_tread
            DECLE   pack(GFX.none, GFX.tread1)




            ; MOBs 3..5 are first tank

            ; tank body
            CALL    MOB_CREATE
            DECLE   MNUM.t1_body
            DECLE   pack(87, 153)
            DECLE   ATR.l_tbody
            DECLE   pack(GFX.tbody+1, GFX.tbody)

            ; tank turret
            CALL    MOB_CREATE
            DECLE   MNUM.t1_tur
            DECLE   pack(87, 153)
            DECLE   ATR.l_tur
            DECLE   pack(GFX.turbot, GFX.tur0)


            ; tank tread
            CALL    MOB_CREATE
            DECLE   MNUM.t1_tread
            DECLE   pack(87, 153)
            DECLE   ATR.l_tread
            DECLE   pack(GFX.none, GFX.tread0)


            ;; Set up the other tank variables

            CLRR    R0
            MVO     R0,     TNK0_ANG        ; turret faces hard right
            MVO     R0,     TNK0_AVL        ; turret isn't moving
            MVO     R0,     TNK1_AVL        ; turret isn't moving

            MVII    #18,    R0
            MVO     R0,     TNK1_ANG        ; turret faces hard left

            MVII    #5,     R0
            MVO     R0,     TNK0_POW        ; initial firing strength of 5
            MVO     R0,     TNK1_POW        ; initial firing strength of 5

            PULR    PC
            ENDP


;; ======================================================================== ;;
;;  MOVE_TANK0 -- Move tank 0 left or right by amount in R2.                ;;
;; ======================================================================== ;;
MOVE_TANK0  PROC
            PSHR    R5

            CALL    MOB_MOVE_M.0            ; DX is already in R2
            DECLE   $0000                   ; make DY = 0.  
            DECLE   $07                     ; Tank #0 is MOBs 0..2

            MVI     XYPAddr(MNUM.t0, x_hi), R1  ; tank 0's X position
            CMPI    #8,         R1
            BGT     @@ok_left

            CLRR    R1
            MVO     R1,         TNK0_MOV    ; halt the tank

            CALL    MOB_GOTO_M              ; 
            DECLE   $08E0                   ; clamp left border, rounded up
            DECLE   87*256                  ; 
            DECLE   $07                     ; Tank #0 is MOBs 0..2

            B       @@ok_right

@@ok_left:  CMPI    #151,       R1
            BLT     @@ok_right

            CLRR    R1
            MVO     R1,         TNK0_MOV    ; halt the tank
            
            CALL    MOB_GOTO_M              ; 
            DECLE   $9700                   ; clamp exactly to right border
            DECLE   87*256                  ; 
            DECLE   $07                     ; Tank #0 is MOBs 0..2


@@ok_right:

            ; Compute tread alignment given final resting position of tank0
            MVI     XYPAddr(MNUM.t0, x_hi), R1  ; tank 0's X position
            MOD3    R1,         R3          ; R1 = R1 MOD 3, using R0 as temp

            ; figure out what way we're facing so we can negate the MOD

            MVI     AtrAddr(MNUM.t0_tread), R2   ; tank 0's tread attribute
            CMPI    #ATR.r_tread,R2         ; is it right-facing?
            BEQ     @@right                 ; yes:  Don't flip
            NEGR    R1                      ; \   no:  Flip in such a way
            BEQ     @@ok                    ;  |- that the flipped tread
            ADDI    #3,         R1          ; /   aligns to the non-flipped
@@ok:
@@right:
            ; store new picture number for tread, one of tread0 thru tread2.
            ADDI    #GFX.tread0+$80, R1
            MVO     R1,         PicAddr(MNUM.t0_tread, top) 

            PULR    R5
            B       FLG_STICSH

            ENDP

;; ======================================================================== ;;
;;  MOVE_TANK1 -- Move tank 1 left or right by amount in R2.                ;;
;; ======================================================================== ;;
MOVE_TANK1  PROC
            PSHR    R5

            CALL    MOB_MOVE_M.0            ; DX is already in R2
            DECLE   $0000                   ; make DY = 0.  
            DECLE   $38                     ; Tank #1 is MOBs 3..5

            MVI     XYPAddr(MNUM.t1, x_hi), R1  ; tank 1's X position
            CMPI    #159,       R1
            BLT     @@ok_right

            CLRR    R1
            MVO     R1,         TNK1_MOV    ; halt the tank

            CALL    MOB_GOTO_M              ; 
            DECLE   159*256                 ; clamp exactly to right border
            DECLE   87*256                  ; 
            DECLE   $38                     ; Tank #1 is MOBs 3..5

            B       @@ok_left 

@@ok_right: CMPI    #16,        R1
            BGT     @@ok_left 
            
            CLRR    R1
            MVO     R1,         TNK1_MOV    ; halt the tank

            CALL    MOB_GOTO_M              ; 
            DECLE   $10E0                   ; clamp exactly to left border
            DECLE   87*256                  ; 
            DECLE   $38                     ; Tank #1 is MOBs 3..5

@@ok_left: 

            ; Compute tread alignment given final resting position of tank1
            MVI     XYPAddr(MNUM.t1, x_hi), R1  ; tank 1's X position
            MOD3    R1,         R3          ; R1 = R1 MOD 3, using R0 as temp

            ; figure out what way we're facing so we can negate the MOD

            MVI     AtrAddr(MNUM.t1_tread), R2   ; tank 1's tread attribute
            CMPI    #ATR.r_tread,R2         ; is it right-facing?
            BEQ     @@right                 ; yes:  Don't flip
            NEGR    R1                      ; \   no:  Flip in such a way
            BEQ     @@ok                    ;  |- that the flipped tread
            ADDI    #3,         R1          ; /   aligns to the non-flipped
@@ok:
@@right:
            ; store new picture number for tread, one of tread0 thru tread2.
            ADDI    #GFX.tread0+$80, R1
            MVO     R1,         PicAddr(MNUM.t1_tread, top) 

            PULR    R5
            B       FLG_STICSH

            ENDP

;; ======================================================================== ;;
;;  MOVE_TUR -- Move a turret CW/CCW by amount in R2.  Tank # in R0.        ;;
;; ======================================================================== ;;
MOVE_TUR    PROC
            PSHR    R5
            
            MVII    #1,         R1
            MVO     R1,         DO_STATS

            MVII    #TNK0_ANG,  R1          ;\__ Index to requested tank
            ADDR    R0,         R1          ;/

            NEGR    R2                      ; CW is negative angle
            ADD@    R1,         R2          ; Add turret rotation to current
            BGE     @@ok_lo                 ; don't let it go negative
            CLRR    R2                      ; clamp to 0
@@ok_lo:    CMPI    #18,        R2          ; don't let it go above 180
            BLE     @@ok_hi                 ;
            MVII    #18,        R2          ; clamp to 180

@@ok_hi:    MVO@    R2,         R1          ; store new turret position

            MVII    #AtrAddr(MNUM.t0), R5   ; might need to face tank other way
            MVII    #PicAddr(MNUM.t0_tur,top), R3
            TSTR    R0
            BEQ     @@tank0
            MVII    #AtrAddr(MNUM.t1), R5   ; might need to face tank other way
            MVII    #PicAddr(MNUM.t1_tur,top), R3
@@tank0:

            ADDI    #TURTBL,    R2          ; Look up required turret angle
            MVI@    R2,         R2      
            SARC    R2,         2
            MVO@    R2,         R3          ; store turret picture # for tank

            BOV     @@no_face
            MVII    #FACE.lft,  R4
            BC      @@face_left
            MVII    #FACE.rgt,  R4
@@face_left:

            REPEAT  3
            MVI@    R4,         R1
            MVO@    R1,         R5
            ENDR
            
            MVII    #1,         R1
            MVII    #TNK0_FLP,  R2
            ADDR    R0,         R2
            MVO@    R1,         R2
           
            PULR    R5 
            B       FLG_STICSH

@@no_face:  PULR    PC
            ENDP


;; ======================================================================== ;;
;;  UPD_TANKS -- Per-frame updates for tanks.                               ;;
;; ======================================================================== ;;
UPD_TANKS   PROC
            PSHR    R5

            ;; ------------------------------------------------------------ ;;
            ;;  If the two tanks collide this tick, then the actual vel     ;;
            ;;  of both is the average of the two requested velocities.     ;;
            ;;  Otherwise, the velocity of each is the requested velocity.  ;;
            ;; ------------------------------------------------------------ ;;
            MVI     TNK0_VEL,   R0              ; get tank 0's desired vel
            MVI     TNK1_VEL,   R1              ; get tank 1's desired vel

            MVII    #XYPAddr(MNUM.t1, x_lo), R4 ;\
            SDBD                                ; |
            MVI@    R4,         R2              ; |_ Find the distance between
            MVII    #XYPAddr(MNUM.t0, x_lo), R4 ; |  the two tanks.
            SDBD                                ; |
            SUB@    R4,         R2              ;/

            CMPI    #$0801,     R2              ; Are they in contact?
            BC      @@no_hit

            ; only do this if they're moving towards each other.  this isn't
            ; a towing service.

            SWAP    R0                          ; |-- skip out if tank 0 
            TSTR    R0
            BMI     @@no_hit0                   ;/    is moving left
            SWAP    R1                          ; |-- skip out if tank 1 
            TSTR    R1
            BEQ     @@hit                       ; |   is moving right
            BPL     @@no_hit1                   ;/ 
@@hit:

            ADDR    R1,         R0              ;\ 
            SAR     R0,         1               ; |  They're in contact, so
            SWAP    R0,         1               ; |_ each has an actual vel.
            MVO     R0,         TNK0_MOV        ; |  that's the average of the
            MVO     R0,         TNK1_MOV        ; |  two desired velocities.
            B       @@done_vel                  ;/   


@@no_hit1:  SWAP    R1
@@no_hit0:  SWAP    R0
@@no_hit:   MVO     R0,         TNK0_MOV
            MVO     R1,         TNK1_MOV
@@done_vel:


            ;; ------------------------------------------------------------ ;;
            ;;  Only do turret updates every few frames.                    ;;
            ;; ------------------------------------------------------------ ;;
            MVI     AVL_TICK,   R0
            DECR    R0
            BPL     @@no_tur_0

            ;; ------------------------------------------------------------ ;;
            ;;  Update Tank 0's turret position based on angular velocity.  ;;
            ;; ------------------------------------------------------------ ;;
            MVI     TNK0_AVL,   R2
            DECR    R2
            BMI     @@no_tur_0
            BNEQ    @@tur_0_cw
            DECR    R2
@@tur_0_cw: CLRR    R0 
            CALL    MOVE_TUR
@@no_tur_0:
            ;; ------------------------------------------------------------ ;;
            ;;  Update Tank 0's position based on its velocity.             ;;
            ;; ------------------------------------------------------------ ;;
            MVI     TNK0_MOV,   R2
            SWAP    R2
            CMP     TNK0_FLP,   R2  ; forces 'MOVE' call if tank flips l/r
            BEQ     @@no_move_0
            MVO     R2,         TNK0_FLP
            SAR     R2,         2
            SAR     R2,         1
            CALL    MOVE_TANK0
@@no_move_0:

            MVI     AVL_TICK,   R0
            DECR    R0
            BPL     @@no_tur_1

            ;; ------------------------------------------------------------ ;;
            ;;  Update Tank 1's turret position based on angular velocity.  ;;
            ;; ------------------------------------------------------------ ;;
            MVI     TNK1_AVL,   R2
            DECR    R2
            BMI     @@no_tur_1
            BNEQ    @@tur_1_cw
            DECR    R2
@@tur_1_cw: MVII    #1,         R0
            CALL    MOVE_TUR
@@no_tur_1:

            ;; ------------------------------------------------------------ ;;
            ;;  Update Tank 1's position based on its velocity.             ;;
            ;; ------------------------------------------------------------ ;;
            MVI     TNK1_MOV,   R2
            SWAP    R2
            CMP     TNK1_FLP,   R2  ; forces 'MOVE' call if tank flips l/r
            BEQ     @@no_move_1
            MVO     R2,         TNK1_FLP
            SAR     R2,         2
            SAR     R2,         1
            CALL    MOVE_TANK1
@@no_move_1:

            MVI     AVL_TICK,   R0
            DECR    R0
            BPL     @@tick_ok
            MVII    #3,         R0
@@tick_ok:  MVO     R0,         AVL_TICK


            PULR    PC
            ENDP


;; ======================================================================== ;;
;;  UPD_BULLET -- Update bullet positions, etc.                             ;;
;; ======================================================================== ;;
UPD_BULLET  PROC
            PSHR    R5

            ;; ------------------------------------------------------------ ;;
            ;;  Update the "bullet no-hit" timers.                          ;;
            ;; ------------------------------------------------------------ ;;
            MVI     B0_TICK,    R0
            DECR    R0
            BMI     @@b0_live
            MVO     R0,         B0_TICK
@@b0_live:  
            MVI     B1_TICK,    R0
            DECR    R0
            BMI     @@b1_live
            MVO     R0,         B1_TICK
@@b1_live:
            ;; ------------------------------------------------------------ ;;
            ;;  Update bullet 0.                                            ;;
            ;; ------------------------------------------------------------ ;;
            MVI     AtrAddr(MNUM.b0),R0     ; Take a look at bullet 0's MOB
            CMPI    #ATR.bullet,R0          ; Is this (still) a bullet?
            BNEQ    @@done_b0               ; no? skip it.

            ; make sure bullet 0 doesn't go below ground level.  If it's
            ; on the ground, just leave it there
            MVI     XYPAddr(MNUM.b0,y_hi), R0
            CMPI    #88,        R0
            BGE     @@done_b0

            ; accelerate bullet 0 by gravity and then move it.
            MVI     BUL0_YVL,   R3          ;\
            ADDI    #GRAVITY,   R3          ; |- Accelerate it by gravity
            CMPI    #$00FF,     R3          ;/
            BLE     @@b0v_ok                ;
            MVII    #$00FF,     R3          ; clamp y velocity to $00.FF
@@b0v_ok:   
            MVO     R3,         BUL0_YVL    ; store updated y velocity
            MVI     BUL0_XVL,   R2          ; get bullet 0's x velocity
            CALL    MOB_MOVE.1              ; move bullet 0
            DECLE   MNUM.b0

            MVI     XYPAddr(MNUM.b0,x_hi), R0   ;\
            CMPI    #1,         R0              ; |_ destroy bullet if it
            BLT     @@kill_b0                   ; |  goes offscreen 
            CMPI    #159,       R0              ;/
            BLE     @@done_b0

@@kill_b0:  CALL    MOB_DESTROY
            DECLE   MNUM.b0

@@done_b0:

            ;; ------------------------------------------------------------ ;;
            ;;  Update bullet 1.                                            ;;
            ;; ------------------------------------------------------------ ;;
            MVI     AtrAddr(MNUM.b1),R0     ; Take a look at bullet 1's MOB
            CMPI    #ATR.bullet,R0          ; Is this (still) a bullet?
            BNEQ    @@done_b1               ; no? skip it.

            ; make sure bullet 1 doesn't go below ground level.  If it's
            ; on the ground, just leave it there
            MVI     XYPAddr(MNUM.b1,y_hi), R0
            CMPI    #88,        R0
            BGE     @@done_b1

            ; accelerate bullet 1 by gravity and then move it.
            MVI     BUL1_YVL,   R3          ;\
            ADDI    #GRAVITY,   R3          ; |- Accelerate it by gravity
            CMPI    #$00FF,     R3          ;/
            BLE     @@b1v_ok                ;
            MVII    #$00FF,     R3          ; clamp y velocity to $00.FF
@@b1v_ok:   
            MVO     R3,         BUL1_YVL    ; store updated y velocity
            MVI     BUL1_XVL,   R2          ; get bullet 1's x velocity
            CALL    MOB_MOVE.1              ; move bullet 1
            DECLE   MNUM.b1

            MVI     XYPAddr(MNUM.b1,x_hi), R0   ;\
            CMPI    #1,         R0              ; |_ destroy bullet if it
            BLT     @@kill_b1                   ; |  goes offscreen 
            CMPI    #159,       R0              ;/
            BLE     @@done_b1

@@kill_b1:  CALL    MOB_DESTROY
            DECLE   MNUM.b1

@@done_b1:

            ;; ------------------------------------------------------------ ;;
            ;;  Update bullet 0's explosion, if it's exploding.             ;;
            ;; ------------------------------------------------------------ ;;
            MVI     PicAddr(MNUM.b0,top),R0 ; Take a look at bullet 0's MOB

            CMPI    #GFX.exp0,  R0          ;\
            BLT     @@done_e0               ; |_ is it one of the explosion
            CMPI    #GFX.exp7,  R0          ; |  frames?
            BGT     @@done_e0               ;/

            BNEQ    @@next_e0               ; Is it one of the first 7?

            CALL    MOB_DESTROY             ;\__ take down the animation
            DECLE   MNUM.b0                 ;/   on its last frame
            B       @@done_e0
@@next_e0:
            INCR    R0                      ;\
            ADDI    #$80,       R0          ; |- advance the explosion anim 
            MVO     R0, PicAddr(MNUM.b0,top);/   by one frame. 
@@done_e0:

            ;; ------------------------------------------------------------ ;;
            ;;  Update bullet 1's explosion, if it's exploding.             ;;
            ;; ------------------------------------------------------------ ;;
            MVI     PicAddr(MNUM.b1,top),R0 ; Take a look at bullet 1's MOB

            CMPI    #GFX.exp0,  R0          ;\
            BLT     @@done_e1               ; |_ is it one of the explosion
            CMPI    #GFX.exp7,  R0          ; |  frames?
            BGT     @@done_e1               ;/

            BNEQ    @@next_e1               ; Is it one of the first 7?

            CALL    MOB_DESTROY             ;\__ take down the animation
            DECLE   MNUM.b1                 ;/   on its last frame
            B       @@done_e1
@@next_e1:
            INCR    R0                      ;\
            ADDI    #$80,       R0          ; |- advance the explosion anim 
            MVO     R0, PicAddr(MNUM.b1,top);/   by one frame. 
@@done_e1:
            PULR    R5
            B       FLG_STICSH
            ENDP


;; ======================================================================== ;;
;;  HIT_BULLET -- Handle bullet collisions w/ tanks, ground, walls.         ;;
;; ======================================================================== ;;
HIT_BULLET  PROC
@@0:        PSHR    R5

            ;; ------------------------------------------------------------ ;;
            ;;  Filter out collisions during first tick or so of bullet's   ;;
            ;;  life, since it might strike the firing tank itself on the   ;;
            ;;  way out if the tank is moving.  We want to ignore that.     ;;
            ;; ------------------------------------------------------------ ;;
            CLRR    R0
            CMP     B0_TICK,    R0
            BNEQ    @@leave

            ;; ------------------------------------------------------------ ;;
            ;;  Convert bullets into explosions.                            ;;
            ;; ------------------------------------------------------------ ;;
            MVI     PicAddr(MNUM.b0, top), R0   ;\_ don't destroy explosions
            CMPI    #GFX.bullet, R0             ;/
            BNEQ    @@already_exploding

            ; Make this an explosion.
            MVII    #GFX.exp0,  R0
            MVO     R0,         PicAddr(MNUM.b0, top)
            MVII    #ATR.explode, R0
            MVO     R0,         AtrAddr(MNUM.b0)

            ; Slide it over 3 pixels to center it
            MVI     XYPAddr(MNUM.b0, x_hi), R0
            ADDI    #3,         R0
            MVO     R0, XYPAddr(MNUM.b0, x_hi)

            CLRR    R0
            MVO     R0,         BUL0_XVL
            MVO     R0,         BUL0_YVL
            B       @@already_exploding





@@1:        PSHR    R5

            ;; ------------------------------------------------------------ ;;
            ;;  Filter out collisions during first tick or so of bullet's   ;;
            ;;  life, since it might strike the firing tank itself on the   ;;
            ;;  way out if the tank is moving.  We want to ignore that.     ;;
            ;; ------------------------------------------------------------ ;;
            CLRR    R0
            CMP     B1_TICK,    R0
            BNEQ    @@leave

            ;; ------------------------------------------------------------ ;;
            ;;  Convert bullets into explosions.                            ;;
            ;; ------------------------------------------------------------ ;;
            MVI     PicAddr(MNUM.b1, top), R0   ;\_ don't destroy explosions
            CMPI    #GFX.bullet, R0             ;/
            BNEQ    @@already_exploding

            ; Make this an explosion.
            MVII    #GFX.exp0,  R0
            MVO     R0,         PicAddr(MNUM.b1, top)
            MVII    #ATR.explode, R0
            MVO     R0,         AtrAddr(MNUM.b1)

            ; Slide it over 4 pixels to center it
            MVI     XYPAddr(MNUM.b1, x_hi), R0
            ADDI    #4,         R0
            MVO     R0, XYPAddr(MNUM.b1, x_hi)

            CLRR    R0
            MVO     R0,         BUL1_XVL
            MVO     R0,         BUL1_YVL

@@already_exploding:

            ;; ------------------------------------------------------------ ;;
            ;;  Check to see whether this bullet/explosion hit one or both  ;;
            ;;  tanks.  For every frame that an explosion hits one tank,    ;;
            ;;  the other tank gets a point.                                ;;
            ;; ------------------------------------------------------------ ;;
            MOVR    R2,         R3              ; save explosion mask

            ANDI    #$07,       R2              ; did we hit tank 0?
            BEQ     @@no_tank_0

            MVI     TNK1_PTS,   R0
            INCR    R0
            BEQ     @@no_tank_0                 ; prevent overflow
            MVO     R0,         TNK1_PTS
            MVII    #1,         R1
            MVO     R1,         DO_STATS

@@no_tank_0:

            ANDI    #$38,       R3              ; did we hit tank 0?
            BEQ     @@no_tank_1

            MVI     TNK0_PTS,   R0  
            INCR    R0
            BEQ     @@no_tank_1                 ; prevent overflow
            MVO     R0,         TNK0_PTS

            MVII    #1,         R1
            MVO     R1,         DO_STATS

@@no_tank_1:

@@leave:    PULR    PC
            ENDP

;; ======================================================================== ;;
;;  FIRE_BULLET -- Fires a bullet for the tank in R0.                       ;;
;; ======================================================================== ;;
FIRE_BULLET PROC
            PSHR    R5

            ;; ------------------------------------------------------------ ;;
            ;;  Check to see if this tank already has a bullet in the air.  ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #AtrAddr(MNUM.b0),R1;\_ Point to this tank's bullet
            ADDR    R0,         R1      ;/  attribute

            MVI@    R1,         R1      ; get the attribute for this bullet
            TSTR    R1                  ;\__ if it's non-zero, it's either
            BNEQ    @@leave             ;/   a bullet or explosion.  Leave.

            MVII    #4,         R3      ; Initial value for "no-hit" timer
            MVII    #ATR.l_tur, R1      ; will need to determine tank's dir.

            TSTR    R0                  ;\__ load parameters for appropriate
            BEQ     @@t0                ;/   tank

@@t1:       MVO     R3,         B1_TICK         ; set bullet 1's no-hit timer
            MVI     TNK1_ANG,   R3              ; get tank 1's firing angle
            CMP     AtrAddr(MNUM.t1_tur),   R1  ;\   add 1 if facing left
            BNEQ    @@t1_rgt                    ; |- to distinguish 90 deg
            INCR    R3                          ;/   left from 90 deg right
@@t1_rgt:   MVII    #XYPAddr(MNUM.t1,x_hi), R1  ; point R1 to tank 1's X coord
            B       @@tdone

@@t0:       MVO     R3,         B0_TICK         ; set bullet 0's no-hit timer
            MVI     TNK0_ANG,   R3              ; get tank 0's firing angle
            CMP     AtrAddr(MNUM.t0_tur),   R1  ;\   add 1 if facing left
            BNEQ    @@t0_rgt                    ; |- to distinguish 90 deg
            INCR    R3                          ;/   left from 90 deg right
@@t0_rgt:   MVII    #XYPAddr(MNUM.t0,x_hi), R1  ; point R1 to tank 0's X coord

@@tdone:

            ;; ------------------------------------------------------------ ;;
            ;;  Compute the initial position for the bullet based on tank's ;;
            ;;  position and turret angle.  R3 has turret angle.            ;;
            ;; ------------------------------------------------------------ ;;

            MVI@    R1,         R2      ; get tank's X position into lower byte
            ADDI    #87*256,    R2      ; Set Y coordinate to base of tank
            ADDI    #BULOFS,    R3      ; point to packed X/Y offset for bullet
            SUB@    R3,         R2      ; move the bullet up/left as necessary

            PSHR    R0                  ; save the tank #

            ;; ------------------------------------------------------------ ;;
            ;;  Create the MOB for the bullet.                              ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #MNUM.b0,   R1      ;\__ Pick the bullet based on tank#
            ADDR    R0,         R1      ;/

            CALL    MOB_CREATE.2                ;\
            DECLE   ATR.bullet                  ; |- make the bullet
            DECLE   pack(GFX.none, GFX.bullet)  ;/

            PULR    R0                  ; restore tank # from stack

            ;; ------------------------------------------------------------ ;;
            ;;  Setup the X/Y velocity for the bullet.  We use these eqs:   ;;
            ;;    x_vel = (fire_power + 2) * cos(angle) / 4 + tank_x_vel    ;;
            ;;    y_vel = (fire_power + 2) * sin(angle) / 4                 ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #BUL0_XVL,  R4      ;\
            ADDR    R0,         R4      ; |- point to the correct bullet's
            ADDR    R0,         R4      ;/   velocity

            MVII    #SINCOS,    R5      ;\
            MVII    #TNK0_ANG,  R2      ; |
            ADDR    R0,         R2      ; |- Index into Sine/Cosine table
            ADD@    R2,         R5      ; |  based on this tank's turret angle
            ADD@    R2,         R5      ;/
            
            ADDI    #TNK0_POW-TNK0_ANG, R2  ;\__ Get tank's firing power 
            MVI@    R2,         R1          ;/   into R1.
            ADDI    #2,         R1          ; Add 2, since '1' is pretty wimpy

            SUBI    #TNK0_POW-TNK0_MOV, R2  ;\
            MVI@    R2,         R2          ; |_ Initialize bullet velocity
            SWAP    R2                      ; |  to this tank's horizontal
            SAR     R2,         1           ;/   velocity * 4.

            CLRR    R3                  ; initialize vertical vel to 0.

@@mul_loop  ; Multiply sine/cosine by tank's firing power.
            ADD@    R5,         R2      ; Add cosine to horizontal
            ADD@    R5,         R3      ; Add sine to vertical
            SUBI    #2,         R5      ; keep pointing to to same sine/cosine
            DECR    R1                  ;\__ multiply iteratively
            BNEQ    @@mul_loop          ;/

            SAR     R2,         2       ; Divide X velocity by 4
            SAR     R3,         2       ; Divide Y velocity by 4

            MVO@    R2,         R4      ; store X velocity
            MVO@    R3,         R4      ; store Y velocity

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
