;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;;==========================================================================;;
;; Intellicart Bank Switch Utility Routines                                 ;;
;;==========================================================================;;

;; ======================================================================== ;;
;;  IC_BANK2REG  -- Converts a bank address to an Intellicart reg address.  ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;    R1 -- 5 MSBs of bank address in bits 7..3                             ;;
;;    R5 -- Return address                                                  ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;    R1 -- Address of Intellicart base control register.                   ;;
;;    R5 -- Trashed.                                                        ;;
;;                                                                          ;;
;; ======================================================================== ;;
IC_BANK2REG PROC
            PSHR    R5                  ; Save return address

            MOVR    R1,     R5          ; Get Hi/Lo 2K bit from address in R5
            ADDR    R5,     R5
            ANDI    #$10,   R5

            SLR     R1,     2           ; Put 4 MSBs of bank in 4 LSBs of R0
            SLR     R1,     2
            ANDI    #$0F,   R1          ; Keep only the four LSBs.
            ADDR    R5,     R1          ; Merge bits: 76543210 -> xxxx37654
            ADDI    #$40,   R1          ; Point it at Intellicart ctrl regs.

            PULR    PC                  ; Return.
            ENDP

;; ======================================================================== ;;
;;  IC_SETBANK    -- Sets a bank for a given 2K bank of memory.             ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;    R1 -- 5 MSBs of bank address in bits 7..3                             ;;
;;    R2 -- New bank address to point to in 8 LSBs.                         ;;
;;    R5 -- Return address                                                  ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;    R1 -- Address of Intellicart base control register.                   ;;
;;    R2 -- Trashed.                                                        ;;
;;    R5 -- Trashed.                                                        ;;
;;                                                                          ;;
;;  EXAMPLE:                                                                ;;
;;    To remap 0x7800-0x7FFF in the Inty's address space to point to        ;;
;;    0x4000-0x47FF in the cart's address space, pass in the following      ;;
;;    parameters:  R1 = 0x0078, R2 = 0x0040.                                ;;
;;                                                                          ;;
;; ======================================================================== ;;
IC_SETBANK  PROC
            PSHR    R5                  ; Save return address.
            CALL    IC_BANK2REG         ; Convert bank to ctrl reg address
            MVO@    R2,     R1          ; Write to control register.
            PULR    PC                  ; Return
            ENDP

;; ======================================================================== ;;
;;  IC_SETFBANK   -- Finer-granularity bankswitching.                       ;;
;;                                                                          ;;
;;  NOTES:                                                                  ;;
;;    This routine is intended for bank-switched pages that do not start    ;;
;;    on a 2K boundary, but rather on a 256-word "fine mapping" boundary.   ;;
;;    The program should pass in the upper 8 bits of the starting address   ;;
;;    for the bank-switched area, and the upper 8 bits of where that bank   ;;
;;    should point in the Intellicart memory map.                           ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;    R1 -- 5 MSBs of bank address in bits 7..3                             ;;
;;    R2 -- New bank address to point to in 8 LSBs.                         ;;
;;    R5 -- Return address                                                  ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;    R1 -- Address of Intellicart base control register.                   ;;
;;    R2 -- Trashed.                                                        ;;
;;    R5 -- Trashed.                                                        ;;
;;                                                                          ;;
;;  EXAMPLE:                                                                ;;
;;    To remap 0x0E00-0x0FFF in the Inty's address space to point to        ;;
;;    0x4300-0x44FF in the cart's address space, pass in the following      ;;
;;    parameters:  R1 = 0x000E, R2 = 0x0043.                                ;;
;;                                                                          ;;
;; ======================================================================== ;;
IC_SETFBANK PROC
            PSHR    R5                  ; Save return address.
            PSHR    R1                  ; Save R1 temporarily
            ANDI    #$07,   R1          ;\__ Adjust bank address to cope w/
            SUBR    R1,     R2          ;/   fine mapping.
            PULR    R1                  ; Restore R1.
            ANDI    #$F8,   R1          ;
            CALL    IC_BANK2REG         ; Convert bank to ctrl reg address
            MVO@    R2,     R1          ; Write to control register.
            PULR    PC                  ; Return
            ENDP

;; ======================================================================== ;;
;;  IC_SETBANKS   -- Sets the bank mapping for a range of memory.           ;;
;;                                                                          ;;
;;  INPUTS:                                                                 ;;
;;    R0 -- High address of range (8 MSBs only)                             ;;
;;    R1 -- Low address of range (8 MSBs only)                              ;;
;;    R2 -- Starting address to point Low address at. (8 MSBs only)         ;;
;;    R5 -- Return address                                                  ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;    R1 -- Address of Intellicart base control register.                   ;;
;;    R2 -- Trashed.                                                        ;;
;;    R5 -- Trashed.                                                        ;;
;;                                                                          ;;
;;  EXAMPLE:                                                                ;;
;;    To remap 0x7800-0x7FFF in the Inty's address space to point to        ;;
;;    0x4000-0x47FF in the cart's address space, pass in the following      ;;
;;    parameters:  R1 = 0x0078, R2 = 0x0040.                                ;;
;; ======================================================================== ;;
IC_SETBANKS PROC
            PSHR    R5

@@loop:
            PSHR    R1
            PSHR    R2

            CALL    IC_SETBANK

            PULR    R2
            PULR    R1

            ADDI    #8,     R1
            ADDI    #8,     R2
            CMPR    R1,     R0
            BGE     @@loop

            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  IC_BANKTBL   -- Resets the bank mappings for this cartridge using a     ;;
;;                  table of initial mappings.                              ;;
;;                                                                          ;;
;;  Inputs:                                                                 ;;
;;    R4 -- Pointer to bank initialization table (format below)             ;;
;;    R5 -- Return address                                                  ;;
;;                                                                          ;;
;;  Outputs:                                                                ;;
;;    R4 -- Points just past end of table.                                  ;;
;;    R5 -- Trashed.                                                        ;;
;;                                                                          ;;
;;  Table format:                                                           ;;
;;    The table is laid out as a series of byte pairs packed into 16-bit    ;;
;;    words.  The current format for each word is like so:                  ;;
;;                                                                          ;;
;;       15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0     ;;
;;      +---+---+---+---+---+---+---+---+-------------------------------+   ;;
;;      |    BANK NUMBER    |n/a|n/a|n/a| INTELLICART BANK ADDR MAPPING |   ;; 
;;      +---+---+---+---+---+---+---+---+-------------------------------+   ;;
;;                                                                          ;;
;;    To end the table, use a word filled with all zeros.                   ;;
;;                                                                          ;;
;; ======================================================================== ;;
IC_BANKTBL  PROC
            PSHR    R5
            PSHR    R2
            PSHR    R1

            B       @@1st_iter
@@loop:
            CALL    IC_BANK2REG
            MVO@    R2,     R1 
@@1st_iter:
            MVI@    R4,     R2
            MOVR    R2,     R1
            SWAP    R1,     1
            BNEQ    @@loop

            PULR    R1
            PULR    R2
            PULR    PC
            ENDP

;; ======================================================================== ;;
;;  End of File:  ic_banksw.asm                                             ;;
;; ======================================================================== ;;
