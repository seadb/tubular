%{
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>

extern int line_num;
extern int yylex();
std::vector< std::string> names ;


void yyerror(std::string err_string) {
  std::cout << "ERROR(line " << line_num << "): "
       << err_string << std::endl;
  exit(1);
}
%}

%union {
  char * lexeme;
}

%token<lexeme> ID INT_LITERAL TYPE COMMAND_PRINT COMMAND_RANDOM

%left '*' '/'
%left '+' '-'

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

statement: assignment { } // has an =
         | expression { } // just the right of =
         | var_declare {  /* Determine if we have a variable declaration */  }
	     | command { } // no expression 
         ;

assignment: var_any '=' expression { } 
         ;

var_any: var_usage { }
       | var_declare { }
       ;

var_declare:	TYPE ID {
                  std::cout << "Do something other than printing var info here!"
                            << "Type=" << $1 << " name=" << $2 << std::endl;
                
        for(std::vector<std::string>::iterator it = names.begin() ; it != names.end(); ++it)
                {
                    if(*it == $2)
                       std::cout<< "variable declared twice" << std::endl;
                        exit(1); 
                }

                names.push_back($2); 
}
                ;

var_usage:      ID '=' expression{
                std::cout<< "Found var_usage" << std::endl;}
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
        |       ID {
                  std::cout << "Instead of printing, check if '" << $1
                            << "' actually exists!" << std::endl;
                }
        ;

command:        COMMAND_PRINT {
                  std::cout << " command print" << std::endl;
                }
        |       COMMAND_RANDOM {
                   std::cout << "command random" << std::endl;
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
