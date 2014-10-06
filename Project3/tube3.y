%{
#include <iostream>
#include <string>
#include <cstdlib>

#include "symbol_table.h"
#include "ast.h"

extern int line_num;
extern int yylex();

symbolTable symbol_table;

void yyerror(std::string err_string) {
  std::cout << "ERROR(line " << line_num << "): " << err_string << std::endl;
  exit(1);
}
%}

%union {
  char * lexeme;
  ASTNode * ast_node;
}

%token<lexeme> ID INT_LITERAL TYPE
%token<lexeme> COMMAND_PRINT COMMAND_RANDOM
%token<lexeme> COMP_EQU COMP_NEQU COMP_LESS COMP_LTE COMP_GTR COMP_GTE
%token<lexeme> BOOL_AND BOOL_OR
%token<lexeme> ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD

%type<ast_node> statement_list statement var_declare var_declare_assign var_usage expression command param_list

%right '='
%right ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD
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

statement:      var_declare        { $$ = $1; }
        |       var_declare_assign { $$ = $1; }
        |       expression         { $$ = $1; }
        |       command { $$ = $1; }

var_declare:	TYPE ID {
                  if (symbol_table.Lookup($2) != 0) {
		    std::string err_string = "re-declaring variable '";
		    err_string += $2;
                    err_string += "'";
                    yyerror(err_string);
		    exit(1);
                  }

                  $$ = new ASTNode_Variable( symbol_table.AddEntry($2) );
                }

var_declare_assign:  var_declare '=' expression {
                  $$ = new ASTNode_Assign($1, $3);
                }

var_usage:      ID {
                  tableEntry * entry = symbol_table.Lookup($1);
                  if (entry == 0) {
                    std::string err_string = "unknown variable '";
		    err_string += $1;
                    err_string += "'";
		    yyerror(err_string);
                    exit(1);
                  }

                  $$ = new ASTNode_Variable( entry );
                }

expression:     INT_LITERAL {
                  $$ = new ASTNode_Literal($1);
                }
        |       expression '+' expression {
                  $$ = new ASTNode_Math2($1, $3, '+');
                }
        |       expression '-' expression {
                  $$ = new ASTNode_Math2($1, $3, '-');
                }
        |       expression '*' expression {
                  $$ = new ASTNode_Math2($1, $3, '*');
                }
        |       expression '/' expression {
                  $$ = new ASTNode_Math2($1, $3, '/');
                }
        |       expression '%' expression {
                  $$ = new ASTNode_Math2($1, $3, '/');
                }
        |       var_usage ASSIGN_ADD expression {
                  $$ = new ASTNode_MathAssign($1, $3, '+');
                }
        |       var_usage ASSIGN_SUB expression {
                  $$ = new ASTNode_MathAssign($1, $3, '-');
                }
        |       var_usage ASSIGN_MULT expression {
                  $$ = new ASTNode_MathAssign($1, $3, '*');
                }
        |       var_usage ASSIGN_DIV expression {
                  $$ = new ASTNode_MathAssign($1, $3, '/');
                }
        |       var_usage ASSIGN_MOD expression {
                  $$ = new ASTNode_MathAssign($1, $3, '%');
                }
        |       var_usage '=' expression {
                  $$ = new ASTNode_Assign($1, $3);
                }
        |       var_usage {
                  $$ = $1;
                }

command:    COMMAND_PRINT param_list {
              $$ = new ASTNode_Print();
              for (int i = 0; i < $2->GetNumChildren(); i++) {
                $$->AddChild($2->RemoveChild(i));
              }
              delete $2;
            }

param_list:  expression {
              $$ = new ASTNode_Temp();
              $$->AddChild($1);
            }
         |   param_list ',' expression {
              $$ = $1;
              $$->AddChild($3);
            }
%%

void LexMain(int argc, char * argv[]);

int main(int argc, char * argv[])
{
  LexMain(argc, argv);

  yyparse();
  std::cout << "#Parse Successful!" << std::endl;
  return 0;
}
