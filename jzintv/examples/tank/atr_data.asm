;; ======================================================================== ;;
;;  ATR_DATA -- Attribute tables for all the MOBs.                          ;;
;; ======================================================================== ;;

ATR_DATA    PROC
@@dead      DECLE   0,0,0

            ;; ------------------------------------------------------------ ;;
            ;;  Tank facing right                                           ;;
            ;; ------------------------------------------------------------ ;;

            ;; body
@@r_tbody:  DECLE   STIC.mobx_visb + STIC.mobx_intr
            DECLE   STIC.moby_yres
            DECLE   STIC.moba_fg4   ; dark green

            ;; tread
@@r_tread:  DECLE   STIC.mobx_visb + STIC.mobx_intr
            DECLE   4               ; 8x8, offset 4 pixels down
            DECLE   STIC.moba_fg0   ; brown

            ;; turret
@@r_tur:    DECLE   STIC.mobx_visb + STIC.mobx_intr
            DECLE   STIC.moby_yres
            DECLE   STIC.moba_fg5   ; green

            ;; ------------------------------------------------------------ ;;
            ;;  Tank facing left                                            ;;
            ;; ------------------------------------------------------------ ;;
            ;; body
@@l_tbody:  DECLE   STIC.mobx_visb + STIC.mobx_intr
            DECLE   STIC.moby_yres + STIC.moby_xflip
            DECLE   STIC.moba_fg4   ; dark green

            ;; tread
@@l_tread:  DECLE   STIC.mobx_visb + STIC.mobx_intr
            DECLE   4              + STIC.moby_xflip  ; 8x8, ofs 4 pixels down
            DECLE   STIC.moba_fg0   ; brown

            ;; turret
@@l_tur:    DECLE   STIC.mobx_visb + STIC.mobx_intr
            DECLE   STIC.moby_yres + STIC.moby_xflip
            DECLE   STIC.moba_fg5   ; green

            ;; ------------------------------------------------------------ ;;
            ;;  Bullet                                                      ;;
            ;; ------------------------------------------------------------ ;;
@@bullet:   DECLE   STIC.mobx_visb + STIC.mobx_intr
            DECLE   STIC.moby_ysize2
            DECLE   STIC.moba_fgF  + STIC.moba_prio

            ;; ------------------------------------------------------------ ;;
            ;;  Explosion                                                   ;;
            ;; ------------------------------------------------------------ ;;
@@explode:  DECLE   STIC.mobx_visb + STIC.mobx_intr
            DECLE   STIC.moby_ysize2
            DECLE   STIC.moba_fg2  + STIC.moba_prio ; red


            ENDP

ATR         PROC
@@dead      EQU     (ATR_DATA.dead    - ATR_DATA)/3
@@r_tbody   EQU     (ATR_DATA.r_tbody - ATR_DATA)/3
@@r_tread   EQU     (ATR_DATA.r_tread - ATR_DATA)/3
@@r_tur     EQU     (ATR_DATA.r_tur   - ATR_DATA)/3
@@l_tbody   EQU     (ATR_DATA.l_tbody - ATR_DATA)/3
@@l_tread   EQU     (ATR_DATA.l_tread - ATR_DATA)/3
@@l_tur     EQU     (ATR_DATA.l_tur   - ATR_DATA)/3
@@bullet    EQU     (ATR_DATA.bullet  - ATR_DATA)/3
@@explode   EQU     (ATR_DATA.explode - ATR_DATA)/3
            ENDP

