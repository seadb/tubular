%{
#include <iostream>
using namespace std;

int line = 0;
%}
%%
int                { cout << "TYPE: " << yytext << endl; }
print              { cout << "COMMAND_PRINT: " << yytext << endl; }
random             { cout << "COMMAND_RANDOM: " << yytext << endl; }
[a-zA-Z_][a-zA-Z]* { cout << "ID: " << yytext << endl; }
\n                 { line++; }
.                  { cout << "Unknown token on line " << line << ": " << yytext << endl; }
%%
int main()
{
    yylex();
    cout << line << endl;
    return 0;
}
