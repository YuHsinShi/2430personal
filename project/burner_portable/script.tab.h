/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

#ifndef YY_YY_SCRIPT_TAB_H_INCLUDED
# define YY_YY_SCRIPT_TAB_H_INCLUDED
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
    NUMBER = 258,
    STRING = 259,
    BEQ = 260,
    BGT = 261,
    BGTE = 262,
    BLT = 263,
    BLTE = 264,
    BNE = 265,
    CALL = 266,
    COMMENT = 267,
    DATA = 268,
    DATA_WAIT0 = 269,
    DATA_WAIT1 = 270,
    GOTO = 271,
    MASK = 272,
    WRITE_MASK = 273,
    PCI_WRITE = 274,
    READ_MASK = 275,
    SKIP = 276,
    WAIT = 277,
    WRITE = 278,
    IF = 279,
    ELSE = 280,
    BEQ_OP = 281,
    BNE_OP = 282,
    BGT_OP = 283,
    BGTE_OP = 284,
    BLT_OP = 285,
    BLTE_OP = 286,
    MOD = 287,
    RIGHTSHIFT = 288,
    LEFTSHIFT = 289,
    PLUS = 290,
    MINUS = 291,
    DIV = 292,
    MUL = 293,
    UNARYMINUS = 294,
    COMPLEMENT = 295,
    XOR = 296,
    OR = 297,
    AND = 298
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

int verbose = 0;
 
#endif /* !YY_YY_SCRIPT_TAB_H_INCLUDED  */
