%{
#include <iostream>
#include <map>
#include <string>
#include <cstdlib>
using namespace std;

extern int line_num;
extern int yylex();

void yyerror(std::string err_string) {
  std::cout << "ERROR(line " << line_num << "): "
       << err_string << std::endl;
  exit(1);
}

struct symbol {
    int symbol_num;
    string type;
};

map<string, symbol> symbol_table;
int symbol_num = 0;

%}

%union {
  char * lexeme;
}

%token<lexeme> ID INT_LITERAL TYPE
%token<lexeme> COMMAND_PRINT COMMAND_RANDOM
%token<lexeme> COMP_EQU COMP_NEQU COMP_LESS COMP_LTE COMP_GTR COMP_GTE
%token<lexeme> ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD


%left '+' '-'
%left '*' '/' '%'

%%

program:        statement_list {
                  /* This is always the last rule to run! */
                }
        ;

statement_list:	{
                  /* This is always the first rule to run! */
                }
	|	statement_list statement ';' {
                  /* This rule will run after each statement is created */
		}
	;

statement:      var_declare  {  /* Determine if we have a variable declaration */  }
	|       expression   {  /* Determine if we have a math expression */  }
	;

var_declare:	TYPE ID {
                    if(symbol_table.find($2) != symbol_table.end()) {
                        string error = "redeclaration of variable '";
                        error += $2;
                        error += "'";
                        yyerror(error);
                    }
                    symbol sym = { symbol_num++, $1 };
                    symbol_table[$2] = sym;
                }

expression:     INT_LITERAL {
                }
        |       expression '+' expression {
                }
        |       expression '-' expression {
                }
        |       expression '*' expression {
                }
        |       expression '/' expression {
                }
        |       expression '%' expression {
                }
        |       ID {
                    if(symbol_table.find($1) == symbol_table.end()) {
                        string error = "unknown variable '";
                        error += $1;
                        error += "'";
                        yyerror(error);
                    }
                }
        ;
%%

void LexMain(int argc, char * argv[]);

int main(int argc, char * argv[])
{
  LexMain(argc, argv);

  yyparse();
  std::cout << "Parse Successful!" << std::endl;
  return 0;
}
