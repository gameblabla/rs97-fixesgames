;;==========================================================================;;
;; Intellicart Bank Switch Utility Routines                                 ;;
;; Copyright 2000, Joe Zbiciak.                                             ;;
;;                                                                          ;;
;; This file contains a number of useful routines that you're welcome       ;;
;; to use in your own software.  Please keep in mind that these routines    ;;
;; are licensed under the GNU General Public License, and so if you plan    ;;
;; to distribute a program which incorporates these routines, it too must   ;;
;; be distributed under the GNU General Public License.                     ;;
;;==========================================================================;;

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
;*                   Copyright (c) 2000, Joseph Zbiciak                     *;
;* ======================================================================== *;


;;==========================================================================;;
;;  IC_BANK2REG  -- Converts a bank address to an Intellicart reg address.  ;;
;;                                                                          ;;
;;  Inputs:                                                                 ;;
;;    R1 -- 5 MSBs of bank address in bits 7..3                             ;;
;;    R5 -- Return address                                                  ;;
;;                                                                          ;;
;;  Outputs:                                                                ;;
;;    R1 -- Address of Intellicart base control register.                   ;;
;;    R5 -- Trashed.                                                        ;;
;;                                                                          ;;
;;==========================================================================;;
IC_BANK2REG PROC

            PSHR    R5                  ; Save return address

            MOVR    R1,     R5          ; Get Hi/Lo 2K bit from address in R5
            ADDR    R5,     R5
            ANDI    #$10,   R5

            SLR     R1,     2           ; Put 4 MSBs of page in 4 LSBs of R0
            SLR     R1,     2
            ADDR    R5,     R1          ; Merge bits: 76543210 -> xxxx37654
            ADDI    #$40,   R1          ; Point it at Intellicart ctrl regs.

            PULR    PC                  ; Return.
            ENDP

;;==========================================================================;;
;; IC_SETBANK    -- Sets a bank for a given 2K page of memory.              ;;
;;                                                                          ;;
;;  Inputs:                                                                 ;;
;;    R1 -- 5 MSBs of bank address in bits 7..3                             ;;
;;    R2 -- New bank address to point to in 8 LSBs.                         ;;
;;    R5 -- Return address                                                  ;;
;;                                                                          ;;
;;  Outputs:                                                                ;;
;;    R1 -- Address of Intellicart base control register.                   ;;
;;    R2 -- Trashed.                                                        ;;
;;    R5 -- Trashed.                                                        ;;
;;                                                                          ;;
;;  Example:                                                                ;;
;;    To remap 0x7800-0x7FFF in the Inty's address space to point to        ;;
;;    0x4000-0x47FF in the cart's address space, pass in the following      ;;
;;    parameters:  R1 = 0x0078, R2 = 0x0040.                                ;;
;;                                                                          ;;
;;==========================================================================;;
IC_SETBANK  PROC
            PSHR    R5                  ; Save return address.
            XORR    R1,     R2          ; Calculate remap value.
            CALL    IC_BANK2REG         ; Convert bank to ctrl reg address
            MVO@    R2,     R1          ; Write to control register.
            PULR    PC                  ; Return
            ENDP
