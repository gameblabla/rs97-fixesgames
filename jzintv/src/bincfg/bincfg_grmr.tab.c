/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         bc_parse
#define yylex           bc_lex
#define yyerror         bc_error
#define yydebug         bc_debug
#define yynerrs         bc_nerrs

#define yylval          bc_lval
#define yychar          bc_char

/* Copy the first part of user declarations.  */
#line 10 "bincfg/bincfg_grmr_real.y" /* yacc.c:339  */


#include "config.h"
#include "lzoe/lzoe.h"
#include "bincfg/bincfg.h"
#include "bincfg/bincfg_lex.h"


static bc_diag_t *bc_diag_list = NULL;
static char bc_errbuf[1024];
static const char *bc_cursect = NULL;

static void yyerror(const char *msg);


static bc_diag_t bc_oom_error_struct =
{
    { NULL }, BC_DIAG_ERROR, 0, NULL, NULL
};


/* ------------------------------------------------------------------------ */
/*  BC_OOM_ERROR -- Out of memory error handling.                           */
/*  BC_CHKOOM    -- Check for out-of-memory condition.                      */
/*  BC_DIAG      -- Queue up a diagnostic message.                          */
/*                                                                          */
/*  I hate making these macros, but YYABORT expands to a 'goto' within the  */
/*  yyparse() function.                                                     */
/* ------------------------------------------------------------------------ */
#define BC_OOM_ERROR                                                        \
        do {                                                                \
            bc_oom_error_struct.line = bc_line_no;                          \
            bc_diag_list = &bc_oom_error_struct;                            \
            YYABORT;                                                        \
        } while (0)

#define BC_CHKOOM(m) do { if (!(m)) BC_OOM_ERROR; } while (0)

static void check_line(ll_t *l, void *opq)
{
    int *lineno = (int *)opq;
    bc_diag_t *diag = (bc_diag_t *)l;

    if (diag->line == *lineno)
        *lineno = -1;
}

#define BC_DIAG(diag, section, line_no, diagmsg)                            \
        do {                                                                \
            bc_diag_t *err;                                                 \
            ll_t *newlist;                                                  \
            int line_no_tmp = line_no;                                      \
                                                                            \
            bc_errbuf[sizeof(bc_errbuf)-1] = 0;                             \
                                                                            \
            if (yychar == TOK_ERROR_OOM)                                    \
                BC_OOM_ERROR;                                               \
                                                                            \
            ll_acton(&(bc_diag_list->l), check_line, (void*)&line_no_tmp);  \
                                                                            \
            if (line_no_tmp != -1)                                          \
            {                                                               \
                err = CALLOC(bc_diag_t, 1);                                 \
                BC_CHKOOM(err);                                             \
                                                                            \
                err->line = line_no;                                        \
                err->type = diag;                                           \
                err->sect = strdup(section);                                \
                err->msg  = strdup(diagmsg);                                \
                BC_CHKOOM(err->sect);                                       \
                BC_CHKOOM(err->msg);                                        \
                                                                            \
                newlist = ll_concat(&(bc_diag_list->l), &(err->l));         \
                bc_diag_list = (bc_diag_t *)newlist;                        \
            }                                                               \
        } while (0)


static int bc_saved_tok = -1, bc_saved_lineno = 0, bc_dont_save = 0;
static const char *bc_saved_sec = NULL;

/* Ugh:  bison magic.  Moo. */
#define BC_TOK (bc_saved_tok < 0 || bc_saved_tok == YYEMPTY ? "<unknown>" : \
                yytname[yytranslate[bc_saved_tok]])

#define BC_SEC (bc_saved_sec ? bc_saved_sec : \
                bc_cursect   ? bc_cursect   : "<toplevel>")

#define S(s) bc_cursect = s


#line 166 "bincfg/bincfg_grmr.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "bincfg_grmr.tab.h".  */
#ifndef YY_BC_BINCFG_BINCFG_GRMR_TAB_H_INCLUDED
# define YY_BC_BINCFG_BINCFG_GRMR_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int bc_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOK_SEC_BANKSWITCH = 258,
    TOK_SEC_MAPPING = 259,
    TOK_SEC_ECSBANK = 260,
    TOK_SEC_MEMATTR = 261,
    TOK_SEC_PRELOAD = 262,
    TOK_SEC_MACRO = 263,
    TOK_SEC_VARS = 264,
    TOK_SEC_JOYSTICK = 265,
    TOK_SEC_KEYS = 266,
    TOK_SEC_CAPSLOCK = 267,
    TOK_SEC_NUMLOCK = 268,
    TOK_SEC_SCROLLLOCK = 269,
    TOK_SEC_DISASM = 270,
    TOK_SEC_VOICES = 271,
    TOK_SEC_UNKNOWN = 272,
    TOK_RAM = 273,
    TOK_ROM = 274,
    TOK_WOM = 275,
    TOK_PAGE = 276,
    TOK_MAC_QUIET = 277,
    TOK_MAC_REG = 278,
    TOK_MAC_AHEAD = 279,
    TOK_MAC_BLANK = 280,
    TOK_MAC_INSPECT = 281,
    TOK_MAC_LOAD = 282,
    TOK_MAC_RUN = 283,
    TOK_MAC_POKE = 284,
    TOK_MAC_RUNTO = 285,
    TOK_MAC_TRACE = 286,
    TOK_MAC_VIEW = 287,
    TOK_MAC_WATCH = 288,
    TOK_DECONLY = 289,
    TOK_DEC = 290,
    TOK_HEX = 291,
    TOK_NAME = 292,
    TOK_ERROR_BAD = 293,
    TOK_ERROR_OOM = 294
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 103 "bincfg/bincfg_grmr_real.y" /* yacc.c:355  */

    int                 intv;
    char                *strv;
    bc_varlike_t        varlike;
    bc_varlike_types_t  varlike_type;
    bc_var_t            *var_list;
    bc_strnum_t         strnum;
    bc_mac_watch_t      mac_watch;
    bc_macro_t          macro;
    bc_macro_t          *macro_list;
    bc_memspan_t        *memspan_list;
    bc_cfgfile_t        *cfgfile;

#line 260 "bincfg/bincfg_grmr.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE bc_lval;

int bc_parse (void);

#endif /* !YY_BC_BINCFG_BINCFG_GRMR_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 275 "bincfg/bincfg_grmr.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  9
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   637

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  45
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  48
/* YYNRULES -- Number of rules.  */
#define YYNRULES  113
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  192

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   294

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      44,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    43,    40,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    42,     2,
       2,    41,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   199,   199,   218,   223,   228,   279,   280,   281,   294,
     303,   318,   319,   320,   321,   322,   328,   329,   335,   339,
     342,   355,   359,   373,   374,   380,   384,   388,   401,   414,
     418,   433,   434,   440,   444,   448,   461,   465,   479,   480,
     487,   491,   494,   507,   520,   524,   539,   540,   546,   550,
     553,   566,   597,   598,   604,   608,   613,   625,   637,   641,
     647,   653,   661,   667,   673,   678,   683,   688,   689,   690,
     691,   692,   702,   708,   723,   726,   727,   730,   739,   759,
     766,   767,   768,   769,   770,   771,   774,   778,   781,   789,
     790,   804,   806,   807,   808,   851,   858,   865,   872,   881,
     882,   885,   886,   889,   890,   891,   892,   895,   896,   897,
     900,   901,   904,   907
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_SEC_BANKSWITCH", "TOK_SEC_MAPPING",
  "TOK_SEC_ECSBANK", "TOK_SEC_MEMATTR", "TOK_SEC_PRELOAD", "TOK_SEC_MACRO",
  "TOK_SEC_VARS", "TOK_SEC_JOYSTICK", "TOK_SEC_KEYS", "TOK_SEC_CAPSLOCK",
  "TOK_SEC_NUMLOCK", "TOK_SEC_SCROLLLOCK", "TOK_SEC_DISASM",
  "TOK_SEC_VOICES", "TOK_SEC_UNKNOWN", "TOK_RAM", "TOK_ROM", "TOK_WOM",
  "TOK_PAGE", "TOK_MAC_QUIET", "TOK_MAC_REG", "TOK_MAC_AHEAD",
  "TOK_MAC_BLANK", "TOK_MAC_INSPECT", "TOK_MAC_LOAD", "TOK_MAC_RUN",
  "TOK_MAC_POKE", "TOK_MAC_RUNTO", "TOK_MAC_TRACE", "TOK_MAC_VIEW",
  "TOK_MAC_WATCH", "TOK_DECONLY", "TOK_DEC", "TOK_HEX", "TOK_NAME",
  "TOK_ERROR_BAD", "TOK_ERROR_OOM", "'-'", "'='", "':'", "','", "'\\n'",
  "$accept", "config_file", "config", "seed_config", "sec_memspan",
  "bankswitch", "sec_banksw", "banksw_list", "banksw_rec", "mapping",
  "sec_mapping", "mapping_list", "mapping_rec", "ecsbank", "sec_ecsbank",
  "ecsbank_list", "ecsbank_rec", "memattr", "sec_memattr", "memattr_list",
  "memattr_rec", "preload", "sec_preload", "preload_list", "preload_rec",
  "macro", "sec_macro", "macro_list", "macro_rec", "macro_line", "mac_reg",
  "watch_list", "watch_span", "watch_addr", "watch_range", "sec_varlike",
  "varlike_head", "var_list", "var_rec", "sec_unsup", "unknown_sec",
  "strnum", "dec_num", "hex_num", "string", "eolns", "eoln", "error_tok", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
      45,    61,    58,    44,    10
};
# endif

#define YYPACT_NINF -105

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-105)))

#define YYTABLE_NINF -80

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     503,  -105,  -105,     5,   525,  -105,   -36,  -105,  -105,  -105,
    -105,  -105,  -105,  -105,  -105,  -105,  -105,  -105,  -105,  -105,
    -105,  -105,  -105,  -105,  -105,  -105,  -105,  -105,  -105,   -36,
    -105,   -36,  -105,   -36,  -105,   -36,  -105,   -36,  -105,   -36,
    -105,  -105,   -36,  -105,   -36,  -105,    23,    23,    23,    23,
      10,   526,    19,  -105,  -105,  -105,   335,  -105,   -30,  -105,
     -36,   377,  -105,   -28,  -105,   -36,   419,  -105,   -12,  -105,
     -36,   461,  -105,   -17,  -105,   -36,   601,  -105,    -6,   -36,
     559,  -105,   -36,   -36,   -14,   -18,   -36,   -14,   -14,   -14,
     -36,   -18,   251,  -105,  -105,   -14,  -105,   -36,  -105,  -105,
    -105,  -105,   293,  -105,   -13,  -105,   -36,  -105,   -14,  -105,
    -105,   -14,  -105,  -105,   -14,  -105,  -105,   -14,  -105,  -105,
     -14,  -105,  -105,  -105,  -105,  -105,   -36,    -8,  -105,   -14,
     -36,   -36,  -105,   -14,  -105,   -36,  -105,  -105,    59,  -105,
     -36,    -9,    -4,   -37,     2,  -105,  -105,  -105,   -14,   -36,
    -105,  -105,    -3,  -105,  -105,  -105,    -2,  -105,  -105,  -105,
    -105,  -105,   -36,  -105,   -14,   -14,    -5,  -105,   -14,   -36,
    -105,   -14,  -105,   -14,  -105,   -19,     9,    -8,   -36,  -105,
    -105,  -105,   -14,  -105,   -14,   -36,  -105,   -36,   -36,  -105,
    -105,  -105
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,   113,   112,     0,     2,     7,    10,   111,     8,     1,
      16,    23,    31,    38,    46,    52,    80,    81,    82,    83,
      84,    85,    92,    93,    94,   107,   108,   109,     3,     0,
      11,     0,    12,     0,    13,     0,    14,     0,    15,     0,
       4,     5,     0,     6,     0,   110,     0,     0,     0,     0,
       0,     0,     0,    91,   102,   101,     0,    19,     0,    21,
       0,     0,    26,     0,    29,     0,     0,    34,     0,    36,
       0,     0,    41,     0,    44,     0,     0,    49,     0,     0,
       0,    72,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    55,    57,     0,    59,     0,   106,   105,
     104,   103,     0,    87,     0,    89,     0,    18,     0,    22,
      25,     0,    30,    33,     0,    37,    40,     0,    45,    48,
       0,    51,    56,    58,    67,    68,     0,     0,    69,     0,
       0,     0,    70,     0,    54,     0,    71,    86,     0,    90,
       0,     0,     0,     0,     0,    64,   100,    99,     0,     0,
      65,    66,     0,    74,    75,    76,    77,    60,    98,    97,
      96,    95,     0,    20,     0,     0,     0,    43,     0,     0,
      63,     0,    62,     0,    88,     0,     0,     0,     0,    61,
      73,    78,     0,    27,     0,     0,    50,     0,     0,    42,
      28,    35
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -105,  -105,  -105,  -105,  -105,  -105,  -105,  -105,     1,  -105,
    -105,  -105,     3,  -105,  -105,  -105,     6,  -104,  -105,  -105,
       7,  -105,  -105,  -105,    -7,  -105,  -105,  -105,   -24,     4,
    -105,  -105,   -98,  -105,  -105,  -105,  -105,  -105,   -26,  -105,
    -105,  -105,  -103,    66,   -82,  -105,     0,   144
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,     5,    28,    29,    30,    56,    57,    31,
      32,    61,    62,    33,    34,    66,    67,    35,    36,    71,
      72,    37,    38,    76,    77,    39,    40,    92,    93,    94,
      95,   152,   153,   154,   155,    41,    42,   102,   103,    43,
      44,   162,   148,    58,   104,     6,    59,    97
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
       7,    13,   182,   127,   166,     9,    45,     2,     2,   133,
     108,     1,   111,    25,    26,    27,    98,    99,   100,   101,
       1,    54,    55,   117,     1,     2,   146,   147,   138,    46,
     114,    47,   164,    48,   120,    49,   165,    50,   173,    51,
     171,     2,    52,   168,    53,    54,    55,    64,    69,    74,
     184,    96,   105,    98,    99,   100,   101,   107,    54,    55,
     109,    64,   177,     2,   110,   112,    69,     2,   134,   119,
     115,    74,   113,   180,   185,   118,   137,     0,   116,   121,
     123,     0,   124,   125,   122,     0,   128,     0,     0,     0,
     132,     0,    96,   158,   159,   160,   161,   136,     0,     0,
       0,     0,   105,     0,     0,     0,   139,     0,     0,     0,
       0,     0,     0,    63,    68,    73,    78,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   145,    63,     0,     0,
     150,   151,    68,     0,     0,   157,     0,    73,     0,     0,
     163,     0,    78,   167,     8,     0,     0,     0,     0,   170,
     126,     0,   172,   129,   130,   131,     0,     0,     0,     0,
       0,   135,   174,     0,     0,     0,     0,     0,     0,   179,
       0,     0,     0,     0,   140,   183,     0,   141,   186,     0,
     142,     0,     0,   143,     0,   189,   144,   190,   191,     0,
      60,    65,    70,    75,    79,   149,   106,     0,     0,   156,
      60,     0,     0,     0,     0,    65,     0,     0,     0,     0,
      70,     0,     0,     0,   169,    75,     0,     0,     0,     0,
      79,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     175,   176,     0,     0,   178,     0,     0,   156,     0,   181,
       0,     0,     0,     0,     0,     0,   106,     0,   187,     0,
     188,   -53,     1,     0,   -53,   -53,   -53,   -53,   -53,   -53,
     -53,   -53,   -53,   -53,   -53,   -53,   -53,   -53,   -53,   -53,
     -53,   -53,     0,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,     0,     0,     0,     0,     0,
       0,     0,     0,   -79,     1,     2,   -79,   -79,   -79,   -79,
     -79,   -79,   -79,   -79,   -79,   -79,   -79,   -79,   -79,   -79,
     -79,   -79,   -79,   -79,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    98,    99,   100,
     101,     0,     0,     0,     0,   -17,     1,     2,   -17,   -17,
     -17,   -17,   -17,   -17,   -17,   -17,   -17,   -17,   -17,   -17,
     -17,   -17,   -17,   -17,   -17,   -17,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      54,    55,     0,     0,     0,     0,     0,   -24,     1,     2,
     -24,   -24,   -24,   -24,   -24,   -24,   -24,   -24,   -24,   -24,
     -24,   -24,   -24,   -24,   -24,   -24,   -24,   -24,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    54,    55,     0,     0,     0,     0,     0,   -32,
       1,     2,   -32,   -32,   -32,   -32,   -32,   -32,   -32,   -32,
     -32,   -32,   -32,   -32,   -32,   -32,   -32,   -32,   -32,   -32,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    54,    55,     0,     0,     0,     0,
       0,   -39,     1,     2,   -39,   -39,   -39,   -39,   -39,   -39,
     -39,   -39,   -39,   -39,   -39,   -39,   -39,   -39,   -39,   -39,
     -39,   -39,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    54,    55,     0,     0,
       0,     0,     0,    -9,     1,     2,    -9,    -9,    -9,    -9,
      -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,
      -9,    -9,    -9,    -9,     0,     0,     0,     1,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,     2,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
       1,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       2,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,     0,     0,     0,     0,     0,     0,     0,
       0,   -47,     1,     2,   -47,   -47,   -47,   -47,   -47,   -47,
     -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,
     -47,   -47,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    54,    55
};

static const yytype_int16 yycheck[] =
{
       0,     6,    21,    85,    41,     0,     6,    44,    44,    91,
      40,     1,    40,    18,    19,    20,    34,    35,    36,    37,
       1,    35,    36,    40,     1,    44,    34,    35,    41,    29,
      42,    31,    41,    33,    40,    35,    40,    37,    40,    39,
      43,    44,    42,    41,    44,    35,    36,    47,    48,    49,
      41,    51,    52,    34,    35,    36,    37,    56,    35,    36,
      60,    61,   166,    44,    61,    65,    66,    44,    92,    76,
      70,    71,    66,   171,   177,    75,   102,    -1,    71,    79,
      80,    -1,    82,    83,    80,    -1,    86,    -1,    -1,    -1,
      90,    -1,    92,    34,    35,    36,    37,    97,    -1,    -1,
      -1,    -1,   102,    -1,    -1,    -1,   106,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    48,    49,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   126,    61,    -1,    -1,
     130,   131,    66,    -1,    -1,   135,    -1,    71,    -1,    -1,
     140,    -1,    76,   143,     0,    -1,    -1,    -1,    -1,   149,
      84,    -1,   152,    87,    88,    89,    -1,    -1,    -1,    -1,
      -1,    95,   162,    -1,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,   108,   175,    -1,   111,   178,    -1,
     114,    -1,    -1,   117,    -1,   185,   120,   187,   188,    -1,
      46,    47,    48,    49,    50,   129,    52,    -1,    -1,   133,
      56,    -1,    -1,    -1,    -1,    61,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,   148,    71,    -1,    -1,    -1,    -1,
      76,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     164,   165,    -1,    -1,   168,    -1,    -1,   171,    -1,   173,
      -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,   182,    -1,
     184,     0,     1,    -1,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    -1,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     0,     1,    44,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    34,    35,    36,
      37,    -1,    -1,    -1,    -1,     0,     1,    44,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      35,    36,    -1,    -1,    -1,    -1,    -1,     0,     1,    44,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    36,    -1,    -1,    -1,    -1,    -1,     0,
       1,    44,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    35,    36,    -1,    -1,    -1,    -1,
      -1,     0,     1,    44,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    35,    36,    -1,    -1,
      -1,    -1,    -1,     0,     1,    44,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    -1,    -1,    -1,     1,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    -1,    44,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
       1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     0,     1,    44,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    35,    36
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    44,    46,    47,    48,    90,    91,    92,     0,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    49,    50,
      51,    54,    55,    58,    59,    62,    63,    66,    67,    70,
      71,    80,    81,    84,    85,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    35,    36,    52,    53,    88,    91,
      92,    56,    57,    88,    91,    92,    60,    61,    88,    91,
      92,    64,    65,    88,    91,    92,    68,    69,    88,    92,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    72,    73,    74,    75,    91,    92,    34,    35,
      36,    37,    82,    83,    89,    91,    92,    53,    40,    91,
      57,    40,    91,    61,    42,    91,    65,    40,    91,    69,
      40,    91,    74,    91,    91,    91,    88,    89,    91,    88,
      88,    88,    91,    89,    73,    88,    91,    83,    41,    91,
      88,    88,    88,    88,    88,    91,    34,    35,    87,    88,
      91,    91,    76,    77,    78,    79,    88,    91,    34,    35,
      36,    37,    86,    91,    41,    40,    41,    91,    41,    88,
      91,    43,    91,    40,    91,    88,    88,    62,    88,    91,
      77,    88,    21,    91,    41,    87,    91,    88,    88,    91,
      91,    91
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    45,    46,    47,    47,    47,    47,    47,    47,    48,
      48,    49,    49,    49,    49,    49,    50,    51,    52,    52,
      53,    53,    53,    54,    55,    56,    56,    57,    57,    57,
      57,    58,    59,    60,    60,    61,    61,    61,    62,    63,
      64,    64,    65,    65,    65,    65,    66,    67,    68,    68,
      69,    69,    70,    71,    72,    72,    73,    73,    73,    73,
      74,    74,    74,    74,    74,    74,    74,    74,    74,    74,
      74,    74,    75,    76,    76,    77,    77,    78,    79,    80,
      81,    81,    81,    81,    81,    81,    82,    82,    83,    83,
      83,    84,    85,    85,    85,    86,    86,    86,    86,    87,
      87,    88,    88,    89,    89,    89,    89,    62,    62,    62,
      90,    90,    91,    92
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     2,     2,     2,     1,     1,     0,
       1,     1,     1,     1,     1,     1,     1,     3,     2,     1,
       4,     1,     2,     1,     3,     2,     1,     6,     8,     1,
       2,     1,     3,     2,     1,     8,     1,     2,     1,     3,
       2,     1,     7,     4,     1,     2,     1,     3,     2,     1,
       6,     2,     1,     3,     2,     1,     2,     1,     2,     1,
       3,     5,     4,     4,     3,     3,     3,     2,     2,     2,
       2,     2,     1,     3,     1,     1,     1,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     2,     1,     4,     1,
       2,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     1,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 200 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                /* Before the chicken or egg came the rooster */
                if (!bc_parsed_cfg)
                {
                    bc_parsed_cfg = CALLOC(bc_cfgfile_t, 1);
                    BC_CHKOOM(bc_parsed_cfg);
                    bc_cursect = NULL;
                }

                bc_parsed_cfg->diags = bc_diag_list;
                bc_diag_list  = NULL;
            }
#line 1600 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 219 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                if ((yyvsp[0].memspan_list)) 
                    LL_CONCAT(bc_parsed_cfg->span, (yyvsp[0].memspan_list), bc_memspan_t);
            }
#line 1609 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 224 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                if ((yyvsp[0].macro_list)) 
                    LL_CONCAT(bc_parsed_cfg->macro, (yyvsp[0].macro_list), bc_macro_t);
            }
#line 1618 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 229 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                if ((yyvsp[0].varlike).vars) switch ((yyvsp[0].varlike).type)
                {
                    case BC_VL_VARS:
                    {
                        LL_CONCAT(bc_parsed_cfg->vars, (yyvsp[0].varlike).vars, bc_var_t);
                        break;
                    }

                    case BC_VL_JOYSTICK:
                    {
                        LL_CONCAT(bc_parsed_cfg->joystick, (yyvsp[0].varlike).vars, bc_var_t);
                        break;
                    }

                    case BC_VL_KEYS:
                    {
                        LL_CONCAT(bc_parsed_cfg->keys[0], (yyvsp[0].varlike).vars, bc_var_t);
                        break;
                    }

                    case BC_VL_CAPSLOCK:
                    {
                        LL_CONCAT(bc_parsed_cfg->keys[1], (yyvsp[0].varlike).vars, bc_var_t);
                        break;
                    }

                    case BC_VL_NUMLOCK:
                    {
                        LL_CONCAT(bc_parsed_cfg->keys[2], (yyvsp[0].varlike).vars, bc_var_t);
                        break;
                    }

                    case BC_VL_SCROLLLOCK:
                    {
                        LL_CONCAT(bc_parsed_cfg->keys[3], (yyvsp[0].varlike).vars, bc_var_t);
                        break;
                    }

                    default:
                    {
                        /* report the error? */
                        BC_DIAG(BC_DIAG_ERROR, 
                                bc_cursect ? bc_cursect : "<toplevel>", 
                                bc_line_no, 
                                "Internal error processing variable lists");
                        break;
                    }
                }
            }
#line 1673 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 282 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                snprintf(bc_errbuf, sizeof(bc_errbuf),
                         "Unexpected token/state '%s'", BC_TOK);
                BC_DIAG(BC_DIAG_ERROR, BC_SEC,
                        bc_saved_lineno, bc_errbuf);
            }
#line 1684 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 294 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                /* Before the chicken or egg came the rooster */
                if (!bc_parsed_cfg)
                {
                    bc_parsed_cfg = CALLOC(bc_cfgfile_t, 1);
                    BC_CHKOOM(bc_parsed_cfg);
                    bc_cursect = NULL;
                }
            }
#line 1698 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 304 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                if (!bc_parsed_cfg)
                {
                    bc_parsed_cfg = CALLOC(bc_cfgfile_t, 1);
                    BC_CHKOOM(bc_parsed_cfg);
                    bc_cursect = NULL;
                }
            }
#line 1711 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 328 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { S("[bankswitch]"); }
#line 1717 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 330 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = LL_REVERSE((yyvsp[0].memspan_list), bc_memspan_t);
            }
#line 1725 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 336 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = (bc_memspan_t*)ll_insert(&((yyvsp[-1].memspan_list)->l), &((yyvsp[0].memspan_list)->l));
            }
#line 1733 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 339 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.memspan_list) = (yyvsp[0].memspan_list);   }
#line 1739 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 343 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM((yyval.memspan_list));
                (yyval.memspan_list)->s_fofs = 0;
                (yyval.memspan_list)->e_fofs = 0;
                (yyval.memspan_list)->s_addr = (yyvsp[-3].intv);
                (yyval.memspan_list)->e_addr = (yyvsp[-1].intv);
                (yyval.memspan_list)->flags  = BC_SPAN_B | BC_SPAN_R;
                (yyval.memspan_list)->width  = 16;
                (yyval.memspan_list)->epage  = BC_SPAN_NOPAGE;
                (yyval.memspan_list)->f_name = NULL;
            }
#line 1756 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 356 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = NULL;
            }
#line 1764 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 360 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = NULL;
                snprintf(bc_errbuf, sizeof(bc_errbuf),
                         "Unexpected token/state '%s'", BC_TOK);
                BC_DIAG(BC_DIAG_ERROR, "[bankswitch]", 
                        bc_saved_lineno, bc_errbuf);
            }
#line 1776 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 373 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { S("[mapping]"); }
#line 1782 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 375 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = LL_REVERSE((yyvsp[0].memspan_list), bc_memspan_t);
            }
#line 1790 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 381 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = (bc_memspan_t*)ll_insert(&((yyvsp[-1].memspan_list)->l), &((yyvsp[0].memspan_list)->l));
            }
#line 1798 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 384 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.memspan_list) = (yyvsp[0].memspan_list); }
#line 1804 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 389 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM((yyval.memspan_list));
                (yyval.memspan_list)->s_fofs = (yyvsp[-5].intv);
                (yyval.memspan_list)->e_fofs = (yyvsp[-3].intv);
                (yyval.memspan_list)->s_addr = (yyvsp[-1].intv);
                (yyval.memspan_list)->e_addr = (yyvsp[-1].intv) + (yyvsp[-3].intv) - (yyvsp[-5].intv);
                (yyval.memspan_list)->flags  = BC_SPAN_PL | BC_SPAN_R;
                (yyval.memspan_list)->width  = 16;
                (yyval.memspan_list)->epage  = BC_SPAN_NOPAGE;
                (yyval.memspan_list)->f_name = NULL;
            }
#line 1821 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 402 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM((yyval.memspan_list));
                (yyval.memspan_list)->s_fofs = (yyvsp[-7].intv);
                (yyval.memspan_list)->e_fofs = (yyvsp[-5].intv);
                (yyval.memspan_list)->s_addr = (yyvsp[-3].intv);
                (yyval.memspan_list)->e_addr = (yyvsp[-3].intv) + (yyvsp[-5].intv) - (yyvsp[-7].intv);
                (yyval.memspan_list)->flags  = BC_SPAN_PL | BC_SPAN_R | BC_SPAN_EP;
                (yyval.memspan_list)->width  = 16;
                (yyval.memspan_list)->epage  = (yyvsp[-1].intv);
                (yyval.memspan_list)->f_name = NULL;
            }
#line 1838 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 415 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = NULL;
            }
#line 1846 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 419 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = NULL;
                snprintf(bc_errbuf, sizeof(bc_errbuf),
                        "Unexpected token/state '%s'", BC_TOK);
                BC_DIAG(BC_DIAG_ERROR, "[mapping]", 
                        bc_saved_lineno, bc_errbuf);
            }
#line 1858 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 433 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { S("[ecsbank]"); }
#line 1864 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 435 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = LL_REVERSE((yyvsp[0].memspan_list), bc_memspan_t);
            }
#line 1872 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 441 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = (bc_memspan_t*)ll_insert(&((yyvsp[-1].memspan_list)->l), &((yyvsp[0].memspan_list)->l));
            }
#line 1880 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 444 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.memspan_list) = (yyvsp[0].memspan_list); }
#line 1886 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 449 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM((yyval.memspan_list));
                (yyval.memspan_list)->s_fofs = (yyvsp[-5].intv);
                (yyval.memspan_list)->e_fofs = (yyvsp[-3].intv);
                (yyval.memspan_list)->s_addr = (yyvsp[-1].intv);
                (yyval.memspan_list)->e_addr = (yyvsp[-1].intv) + (yyvsp[-3].intv) - (yyvsp[-5].intv);
                (yyval.memspan_list)->flags  = BC_SPAN_PL | BC_SPAN_R | BC_SPAN_EP;
                (yyval.memspan_list)->width  = 16;
                (yyval.memspan_list)->epage  = (yyvsp[-7].intv);
                (yyval.memspan_list)->f_name = NULL;
            }
#line 1903 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 462 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = NULL;
            }
#line 1911 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 466 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = NULL;
                snprintf(bc_errbuf, sizeof(bc_errbuf),
                        "Unexpected token/state '%s'", BC_TOK);
                BC_DIAG(BC_DIAG_ERROR, "[ecsbank]", 
                        bc_saved_lineno, bc_errbuf);
            }
#line 1923 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 479 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { S("[memattr]"); }
#line 1929 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 481 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = LL_REVERSE((yyvsp[0].memspan_list), bc_memspan_t);
            }
#line 1937 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 488 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = (bc_memspan_t*)ll_insert(&((yyvsp[-1].memspan_list)->l), &((yyvsp[0].memspan_list)->l));
            }
#line 1945 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 491 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.memspan_list) = (yyvsp[0].memspan_list); }
#line 1951 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 495 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM((yyval.memspan_list));
                (yyval.memspan_list)->s_fofs = 0;
                (yyval.memspan_list)->e_fofs = 0;
                (yyval.memspan_list)->s_addr = (yyvsp[-6].intv);
                (yyval.memspan_list)->e_addr = (yyvsp[-4].intv);
                (yyval.memspan_list)->flags  = (yyvsp[-2].intv) | ((yyvsp[-1].intv) < 16 ? BC_SPAN_N : 0);
                (yyval.memspan_list)->width  = (yyvsp[-1].intv);
                (yyval.memspan_list)->epage  = BC_SPAN_NOPAGE;
                (yyval.memspan_list)->f_name = NULL;
            }
#line 1968 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 508 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM((yyval.memspan_list));
                (yyval.memspan_list)->s_fofs = 0;
                (yyval.memspan_list)->e_fofs = 0;
                (yyval.memspan_list)->s_addr = (yyvsp[-3].intv);
                (yyval.memspan_list)->e_addr = (yyvsp[-1].intv);
                (yyval.memspan_list)->flags  = BC_SPAN_R;
                (yyval.memspan_list)->width  = 16;
                (yyval.memspan_list)->epage  = BC_SPAN_NOPAGE;
                (yyval.memspan_list)->f_name = NULL;
            }
#line 1985 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 521 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {   
                (yyval.memspan_list) = NULL;
            }
#line 1993 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 525 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = NULL;
                snprintf(bc_errbuf, sizeof(bc_errbuf),
                        "Unexpected token/state '%s'\n", BC_TOK);
                BC_DIAG(BC_DIAG_ERROR, "[memattr]", 
                        bc_saved_lineno, bc_errbuf);
            }
#line 2005 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 539 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { S("[preload]"); }
#line 2011 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 541 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = LL_REVERSE((yyvsp[0].memspan_list), bc_memspan_t);
            }
#line 2019 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 547 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = (bc_memspan_t*)ll_insert(&((yyvsp[-1].memspan_list)->l), &((yyvsp[0].memspan_list)->l));
            }
#line 2027 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 550 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.memspan_list) = (yyvsp[0].memspan_list); }
#line 2033 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 554 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = CALLOC(bc_memspan_t, 1);
                BC_CHKOOM((yyval.memspan_list));
                (yyval.memspan_list)->s_fofs = (yyvsp[-5].intv);
                (yyval.memspan_list)->e_fofs = (yyvsp[-3].intv);
                (yyval.memspan_list)->s_addr = (yyvsp[-1].intv);
                (yyval.memspan_list)->e_addr = (yyvsp[-1].intv) + (yyvsp[-3].intv) - (yyvsp[-5].intv);
                (yyval.memspan_list)->flags  = BC_SPAN_PL;
                (yyval.memspan_list)->width  = 16;
                (yyval.memspan_list)->epage  = BC_SPAN_NOPAGE;
                (yyval.memspan_list)->f_name = NULL;
            }
#line 2050 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 567 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.memspan_list) = NULL;
                snprintf(bc_errbuf, sizeof(bc_errbuf),
                        "Unexpected token/state '%s'", BC_TOK);
                BC_DIAG(BC_DIAG_ERROR, "[preload]", 
                        bc_saved_lineno, bc_errbuf);
            }
#line 2062 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 597 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { S("[macro]"); }
#line 2068 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 599 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.macro_list) = LL_REVERSE((yyvsp[0].macro_list), bc_macro_t);
            }
#line 2076 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 605 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.macro_list) = (bc_macro_t*)ll_insert(&((yyvsp[-1].macro_list)->l), &((yyvsp[0].macro_list)->l));
            }
#line 2084 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 608 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.macro_list) = (yyvsp[0].macro_list); }
#line 2090 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 614 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                if ((yyvsp[0].macro).cmd == BC_MAC_ERROR)
                    (yyval.macro_list) = NULL;
                else
                {
                    (yyval.macro_list)  = CALLOC(bc_macro_t, 1);
                    BC_CHKOOM((yyval.macro_list));
                    *(yyval.macro_list) = (yyvsp[0].macro);
                    (yyval.macro_list)->quiet = 1;
                }
            }
#line 2106 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 626 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                if ((yyvsp[0].macro).cmd == BC_MAC_ERROR)
                    (yyval.macro_list) = NULL;
                else
                {
                    (yyval.macro_list)  = CALLOC(bc_macro_t, 1);
                    BC_CHKOOM((yyval.macro_list));
                    *(yyval.macro_list) = (yyvsp[0].macro);
                    (yyval.macro_list)->quiet = 0;
                }
            }
#line 2122 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 638 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.macro_list) = NULL;
            }
#line 2130 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 642 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.macro_list) = NULL;
            }
#line 2138 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 648 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.macro).cmd            = BC_MAC_REG;
                (yyval.macro).arg.reg.reg    = (yyvsp[-2].intv);
                (yyval.macro).arg.reg.value  = (yyvsp[-1].intv);
            }
#line 2148 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 654 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.macro).cmd            = BC_MAC_LOAD;
 //fprintf(stderr, "name = %s\n", $2);
                (yyval.macro).arg.load.name  = (yyvsp[-3].strv);
                (yyval.macro).arg.load.width = (yyvsp[-2].intv);
                (yyval.macro).arg.load.addr  = (yyvsp[-1].intv);
            }
#line 2160 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 662 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.macro).cmd            = BC_MAC_WATCH;
                (yyval.macro).arg.watch      = (yyvsp[-1].mac_watch);
                (yyval.macro).arg.watch.name = (yyvsp[-2].strv);
            }
#line 2170 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 668 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.macro).cmd = BC_MAC_POKE;
                (yyval.macro).arg.poke.addr  = (yyvsp[-2].intv);
                (yyval.macro).arg.poke.value = (yyvsp[-1].intv);
            }
#line 2180 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 674 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.macro).cmd = BC_MAC_INSPECT;
                (yyval.macro).arg.inspect.addr = (yyvsp[-1].intv);
            }
#line 2189 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 679 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.macro).cmd = BC_MAC_RUNTO;
                (yyval.macro).arg.runto.addr = (yyvsp[-1].intv);
            }
#line 2198 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 684 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.macro).cmd = BC_MAC_TRACE;
                (yyval.macro).arg.runto.addr = (yyvsp[-1].intv);
            }
#line 2207 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 688 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {   (yyval.macro).cmd  = BC_MAC_AHEAD;     }
#line 2213 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 689 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {   (yyval.macro).cmd  = BC_MAC_BLANK;     }
#line 2219 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 690 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {   (yyval.macro).cmd  = BC_MAC_RUN;       }
#line 2225 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 691 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {   (yyval.macro).cmd  = BC_MAC_VIEW;      }
#line 2231 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 693 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.macro).cmd = BC_MAC_ERROR;
                snprintf(bc_errbuf, sizeof(bc_errbuf),
                        "Unexpected token/state '%s'", BC_TOK);
                BC_DIAG(BC_DIAG_WARNING, "[macro]", 
                        bc_saved_lineno, bc_errbuf);
            }
#line 2243 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 702 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.intv) = bc_hex; }
#line 2249 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 709 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                size_t new_size;
                int new_spans, i;

                new_spans = (yyvsp[-2].mac_watch).spans + (yyvsp[0].mac_watch).spans;
                new_size  = new_spans*sizeof(uint_16)*2;

                (yyval.mac_watch).addr  = (uint_16 *)realloc((yyvsp[-2].mac_watch).addr, new_size);
                (yyval.mac_watch).spans = new_spans;
                BC_CHKOOM((yyval.mac_watch).addr);

                for (i = 0; i < (yyvsp[0].mac_watch).spans*2; i++)
                    (yyval.mac_watch).addr[i + 2*(yyvsp[-2].mac_watch).spans] = (yyvsp[0].mac_watch).addr[i];
            }
#line 2268 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 723 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.mac_watch) = (yyvsp[0].mac_watch); }
#line 2274 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 731 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.mac_watch).spans   = 1;
                (yyval.mac_watch).addr    = CALLOC(uint_16, 2); BC_CHKOOM((yyval.mac_watch).addr);
                (yyval.mac_watch).addr[0] = (yyvsp[0].intv);
                (yyval.mac_watch).addr[1] = (yyvsp[0].intv);
            }
#line 2285 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 740 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.mac_watch).spans   = 1;
                (yyval.mac_watch).addr    = CALLOC(uint_16, 2); BC_CHKOOM((yyval.mac_watch).addr);
                (yyval.mac_watch).addr[0] = (yyvsp[-2].intv);
                (yyval.mac_watch).addr[1] = (yyvsp[0].intv);
            }
#line 2296 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 760 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.varlike).type = (yyvsp[-2].varlike_type);
                (yyval.varlike).vars = LL_REVERSE((yyvsp[0].var_list), bc_var_t);
            }
#line 2305 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 766 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.varlike_type)=BC_VL_VARS;       S("[vars]"      ); }
#line 2311 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 767 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.varlike_type)=BC_VL_JOYSTICK;   S("[joystick]"  ); }
#line 2317 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 768 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.varlike_type)=BC_VL_KEYS;       S("[keys]"      ); }
#line 2323 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 769 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.varlike_type)=BC_VL_CAPSLOCK;   S("[capslock]"  ); }
#line 2329 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 770 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.varlike_type)=BC_VL_NUMLOCK;    S("[numlock]"   ); }
#line 2335 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 771 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.varlike_type)=BC_VL_SCROLLLOCK; S("[scrolllock]"); }
#line 2341 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 775 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.var_list) = (bc_var_t*)ll_insert(&((yyvsp[-1].var_list)->l), &((yyvsp[0].var_list)->l));
            }
#line 2349 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 778 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.var_list) = (yyvsp[0].var_list); }
#line 2355 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 782 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.var_list) = CALLOC(bc_var_t, 1);
                BC_CHKOOM((yyval.var_list));

                (yyval.var_list)->name = (yyvsp[-3].strv);
                (yyval.var_list)->val  = (yyvsp[-1].strnum);
            }
#line 2367 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 789 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.var_list) = NULL; }
#line 2373 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 791 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.var_list) = NULL;
                snprintf(bc_errbuf, sizeof(bc_errbuf),
                        "Unexpected token/state '%s'\n", BC_TOK);
                BC_DIAG(BC_DIAG_WARNING, "[vars]", 
                        bc_saved_lineno, bc_errbuf);
            }
#line 2385 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 806 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { S("[disasm]"); }
#line 2391 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 807 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { S("[voices]"); }
#line 2397 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 809 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                static char bc_unknown[256];

                if (bc_txt)
                {
                    strncpy(bc_unknown, bc_txt, 255);
                    bc_unknown[255] = 0;
                }

                S(bc_unknown);
            }
#line 2413 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 852 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.strnum).flag    = BC_VAR_STRING;
                (yyval.strnum).str_val = strdup(bc_txt);  BC_CHKOOM((yyval.strnum).str_val);
                (yyval.strnum).dec_val = 0;
                (yyval.strnum).hex_val = 0;
            }
#line 2424 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 859 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.strnum).flag    = BC_VAR_STRING | BC_VAR_HEXNUM;
                (yyval.strnum).str_val = strdup(bc_txt);  BC_CHKOOM((yyval.strnum).str_val);
                (yyval.strnum).dec_val = 0;
                (yyval.strnum).hex_val = bc_hex;
            }
#line 2435 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 866 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.strnum).flag    = BC_VAR_STRING | BC_VAR_HEXNUM | BC_VAR_DECNUM;
                (yyval.strnum).str_val = strdup(bc_txt);  BC_CHKOOM((yyval.strnum).str_val);
                (yyval.strnum).dec_val = bc_dec;
                (yyval.strnum).hex_val = bc_hex;
            }
#line 2446 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 873 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                (yyval.strnum).flag    = BC_VAR_STRING | BC_VAR_DECNUM;
                (yyval.strnum).str_val = strdup(bc_txt);  BC_CHKOOM((yyval.strnum).str_val);
                (yyval.strnum).dec_val = bc_dec;
                (yyval.strnum).hex_val = 0;
            }
#line 2457 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 881 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.intv) = bc_dec; }
#line 2463 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 882 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.intv) = bc_dec; }
#line 2469 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 885 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.intv) = bc_hex; }
#line 2475 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 886 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.intv) = bc_hex; }
#line 2481 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 889 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.strv) = strdup(bc_txt); BC_CHKOOM((yyval.strv)); }
#line 2487 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 890 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.strv) = strdup(bc_txt); BC_CHKOOM((yyval.strv)); }
#line 2493 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 891 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.strv) = strdup(bc_txt); BC_CHKOOM((yyval.strv)); }
#line 2499 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 892 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.strv) = strdup(bc_txt); BC_CHKOOM((yyval.strv)); }
#line 2505 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 895 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.intv) = BC_SPAN_RAM; }
#line 2511 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 896 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.intv) = BC_SPAN_ROM; }
#line 2517 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 897 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { (yyval.intv) = BC_SPAN_WOM; }
#line 2523 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 904 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    { bc_dont_save = 0; bc_saved_tok = YYEMPTY; }
#line 2529 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 908 "bincfg/bincfg_grmr_real.y" /* yacc.c:1646  */
    {
                if (!bc_dont_save)
                {
                    if (yychar > 0 && yychar != YYEMPTY)
                    {
                        bc_dont_save    = 1;
                        bc_saved_tok    = yychar;
                        bc_saved_lineno = bc_line_no;
                        bc_saved_sec    = bc_cursect ? bc_cursect:"<toplevel>";
                    }
                }
            }
#line 2546 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
    break;


#line 2550 "bincfg/bincfg_grmr.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 921 "bincfg/bincfg_grmr_real.y" /* yacc.c:1906  */


/* ------------------------------------------------------------------------ */
/*  YYERROR -- required by Bison/YACC.  Note that this can leak memory in   */
/*             an OOM condition.  It's an acceptable irony, given we're     */
/*             about to die anyway.                                         */
/* ------------------------------------------------------------------------ */
static void yyerror(const char *diagmsg)
{
    bc_diag_t *err;
    ll_t *p;
    const char *cursect = bc_cursect ? bc_cursect : "<internal>";

    err = CALLOC(bc_diag_t, 1);
    if (!err) return;

    if (bc_txt && yychar == TOK_ERROR_BAD)
        snprintf(bc_errbuf, sizeof(bc_errbuf),
                "%s.  Text at error: '%s'", diagmsg, bc_txt);
    else
        snprintf(bc_errbuf, sizeof(bc_errbuf), "%s.", diagmsg);

    err->line = bc_line_no;
    err->type = BC_DIAG_ERROR;
    err->sect = strdup(cursect);    if (!err->sect) return;
    err->msg  = strdup(bc_errbuf);  if (!err->msg ) return;

    p = ll_concat(&(bc_diag_list->l), &(err->l));
    bc_diag_list = (bc_diag_t*) p;
}

/* ======================================================================== */
/*  This program is free software; you can redistribute it and/or modify    */
/*  it under the terms of the GNU General Public License as published by    */
/*  the Free Software Foundation; either version 2 of the License, or       */
/*  (at your option) any later version.                                     */
/*                                                                          */
/*  This program is distributed in the hope that it will be useful,         */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       */
/*  General Public License for more details.                                */
/*                                                                          */
/*  You should have received a copy of the GNU General Public License       */
/*  along with this program; if not, write to the Free Software             */
/*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.               */
/* ======================================================================== */
/*                 Copyright (c) 2003-+Inf, Joseph Zbiciak                  */
/* ======================================================================== */
