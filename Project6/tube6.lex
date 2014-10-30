%{
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


%%

void LexMain(int argc, char * argv[])
{
  FILE * file = NULL;
  bool input_found = false;

  // Loop through all of the command-line arguments.
  for (int arg_id = 1; arg_id < argc; arg_id++) {
    std::string cur_arg(argv[arg_id]);

    if (cur_arg == "-h") {
      std::cout << "Tubular Assembler v. 0.6 (Project 6)"  << std::endl
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
    std::cerr << "Format: " << argv[0] << "[flags] [input filename] [output filename]" << std::endl;
    std::cerr << "Type '" << argv[0] << " -h' for help." << std::endl;
    exit(1);
  }
 
  return;
}

