;* ======================================================================== *;
;*     This file is hereby placed into the public domain by its author,     *;
;*     Joseph Zbiciak.  It may therefore be incorporated into programs      *;
;*     with any license with no restrictions on the resulting program.      *;
;* ======================================================================== *;

;; ======================================================================== ;;
;;  INITMEM                                                                 ;;
;;                                                                          ;;
;;  Initialize a set of variables to initial values.  Initialization        ;;
;;  records are 1 word each typically.  All variables are in the range      ;;
;;  $0F0 - $35F.  So, each record is formatted as follows.                  ;;
;;                                                                          ;;
;;        15                 10  9                                0         ;;
;;       +---------------------+-----------------------------------+        ;;
;;       |       Value         |             Address               |        ;;
;;       +---------------------+-----------------------------------+        ;;
;;                                                                          ;;
;;  A record of 0 terminates the initializer list.                          ;;
;;                                                                          ;;
;;  The "value" field is actually encoded.  $20 - $3F directly map to       ;;
;;  the constants $00 - $1F.  $00 - $1E map to constants in a separate      ;;
;;  constants table.  $1F indicates the constant appears in the following   ;;
;;  word.  Furthermore, constant table entries $00 - $1F are packed 8-bit   ;;
;;  contants, whereas $20 - $2E are unpacked 16-bit constants.              ;;
;;                                                                          ;;
;;  INPUTS  (INITMEM.0)                                                     ;;
;;      R5  Initializer table.  Returns following table.                    ;;
;;                                                                          ;;
;;  INPUTS  (INITMEM.1)                                                     ;;
;;      R5  Pointer to initializer table.  Returns following pointer.       ;;
;;                                                                          ;;
;;  INPUTS  (INITMEM.2)                                                     ;;
;;      R5  Return address                                                  ;;
;;      R4  Initialization table                                            ;;
;;                                                                          ;;
;; ======================================================================== ;;

INITMEM     PROC
@@0:        MOVR    R5,     R4          ; Read init table from after CALL
            CLRR    R5                  ; Flag:  Return to R4.
            INCR    PC                  ; Skip MVI@
@@1:        MVI@    R5,     R4          ; Read init table from elsewhere
@@2:    


@@next:
            ;; ------------------------------------------------------------ ;;
            ;;  Decode next initializer symbol.  R1 will hold the 10-bit    ;;
            ;;  variable address and R2 holds the encoded constant value.   ;;
            ;; ------------------------------------------------------------ ;;
            MVI@    R4,     R2          ; Get next record
            MOVR    R2,     R1          ; Save for factoring addr from cst

            BEQ     @@done              ; Terminate on zero
            ANDI    #$3FF,  R1          ; Factor address from data
            XORR    R1,     R2          ; Clear address from data
            SWAP    R2                  ; \_ put data in 6 LSBs
            SLR     R2,     2           ; /

            SUBI    #$20,   R2          ; $20-$3F are cst $00-$1F.  
            BMI     @@cst_tbl           ; $00-$1F indicate "constant tbl"

            MVO@    R2,     R1          ; \_ Store decoded constant 
            B       @@next              ; /  and move to next initializer

            ;; ------------------------------------------------------------ ;;
            ;;  If the encoded value was $00 - $1F, then we have one of     ;;
            ;;  the following:                                              ;;
            ;;      $00 - $0F   8 bit constant in packed 8-bit table        ;;
            ;;      $10 - $1E   16 bit constant in 16-bit table             ;;
            ;;      $1F         Escape code for arbitrary constant.         ;;
            ;; ------------------------------------------------------------ ;;
@@cst_tbl   INCR    R2                  ; \_ $1F specifies an escape
            BEQ     @@escape            ; /

            ADDI    #$F,    R2          ; \_ $00-$0F specify 8 bit cst
            BMI     @@8bit              ; /  $10-$1E specify 16 bit cst

            ;; ------------------------------------------------------------ ;;
            ;;  R2 now $00-$0E for $10-$1E input.  Use it to index into     ;;
            ;;  16-bit constants table.                                     ;;
            ;; ------------------------------------------------------------ ;;
            ADDI    #CST16, R2
            MVI@    R2,     R0          ; \_ Copy constant of interest out
            MVO@    R0,     R1          ; /  
            B       @@next

@@escape    MVI@    R4,     R2          ; \_ Escape:  Copy next word to
            MVO@    R2,     R1          ; /  indicated address
            B       @@next

@@8bit:
            ;; ------------------------------------------------------------ ;;
            ;;  R2 is now -$10 to -$01 to specify one of the 16 8-bit csts. ;;
            ;;  We add CST8*2 to the index and then right-shift by 1 to     ;;
            ;;  determine the integer index.  The C bit will tell us hi or  ;;
            ;;  lo byte.  We bias the result up by $20 since $00-$1F don't  ;;
            ;;  require the constant table.                                 ;;
            ;; ------------------------------------------------------------ ;;
            ADDI    #$FFFF AND (CST8*2+16), R2      ; 
    IF (DEFINED CST8) AND ((CST8 AND $8000) = 0)
            CLRC                        ; MSB of CST8 was 0
    ELSE
_cst8_fix   SETC                        ; MSB of CST8 was 1
    ENDI
            RRC     R2                  ; Divide index by 2
            MVI@    R2,     R0          ; Get pair of 8 bit values
            ADCR    PC                  ; If idx%2 == 1, take lo half
            SWAP    R0                  ; else take hi half
            ANDI    #$FF,   R0          ; Clear away upper byte
            ADDI    #$20,   R0          ; 
            MVO@    R0,     R1          ; Store it out
            B       @@next

@@done      TSTR    R5                  ; Return after data?
            BEQ     @@jrr4              ; If R5==0, return after data.
            JR      R5                  ; Else return to R5.
@@jrr4      JR      R4                  
            ENDP

;; ======================================================================== ;;
;;  End of File:  initmem.asm                                               ;;
;; ======================================================================== ;;
