%{
#include <iostream>
#include <fstream>
  int char_count = 0, word_count = 0, line_count = 0;
%}
%option c++ noyywrap

word [^ \t\n]+
eol  \n
%%
{word}         { word_count++; char_count += yyleng; }
{eol}          { char_count++; line_count++; }
.              char_count++;
%%
int main (int argc, char* argv[]) {
    if (argc != 2) {  // Make sure the arguments are correct
      std::cerr << "Format: " << argv[0] << " [source filename]" << std::endl;
      exit(1);
    }

    std::ifstream sfile(argv[1]);// Open the file
    if (sfile.fail()) {// If the open failed, give an error and exit.
      std::cerr << "Error: Unable to open '" << argv[1] << "'.  Halting." << std::endl;
      exit(2);
    }

    FlexLexer* lexer = new yyFlexLexer(&sfile);
    while (lexer->yylex());

    std::cout << "Line count:\t" << line_count << std::endl;
    std::cout <<  "Word count:\t" << word_count << std::endl;
    std::cout <<  "Char Count:\t" << char_count << std::endl;
    return 0;
}
