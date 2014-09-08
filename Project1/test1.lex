%{
#include <iostream>
#include <cstring>
using namespace std;

int line = 0;
%}
%%
"/*"[.\n]*"*/" {
    // Comment, ignore everything in between
    // However, still increment line
    for(int i = 0; i < yyleng; i++) {
        if(yytext[i] == '\n') {
            line++;
        }
    } 
}
#.*                  ; // Comment, ignore remainder of line
int                  { cout << "TYPE: " << yytext << endl; }
print                { cout << "COMMAND_PRINT: " << yytext << endl; }
random               { cout << "COMMAND_RANDOM: " << yytext << endl; }
[a-zA-Z_][a-zA-Z]*   { cout << "ID: " << yytext << endl; }
[0-9]+               { cout << "INT_LITERAL: " << yytext << endl; }
"=="                 { cout << "COMP_EQU: " << yytext << endl; }
"!="                 { cout << "COMP_NEQU: " << yytext << endl; }
"<"                  { cout << "COMP_LESS: " << yytext << endl; }
"<="                 { cout << "COMP_LTE: " << yytext << endl; }
">"                  { cout << "COMP_GTR: " << yytext << endl; }
">="                 { cout << "COMP_GTE: " << yytext << endl; }
"&&"                 { cout << "BOOL_AND: " << yytext << endl; }
"||"                 { cout << "BOOL_OR: " << yytext << endl; }
"+="                 { cout << "ASSIGN_ADD: " << yytext << endl; }
"-="                 { cout << "ASSIGN_SUB: " << yytext << endl; }
"*="                 { cout << "ASSIGN_MULT: " << yytext << endl; }
"/="                 { cout << "ASSIGN_DIV: " << yytext << endl; }
"%="                 { cout << "ASSIGN_MOD: " << yytext << endl; }
[+\-*/%()=,{}\[\].;] { cout << "ASCII_CHAR: " << yytext << endl; }
[ \t]*               ; // Ignore whitespace
\n                   { line++; }
. {
    cout << "Unknown token on line " << line << ": " << yytext << endl;
}
%%
int main() {
    yylex();
    cout << "Line Count: " << line << endl;
    return 0;
}
