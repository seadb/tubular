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
%token<lexeme> BOOL_AND BOOL_OR
%token<lexeme> ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD

%right '=' ASSIGN_ADD ASSIGN_SUB
%right ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD
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
    |       assign       {                                                }
	|	print {								}
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
    |       TYPE ID '=' expression {
                if(symbol_table.find($2) != symbol_table.end()) {
                    string error = "redeclaration of variable '";
                    error += $2;
                    error += "'";
                    yyerror(error);
                }
                symbol sym = { symbol_num++, $1 };
                symbol_table[$2] = sym;
            }
    ;

assign:         ID '=' expression {
                    if(symbol_table.find($1) == symbol_table.end()) {
                        string error = "unknown variable '";
                        error += $1;
                        error += "'";
                        yyerror(error);
                    }

                }
print:		COMMAND_PRINT RvalCommaListLoop 

RvalCommaListLoop:	expression ',' RvalCommaListLoop {
		}
	| 	expression {
		}
	;

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
	|	expression ASSIGN_ADD expression {
		}
	|	expression ASSIGN_SUB expression {
		}
	|	expression ASSIGN_MOD expression {
		}
	|	expression ASSIGN_MULT expression {
		}
	|	expression ASSIGN_DIV expression {
		}
        |       expression COMP_EQU expression {
                }
        |       expression COMP_NEQU expression {
                }
        |       expression COMP_GTE expression {
                }
        |       expression COMP_LESS expression {
                }
        |       expression COMP_LTE expression {
                }
        |       expression COMP_GTR expression {
                }
        |       expression BOOL_AND expression {
                }
        |       expression BOOL_OR expression {
                }
	|	'(' expression ')' {
		}
	|	COMMAND_RANDOM '(' expression ')' {
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
