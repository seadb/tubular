/*
 * Authors: Chelsea Bridson
 */
%{
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <vector>

#include "symbol_table.h"
#include "ast.h"

#include <stdio.h>
#define YYDEBUG 1


extern int line_count;
extern int yylex();
extern std::string out_filename;

//std::map <std::string, Var *> symbol_table; //symbol table
symbolTable symbol_table;

std::vector<symbolTable> discarded_tables;
std::vector<symbolTable> symbol_tables;
int scope = 0;

void yyerror(std::string err_string) {
  std::cout << "ERROR(line " << line_count << "): " << err_string << std::endl;
  exit(1);
}
%}

%union {
  char * lexeme;
  ASTNode * ast_node;
}

%token<lexeme> ID INT_LITERAL TYPE COMMAND_PRINT COMMAND_RANDOM OPEN_BRACE CLOSE_BRACE
%type<ast_node> statement_list statement declare expression command declaration//
declare_assign literal operation compare assignment parameter_list command_print//
negative variable block open close

%right ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD '='
%right '?' ':'
%left BOOL_OR
%left BOOL_AND
%left LOGIC_OR
%left LOGIC_AND
%left COMP_NEQU COMP_EQU COMP_LTE COMP_GTE COMP_GTR COMP_LESS
%left '+' '-'
%left '*' '/' '%'
%nonassoc UMINUS

%%

program:        statement_list {
                // This is always the last rule to run so $$ is the full AST
                //$1->DebugPrint();
                std::ofstream out_file;
                out_file.open(out_filename.c_str());
                std::ofstream & out = out_file;
                //$1->CompileTubeIC(symbol_table, out_file);
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
  | statement_list block {
      $1->AddChild($2);
      $$ = $1;
  }
;
//I am not sure if this is a good way to implement the code blocks yet
block:
     open close { $$ = new ASTNode_Block (); }

open:     OPEN_BRACE statement_list {
     scope += 1;
     symbolTable temp;
     temp.SetVisible(true);
     std::vector<symbolTable>::iterator iterator = symbol_tables.begin();
     symbol_tables.insert(iterator+scope, temp);

     //$$ = new ASTNode_Block( );
     };
close:  CLOSE_BRACE {
    // TODO: remove symbol table and put it in the discard pile
    symbolTable temp = symbol_tables.back(); //last element
    std::vector<symbolTable>::iterator it = discarded_tables.begin();
    discarded_tables.insert(it+scope, temp); //insert into discarded
    symbol_tables.pop_back(); //delete last element
    scope +=1;
    // PROBLEM:  can there be an empty element at 2, with elements at 1 and 3?
    // this is problem because variable searchs each element
    // ie: vector = <symbolTable, , symbolTable>

     }
statement:
    declare     { $$ = $1; }
  | declare_assign { $$ = $1; }
  | expression  { $$ = $1; }
  | command     { $$ = $1; }

declare: declaration { $$ = $1; }

  declaration:  TYPE ID {
    if (symbol_tables[scope].Lookup($2) != 0) {
      std::string err_string = "re-declaring variable '";
      err_string += $2;
      err_string += "'";
      yyerror(err_string);
      exit(1);
      }

      $$ = new ASTNode_Variable( symbol_table.AddEntry($2) );
     }

  declare_assign:    declaration '=' expression {   // newly declared variable
        $$ = new ASTNode_Assign($1, $3);
      }


 /*----------EXPRESSION---------------------------------------------*/
expression:   literal     { $$ = $1; }
          |   negative    { $$ = $1; }
          |   variable    { $$ = $1; }
          |   operation   { $$ = $1; }
          |   compare     { $$ = $1; }
          |   assignment  { $$ = $1; }

  literal:  INT_LITERAL { // Integer
              $$ = new ASTNode_Literal($1);
    }
  negative: '-' expression %prec UMINUS {
              ASTNode * temp = new ASTNode_Literal("-1");
              $$ = temp;
              $$ = new ASTNode_Math2(temp, $2, '*');
    }
  ;

variable:  ID { // Identifier
          tableEntry * temp;
          for(int i=scope-1; i>=0; i--) { //start with most recent scope
            symbolTable itr = symbol_tables.at(i);
            if(itr.Visible())
            {
              temp = symbol_tables[i].Lookup($1);
            }
            if ( temp != 0)
            {
              $$ = new ASTNode_Variable( temp);
              break;
            }
          }

        if (temp == 0) {
          std::string err_string = "unknown variable '";
          err_string += $1;
          err_string += "'";
          yyerror(err_string);
          exit(2);
        }

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
   |  COMMAND_RANDOM '(' expression ')' {
              $$ = new ASTNode_Random($3);
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
      $$ = new ASTNode_Compare($1, $3, '<=');
    }
  |  expression COMP_GTR expression { // greater than
      $$ = new ASTNode_Compare($1, $3, '>');
    }
  |  expression COMP_GTE expression { // >=
      $$ = new ASTNode_Compare($1, $3, '>=');
    }
  |  expression BOOL_AND expression { // &&
      $$ = new ASTNode_Compare($1, $3, '&&');
    }
  |  expression BOOL_OR expression { // ||
      $$ = new ASTNode_Compare($1, $3, '||');
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

command_print:  COMMAND_PRINT parameter_list {

                  $$ = new ASTNode_Print();
                  for (int i = 0; i < $2->GetNumChildren(); i++) {
                    $$->AddChild($2->RemoveChild(i));
                  }
                  delete $2;
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
