;* ======================================================================== *;
;*  These routines are placed into the public domain by their author.  All  *;
;*  copyright rights are hereby relinquished on the routines and data in    *;
;*  this file.  -- Joseph Zbiciak, 2008                                     *;
;* ======================================================================== *;

;; ======================================================================== ;;
;;  Intellivoice Support                                                    ;;
;;  RESROM phrase indices                                                   ;;
;; ======================================================================== ;;

RESROM      PROC

@@fifo      EQU     0       ; Initiates FIFO playback

@@pa5       EQU     1       ; Pause 1000ms
@@pa4       EQU     2       ; Pause  500ms
@@pa3       EQU     3       ; Pause  200ms
@@pa2       EQU     4       ; Pause  100ms
@@pa1       EQU     5       ; Pause   50ms

@@mattel    EQU     6       ; Mattel Electronics Presents

            ; number phrases
@@0         EQU     7       ; "ZERO"
@@1         EQU     8       ; "ONE"
@@2         EQU     9       ; "TWO"
@@3         EQU     10      ; "THREE"
@@4         EQU     11      ; "FOUR"
@@5         EQU     12      ; "FIVE"
@@6         EQU     13      ; "SIX"
@@7         EQU     14      ; "SEVEN"
@@8         EQU     15      ; "EIGHT"
@@9         EQU     16      ; "NINE"
@@10        EQU     17      ; "TEN"
@@11        EQU     18      ; "ELEVEN"
@@12        EQU     19      ; "TWELVE"
@@13        EQU     20      ; "THIRTEEN"
@@14        EQU     21      ; "FOURTEEN"
@@15        EQU     22      ; "FIFTEEN"
@@16        EQU     23      ; "SIXTEEN"
@@17        EQU     24      ; "SEVENTEEN"
@@18        EQU     25      ; "EIGHTEEN"
@@19        EQU     26      ; "NINETEEN"
@@20        EQU     27      ; "TWENTY"
@@30        EQU     28      ; "THIRTY"
@@40        EQU     29      ; "FOURTY"
@@50        EQU     30      ; "FIFTY"
@@60        EQU     31      ; "SIXTY"
@@70        EQU     32      ; "SEVENTY"
@@80        EQU     33      ; "EIGHTY"
@@90        EQU     34      ; "NINETY"
@@00        EQU     35      ; "HUNDRED"
@@000       EQU     36      ; "THOUSAND"

            ; number phrases (alternate naming, same indices)
@@zero      EQU     7       ; "ZERO"
@@one       EQU     8       ; "ONE"
@@two       EQU     9       ; "TWO"
@@three     EQU     10      ; "THREE"
@@four      EQU     11      ; "FOUR"
@@five      EQU     12      ; "FIVE"
@@six       EQU     13      ; "SIX"
@@seven     EQU     14      ; "SEVEN"
@@eight     EQU     15      ; "EIGHT"
@@nine      EQU     16      ; "NINE"
@@ten       EQU     17      ; "TEN"
@@eleven    EQU     18      ; "ELEVEN"
@@twelve    EQU     19      ; "TWELVE"
@@thirteen  EQU     20      ; "THIRTEEN"
@@fourteen  EQU     21      ; "FOURTEEN"
@@fifteen   EQU     22      ; "FIFTEEN"
@@sixteen   EQU     23      ; "SIXTEEN"
@@seventeen EQU     24      ; "SEVENTEEN"
@@eighteen  EQU     25      ; "EIGHTEEN"
@@nineteen  EQU     26      ; "NINETEEN"
@@twenty    EQU     27      ; "TWENTY"
@@thirty    EQU     28      ; "THIRTY"
@@fourty    EQU     29      ; "FOURTY"
@@fifty     EQU     30      ; "FIFTY"
@@sixty     EQU     31      ; "SIXTY"
@@seventy   EQU     32      ; "SEVENTY"
@@eighty    EQU     33      ; "EIGHTY"
@@ninety    EQU     34      ; "NINETY"
@@hundred   EQU     35      ; "HUNDRED"
@@thousand  EQU     36      ; "THOUSAND"

            ; Suffixes
@@_teen     EQU     37      ; "-TEEN"
@@_ty       EQU     38      ; "-TY"

            ; Misc words
@@press     EQU     39      ; "PRESS"
@@enter     EQU     40      ; "ENTER"
@@or        EQU     41      ; "OR"
@@and       EQU     42      ; "AND"
            ENDP
