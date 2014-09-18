%{
#include <iostream>
#include <string>
#include <cstdlib>

extern int line_count;
extern int yylex();

void yyerror(std::string err_string) {
  std::cout << "ERROR(line " << line_count << "): "
       << err_string << std::endl;
  exit(1);
}
%}

%union {
  char * lexeme;
}

%token<lexeme> ID INT_LITERAL TYPE COMMAND_PRINT COMMAND_RANDOM ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD COMP_NEQU COMP_EQU COMP_LTE COMP_GTE BOOLAND BOOLOR


%left '+' '-' '*' '/' '%' '<' '>' '(' ')'
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
	|	printline    {  /* Determine if we have a print statement */  }
	;

var_declare:	TYPE ID {
                  std::cout << "Do something other than printing var info here!"
                            << "Type=" << $1 << " name=" << $2 << std::endl;
                }
	;
expression:     INT_LITERAL {
                  std::cout << "Found int: " << $1
                            << " (but you shouldn't print it!)" << std::endl;
                }
        |       expression '+' expression {
                  std::cout << "Doing addition! (but you shouldn't print it!)" << std::endl;
                }
        |       expression '-' expression {
                  std::cout << "Doing subtraction! (but you shouldn't print it!)" << std::endl;
                }
	|	expression '*' expression { 
		  std::cout << "NICE" << std::endl; 
		}
	|	expression '/' expression {}
	|	expression '%' expression {}
	|	ID ASSIGN_ADD expression {}
	|	ID ASSIGN_SUB expression {}
	|	ID ASSIGN_MULT expression {}
	|	expression ASSIGN_DIV expression {}
	|	expression ASSIGN_MOD expression {}
	|	expression COMP_EQU expression {

		}
	|	expression COMP_NEQU expression {

		}
	|	expression '<' expression {

		}
	|	expression COMP_LTE expression {

		}
	|	expression '>' expression {

		}
	|	expression COMP_GTE expression {

		}
	|	expression BOOLAND expression {

		}
	|	expression BOOLOR expression {

		}
	|	'(' expression ')' {

		}
        |       ID {
                  std::cout << "Instead of printing, check if '" << $1
                            << "' actually exists!" << std::endl;
                }
printline:	COMMAND_PRINT {

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
