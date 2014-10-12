%{
#include <iostream>
#include <fstream>

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

%type<ast_node> statement_list statement var_declare var_declare_assign var_usage expression command param_list //
block open close

%right '=' ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD
%right '?' ':'
%left BOOL_OR
%left BOOL_AND
%nonassoc COMP_EQU COMP_NEQU COMP_LESS COMP_LTE COMP_GTR COMP_GTE
%left '+' '-'
%left '*' '/' '%'
%nonassoc UMINUS

%%

program:        statement_list {
                  // This is always the last rule to run so $$ is the full AST
                  $1->CompileTubeIC(symbol_tables, fs);
                  //$1->DebugPrint();
                }

statement_list:	{
                  // Start the statement list by creating a block.
						symbol_tables.AddTable();
                  $$ = new ASTNode_Block();
                }
	|	statement_list statement ';' {
                  $1->AddChild($2); // Add each statement to the block
                  $$ = $1;          // Pass the block along
		}
  | statement_list block {
      $1->AddChild($2);
      $$ = $1;
  }

block:
     open close { $$ = new ASTNode_Block (); }

open:   '{'  statement_list {
    symbol_tables.AddTable();
     /*symbolTable temp;
     temp.SetVisible(true);
     std::vector<symbolTable>::iterator iterator = symbol_tables.begin();
     symbol_tables.insert(iterator+scope, temp);
    */
     //$$ = new ASTNode_Block( );
     };
close: '}'  {
     symbol_tables.PopTable();
    /*
    // TODO: remove symbol table and put it in the discard pile
    symbolTable temp = symbol_tables.back(); //last element
    std::vector<symbolTable>::iterator it = discarded_tables.begin();
    discarded_tables.insert(it+scope, temp); //insert into discarded
    symbol_tables.pop_back(); //delete last element
    scope +=1;*/

     };


statement:      var_declare        { $$ = $1; }
        |       var_declare_assign { $$ = $1; }
        |       expression         { $$ = $1; }
        |       command { $$ = $1; }

var_declare:	TYPE ID {

    if (symbol_tables.Lookup($2) != 0) {
      std::string err_string = "re-declaring variable '";
      err_string += $2;
      err_string += "'";
      yyerror(err_string);
      exit(1);
      }

      $$ = new ASTNode_Variable( symbol_tables.AddEntry($2) );

                }

var_declare_assign:  var_declare '=' expression {
                  $$ = new ASTNode_Assign($1, $3);
                }

var_usage:      ID {
                  tableEntry * entry = symbol_tables.Lookup($1);

                  $$ = new ASTNode_Variable( entry );
			 /*tableEntry * temp;
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
          }*/

        if (entry == 0) {
          std::string err_string = "unknown variable '";
          err_string += $1;
          err_string += "'";
          yyerror(err_string);
          exit(2);
        }


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
                  $$ = new ASTNode_Math2($1, $3, '%');
                }
        |       '-' expression %prec UMINUS {
                  $$ = new ASTNode_Negation($2);
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
        |       expression COMP_EQU expression {
                  $$ = new ASTNode_Comparison($1, $3, "==");
                }
        |       expression COMP_NEQU expression {
                  $$ = new ASTNode_Comparison($1, $3, "!=");
                }
        |       expression COMP_GTE expression {
                  $$ = new ASTNode_Comparison($1, $3, ">=");
                }
        |       expression COMP_LESS expression {
                  $$ = new ASTNode_Comparison($1, $3, "<");
                }
        |       expression COMP_LTE expression {
                  $$ = new ASTNode_Comparison($1, $3, "<=");
                }
        |       expression COMP_GTR expression {
                  $$ = new ASTNode_Comparison($1, $3, ">");
                }
        |       expression BOOL_AND expression {
                  $$ = new ASTNode_Logical($1, $3, "&&");
                }
        |       expression BOOL_OR expression {
                  $$ = new ASTNode_Logical($1, $3, "||");
                }
        |       expression '?' expression ':' expression {
                  $$ = new ASTNode_Conditional($1, $3, $5);
                }
        |       '(' expression ')' {
                  $$ = $2;
                }
        |       var_usage '=' expression {
                  $$ = new ASTNode_Assign($1, $3);
                }
        |       COMMAND_RANDOM '(' expression ')' {
                  $$ = new ASTNode_Random($3);
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
  fs.open(argv[2]);
  if (!fs.is_open()) {
    std::cerr << "Error opening file " << argv[2] << std::endl;
    exit(2);
  }
  yyparse();
  std::cout << "Parse Successful!" << std::endl;
  return 0;
}
