#This is an example Makefile.
# It will convert a file named proj1.lex to an executable named proj1.

# Setup some aliases to these can be easily altered in the future.
GCC = g++
CFLAGS = 
LFLAGS = -ll

# Link the object files together into the final executable.

proj1: proj1-lexer.o    
    $(GCC) proj1-lexer.o -o proj1 $(LFLAGS)


# Use the flex configuration to build the C++ code files.

proj1-lexer.o: proj1-lexer.cc
    $(GCC) $(CFLAGS) -c proj1-lexer.cc


# Compile the individual code files into object files.

proj1-lexer.cc: proj1.lex
    flex -o proj1-lexer.cc proj1.lex

# Cleanup all of the auto-generated files

clean:
    rm -f proj1 *.o proj1-lexer.cc *.tab.* *~
