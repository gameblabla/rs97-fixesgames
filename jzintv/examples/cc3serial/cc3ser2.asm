;; ======================================================================== ;;
;;  An attempt at a more complex serial driver                              ;;
;;                                                                          ;;
;;  This driver requires a pool of RAM to hold TX and RX buffers, as well   ;;
;;  as associated pointers, etc.                                            ;;
;;                                                                          ;;
;;  Requires "cart.mac" macros for declaring storage.                       ;;
;; ======================================================================== ;;

             
;; ------------------------------------------------------------------------ ;;
;;  UART0 Registers                                                         ;;
;; ------------------------------------------------------------------------ ;;
CC3U0           PROC
@@dll           EQU     $0F00       ; Divisor Latch LSB
@@dlm           EQU     $0F01       ; Divisor Latch MSB
             
@@thr           EQU     $0F00       ; Transmit Holding Register
@@rbr           EQU     $0F00       ; Receive Buffer Register
             
@@fcr           EQU     $0F02       ; FIFO Control Registers
             
@@lsr           EQU     $0F03       ; Line Status Register
@@lcr           EQU     $0F04       ; Line Control Register
             
@@ter           EQU     $0F05       ; Transmit Enable Register

@@PCLK          EQU     15000000/16
                ENDP


;; ------------------------------------------------------------------------ ;;
;;  U0FCR Fields                                                            ;;
;; ------------------------------------------------------------------------ ;;
U0FCR           PROC
@@tx_fifo_rst   EQU     $0004       ; TX FIFO Reset
@@rx_fifo_rst   EQU     $0002       ; RX FIFO Reset
@@fifo_enable   EQU     $0001       ; FIFO enable
                ENDP


;; ------------------------------------------------------------------------ ;;
;;  U0LCR Fields                                                            ;;
;; ------------------------------------------------------------------------ ;;
U0LCR           PROC
@@dlab          EQU     $0080       ; Enable access to divisor latches
@@break         EQU     $0040       ; Send break
@@par_n         EQU     $0000       ; No parity
@@par_o         EQU     $0008       ; Odd parity
@@par_e         EQU     $0018       ; Even parity
@@par_0         EQU     $0028       ; '0' parity
@@par_1         EQU     $0038       ; '1' parity
@@stop_1        EQU     $0000       ; 1 stop bit
@@stop_2        EQU     $0004       ; 2 stop bits
@@char_5        EQU     $0000       ; 5 bit characters
@@char_6        EQU     $0001       ; 6 bit characters
@@char_7        EQU     $0002       ; 7 bit characters
@@char_8        EQU     $0003       ; 8 bit characters

@@8n1           EQU     @@char_8 OR @@par_n OR @@stop_1
@@7e1           EQU     @@char_7 OR @@par_e OR @@stop_1
@@7o1           EQU     @@char_7 OR @@par_o OR @@stop_1
                ENDP

;; ------------------------------------------------------------------------ ;;
;;  U0LSR Fields                                                            ;;
;; ------------------------------------------------------------------------ ;;
U0LSR           PROC
@@rdr           EQU     $0001       ; Receive Data Ready
@@oe            EQU     $0002       ; Overrun Error
@@pe            EQU     $0004       ; Parity Error
@@fe            EQU     $0008       ; Framing Error
@@bi            EQU     $0010       ; Break Interrupt (we've received a break)
@@thre          EQU     $0020       ; Transmit Holding Register Empty
@@temt          EQU     $0040       ; Transmitter EMpTy
@@rxfe          EQU     $0080       ; Error in RX FIFO
                ENDP


;; ------------------------------------------------------------------------ ;;
;;  U0TER Field                                                             ;;
;; ------------------------------------------------------------------------ ;;
U0TER           PROC
@@txen          EQU     $0080       ; Transmit enable   
                ENDP

;; ------------------------------------------------------------------------ ;;
;;  Baud rates                                                              ;;
;; ------------------------------------------------------------------------ ;;
MACRO           cc3_baud x 
                DECLE   %x% / 2, ((CC3U0.PCLK + (%x%/2)) / %x%)
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
;;  Transmit and receive queues                                             ;;
;; ======================================================================== ;;

    IF (DEFINED CC3_RXQ_DEPTH) = 0
CC3_RXQ_DEPTH   EQU         128             ; must be power of 2 
    ENDI
    IF (DEFINED CC3_TXQ_DEPTH) = 0
CC3_TXQ_DEPTH   EQU         128             ; must be power of 2 
    ENDI
                WORDVAR     CC3_RXQ_WR
                WORDVAR     CC3_RXQ_RD
                WORDARRAY   CC3_RXQ,    CC3_RXQ_DEPTH

                WORDVAR     CC3_TXQ_WR
                WORDVAR     CC3_TXQ_RD
                BYTEARRAY   CC3_TXQ,    CC3_TXQ_DEPTH

                BYTEVAR     CC3_RXBUSY
                BYTEVAR     CC3_TXBUSY

CC3_RXQ_END     EQU         CC3_RXQ + CC3_RXQ_DEPTH
CC3_TXQ_END     EQU         CC3_TXQ + CC3_TXQ_DEPTH

;; ======================================================================== ;;
;;  CC3_PUMP_RX  -- Receive as much as we can.                              ;;
;; ======================================================================== ;;
CC3_PUMP_RX     PROC
                ;; -------------------------------------------------------- ;;
                ;;  Lock:  Protect against reentrancy.  Allows PUMP_RX to   ;;
                ;;  get called w/in an interrupt and outside an interrupt.  ;;
                ;; -------------------------------------------------------- ;;

                ;; Set our busy flag. 
                MVII    #$8E,       R0
                MVI     CC3_RXBUSY, R1 
                ; interrupt here will let other copy run and return here: OK
                MVO     R0, CC3_RXBUSY      ; Attempt to acquire lock.
                                            ; It may already be acquired.

                TSTR    R1                  ; Leave:  We were already busy.
                BNEQ    @@busy

                ;; -------------------------------------------------------- ;;
                ;;  Receive code.                                           ;;
                ;; -------------------------------------------------------- ;;
                MVI     CC3_RXQ_WR, R4
                MVI     CC3_RXQ_RD, R3
                SUBR    R4,         R3
                ADDI    #CC3_RXQ_DEPTH,  R3 ; Room remaining in rx q
                BEQ     @@rx_skip

                ANDI    #CC3_RXQ_DEPTH-1,R4 ; \_ R4 points into rx q
                ADDI    #CC3_RXQ,   R4      ; /

                PSHR    R3                  ; remember init words remaining

                ;; Receive as many bytes as we can
@@rx_loop:      MVI     CC3U0.lsr,  R0      ; \
                SARC    R0,         1       ;  |- check rx data ready
                BNC     @@rx_done           ; /

                SWAP    R0                  ; \_ merge received data
                XOR     CC3U0.rbr,  R0      ; /  with its flags

                MVO@    R0,             R4
                CMPI    #CC3_RXQ_END,   R4
                BNEQ    @@rxq_ok
                SUBI    #CC3_RXQ_DEPTH, R4
@@rxq_ok
                DECR    R3
                BNEQ    @@rx_loop

@@rx_done:      
                SUB@    SP,         R3      ; final - init = -received
                MVI     CC3_RXQ_WR, R4      ; \_ old WR pointer - (-received)
                SUBR    R3,         R4      ; /
                MVO     R4, CC3_RXQ_WR      ; Store queue write pointer
@@rx_skip:
                ;; -------------------------------------------------------- ;;
                ;;  Unlock:  Safe for other instances of CC3_PUMP_RX now.   ;;
                ;; -------------------------------------------------------- ;;
                CLRR    R0                  ; \_ CC3_PUMP_RX no longer busy.
                MVO     R0, CC3_RXBUSY      ; /  Remove lock.
@@busy:         JR      R5
                ENDP


;; ======================================================================== ;;
;;  CC3_PUMP_TX  -- Transmit as much as we can.                             ;;
;; ======================================================================== ;;
CC3_PUMP_TX     PROC
                ;; -------------------------------------------------------- ;;
                ;;  Lock:  Protect against reentrancy.  Allows PUMP_TX to   ;;
                ;;  get called w/in an interrupt and outside an interrupt.  ;;
                ;; -------------------------------------------------------- ;;

                ;; Set our busy flag. 
                MVII    #$8E,       R0
                MVI     CC3_TXBUSY, R1 
                ; interrupt here will let other copy run and return here: OK
                MVO     R0, CC3_TXBUSY      ; Attempt to acquire lock.
                                            ; It may already be acquired.

                TSTR    R1                  ; Leave:  We were already busy.
                BNEQ    @@busy

                ;; -------------------------------------------------------- ;;
                ;;  Transmit code.                                          ;;
                ;; -------------------------------------------------------- ;;
                MVI     CC3_TXQ_RD, R1
                B       @@tx_first

@@tx_loop:      MOVR    R1,         R2      ; Copy receive queue pointer
                INCR    R1                  ; Increment queue read pointer
                ANDI    #CC3_TXQ_DEPTH-1,R2 ; Circular addressing w/in queue
                ADDI    #CC3_TXQ,   R2      ; Index into transmit queue

                MVO     R1, CC3_TXQ_RD      ; \_ Atomically store read pointer
                MVI@    R2,         R0      ; /  and read byte to transmit

                MVO     R0, CC3U0.thr       ; Send the byte

@@tx_first:     CMP     CC3_TXQ_WR, R1      ; Anything more to send?
                BEQ     @@tx_done

                MVI     CC3U0.lsr,  R0
                ANDI    #U0LSR.thre SHR 1,R0; Can we send anything?
                BNEQ    @@tx_loop           ; Nope:  Leave
@@tx_done:      
                ;; -------------------------------------------------------- ;;
                ;;  Unlock:  Safe for other instances of CC3_PUMP_TX now.   ;;
                ;; -------------------------------------------------------- ;;
                CLRR    R0                  ; \_ CC3_PUMP_TX no longer busy.
                MVO     R0, CC3_TXBUSY      ; /  Remove lock.
@@busy:         JR      R5

                ENDP


;; ======================================================================== ;;
;;  CC3_XMIT     -- Send byte in R0, non-blocking                           ;;
;;                  Returns C=1 if queue is full.                           ;;
;; ======================================================================== ;;
CC3_XMIT        PROC
                MVI     CC3_TXQ_RD, R1
                MVI     CC3_TXQ_WR, R2
                SUBR    R2,         R1
                CMPI    #-CC3_TXQ_DEPTH, R2
                BEQ     @@full

                MOVR    R2,         R1
                INCR    R1
                ANDI    #CC3_TXQ_DEPTH-1,R2
                ADDI    #CC3_TXQ,   R2

                ; The following two updates are atomic wrt. to interrupts
                MVO@    R0,         R2      ; Insert byte into queue
                MVO     R1, CC3_TXQ_WR      ; Update write pointer

@@success:      CLRC
                JR      R5

@@full:         SETC
                JR      R5
                ENDP

;; ======================================================================== ;;
;;  CC3_XMITBLK  -- Send byte in R0, blocking.  Keeps trying to transmit    ;;
;;                  until it succeeds.  Calls CC3_SERPUMP if queue is full. ;;
;; ======================================================================== ;;
CC3_XMITBLK     PROC
                PSHR    R5

@@xmit_loop     CALL    CC3_XMIT
                BNC     @@xmit_done

                PSHR    R0
                CALL    CC3_PUMP_TX
                PULR    R0
                B       @@xmit_loop

@@xmit_done:    PULR    PC
                ENDP
                

;; ======================================================================== ;;
;;  CC3_RECV     -- Receive byte in R0.  Status in R1.  Non-blocking.       ;;
;;                  Returns C=1 if no character ready.                      ;;
;; ======================================================================== ;;
CC3_RECV        PROC
                MVI     CC3_RXQ_RD, R1
                CMP     CC3_RXQ_WR, R1
                BEQ     @@empty

                MOVR    R1,         R2
                INCR    R1
                ANDI    #CC3_RXQ_DEPTH-1, R2
                ADDI    #CC3_RXQ,   R2

                ; The following two instructions are atomic wrt to interrupts
                MVO     R1, CC3_RXQ_RD      ; Update read pointer
                MVI@    R2,         R0      ; Get word from receive queue

                MOVR    R0,         R1      ; Status in R1
                ANDI    #$FF,       R0      ; Data in R0
                SWAP    R1                  ; 
                ANDI    #$47,       R1      ; Leave only error stat in R1

@@success:      CLRC
                JR      R5

@@empty:        SETC
                JR      R5
                ENDP

;; ======================================================================== ;;
;;  CC3_RECVBLK  -- Receive byte in R0, blocking.  Keeps trying to receive  ;;
;;                  until it succeeds.  Calls CC3_SERPUMP unconditionally.  ;;
;; ======================================================================== ;;
CC3_RECVBLK     PROC
                PSHR    R5

;   MVI CC3_RXQ_WR, R1
;   SUB CC3_RXQ_RD, R1
;   SLL R1, 2
;   SLL R1, 1
;   ADDI #$87, R1
;   MVO R1, $2EE
                MVI     CC3_RXQ_WR, R1
                SUB     CC3_RXQ_RD, R1
                CMPI    #CC3_RXQ_DEPTH/2, R1
                BGT     @@no_pump

@@recv_loop     CALL    CC3_PUMP_RX
@@no_pump       CALL    CC3_RECV
                BC      @@recv_loop

@@recv_done:    PULR    PC
                ENDP

;; ======================================================================== ;;
;;  CC3_SERINIT -- Init / reset the serial port.  Sets it to 19200 8-N-1.   ;;
;; ======================================================================== ;;
CC3_SERINIT     PROC
               
                CLRR    R0
                MVO     R0, CC3_TXQ_RD
                MVO     R0, CC3_TXQ_WR
                NOP
                MVO     R0, CC3_RXQ_RD
                MVO     R0, CC3_RXQ_WR
                CLRR    R0
                MVO     R0, CC3_RXBUSY
                MVO     R0, CC3_TXBUSY
               
                MVII    #19200/2,   R0
;               B       CC3_SETBAUD
                ; fallthru
               
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
                MVO     R0,         CC3U0.lcr   ;/
                MVI@    R4,         R1          ; get baud rate from table
                MVO     R1,         CC3U0.dll   ;\
                SWAP    R1                      ; |- Set desired baud rate
                MVO     R1,         CC3U0.dlm   ;/
                MVII    #$03,       R1          ;\_ 8-N-1 format
                MVO     R1,         CC3U0.lcr   ;/
                MVII    #$07,       R1          ;\_ Enable, reset FIFOs
                MVO     R1,         CC3U0.fcr   ;/
                MVO     R0,         CC3U0.ter   ; Enable transmitter
               
                CLRC
                JR      R5
                ENDP
