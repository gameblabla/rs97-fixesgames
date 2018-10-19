;; ======================================================================== ;;
;;  Serial Driver for CC3                                                   ;;
;;  Slavish copy of C code from Chad Schell.                                ;;
;; ======================================================================== ;;


CC3         PROC
@@u0dll     EQU     $0F00
@@u0thr     EQU     $0F00
@@u0rbr     EQU     $0F00
@@u0dlm     EQU     $0F01
@@u0fcr     EQU     $0F02
@@u0lsr     EQU     $0F03
@@u0lcr     EQU     $0F04
@@u0ter     EQU     $0F05
@@PCLK      EQU     15000000/16
            ENDP
;; ======================================================================== ;;
;;  CC3_SERINIT -- Init / reset the serial port.  Sets it to 19200 8-N-1.   ;;
;; ======================================================================== ;;
CC3_SERINIT PROC

    IF 1
            MVII    #$80,       R0          ;\_ enable divisor access
            MVO     R0,         CC3.u0lcr   ;/
            CLRR    R1                      ;\
            MVO     R1,         CC3.u0dlm   ; |_ Set for 19200 baud
            MVII    #$31,       R1          ; |
            MVO     R1,         CC3.u0dll   ;/
            MVII    #$03,       R1          ;\_ 8-N-1 format
            MVO     R1,         CC3.u0lcr   ;/
            MVII    #$07,       R1          ;\_ Enable, reset FIFOs
            MVO     R1,         CC3.u0fcr   ;/
            MVO     R0,         CC3.u0ter   ; Enable transmitter
            JR      R5
    ELSE
            MVII    #$80,       R0          ;\_ enable divisor access
            MVO     R0,         CC3.u0lcr   ;/
            CLRR    R1                      ;\
            MVO     R1,         CC3.u0dlm   ; |_ Set for 4800 baud
            MVII    #$C4,       R1          ; |
            MVO     R1,         CC3.u0dll   ;/
            MVII    #$03,       R1          ;\_ 8-N-1 format
            MVO     R1,         CC3.u0lcr   ;/
            MVII    #$07,       R1          ;\_ Enable, reset FIFOs
            MVO     R1,         CC3.u0fcr   ;/
            MVO     R0,         CC3.u0ter   ; Enable transmitter
            JR      R5
    ENDI

            ENDP


;; ======================================================================== ;;
;;  CC3_XMIT    -- Send byte in R0                                          ;;
;; ======================================================================== ;;
CC3_XMIT    PROC
            PSHR    R5

@@wait_tx:
            MVI     CC3.u0lsr,  R5
            ANDI    #$20,       R5
            BEQ     @@wait_tx

            MVO     R0,         CC3.u0thr
            PULR    PC
            ENDP


;; ======================================================================== ;;
;;  CC3_RECV    -- Receive byte in R0.  Status in R1.                       ;;
;; ======================================================================== ;;
CC3_RECV    PROC
@@recv_loop MVI     CC3.u0lsr,  R1          ;\
            SARC    R1                      ; |- Wait for LSB to be 1
            BNC     @@recv_loop             ;/

            MVI     CC3.u0rbr,  R0          ; Read data
            ANDI    #$47,       R1          ; Leave only error status in R1

            JR      R5
            ENDP




;; ------------------------------------------------------------------------ ;;
;;  Baud rates                                                              ;;
;; ------------------------------------------------------------------------ ;;
MACRO           cc3_baud x 
                DECLE   %x% / 2, ((CC3.PCLK + (%x%/2)) / %x%)
ENDM
CC3_BAUD        PROC
                cc3_baud    115200
                cc3_baud    57600
                cc3_baud    38400
                cc3_baud    19200
                cc3_baud    9600
                cc3_baud    4800
                cc3_baud    2400
                cc3_baud    1200
                cc3_baud    600
                cc3_baud    300
                cc3_baud    110

@@rates         EQU         ($ - CC3_BAUD) / 2
                ENDP

;; ======================================================================== ;;
;;  CC3_SETBAUD  -- Set the baud rate for the serial port.                  ;;
;;                  R0 is desired baud rate divided by 2, up to 115200.     ;;
;;                  C=1 if baud rate is unsupported.                        ;;
;; ======================================================================== ;;
CC3_SETBAUD     PROC
                MVII    #CC3_BAUD,       R4
                MVII    #CC3_BAUD.rates, R1
               
@@loop:         CMP@    R4,         R0
                BEQ     @@got_it
                INCR    R4
                DECR    R1
                BNEQ    @@loop
               
                ;; Didn't find it.  Report error with C=1
                SETC
                JR      R5
@@got_it:      
                MVII    #$80,       R0          ;\_ enable divisor access
                MVO     R0,         CC3.u0lcr   ;/
                MVI@    R4,         R1          ; get baud rate from table
                MVO     R1,         CC3.u0dll   ;\
                SWAP    R1                      ; |- Set desired baud rate
                MVO     R1,         CC3.u0dlm   ;/
                MVII    #$03,       R1          ;\_ 8-N-1 format
                MVO     R1,         CC3.u0lcr   ;/
                MVII    #$07,       R1          ;\_ Enable, reset FIFOs
                MVO     R1,         CC3.u0fcr   ;/
                MVO     R0,         CC3.u0ter   ; Enable transmitter
               
                CLRC
                JR      R5
                ENDP
