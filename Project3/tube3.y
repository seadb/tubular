/*
 * Authors: Chelsea Bridson
 */
%{
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
//#include <map>

#include "symbol_table.h"
#include "ast.h"

extern int line_count;
extern int yylex();

//std::map <std::string, Var *> symbol_table; //symbol table
symbolTable symbol_table;

void yyerror(std::string err_string) {
  std::cout << "ERROR(line " << line_count << "): " << err_string << std::endl;
  exit(1);
}
%}

%union {
  char * lexeme;
  ASTNode * ast_node;
}


%token<lexeme> ID INT_LITERAL TYPE COMMAND_PRINT COMMAND_RANDOM
%type<ast_node> statement_list statement var_declare var_any assignment var_usage expression command parameter_list command_print command_random

%right ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD '='
%right '?' ':'
%left BOOLOR
%left BOOLAND
%left '<' '>' COMP_NEQU COMP_EQU COMP_LTE COMP_GTE
%left '+' '-'
%left '*' '/' '%'

%%

program:        statement_list {
                // This is always the last rule to run so $$ is the full AST
                $1->CompileTubeIC(symbol_table, std::cout);
                //$1->DebugPrint();
                }

statement_list:	{
                  // Start the statement list by creating a block.
                  $$ = new ASTNode_Block();
                }
	|	statement_list statement ';' {
                  $1->AddChild($2); // Add each statement to the block
                  $$ = $1;          // Pass the block along
		}


statement:      var_any            { $$ = $1; }
        |       assignment         { $$ = $1; }
	|       expression         { $$ = $1; }
        |       command            { $$ = $1; }

command:        command_print {}
                command_random{}

assignment:	var_declare '=' expression {
                  $$ = new ASTNode_Assign($1, $3);
		}
	|	var_usage ASSIGN_ADD expression {
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
		};

var_usage:	ID { // Identifier
                  tableEntry * entry = symbol_table.Lookup($1);
                  if (entry == 0) {
                    char buff[100];
                    sprintf(buff,"unknown variable '%s'", $1);
                    std::string err = buff;
                    yyerror(err);
                    exit(1);
                  }

                  $$ = new ASTNode_Variable( entry );
                }


var_declare:	TYPE ID { // a new variable
            tableEntry * entry = symbol_table.Lookup($2);
            if(entry == 0){               //ID not found in symbol table
                // New Variable declared
                $$ = new ASTNode_Variable( entry );
                symbol_table.AddEntry($2);
            }
            else
            {
                char buff[100];
                sprintf(buff, "redeclaration of variable '%s'", $2);
                std::string err = buff;
                yyerror(err);
                exit(2);
            }
        }
expression:     INT_LITERAL { // Integer
                  $$ = new ASTNode_Literal($1);
                }
        |       expression '+' expression { // Addition
                  $$ = new ASTNode_Math2($1, $3, '+');
                }
        |       expression '-' expression { // Subtraction
                  $$ = new ASTNode_Math2($1, $3, '-');
                }
	|	expression '*' expression { // Multiplication
		  $$ = new ASTNode_Math2($1, $3, '*');
                }
	|	expression '/' expression { // Division
                  $$ = new ASTNode_Math2($1, $3, '/');
		}
	|	expression '%' expression { // Mod
                  $$ = new ASTNode_Math2($1, $3, '%');
		}
	|	var_usage '=' expression { // equals
                  $$ = new ASTNode_Assign($1, $3);
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
/*	|	COMMAND_RANDOM '(' expression ')' { // random(x)

		}*/
	|	var_usage { // variable
		  $$ = $1;
                }

command_print:    COMMAND_PRINT parameter_list {
              $$ = new ASTNode_Print();
              for (int i = 0; i < $2->GetNumChildren(); i++) {
                $$->AddChild($2->RemoveChild(i));
              }
              delete $2;
            }

command_random:     COMMAND_RANDOM  { }

parameter_list:  expression {
              $$ = new ASTNode_Temp();
              $$->AddChild($1);
            }
         |   parameter_list ',' expression {
              $$ = $1;
              $$->AddChild($3);
            }

%%

void LexMain(int argc, char * argv[]);

int main(int argc, char * argv[])
{
  LexMain(argc, argv);

  yyparse();
  std::cout << "Parse Successful!" << std::endl;
  return 0;
}
