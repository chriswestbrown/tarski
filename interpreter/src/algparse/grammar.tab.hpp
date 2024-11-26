/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_ALG_GRAMMAR_TAB_HPP_INCLUDED
# define YY_ALG_GRAMMAR_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef ALGDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define ALGDEBUG 1
#  else
#   define ALGDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define ALGDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined ALGDEBUG */
#if ALGDEBUG
extern int algdebug;
#endif
/* "%code requires" blocks.  */
#line 7 "grammar.ypp"

    void algerror(tarski::LexContext *LC, tarski::TarskiRef &result, char const *msg);
    int algparse(tarski::LexContext*, tarski::TarskiRef&);
 

#line 63 "grammar.tab.hpp"

/* Token kinds.  */
#ifndef ALGTOKENTYPE
# define ALGTOKENTYPE
  enum algtokentype
  {
    ALGEMPTY = -2,
    ALGEOF = 0,                    /* "end of file"  */
    ALGerror = 256,                /* error  */
    ALGUNDEF = 257,                /* "invalid token"  */
    PLUS = 258,                    /* PLUS  */
    MINUS = 259,                   /* MINUS  */
    MULT = 260,                    /* MULT  */
    DIV = 261,                     /* DIV  */
    EXP = 262,                     /* EXP  */
    VAR = 263,                     /* VAR  */
    NUM = 264,                     /* NUM  */
    LP = 265,                      /* LP  */
    RP = 266,                      /* RP  */
    RELOP = 267,                   /* RELOP  */
    TCONST = 268,                  /* TCONST  */
    FCONST = 269,                  /* FCONST  */
    LB = 270,                      /* LB  */
    RB = 271,                      /* RB  */
    AND = 272,                     /* AND  */
    OR = 273,                      /* OR  */
    LIMPLR = 274,                  /* LIMPLR  */
    NEG = 275,                     /* NEG  */
    ROOT = 276,                    /* ROOT  */
    LEXERR = 277,                  /* LEXERR  */
    QF = 278,                      /* QF  */
    COMMA = 279,                   /* COMMA  */
    INTERP = 280,                  /* INTERP  */
    PERCENT = 281                  /* PERCENT  */
  };
  typedef enum algtokentype algtoken_kind_t;
#endif

/* Value type.  */
#if ! defined ALGSTYPE && ! defined ALGSTYPE_IS_DECLARED
#line 21 "grammar.ypp"
union algValue
{
#line 21 "grammar.ypp"

  tarski::TokenRef*     tokPtr;
  tarski::AlgebraicRef* algPtr;
  tarski::FormulaRef*   frmPtr;

#line 113 "grammar.tab.hpp"

};
#line 21 "grammar.ypp"
typedef union algValue ALGSTYPE;
# define ALGSTYPE_IS_TRIVIAL 1
# define ALGSTYPE_IS_DECLARED 1
#endif




int algparse (tarski::LexContext *LC, tarski::TarskiRef &result);


#endif /* !YY_ALG_GRAMMAR_TAB_HPP_INCLUDED  */
