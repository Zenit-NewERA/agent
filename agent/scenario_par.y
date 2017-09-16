/* -*-bison-*- */
/*
 *Copyright:

    Copyright (C) 2002 Anton Ivanov (email: robocup@mail.ru)
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *EndCopyright:
 */
%{
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

%}
     
/* BISON DECLARATIONS */
%pure_parser

%token STRING ERROR
%token FLOAT UNUM INT
%token O_PAREN "("
%token C_PAREN ")"
%token COLON ":"
%token STEPS_DEFINITION "Steps:"
%token BALL_OWNER "Ball_owner:"
%token BTO "ballto"
%token PNH "passnahod"
%token O_BRACE "{"
%token C_BRACE "}"
%token NONE "none"
%token WBALL "No_ball:"
%token POS "pos"
%token MARK "mark"
%token LEAVE_COND "Leave_conditions:"
%token IF "if"
%token GOTO "goto"
%token END "end"
%token DRIBBLE "dribble"
%token CLEAR "clear"
%token HOLD "hold"
%token SCORE "score"
%token INITS "Init_scenario:"
%token INITTM "Init_teammates:"
%token EQUEL "="
%token BPOS "bpos"
%token PPOS "ppos"
%token BOWNER "bowner"
%token AND "and"
%token OR "or"
%token NOT "not"
%token OUR "our"
%token OPP "opp"
%token COMMA ","
%token PALKA "|"
%token FASTEST_TM "FastestTm"
%token PT_SWEEPER "Sweeper"
%token PT_DEFENDER "Defender"
%token PT_MIDFIELDER "Midfielder"
%token PT_FORWARD "Forward"
%token ALL "All"
%token PS_CENTER "Center"
%token PS_WINGWB "WingWB"
%token PS_WINGNB "WingNB"
%token PS_LEFT   "Left"
%token PS_RIGHT  "Right"
%token RECTANGLE "rectangle"
%token CIRCLE "circle"
%token ARC "arc"
%token REG "reg"
%token VECTOR "Vector"
%token BALL "ball"
%token SCENARIO "Scenario:"
%token BALLX "ball_x"
%token BALLY "ball_y"
%token OFFSIDE "offside"
%token TM1_X TM2_X TM3_X TM4_X TM5_X TM6_X TM7_X TM8_X
%token TM1_Y TM2_Y TM3_Y TM4_Y TM5_Y TM6_Y TM7_Y TM8_Y

%token END_NOW

%start document
%%

/* Grammar RULES */
document:	list_of_scenario END_NOW
					{B.BuildScenarioSet();YYACCEPT;}
;
list_of_scenario: list_of_scenario scenario
								| scenario
;
scenario: SCENARIO STRING FLOAT scenario_list
						{B.BuildScenario($2.str,$3.fval);}
;
scenario_list: scenario_list scenario_tok
						 | scenario_tok
;
scenario_tok: init_check
						| steps
						| init_tm
;
init_check: INITS condition
						 {B.BuildInitCond();}
;
init_tm: INITTM init_tm_list
;
init_tm_list: init_tm_list init_tm_token
						| init_tm_token
;
init_tm_token: init_tm_token_list EQUEL UNUM
								{B.BuildInitTmFromList($3.val);}
;
init_tm_token_list: init_tm_token_list init_tm_cond
							| init_tm_cond
;
init_tm_cond: player_type
								{B.BuildInitTmFromPt();}
						| region
								{B.BuildInitTmFromReg();}
						| FASTEST_TM
								{B.BuildInitTmFromFastest();}
;
steps: STEPS_DEFINITION steps_list
;
steps_list: steps_list step
					| step
;
step: STRING step_list
				{B.BuildStep($1.str);}
;
step_list: step_list step_token
				 | step_token
;
step_token: leave_cond
					| ballo
					| wball
;
ballo: BALL_OWNER actionbo_list
				{B.BuildWithBallActions();}
;
actionbo_list: actionbo_list actionbo
						 | actionbo
;
actionbo: O_PAREN BTO pl_set C_PAREN
						{B.BuildPTPAction();}
				| O_PAREN BTO region bmove_set C_PAREN
						{B.BuildMBAction();}
				| O_PAREN PNH pl_set C_PAREN
						{B.BuildPNHAction();}
				| NONE
						{B.BuildNoneWBAction();}
;
wball: WBALL wball_list
;
wball_list: wball_list wball_token
					| wball_token
;
wball_token: UNUM COLON action
				{B.BuildNoBallActions($1.val);}
;
action:  O_PAREN POS region C_PAREN
					{B.BuildMTAction();}
			 | O_PAREN MARK pl_set C_PAREN
			 		{B.BuildMarkAction();}
			 | NONE
			 		{B.BuildNoneNBAction();}
;

leave_cond: LEAVE_COND leave_cond_list
;
leave_cond_list: leave_cond_list leave_cond_token
							 | leave_cond_token
;
leave_cond_token: IF condition GOTO STRING
										{B.BuildLeaveCondition($4.str);}
								| IF condition GOTO END
										{B.BuildLeaveCondition("end");}
;

bmove_set: O_BRACE bmove_list C_BRACE
;
bmove_list: bmove_list bmove_token
						{B.AddBallMove(Action::BallMove($2.val));}
					| bmove_token
						{B.AddBallMove(Action::BallMove($1.val));}
;
bmove_token: DRIBBLE
					 | CLEAR
					 | HOLD
					 | SCORE
;					 
condition: O_PAREN BPOS region C_PAREN
						{B.BuildBposCondition();}
				 | O_PAREN PPOS player_type UNUM UNUM region C_PAREN
				 		{B.BuildPposCondition($4.val,$5.val);}
				 | O_PAREN BOWNER player_type C_PAREN
				 		{B.BuildBownerCondition();}
				 | O_PAREN AND condition_list C_PAREN
				 		{B.BuildAndCondition();}
				 | O_PAREN OR condition_list C_PAREN
				 		{B.BuildOrCondition();}
				 | O_PAREN NOT condition C_PAREN
				 		{B.BuildNotCondition();}
				 | O_PAREN team pl_set UNUM UNUM region C_PAREN
				 		{B.BuildPposExtCondition($4.val,$5.val);}
				 | O_PAREN BOWNER UNUM C_PAREN
				 		{B.BuildBownerCondition($3.val);}
;	
condition_list: condition_list condition
							| condition
;
pl_set: O_BRACE num_list C_BRACE
;
num_list: num_list num_token
				| num_token
;
num_token: UNUM
						{B.AddToPlayerSet($1.val);}
					| ALL
						{B.AddAllToPlayerSet();}
;	
player_type: O_PAREN pt COMMA ps C_PAREN
;
pt:		pt PALKA pt_token
	|   pt_token
;
ps:   ps PALKA ps_token
	|   ps_token
;
pt_token: PT_SWEEPER
					{B.AddPlayerType($1.pt);}
				| PT_DEFENDER
					{B.AddPlayerType($1.pt);}
				| PT_MIDFIELDER
					{B.AddPlayerType($1.pt);}
				| PT_FORWARD
					{B.AddPlayerType($1.pt);}
				| ALL
					{B.AddPlayerType($1.pt);}
;
ps_token: PS_CENTER
					{B.AddPlayerSide(PTSType::center);}
				| PS_WINGWB
					{B.AddPlayerSide(PTSType::wingwb);}
				| PS_WINGNB
					{B.AddPlayerSide(PTSType::wingnb);}
				| PS_RIGHT
					{B.AddPlayerSide(PTSType::right);}
				| PS_LEFT
					{B.AddPlayerSide(PTSType::left);}
				| ALL
					{B.AddPlayerSide(PTSType::all);}
;
region:  point 
					{B.BuildPointRegion();}
		 |  O_PAREN RECTANGLE point point C_PAREN
		 			{B.BuildRectangleRegion();}
		 |  O_PAREN CIRCLE point FLOAT C_PAREN
		 			{B.BuildCircleRegion($4.fval);}
		 |  O_PAREN ARC point FLOAT FLOAT FLOAT FLOAT C_PAREN
		 		  {B.BuildArcRegion($4.fval,$5.fval,$6.fval,$7.fval);}
		 |  O_PAREN REG region_list C_PAREN	
		 			{B.BuildRegRegion();}
;
region_list: region_list region
						| region
;
point: VECTOR O_PAREN value COMMA value C_PAREN
				{B.BuildPoint();}
		 | VECTOR O_PAREN BALL C_PAREN
		 		{B.BuildPointFromBall();}
		 |VECTOR O_PAREN team COMMA UNUM C_PAREN
		 		{B.BuildPointFromPlayer($5.val);}
		 | VECTOR O_PAREN value COMMA value point C_PAREN
		 		{B.BuildAddPoint();}
;
value: FLOAT
				{B.BuildValue($1.fval);}
			| BALLX
				{B.BuildValueBx();}
			| BALLY
				{B.BuildValueBy();}
			| OFFSIDE
				{B.BuildValueOffside();}
			| TM1_X
				{B.BuildValueTmX(1);}
			| TM2_X
				{B.BuildValueTmX(2);}
			| TM3_X
				{B.BuildValueTmX(3);}
			| TM4_X
				{B.BuildValueTmX(4);}
			| TM5_X
				{B.BuildValueTmX(5);}
			| TM6_X
				{B.BuildValueTmX(6);}
			| TM7_X
				{B.BuildValueTmX(7);}
			| TM8_X
				{B.BuildValueTmX(8);}
			| TM1_Y
				{B.BuildValueTmY(1);}
			| TM2_Y
				{B.BuildValueTmY(2);}
			| TM3_Y
				{B.BuildValueTmY(3);}
			| TM4_Y
				{B.BuildValueTmY(4);}
			| TM5_Y
				{B.BuildValueTmY(5);}
			| TM6_Y
				{B.BuildValueTmY(6);}
			| TM7_Y
				{B.BuildValueTmY(7);}
			| TM8_Y
				{B.BuildValueTmY(8);}

;
team:  OUR
			{B.SetTeam($1.bval);}
		|  OPP
			{B.SetTeam($1.bval);}
;

%%


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
