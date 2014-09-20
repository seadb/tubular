%{
#include "tube2.tab.hh"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

int line_count = 1;
%}

eol "\n"
print "print"
type "int"
random "random"
id [a-zA-Z_]+[a-zA-Z0-9_]*
int_lit [0-9]+
assignadd "+="
assignsub "-="
assignmult "*="
assigndiv "/="
assignmod "%="
compequ "=="
compnequ "!="
complte "<="
compless "<"
compgte ">="
compgtr ">"
booland "&&"
boolor "||"
ascii [\+\-\*/=;\(\)%,{}[\]\.]
white [ \t\n]+
comment1 "#".*\n
%s IN_COMMENT
unknown .

%%
{eol} { line_count++; }
{print} { return COMMAND_PRINT; }
{random} { return COMMAND_RANDOM; }
{type} { yylval.lexeme = strdup(yytext); return TYPE; }
{id} { yylval.lexeme = strdup(yytext); return ID;}
{int_lit} { yylval.lexeme = strdup(yytext); return INT_LITERAL; }
{assignadd} { return ASSIGN_ADD;  } 
{assignsub} { return ASSIGN_SUB;  }
{assignmult} { return ASSIGN_MULT;  }
{assigndiv} { return ASSIGN_DIV;  }
{assignmod} { return ASSIGN_MOD;  }
{compnequ} { return COMP_NEQU;  }
{compequ} { return COMP_EQU;  }
{complte} { return COMP_LTE;  }
{compless} { return yytext[0]; }
{compgte} { return COMP_GTE;  }
{compgtr} { return yytext[0];}
{booland} { return BOOLAND;  }
{boolor} { return BOOLOR; }
{ascii} { return yytext[0];}
{white} { 
	for(int i=0; i < yyleng;i++){
	  if(yytext[i] == '\n') { 
	    line_count++;
	  }
	}
}
{comment1} { line_count++;}
<INITIAL>{
"/*" BEGIN(IN_COMMENT);
}
<IN_COMMENT>{
 "*/" BEGIN(INITIAL);
  [^*\n]+ ; // eat comment in chunks
  "*" ;// eat the lone star
  \n line_count++;
}
{unknown} { line_count++; printf("Unknown token on line %i: %s\n", line_count, yytext);
  exit(3);
}
%%

void LexMain(int argc, char * argv[])
{
  int arg_id = 0;
  if (argc != 2) {
    std::cerr << "Format: " << argv[0] << " [filename]" << std::endl;
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
