/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

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
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "script.y" /* yacc.c:339  */


/******************************************************************
 *
 * Grammar rule for mkrom script.
 *
 * Filename: script.y
 * Author  : Kuoping Hsu, kuoping.hsu@ite.com.tw
 * Date    : 2012/06/12
 *
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "config.h"

#define YYSTYPE int
#undef  YYERROR_VERBOSE

//#define STANDALONE

extern int  yylex();
extern int  lines;
void yyrestart(FILE*);

int *cmd = NULL;
int  cmd_len;
char rem[MAX_REM_SIZE] = {0};

#ifdef STANDALONE
int  verbose = 1;
#else
extern int  verbose;
#endif

void yyerror(const char* msg);
void showMessage(char* msg);
char msg[1024];
static int stack[MAX_STACK_SIZE];
static int top = 0;

enum tag {
    WAIT_TAG       = 0,
    DATA_TAG       = 1,
    DATA_WAIT0_TAG = 2,
    DATA_WAIT1_TAG = 3,
    CALL_TAG       = 4,
    MASK_TAG       = 5,
    WRITE_MASK_TAG = 6,
    GOTO_TAG       = 7,
    READ_MASK_TAG  = 8,
    SKIP_TAG       = 9,
    BEQ_TAG        =10,
    BNE_TAG        =11,
    BGT_TAG        =12,
    BGTE_TAG       =13,
    BLT_TAG        =14,
    BLTE_TAG       =15,

    WRITE_TAG      =16,
    COMMENT_TAG    =17,
    BEQ_OP_TAG     =18,
    BNE_OP_TAG     =19,
    BGT_OP_TAG     =20,
    BGTE_OP_TAG    =21,
    BLT_OP_TAG     =22,
    BLTE_OP_TAG    =23,
    RESERVED       =0xffffffff
};

struct _op {
    char *name;
    int   tag;
    int   p;
} op[] = {
    {"WAIT"      , 0xffffffff, 1},
    {"DATA"      , 0xfffffffe, 5},
    {"DATA_WAIT0", 0xfffffffd, 2},
    {"DATA_WAIT1", 0xfffffffc, 2},
    {"CALL"      , 0xfffffffb, 1},
    {"MASK"      , 0xfffffffa, 3},
    {"WRITE_MASK", 0xfffffffa, 3},
    {"GOTO"      , 0xfffffff9, 1},
    {"READ_MASK" , 0xfffffff8, 2},
    {"SKIP"      , 0xfffffff7, 1},
    {"BEQ"       , 0xffffffe0, 2},
    {"BNE"       , 0xffffffe1, 2},
    {"BGT"       , 0xffffffe2, 2},
    {"BGTE"      , 0xffffffe3, 2},
    {"BLT"       , 0xffffffe4, 2},
    {"BLTE"      , 0xffffffe5, 2},
    {"WRITE"     , 0x00000000, 2},

    {"COMMENT"   , 0x00000000, 0},
    {"=="        , 0x00000000, 0},
    {"!="        , 0x00000000, 0},
    {">"         , 0x00000000, 0},
    {">="        , 0x00000000, 0},
    {"<"         , 0x00000000, 0},
    {"<="        , 0x00000000, 0},
    {""          , 0x00000000, 0},
};

static void push(int tag);
static int pop(void);
void dump(FILE *fp);


#line 178 "script.tab.c" /* yacc.c:339  */

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
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "script.tab.h".  */
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

#endif /* !YY_YY_SCRIPT_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 273 "script.tab.c" /* yacc.c:358  */

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
#define YYFINAL  30
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   168

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  50
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  16
/* YYNRULES -- Number of rules.  */
#define YYNRULES  64
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  105

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   298

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      47,    48,     2,     2,    49,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    44,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    45,     2,    46,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41,    42,    43
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   131,   131,   132,   133,   137,   138,   139,   146,   156,
     174,   186,   211,   248,   273,   339,   410,   414,   435,   481,
     482,   483,   484,   485,   486,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   512,   513,   517,   518,   522,   523,
     527,   528,   529,   533,   534,   535,   539,   540,   541,   542,
     546,   547,   548,   552,   556
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NUMBER", "STRING", "BEQ", "BGT", "BGTE",
  "BLT", "BLTE", "BNE", "CALL", "COMMENT", "DATA", "DATA_WAIT0",
  "DATA_WAIT1", "GOTO", "MASK", "WRITE_MASK", "PCI_WRITE", "READ_MASK",
  "SKIP", "WAIT", "WRITE", "IF", "ELSE", "BEQ_OP", "BNE_OP", "BGT_OP",
  "BGTE_OP", "BLT_OP", "BLTE_OP", "MOD", "RIGHTSHIFT", "LEFTSHIFT", "PLUS",
  "MINUS", "DIV", "MUL", "UNARYMINUS", "COMPLEMENT", "XOR", "OR", "AND",
  "';'", "'{'", "'}'", "'('", "')'", "','", "$accept", "root", "statement",
  "if_statement", "expression", "cond_expression", "cond_op", "command",
  "number", "exclusive_or_expr", "and_expr", "shift_expr", "add_expr",
  "mul_expr", "unary_expr", "digit", YY_NULLPTR
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
     295,   296,   297,   298,    59,   123,   125,    40,    41,    44
};
# endif

#define YYPACT_NINF -38

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-38)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     123,   -38,   -38,   -38,   -38,   -38,   -38,   -38,    -3,   -38,
     -38,   -38,   -38,   -38,   -38,   -38,   -38,   -38,   -38,   -38,
     -14,   123,    61,   -38,    13,   -30,    17,   -38,    -1,    81,
     -38,   -38,   123,   -38,     5,   -38,    60,     4,     4,     5,
     -37,     7,     1,    75,    26,   -11,    -6,   -38,   -38,   -38,
     -38,   -36,     5,   -38,   -38,    -5,   123,     5,     5,     5,
       5,     5,     5,     5,     5,     5,     5,   -38,     0,   -20,
     -38,   -38,     1,    75,    26,   -11,   -11,    -6,    -6,   -38,
     -38,   -38,    71,   -33,     5,   -38,   -38,     5,     6,   -21,
      82,   -38,     5,   -38,   -38,   -38,   -38,   -38,   -38,     5,
     -19,     7,   116,    73,   -38
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
       0,     4,    16,     2,     7,     0,     0,    10,     0,    16,
       1,     3,    16,     5,     0,    63,     0,     0,     0,     0,
       0,    17,    44,    46,    48,    50,    53,    56,    60,     6,
       8,     0,     0,    61,    62,     0,    16,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,     0,     0,
      64,     9,    45,    47,    49,    52,    51,    54,    55,    59,
      58,    57,     0,     0,     0,    14,    12,     0,     0,     0,
       0,    13,     0,    19,    20,    21,    22,    23,    24,     0,
       0,    18,     0,     0,    15
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -38,   101,   -12,   -38,   -38,   -38,   -38,   -38,   -34,    66,
      67,    65,     2,    52,    -9,    79
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    22,    23,    24,    25,    40,    99,    26,    41,    42,
      43,    44,    45,    46,    47,    48
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      51,    27,    35,    35,    82,    55,    57,    35,    35,    57,
      31,    56,    67,    68,    33,    86,    87,    31,    69,    36,
      50,    57,    57,    57,    62,    63,    64,    91,    92,    84,
     102,    65,    66,    28,    83,    37,    37,    57,    32,    38,
      38,    37,    58,    70,    71,    38,    39,    39,    57,    57,
      88,    39,    39,    89,    90,    79,    80,    81,   100,    60,
      61,    30,    75,    76,    34,   101,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    52,    93,    94,
      95,    96,    97,    98,    77,    78,    53,    54,    59,    85,
     103,   104,    29,    72,    74,    73,    21,    49,     1,     2,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    21
};

static const yytype_int8 yycheck[] =
{
      34,     4,     3,     3,     4,    39,    42,     3,     3,    42,
      22,    48,    48,    49,    44,    48,    49,    29,    52,    20,
      32,    42,    42,    42,    35,    36,    32,    48,    49,    49,
      49,    37,    38,    47,    68,    36,    36,    42,    25,    40,
      40,    36,    41,    48,    56,    40,    47,    47,    42,    42,
      84,    47,    47,    87,    48,    64,    65,    66,    92,    33,
      34,     0,    60,    61,    47,    99,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    45,    47,    26,    27,
      28,    29,    30,    31,    62,    63,    37,    38,    43,    48,
       4,    48,    21,    57,    59,    58,    45,    46,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    45,    51,    52,    53,    54,    57,     4,    47,    51,
       0,    52,    25,    44,    47,     3,    20,    36,    40,    47,
      55,    58,    59,    60,    61,    62,    63,    64,    65,    46,
      52,    58,    47,    65,    65,    58,    48,    42,    41,    43,
      33,    34,    35,    36,    32,    37,    38,    48,    49,    58,
      48,    52,    59,    60,    61,    62,    62,    63,    63,    64,
      64,    64,     4,    58,    49,    48,    48,    49,    58,    58,
      48,    48,    49,    26,    27,    28,    29,    30,    31,    56,
      58,    58,    49,     4,    48
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    50,    51,    51,    51,    52,    52,    52,    52,    53,
      54,    54,    54,    54,    54,    54,    54,    55,    55,    56,
      56,    56,    56,    56,    56,    57,    57,    57,    57,    57,
      57,    57,    57,    57,    57,    57,    57,    57,    57,    57,
      57,    57,    57,    57,    58,    58,    59,    59,    60,    60,
      61,    61,    61,    62,    62,    62,    63,    63,    63,    63,
      64,    64,    64,    65,    65
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     0,     2,     3,     1,     3,     5,
       2,     4,     6,     8,     6,    12,     0,     1,     8,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     3,     1,     3,
       1,     3,     3,     1,     3,     3,     1,     3,     3,     3,
       1,     2,     2,     1,     3
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
        case 7:
#line 140 "script.y" /* yacc.c:1646  */
    {
        int n = pop();
        cmd[n] = cmd_len - n - 1;

        if (verbose) printf("#%d/%04d: update index %d to %d.\n", lines, cmd_len, n, cmd[n]);
    }
#line 1441 "script.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 147 "script.y" /* yacc.c:1646  */
    {
        int n = pop();
        cmd[n] = cmd_len - n - 1;

        if (verbose) printf("#%d/%04d: update index %d to %d.\n", lines, cmd_len, n, cmd[n]);
    }
#line 1452 "script.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 157 "script.y" /* yacc.c:1646  */
    {
        int n = pop();
        cmd[n] = cmd_len - n - 1 + (op[SKIP_TAG].p + 1);

        if (verbose) {
            printf("#%d/%04d: update index %d to %d.\n", lines, cmd_len, n, cmd[n]);
            printf("#%d/%04d: SKIP (XXXX);\n", lines, cmd_len);
        }

        cmd[cmd_len++] = op[SKIP_TAG].tag;
        push(cmd_len);
        if (verbose) printf("#%d/%04d: push %d;\n", lines, cmd_len, cmd_len);
        cmd[cmd_len++] = -1;
    }
#line 1471 "script.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 175 "script.y" /* yacc.c:1646  */
    {
        char date[128];
        time_t timep;

        time(&timep);
        sprintf(date, "%s", ctime(&timep));
        if (date[strlen(date)-1] == '\n') date[strlen(date)-1] = 0;
        sprintf(rem, (const char*)(yyvsp[0]), date);

        if (verbose) printf("#%d/%04d: %s %s;\n", lines, cmd_len, op[COMMENT_TAG].name, (char*)(yyvsp[0]));
    }
#line 1487 "script.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 187 "script.y" /* yacc.c:1646  */
    {
        if (verbose)
            printf("#%d/%04d: %s (0x%08x);\n", lines, cmd_len, op[(yyvsp[-3])].name, (yyvsp[-1]));

        if (op[(yyvsp[-3])].p != 1) {
            sprintf(msg, "syntax error, command %s has 1 parameter.", op[(yyvsp[-3])].name);
            yyerror(msg);
            YYERROR;
        }

        switch ((yyvsp[-3])) {
            case WAIT_TAG:
            case CALL_TAG:
            case GOTO_TAG:
            case SKIP_TAG:
                          cmd[cmd_len++] = op[(yyvsp[-3])].tag;
                          cmd[cmd_len++] = (yyvsp[-1]);
                          break;
            default:
                          sprintf(msg, "syntax error, unknown command %s.", op[(yyvsp[-3])].name);
                          yyerror(msg);
                          YYERROR;
        }
    }
#line 1516 "script.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 212 "script.y" /* yacc.c:1646  */
    {
        if (verbose)
            printf("#%d/%04d: %s (0x%08x, 0x%08x);\n", lines, cmd_len, op[(yyvsp[-5])].name, (yyvsp[-3]), (yyvsp[-1]));

        if (op[(yyvsp[-5])].p != 2) {
            sprintf(msg, "syntax error, command %s has 2 parameters.", op[(yyvsp[-5])].name);
            yyerror(msg);
            YYERROR;
        }

        switch ((yyvsp[-5])) {
            case DATA_WAIT0_TAG:
            case DATA_WAIT1_TAG:
            case READ_MASK_TAG:
            case BEQ_TAG:
            case BNE_TAG:
            case BGT_TAG:
            case BGTE_TAG:
            case BLT_TAG:
            case BLTE_TAG:
                          cmd[cmd_len++] = op[(yyvsp[-5])].tag;
                          cmd[cmd_len++] = (yyvsp[-3]);
                          cmd[cmd_len++] = (yyvsp[-1]);
                          break;

            case WRITE_TAG:
                          cmd[cmd_len++] = (yyvsp[-3]);
                          cmd[cmd_len++] = (yyvsp[-1]);
                          break;

            default:
                          sprintf(msg, "syntax error, unknown command %s.", op[(yyvsp[-5])].name);
                          yyerror(msg);
                          YYERROR;
        }
    }
#line 1557 "script.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 249 "script.y" /* yacc.c:1646  */
    {
        if (verbose)
            printf("#%d/%04d: %s (0x%08x, 0x%08x, 0x%08x);\n", lines, cmd_len, op[(yyvsp[-7])].name, (yyvsp[-5]), (yyvsp[-3]), (yyvsp[-1]));

        if (op[(yyvsp[-7])].p != 3) {
            sprintf(msg, "syntax error, command %s has 3 parameters.", op[(yyvsp[-7])].name);
            yyerror(msg);
            YYERROR;
        }

        switch ((yyvsp[-7])) {
            case MASK_TAG:
            case WRITE_MASK_TAG:
                          cmd[cmd_len++] = op[(yyvsp[-7])].tag;
                          cmd[cmd_len++] = (yyvsp[-5]);
                          cmd[cmd_len++] = (yyvsp[-3]);
                          cmd[cmd_len++] = (yyvsp[-1]);
                          break;
            default:
                          sprintf(msg, "syntax error, unknown command %s.", op[(yyvsp[-7])].name);
                          yyerror(msg);
                          YYERROR;
        }
    }
#line 1586 "script.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 274 "script.y" /* yacc.c:1646  */
    {
        if (verbose)
            printf("#%d/%04d: %s (0x%08x, %s);\n", lines, cmd_len, op[(yyvsp[-5])].name, (yyvsp[-3]), (char*)(yyvsp[-1]));

        if ((yyvsp[-5]) != DATA_TAG) {
            sprintf(msg, "syntax error, unknown command %s.", op[(yyvsp[-5])].name);
            yyerror(msg);
            YYERROR;
        }

        {
            FILE *fp = NULL;
            int   addr   = (yyvsp[-3]);
            int   height = 1;
            int   width  = 0;
            int   pitch  = 0;
            char *fname  = (char*)(yyvsp[-1]);
            int   fsize;

            if ( (addr  % 4) != 0 ) { yyerror("Data start address is not 32-bit alignment."); YYERROR; }

            if ((fp = fopen(fname, "rb")) == NULL) {
                sprintf(msg, "Can not open '%s' file.\n", fname);
                showMessage(msg);
                YYERROR;
            }

            fseek(fp, 0, SEEK_END);
            fsize=ftell(fp);
            fseek(fp, 0, SEEK_SET);

            // the width & pitch must be 32 bit-alignment
            width = ((int)((fsize + 3) / 4)) * 4;
            pitch = width;

            cmd[cmd_len++] = op[(yyvsp[-5])].tag;
            cmd[cmd_len++] = addr;
            cmd[cmd_len++] = height;
            cmd[cmd_len++] = width;
            cmd[cmd_len++] = pitch;

            if (fsize == 0) {
                sprintf(msg, "Zero size of file '%s'\n", fname);
                showMessage(msg);
                YYERROR;
            }

            if (width - fsize != 0) {
                sprintf(msg, "File '%s' size is not 32-bit alignment, it will write extra %d bytes to memory.\n", fname, width - fsize);
                showMessage(msg);
            }

            if (cmd_len*4+fsize > MAX_CMD_SIZE) {
                sprintf(msg, "Command size (%d) is larger than MAX_CMD_SIZE (%d)\n", (int)(cmd_len*4+fsize), (int)MAX_CMD_SIZE);
                showMessage(msg);
                YYERROR;
            }

            /* little endian read on x86 */
            fread(&cmd[cmd_len], sizeof(char), fsize, fp);
            fclose(fp);

            cmd_len += (width/4);
        }
    }
#line 1656 "script.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 340 "script.y" /* yacc.c:1646  */
    {
        if (verbose)
            printf("#%d/%04d: %s (0x%08x, 0x%08x, 0x%08x, 0x%08x, %s);\n", lines, cmd_len, op[(yyvsp[-11])].name, (yyvsp[-9]), (yyvsp[-7]), (yyvsp[-5]), (yyvsp[-3]), (char*)(yyvsp[-1]));

        if (op[(yyvsp[-11])].p != 5) {
            sprintf(msg, "syntax error, command %s has 5 parameters.", op[(yyvsp[-11])].name);
            yyerror(msg);
            YYERROR;
        }

        if ((yyvsp[-11]) != DATA_TAG) {
            sprintf(msg, "syntax error, unknown command %s.", op[(yyvsp[-11])].name);
            yyerror(msg);
            YYERROR;
        }

        {
            FILE *fp = NULL;
            int   addr   = (yyvsp[-9]);
            int   height = (yyvsp[-7]);
            int   width  = (yyvsp[-5]);
            int   pitch  = (yyvsp[-3]);
            char *fname  = (char*)(yyvsp[-1]);
            int   fsize;

            if (height <= 0 || width <= 0 || pitch < width) {
                if (height <= 0) { yyerror("WARNING!! DATA height is less or equal to zero.\n"); YYERROR; }
                if (width  <= 0) { yyerror("WARNING!! DATA width is less or equal to zero.\n"); YYERROR; }
                if (pitch < width) { yyerror("WARNING!! DATA pitch is less than width.\n"); YYERROR; }
            } else {
                cmd[cmd_len++] = op[(yyvsp[-11])].tag;
                cmd[cmd_len++] = addr;
                cmd[cmd_len++] = height;
                cmd[cmd_len++] = width;
                cmd[cmd_len++] = pitch;

                if ( (addr  % 4) != 0 ) { yyerror("Data start address is not 32-bit alignment."); YYERROR; }
                if ( (width % 4) != 0 ) { yyerror("Data width is not 32-bit alignment."); YYERROR; }
                if ( (pitch % 4) != 0 ) { yyerror("Data pitch is not 32-bit alignment."); YYERROR; }

                if ((fp = fopen(fname, "rb")) == NULL) {
                    sprintf(msg, "Can not open '%s' file.\n", fname);
                    showMessage(msg);
                    YYERROR;
                }

                fseek(fp, 0, SEEK_END);
                fsize=ftell(fp);
                fseek(fp, 0, SEEK_SET);

                if (fsize != height*width) {
                    sprintf(msg, "File %s size (%d bytes) is not equal to width*height (%d * %d).\n", fname, fsize, width, height);
                    showMessage(msg);
                    YYERROR;
                }

                if (cmd_len*4+fsize > MAX_CMD_SIZE) {
                    sprintf(msg, "Command size (%d) is larger than MAX_CMD_SIZE (%d)\n", (int)(cmd_len*4+fsize), (int)MAX_CMD_SIZE);
                    showMessage(msg);
                    YYERROR;
                }

                /* little endian read on x86 */
                fread(&cmd[cmd_len], sizeof(int), fsize/4, fp);
                fclose(fp);

                cmd_len += (fsize/4);
            }
        }
    }
#line 1731 "script.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 415 "script.y" /* yacc.c:1646  */
    {
        if (verbose) printf("#%d/%04d: %s (0, 0, 0);\n", lines, cmd_len, op[READ_MASK_TAG].name);

        cmd[cmd_len++] = op[READ_MASK_TAG].tag;
        cmd[cmd_len++] = 0;
        cmd[cmd_len++] = 0;

        if ((yyvsp[0])) {
            cmd[cmd_len++] = op[BNE_TAG].tag;
            if (verbose) printf("#%d/%04d: BNE (0x%08x, XXXX);\n", lines, cmd_len, 0);
        } else {
            cmd[cmd_len++] = op[BEQ_TAG].tag;
            if (verbose) printf("#%d/%04d: BEQ (0x%08x, XXXX);\n", lines, cmd_len, 0);
        }

        cmd[cmd_len++] = 0;
        push(cmd_len);
        if (verbose) printf("#%d/%04d: push %d;\n", lines, cmd_len, cmd_len);
        cmd[cmd_len++] = -1;
    }
#line 1756 "script.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 436 "script.y" /* yacc.c:1646  */
    {
        int cmp = (yyvsp[-1]);
        int val = (yyvsp[0]);

        if (verbose) printf("#%d/%04d: %s (0x%08x, 0x%08x) %s 0x%08x;\n", lines, cmd_len, op[READ_MASK_TAG].name, (yyvsp[-5]), (yyvsp[-3]), op[cmp].name, val);

        cmd[cmd_len++] = op[READ_MASK_TAG].tag;
        cmd[cmd_len++] = (yyvsp[-5]);
        cmd[cmd_len++] = (yyvsp[-3]);

        if (verbose) {
            switch(cmp) {
                case BEQ_OP_TAG : printf("#%d/%04d: BNE (0x%08x, XXXX);\n", lines, cmd_len, val); break;
                case BNE_OP_TAG : printf("#%d/%04d: BEQ (0x%08x, XXXX);\n", lines, cmd_len, val); break;
                case BGT_OP_TAG : printf("#%d/%04d: BLTE (0x%08x, XXXX);\n", lines, cmd_len, val); break;
                case BGTE_OP_TAG: printf("#%d/%04d: BLT (0x%08x, XXXX);\n", lines, cmd_len, val); break;
                case BLT_OP_TAG : printf("#%d/%04d: BGTE (0x%08x, XXXX);\n", lines, cmd_len, val); break;
                case BLTE_OP_TAG: printf("#%d/%04d: BGT (0x%08x, XXXX);\n", lines, cmd_len, val); break;
                default:
                       yyerror("Unknown conditional operation");
                       YYERROR;
            }

        }

        switch(cmp) {
            case BEQ_OP_TAG : cmd[cmd_len++] = op[BNE_TAG].tag; break;
            case BNE_OP_TAG : cmd[cmd_len++] = op[BEQ_TAG].tag; break;
            case BGT_OP_TAG : cmd[cmd_len++] = op[BLTE_TAG].tag; break;
            case BGTE_OP_TAG: cmd[cmd_len++] = op[BLT_TAG].tag; break;
            case BLT_OP_TAG : cmd[cmd_len++] = op[BGTE_TAG].tag; break;
            case BLTE_OP_TAG: cmd[cmd_len++] = op[BGT_TAG].tag; break;
            default:
                   yyerror("Unknown conditional operation");
                   YYERROR;
        }

        cmd[cmd_len++] = val;
        push(cmd_len);
        if (verbose) printf("#%d/%04d: push %d;\n", lines, cmd_len, cmd_len);
        cmd[cmd_len++] = -1;
    }
#line 1803 "script.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 481 "script.y" /* yacc.c:1646  */
    { (yyval) = BEQ_OP_TAG    ; }
#line 1809 "script.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 482 "script.y" /* yacc.c:1646  */
    { (yyval) = BNE_OP_TAG    ; }
#line 1815 "script.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 483 "script.y" /* yacc.c:1646  */
    { (yyval) = BGT_OP_TAG    ; }
#line 1821 "script.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 484 "script.y" /* yacc.c:1646  */
    { (yyval) = BGTE_OP_TAG   ; }
#line 1827 "script.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 485 "script.y" /* yacc.c:1646  */
    { (yyval) = BLT_OP_TAG    ; }
#line 1833 "script.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 486 "script.y" /* yacc.c:1646  */
    { (yyval) = BLTE_OP_TAG   ; }
#line 1839 "script.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 490 "script.y" /* yacc.c:1646  */
    { (yyval) = BEQ_TAG       ; }
#line 1845 "script.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 491 "script.y" /* yacc.c:1646  */
    { (yyval) = BGT_TAG       ; }
#line 1851 "script.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 492 "script.y" /* yacc.c:1646  */
    { (yyval) = BGTE_TAG      ; }
#line 1857 "script.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 493 "script.y" /* yacc.c:1646  */
    { (yyval) = BLT_TAG       ; }
#line 1863 "script.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 494 "script.y" /* yacc.c:1646  */
    { (yyval) = BLTE_TAG      ; }
#line 1869 "script.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 495 "script.y" /* yacc.c:1646  */
    { (yyval) = BNE_TAG       ; }
#line 1875 "script.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 496 "script.y" /* yacc.c:1646  */
    { (yyval) = CALL_TAG      ; }
#line 1881 "script.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 497 "script.y" /* yacc.c:1646  */
    { (yyval) = COMMENT_TAG   ; }
#line 1887 "script.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 498 "script.y" /* yacc.c:1646  */
    { (yyval) = DATA_TAG      ; }
#line 1893 "script.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 499 "script.y" /* yacc.c:1646  */
    { (yyval) = DATA_WAIT0_TAG; }
#line 1899 "script.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 500 "script.y" /* yacc.c:1646  */
    { (yyval) = DATA_WAIT1_TAG; }
#line 1905 "script.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 501 "script.y" /* yacc.c:1646  */
    { (yyval) = GOTO_TAG      ; }
#line 1911 "script.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 502 "script.y" /* yacc.c:1646  */
    { (yyval) = MASK_TAG      ; }
#line 1917 "script.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 503 "script.y" /* yacc.c:1646  */
    { (yyval) = WRITE_MASK_TAG; }
#line 1923 "script.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 504 "script.y" /* yacc.c:1646  */
    { (yyval) = WRITE_TAG     ; }
#line 1929 "script.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 505 "script.y" /* yacc.c:1646  */
    { (yyval) = READ_MASK_TAG ; }
#line 1935 "script.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 506 "script.y" /* yacc.c:1646  */
    { (yyval) = SKIP_TAG      ; }
#line 1941 "script.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 507 "script.y" /* yacc.c:1646  */
    { (yyval) = WAIT_TAG      ; }
#line 1947 "script.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 508 "script.y" /* yacc.c:1646  */
    { (yyval) = WRITE_TAG     ; }
#line 1953 "script.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 513 "script.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) | (yyvsp[0]); }
#line 1959 "script.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 518 "script.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) ^ (yyvsp[0]); }
#line 1965 "script.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 523 "script.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) & (yyvsp[0]); }
#line 1971 "script.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 528 "script.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) << (yyvsp[0]); }
#line 1977 "script.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 529 "script.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) >> (yyvsp[0]); }
#line 1983 "script.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 534 "script.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[0]); }
#line 1989 "script.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 535 "script.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) - (yyvsp[0]); }
#line 1995 "script.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 540 "script.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) * (yyvsp[0]); }
#line 2001 "script.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 541 "script.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) / (yyvsp[0]); }
#line 2007 "script.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 542 "script.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) % (yyvsp[0]); }
#line 2013 "script.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 547 "script.y" /* yacc.c:1646  */
    { (yyval) = -(yyvsp[0]); }
#line 2019 "script.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 548 "script.y" /* yacc.c:1646  */
    { (yyval) = ~(yyvsp[0]); }
#line 2025 "script.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 553 "script.y" /* yacc.c:1646  */
    {
        (yyval) = (yyvsp[0]);
    }
#line 2033 "script.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 557 "script.y" /* yacc.c:1646  */
    {
        (yyval) = (yyvsp[-1]);
    }
#line 2041 "script.tab.c" /* yacc.c:1646  */
    break;


#line 2045 "script.tab.c" /* yacc.c:1646  */
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
#line 562 "script.y" /* yacc.c:1906  */

extern FILE *yyin;
static char *filename;
#if 0 //def STANDALONE
int main(int argc, char **argv)
#else
int read_scr(char *fname)
#endif
{
    FILE *fp = NULL;
    int result = 0;

#ifdef STANDALONE
    if (argc != 2) {
        yyin = stdin;
        filename = "stdin";
    } else {
        filename = argv[1];
        if ((fp = fopen(argv[1], "r")) == NULL) {
            sprintf(msg, "Can not open file %s\n", argv[1]);
            showMessage(msg);
            return -1;
        }
        yyin = fp;
    }
#else
    filename = fname;
    if ((fp = fopen(fname, "r")) == NULL) {
        sprintf(msg, "Can not open file %s\n", fname);
        showMessage(msg);
        return -1;
    }
    yyin = fp;
#endif

    if ((cmd = malloc(MAX_CMD_SIZE)) == NULL) {
        sprintf(msg, "Out of memory!!\n");
        showMessage(msg);
        return -1;
    }

    cmd_len = 0;
    lines   = 1;

    yyrestart(yyin);
    if ((result=yyparse()) == 0) {
        fclose(fp);

        if (verbose) dump(stdout);

        if (rem[0] == 0) {
            char date[128];
            time_t timep;
            time(&timep);
            sprintf(date, "%s", ctime(&timep));
            if (date[strlen(date)-1] == '\n') date[strlen(date)-1] = 0;
            sprintf(rem, "(c) 2009 ITE Tech. Corp. %s", date);
            if (verbose) printf("Copyright info: %s\n", rem);
        }
    }

    /* error handling */
    if (cmd_len*4 >= MAX_CMD_SIZE) {
        sprintf(msg, "[ERROR] command out of range.\n");
        showMessage(msg);
        return -1;
    }

    return result;
}

void yyerror(const char* msg)
{
    static char str[1024];
    sprintf(str, "Error: %s:%d: %s\n", filename, lines, msg);
    showMessage(str);
}

static void push(int tag)
{
    if (top >= MAX_STACK_SIZE-1)
        yyerror("Stack out of range.");

    stack[top++] = tag;
}

static int pop(void)
{
    if (top <= 0)
        yyerror("Stack under run.");

    return stack[--top];
}

void dump(FILE *fp)
{
    int idx = 0;

    while (idx < cmd_len)
    {
        int i;
        int cnt = 0;

        for(i=0; i < (sizeof(op) / (3*sizeof(int))); i++)
            if (op[i].tag == cmd[idx]) break;

        if (i == (sizeof(op) / (3*sizeof(int)))) {
            fprintf(fp, "%d: WRITE (0x%08x, 0x%08x);\n", idx, cmd[idx], cmd[idx+1]);
            idx += 2;
        } else {
            int j;
            int n;

            fprintf(fp, "%d: %s (", idx, op[i].name);

            n = op[i].p;
            if (n == 5) n--;

            for(j=0; j<n; j++) {
                fprintf(fp, "0x%08x", cmd[idx+j+1]);
                if (j != (n-1)) fprintf(fp, ", ");
            }

            fprintf(fp, ");\n");

            if (n == 4) cnt = (cmd[idx+2] * cmd[idx+3] / sizeof(int));

            idx += (cnt + n + 1);
        }
    }
}

//void showMessage(char *str) {
//    fprintf(stderr, str);
//}

