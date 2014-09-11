%{
#include <iostream>
#include <string>

extern int line_num;
extern int yylex();

void yyerror(std::string err_string) {
  std::cout << "ERROR(line " << line_num << "): "
       << err_string << std::endl;
}
%}

%union {
  char * lexeme;
}

%token<lexeme> ID INT_LITERAL TYPE

%left '+' '-'

%%

program:        statement_list {
                  /* This is always the last rule to run! */
                  std::cout << "Parse Successful!" << std::endl;
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
                  std::cout << "Do something other than printing var info here!"
                            << "Type=" << $1 << " name=" << $2 << std::endl;
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
        |       ID {
                  std::cout << "Instead of printing, check if '" << $1
                            << "' actually exists!" << std::endl;
                }
        ;
%%

void LexMain(int argc, char * argv[]);

int main(int argc, char * argv[])
{
  LexMain(argc, argv);

  yyparse();
  return 0;
}
