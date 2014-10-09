/*
 * Authors: Chelsea Bridson
 */
%{
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <fstream>
//#include <map>
#include "symbol_table.h"
#include "ast.h"

extern int line_count;
extern int yylex();
extern std::string out_filename;

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
%type<ast_node> statement_list statement declare variable expression command declaration declare_assign literal operation compare assignment parameter_list command_print command_random

%right ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD '='
%right '?' ':'
%left BOOLOR
%left BOOLAND
%left COMP_NEQU COMP_EQU COMP_LTE COMP_GTE COMP_GTR COMP_LESS
%left '+' '-'
%left '*' '/' '%'
%nonassoc UMINUS

%%

program:        statement_list {
                // This is always the last rule to run so $$ is the full AST
                $1->DebugPrint();
                std::ofstream out_file;
                out_file.open(out_filename.c_str());
                std::ofstream & out = out_file;
                out_file << $1->CompileTubeIC(symbol_table, out_file);
                out_file.close();
                }


statement_list:  {
                  // Start the statement list by creating a block.
                  $$ = new ASTNode_Block();
                 }
  |  statement_list statement ';' {
                  $1->AddChild($2); // Add each statement to the block
                  $$ = $1;          // Pass the block along
   }


statement:
  | declare     { $$ = $1; }
  | variable    { $$ = $1; }
 /* | declare_assign { $$ = $1; }*/
  | expression  { $$ = $1; }
  | command     { $$ = $1; }

declare: declaration { $$ = $1; }
  |      declare_assign { $$ = $1; }

  declaration:  TYPE ID {
      if (symbol_table.Lookup($2) != 0) {
        std::string err_string = "re-declaring variable '";
        err_string += $2;
        err_string += "'";
        yyerror(err_string);
        exit(1);
      }
      $$ = new ASTNode_Variable( symbol_table.AddEntry($2) );
     }

  declare_assign:    declaration '=' expression {        // newly declared variable
                          $$ = new ASTNode_Assign($1, $3);
      }

variable:  ID { // Identifier
                tableEntry * entry = symbol_table.Lookup($1);
                if (entry == 0) {
                  std::string err_string = "unknown variable '";
                  err_string += $1;
                  err_string += "'";
                  yyerror(err_string);
                  exit(2);
                }

                $$ = new ASTNode_Variable( entry );
         }

 /*----------EXPRESSION---------------------------------------------*/
expression:   literal     { $$ = $1; }
          |   operation   { $$ = $1; }
          |   compare     { $$ = $1; }
          |   assignment  { $$ = $1; }

  literal:  INT_LITERAL { // Integer
              $$ = new ASTNode_Literal($1);
    }

  operation:
         expression '+' expression { // Addition
                $$ = new ASTNode_Math2($1, $3, '+');
     }
  |     expression '-' expression { // Subtraction
                $$ = new ASTNode_Math2($1, $3, '-');
    }
  |     expression '*' expression { // Multiplication
          $$ = new ASTNode_Math2($1, $3, '*');
    }
  |     expression '/' expression { // Division
          $$ = new ASTNode_Math2($1, $3, '/');
    }
  |     expression '%' expression {//$1 - ($1/expression)*expression
          $$ = new ASTNode_Math2($1, $3, '%');
    }
  |     '(' expression ')' { // parentheses
          $$ = $2; // '(' is $1
    }
  |     variable {
          $$ = $1;
    }
  ;

 compare:
     expression COMP_EQU expression { // ==
      $$ = new ASTNode_Compare($1, $3, '==');
    }
  |  expression COMP_NEQU expression { // !=
      $$ = new ASTNode_Compare($1, $3, '!=');
    }
  |  expression COMP_LESS expression { // less than
      $$ = new ASTNode_Compare($1, $3, '<');
    }
  |  expression COMP_LTE expression { // <=
      $$ = new ASTNode_Compare($1, $3, '<');
    }
  |  expression COMP_GTR expression { // greater than
      $$ = new ASTNode_Compare($1, $3, '<');
    }
  |  expression COMP_GTE expression { // >=
      $$ = new ASTNode_Compare($1, $3, '<');
    }
  |  expression BOOLAND expression { // &&
      $$ = new ASTNode_Compare($1, $3, '<');
    }
  |  expression BOOLOR expression { // ||
      $$ = new ASTNode_Compare($1, $3, '<');
    }
  ;
/*-----------ASSIGNMENT----------------------------------------------*/

assignment:

     variable '=' expression {           //previously declared variable
                    $$ = new ASTNode_Assign($1, $3);
    }
  |
     variable ASSIGN_ADD expression { // $1 '+=' expression
                  ASTNode * add = new ASTNode_Math2($1,$3,'+');
                  $$ = add;
                  $$ = new ASTNode_Assign($1, add);
    }
  |  variable ASSIGN_SUB expression {
                  ASTNode * sub = new ASTNode_Math2($1,$3,'-');
                  $$ = sub;
                  $$ = new ASTNode_Assign($1, sub);
    }
  |  variable ASSIGN_MULT expression { // ensure '*=' starts correctly
                  ASTNode * mult = new ASTNode_Math2($1,$3,'*');
                  $$ = mult;
                  $$ = new ASTNode_Assign($1, mult);
    }
  |  variable ASSIGN_DIV expression { // ensure '/=' starts correctly
                  ASTNode * divv = new ASTNode_Math2($1,$3,'/');
                  $$ = divv;
                  $$ = new ASTNode_Assign($1, divv);
    }
  |  variable ASSIGN_MOD expression { // ensure '%=' starts correctly
                  //$1 = $1 - ($1/expression)*expression
                  ASTNode * mod = new ASTNode_Math2($1,$3,'%');
                  $$ = mod;
                  $$ = new ASTNode_Assign($1, mod);
   }
  ;


/*------------COMMANDS----------------------------------------------*/
command:        command_print { $$ = $1; }
       |        command_random{ $$ = $1; }

command_print:  COMMAND_PRINT parameter_list {

                  $$ = new ASTNode_Print();
                  for (int i = 0; i < $2->GetNumChildren(); i++) {
                    $$->AddChild($2->RemoveChild(i));
                  }
                  delete $2;
                }

command_random:     COMMAND_RANDOM '(' expression ')' {

              }

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
