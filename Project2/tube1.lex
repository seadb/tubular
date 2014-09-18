%{
#include <iostream>
#include <fstream>
<<<<<<< HEAD
#include <string>
#include <sstream>
int temp=0, line_count=0;
void printlex(std::string token, std::string lexeme)
 { std::cout << token << ": " << lexeme << std::endl;}
%}
%option c++ noyywrap
eol         "\n"
print       "print"
type		    "int"
random	    "random"
id			[a-zA-Z_]+[a-zA-Z0-9_]*
int_lit	    [0-9]+
assignadd	"+="
assignsub   "-="
assignmult  "*="
assigndiv   "/="
assignmod   "%="
compequ    "=="
compnequ    "!="
complte     "<="
compless    "<"
compgte     ">="
compgtr     ">"
booland     "&&"
boolor      "||"
ascii		[\+\-\*/=;\(\)%,{}[\]\.]
white		[ \t\n]+
comment1 	"#".*\n
%s IN_COMMENT
comment2    \\/\\*[^(\\/\\*)]*?\\*\\/  
unknown     .

%%
 /*COMMANDS*/
{eol}           { line_count++; /* printlex("EOL", yytext);*/ }
{print}		    { printlex("COMMAND_PRINT", yytext);}
{random} 	    { printlex("COMMAND_RANDOM", yytext);}
 /*TYPES*/
{type}			{ printlex("TYPE", yytext);}
{id}  		    { printlex("ID", yytext);}
{int_lit} 	    { printlex("INT_LITERAL", yytext);}
 /*ASSIGN*/
{assignadd}		{ printlex("ASSIGN_ADD", yytext);}
{assignsub}		{ printlex("ASSIGN_SUB", yytext);}
{assignmult}	{ printlex("ASSIGN_MULT", yytext);}
{assigndiv}		{ printlex("ASSIGN_DIV", yytext);}
{assignmod}		{ printlex("ASSIGN_MOD", yytext);}
 /*COMP*/
{compnequ}      { printlex("COMP_NEQU", yytext);}
{compequ}       { printlex("COMP_EQU", yytext);}
{complte}       { printlex("COMP_LTE", yytext);}
{compless}      { printlex("COMP_LESS", yytext);}
{compgte}       { printlex("COMP_GTE", yytext);}
{compgtr}       { printlex("COMP_GTR", yytext);}
 /*BOOL*/
{booland}       { printlex("BOOL_AND", yytext);}
{boolor}        { printlex("BOOL_OR", yytext);}
 /*SPECIAL CHARACTERS*/
{ascii}		    { printlex("ASCII_CHAR", yytext);}
{white}		    { for(int i=0; i < yyleng;i++){
                    if(yytext[i] == '\n') { line_count++; /*printlex("newline", "in consecutive ws");*/ }
                   }
                }
{comment1}       { line_count++;} 

<INITIAL>{
     "/*"          BEGIN(IN_COMMENT);     
}
<IN_COMMENT>{
     "*/"      BEGIN(INITIAL);
     [^*\n]+   ; // eat comment in chunks
     "*"       ;// eat the lone star
     \n        line_count++;
     }

{unknown}	    { line_count++; printf("Unknown token on line %i: %s\n", line_count, yytext);
                  exit(3);
                }
%%
int main(int argc, char* argv[]) {
	if (argc != 2) { //Make sure the arguments are correct
		std::cerr << "Format:" << argv[0] << " [source filename]" << std::endl;
		exit(1);
	}

	std::ifstream sfile(argv[1]);	 	 	 // Open the file

	if (sfile.fail()) {	 	 	 	 	 	 // If the open failed, give an error and exit.
	std::cerr << "Error: Unable to open '" << argv[1] << "'. Halting." << std::endl;
	exit(2);
	}

	FlexLexer* lexer = new yyFlexLexer(&sfile);
	while (lexer->yylex());

    std::stringstream ss;
    ss << line_count;

    printlex("Line Count",ss.str());
	return 0;

 }
=======
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
>>>>>>> group/master
