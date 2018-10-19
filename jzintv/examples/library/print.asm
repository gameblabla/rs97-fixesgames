;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;; ======================================================================== ;;
;;  PRINT.xxx     Prints an ASCIIZ string.                                  ;;
;;                                                                          ;;
;;  PRINT.R       Ptr to string in R0, format in R1, location in R4.        ;;
;;  PRINT.S       String follows CALL.  Format in R1, location in R4.       ;;
;;  PRINT.LS      Location and string itself follows CALL.  Format in R1.   ;;
;;  PRINT.FLS     Format, location, and string itself follows CALL          ;;
;;  PRINT.P       Ptr to string follows CALL.  Format in R1, loc. in R4.    ;;
;;  PRINT.LP      Location and ptr to string follows CALL.  Format in R1.   ;;
;;  PRINT.FLP     Format, location, and ptr to string follows CALL          ;;
;;                                                                          ;;
;;  PRINT alone is an alias for PRINT.FLS.                                  ;;
;;                                                                          ;;
;;  INPUTS for PRINT.R:                                                     ;;
;;      R0 -- Pointer to ASCIIZ string                                      ;;
;;      R1 -- Screen format word                                            ;;
;;      R4 -- Pointer to display location                                   ;;
;;      R5 -- Return address                                                ;;
;;                                                                          ;;
;;  INPUTS for PRINT.S:                                                     ;;
;;      R1 -- Screen format word                                            ;;
;;      R4 -- Pointer to display location                                   ;;
;;      R5 -- Invocation record, followed by return code.                   ;;
;;              String                      n DECLEs (NUL terminated)       ;;
;;                                                                          ;;
;;  INPUTS for PRINT.LS:                                                    ;;
;;      R1 -- Screen format word                                            ;;
;;      R5 -- Invocation record, followed by return code.                   ;;
;;              Ptr to display location     1 DECLE                         ;;
;;              String                      n DECLEs (NUL terminated)       ;;
;;                                                                          ;;
;;  INPUTS for PRINT.FLS:                                                   ;;
;;      R5 -- Invocation record, followed by return code.                   ;;
;;              Screen format word          1 DECLE                         ;;
;;              Ptr to display location     1 DECLE                         ;;
;;              String                      n DECLEs (NUL terminated)       ;;
;;                                                                          ;;
;;  INPUTS for PRINT.P:                                                     ;;
;;      R1 -- Screen format word                                            ;;
;;      R4 -- Pointer to display location                                   ;;
;;      R5 -- Invocation record, followed by return code.                   ;;
;;              Pointer to ASCIIZ string    1 DECLE                         ;;
;;                                                                          ;;
;;  INPUTS for PRINT.LP:                                                    ;;
;;      R1 -- Screen format word                                            ;;
;;      R5 -- Invocation record, followed by return code.                   ;;
;;              Ptr to display location     1 DECLE                         ;;
;;              Pointer to ASCIIZ string    1 DECLE                         ;;
;;                                                                          ;;
;;  INPUTS for PRINT.FLP:                                                   ;;
;;      R5 -- Invocation record, followed by return code.                   ;;
;;              Screen format word          1 DECLE                         ;;
;;              Location                    1 DECLE                         ;;
;;              Pointer to ASCIIZ string    1 DECLE                         ;;
;;                                                                          ;;
;;  OUTPUTS:                                                                ;;
;;      R0 -- Trashed                                                       ;;
;;      R1 -- Same as screen format word, EXCEPT Bit 15 is modified.        ;;
;;      R4 -- Points to display location just after displayed string.       ;;
;;      R5 -- Points just past end of source string in memory.              ;;
;;      R2 and R3 are not modified.                                         ;;
;;                                                                          ;;
;;  NOTES                                                                   ;;
;;      ASCIIZ strings are ASCII strings that are terminated with a         ;;
;;      single NUL character.  Although it was originally intended for use  ;;
;;      with ASCII strings, this routine does allow for characters outside  ;;
;;      the normal range of ASCII characters as well.  This can be useful   ;;
;;      for displaying graphic characters within a string.                  ;;
;;                                                                          ;;
;;      The card # displayed for a given character in the string is given   ;;
;;      by the following formula:                                           ;;
;;                                                                          ;;
;;          (character_number - 32) + (format_word SHR 3) = card_number.    ;;
;;                                                                          ;;
;;      To display a single character from GRAM, for instance, just         ;;
;;      insert a word whose value is "GRAM_picture_number + 288" in your    ;;
;;      string.                                                             ;;
;;                                                                          ;;
;;      You can use PRINT to display entire strings of characters in        ;;
;;      alternate fonts (such as a font loaded in GRAM) as well.  To do     ;;
;;      this, merely add the appropriate offset to the format_word.         ;;
;;      The correct offset depends on what characters are in your font      ;;
;;      and where your font begins in GRAM.                                 ;;
;;                                                                          ;;
;;      For example, suppose your font only contains uppercase alphabetic   ;;
;;      characters, and is loaded in GRAM positions 10 through 35.  That    ;;
;;      is, the picture for 'A' is in GRAM position #10, and 'Z' is in      ;;
;;      GRAM position #35.  We want 'A' to map to GRAM position #10.        ;;
;;      GRAM position #10 is equivalent to card #266.  The ASCII value for  ;;
;;      'A' is 65.  Using the relationship given above, we have:            ;;
;;                                                                          ;;
;;          (character_number - 32) + (format_word SHR 3) = card_number.    ;;
;;          (        65       - 32) + (format_word SHR 3) = 266             ;;
;;                                                                          ;;
;;      Solving this, we get:                                               ;;
;;                                                                          ;;
;;          format_word SHR 3 = 233                                         ;;
;;          format_word       = 233 SHL 3                                   ;;
;;                                                                          ;;
;;      So, to display this particular GRAM font, we must add 233 SHL 3     ;;
;;      to our format word.  (We can still set other bits in the format     ;;
;;      word to control its color, etc.)                                    ;;
;; ======================================================================== ;;
PRINT   PROC
@@FLS:  MVI@    R5,     R1      ; Load format word from invoc. record
@@LS:   MVI@    R5,     R4      ; Load ptr to display loc from invoc record
        B       @@S             ; Continue w/ string pointer in R5.

@@FLP:  MVI@    R5,     R1      ; Load ptr to string from invoc. record
@@LP:   MVI@    R5,     R4      ; Load format word from invoc. record
@@P:    MVI@    R5,     R0      ; Load ptr to display loc from invoc record

@@R:    PSHR    R5              ; Save return address if string ptr is in R0
        MOVR    R0,     R5      ; Use auto-incr pointer for reading string
        SETC                    ; Flag:  C==1 means ret addr is on stack
        INCR    PC              ; Skip the CLRC.

@@S     CLRC                    ; Flag:  C==0 means return after string.
        SLL     R1,     1       ; \__ Hide the flag in MSB of screen fmt word
        RRC     R1,     1       ; /
        B       @@1st           ; Get first char of string
@@tloop:
        SUBI    #32,    R0      ; Shift ASCII range to charset
        SLL     R0,     2       ; Move it to position for BTAB word
        SLL     R0,     1
        ADDR    R1,     R0      ; Merge with color info
        MVO@    R0,     R4      ; Write to display
@@1st:  MVI@    R5,     R0      ; Get next character
        TSTR    R0              ; Is it NUL?
        BNEQ    @@tloop         ; --> No, keep copying then

        TSTR    R1              ; Is MSB set?                             
        BMI     @@pulr          ; Yes:  PULR PC to exit
        JR      R5              ; Return to R5 if flag was clear
@@pulr: PULR    PC              ; Return to saved address if flag was set.
        ENDP

;; ======================================================================== ;;
;;  End of File:  print.asm                                                 ;;
;; ======================================================================== ;;
