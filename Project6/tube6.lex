%{
#include "symbol_table.h"
#include "type_info.h"
#include "ast.h"
#include "tube6-parser.tab.hh"

#include <iostream>
#include <stdio.h>
#include <string>

// Two global variables (not clean, but works...)
int line_num = 1;
enum eMode { IC=1, AC=2 };
int mode = 0;
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

%%

void LexMain(int argc, char * argv[])
{
  FILE * file = NULL;
  bool input_found = false;

  // Loop through all of the command-line arguments.
  for (int arg_id = 1; arg_id < argc; arg_id++) {
    std::string cur_arg(argv[arg_id]);

    if (cur_arg == "-h") {
      std::cout << "Tubular Compiler v. 0.4 (Project 4)"  << std::endl
           << "Format: " << argv[0] << "[flags] [filemName]" << std::endl
           << std::endl
           << "Available Flags:" << std::endl
           << "  -h  :  Help (this information)" << std::endl
        ;
      exit(0);
    }

    // PROCESS OTHER ARGUMENTS HERE IF YOU ADD THEM
    else if (cur_arg == "-i")
    {
      mode = 1;
    }

    else if (cur_arg == "-a")
    {
      mode = 2;
    }
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
    std::cerr << "Format: " << argv[0] << "[flags] [input filename] [output filename]" << std::endl;
    std::cerr << "Type '" << argv[0] << " -h' for help." << std::endl;
    exit(1);
  }

  return;
}

