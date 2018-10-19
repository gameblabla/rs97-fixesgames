/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

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

#ifndef YY_YY_ASM_AS1600_TAB_H_INCLUDED
# define YY_YY_ASM_AS1600_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    CP1600_REG = 258,
    KOC_BDEF = 259,
    KOC_ELSE = 260,
    KOC_END = 261,
    KOC_ENDI = 262,
    KOC_EQU = 263,
    KOC_IF = 264,
    KOC_INCLUDE = 265,
    KOC_ORG = 266,
    KOC_RESM = 267,
    KOC_SDEF = 268,
    KOC_SET = 269,
    KOC_WDEF = 270,
    KOC_CHSET = 271,
    KOC_CHDEF = 272,
    KOC_CHUSE = 273,
    KOC_opcode = 274,
    KOC_opcode_i = 275,
    KOC_relbr = 276,
    KOC_relbr_x = 277,
    KOC_SDBD = 278,
    KOC_ROMW = 279,
    KOC_PROC = 280,
    KOC_ENDP = 281,
    KOC_STRUCT = 282,
    KOC_ENDS = 283,
    KOC_MEMATTR = 284,
    KOC_DDEF = 285,
    KOC_RPT = 286,
    KOC_ENDR = 287,
    KOC_USRERR = 288,
    KOC_LIST = 289,
    KOC_QEQU = 290,
    KOC_QSET = 291,
    KOC_MACERR = 292,
    KOC_BRKIF = 293,
    KOC_CMSG = 294,
    KOC_SMSG = 295,
    KOC_WMSG = 296,
    KOC_CFGVAR = 297,
    KOC_SRCFILE = 298,
    CONSTANT = 299,
    EOL = 300,
    KEOP_AND = 301,
    KEOP_DEFINED = 302,
    KEOP_EQ = 303,
    KEOP_GE = 304,
    KEOP_GT = 305,
    KEOP_HIGH = 306,
    KEOP_LE = 307,
    KEOP_LOW = 308,
    KEOP_LT = 309,
    KEOP_MOD = 310,
    KEOP_MUN = 311,
    KEOP_NE = 312,
    KEOP_NOT = 313,
    KEOP_OR = 314,
    KEOP_SHL = 315,
    KEOP_SHR = 316,
    KEOP_SHRU = 317,
    KEOP_XOR = 318,
    KEOP_locctr = 319,
    KEOP_STRLEN = 320,
    KEOP_ASC = 321,
    LABEL = 322,
    STRING = 323,
    QCHAR = 324,
    SYMBOL = 325,
    FEATURE = 326,
    KTK_invalid = 327
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 629 "asm/as1600_real.y" /* yacc.c:1909  */

    int             intv;
    int             longv;
    char            *strng;
    struct symel    *symb;
    struct slidx    slidx;

#line 135 "asm/as1600.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ASM_AS1600_TAB_H_INCLUDED  */
