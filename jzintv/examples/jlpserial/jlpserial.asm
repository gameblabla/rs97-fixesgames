;; ======================================================================== ;;
;;  Very Simple Serial Driver for JLP                                       ;;
;; ======================================================================== ;;

JLP_UART        PROC
@@rx            EQU     $F10
@@tx            EQU     $F11
@@baud          EQU     $F12
                ENDP

;; ======================================================================== ;;
;;  JLP_XMIT    -- Send byte in R0                                          ;;
;; ======================================================================== ;;
JLP_XMIT        PROC

                CLRR    R1

@@wait_tx:      CMP     JLP_UART.tx,    R1
                BNEQ    @@wait_tx

                MVO     R0,    JLP_UART.tx
                JR      R5
                ENDP

;; ======================================================================== ;;
;;  JLP_RECV    -- Receive byte in R0.  Status in R1.                       ;;
;; ======================================================================== ;;
JLP_RECV        PROC
                CLRR    R0
@@recv_loop     ADD     JLP_UART.rx,    R0
                BEQ     @@recv_loop

                JR      R5
                ENDP


;; ======================================================================== ;;
;;  JLP_SERINIT -- Init / reset the serial port.  Sets it to 115200 8-N-1.  ;;
;; ======================================================================== ;;
JLP_SERINIT     PROC
                MVII    #115200 / 100,  R0
                ; fallthru into JLP_SETBAUD
                ENDP

;; ======================================================================== ;;
;;  JLP_SETBAUD  -- Set the baud rate for the serial port.                  ;;
;;                  R0 is desired baud rate divided by 100.                 ;;
;; ======================================================================== ;;
JLP_SETBAUD     PROC
                MVO     R0, JLP_UART.baud
                JR      R5
                ENDP
