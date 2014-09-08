%{
#include <iostream>
#include <fstream>
int line_count = 1, err = 0;
%}
%x IN_COMMENT
%option c++ noyywrap
%%
int {
  std::cout << "TYPE: " << yytext << std::endl;  }
print {
  std::cout << "COMMAND_PRINT: " << yytext << std::endl;  }
random {
  std::cout << "COMMAND_RANDOM: " << yytext << std::endl;  }
[a-zA-Z_][a-zA-Z0-9_]* {
  std::cout << "ID: " << yytext << std::endl;  }
[0-9]+ {
  std::cout << "INT_LITERAL: " << yytext << std::endl;  }
[+\-*/()=,}{.;%\[\]] {
  std::cout << "ASCII_CHAR: " << yytext << std::endl;  }
"+=" {
  std::cout << "ASSIGN_ADD: " << yytext << std::endl;  }
"-=" {
  std::cout << "ASSIGN_SUB: " << yytext << std::endl;  }
"*=" {
  std::cout << "ASSIGN_MULT: " << yytext << std::endl;  }
"/=" {
  std::cout << "ASSIGN_DIV: " << yytext << std::endl;  }
"%=" {
  std::cout << "ASSIGN_MOD: " << yytext << std::endl;  }
"==" {
  std::cout << "COMP_EQU: " << yytext << std::endl;  }
"!=" {
  std::cout << "COMP_NEQU: " << yytext << std::endl;  }
"<=" {
  std::cout << "COMP_LTE: " << yytext << std::endl;  }
">=" {
  std::cout << "COMP_GTE: " << yytext << std::endl;  }
"<" {
  std::cout << "COMP_LESS: " << yytext << std::endl;  }
">" {
  std::cout << "COMP_GTR: " << yytext << std::endl;  }
"&&" {
  std::cout << "BOOL_AND: " << yytext << std::endl;  }
"||" {
  std::cout << "BOOL_OR: " << yytext << std::endl;  }

[\t ]*  /* Ignores tabs and spaces */;
\n {  line_count++;  }
#.*	/* Ignores C-Style comments */;
<INITIAL>{
     "/*"              BEGIN(IN_COMMENT);
}
<IN_COMMENT>{
     "*/"      BEGIN(INITIAL);
     [^*\n]+   // eat comment in chunks
     "*"       // eat the lone star
     \n        line_count++;
}
. {  
  std::cout << "Unknown token on line " << line_count << ": " << yytext << std::endl;
  err = 1;
  return 0;
}
%%
int main (int argc, char* argv[]) {
  if (argc != 2) 
  { // Make sure the arguments are correct
    std::cerr << "Format: " << argv[0] << " [source filename]" << std::endl;
    exit(1);
  }

  std::ifstream sfile(argv[1]); // Open the file

  if (sfile.fail()) 
  { // If the open failed, give an error and exit.
    std::cerr << "Error: Unable to open '" << argv[1] << "'. Halting." << std::endl;
    exit(2);
  }

  FlexLexer* lexer = new yyFlexLexer(&sfile);

  while (lexer->yylex());

  if (err != 1)
    std::cout << "Line Count: " << line_count-1 << std::endl;
  return 0;
}
