%{
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "ast.h"
#include "symbol_table.h"
#include "tube3.tab.hh"

std::string out_filename;
int line_count = 0;
%}

EOL           "\n"
print         "print"
type          "int"
random        "random"
id            [a-zA-Z_]+[a-zA-Z0-9_]*
int_lit       [0-9]+
assign_add   "+="
assign_sub    "-="
assign_mult   "*="
assign_div    "/="
assign_mod    "%="
compare_equ   "=="
compare_nequ  "!="
compare_lte   "<="
compare_less  "<"
compare_gte   ">="
compare_gtr   ">"
bool_and      "&&"
bool_or       "||"
sign          [-+]
ascii         [\+\-/=;\(\)%,{}[\]\*\.]
star          "*"
white         [ \t\n]+
comment       "#".*\n
%s            IN_COMMENT
unknown       .

%%
{EOL}           { line_count++; }
{print}         { return COMMAND_PRINT; }
{random}        { return COMMAND_RANDOM; }
{type}          { yylval.lexeme = strdup(yytext); return TYPE; }
{id}            { yylval.lexeme = strdup(yytext); return ID;}
{int_lit}       { yylval.lexeme = strdup(yytext); return INT_LITERAL; }
{assign_add}    { return ASSIGN_ADD; }
{assign_sub}    { return ASSIGN_SUB; }
{assign_mult}   { return ASSIGN_MULT; }
{assign_div}    { return ASSIGN_DIV; }
{assign_mod}    { return ASSIGN_MOD; }
{compare_nequ}  { return COMP_NEQU; }
{compare_equ}   { return COMP_EQU; }
{compare_lte}   { return COMP_LTE; }
{compare_less}  { return COMP_LESS; }
{compare_gte}   { return COMP_GTE; }
{compare_gtr}   { return COMP_GTR; }
{bool_and}   { return BOOLAND;  }
{bool_or}    { return BOOLOR; }
{ascii}     { return yytext[0];}
{white}     {
	for(int i=0; i < yyleng;i++){
	  if(yytext[i] == '\n') {
	    line_count++;
	  }
	}
}
{comment} { line_count++;}



<INITIAL>{
"/*"              BEGIN(IN_COMMENT);
}
<IN_COMMENT>{
"*/"      BEGIN(INITIAL);
[^*\n]+   // eat comment in chunks
{star}     { return yytext[0];}  // eat the lone star
}
{unknown} { line_count++; printf("Unknown token on line %i: %s\n", line_count, yytext);
  exit(3);
}
%%

void LexMain(int argc, char * argv[])
{
  int arg_id = 0;
  if (argc != 3) {
    std::cerr << "Format: " << argv[0] << " [input] [output]" << std::endl;
    exit(1);
  }

  // open input file
  FILE *file = fopen(argv[1], "r");
  if (!file) {
    std::cerr << "Error opening " << argv[1] << std::endl;
    exit(2);
  }

  yyin = file;

  out_filename = argv[2];
  //open output file
 // output_file.open(argv[2]);

/*  OLD CODE
#  //open file to write
#  FILE *output = fopen(argv[2], "w");
#  if (!output) {
#  std::cerr << "Error opening " << argv[2] << std::endl;
#  exit(3);
#  }
*/
  // TODO: write to file

  return;
}
