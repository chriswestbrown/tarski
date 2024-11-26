/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         ALGSTYPE
/* Substitute the variable and function names.  */
#define yyparse         algparse
#define yylex           alglex
#define yyerror         algerror
#define yydebug         algdebug
#define yynerrs         algnerrs

/* First part of user prologue.  */
#line 1 "grammar.ypp"

#include "alglex.h"

  

#line 83 "grammar.tab.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "grammar.tab.hpp"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_PLUS = 3,                       /* PLUS  */
  YYSYMBOL_MINUS = 4,                      /* MINUS  */
  YYSYMBOL_MULT = 5,                       /* MULT  */
  YYSYMBOL_DIV = 6,                        /* DIV  */
  YYSYMBOL_EXP = 7,                        /* EXP  */
  YYSYMBOL_VAR = 8,                        /* VAR  */
  YYSYMBOL_NUM = 9,                        /* NUM  */
  YYSYMBOL_LP = 10,                        /* LP  */
  YYSYMBOL_RP = 11,                        /* RP  */
  YYSYMBOL_RELOP = 12,                     /* RELOP  */
  YYSYMBOL_TCONST = 13,                    /* TCONST  */
  YYSYMBOL_FCONST = 14,                    /* FCONST  */
  YYSYMBOL_LB = 15,                        /* LB  */
  YYSYMBOL_RB = 16,                        /* RB  */
  YYSYMBOL_AND = 17,                       /* AND  */
  YYSYMBOL_OR = 18,                        /* OR  */
  YYSYMBOL_LIMPLR = 19,                    /* LIMPLR  */
  YYSYMBOL_NEG = 20,                       /* NEG  */
  YYSYMBOL_ROOT = 21,                      /* ROOT  */
  YYSYMBOL_LEXERR = 22,                    /* LEXERR  */
  YYSYMBOL_QF = 23,                        /* QF  */
  YYSYMBOL_COMMA = 24,                     /* COMMA  */
  YYSYMBOL_INTERP = 25,                    /* INTERP  */
  YYSYMBOL_PERCENT = 26,                   /* PERCENT  */
  YYSYMBOL_YYACCEPT = 27,                  /* $accept  */
  YYSYMBOL_O = 28,                         /* O  */
  YYSYMBOL_F = 29,                         /* F  */
  YYSYMBOL_C = 30,                         /* C  */
  YYSYMBOL_N = 31,                         /* N  */
  YYSYMBOL_A = 32,                         /* A  */
  YYSYMBOL_Q = 33,                         /* Q  */
  YYSYMBOL_E = 34,                         /* E  */
  YYSYMBOL_T = 35,                         /* T  */
  YYSYMBOL_P = 36,                         /* P  */
  YYSYMBOL_Pb = 37,                        /* Pb  */
  YYSYMBOL_B = 38                          /* B  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
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

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined ALGSTYPE_IS_TRIVIAL && ALGSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
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
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  22
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   91

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  27
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  12
/* YYNRULES -- Number of rules.  */
#define YYNRULES  36
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  69

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   281


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26
};

#if ALGDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int8 yyrline[] =
{
       0,    43,    43,    44,    48,    49,    50,    52,    53,    55,
      56,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      71,    72,    75,    76,    77,    79,    80,    81,    82,    84,
      85,    86,    88,    89,    91,    92,    93
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if ALGDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "PLUS", "MINUS",
  "MULT", "DIV", "EXP", "VAR", "NUM", "LP", "RP", "RELOP", "TCONST",
  "FCONST", "LB", "RB", "AND", "OR", "LIMPLR", "NEG", "ROOT", "LEXERR",
  "QF", "COMMA", "INTERP", "PERCENT", "$accept", "O", "F", "C", "N", "A",
  "Q", "E", "T", "P", "Pb", "B", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-12)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      18,     6,     4,    59,   -12,   -12,     6,   -12,   -12,     6,
       6,    31,     1,    60,    26,   -12,   -12,     8,    32,    65,
     -12,    42,   -12,    59,   -12,    19,    47,    64,    50,   -12,
     -12,   -12,   -12,   -12,     6,     6,     6,     6,    44,    59,
      59,     3,   -12,    59,    59,   -12,    53,    59,    14,   -12,
     -12,   -12,    26,    26,   -12,    68,   -12,    65,    65,    59,
      52,   -12,   -12,    59,   -12,   -12,    59,   -12,    52
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,    34,    35,     0,    16,    17,     0,
       0,     0,     0,     0,     6,     8,    10,     0,     0,    24,
      28,    31,     1,     0,    29,     0,     0,     0,     0,     9,
      21,    18,    19,     3,     0,     0,     0,     0,     0,     0,
       0,     0,     2,     0,     0,    25,    33,     0,     0,    15,
      36,    14,     4,     5,     7,     0,    20,    22,    23,     0,
      13,    26,    27,     0,    30,    11,     0,    32,    12
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -12,   -12,    -3,    54,    -8,   -12,   -12,    -1,    51,    -2,
     -12,   -11
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     2,    13,    14,    15,    16,    17,    28,    19,    20,
      45,    21
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      18,    24,    29,    25,    22,    26,    27,     3,    46,    31,
       3,     4,     5,    23,     4,     5,     6,    39,    40,     7,
       8,     9,    48,    37,    59,    50,    10,    32,    54,    11,
      49,    12,    38,     1,    55,    39,    40,    34,    35,    30,
      60,    61,    62,    36,    41,    64,    46,    46,    42,    47,
      39,    40,    56,    39,    40,    39,    40,    66,    50,    41,
      63,    67,    41,     3,     0,    68,     0,     4,     5,    23,
      43,    44,     0,     4,     5,    23,    33,     0,    34,    35,
      51,     0,    34,    35,    65,     0,    34,    35,    52,    53,
      57,    58
};

static const yytype_int8 yycheck[] =
{
       1,     3,    10,     6,     0,     6,     9,     4,    19,     8,
       4,     8,     9,    10,     8,     9,    10,     3,     4,    13,
      14,    15,    23,    15,    21,    11,    20,    26,    36,    23,
      11,    25,    24,    15,    37,     3,     4,    18,    19,     8,
      41,    43,    44,    17,    12,    47,    57,    58,    16,     7,
       3,     4,     8,     3,     4,     3,     4,    59,    11,    12,
       7,    63,    12,     4,    -1,    66,    -1,     8,     9,    10,
       5,     6,    -1,     8,     9,    10,    16,    -1,    18,    19,
      16,    -1,    18,    19,    16,    -1,    18,    19,    34,    35,
      39,    40
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    15,    28,     4,     8,     9,    10,    13,    14,    15,
      20,    23,    25,    29,    30,    31,    32,    33,    34,    35,
      36,    38,     0,    10,    36,    29,    34,    29,    34,    31,
       8,     8,    26,    16,    18,    19,    17,    15,    24,     3,
       4,    12,    16,     5,     6,    37,    38,     7,    34,    11,
      11,    16,    30,    30,    31,    29,     8,    35,    35,    21,
      34,    36,    36,     7,    36,    16,    36,    36,    34
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    27,    28,    28,    29,    29,    29,    30,    30,    31,
      31,    32,    32,    32,    32,    32,    32,    32,    32,    32,
      33,    33,    34,    34,    34,    35,    35,    35,    35,    36,
      36,    36,    37,    37,    38,    38,    38
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     3,     3,     3,     3,     1,     3,     1,     2,
       1,     4,     5,     3,     3,     3,     1,     1,     2,     2,
       3,     2,     3,     3,     1,     2,     3,     3,     1,     2,
       3,     1,     3,     1,     1,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = ALGEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == ALGEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (LC, result, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use ALGerror or ALGUNDEF. */
#define YYERRCODE ALGUNDEF


/* Enable debugging if requested.  */
#if ALGDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, LC, result); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, tarski::LexContext *LC, tarski::TarskiRef &result)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (LC);
  YY_USE (result);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, tarski::LexContext *LC, tarski::TarskiRef &result)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, LC, result);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, tarski::LexContext *LC, tarski::TarskiRef &result)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], LC, result);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, LC, result); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !ALGDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !ALGDEBUG */


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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, tarski::LexContext *LC, tarski::TarskiRef &result)
{
  YY_USE (yyvaluep);
  YY_USE (LC);
  YY_USE (result);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_PLUS: /* PLUS  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 891 "grammar.tab.cpp"
        break;

    case YYSYMBOL_MINUS: /* MINUS  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 897 "grammar.tab.cpp"
        break;

    case YYSYMBOL_MULT: /* MULT  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 903 "grammar.tab.cpp"
        break;

    case YYSYMBOL_DIV: /* DIV  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 909 "grammar.tab.cpp"
        break;

    case YYSYMBOL_EXP: /* EXP  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 915 "grammar.tab.cpp"
        break;

    case YYSYMBOL_VAR: /* VAR  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 921 "grammar.tab.cpp"
        break;

    case YYSYMBOL_NUM: /* NUM  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 927 "grammar.tab.cpp"
        break;

    case YYSYMBOL_LP: /* LP  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 933 "grammar.tab.cpp"
        break;

    case YYSYMBOL_RP: /* RP  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 939 "grammar.tab.cpp"
        break;

    case YYSYMBOL_RELOP: /* RELOP  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 945 "grammar.tab.cpp"
        break;

    case YYSYMBOL_TCONST: /* TCONST  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 951 "grammar.tab.cpp"
        break;

    case YYSYMBOL_FCONST: /* FCONST  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 957 "grammar.tab.cpp"
        break;

    case YYSYMBOL_LB: /* LB  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 963 "grammar.tab.cpp"
        break;

    case YYSYMBOL_RB: /* RB  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 969 "grammar.tab.cpp"
        break;

    case YYSYMBOL_AND: /* AND  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 975 "grammar.tab.cpp"
        break;

    case YYSYMBOL_OR: /* OR  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 981 "grammar.tab.cpp"
        break;

    case YYSYMBOL_LIMPLR: /* LIMPLR  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 987 "grammar.tab.cpp"
        break;

    case YYSYMBOL_NEG: /* NEG  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 993 "grammar.tab.cpp"
        break;

    case YYSYMBOL_ROOT: /* ROOT  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 999 "grammar.tab.cpp"
        break;

    case YYSYMBOL_LEXERR: /* LEXERR  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 1005 "grammar.tab.cpp"
        break;

    case YYSYMBOL_INTERP: /* INTERP  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 1011 "grammar.tab.cpp"
        break;

    case YYSYMBOL_PERCENT: /* PERCENT  */
#line 33 "grammar.ypp"
            { /*delete $$;*/ }
#line 1017 "grammar.tab.cpp"
        break;

    case YYSYMBOL_O: /* O  */
#line 35 "grammar.ypp"
            { /*delete $$;*/ }
#line 1023 "grammar.tab.cpp"
        break;

    case YYSYMBOL_F: /* F  */
#line 35 "grammar.ypp"
            { /*delete $$;*/ }
#line 1029 "grammar.tab.cpp"
        break;

    case YYSYMBOL_C: /* C  */
#line 35 "grammar.ypp"
            { /*delete $$;*/ }
#line 1035 "grammar.tab.cpp"
        break;

    case YYSYMBOL_N: /* N  */
#line 35 "grammar.ypp"
            { /*delete $$;*/ }
#line 1041 "grammar.tab.cpp"
        break;

    case YYSYMBOL_A: /* A  */
#line 35 "grammar.ypp"
            { /*delete $$;*/ }
#line 1047 "grammar.tab.cpp"
        break;

    case YYSYMBOL_Q: /* Q  */
#line 35 "grammar.ypp"
            { /*delete $$;*/ }
#line 1053 "grammar.tab.cpp"
        break;

    case YYSYMBOL_E: /* E  */
#line 34 "grammar.ypp"
            { /*delete $$;*/ }
#line 1059 "grammar.tab.cpp"
        break;

    case YYSYMBOL_T: /* T  */
#line 34 "grammar.ypp"
            { /*delete $$;*/ }
#line 1065 "grammar.tab.cpp"
        break;

    case YYSYMBOL_P: /* P  */
#line 34 "grammar.ypp"
            { /*delete $$;*/ }
#line 1071 "grammar.tab.cpp"
        break;

    case YYSYMBOL_Pb: /* Pb  */
#line 34 "grammar.ypp"
            { /*delete $$;*/ }
#line 1077 "grammar.tab.cpp"
        break;

    case YYSYMBOL_B: /* B  */
#line 34 "grammar.ypp"
            { /*delete $$;*/ }
#line 1083 "grammar.tab.cpp"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (tarski::LexContext *LC, tarski::TarskiRef &result)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = ALGEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == ALGEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, LC);
    }

  if (yychar <= ALGEOF)
    {
      yychar = ALGEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == ALGerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = ALGUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = ALGEMPTY;
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
| yyreduce -- do a reduction.  |
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
  case 2: /* O: LB E RB  */
#line 43 "grammar.ypp"
            { result = *((tarski::TarskiRef*)(yyvsp[-1].algPtr));  (yyval.frmPtr) = 0; delete (yyvsp[-2].tokPtr); delete (yyvsp[-1].algPtr); delete (yyvsp[0].tokPtr); YYACCEPT; }
#line 1359 "grammar.tab.cpp"
    break;

  case 3: /* O: LB F RB  */
#line 44 "grammar.ypp"
            { result = *((tarski::TarskiRef*)(yyvsp[-1].frmPtr));  (yyval.frmPtr) = 0; delete (yyvsp[-2].tokPtr); delete (yyvsp[-1].frmPtr); delete (yyvsp[0].tokPtr); YYACCEPT; }
#line 1365 "grammar.tab.cpp"
    break;

  case 4: /* F: F OR C  */
#line 48 "grammar.ypp"
             { (yyval.frmPtr) = tarski::newOr((yyvsp[-2].frmPtr),(yyvsp[0].frmPtr));             delete (yyvsp[-2].frmPtr); delete (yyvsp[-1].tokPtr); delete (yyvsp[0].frmPtr); }
#line 1371 "grammar.tab.cpp"
    break;

  case 5: /* F: F LIMPLR C  */
#line 49 "grammar.ypp"
               { tarski::FormulaRef* p = newNot((yyvsp[-2].frmPtr)); (yyval.frmPtr) = newOr(p,(yyvsp[0].frmPtr));   delete (yyvsp[-2].frmPtr); delete (yyvsp[-1].tokPtr); delete (yyvsp[0].frmPtr); delete p; }
#line 1377 "grammar.tab.cpp"
    break;

  case 6: /* F: C  */
#line 50 "grammar.ypp"
      { (yyval.frmPtr) = tarski::copy((yyvsp[0].frmPtr));                        delete (yyvsp[0].frmPtr); }
#line 1383 "grammar.tab.cpp"
    break;

  case 7: /* C: C AND N  */
#line 52 "grammar.ypp"
            { (yyval.frmPtr) = tarski::newAnd((yyvsp[-2].frmPtr),(yyvsp[0].frmPtr));             delete (yyvsp[-2].frmPtr); delete (yyvsp[-1].tokPtr); delete (yyvsp[0].frmPtr); }
#line 1389 "grammar.tab.cpp"
    break;

  case 8: /* C: N  */
#line 53 "grammar.ypp"
      { (yyval.frmPtr) = tarski::copy((yyvsp[0].frmPtr));                        delete (yyvsp[0].frmPtr); }
#line 1395 "grammar.tab.cpp"
    break;

  case 9: /* N: NEG N  */
#line 55 "grammar.ypp"
          { (yyval.frmPtr) = tarski::newNot((yyvsp[0].frmPtr));                  delete (yyvsp[-1].tokPtr); delete (yyvsp[0].frmPtr); }
#line 1401 "grammar.tab.cpp"
    break;

  case 10: /* N: A  */
#line 56 "grammar.ypp"
      { (yyval.frmPtr) = tarski::copy((yyvsp[0].frmPtr));                        delete (yyvsp[0].frmPtr); }
#line 1407 "grammar.tab.cpp"
    break;

  case 11: /* A: Q LB F RB  */
#line 58 "grammar.ypp"
              { (yyval.frmPtr) = tarski::addFormulaToQuantifier((yyvsp[-3].frmPtr),(yyvsp[-1].frmPtr)); delete (yyvsp[-3].frmPtr); delete (yyvsp[-2].tokPtr); delete (yyvsp[-1].frmPtr); delete (yyvsp[0].tokPtr); }
#line 1413 "grammar.tab.cpp"
    break;

  case 12: /* A: E RELOP ROOT P E  */
#line 59 "grammar.ypp"
                     { (yyval.frmPtr) = tarski::newExtTarskiAtom((yyvsp[-4].algPtr),(yyvsp[-3].tokPtr),(yyvsp[-1].algPtr),(yyvsp[0].algPtr)); delete (yyvsp[-4].algPtr); delete (yyvsp[-3].tokPtr); delete (yyvsp[-2].tokPtr); delete (yyvsp[-1].algPtr); delete (yyvsp[0].algPtr); }
#line 1419 "grammar.tab.cpp"
    break;

  case 13: /* A: E RELOP E  */
#line 60 "grammar.ypp"
              { (yyval.frmPtr) = tarski::newTarskiAtom((yyvsp[-2].algPtr),(yyvsp[-1].tokPtr),(yyvsp[0].algPtr)); delete (yyvsp[-2].algPtr); delete (yyvsp[-1].tokPtr); delete (yyvsp[0].algPtr); }
#line 1425 "grammar.tab.cpp"
    break;

  case 14: /* A: LB F RB  */
#line 61 "grammar.ypp"
            { (yyval.frmPtr) = tarski::copy((yyvsp[-1].frmPtr));                  delete (yyvsp[-2].tokPtr); delete (yyvsp[-1].frmPtr); delete (yyvsp[0].tokPtr); }
#line 1431 "grammar.tab.cpp"
    break;

  case 15: /* A: LP F RP  */
#line 62 "grammar.ypp"
            { (yyval.frmPtr) = tarski::copy((yyvsp[-1].frmPtr));                  delete (yyvsp[-2].tokPtr); delete (yyvsp[-1].frmPtr); delete (yyvsp[0].tokPtr); }
#line 1437 "grammar.tab.cpp"
    break;

  case 16: /* A: TCONST  */
#line 63 "grammar.ypp"
           { (yyval.frmPtr) = tarski::newTrue();                  delete (yyvsp[0].tokPtr); }
#line 1443 "grammar.tab.cpp"
    break;

  case 17: /* A: FCONST  */
#line 64 "grammar.ypp"
           { (yyval.frmPtr) = tarski::newFalse();                 delete (yyvsp[0].tokPtr); }
#line 1449 "grammar.tab.cpp"
    break;

  case 18: /* A: INTERP VAR  */
#line 65 "grammar.ypp"
               { (yyval.frmPtr) = tarski::newInterpVar((yyvsp[0].tokPtr));       delete (yyvsp[-1].tokPtr);    delete (yyvsp[0].tokPtr); }
#line 1455 "grammar.tab.cpp"
    break;

  case 19: /* A: INTERP PERCENT  */
#line 66 "grammar.ypp"
                   { (yyval.frmPtr) = tarski::newInterpVar((yyvsp[0].tokPtr));       delete (yyvsp[-1].tokPtr);    delete (yyvsp[0].tokPtr); }
#line 1461 "grammar.tab.cpp"
    break;

  case 20: /* Q: Q COMMA VAR  */
#line 71 "grammar.ypp"
                { (yyval.frmPtr) = tarski::addVarToQuantifier((yyvsp[-2].frmPtr),(yyvsp[0].tokPtr)); delete (yyvsp[-2].frmPtr); delete (yyvsp[-1].tokPtr); delete (yyvsp[0].tokPtr); }
#line 1467 "grammar.tab.cpp"
    break;

  case 21: /* Q: QF VAR  */
#line 72 "grammar.ypp"
           { (yyval.frmPtr) = tarski::newQuantifierBlock((yyvsp[-1].tokPtr),(yyvsp[0].tokPtr));      delete (yyvsp[-1].tokPtr); delete (yyvsp[0].tokPtr); }
#line 1473 "grammar.tab.cpp"
    break;

  case 22: /* E: E PLUS T  */
#line 75 "grammar.ypp"
              { (yyval.algPtr) = tarski::newAdd((yyvsp[-2].algPtr),(yyvsp[0].algPtr));           delete (yyvsp[-2].algPtr); delete (yyvsp[-1].tokPtr); delete (yyvsp[0].algPtr); }
#line 1479 "grammar.tab.cpp"
    break;

  case 23: /* E: E MINUS T  */
#line 76 "grammar.ypp"
              { (yyval.algPtr) = tarski::newSbt((yyvsp[-2].algPtr),(yyvsp[0].algPtr));           delete (yyvsp[-2].algPtr); delete (yyvsp[-1].tokPtr); delete (yyvsp[0].algPtr); }
#line 1485 "grammar.tab.cpp"
    break;

  case 24: /* E: T  */
#line 77 "grammar.ypp"
      { (yyval.algPtr) = tarski::copy((yyvsp[0].algPtr));                        delete (yyvsp[0].algPtr); }
#line 1491 "grammar.tab.cpp"
    break;

  case 25: /* T: T Pb  */
#line 79 "grammar.ypp"
         { (yyval.algPtr) = tarski::newMlt((yyvsp[-1].algPtr),(yyvsp[0].algPtr));                delete (yyvsp[-1].algPtr); delete (yyvsp[0].algPtr); }
#line 1497 "grammar.tab.cpp"
    break;

  case 26: /* T: T MULT P  */
#line 80 "grammar.ypp"
             { (yyval.algPtr) = tarski::newMlt((yyvsp[-2].algPtr),(yyvsp[0].algPtr));            delete (yyvsp[-2].algPtr); delete (yyvsp[-1].tokPtr); delete (yyvsp[0].algPtr); }
#line 1503 "grammar.tab.cpp"
    break;

  case 27: /* T: T DIV P  */
#line 81 "grammar.ypp"
             { (yyval.algPtr) = tarski::newDiv((yyvsp[-2].algPtr),(yyvsp[0].algPtr));            delete (yyvsp[-2].algPtr); delete (yyvsp[-1].tokPtr); delete (yyvsp[0].algPtr); }
#line 1509 "grammar.tab.cpp"
    break;

  case 28: /* T: P  */
#line 82 "grammar.ypp"
      {  (yyval.algPtr) = tarski::copy((yyvsp[0].algPtr));                       delete (yyvsp[0].algPtr); }
#line 1515 "grammar.tab.cpp"
    break;

  case 29: /* P: MINUS P  */
#line 84 "grammar.ypp"
            { (yyval.algPtr) = tarski::newOpo((yyvsp[0].algPtr));                delete (yyvsp[-1].tokPtr); delete (yyvsp[0].algPtr); }
#line 1521 "grammar.tab.cpp"
    break;

  case 30: /* P: B EXP P  */
#line 85 "grammar.ypp"
            { (yyval.algPtr) = tarski::newExp((yyvsp[-2].algPtr),(yyvsp[0].algPtr));             delete (yyvsp[-2].algPtr); delete (yyvsp[-1].tokPtr); delete (yyvsp[0].algPtr); }
#line 1527 "grammar.tab.cpp"
    break;

  case 31: /* P: B  */
#line 86 "grammar.ypp"
      { (yyval.algPtr) = tarski::copy((yyvsp[0].algPtr));                        delete (yyvsp[0].algPtr);  }
#line 1533 "grammar.tab.cpp"
    break;

  case 32: /* Pb: B EXP P  */
#line 88 "grammar.ypp"
            { (yyval.algPtr) = tarski::newExp((yyvsp[-2].algPtr),(yyvsp[0].algPtr));             delete (yyvsp[-2].algPtr); delete (yyvsp[-1].tokPtr); delete (yyvsp[0].algPtr); }
#line 1539 "grammar.tab.cpp"
    break;

  case 33: /* Pb: B  */
#line 89 "grammar.ypp"
      { (yyval.algPtr) = tarski::copy((yyvsp[0].algPtr));                        delete (yyvsp[0].algPtr); }
#line 1545 "grammar.tab.cpp"
    break;

  case 34: /* B: VAR  */
#line 91 "grammar.ypp"
        { (yyval.algPtr) = tarski::newVar((yyvsp[0].tokPtr));                    delete (yyvsp[0].tokPtr);  }
#line 1551 "grammar.tab.cpp"
    break;

  case 35: /* B: NUM  */
#line 92 "grammar.ypp"
        { (yyval.algPtr) = tarski::newNum((yyvsp[0].tokPtr));                    delete (yyvsp[0].tokPtr);  }
#line 1557 "grammar.tab.cpp"
    break;

  case 36: /* B: LP E RP  */
#line 93 "grammar.ypp"
            {  (yyval.algPtr) = tarski::copy((yyvsp[-1].algPtr));                 delete (yyvsp[-2].tokPtr); delete (yyvsp[-1].algPtr); delete (yyvsp[0].tokPtr); }
#line 1563 "grammar.tab.cpp"
    break;


#line 1567 "grammar.tab.cpp"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == ALGEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (LC, result, YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= ALGEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == ALGEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, LC, result);
          yychar = ALGEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, LC, result);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (LC, result, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != ALGEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, LC, result);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, LC, result);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 95 "grammar.ypp"


  void algerror(tarski::LexContext *LC, tarski::TarskiRef &result, char const *msg)
  {
  }

