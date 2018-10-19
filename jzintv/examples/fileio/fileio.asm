;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2010                                     *;
;* ======================================================================== *;

;; ======================================================================== ;;
;;  Joe Zbiciak's File I/O demo                                             ;;
;;  Copyright 2011, Joe Zbiciak, intvnut AT gmail DOT com                   ;;
;;                                                                          ;;
;;  This demo tests out jzIntv's Emu-Link File I/O API.  This API lets      ;;
;;  programs running within jzIntv access files in the filesystem.  For     ;;
;;  this demo to work, you must start jzintv with "--file-io=<dir>",        ;;
;;  where <dir> is the directory that jzIntv will allow this program to     ;;
;;  read and write files.                                                   ;;
;; ======================================================================== ;;

;; ======================================================================== ;;
;;  Set up the cartridge                                                    ;;
;; ======================================================================== ;;

            INCLUDE "../macro/cart.mac"
            ROMSETUP 16K, 2011, "Emu-Link File I/O Demo", MAIN, 32

;; ======================================================================== ;;
;;  VARIABLES                                                               ;;
;; ======================================================================== ;;
TSKQM       EQU         15
MAXTSK      EQU         4

            BYTEVAR     FD0
            BYTEVAR     FD1
            BYTEARRAY   NBUF, 6

            WORDARRAY   BUF0,   16
            WORDARRAY   BUF1,   16

            BYTEARRAY   COLSTK, 5               ; Color-stack shadow
BORDER      EQU         COLSTK + 4              ; Border-color shadow
            BYTEVAR     CSTK_FGBG

;; ======================================================================== ;;
;;  LIBRARY INCLUDES                                                        ;;
;; ======================================================================== ;;
            INCLUDE "../library/gimini.asm"
            INCLUDE "../library/fillmem.asm"
            INCLUDE "../library/print.asm"
            INCLUDE "a_prnum16.asm"
            INCLUDE "../library/memcpy.asm"
            INCLUDE "../library/wnk.asm"

            INCLUDE "../macro/util.mac"
            INCLUDE "../macro/stic.mac"
            INCLUDE "../macro/gfx.mac"
            INCLUDE "../macro/print.mac"
            INCLUDE "../macro/emu_link.mac"
            INCLUDE "../macro/el_fileio.mac"

            INCLUDE "../terminal/ansi.asm"
            INCLUDE "ascii.asm"

ISRRET      EQU     $1014

TEST1       STRING  "TEST1", 0
TEST2       STRING  "TEST2", 0
TEST3       STRING  "TEST3", 0

;; ======================================================================== ;;
;;  MAIN:   Where it all happens.                                           ;;
;; ======================================================================== ;;
MAIN        PROC
            ;; ------------------------------------------------------------ ;;
            ;;  Title screen and initialization                             ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #$25D,  R1      ;\
            MVII    #$102,  R4      ; |-- Clear all of memory
            CALL    FILLZERO        ;/

                                    ;     01234567890123456789
            PRINT_CSTK  1,  2, White,      ">>> SDK-1600 <<<"
            PRINT_CSTK  2,  6, White,          "Presents"
            PRINT_CSTK  6,  1, Yellow,    "Emu-Link  File I/O"
            PRINT_CSTK 10,  3, White,       "Copyright 2011"

            SETISR  INIT_ISR,   R0
            EIS

            CALL    WAITKEY
            CALL    CLRSCR
            CALL    A_INIT
            MVII    #1,     R0
            MVO     R0,     CSTK_FGBG

            ;; ------------------------------------------------------------ ;;
            ;;  Do we have an emulator that supports Emu-Link?              ;;
            ;; ------------------------------------------------------------ ;;
            EL_EMU_DETECT
            BNC     @@ok

            CALL    A_PUTS  
            STRING  "\033[H\033[33;41m\033[2J"
            STRING  "\033[6;5HNo emulator\033[7;6Hdetected!", 0

            DECR    PC
@@ok:
            ;; ------------------------------------------------------------ ;;
            ;;  Now walk through all the tests.                             ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS  
            STRING  "\033[H\033[37;44m\033[2JTap DISC after each test.\n\r", 0

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 1:  Open a file named "TEST1" and print its fd.        ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                            ;01234567890123456789
            STRING  "\033[37mCreate TEST1: ",0

            ELFI_OPEN TEST1, O_CREAT + O_TRUNC + O_WRONLY
            BC      @@fail

            MVO     R2,     FD0

            CALL    A_PUTS
            STRING  "\033[33mFD ", 0

            MVI     FD0,    R0
            MVII    #NBUF,  R4
            CALL    A_PRNUM16.l

            CALL    A_PUTBUF
            DECLE   NBUF

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 2:  Open a file named "TEST2" and print its fd.        ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mCreate TEST2: ",0

            ELFI_OPEN TEST2, O_CREAT + O_TRUNC + O_WRONLY
            BC      @@fail

            MVO     R2,     FD1

            CALL    A_PUTS
            STRING  "\033[33mFD ", 0

            MVI     FD1,    R0
            MVII    #NBUF,  R4
            CALL    A_PRNUM16.l

            CALL    A_PUTBUF
            DECLE   NBUF

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 3:  Write 16 bytes of RAM to TEST1.                    ;;
            ;; ------------------------------------------------------------ ;;

            MVII    #$1234,     R0
            MVII    #BUF0,      R4
            MVII    #16,        R1
@@t3l:      MVO@    R0,         R4      ; \
            ADDI    #$1111,     R0      ;  |- Fill BUF0 with interesting data
            DECR    R1                  ; /
            BNEQ    @@t3l

            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mWRITE=>TEST1: ", 0

            MVI     FD0,    R2
            ELFI_WRITE      BUF0,   16
            BC      @@fail

            CMPI    #16,    R1
            BEQ     @@t3ok

            MOVR    R1,     R0
            MVII    #NBUF,  R4
            CALL    A_PRNUM16.l

            CALL    A_PUTBUF
            DECLE   NBUF
            B       @@fail


@@t3ok
            CALL    A_PUTS
            STRING  "\033[33mOk!", 0

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 4:  Write 16 words of RAM to TEST2.                    ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mWRITE16=>TEST2: ", 0

            MVI     FD1,    R2
            ELFI_WRITE16    BUF0,   16
            BC      @@fail

            CMPI    #16,    R1
            BEQ     @@t4ok

            MOVR    R1,     R0
            MVII    #NBUF,  R4
            CALL    A_PRNUM16.l

            CALL    A_PUTBUF
            DECLE   NBUF
            B       @@fail

@@t4ok:
            CALL    A_PUTS
            STRING  "\033[33mOk!", 0

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 5:  Close TEST1.                                       ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mCLOSE TEST1: ", 0

            MVI     FD0,    R2
            ELFI_CLOSE
            BC      @@fail

            CALL    A_PUTS
            STRING  "\033[33mOk!", 0

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 6:  Close TEST2.                                       ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mCLOSE TEST2: ", 0

            MVI     FD1,    R2
            ELFI_CLOSE
            BC      @@fail

            CALL    A_PUTS
            STRING  "\033[33mOk!", 0

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 7:  Rename TEST1 to TEST3                              ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mTEST1->TEST3: ", 0

            ELFI_RENAME TEST1, TEST3
            BC      @@fail

            CALL    A_PUTS
            STRING  "\033[33mOk!", 0

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 8:  Open a file named "TEST3" and print its fd.        ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mOpen TEST3: ",0

            ELFI_OPEN TEST3, O_RDONLY
            BC      @@fail

            MVO     R2,     FD0

            CALL    A_PUTS
            STRING  "\033[33mFD ", 0

            MVI     FD0,    R0
            MVII    #NBUF,  R4
            CALL    A_PRNUM16.l

            CALL    A_PUTBUF
            DECLE   NBUF

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 9:  Open a file named "TEST2" and print its fd.        ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mOpen TEST2: ",0

            ELFI_OPEN TEST2, O_RDONLY
            BC      @@fail

            MVO     R2,     FD1

            CALL    A_PUTS
            STRING  "\033[33mFD ", 0

            MVI     FD1,    R0
            MVII    #NBUF,  R4
            CALL    A_PRNUM16.l

            CALL    A_PUTBUF
            DECLE   NBUF

            CALL    WAITKEY


            ;; ------------------------------------------------------------ ;;
            ;;  Test 10: Check the size of TEST3 with LSEEK                 ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mSEEK end TEST3: ",0

            MVI     FD0,    R2
            ELFI_LSEEK  0, SEEK_END
            BC      @@fail

            TSTR    R2
            BNEQ    @@fail

            CMPI    #16,    R1
            BNEQ    @@fail

            CALL    A_PUTS
            STRING  "\033[33mOk!", 0

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 11: Check the size of TEST2 with LSEEK                 ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mSEEK end TEST2: ",0

            MVI     FD1,    R2
            ELFI_LSEEK  0, SEEK_END
            BC      @@fail

            TSTR    R2
            BNEQ    @@fail

            CMPI    #32,    R1
            BNEQ    @@fail

            CALL    A_PUTS
            STRING  "\033[33mOk!", 0

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 12:  Rewind TEST3                                      ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mRewind TEST3: ",0

            MVI     FD0,    R2
            ELFI_LSEEK  0, SEEK_SET
            BC      @@fail

            TSTR    R2
            BNEQ    @@fail

            TSTR    R1
            BNEQ    @@fail

            CALL    A_PUTS
            STRING  "\033[33mOk!", 0

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 13:  Rewind TEST2                                      ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mRewind TEST2: ",0

            MVI     FD1,    R2
            ELFI_LSEEK  0, SEEK_SET
            BC      @@fail

            TSTR    R2
            BNEQ    @@fail

            TSTR    R1
            BNEQ    @@fail

            CALL    A_PUTS
            STRING  "\033[33mOk!", 0

            CALL    WAITKEY


            ;; ------------------------------------------------------------ ;;
            ;;  Test 14:  Read 16 bytes of RAM from TEST3.                  ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mREAD<=TEST3: ", 0

            MVI     FD0,    R2
            ELFI_READ BUF1,   16
            BC      @@fail

            CMPI    #16,    R1
            BEQ     @@t14ok

            MOVR    R1,     R0
            MVII    #NBUF,  R4
            CALL    A_PRNUM16.l

            CALL    A_PUTBUF
            DECLE   NBUF
            B       @@fail

@@t14ok
            ;; Lower 8 bits of each word should match lower 8 of buf 0
            MVII    #BUF0,  R4
            MVII    #BUF1,  R5
            MVII    #16,    R1
@@t14lp     MVI@    R4,     R0
            ANDI    #$FF,   R0
            CMP@    R5,     R0
            BNEQ    @@data_mismatch
            DECR    R1
            BNEQ    @@t14lp

            CALL    A_PUTS
            STRING  "\033[33mOk!", 0

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 15:  Read 16 words of RAM from TEST2.                  ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mREAD16<=TEST2: ", 0

            MVI     FD1,    R2
            ELFI_READ16 BUF1,   16
            BC      @@fail

            CMPI    #16,    R1
            BEQ     @@t15ok

            MOVR    R1,     R0
            MVII    #NBUF,  R4
            CALL    A_PRNUM16.l

            CALL    A_PUTBUF
            DECLE   NBUF
            B       @@fail

@@t15ok
            ;; Each word in BUF0 should match BUF1
            MVII    #BUF0,  R4
            MVII    #BUF1,  R5
            MVII    #16,    R1
@@t15lp     MVI@    R4,     R0
            CMP@    R5,     R0
            BNEQ    @@data_mismatch
            DECR    R1
            BNEQ    @@t15lp

            CALL    A_PUTS
            STRING  "\033[33mOk!", 0

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 16:  Close TEST2.                                      ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mCLOSE TEST2: ", 0

            MVI     FD1,    R2
            ELFI_CLOSE
            BC      @@fail

            CALL    A_PUTS
            STRING  "\033[33mOk!", 0

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 17:  Close TEST2.                                      ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mCLOSE TEST3: ", 0

            MVI     FD0,    R2
            ELFI_CLOSE
            BC      @@fail

            CALL    A_PUTS
            STRING  "\033[33mOk!", 0

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 18:  Unlink TEST2.                                     ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mUNLINK TEST2: ", 0

            ELFI_UNLINK TEST2
            BC      @@fail

            CALL    A_PUTS
            STRING  "\033[33mOk!", 0

            CALL    WAITKEY

            ;; ------------------------------------------------------------ ;;
            ;;  Test 19:  Unlink TEST2.                                     ;;
            ;; ------------------------------------------------------------ ;;
            CALL    A_PUTS
                                ;01234567890123456789
            STRING  "\n\r\033[37mUNLINK TEST3: ", 0

            ELFI_UNLINK TEST3
            BC      @@fail

            CALL    A_PUTS
            STRING  "\033[33mOk!", 0

            CALL    WAITKEY



            ;; ------------------------------------------------------------ ;;
            ;;  SUCCESS:    All tests pass.                                 ;;
            ;; ------------------------------------------------------------ ;;
@@pass
            CALL    A_PUTS
            STRING  "\r\n\r\nAll Tests Pass!!", 0


            DECR    PC
            ;; ------------------------------------------------------------ ;;
            ;;  FAILURE:  Just print FAIL and halt.                         ;;
            ;; ------------------------------------------------------------ ;;
@@data_mismatch
            CALL    A_PUTS
            STRING  "\n\r\033[33;41m    DATA MISMATCH   "
            STRING                 "        FAIL!       ", 0
            DECR    PC
@@fail:
            CALL    A_PUTS
            STRING  "\n\r\033[33;41m        FAIL!       ", 0

            DECR    PC

            ENDP


;; ======================================================================== ;;
;;  A_PUTS      Prints an ASCIIZ string via A_interp                        ;;
;; ======================================================================== ;;
A_PUTS      PROC
            
            INCR    PC
@@loop      PULR    R5
            MVI@    R5,      R0
            TSTR    R0
            BEQ     @@done
            PSHR    R5
            MVII    #@@loop, R5
            MVI     A_interp, PC

@@done      JR      R5
            ENDP
            
A_PUTBUF    PROC
            MVI@    R5,     R4
            PSHR    R5
            
            INCR    PC
@@loop      PULR    R4
            MVI@    R4,      R0
            TSTR    R0
            BEQ     @@done
            PSHR    R4
            MVII    #@@loop, R5
            MVI     A_interp, PC

@@done      PULR    PC
            ENDP


A_CPTR      PROC
            MVI     A_r,    R1
            SLR     R1,     2
            ADD     A_r,    R1
            SLR     R1,     2
            ADD     A_c,    R1
            MOVR    R1,     R4
            ADDI    #A_fb,  R4 
            JR      R5
            ENDP


;; ======================================================================== ;;
;;  ISR -- Just keep the screen on, and copy the STIC shadow over.          ;;
;; ======================================================================== ;;
ISR    PROC
            ;; ------------------------------------------------------------ ;;
            ;;  Enable video and update color stack.                        ;;
            ;; ------------------------------------------------------------ ;;
            MVO     R0,     STIC.viden  ; Enable display

            CLRR    R0
            CMP     CSTK_FGBG,  R0
            BEQ     @@cstk
            MVO     R0,     STIC.mode   ; ...foreground/background mode
            B       @@done_fgbg
@@cstk      MVI     STIC.mode,  R0      ; ...color stack mode
@@done_fgbg:

            MVII    #COLSTK,    R4
            MVII    #STIC.cs0,  R5

            REPEAT  5
            MVI@    R4,         R0      ; \_ Copy over color-stack shadow
            MVO@    R0,         R5      ; /
            ENDR

            B       ISRRET
            ENDP

;; ======================================================================== ;;
;;  INIT_ISR    Clear up things, detect NTSC vs. PAL, and then fall into    ;;
;;              normal ISR duties.                                          ;;
;; ======================================================================== ;;
INIT_ISR    PROC
            DIS

            ;; ------------------------------------------------------------ ;;
            ;;  Initialize the STIC                                         ;;
            ;; ------------------------------------------------------------ ;;
            CLRR    R0
            CLRR    R5
            MVII    #24,    R1
@@loop1:
            MVO@    R0,     R5          ; clear STIC registers
            DECR    R1
            BNEQ    @@loop1

            CLRR    R0
            MVO     R0,     $30         ; \
            MVO     R0,     $31         ;  |- border/scroll regs
            MVO     R0,     $32         ; /

            ;; ------------------------------------------------------------ ;;
            ;;  Copy lower-case from GROM to GRAM.                          ;;
            ;; ------------------------------------------------------------ ;;
            MVII    #$3200, R4
            MVII    #$3800, R5
            MVII    #$200,  R1
@@gloop:    MVI@    R4,     R0
            MVO@    R0,     R5
            DECR    R1
            BNEQ    @@gloop

            MVII    #ISR,   R0
            MVO     R0,     $100
            SWAP    R0
            MVO     R0,     $101

            JE      ISRRET
            ENDP

