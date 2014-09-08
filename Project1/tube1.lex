%{
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
using namespace std;
int temp=0, line_count=0;
void printlex(string token, string lexeme){ cout << token << ": " << lexeme << endl;}
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
comment2    "/*".*"*/"
unknown     .
%%
 /*COMMANDS*/
{eol}           { line_count++;  printlex("EOL", yytext); }
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
{white}		    {for(int i=0; i<yyleng;i++){
                    if(&yytext[i]=="\n")
                      {line_count++; 
                       std::cout << "White newline" << std::endl; 
                      }
                    }
                 }
{comment1}       { line_count++; std::cout << "Comment newline" << std::endl; }
{comment2}       { line_count++; }

                 
                  
                 
                      
{unknown}	    { line_count++; printf("Unknown token on line %i: %s \n", line_count, yytext);}
%%
int main(int argc, char* argv[]) {
	if (argc != 2) { //Make sure the arguments are correct
		cerr << "Format:" << argv[0] << " [source filename]" << endl;
		exit(1);
	}

	ifstream sfile(argv[1]);	 	 	 // Open the file

	if (sfile.fail()) {	 	 	 	 	 	 // If the open failed, give an error and exit.
	cerr << "Error: Unable to open '" << argv[1] << "'. Halting." << std::endl;
	exit(2);
	}

	FlexLexer* lexer = new yyFlexLexer(&sfile);
	while (lexer->yylex());

    stringstream ss;
    ss << line_count;

    printlex("Line Count",ss.str());
	return 0;

 }
