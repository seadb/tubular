# Setup some aliases to these can be easily altered in the future.
GCC = g++
CFLAGS = -g
YACC = bison
LEX = flex

# Link the object files together into the final executable.

tube8: tube8-lexer.o tube8-parser.tab.o ast.o ic.o type_info.o
	$(GCC) tube8-parser.tab.o tube8-lexer.o ast.o ic.o type_info.o -o tube8 -ll -ly


# Use the lex and yacc templates to build the C++ code files.

tube8-lexer.o: tube8-lexer.cc tube8.lex symbol_table.h
	$(GCC) $(CFLAGS) -c tube8-lexer.cc

tube8-parser.tab.o: tube8-parser.tab.cc tube8.y symbol_table.h
	$(GCC) $(CFLAGS) -c tube8-parser.tab.cc


# Compile the individual code files into object files.

tube8-lexer.cc: tube8.lex tube8-parser.tab.cc symbol_table.h
	$(LEX) -otube8-lexer.cc tube8.lex

tube8-parser.tab.cc: tube8.y symbol_table.h
	$(YACC) -o tube8-parser.tab.cc -d tube8.y -v -t

ast.o: ast.cc ast.h symbol_table.h
	$(GCC) $(CFLAGS) -c ast.cc

ic.o: ic.cc ic.h symbol_table.h
	$(GCC) $(CFLAGS) -c ic.cc

type_info.o: type_info.h type_info.cc
	$(GCC) $(CFLAGS) -c type_info.cc


# Cleanup all auto-generated files

clean:
	rm -f tube8 *.o tube8-lexer.cc *.tab.cc *.tab.hh *~
