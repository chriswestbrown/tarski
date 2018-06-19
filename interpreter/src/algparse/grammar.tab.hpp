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

#ifndef YY_ALG_GRAMMAR_TAB_HPP_INCLUDED
# define YY_ALG_GRAMMAR_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int algdebug;
#endif
/* "%code requires" blocks.  */
#line 7 "grammar.ypp" /* yacc.c:1909  */

    void algerror(tarski::LexContext *LC, tarski::TarskiRef &result, char const *msg);
    int algparse(tarski::LexContext*, tarski::TarskiRef&);
 

#line 50 "grammar.tab.hpp" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    PLUS = 258,
    MINUS = 259,
    MULT = 260,
    DIV = 261,
    EXP = 262,
    VAR = 263,
    NUM = 264,
    LP = 265,
    RP = 266,
    RELOP = 267,
    TCONST = 268,
    FCONST = 269,
    LB = 270,
    RB = 271,
    AND = 272,
    OR = 273,
    LIMPLR = 274,
    NEG = 275,
    ROOT = 276,
    LEXERR = 277,
    QF = 278,
    COMMA = 279,
    INTERP = 280,
    PERCENT = 281
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
#line 21 "grammar.ypp" /* yacc.c:1909  */
union algValue
{
#line 21 "grammar.ypp" /* yacc.c:1909  */

  tarski::TokenRef*     tokPtr;
  tarski::AlgebraicRef* algPtr;
  tarski::FormulaRef*   frmPtr;

#line 95 "grammar.tab.hpp" /* yacc.c:1909  */
};
#line 21 "grammar.ypp" /* yacc.c:1909  */
typedef union algValue YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int algparse (tarski::LexContext *LC, tarski::TarskiRef &result);

#endif /* !YY_ALG_GRAMMAR_TAB_HPP_INCLUDED  */
