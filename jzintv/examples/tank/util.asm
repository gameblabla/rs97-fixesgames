
;; ======================================================================== ;;
;;  pack(h,l) -- Packs two 8 bit constants into a 16-bit constant.          ;;
;; ======================================================================== ;;
MACRO       pack(h,l)
            (((%h%) * 256) + (%l%))
ENDM

;; ======================================================================== ;;
;;  MOD3 -- Ugly macro that finds a number modulo 3 for a byte.             ;;
;; ======================================================================== ;;
MACRO       MOD3    r,  t

            MOVR    %r%,    %t%
            ANDI    #$CC,   %t%
            XORR    %t%,    %r%
            SLR     %t%,    2
            ADDR    %t%,    %r%

            MOVR    %r%,    %t%
            ANDI    #$70,   %t%
            ANDI    #$07,   %r%
            SLR     %t%,    2
            SLR     %t%,    2
            ADDR    %t%,    %r%

            MOVR    %r%,    %t%
            ANDI    #$03,   %r%
            SLR     %t%,    2
            ADDR    %t%,    %r%

            MOVR    %r%,    %t%
            ANDI    #$03,   %r%
            SLR     %t%,    2
            ADDR    %t%,    %r%
        
            CMPI    #3,     %r%
            BNEQ    $ + 3           ; clear register if == 3
            CLRR    %r%

ENDM

