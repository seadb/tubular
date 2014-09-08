# Setup some aliases to these can be easily altered in the future.
GCC = g++
CFLAGS = 
LFLAGS = -ll

# Link the object files together into the final executable.

test1: test1-lexer.o	
	$(GCC) test1-lexer.o -o test1 $(LFLAGS)


# Use the flex configuration to build the C++ code files.

test1-lexer.o: test1-lexer.cc
	$(GCC) $(CFLAGS) -c test1-lexer.cc


# Compile the individual code files into object files.

test1-lexer.cc: test1.lex
	flex -o test1-lexer.cc test1.lex

# Cleanup all of the auto-generated files

clean:
	rm -f test1 *.o test1-lexer.cc *.tab.* *~

