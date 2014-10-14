%{
#include <iostream>
#include <fstream>
#include <stdio.h>

#include <string>
#include <cstdlib>

#include "ast.h"

extern int line_num;
extern int yylex();
std::ofstream fs;

symbolTables symbol_tables;

//std::vector<symbolTable> discarded_tables;
//std::vector<symbolTable> symbol_tables;
//int scope = 0;

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
%type<ast_node> statement_list statement block declare declare_assign//
variable expression assignment operation compare literal negative//
command parameters if open close//


%right '=' ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD
%right '?' ':'
%left BOOL_OR
%left BOOL_AND
%nonassoc COMP_EQU COMP_NEQU COMP_LESS COMP_LTE COMP_GTR COMP_GTE
%left '+' '-'
%left '*' '/' '%'
%nonassoc UMINUS
%left OPEN_BRACE CLOSE_BRACE

%%

program:        statement_list {
                  // This is always the last rule to run so $$ is the full AST
                  $1->CompileTubeIC(symbol_tables, fs);
                  $1->DebugPrint();
                }

statement_list:  {
                  // Start the statement list by creating a block.
    //symbol_tables.AddTable();
                  $$ = new ASTNode_Block();
                }
  | statement_list statement ';' {
    $1->AddChild($2); // Add each statement to the block
    $$ = $1;          // Pass the block along
    }
  | statement_list block {
      $1->AddChild($2);
      $$ = $1;
    }



block: open close { $$ = $1; }

open:     OPEN_BRACE  statement_list {
     symbol_tables.AddTable();
     $$ = new ASTNode_Block ();
     }

close:    CLOSE_BRACE {
     symbol_tables.PopTable();
     };



statement:      declare        { $$ = $1; }
        |       declare_assign { $$ = $1; }
        |       expression         { $$ = $1; }
        |       command            { $$ = $1; }
        |       if                 { $$ = $1; }

if:   'if(' expression ')' {
  }

declare:  TYPE ID {

    if (symbol_tables.Lookup($2) != 0) {
      std::string err_string = "re-declaring variable '";
      err_string += $2;
      err_string += "'";
      yyerror(err_string);
      exit(1);
      }

      $$ = new ASTNode_Variable( symbol_tables.AddEntry($2) );

    }

declare_assign:  declare '=' expression {
                  $$ = new ASTNode_Assign($1, $3);
                }

expression:     literal { $$ = $1; }
          |     negative { $$ = $1; }
          |     variable { $$ = $1; }
          |     operation { $$ = $1; }
          |     compare { $$ = $1; }
          |     assignment { $$ = $1; }

literal:        INT_LITERAL {
                  $$ = new ASTNode_Literal($1);
                }

negative:       '-' expression %prec UMINUS {
                  $$ = new ASTNode_Negation($2);
                }
;

variable:      ID {
        tableEntry * entry = symbol_tables.Lookup($1);
        if (entry == 0) {
          std::string err_string = "unknown variable '";
          err_string += $1;
          err_string += "'";
          yyerror(err_string);
          exit(2);
        }
      }

operation:
         expression '+' expression {
                  $$ = new ASTNode_Math2($1, $3, '+');
         }
    |    expression '-' expression {
                  $$ = new ASTNode_Math2($1, $3, '-');
         }
    |    expression '*' expression {
                  $$ = new ASTNode_Math2($1, $3, '*');
         }
    |    expression '/' expression {
                  $$ = new ASTNode_Math2($1, $3, '/');
         }
    |    expression '%' expression {
                  $$ = new ASTNode_Math2($1, $3, '%');
         }
    |    '(' expression ')' {
                  $$ = $2;
         }
    |    COMMAND_RANDOM '(' expression ')' {
                  $$ = new ASTNode_Random($3);
                }
assignment:
          variable '=' expression {
            $$ = new ASTNode_Assign($1, $3);
          }
    |     variable ASSIGN_ADD expression {
            $$ = new ASTNode_MathAssign($1, $3, '+');
          }
    |     variable ASSIGN_SUB expression {
            $$ = new ASTNode_MathAssign($1, $3, '-');
          }
    |     variable ASSIGN_MULT expression {
            $$ = new ASTNode_MathAssign($1, $3, '*');
          }
    |     variable ASSIGN_DIV expression {
            $$ = new ASTNode_MathAssign($1, $3, '/');
          }
    |     variable ASSIGN_MOD expression {
            $$ = new ASTNode_MathAssign($1, $3, '%');
          }
compare:
       expression COMP_EQU expression {
                  $$ = new ASTNode_Comparison($1, $3, "==");
       }
    |  expression COMP_NEQU expression {
                  $$ = new ASTNode_Comparison($1, $3, "!=");
       }
    |  expression COMP_GTE expression {
                  $$ = new ASTNode_Comparison($1, $3, ">=");
       }
    |  expression COMP_LESS expression {
                  $$ = new ASTNode_Comparison($1, $3, "<");
       }
    |  expression COMP_LTE expression {
                  $$ = new ASTNode_Comparison($1, $3, "<=");
       }
    |  expression COMP_GTR expression {
                 $$ = new ASTNode_Comparison($1, $3, ">");
       }
    |  expression BOOL_AND expression {
                  $$ = new ASTNode_Logical($1, $3, "&&");
       }
    |  expression BOOL_OR expression {
                  $$ = new ASTNode_Logical($1, $3, "||");
       }
    |  expression '?' expression ':' expression {
                  $$ = new ASTNode_Conditional($1, $3, $5);
       }

command:    COMMAND_PRINT parameters {
              $$ = new ASTNode_Print();
              for (int i = 0; i < $2->GetNumChildren(); i++) {
                $$->AddChild($2->RemoveChild(i));
              }
              delete $2;
            }

parameters:  expression {
              $$ = new ASTNode_Temp();
              $$->AddChild($1);
            }
         |   parameters ',' expression {
              $$ = $1;
              $$->AddChild($3);
            }
%%

void LexMain(int argc, char * argv[]);

int main(int argc, char * argv[])
{
  LexMain(argc, argv);
  fs.open(argv[2]);
  if (!fs.is_open()) {
    std::cerr << "Error opening file " << argv[2] << std::endl;
    exit(2);
  }
  yyparse();
  std::cout << "Parse Successful!" << std::endl;
  return 0;
}
