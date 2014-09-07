%{
#include <iostream>
#include <fstream>
int line_count = 0;
%}
%option c++ noyywrap
eol \n
%%
int { std::cout << "TYPE: " << yytext << std::endl; }
print { std::cout << "COMMAND_PRINT: " << yytext << std::endl; }
random { std::cout << "COMMAND_RANDOM: " << yytext  << std::endl; }
[a-zA-z_][a-zA-Z0-9_]* { std::cout << "ID: " << yytext << std::endl; }
[0-9]+ { std::cout << "INT_LITERAL: " << yytext << std::endl; }
[+\-*/%\(\)=,\{\}\[\].;] { std::cout << "ASCII_CHAR: " << yytext << std::endl; }
"+=" { std::cout << "ASSIGN_ADD: " << yytext << std::endl; }
"-=" { std::cout << "ASSIGN_SUB: " << yytext << std::endl; }
"*=" { std::cout << "ASSIGN_MULT: " << yytext << std::endl; }
"/=" { std::cout << "ASSIGN_DIV: " << yytext << std::endl; }
"%=" { std::cout << "ASSIGN_MOD: " << yytext << std::endl; }
"==" { std::cout << "COMP_EQU: " << yytext << std::endl; }
"!=" { std::cout << "COMP_NEQU: " << yytext << std::endl; }
"<" { std::cout << "COMP_LESS: " << yytext << std::endl; }
"<=" { std::cout << "COMP_LTE: " << yytext << std::endl; }
">" { std::cout << "COMP_GTR: " << yytext << std::endl; }
">=" { std::cout << "COMP_GTE: " << yytext << std::endl; }
"&&" { std::cout << "BOOL_AND: " << yytext << std::endl; }
"||" { std::cout << "BOOL_OR: " << yytext << std::endl; } 
[ \t]+ { ; }
{eol} { line_count++; }
#[^\n]* { ; }
. {	line_count++; 
	std::cout << "Unknown token on line " << line_count << ": " << yytext << std::endl;
	exit(1); }
%%
int main (int argc, char* argv[]) {
	if (argc != 2) { // Make sure the arguments are correct
		std::cout << "Format: " << argv[0] << " [source filename]" << std::endl;
		exit(1);
	}
	
	std::ifstream sfile(argv[1]);
	if (sfile.fail()) {
		std::cout << "Error: Unable to open '" << argv[1] << "'. Halting." << std::endl;
		exit(2);
	}
	FlexLexer* lexer = new yyFlexLexer(&sfile);
	while (lexer->yylex());
	std::cout << "Line Count: " << line_count << std::endl;
	return 0;
}
