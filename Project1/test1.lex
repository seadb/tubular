%{
#include <iostream>
using namespace std;

int line = 0;
%}
%%
int                  { cout << "TYPE: " << yytext << endl; }
print                { cout << "COMMAND_PRINT: " << yytext << endl; }
random               { cout << "COMMAND_RANDOM: " << yytext << endl; }
[a-zA-Z_][a-zA-Z]*   { cout << "ID: " << yytext << endl; }
[0-9]+               { cout << "INT_LITERAL: " << yytext << endl; }
"+="                 { cout << "ASSIGN_ADD: " << yytext << endl; }
"-="                 { cout << "ASSIGN_SUB: " << yytext << endl; }
"*="                 { cout << "ASSIGN_MULT: " << yytext << endl; }
"/="                 { cout << "ASSIGN_DIV: " << yytext << endl; }
"%="                 { cout << "ASSIGN_MOD: " << yytext << endl; }
[+\-*/%()=,{}\[\].;] { cout << "ASCII_CHAR: " << yytext << endl; }
\n                   { line++; }
. {
    cout << "Unknown token on line " << line << ": " << yytext << endl;
}
%%
int main()
{
    yylex();
    cout << line << endl;
    return 0;
}
