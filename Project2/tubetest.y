%{
#include <iostream>
#include <string>
#include <cstdlib>

extern int line_num;
extern int yylex();

void yyerror(std::string err_string) {
  std::cout << "ERROR(line " << line_num << "): "
       << err_string << std::endl;
  exit(1);
}
%}

%union {
  char * lexeme;
}

%token<lexeme> ID INT_LITERAL TYPE

%left '+' '-' '*' '/' '%' '+=' '-=' '*=' '/=' '%=' '==' '!=' '<' '<=' '>' '>=' '&&' '||' '(' ')'

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
	|	expression {

		}

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

		}
	|	expression '/' expression {

		}
	|	expression '%' expression {

		}
	|	expression '+=' expression {

		}
	|	expression '-=' expression {

		}
	|	expression '*=' expression {

		}
	|	expression '/=' expression {

		}
	|	expression '%=' expression {

		}
	|	expression '==' expression {

		}
	|	expression '!=' expression {

		}
	|	expression '<' expression {

		}
	|	expression '<=' expression {

		}
	|	expression '>' expression {

		}
	|	expression '>=' expression {

		}
	|	expression '&&' expression {

		}
	|	expression '||' expression {

		}
	|	'(' expression ')' {

		}
        |       ID {
                  std::cout << "Instead of printing, check if '" << $1
                            << "' actually exists!" << std::endl;
                }
printline:	expression {

		}
	|	
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
