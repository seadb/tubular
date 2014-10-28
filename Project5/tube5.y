%{
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>

#include "symbol_table.h"
#include "ast.h"
#include "type_info.h"

#define YYDEBUG 1

extern int line_num;
extern int yylex();
extern std::string out_filename;
 
CSymbolTable symbol_table;
int error_count = 0;

// Create an error function to call when the current line has an error
void yyerror(std::string errString) {
  std::cout << "ERROR(line " << line_num << "): "
       << errString << std::endl;
  error_count++;
}

// Create an alternate error function when a *different* line than being read in has an error.
void yyerror2(std::string errString, int orig_line) {
  std::cout << "ERROR(line " << orig_line << "): "
       << errString << std::endl;
  error_count++;
}

%}

%union {
  char * lexeme;
  ASTNode * ast_node;
}

%token ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD ARRAY //
COMP_EQU COMP_NEQU COMP_LESS COMP_LTE COMP_GTR COMP_GTE BOOL_AND BOOL_OR//
COMMAND_PRINT COMMAND_IF COMMAND_ELSE COMMAND_WHILE COMMAND_BREAK COMMAND_FOR//
COMMAND_RANDOM SIZE RESIZE
%token <lexeme> INT_LIT CHAR_LIT STRING_LIT UNTERM_STRING ID TYPE

%right '=' ASSIGN_ADD ASSIGN_SUB ASSIGN_MULT ASSIGN_DIV ASSIGN_MOD
%left BOOL_OR
%left BOOL_AND
%nonassoc COMP_EQU COMP_NEQU COMP_LESS COMP_LTE COMP_GTR COMP_GTE
%left '+' '-'
%left '*' '/' '%'
%nonassoc UMINUS '!'
%left '.'

%nonassoc NOELSE
%nonassoc COMMAND_ELSE


%type <ast_node> declare expression declare_assign statement statement_list//
variable command argument_list code_block if_start while_start flow_control//
literal assignment operation compare negative not_
%%

program:      statement_list {
                 ICArray ic_array;             // Array to contain the IC 
                 $1->CompileTubeIC(symbol_table, ic_array); //Fill IC array 
                 std::ofstream out_file(out_filename.c_str());  // Open the output file
                 ic_array.PrintIC(out_file);             // Write ic to output file
              }
       ;

statement_list:   {
             $$ = new ASTNodeBlock;
                 }
  |        statement_list statement {
                   if ($2 != NULL) $1->AddChild($2);
                   $$ = $1;
     }
  ;

statement:   declare ';'        {  $$ = $1;  }
        |    declare_assign ';' {  $$ = $1;  }
        |    expression ';'     {  $$ = $1;  }
        |    command ';'        {  $$ = $1;  }
        |    flow_control       {  $$ = $1;  }
        |    code_block         {  $$ = $1;  }
        |    ';'                {  $$ = NULL;  }
  ;

code_block:  block_start statement_list block_end { $$ = $2; } ;
block_start: '{' { symbol_table.IncScope(); } ;
block_end:   '}' { symbol_table.DecScope(); } ;

declare:  TYPE ID {
            if (symbol_table.InCurScope($2) != 0) {
              std::string errString = "redeclaration of variable '";
              errString += $2;
              errString += "'";
              yyerror(errString);
              exit(1);
            }
            
            std::string type_name = $1;
            int type_id = 0;
            
            if (type_name == "int") type_id = Type::INT;
            else if (type_name == "char") type_id = Type::CHAR;
            else if (type_name == "string") type_id = Type::CHAR_ARRAY;
            else {
              std::string errString = "unknown type '";
              errString += $1;
              errString += "'";
              yyerror(errString);
            }
            
            CTableEntry * cur_entry = symbol_table.AddEntry(type_id, $2);

            $$ = new ASTNodeVariable(cur_entry);
            $$->SetLineNum(line_num);
          }
      |   ARRAY '(' TYPE ')' ID {
            if (symbol_table.InCurScope($5) != 0) {
              std::string errString = "redeclaration of variable '";
              errString += $5;
              errString += "'";
              yyerror(errString);
              exit(1);
            }
            
            std::string type_name = $3;
            int type_id = 0;

            if (type_name == "int") type_id = Type::INT_ARRAY;
            else if (type_name == "char") type_id = Type::CHAR_ARRAY;
            else {
              std::string errString = "unknown type '";
              errString += $3;
              errString += "'";
              yyerror(errString);
            }
            
            CTableEntry * cur_entry = symbol_table.AddEntry(type_id, $5);
            
            $$ = new ASTNodeVariable(cur_entry);
            $$->SetLineNum(line_num);
      
      }
      ;

declare_assign:  declare '=' expression {
             $$ = new ASTNodeAssign($1, $3);
                   $$->SetLineNum(line_num);
           }
  ;

expression:     literal    { $$ = $1; }
          |     negative   { $$ = $1; }
          |     not_       { $$ = $1; }
          |     variable   { $$ = $1; }
          |     operation  { $$ = $1; }
          |     compare    { $$ = $1; }
          |     assignment { $$ = $1; }

literal:   
       INT_LIT {
               $$ = new ASTNodeLiteral(Type::INT, (std::string)$1);
               $$->SetLineNum(line_num);
             }
  |    CHAR_LIT {
               $$ = new ASTNodeLiteral(Type::CHAR, (std::string)$1);
               $$->SetLineNum(line_num);
             }
  |    STRING_LIT {
              std::string literal = $1;
              bool good = (literal[0] == '"');
              good = good && (literal[literal.size() - 1] == '"');
              good = good && (literal[literal.size() - 2] != '\\');
              for(int i = 1; i < literal.size() - 1; i++)
              {
                if(literal[i] == '"' && literal[i - 1] != '\\')
                 good = false;
              }
              if(!good)
                yyerror("Unterminated string.");
              $$ = new ASTNodeLiteral(Type::CHAR_ARRAY,
                        literal.substr(1,literal.size()-2));
              $$->SetLineNum(line_num);
            }
          
  |    UNTERM_STRING {
            yyerror("Unterminated string.");
            }
negative:    '-' expression %prec UMINUS {
               $$ = new ASTNodeMath1($2, '-');
               $$->SetLineNum(line_num);
             }

not_:         '!' expression %prec UMINUS {
               $$ = new ASTNodeMath1($2, '!');
               $$->SetLineNum(line_num);
             }

variable:   ID {
                CTableEntry * cur_entry = symbol_table.Lookup($1);
                if (cur_entry == NULL) {
                  std::string errString = "unknown variable '";
                  errString += $1;
                  errString += "'";
                  yyerror(errString);
                  exit(1);
                }
                $$ = new ASTNodeVariable(cur_entry);
                $$->SetLineNum(line_num);
             }

        |    ID '[' expression ']' {
                CTableEntry * cur_entry = symbol_table.Lookup($1);
                if (cur_entry == NULL) 
                {
                  std::string errString = "unknown variable '";
                  errString += $1;
                  errString += "'";
                  yyerror(errString);
                  exit(1);
                }

                //$$ = new ASTNodeVariable(cur_entry);

                $$ = new ASTNodeIndex(cur_entry,$3);
                $$->SetLineNum(line_num);
        }
        |   ID '.' SIZE '(' ')' 
        {
        //These are in the wrong spot. things in this section
        //should be for left side of the equation only
        //id.size() = 3; is an invalid expression and should throw an error..

                CTableEntry * cur_entry = symbol_table.Lookup($1);
                if (cur_entry == NULL) 
                {
                  std::string errString = "unknown variable '";
                  errString += $1;
                  errString += "'";
                  yyerror(errString);
                  exit(1);
                }
                $$ = new ASTNodeSize(cur_entry);
            }
        |   ID '.' RESIZE '(' expression ')' {
                CTableEntry * cur_entry = symbol_table.Lookup($1);
                if (cur_entry == NULL) 
                {
                  std::string errString = "unknown variable '";
                  errString += $1;
                  errString += "'";
                  yyerror(errString);
                  exit(1);
                }
                $$ = new ASTNodeResize(cur_entry, $5);
            }
	|   ID '.' ID '(' ')' {
		std::string errString = "unknown method '";
		errString += $3;
		errString += "'";
		yyerror(errString);
		exit(1);
		}
	|   ID '.' ID '(' expression ')' {
                std::string errString = "unknown method '";
                errString += $3;
                errString += "'";
                yyerror(errString);
                exit(1);
                }
        | expression '[' expression ']' {
                int type_id = $3->GetType();
                if(type_id!=Type::INT_ARRAY && type_id !=Type::CHAR_ARRAY)
                {
                  std::string type_str = Type::AsString($3->GetType());
		  std::string errString = "array methods cannot be run on type  '";
		  errString += type_str;
		  errString += "'";
		  yyerror(errString);
		  exit(1);
		}
        } 
;

operation:
       expression '+' expression { 
              $$ = new ASTNodeMath2($1, $3, '+');
              $$->SetLineNum(line_num);
             }
  |    expression '-' expression {
              $$ = new ASTNodeMath2($1, $3, '-');
              $$->SetLineNum(line_num);
             }
  |    expression '*' expression {
              $$ = new ASTNodeMath2($1, $3, '*');
              $$->SetLineNum(line_num);
             }
  |    expression '/' expression {
              $$ = new ASTNodeMath2($1, $3, '/');
              $$->SetLineNum(line_num);
             }
  |    expression '%' expression {
              $$ = new ASTNodeMath2($1, $3, '%');
              $$->SetLineNum(line_num);
             }
  |    '(' expression ')' { $$ = $2; } // Ignore parens used for order

  |    COMMAND_RANDOM '(' expression ')'{
              $$ = new ASTNodeRandom($3);
             }
 /* |    expression '?' expression ':' expression {}
  }
   */           
             
compare: 
       
       expression COMP_NEQU expression {
               $$ = new ASTNodeMath2($1, $3, COMP_NEQU);
               $$->SetLineNum(line_num);
             }
  |    expression COMP_EQU expression {
              $$ = new ASTNodeMath2($1, $3, COMP_EQU);
              $$->SetLineNum(line_num);
             }
  |    expression COMP_LESS expression {
               $$ = new ASTNodeMath2($1, $3, COMP_LESS);
               $$->SetLineNum(line_num);
             }
  |    expression COMP_LTE expression {
               $$ = new ASTNodeMath2($1, $3, COMP_LTE);
               $$->SetLineNum(line_num);
             }
  |    expression COMP_GTR expression {
               $$ = new ASTNodeMath2($1, $3, COMP_GTR);
               $$->SetLineNum(line_num);
             }
  |    expression COMP_GTE expression {
               $$ = new ASTNodeMath2($1, $3, COMP_GTE);
               $$->SetLineNum(line_num);
             }
  |    expression BOOL_AND expression {
               $$ = new ASTNodeBool2($1, $3, '&');
               $$->SetLineNum(line_num);
             }
  |    expression BOOL_OR expression {
               $$ = new ASTNodeBool2($1, $3, '|');
               $$->SetLineNum(line_num);
             }
assignment:
       variable '=' expression {
               $$ = new ASTNodeAssign($1, $3);
               $$->SetLineNum(line_num);
             }
  |    variable ASSIGN_ADD expression {
               $$ = new ASTNodeAssign($1, new ASTNodeMath2($1, $3, '+') );
               $$->SetLineNum(line_num);
             }
  |    variable ASSIGN_SUB expression {
               $$ = new ASTNodeAssign($1, new ASTNodeMath2($1, $3, '-') );
               $$->SetLineNum(line_num);
             }
  |    variable ASSIGN_MULT expression {
               $$ = new ASTNodeAssign($1, new ASTNodeMath2($1, $3, '*') );
               $$->SetLineNum(line_num);
             }
  |    variable ASSIGN_DIV expression {
               $$ = new ASTNodeAssign($1, new ASTNodeMath2($1, $3, '/') );
               $$->SetLineNum(line_num);
             }
  |    variable ASSIGN_MOD expression {
               $$ = new ASTNodeAssign($1, new ASTNodeMath2($1, $3, '%') );
               $$->SetLineNum(line_num);
             }
  ;

argument_list:  argument_list ',' expression {
                  ASTNode * node = $1; // Grab the node used for arg list.
                  node->AddChild($3);    // Save this argument in the node.
                  $$ = node;
    }
  |  expression {
                  // Create a temporary AST node to hold the arg list.
                  $$ = new ASTNodeTempNode(Type::VOID);
                  $$->AddChild($1);   // Save this argument in the temp node.
                  $$->SetLineNum(line_num);
    }
  ;

command:   COMMAND_PRINT argument_list {
             $$ = new ASTNodePrint(NULL);
             $$->TransferChildren($2);
             $$->SetLineNum(line_num);
             delete $2;
           }
        |  COMMAND_BREAK {
             $$ = new ASTNodeBreak();
             $$->SetLineNum(line_num);
           }
  ;

if_start:  COMMAND_IF '(' expression ')' {
             $$ = new ASTNodeIf($3, NULL, NULL);
             $$->SetLineNum(line_num);
           }
        ;

while_start:  COMMAND_WHILE '(' expression ')' {
                $$ = new ASTNodeWhile($3, NULL);
                $$->SetLineNum(line_num);
              }
           ;

flow_control:  if_start statement COMMAND_ELSE statement {
                 $$->SetChild(1, $2);
                 $$->SetChild(2, $4);
                 $$ = $1;
               }
            |  if_start statement %prec NOELSE {
                 $$ = $1;
                 $$->SetChild(1, $2);
               }
            |  while_start statement {
                 $$ = $1;
                 $$->SetChild(1, $2);
               }
            ;


%%
void LexMain(int argc, char * argv[]);

int main(int argc, char * argv[])
{
  error_count = 0;
  LexMain(argc, argv);

  yyparse();

  if (error_count == 0) std::cout << "Parse Successful!" << std::endl;

  return 0;
}
