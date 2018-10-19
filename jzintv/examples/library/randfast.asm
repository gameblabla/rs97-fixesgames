;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;
;; ======================================================================== ;;
;;  RANDFAST                                                                ;;
;;      Returns random bits in R0.                                          ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;      R5 -- Return address                                                ;;
;;      Random state in RSEED                                               ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0 -- 16 random bits.                                               ;;
;;      R1 -- Set to $AB19                                                  ;;
;;      R2..R4 -- Unmodified                                                ;;
;;                                                                          ;;
;;  NOTES:                                                                  ;;
;;      You are encouraged to add additional "randomness" by adding or      ;;
;;      XORing other values into RSEED.                                     ;;
;;                                                                          ;;
;;      Implementation is a Galois realization of a 16-bit LFSR with the    ;;
;;      following polynomial:                                               ;;
;;                                                                          ;;
;;          x^16 = x^15 + x^13 + x^11 + x^9 + x^8 + x^4 + x^3 + x^0         ;;
;;                                                                          ;;
;;      The random number generator is only advanced by 4 bits per call.    ;;
;;      Call twice if you need 8 truly random bits.  The generator uses     ;;
;;      a dense polynomial, so the top 12 bits will be different than the   ;;
;;      bottom 12 bits returned on the previous call 15 out of 16 times.    ;;
;; ======================================================================== ;;
RANDFAST    PROC
            MVI     RSEED,      R0      ;  10   Get rand seed
            XORR    R5,         R0      ;   8   XOR in caller ret address
            MVII    #$AB19,     R1      ;   8   Field polynomial
                                        ;----
                                        ;  26

            REPEAT  4
            SLLC    R0,         1       ;   6   Multiply by x^1
            ADCR    PC                  ;   7   \_ If x^16 generated, XOR in
            XORR    R1,         R0      ;   6   /  polynomial (-ve logic)
                                        ;----
                                        ;  19
            ENDR
                                        ;  76  (4 unrolled iterations)
                                        ;  26  (carried forward)
                                        ;----
                                        ; 102
        
            MVO     R0,     RSEED       ;  11
            JR      R5                  ;   7
                                        ;----
                                        ;  18
                                        ; 102
                                        ;----
                                        ; 130
            ENDP

;; ======================================================================== ;;
;;  End of File:  randfast.asm                                              ;;
;; ======================================================================== ;;
