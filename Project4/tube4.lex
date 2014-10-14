%{
#include <iostream>

#include "ast.h"
#include "symbol_table.h"
#include "tube4.tab.hh"

int line_num = 1;
%}

id              [a-zA-Z_][a-zA-Z0-9_]*
ascii           [+\-*/;\(\)\{\}=,%?:\!]

%%

#.* ; // Comment, ignore remainder of line

"int"   { /* Types; right now, just "int" */
          yylval.lexeme = strdup(yytext);
	  return TYPE_INT;
        }

"char"  {
	  yylval.lexeme = strdup(yytext);
	  return TYPE_CHAR;
	}

"print" {
          yylval.lexeme = strdup(yytext);
	  return COMMAND_PRINT;
        }

random  {
 	  yylval.lexeme = strdup(yytext);
	  return COMMAND_RANDOM;
        }

"if"	{
	  yylval.lexeme = strdup(yytext);
	  return IF;
	}

{id}    { /* Identifier */
          yylval.lexeme = strdup(yytext);
	  return ID;
        }

[0-9]+  { /* Int Literal */
          yylval.lexeme = strdup(yytext);
          return INT_LITERAL;
        }

"==" { return COMP_EQU; }
"!=" { return COMP_NEQU; }
"<" { return COMP_LESS; }
"<=" { return COMP_LTE; }
">" { return COMP_GTR; }
">=" { return COMP_GTE; }
"&&" { return BOOL_AND; }
"||" { return BOOL_OR; }
"+=" { return ASSIGN_ADD; }
"-=" { return ASSIGN_SUB; }
"*=" { return ASSIGN_MULT; }
"/=" { return ASSIGN_DIV; }
"%=" { return ASSIGN_MOD; }
"{"  { return OPEN_BRACE; }
"}"  { return CLOSE_BRACE; }

{ascii} { /* Chars to return directly! */
          return yytext[0];
        }

[ \t\r] ; /* Ignore whitespace*/

[\n]    { /* Increment line number */
          line_num++;
        }

.       { /* Deal with unknown token! */
          std::cout << "ERROR(line " << line_num << "): Unknown Token '"
                    << yytext << "'." << std::endl;
          exit(1);
        }

%%

void LexMain(int argc, char * argv[])
{
  int arg_id = 0;

  if (argc != 3) {
    std::cerr << "Format: " << argv[0] << " [filename] [filename]" << std::endl;
    exit(1);
  }

  // The only thing left to do is assume the current argument is the filename.
  FILE *file = fopen(argv[1], "r");
  if (!file) {
    std::cerr << "Error opening " << argv[1] << std::endl;
    exit(2);
  }
  yyin = file;

  return;
}
