<<<<<<< HEAD
GCC = g++
LFLAGS = -ll

tube1: tube1-lexer.o
	$(GCC) tube1-lexer.o -o tube1 $(LFLAGS)

tube1-lexer.o: tube1-lexer.cc
	$(GCC) $(CFLAGS) -c tube1-lexer.cc

tube1-lexer.cc: tube1.lex
	flex -o tube1-lexer.cc tube1.lex
=======
# Setup some aliases to these can be easily altered in the future.
GCC = g++
CFLAGS = 
LFLAGS = -ll

# Link the object files together into the final executable.

tube1: tube1-lexer.o	
	$(GCC) tube1-lexer.o -o tube1 $(LFLAGS)


# Use the flex configuration to build the C++ code files.

tube1-lexer.o: tube1-lexer.cc
	$(GCC) $(CFLAGS) -c tube1-lexer.cc


# Compile the individual code files into object files.

tube1-lexer.cc: tube1.lex
	flex -+ -o tube1-lexer.cc tube1.lex

# Cleanup all of the auto-generated files

clean:
	rm -f tube1 *.o tube1-lexer.cc *.tab.* *~

>>>>>>> 18df0ac4bc62e8160874e4a238bce534385fea55
