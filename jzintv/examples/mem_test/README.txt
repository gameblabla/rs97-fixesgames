;;==========================================================================;;
;; INTV RAM Tests.  Performs tests on a range of RAM in the Intellivision.  ;;
;; This is most useful for testing the Intellicart's RAM.                   ;;
;; Copyright 2000, Joe Zbiciak, intvnut AT gmail.com.                       ;;
;; http://spatula-city.org/~im14u2c/intv/                                   ;;
;;==========================================================================;;

This program is a simple memory tester that I wrote awhile back to help
Chad Schell verify the Intellicart.  The memory tester tries various
sequences of reads and writes to a range of addresses, verifying that
the memory is operating correctly.

The test can be configured to test various widths of RAM at various
addresses.  See the equates at the top of the test.

The test display is reasonably straghtforward:

                        > RAM Test: #00001 <

                        IPT: 00001 A: LINEAR
                        Total Errors: None.

                          Fill Const  *OK*
                          Walking 1s  *OK*
                          Walking 0s  *OK*
                         >CheckerBrd< AF39
                          Rand Value 

                        Elapsed: 00:00:00:31


The very top of the screen gives the current RAM test iteration number.
The 'IPT' is "Iterations Per Test."  'A' is "Addressing Mode", which is
either Linear or Random.   'Total Errors' is, uhm, the total number of
errors.  :-)

The next 5 lines show the list of test phases, followed by the current
status for that test in this iteration.  If a test says "*OK*" next
to it, then that means that phase passed without errors.  If it shows
a number next to it, then that phase is in-progress.  If it's blank,
then the test hasn't gotten there yet.

Run it and see what I mean -- it's pretty self-explanatory.
