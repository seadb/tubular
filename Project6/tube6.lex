%{
<<<<<<< HEAD
//C declarations

#include <iostream>

%}


%%

val_copy { return VAL_COPY; }
add { return ADD; }
sub { return SUB; }
mult { return MULT; }
div { return DIV; }
mod { return MOD; }
test_less { return TEST_LESS; }
test_gtr { return TEST_GTR; }
test_equ { return TEST_EQU; }
test_nequ { return TEST_NEQU; }
test_gte { return TEST_GTE; }
test_lte { return TEST_LTE; }
jump { return JUMP; }
jump_if_0 { return JUMP_IF_0; }
jump_if_n0 { return JUMP_IF_N0; }
jmp_if_0 { return JUMP_IF_0; }
jmp_if_n0 { return JUMP_IF_N0; }
nop { return NOP; }
random { return RANDOM; }
out_int { return OUT_INT; }
out_float { return OUT_FLOAT; }
out_char { return OUT_CHAR; }

load { return LOAD; }
store { return STORE; }
mem_copy { return MEM_COPY; }

debug_status { return DEBUG_STATUS; }

"ar_get_idx"

"ar_set_idx"
"ar_get_siz"|"ar_get_size"
"ar_set_siz"|"ar_set_size"
"ar_copy"
"push"
"pop"
((a|s){int})
{ std::cerr << "Error(line " << line_num << "): instruction '" << yytext << "' valid only in TubeIC, not TubeCode assembly." << std::endl; exit(1); }


-?{int} { yylval.int_val = atoi(yytext); return ARG_INT; }
reg[A-H] { yylval.int_val = yytext[3]-'A'; return ARG_REG; }
reg[I-Z] { std::cerr << "Error(line " << line_num << "): " << yytext << "not a legal register; only 8 registers available." << std::endl; exit(1); }
IP { return ARG_IP; }
'.' { yylval.int_val = (int) yytext[1]; return ARG_CHAR; }
'\\n' { yylval.int_val = (int) '\n'; return ARG_CHAR; }
'\\t' { yylval.int_val = (int) '\t'; return ARG_CHAR; }
'\\'' { yylval.int_val = (int) '\''; return ARG_CHAR; }
'\\\\' { yylval.int_val = (int) '\\'; return ARG_CHAR; }
'\\\"' { yylval.int_val = (int) '\"'; return ARG_CHAR; }
[a-zA-Z][a-zA-Z0-9_]* { yylval.lexeme = strdup(yytext); return ARG_LABEL; }

[:] { return yytext[0]; }

{eol}  { line_num++; return ENDLINE; }
{comment} { ; }

=======
#include "symbol_table.h"
#include "type_info.h"
#include "ast.h"
#include "tube6-parser.tab.hh"

#include <iostream>
#include <stdio.h>
#include <string>

// Two global variables (not clean, but works...)
int line_num = 1;
std::string out_filename = "";
int MAX_STR_CONST=1000;
char string_buf[1000];
char *string_buf_ptr;
%}

%option nounput

%x str

type		int|char|string
id	        [_a-zA-Z][a-zA-Z0-9_]*
int_lit         [0-9]+
char_lit        '(.|(\\[\\'nt]))'
string_lit      \"(.|\n\t|(\\[\\nt]))*\"
comment		#.*
whitespace	[ \t\r]
passthrough	[+\-*/%=(),!{}[\].;]

  //unterminated_string    \"(.|\n\t|(\\[\\"nt]))*
  //{unterminated_string} {yylval.lexeme = strdup(yytext);  return UNTERM_STRING; }
%%

"print" { return COMMAND_PRINT; }
"if"    { return COMMAND_IF; }
"else"  { return COMMAND_ELSE; }
"while" { return COMMAND_WHILE; }
"break" { return COMMAND_BREAK; }
"for"   { return COMMAND_FOR; }
"array" { return ARRAY;}
"random" { return COMMAND_RANDOM;}
"size"  { return SIZE; }
"resize"  { return RESIZE; }
{type}        { yylval.lexeme = strdup(yytext);  return TYPE; }
{id}          { yylval.lexeme = strdup(yytext);  return ID; }
{int_lit}     { yylval.lexeme = strdup(yytext);  return INT_LIT; }
{char_lit}    { yylval.lexeme = strdup(yytext);  return CHAR_LIT; }
{string_lit}  { yylval.lexeme = strdup(yytext);  return STRING_LIT; }
{passthrough}  { yylval.lexeme = strdup(yytext);  return (int) yytext[0]; }

"+=" { return ASSIGN_ADD; }
"-=" { return ASSIGN_SUB; }
"*=" { return ASSIGN_MULT; }
"/=" { return ASSIGN_DIV; }
"%=" { return ASSIGN_MOD; }

"==" { return COMP_EQU; }
"!=" { return COMP_NEQU; }
"<" { return COMP_LESS; }
"<=" { return COMP_LTE; }
">" { return COMP_GTR; }
">=" { return COMP_GTE; }

"&&" { return BOOL_AND; }
"||" { return BOOL_OR; }

{comment} { ; }
{whitespace} { ; }
\n  { line_num++; }
     
     
     
\"      string_buf_ptr = string_buf; BEGIN(str);
     
<str>\"        { /* saw closing quote - all done */
                BEGIN(INITIAL);
                 *string_buf_ptr = '\0';
                 /* return string constant token type and
                  * value to parser
                  */
                 }
     
<str>\n|;   {
              std::cout << "ERROR(line " << line_num << 
              "): Unterminated string." << std::endl; exit(1);        
               /* error - unterminated string constant */
               /* generate error message */
            }
     
<str>\\[0-7]{1,3} {
                 /* octal escape sequence */
                 int result;
     
                 (void) sscanf( yytext + 1, "%o", &result );
     
                 if ( result > 0xff )
                         /* error, constant is out-of-bounds */
     
                 *string_buf_ptr++ = result;
                 }
     
<str>\\[0-9]+ {
                 /* generate error - bad escape sequence; something
                  * like '\48' or '\0777777'
                  */
              }
     
<str>\\n  *string_buf_ptr++ = '\n';
<str>\\t  *string_buf_ptr++ = '\t';
<str>\\r  *string_buf_ptr++ = '\r';
<str>\\b  *string_buf_ptr++ = '\b';
<str>\\f  *string_buf_ptr++ = '\f';
    
<str>\\(.|\n)  *string_buf_ptr++ = yytext[1];
     
<str>[^\\\n\"]+  {
                   char *yptr = yytext;
                   while ( *yptr )
                     *string_buf_ptr++ = *yptr++;
                 }
 
.      { 
        std::cout << "ERROR(line " << line_num << "): Unknown Token '" 
        << yytext << "'." << std::endl; exit(1); 
       }
>>>>>>> aba9d6ad4e389ede45cdba7e89669cb435b3791b

%%

void LexMain(int argc, char * argv[])
{
  FILE * file = NULL;
  bool input_found = false;

  // Loop through all of the command-line arguments.
  for (int arg_id = 1; arg_id < argc; arg_id++) {
    std::string cur_arg(argv[arg_id]);

    if (cur_arg == "-h") {
<<<<<<< HEAD
      std::cout << "Tubular Assembler v. 0.6 (Project 6)"  << std::endl
=======
      std::cout << "Tubular Compiler v. 0.4 (Project 4)"  << std::endl
>>>>>>> aba9d6ad4e389ede45cdba7e89669cb435b3791b
           << "Format: " << argv[0] << "[flags] [filemName]" << std::endl
           << std::endl
           << "Available Flags:" << std::endl
           << "  -h  :  Help (this information)" << std::endl
        ;
      exit(0);
    }

    // PROCESS OTHER ARGUMENTS HERE IF YOU ADD THEM

    // If the next argument begins with a dash, assume it's an unknown flag...
    if (cur_arg[0] == '-') {
      std::cerr << "ERROR: Unknown command-line flag: " << cur_arg << std::endl;
      exit(1);
    }

    // Assume the current argument is a filemName (first input, then output)
    if (!input_found) {
      file = fopen(argv[arg_id], "r");
      if (!file) {
        std::cerr << "Error opening " << cur_arg << std::endl;
        exit(1);
      }
      yyin = file;
      input_found = true;
      continue;
    } else if (out_filename == "") {
      out_filename = cur_arg;
    }

    // Both files already loaded!
    else {
      std::cout << "ERROR: Unknown argument '" << cur_arg << "'" << std::endl;
      exit(1);
    }
  }

  // Make sure we've loaded input and output filemNames before we finish...
  if (input_found == false || out_filename == "") {
<<<<<<< HEAD
    std::cerr << "Format: " << argv[0] << "[flags] [input filename] [output filename]" << std::endl;
=======
    std::cerr << "Format: " << argv[0] << "[flags] [input filemName] [output filemName]" << std::endl;
>>>>>>> aba9d6ad4e389ede45cdba7e89669cb435b3791b
    std::cerr << "Type '" << argv[0] << " -h' for help." << std::endl;
    exit(1);
  }
 
  return;
}

