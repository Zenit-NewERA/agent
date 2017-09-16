/* -*-flex-*- */
%{

#include "scenario.h"
#include <string>
#include "scenario_par.h"
using namespace std;

%}

%option nomain
%option noyywrap
%option c++
%option yyclass="ScenarioLexer"

%x COMMENT

digit  [0-9]
unum   {digit}+
int    [\+\-]?{unum}+
float   [\+\-]?{unum}?\.{digit}*
%%

"(" {return O_PAREN;}
")" {return C_PAREN;}
"Steps:" {return STEPS_DEFINITION;}
"Ball_owner:" {return BALL_OWNER;}
"ballto" {return BTO;} 
"passnahod" {return PNH;}
"{"      {return O_BRACE;}
"}"			 {return C_BRACE;}
":"      {return COLON;}
"none"   {return NONE;}
"No_ball:" {return WBALL;}
"pos"    {return POS;}
"mark"   {return MARK;} 
"Leave_conditions:" {return LEAVE_COND;} 
"if"     {return IF;}
"goto"   {return GOTO;}
"end"    {return END;}
"dribble" {holder.val=int(Action::dribble);return DRIBBLE;}
"clear"   {holder.val=int(Action::clear);return CLEAR;}
"hold" {holder.val=int(Action::hold);return HOLD;}
"score" {holder.val=int(Action::score);return SCORE;}
"Init_scenario:" {return INITS;}
"Init_teammates:" {return INITTM;}
"="       { return EQUEL;}
"bpos"    {return BPOS ;}
"ppos"    {return PPOS ;}
"bowner"  {return BOWNER ;}
"and"     {return AND ;}
"or"      {return OR ;}
"not"     {return NOT ;}
"our"     {holder.bval=true;return OUR ;}
"opp"     {holder.bval=false;return OPP ;}
","       {return COMMA ;}
"|"       {return PALKA ;}
"FastestTm" {return FASTEST_TM;}

"Sweeper" {holder.pt=PT_Sweeper;return PT_SWEEPER ;}
"Defender" {holder.pt=PT_Defender;return PT_DEFENDER ;}
"Midfielder" {holder.pt=PT_Midfielder;return PT_MIDFIELDER ;}
"Forward"  {holder.pt=PT_Forward;return PT_FORWARD ;}
"All"      {holder.pt=PT_All;return ALL ;}

"Center"   {return PS_CENTER ;}
"WingWB"   {return PS_WINGWB ;}
"WingNB"   {return PS_WINGNB ;}
"Left"     {return PS_LEFT;}
"Right"    {return PS_RIGHT;} 
"rectangle" {return RECTANGLE ;}
"circle"    {return CIRCLE ;}
"reg"       {return REG ;}
"arc"       {return ARC;}
"Vector"    {return VECTOR ;}
"ball"      {return BALL ;}
"Scenario:" {return SCENARIO;}

"ball_x" {return BALLX;}
"ball_y" {return BALLY;}
"offside" {return OFFSIDE;}
"tm1_x" {return TM1_X;}
"tm2_x" {return TM2_X;} 
"tm3_x" {return TM3_X;} 
"tm4_x" {return TM4_X;} 
"tm5_x" {return TM5_X;} 
"tm6_x" {return TM6_X;} 
"tm7_x" {return TM7_X;} 
"tm8_x" {return TM8_X;} 
"tm1_y" {return TM1_Y;}
"tm2_y" {return TM2_Y;} 
"tm3_y" {return TM3_Y;} 
"tm4_y" {return TM4_Y;} 
"tm5_y" {return TM5_Y;} 
"tm6_y" {return TM6_Y;} 
"tm7_y" {return TM7_Y;} 
"tm8_y" {return TM8_Y;} 

{unum} 	{holder.val=atoi(yytext);return UNUM;}
{int}  	{holder.val=atoi(yytext);return INT;}
{float} {holder.fval=atof(yytext);return FLOAT;}
\"[0-9A-Za-z\(\)\.\+\-\*\/\?\<\>\_ ]+\" {holder.str=yytext; return STRING;}
[ \n\t]  ;

<COMMENT>"#" {}
<COMMENT>\n  {BEGIN INITIAL;}
<COMMENT>[^#\n] ;
"#"      {BEGIN COMMENT;}

<<EOF>>     {return END_NOW;}

.      {}
%%
