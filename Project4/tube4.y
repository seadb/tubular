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

void yyerror(std::string err_string) {
  std::cout << "ERROR(line " << line_num << "): " << err_string << std::endl;
  exit(1);
}

bool check_types(ASTNode* lhs_in, ASTNode * rhs_in) {
	ASTNode * lhs = lhs_in;
	ASTNode * rhs = rhs_in;
	if (lhs->GetType() == rhs->GetType()) {
		return true;
                } else {
                        std::string err_string = "types do not match for assignment (lhs = '";
                        err_string += lhs->GetType();
                        err_string += "', rhs = '";
                        err_string += rhs->GetType();
                        err_string += "')";
                        yyerror(err_string);
                        exit(1);
                }
}

bool check_type_char(ASTNode* lhs_in, ASTNode * rhs_in) {
        ASTNode * lhs = lhs_in;
        ASTNode * rhs = rhs_in;
	std::string lhs_type = lhs->GetType(), rhs_type = rhs->GetType();
        if ((lhs_type == "char") || (rhs_type == "char")) {
                 std::string err_string = "cannot use type '";
		if (lhs_type == "char") {
                     err_string += lhs->GetType();
		} else {
		     err_string += rhs->GetType();
		}
                 err_string += "' in mathematical expressions";
                 yyerror(err_string);
                 exit(1);
        } else return true;
}

%}

%union {
  char * lexeme;
  ASTNode * ast_node;
}

%token<lexeme> ID INT_LITERAL CHAR_LITERAL TYPE_INT TYPE_CHAR IF BREAK WHILE ELSE
%token<lexeme> COMMAND_PRINT COMMAND_RANDOM CHAR_LITERAL_NEWLINE CHAR_LITERAL_TAB CHAR_LITERAL_QUOTE CHAR_LITERAL_BACKSLASH
%token<lexeme> COMP_EQU COMP_NEQU COMP_LESS COMP_LTE COMP_GTR COMP_GTE
%token<lexeme> BOOL_AND BOOL_OR
%token<lexeme> ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD
%type<ast_node> statement_list statement block declare declare_assign//
variable expression assignment operation compare literal negative//
command parameters if if_block while while_block not //

%right '=' ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD
%right '?' ':'
%left BOOL_OR
%left BOOL_AND
%nonassoc COMP_EQU COMP_NEQU COMP_LESS COMP_LTE COMP_GTR COMP_GTE
%left '+' '-'
%left '*' '/' '%'
%nonassoc UMINUS NOT
%left OPEN_BRACE CLOSE_BRACE

%%

program:        statement_list {
                  // This is always the last rule to run so $$ is the full AST
                  symbol_tables.ShowAll();
                  //cout << "before";
                  $1->CompileTubeIC(symbol_tables, fs);
                  //cout << "after";
	          //$1->DebugPrint();
                }

statement_list:  {
                  // Start the statement list by creating a block.
                  //symbol_tables.AddTable();
                 // symbol_tables.HideTable();
                  symbol_tables.AddTable();
                  $$ = new ASTNode_Block();
 //               std::cout << "IN STATEMENT_LIST 1" << std::endl;
                }
  | statement_list statement {
    $1->AddChild($2); // Add each statement to the block
    $$ = $1;          // Pass the block along
    }

statement:
                block {
 //                std::cout<< "IN BLOCK1" <<std::endl;
                  //ASTNode * b = new ASTNode_Block();
                  //b->AddChild($3);
                  $$ = $1;
                }
        |       declare  ';'      { $$ = $1; }
        |       declare_assign ';' { $$ = $1; }
        |       expression ';'        { $$ = $1; }
        |       command  ';'          { $$ = $1; }
        |       if          ';'       { $$ = $1; }
        |       if_block              { $$ = $1; }
        |       while ';'             { $$ = $1; }
        |       while_block           { $$ = $1; }

block: OPEN_BRACE {
    //    std::cout << "IN OPEN" << std::endl;
        //symbol_tables.AddTable();
    }
    statement_list {
     //             std::cout<< "IN MIDDLE STATEMENT LIST" << std::endl;
                  ASTNode * b = new ASTNode_Block();
                  b->AddChild($3);
                  $<ast_node>$ = b;
                 symbol_tables.HideTable();
   }
   CLOSE_BRACE {
       // std::cout<< "IN CLOSE" << std::endl;
        //symbol_tables.HideTable();
        $$ = $<ast_node>4;
    }

if:         IF '(' expression ')' expression { $$ = new ASTNode_If($3, $5); }
if_block:   IF '(' expression ')' block { $$ = new ASTNode_If($3, $5); }

while: WHILE '(' expression ')' expression { $$ = new ASTNode_While($3, $5); }
while_block: WHILE '(' expression ')' block { $$ = new ASTNode_While($3, $5); }

declare:  TYPE_INT ID {
 //   std::cout << "IN DECLARE" << std::endl;
    if (symbol_tables.current()->Lookup($2) != 0) {
      std::string err_string = "redeclaration of variable '";
      err_string += $2;
      err_string += "'";
      yyerror(err_string);
      exit(1);
      }

      $$ = new ASTNode_Variable( symbol_tables.AddEntry($2, $1)  );

    }
	| TYPE_CHAR ID {
  //  std::cout << "IN DECLARE" << std::endl;
    if (symbol_tables.current()->Lookup($2) != 0) {
      std::string err_string = "redeclaration of variable '";
      err_string += $2;
      err_string += "'";
      yyerror(err_string);
      exit(1);
      }

      $$ = new ASTNode_Variable( symbol_tables.AddEntry($2, $1)  );
    }


declare_assign:  declare '=' expression {
		if ($1->GetType() == $3->GetType()) {
                  $$ = new ASTNode_Assign($1, $3);
                } else {
			std::string err_string = "types do not match for assignment (lhs = '";
			err_string += $1->GetType();
			err_string += "', rhs = '";
			err_string += $3->GetType();
			err_string += "')";
			yyerror(err_string);
			exit(1);
		}
}

expression:     literal { $$ = $1; }
          |     negative { $$ = $1; }
    	  |	    not { $$ = $1; }
          |     variable { $$ = $1; }
          |     operation { $$ = $1; }
          |     compare { $$ = $1; }
          |     assignment { $$ = $1; }
          |     BREAK     { $$ = new ASTNode_Break(); }

literal:        INT_LITERAL {
                  $$ = new ASTNode_Literal($1, "int");
                }
		| CHAR_LITERAL {
		  $$ = new ASTNode_Literal($1, "char");
		}
		| CHAR_LITERAL_NEWLINE {
                  $$ = new ASTNode_Literal($1, "char");
                }
		| CHAR_LITERAL_TAB {
                  $$ = new ASTNode_Literal($1, "char");
                }
		| CHAR_LITERAL_QUOTE {
                  $$ = new ASTNode_Literal($1, "char");
                }
		| CHAR_LITERAL_BACKSLASH {
                  $$ = new ASTNode_Literal($1, "char");
                }


negative:       '-' expression %prec UMINUS {
		  if ($2->GetType() == "char") {
                        std::string err_string = "cannot use type '";
                        err_string += $2->GetType();
                        err_string += "' in mathematical expressions";
                        yyerror(err_string);
                        exit(1);
                  } else {
                        $$ = new ASTNode_Negation($2);
                  }
        }


not:		'!' expression %prec NOT {
		  if ($2->GetType() == "char") {
			std::string err_string = "cannot use type '";
			err_string += $2->GetType();
			err_string += "' in mathematical expressions";
			yyerror(err_string);
			exit(1);
		  } else {
		  	$$ = new ASTNode_Not($2);
		  }
	}

variable:      ID {
   // std::cout << "IN VARIABLE" << std::endl;
    tableEntry * entry = symbol_tables.Lookup($1);
        if (entry == 0) {
          std::string err_string = "unknown variable '";
          err_string += $1;
          err_string += "'";
          yyerror(err_string);
          exit(2);
        }
	$$ = new ASTNode_Variable( entry );
      }

operation:
         expression '+' expression {
		  check_type_char($1, $3);
                  $$ = new ASTNode_Math2($1, $3, '+');
         }
    |    expression '-' expression {
                  check_type_char($1, $3);
                  $$ = new ASTNode_Math2($1, $3, '-');
         }
    |    expression '*' expression {
                  check_type_char($1, $3);
                  $$ = new ASTNode_Math2($1, $3, '*');
         }
    |    expression '/' expression {
                  check_type_char($1, $3);
                  $$ = new ASTNode_Math2($1, $3, '/');
         }
    |    expression '%' expression {
                  check_type_char($1, $3);
                  $$ = new ASTNode_Math2($1, $3, '%');
         }
    |    '(' expression ')' {
                  $$ = $2;
         }
    |    COMMAND_RANDOM '(' expression ')' {
		if ($3->GetType() == "char") {
			 std::string err_string = "cannot use type '";
                 	err_string += $3->GetType();
                 	err_string += "' as an argument to random";
                 	yyerror(err_string);
                 	exit(1);
		} else {
                  $$ = new ASTNode_Random($3);
                }
	}
assignment:
          variable '=' expression {
                  check_types($1, $3);
            $$ = new ASTNode_Assign($1, $3);
          }
    |     variable ASSIGN_ADD expression {
                  check_type_char($1, $3);
            $$ = new ASTNode_MathAssign($1, $3, '+');
          }
    |     variable ASSIGN_SUB expression {
                  check_type_char($1, $3);
            $$ = new ASTNode_MathAssign($1, $3, '-');
          }
    |     variable ASSIGN_MULT expression {
                  check_type_char($1, $3);
            $$ = new ASTNode_MathAssign($1, $3, '*');
          }
    |     variable ASSIGN_DIV expression {
                  check_type_char($1, $3);
            $$ = new ASTNode_MathAssign($1, $3, '/');
          }
    |     variable ASSIGN_MOD expression {
                  check_type_char($1, $3);
            $$ = new ASTNode_MathAssign($1, $3, '%');
          }
compare:
        expression COMP_EQU expression {
		  check_types($1, $3);
      	          $$ = new ASTNode_Comparison($1, $3, "==");
   	}
    	|  expression COMP_NEQU expression {
                  check_types($1, $3);
              	  $$ = new ASTNode_Comparison($1, $3, "!=");
    	}
    	|  expression COMP_GTE expression {
                  check_types($1, $3);
               	  $$ = new ASTNode_Comparison($1, $3, ">=");
    	}
    	|  expression COMP_LESS expression {
                  check_types($1, $3);
                  $$ = new ASTNode_Comparison($1, $3, "<");
       	}
	|  expression COMP_LTE expression {
                  check_types($1, $3);
                  $$ = new ASTNode_Comparison($1, $3, "<=");
       	}
    	|  expression COMP_GTR expression {
                  check_types($1, $3);
                  $$ = new ASTNode_Comparison($1, $3, ">");
       	}
    	|  expression BOOL_AND expression {
                  check_type_char($1, $3);
                  $$ = new ASTNode_Logical($1, $3, "&&");
       	}
    	|  expression BOOL_OR expression {
                  check_type_char($1, $3);
                  $$ = new ASTNode_Logical($1, $3, "||");
       	}
    	|  expression '?' expression ':' expression {
                  check_types($1, $3);
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
