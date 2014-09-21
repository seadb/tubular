%{
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <map>

extern int line_count;
extern int yylex();

void yyerror(std::string err_string) {
  std::cout << "ERROR(line " << line_count << "): "
       << err_string << std::endl;
  exit(1);
}

struct Var {
    std::string name;
    std::string type;
    int line_count;
};

std::map <std::string, Var *> symbol_table; //symbol table

%}

%union {
  char * lexeme;
}

%token<lexeme> ID INT_LITERAL TYPE 

%token COMMAND_PRINT

%left COMMAND_RANDOM 
%left '<' '>' COMP_NEQU COMP_EQU COMP_LTE COMP_GTE BOOLAND BOOLOR
%left '*' '/' '%'
%left '+' '-'
%right ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD '='

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

statement:  assignment   {	/* Determine if we have an assignment */ }
	     |  expression   {  /* Determine if we have a math expression (the right of =) */  }
        |	var_any      {  /* Determine if we have a variable declaration or usage */  }
	     |	printlist    {  /* Determine if we have a print statement */  }
	    ;

// These statements are causing the shift/reduce warnings, they are harmless
assignment:	var_any '=' expression { // ensures '=' starts with int Var or Var

		}
	|	var_usage ASSIGN_ADD expression { // ensure '+=' starts correctly
        //this allows var declare += expression
		}
	|	var_usage ASSIGN_SUB expression { // ensure '-=' starts corrently

		}
	|	var_usage ASSIGN_MULT expression { // ensure '*=' starts correctly

		}
	|	var_usage ASSIGN_DIV expression { // ensure '/=' starts correctly

		}
	|	var_usage ASSIGN_MOD expression { // ensure '%=' starts correctly

		}
	;

var_any:	var_declare{
                  //std::cout << "Declaration left of all " << std::endl;                  
		}
	|	var_usage {
		  //std::cout << "Just the ID left of all " << std::endl;
		}
	;
var_usage:	ID { // Identifier
	 	  //std::cout << "Just the ID " << $1 << std::endl; 
		  //std::cout << "No fear!" << std::endl;

            if(symbol_table.find($1) == symbol_table.end()) //ID not found in symbol table
            {     
                char buff[100];      
                sprintf(buff,"unknown variable '%s'", $1);
                std::string err = buff;
                yyerror(err);
            }

}
	;
var_declare:	TYPE ID { // int Identifier
	          //std::cout << "Do something other than printing var info here!" << "Type=" << $1 << " name=" << $2 << std::endl; 
		      //std::cout << "Var declare" << "Type=" << $1 << " name=" << $2 << std::endl; 

            std::map<std::string, Var *>::iterator match;
            match = symbol_table.find($2);
            if(match == symbol_table.end())     //ID not found in symbol table
            {                                   // New Variable declared
                struct Var temp; 
                temp.type = $1;
                temp.name = $2;
                temp.line_count = line_count;
                symbol_table[$2]= &temp; 
            }
            else
            {
                char buff[100];
                sprintf(buff, "redeclaration of variable '%s'", $2);
                std::string err = buff; 
                yyerror(err);
            } 
        }

	;
expression:     INT_LITERAL { // Integer
                  //std::cout << "Found int: " << $1
                    //        << " (but you shouldn't print it!)" << std::endl;
                }
        |       expression '+' expression { // Addition
                  //std::cout << "Doing addition! (but you shouldn't print it!)" << std::endl;
                }
        |       expression '-' expression { // Subtraction
                  //std::cout << "Doing subtraction! (but you shouldn't print it!)" << std::endl;
                }
	|	expression '*' expression { // Multiplication 
		  //std::cout << "Doing multiplication!" << std::endl; 
		}
	|	expression '/' expression { // Division

		}
	|	expression '%' expression { // Mod

		}
	|	var_usage '=' expression { // equals

		}
	|	var_usage ASSIGN_ADD expression { // +=

		}
	|	var_usage ASSIGN_SUB expression { // -=

		}
	|	var_usage ASSIGN_MULT expression { // *=

		}
	|	var_usage ASSIGN_DIV expression { // /=

		}
	|	var_usage ASSIGN_MOD expression { // %=

		}
	|	expression COMP_EQU expression { // ==

		}
	|	expression COMP_NEQU expression { // !=

		}
	|	expression '<' expression { // less than

		}
	|	expression COMP_LTE expression { // <=

		}
	|	expression '>' expression { // greater than

		}
	|	expression COMP_GTE expression { // >=

		}
	|	expression BOOLAND expression { // &&

		}
	|	expression BOOLOR expression { // ||

		}
	|	'(' expression ')' { // parentheses

		}
	|	COMMAND_RANDOM '(' expression ')' { // random(x)

		}
	// this is causing the reduce/reduce warning; aka when this is called,
	// both this and the code within var_usage will execute; for our program
	// I don't think this will cause an issue (for now, at least)
	|	var_usage { // variable
		  //std::cout << "Instead of printing, check if '" << $1
		  //          << "' actually exists!" << std::endl;
		  //std::cout << "Why not both? " << std::endl;
		}
	;

printlist:	/*empty*/
	|	printlist COMMAND_PRINT printing {}
        ;
printing:	expression
	|	printing ',' expression
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
