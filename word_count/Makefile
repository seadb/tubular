#This is an example Makefile.
# It will convert a file named count.lex to an executable named count.

# Setup some aliases to these can be easily altered in the future.
GCC = g++
CFLAGS = 
LFLAGS = -ll

# Link the object files together into the final executable.
count: count.o    
	$(GCC) count.o -o count $(LFLAGS)

# Use the flex configuration to build the C++ code files.
count.o: count.cc
	$(GCC) $(CFLAGS) -c count.cc

# Compile the individual code files into object files.
count.cc: count.lex
	flex -o count.cc count.lex

# Cleanup all of the auto-generated files
clean:
	rm -f count *.o count.cc *.tab.* *~
