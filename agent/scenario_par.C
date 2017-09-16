/* A Bison parser, made from scenario_par.y
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

# define	STRING	257
# define	ERROR	258
# define	FLOAT	259
# define	UNUM	260
# define	INT	261
# define	O_PAREN	262
# define	C_PAREN	263
# define	COLON	264
# define	STEPS_DEFINITION	265
# define	BALL_OWNER	266
# define	BTO	267
# define	PNH	268
# define	O_BRACE	269
# define	C_BRACE	270
# define	NONE	271
# define	WBALL	272
# define	POS	273
# define	MARK	274
# define	LEAVE_COND	275
# define	IF	276
# define	GOTO	277
# define	END	278
# define	DRIBBLE	279
# define	CLEAR	280
# define	HOLD	281
# define	SCORE	282
# define	INITS	283
# define	INITTM	284
# define	EQUEL	285
# define	BPOS	286
# define	PPOS	287
# define	BOWNER	288
# define	AND	289
# define	OR	290
# define	NOT	291
# define	OUR	292
# define	OPP	293
# define	COMMA	294
# define	PALKA	295
# define	FASTEST_TM	296
# define	PT_SWEEPER	297
# define	PT_DEFENDER	298
# define	PT_MIDFIELDER	299
# define	PT_FORWARD	300
# define	ALL	301
# define	PS_CENTER	302
# define	PS_WINGWB	303
# define	PS_WINGNB	304
# define	PS_LEFT	305
# define	PS_RIGHT	306
# define	RECTANGLE	307
# define	CIRCLE	308
# define	ARC	309
# define	REG	310
# define	VECTOR	311
# define	BALL	312
# define	SCENARIO	313
# define	BALLX	314
# define	BALLY	315
# define	OFFSIDE	316
# define	TM1_X	317
# define	TM2_X	318
# define	TM3_X	319
# define	TM4_X	320
# define	TM5_X	321
# define	TM6_X	322
# define	TM7_X	323
# define	TM8_X	324
# define	TM1_Y	325
# define	TM2_Y	326
# define	TM3_Y	327
# define	TM4_Y	328
# define	TM5_Y	329
# define	TM6_Y	330
# define	TM7_Y	331
# define	TM8_Y	332
# define	END_NOW	333

#line 18 "scenario_par.y"

#define YYERROR_VERBOSE

#include <string>
using namespace std;
#include "scenario.h"

void yyerror (const char* s);
int yyerror (char* s);

#define YYPARSE_PARAM param

 ScenarioLexer&
 getParam( void* param )
 { 
   static ScenarioLexer* cached_param 
                  = reinterpret_cast< ScenarioLexer* >( param );
   if( cached_param != param )
     cached_param = reinterpret_cast< ScenarioLexer* >( param );
   return *cached_param;
 }

#define YYSTYPE ScenarioLexer::Holder

 inline
 int
 yylex( YYSTYPE* holder, ScenarioLexer& param )
 {
   int rval =  param.yylex();
   *holder=param.holder;
   return rval;
 }

#define YYLEX_PARAM getParam( param )

#define B getParam(param).builder

#ifndef YYSTYPE
# define YYSTYPE int
# define YYSTYPE_IS_TRIVIAL 1
#endif
#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		209
#define	YYFLAG		-32768
#define	YYNTBASE	80

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 333 ? yytranslate[x] : 124)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
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
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     3,     6,     8,    13,    16,    18,    20,    22,
      24,    27,    30,    33,    35,    39,    42,    44,    46,    48,
      50,    53,    56,    58,    61,    64,    66,    68,    70,    72,
      75,    78,    80,    85,    91,    96,    98,   101,   104,   106,
     110,   115,   120,   122,   125,   128,   130,   135,   140,   144,
     147,   149,   151,   153,   155,   157,   162,   170,   175,   180,
     185,   190,   198,   203,   206,   208,   212,   215,   217,   219,
     221,   227,   231,   233,   237,   239,   241,   243,   245,   247,
     249,   251,   253,   255,   257,   259,   261,   263,   269,   275,
     284,   289,   292,   294,   301,   306,   313,   321,   323,   325,
     327,   329,   331,   333,   335,   337,   339,   341,   343,   345,
     347,   349,   351,   353,   355,   357,   359,   361,   363
};
static const short yyrhs[] =
{
      81,    79,     0,    81,    82,     0,    82,     0,    59,     3,
       5,    83,     0,    83,    84,     0,    84,     0,    85,     0,
      91,     0,    86,     0,    29,   109,     0,    30,    87,     0,
      87,    88,     0,    88,     0,    89,    31,     6,     0,    89,
      90,     0,    90,     0,   114,     0,   119,     0,    42,     0,
      11,    92,     0,    92,    93,     0,    93,     0,     3,    94,
       0,    94,    95,     0,    95,     0,   103,     0,    96,     0,
      99,     0,    12,    97,     0,    97,    98,     0,    98,     0,
       8,    13,   111,     9,     0,     8,    13,   119,   106,     9,
       0,     8,    14,   111,     9,     0,    17,     0,    18,   100,
       0,   100,   101,     0,   101,     0,     6,    10,   102,     0,
       8,    19,   119,     9,     0,     8,    20,   111,     9,     0,
      17,     0,    21,   104,     0,   104,   105,     0,   105,     0,
      22,   109,    23,     3,     0,    22,   109,    23,    24,     0,
      15,   107,    16,     0,   107,   108,     0,   108,     0,    25,
       0,    26,     0,    27,     0,    28,     0,     8,    32,   119,
       9,     0,     8,    33,   114,     6,     6,   119,     9,     0,
       8,    34,   114,     9,     0,     8,    35,   110,     9,     0,
       8,    36,   110,     9,     0,     8,    37,   109,     9,     0,
       8,   123,   111,     6,     6,   119,     9,     0,     8,    34,
       6,     9,     0,   110,   109,     0,   109,     0,    15,   112,
      16,     0,   112,   113,     0,   113,     0,     6,     0,    47,
       0,     8,   115,    40,   116,     9,     0,   115,    41,   117,
       0,   117,     0,   116,    41,   118,     0,   118,     0,    43,
       0,    44,     0,    45,     0,    46,     0,    47,     0,    48,
       0,    49,     0,    50,     0,    52,     0,    51,     0,    47,
       0,   121,     0,     8,    53,   121,   121,     9,     0,     8,
      54,   121,     5,     9,     0,     8,    55,   121,     5,     5,
       5,     5,     9,     0,     8,    56,   120,     9,     0,   120,
     119,     0,   119,     0,    57,     8,   122,    40,   122,     9,
       0,    57,     8,    58,     9,     0,    57,     8,   123,    40,
       6,     9,     0,    57,     8,   122,    40,   122,   121,     9,
       0,     5,     0,    60,     0,    61,     0,    62,     0,    63,
       0,    64,     0,    65,     0,    66,     0,    67,     0,    68,
       0,    69,     0,    70,     0,    71,     0,    72,     0,    73,
       0,    74,     0,    75,     0,    76,     0,    77,     0,    78,
       0,    38,     0,    39,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,   126,   129,   130,   132,   135,   136,   138,   139,   140,
     142,   145,   147,   148,   150,   153,   154,   156,   158,   160,
     163,   165,   166,   168,   171,   172,   174,   175,   176,   178,
     181,   182,   184,   186,   188,   190,   193,   195,   196,   198,
     201,   203,   205,   209,   211,   212,   214,   216,   220,   222,
     224,   227,   228,   229,   230,   232,   234,   236,   238,   240,
     242,   244,   246,   249,   250,   252,   254,   255,   257,   259,
     262,   264,   265,   267,   268,   270,   272,   274,   276,   278,
     281,   283,   285,   287,   289,   291,   294,   296,   298,   300,
     302,   305,   306,   308,   310,   312,   314,   317,   319,   321,
     323,   325,   327,   329,   331,   333,   335,   337,   339,   341,
     343,   345,   347,   349,   351,   353,   355,   359,   361
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "STRING", "ERROR", "FLOAT", "UNUM", "INT", 
  "\"(\"", "\")\"", "\":\"", "\"Steps:\"", "\"Ball_owner:\"", 
  "\"ballto\"", "\"passnahod\"", "\"{\"", "\"}\"", "\"none\"", 
  "\"No_ball:\"", "\"pos\"", "\"mark\"", "\"Leave_conditions:\"", 
  "\"if\"", "\"goto\"", "\"end\"", "\"dribble\"", "\"clear\"", "\"hold\"", 
  "\"score\"", "\"Init_scenario:\"", "\"Init_teammates:\"", "\"=\"", 
  "\"bpos\"", "\"ppos\"", "\"bowner\"", "\"and\"", "\"or\"", "\"not\"", 
  "\"our\"", "\"opp\"", "\",\"", "\"|\"", "\"FastestTm\"", "\"Sweeper\"", 
  "\"Defender\"", "\"Midfielder\"", "\"Forward\"", "\"All\"", 
  "\"Center\"", "\"WingWB\"", "\"WingNB\"", "\"Left\"", "\"Right\"", 
  "\"rectangle\"", "\"circle\"", "\"arc\"", "\"reg\"", "\"Vector\"", 
  "\"ball\"", "\"Scenario:\"", "\"ball_x\"", "\"ball_y\"", "\"offside\"", 
  "TM1_X", "TM2_X", "TM3_X", "TM4_X", "TM5_X", "TM6_X", "TM7_X", "TM8_X", 
  "TM1_Y", "TM2_Y", "TM3_Y", "TM4_Y", "TM5_Y", "TM6_Y", "TM7_Y", "TM8_Y", 
  "END_NOW", "document", "list_of_scenario", "scenario", "scenario_list", 
  "scenario_tok", "init_check", "init_tm", "init_tm_list", 
  "init_tm_token", "init_tm_token_list", "init_tm_cond", "steps", 
  "steps_list", "step", "step_list", "step_token", "ballo", 
  "actionbo_list", "actionbo", "wball", "wball_list", "wball_token", 
  "action", "leave_cond", "leave_cond_list", "leave_cond_token", 
  "bmove_set", "bmove_list", "bmove_token", "condition", "condition_list", 
  "pl_set", "num_list", "num_token", "player_type", "pt", "ps", 
  "pt_token", "ps_token", "region", "region_list", "point", "value", 
  "team", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    80,    81,    81,    82,    83,    83,    84,    84,    84,
      85,    86,    87,    87,    88,    89,    89,    90,    90,    90,
      91,    92,    92,    93,    94,    94,    95,    95,    95,    96,
      97,    97,    98,    98,    98,    98,    99,   100,   100,   101,
     102,   102,   102,   103,   104,   104,   105,   105,   106,   107,
     107,   108,   108,   108,   108,   109,   109,   109,   109,   109,
     109,   109,   109,   110,   110,   111,   112,   112,   113,   113,
     114,   115,   115,   116,   116,   117,   117,   117,   117,   117,
     118,   118,   118,   118,   118,   118,   119,   119,   119,   119,
     119,   120,   120,   121,   121,   121,   121,   122,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   123,   123
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     2,     2,     1,     4,     2,     1,     1,     1,     1,
       2,     2,     2,     1,     3,     2,     1,     1,     1,     1,
       2,     2,     1,     2,     2,     1,     1,     1,     1,     2,
       2,     1,     4,     5,     4,     1,     2,     2,     1,     3,
       4,     4,     1,     2,     2,     1,     4,     4,     3,     2,
       1,     1,     1,     1,     1,     4,     7,     4,     4,     4,
       4,     7,     4,     2,     1,     3,     2,     1,     1,     1,
       5,     3,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     5,     5,     8,
       4,     2,     1,     6,     4,     6,     7,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       0,     0,     0,     3,     0,     1,     2,     0,     0,     0,
       0,     4,     6,     7,     9,     8,     0,    20,    22,     0,
      10,     0,    19,     0,    11,    13,     0,    16,    17,    18,
      86,     5,     0,     0,     0,    23,    25,    27,    28,    26,
      21,     0,     0,     0,     0,     0,     0,   117,   118,     0,
      75,    76,    77,    78,    79,     0,     0,     0,     0,     0,
      72,     0,    12,     0,    15,     0,    35,    29,    31,     0,
      36,    38,     0,    43,    45,    24,     0,     0,     0,     0,
       0,     0,    64,     0,     0,     0,     0,     0,     0,     0,
       0,    92,     0,     0,     0,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,     0,     0,    14,     0,
       0,    30,     0,    37,     0,    44,    55,     0,    62,    57,
      58,    63,    59,    60,    68,    69,     0,    67,     0,     0,
       0,     0,    90,    91,    85,    80,    81,    82,    84,    83,
       0,    74,    71,    94,     0,     0,     0,     0,     0,     0,
      42,    39,     0,     0,    65,    66,     0,    87,    88,     0,
      70,     0,     0,     0,    32,     0,     0,    34,     0,     0,
      46,    47,     0,     0,     0,    73,    93,     0,    95,    51,
      52,    53,    54,     0,    50,    33,     0,     0,    56,    61,
       0,    96,    48,    49,    40,    41,    89,     0,     0,     0
};

static const short yydefgoto[] =
{
     207,     2,     3,    11,    12,    13,    14,    24,    25,    26,
      27,    15,    17,    18,    35,    36,    37,    67,    68,    38,
      70,    71,   161,    39,    73,    74,   176,   193,   194,    82,
      83,    87,   136,   137,    28,    59,   150,    60,   151,    29,
      92,    30,   116,    49
};

static const short yypact[] =
{
     -38,    30,   -40,-32768,    33,-32768,-32768,    11,    32,    41,
       2,    11,-32768,-32768,-32768,-32768,     8,    32,-32768,    94,
  -32768,    60,-32768,    45,     2,-32768,     1,-32768,-32768,-32768,
  -32768,-32768,    47,    50,    53,     8,-32768,-32768,-32768,-32768,
  -32768,     6,    69,    19,    41,    41,    41,-32768,-32768,    67,
  -32768,-32768,-32768,-32768,-32768,    64,    64,    64,     6,   -10,
  -32768,    23,-32768,   111,-32768,   106,-32768,    47,-32768,   138,
      50,-32768,    41,    53,-32768,-32768,    85,   142,    65,   130,
     143,   144,-32768,   115,   126,   145,    18,   149,    64,   151,
     171,-32768,     3,    22,    65,-32768,   168,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,   139,   140,-32768,     0,
      67,-32768,    59,-32768,   155,-32768,-32768,   175,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,     7,-32768,   176,   174,
     177,   179,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
       9,-32768,-32768,-32768,    97,   181,   180,   170,   182,   127,
  -32768,-32768,    13,     6,-32768,-32768,     6,-32768,-32768,   183,
  -32768,    22,    -5,   184,-32768,   117,   185,-32768,     6,    67,
  -32768,-32768,   186,   187,   192,-32768,-32768,   189,-32768,-32768,
  -32768,-32768,-32768,    20,-32768,-32768,   190,   191,-32768,-32768,
     193,-32768,-32768,-32768,-32768,-32768,-32768,   201,   203,-32768
};

static const short yypgoto[] =
{
  -32768,-32768,   188,-32768,   194,-32768,-32768,-32768,   195,-32768,
     166,-32768,-32768,   196,-32768,   169,-32768,-32768,   141,-32768,
  -32768,   136,-32768,-32768,-32768,   134,-32768,-32768,    16,    -4,
     165,  -113,-32768,    75,   107,-32768,-32768,   118,    43,   -41,
  -32768,   -54,    61,   156
};


#define	YYLAST		219


static const short yytable[] =
{
      77,    88,    89,    90,   186,    20,   156,   158,    76,    21,
      21,    76,   142,   134,    76,    86,   180,    91,   170,     1,
      32,     1,     8,   164,   134,    80,    33,    78,    95,    34,
      93,    94,    63,     4,   139,    16,   202,   181,     7,     5,
       9,    10,    85,    22,    22,   189,   190,   191,   192,    19,
     171,   143,    23,    61,   135,    65,    69,    23,    23,    23,
      23,    47,    48,    23,    66,   135,   197,   159,   124,   144,
     145,   146,   147,   148,   149,    72,   160,    78,   157,   131,
     131,    96,    86,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,    95,    50,    51,    52,    53,    54,    50,    51,
      52,    53,    54,    55,    56,    57,    58,   118,   187,   119,
     120,    23,   182,    19,   130,   183,    41,    42,    43,    44,
      45,    46,    47,    48,    19,   132,   127,   196,    55,    56,
      57,    58,   189,   190,   191,   192,   178,   179,   122,    79,
      81,   126,   128,   129,   133,   138,   140,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   141,   153,   162,   154,
     155,   163,   166,   167,   169,   175,   168,   173,   184,   174,
       6,   177,    64,   188,   195,   198,   199,   200,   201,   204,
     205,   208,   206,   209,    75,    31,   123,   125,   121,   203,
      84,   165,   152,    40,   185,   172,     0,   117,     0,    62
};

static const short yycheck[] =
{
      41,    55,    56,    57,     9,     9,   119,   120,     8,     8,
       8,     8,     9,     6,     8,    15,     3,    58,     9,    59,
      12,    59,    11,    16,     6,     6,    18,     8,     5,    21,
      40,    41,    31,     3,    88,     3,    16,    24,     5,    79,
      29,    30,    46,    42,    42,    25,    26,    27,    28,     8,
      41,    92,    57,     8,    47,     8,     6,    57,    57,    57,
      57,    38,    39,    57,    17,    47,   179,     8,    72,    47,
      48,    49,    50,    51,    52,    22,    17,     8,   119,    83,
      84,    58,    15,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,     5,    43,    44,    45,    46,    47,    43,    44,
      45,    46,    47,    53,    54,    55,    56,     6,   172,    13,
      14,    57,   163,     8,     9,   166,    32,    33,    34,    35,
      36,    37,    38,    39,     8,     9,     6,   178,    53,    54,
      55,    56,    25,    26,    27,    28,    19,    20,    10,    42,
      43,     9,     9,     9,     9,     6,     5,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,     5,     9,    23,    40,
      40,     6,     6,     9,     5,    15,     9,     6,     5,     9,
       2,     9,    26,     9,     9,     9,     9,     5,     9,     9,
       9,     0,     9,     0,    35,    11,    70,    73,    67,   193,
      45,   136,    94,    17,   171,   154,    -1,    61,    -1,    24
};
#define YYPURE 1

/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#ifdef YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif
#endif

#line 315 "/usr/share/bison/bison.simple"


/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 1:
#line 127 "scenario_par.y"
{B.BuildScenarioSet();YYACCEPT;;
    break;}
case 4:
#line 133 "scenario_par.y"
{B.BuildScenario(yyvsp[-2].str,yyvsp[-1].fval);;
    break;}
case 10:
#line 143 "scenario_par.y"
{B.BuildInitCond();;
    break;}
case 14:
#line 151 "scenario_par.y"
{B.BuildInitTmFromList(yyvsp[0].val);;
    break;}
case 17:
#line 157 "scenario_par.y"
{B.BuildInitTmFromPt();;
    break;}
case 18:
#line 159 "scenario_par.y"
{B.BuildInitTmFromReg();;
    break;}
case 19:
#line 161 "scenario_par.y"
{B.BuildInitTmFromFastest();;
    break;}
case 23:
#line 169 "scenario_par.y"
{B.BuildStep(yyvsp[-1].str);;
    break;}
case 29:
#line 179 "scenario_par.y"
{B.BuildWithBallActions();;
    break;}
case 32:
#line 185 "scenario_par.y"
{B.BuildPTPAction();;
    break;}
case 33:
#line 187 "scenario_par.y"
{B.BuildMBAction();;
    break;}
case 34:
#line 189 "scenario_par.y"
{B.BuildPNHAction();;
    break;}
case 35:
#line 191 "scenario_par.y"
{B.BuildNoneWBAction();;
    break;}
case 39:
#line 199 "scenario_par.y"
{B.BuildNoBallActions(yyvsp[-2].val);;
    break;}
case 40:
#line 202 "scenario_par.y"
{B.BuildMTAction();;
    break;}
case 41:
#line 204 "scenario_par.y"
{B.BuildMarkAction();;
    break;}
case 42:
#line 206 "scenario_par.y"
{B.BuildNoneNBAction();;
    break;}
case 46:
#line 215 "scenario_par.y"
{B.BuildLeaveCondition(yyvsp[0].str);;
    break;}
case 47:
#line 217 "scenario_par.y"
{B.BuildLeaveCondition("end");;
    break;}
case 49:
#line 223 "scenario_par.y"
{B.AddBallMove(Action::BallMove(yyvsp[0].val));;
    break;}
case 50:
#line 225 "scenario_par.y"
{B.AddBallMove(Action::BallMove(yyvsp[0].val));;
    break;}
case 55:
#line 233 "scenario_par.y"
{B.BuildBposCondition();;
    break;}
case 56:
#line 235 "scenario_par.y"
{B.BuildPposCondition(yyvsp[-3].val,yyvsp[-2].val);;
    break;}
case 57:
#line 237 "scenario_par.y"
{B.BuildBownerCondition();;
    break;}
case 58:
#line 239 "scenario_par.y"
{B.BuildAndCondition();;
    break;}
case 59:
#line 241 "scenario_par.y"
{B.BuildOrCondition();;
    break;}
case 60:
#line 243 "scenario_par.y"
{B.BuildNotCondition();;
    break;}
case 61:
#line 245 "scenario_par.y"
{B.BuildPposExtCondition(yyvsp[-3].val,yyvsp[-2].val);;
    break;}
case 62:
#line 247 "scenario_par.y"
{B.BuildBownerCondition(yyvsp[-1].val);;
    break;}
case 68:
#line 258 "scenario_par.y"
{B.AddToPlayerSet(yyvsp[0].val);;
    break;}
case 69:
#line 260 "scenario_par.y"
{B.AddAllToPlayerSet();;
    break;}
case 75:
#line 271 "scenario_par.y"
{B.AddPlayerType(yyvsp[0].pt);;
    break;}
case 76:
#line 273 "scenario_par.y"
{B.AddPlayerType(yyvsp[0].pt);;
    break;}
case 77:
#line 275 "scenario_par.y"
{B.AddPlayerType(yyvsp[0].pt);;
    break;}
case 78:
#line 277 "scenario_par.y"
{B.AddPlayerType(yyvsp[0].pt);;
    break;}
case 79:
#line 279 "scenario_par.y"
{B.AddPlayerType(yyvsp[0].pt);;
    break;}
case 80:
#line 282 "scenario_par.y"
{B.AddPlayerSide(PTSType::center);;
    break;}
case 81:
#line 284 "scenario_par.y"
{B.AddPlayerSide(PTSType::wingwb);;
    break;}
case 82:
#line 286 "scenario_par.y"
{B.AddPlayerSide(PTSType::wingnb);;
    break;}
case 83:
#line 288 "scenario_par.y"
{B.AddPlayerSide(PTSType::right);;
    break;}
case 84:
#line 290 "scenario_par.y"
{B.AddPlayerSide(PTSType::left);;
    break;}
case 85:
#line 292 "scenario_par.y"
{B.AddPlayerSide(PTSType::all);;
    break;}
case 86:
#line 295 "scenario_par.y"
{B.BuildPointRegion();;
    break;}
case 87:
#line 297 "scenario_par.y"
{B.BuildRectangleRegion();;
    break;}
case 88:
#line 299 "scenario_par.y"
{B.BuildCircleRegion(yyvsp[-1].fval);;
    break;}
case 89:
#line 301 "scenario_par.y"
{B.BuildArcRegion(yyvsp[-4].fval,yyvsp[-3].fval,yyvsp[-2].fval,yyvsp[-1].fval);;
    break;}
case 90:
#line 303 "scenario_par.y"
{B.BuildRegRegion();;
    break;}
case 93:
#line 309 "scenario_par.y"
{B.BuildPoint();;
    break;}
case 94:
#line 311 "scenario_par.y"
{B.BuildPointFromBall();;
    break;}
case 95:
#line 313 "scenario_par.y"
{B.BuildPointFromPlayer(yyvsp[-1].val);;
    break;}
case 96:
#line 315 "scenario_par.y"
{B.BuildAddPoint();;
    break;}
case 97:
#line 318 "scenario_par.y"
{B.BuildValue(yyvsp[0].fval);;
    break;}
case 98:
#line 320 "scenario_par.y"
{B.BuildValueBx();;
    break;}
case 99:
#line 322 "scenario_par.y"
{B.BuildValueBy();;
    break;}
case 100:
#line 324 "scenario_par.y"
{B.BuildValueOffside();;
    break;}
case 101:
#line 326 "scenario_par.y"
{B.BuildValueTmX(1);;
    break;}
case 102:
#line 328 "scenario_par.y"
{B.BuildValueTmX(2);;
    break;}
case 103:
#line 330 "scenario_par.y"
{B.BuildValueTmX(3);;
    break;}
case 104:
#line 332 "scenario_par.y"
{B.BuildValueTmX(4);;
    break;}
case 105:
#line 334 "scenario_par.y"
{B.BuildValueTmX(5);;
    break;}
case 106:
#line 336 "scenario_par.y"
{B.BuildValueTmX(6);;
    break;}
case 107:
#line 338 "scenario_par.y"
{B.BuildValueTmX(7);;
    break;}
case 108:
#line 340 "scenario_par.y"
{B.BuildValueTmX(8);;
    break;}
case 109:
#line 342 "scenario_par.y"
{B.BuildValueTmY(1);;
    break;}
case 110:
#line 344 "scenario_par.y"
{B.BuildValueTmY(2);;
    break;}
case 111:
#line 346 "scenario_par.y"
{B.BuildValueTmY(3);;
    break;}
case 112:
#line 348 "scenario_par.y"
{B.BuildValueTmY(4);;
    break;}
case 113:
#line 350 "scenario_par.y"
{B.BuildValueTmY(5);;
    break;}
case 114:
#line 352 "scenario_par.y"
{B.BuildValueTmY(6);;
    break;}
case 115:
#line 354 "scenario_par.y"
{B.BuildValueTmY(7);;
    break;}
case 116:
#line 356 "scenario_par.y"
{B.BuildValueTmY(8);;
    break;}
case 117:
#line 360 "scenario_par.y"
{B.SetTeam(yyvsp[0].bval);;
    break;}
case 118:
#line 362 "scenario_par.y"
{B.SetTeam(yyvsp[0].bval);;
    break;}
}

#line 705 "/usr/share/bison/bison.simple"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
yyerrhandle:
  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

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

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 365 "scenario_par.y"



void yyerror (const char* s)
{
  cerr << s << endl;
  //do nothing
}

int yyerror (char* s)
{
	yyerror ( (const char*)s );
	return 0;
}
