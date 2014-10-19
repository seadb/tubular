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
CSymbolTables symbol_tables;

void yyerror(std::string err_string) {
  std::cout << "ERROR(line " << line_num << "): " << err_string << std::endl;
  exit(1);
}

%}

%union {
  char * lexeme;
  CNode * ast_node;
}

%token<lexeme> ID INT_LITERAL CHAR_LITERAL STRING_LITERAL
%token<lexeme> TYPE_INT TYPE_CHAR TYPE_ARRAY TYPE_STRING
%token<lexeme> IF BREAK WHILE ELSE FOR
%token<lexeme> COMMAND_PRINT COMMAND_RANDOM
%token<lexeme> CHAR_LITERAL_NEWLINE CHAR_LITERAL_TAB CHAR_LITERAL_QUOTE //
               CHAR_LITERAL_BACKSLASH 
%token<lexeme> COMP_EQU COMP_NEQU COMP_LESS COMP_LTE COMP_GTR COMP_GTE
%token<lexeme> BOOL_AND BOOL_OR
%token<lexeme> ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD
%type<ast_node> statement_list statement block declare declare_assign//
variable expression assignment operation compare literal negative flow_control//
command parameters if_ while_ not_ for_ //
  /* if_ while_ not_ have a '_' so they won't clash w/ c types 'if while not'*/


%left OPEN_BRACE CLOSE_BRACE
%right '=' ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD
%right '?' ':'
%left BOOL_OR
%left BOOL_AND
%nonassoc COMP_EQU COMP_NEQU COMP_LESS COMP_LTE COMP_GTR COMP_GTE
%left '+' '-'
%left '*' '/' '%'
%nonassoc UMINUS NOT

%%

program:        statement_list {
                  // This is always the last rule to run so $$ is the full AST
                  symbol_tables.ShowAll();
                  $1->CompileTubeIC(symbol_tables, fs);
                  //$1->DebugPrint();
                }

statement_list:  {
                  // Start the statement list by creating a block.
                  symbol_tables.AddTable();
                  $$ = new CNodeBlock();
                 }
| statement_list statement {
                          $1->AddChild($2); // Add each statement to the block
                          $$ = $1;          // Pass the block along
  }

statement:
                block               { $$ = $1; }
        |       declare  ';'        { $$ = $1; }
        |       declare_assign ';'  { $$ = $1; }
        |       flow_control        { $$ = $1; }
        |       expression ';'      { $$ = $1; }
        |       command  ';'        { $$ = $1; }


block: OPEN_BRACE     { ; }
       statement_list {
                        CNode * b = new CNodeBlock();
                        b->AddChild($3);
                        $<ast_node>$ = b;
                        symbol_tables.HideTable();
       }
       CLOSE_BRACE    { $$ = $<ast_node>4; }


declare:  TYPE_INT ID {
            if (symbol_tables.current()->Lookup($2) != 0) {
              std::string err_string = "redeclaration of variable '";
              err_string += $2;
              err_string += "'";
              yyerror(err_string);
              exit(1);
              }
            $$ = new CNodeVariable( symbol_tables.AddEntry($2, $1)  );
            }
       |  TYPE_CHAR ID {
            if (symbol_tables.current()->Lookup($2) != 0) {
              std::string err_string = "redeclaration of variable '";
              err_string += $2;
              err_string += "'";
              yyerror(err_string);
              exit(1);
              }
            $$ = new CNodeVariable( symbol_tables.AddEntry($2, $1)  );
            }

       | TYPE_ARRAY '(' TYPE_CHAR ')' ID {
       
       }

       | TYPE_ARRAY '(' TYPE_INT ')' ID {
       
       }

       | TYPE_STRING ID 
       {
         $$ = new CNodeVariable( symbol_tables.AddArray($2, $1, 0));
       }

declare_assign:  declare '=' expression {
                   $$ = new CNodeAssign($1, $3, line_num);
                 }

flow_control:   if_                 { $$ = $1; }
        |       while_              { $$ = $1; }
        |       for_                { $$ = $1; }

if_:        IF '(' expression ')' statement 
              { $$ = new CNodeIf($3, $5, line_num); }

   |        IF '(' expression ')' ';' { 
              CNode * blank = new CNodeBlank();
              $$ = new CNodeIf($3, blank, line_num); 
            }
 
   |        IF '(' expression ')' statement ELSE statement 
              { $$ = new CNodeElse($3, $5, $7, line_num); }
       
   |        IF '(' expression ')' ';' ELSE statement{
                CNode * blank = new CNodeBlank();
                $$ = new CNodeElse($3, blank, $7, line_num); }

   |        IF '(' expression ')' statement ELSE ';'{
                CNode * blank = new CNodeBlank();
                $$ = new CNodeElse($3, $5, blank, line_num); }
   
while_:     WHILE '(' expression ')' statement
               { $$ = new CNodeWhile($3, $5, line_num); }

for_:       FOR '(' expression ';' expression ';' expression ')' statement
               { $$ = new CNodeFor($3, $5, $7, $9, line_num); }

    |       FOR '(' declare_assign ';' expression ';' expression ')' statement
               { $$ = new CNodeFor($3, $5, $7, $9, line_num); }

    |       FOR '(' ';' ';' ')' statement
              { CNode * temp = new CNodeBlank(); 
                $$ = new CNodeFor(temp, temp, temp, $6, line_num);
              }
expression:     literal    { $$ = $1; }
          |     negative   { $$ = $1; }
          |     not_       { $$ = $1; }
          |     variable   { $$ = $1; }
          |     operation  { $$ = $1; }
          |     compare    { $$ = $1; }
          |     assignment { $$ = $1; }
          |     BREAK      { $$ = new CNodeBreak(line_num); }

literal:        INT_LITERAL {
                  $$ = new CNodeLiteral($1, "int");
                }
       |        CHAR_LITERAL {
                  $$ = new CNodeLiteral($1, "char");
                }

       |        STRING_LITERAL {
                  $$ = new CNodeLiteral($1, "char_array");
                }
  /*
       |        CHAR_LITERAL_NEWLINE {
                  $$ = new CNodeLiteral($1, "char");
                }
       |        CHAR_LITERAL_TAB {
                  $$ = new CNodeLiteral($1, "char");
                }
       |        CHAR_LITERAL_QUOTE {
                  $$ = new CNodeLiteral($1, "char");
                }
       |        CHAR_LITERAL_BACKSLASH {
                  $$ = new CNodeLiteral($1, "char");
                }
  */

negative: '-' expression %prec UMINUS {
            $$ = new CNodeNegation($2, line_num);
            }

not_:      '!' expression %prec NOT {
            $$ = new CNodeNot($2, line_num);
            }

variable:     ID {
                CTableEntry * entry = symbol_tables.Lookup($1);
                if (entry == 0) {
                  std::string err_string = "unknown variable '";
                  err_string += $1;
                  err_string += "'";
                  yyerror(err_string);
                  exit(2);
                  }
                $$ = new CNodeVariable( entry );
                }

operation:
         expression '+' expression {
                  $$ = new CNodeMath2($1, $3, '+', line_num);
         }
    |    expression '-' expression {
                  $$ = new CNodeMath2($1, $3, '-', line_num);
         }
    |    expression '*' expression {
                  $$ = new CNodeMath2($1, $3, '*', line_num);
         }
    |    expression '/' expression {
                  $$ = new CNodeMath2($1, $3, '/', line_num);
         }
    |    expression '%' expression {
                  $$ = new CNodeMath2($1, $3, '%', line_num);
         }
    |    '(' expression ')' {
                  $$ = $2;
         }
    |    COMMAND_RANDOM '(' expression ')' {
                  $$ = new CNodeRandom($3, line_num);
  }

assignment:
          variable '=' expression {
            $$ = new CNodeAssign($1, $3, line_num);
          }
    |     variable ASSIGN_ADD expression {
            $$ = new CNodeMathAssign($1, $3, '+', line_num);
          }
    |     variable ASSIGN_SUB expression {
            $$ = new CNodeMathAssign($1, $3, '-', line_num);
          }
    |     variable ASSIGN_MULT expression {
            $$ = new CNodeMathAssign($1, $3, '*', line_num);
          }
    |     variable ASSIGN_DIV expression {
            $$ = new CNodeMathAssign($1, $3, '/', line_num);
          }
    |     variable ASSIGN_MOD expression {
            $$ = new CNodeMathAssign($1, $3, '%', line_num);
          }
compare:
           expression COMP_EQU expression {
                  $$ = new CNodeComparison($1, $3, "==", line_num);
           }
        |  expression COMP_NEQU expression {
                  $$ = new CNodeComparison($1, $3, "!=", line_num);
           }
        |  expression COMP_GTE expression {
                    $$ = new CNodeComparison($1, $3, ">=", line_num);
           }
        |  expression COMP_LESS expression {
                    $$ = new CNodeComparison($1, $3, "<", line_num);
           }
        |  expression COMP_LTE expression {
                    $$ = new CNodeComparison($1, $3, "<=", line_num);
           }
        |  expression COMP_GTR expression {
                    $$ = new CNodeComparison($1, $3, ">", line_num);
           }
        |  expression BOOL_AND expression {
                    $$ = new CNodeLogical($1, $3, "&&", line_num);
           }
        |  expression BOOL_OR expression {
                    $$ = new CNodeLogical($1, $3, "||", line_num);
           }
        |  expression '?' expression ':' expression {
                    $$ = new CNodeConditional($1, $3, $5, line_num);
           }

command:   COMMAND_PRINT parameters {
             $$ = new CNodePrint();
             for (int i = 0; i < $2->GetNumChildren(); i++) {
               $$->AddChild($2->RemoveChild(i));
             }
             delete $2;
           }

parameters:  expression {
              $$ = new CNodeTemp();
              $$->AddChild($1);
             }
          |  parameters ',' expression {
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
