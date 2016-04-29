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
extern bool debug;
extern bool ICmode;
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
COMMAND_RANDOM COMMAND_DEFINE COMMAND_RETURN SIZE RESIZE
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


%type <ast_node> variable_declare expression declare_assign statement statement_list//
variable command argument_list code_block if_start while_start flow_control//
literal assignment operation compare negative not_ function_declare function_argument_list//
function_arguments args_begin args_end function_define function_call

%%

program:      statement_list {
                 ICArray ic_array;             // Array to contain the IC

                 //this line is causing a seg fault
                 $1->CompileTubeIC(symbol_table, ic_array); //Fill IC array

                 //std::cout << "statement_list" << std::endl;
                 std::ofstream out_file(out_filename.c_str());  // Open the output file
                 ic_array.static_memory_size = symbol_table.GetTempVarID();
                 if (ICmode) {
                   ic_array.PrintIC(out_file);                  // Write IC to output file!
                 } else {
                   ic_array.PrintTC(out_file);            // Write Tubecode Assembly to output file!
                 }

              }
       ;

statement_list:   {
             $$ = new ASTNodeBlock;
                 }
  |        statement_list statement {
                   if ($2 != NULL) $1->AddChild($2);
                   $$ = $1;
     }
  |
          statement_list function_declare {
                  if(symbol_table.GetCurScope() != 0)
                  {
                    std::string errString = "function definition only allowed "
                      "in global scope. Current scope: ";
                    std::stringstream ss; ss << symbol_table.GetCurScope();
                    errString += ss.str();
                    yyerror(errString);
                    exit(1);
                  }

                  if($2 != NULL) $1->AddChild($2);
                  $$ = $1;

                  symbol_table.SetCurrentFunction(NULL);
     }
  ;

statement:   variable_declare ';'    {  $$ = $1;  }
        |    declare_assign ';' {  $$ = $1;  }
        |    code_block         {  $$ = $1;  }
        |    flow_control       {  $$ = $1;  }
        |    expression ';'     {  $$ = $1;  }
        |    command ';'        {  $$ = $1;  }
        |    ';'                {  $$ = NULL;  }
  ;


variable_declare:
       TYPE ID {
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

function_declare:
                function_define function_arguments code_block{
                  CFunctionEntry * function= symbol_table.GetCurrentFunction();
                  function->SetBody($3);
                  symbol_table.DecScope();

                  $$ = new ASTNodeFunction(function, $3);
                  $$->SetLineNum(line_num);
                }
               | function_define function_arguments command ';' {
                  CFunctionEntry * function= symbol_table.GetCurrentFunction();
                  function->SetBody($3);
                  symbol_table.DecScope();

                  $$ = new ASTNodeFunction(function, $3);
                  $$->SetLineNum(line_num);
}

function_define:    COMMAND_DEFINE TYPE ID {
                  std::string type_name = $2;
                  int type_id = 0;

                  if (type_name == "int") type_id = Type::INT;
                  else if (type_name == "char") type_id = Type::CHAR;
                  else if (type_name == "string") type_id = Type::CHAR_ARRAY;
                  else {
                    std::string errString = "unknown type '";
                    errString += $2;
                    errString += "'";
                    yyerror(errString);
                  }
                  std::string name = $3;
                  CFunctionEntry * function = symbol_table.AddFunction(name, type_id);
                  symbol_table.SetCurrentFunction(function);

                  $$ = new ASTNodeTempNode(Type::VOID);
                  symbol_table.SetMode(true);
                }
   |         COMMAND_DEFINE ARRAY '(' TYPE ')' ID {
                  std::string type_name = $4;
                  int type_id = 0;

                  if (type_name == "int") type_id = Type::INT_ARRAY;
                  else if (type_name == "char") type_id = Type::CHAR_ARRAY;
                  else if (type_name == "string") type_id = Type::CHAR_ARRAY;
                  else {
                    std::string errString = "unknown type '";
                    errString += $4;
                    errString += "'";
                    yyerror(errString);
                  }
                  std::string name = $6;
                  std::cout << "function_define" << std::endl;
                  CFunctionEntry * function = symbol_table.AddFunction(name, type_id);
                  symbol_table.SetCurrentFunction(function);

                  $$ = new ASTNodeTempNode(Type::VOID);
                  symbol_table.SetMode(true);
                }
function_arguments: args_begin function_argument_list args_end {
                      $$ = $2;
                  }
                 | args_begin args_end {
                     $$ = new ASTNodeTempNode(Type::VOID);
                     $$->SetLineNum(line_num);
                 }

args_begin: '(' { symbol_table.IncScope(); }

function_argument_list:
                  function_argument_list ',' variable_declare {
                    ASTNode * node = $1; // Grab the node for func arg list.
                    node->AddChild($3);  // Save this argument in the node.
                    $$ = node;
                  }

    |           variable_declare {
                    // Create a temporary AST node to hold the arg list.
                    $$ = new ASTNodeTempNode(Type::VOID);
                    $$->AddChild($1);  // Save this argument in the temp node.
                    $$->SetLineNum(line_num);
                  }

args_end:    ')' {
                  symbol_table.SetMode(false); //stop adding args
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

declare_assign:  variable_declare '=' expression {
                   $$ = new ASTNodeAssign($1, $3);
                   $$->SetLineNum(line_num);
           }
  ;

code_block:  block_start statement_list block_end { $$ = $2; }
block_start: '{' { symbol_table.IncScope(); }
block_end:   '}' { symbol_table.DecScope(); };

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


 /* BEGIN EXPRESSION */
expression:     literal    { $$ = $1; }
          |     negative   { $$ = $1; }
          |     not_       { $$ = $1; }
          |     variable   { $$ = $1; }
          |     operation  { $$ = $1; }
          |     compare    { $$ = $1; }
          |     assignment { $$ = $1; }
          |  function_call { $$ = $1; }

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
              $$ = new ASTNodeLiteral(Type::CHAR_ARRAY,
              literal.substr(1,literal.size()-2));
              $$->SetLineNum(line_num);
            }
negative:    '-' expression %prec UMINUS {
               $$ = new ASTNodeMath1($2, '-');
               $$->SetLineNum(line_num);
             }

not_:         '!' expression %prec UMINUS {
               $$ = new ASTNodeMath1($2, '!');
               $$->SetLineNum(line_num);
             }

 /* things in this section can be on the left hand side of assignment */
variable:
        ID {
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

    |   ID '[' expression ']' {
            CTableEntry * cur_entry = symbol_table.Lookup($1);
            if (cur_entry == NULL)
            {
              std::string errString = "unknown variable '";
              errString += $1;
              errString += "'";
              yyerror(errString);
              exit(1);
            }
            int type_id = cur_entry->GetType();

            if(type_id!=Type::INT_ARRAY && type_id !=Type::CHAR_ARRAY)
            {
              std::string type_str = Type::AsString($3->GetType());
              std::string errString = "array methods cannot be run on type  '";
              errString += type_str;
              errString += "'";
              yyerror(errString);
              exit(1);
            }

            $$ = new ASTNodeIndex(cur_entry,$3,debug);
            $$->SetLineNum(line_num);
        }
;

operation:
      expression '+' expression {
              $$ = new ASTNodeMath2($1, $3, '+');
              $$->SetLineNum(line_num);
      }
  |   expression '-' expression {
              $$ = new ASTNodeMath2($1, $3, '-');
              $$->SetLineNum(line_num);
      }
  |   expression '*' expression {
              $$ = new ASTNodeMath2($1, $3, '*');
              $$->SetLineNum(line_num);
      }
  |   expression '/' expression {
              $$ = new ASTNodeMath2($1, $3, '/');
              $$->SetLineNum(line_num);
      }
  |   expression '%' expression {
              $$ = new ASTNodeMath2($1, $3, '%');
              $$->SetLineNum(line_num);
      }
  |   '(' expression ')' { // ignore parenthesis used for order
              $$ = $2;
      }

  |   COMMAND_RANDOM '(' expression ')' {
              $$ = new ASTNodeRandom($3);
      }
  |   ID '.' SIZE '(' ')' {
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
          $$ = new ASTNodeResize(cur_entry, $5, debug);
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

function_call: ID '(' argument_list ')' {
                CFunctionEntry * cur_entry = symbol_table.FunctionLookup($1);
                if (cur_entry == NULL)
                {
                  std::string errString = "unknown function '";
                  errString += $1;
                  errString += "'";
                  yyerror(errString);
                  exit(1);
                }
                //std::cout << cur_entry->GetNumArgs() << " " << $3->GetNumChildren() << std::endl;
                if(cur_entry->GetNumArgs() != $3->GetNumChildren())
                {
                  std::string errString = "incorrect call to function '";
                  errString += $1;
                  errString += "'. incorrect number of arguments";
                  yyerror(errString);
                  exit(1);
                }

                for(int i = 0; i < $3->GetNumChildren(); i++)
                {
                  // Check if passed argument matches type of defined argument
                  if( $3->GetChild(i)->GetType() != cur_entry->GetArg(i)->GetType())
                  {
                    std::string errString = "argument types don't match. passed in type '";
                    errString += Type::AsString($3->GetChild(i)->GetType());
                    errString += "', expected type is '";
                    errString += Type::AsString(cur_entry->GetArg(i)->GetType());
                    errString += "'";
                    yyerror(errString);
                    exit(1);
                  }
                }
                $$ = new ASTNodeFunctionCall(cur_entry);
                $$->SetType(cur_entry->GetReturnType());
                $$->TransferChildren($3);
             }
           | ID '(' ')' {
                CFunctionEntry * cur_entry = symbol_table.FunctionLookup($1);
                if (cur_entry == NULL)
                {
                  std::string errString = "unknown function '";
                  errString += $1;
                  errString += "'";
                  yyerror(errString);
                  exit(1);
                } if(cur_entry->GetNumArgs() != 0)
                {
                  std::string errString = "incorrect call to function '";
                  errString += $1;
                  errString += "'. incorrect number of arguments";
                  yyerror(errString);
                  exit(1);
                }
                $$ = new ASTNodeFunctionCall(cur_entry);
                $$->SetType(cur_entry->GetReturnType());
             }

 /* END EXPRESSION */

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
        |  COMMAND_RETURN expression {
             CFunctionEntry * function = symbol_table.GetCurrentFunction();
             if(symbol_table.GetCurScope() == 0 || function == NULL)
             {
                std::string errString = "return statement used outside function definition";
                yyerror(errString);
                exit(1);
             }
             if($2->GetType() != function->GetReturnType())
             {
                std::string errString = "return type does not match function type";
                yyerror(errString);
                exit(1);
             }
             $$ = new ASTNodeReturn($2, symbol_table.GetCurrentFunction());
             $$->SetLineNum(line_num);
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
