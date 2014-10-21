# Setup some aliases to these can be easily altered in the future.
GCC = g++
CFLAGS = -g
YACC = bison
LEX = flex

# Link the object files together into the final executable.

tube5: tube5-lexer.o tube5-parser.tab.o ast.o ic.o type_info.o
	$(GCC) tube5-parser.tab.o tube5-lexer.o ast.o ic.o type_info.o -o tube5 -ll -ly


# Use the lex and yacc templates to build the C++ code files.

tube5-lexer.o: tube5-lexer.cc tube5.lex symbol_table.h
	$(GCC) $(CFLAGS) -c tube5-lexer.cc

tube5-parser.tab.o: tube5-parser.tab.cc tube5.y symbol_table.h
	$(GCC) $(CFLAGS) -c tube5-parser.tab.cc


# Compile the individual code files into object files.

tube5-lexer.cc: tube5.lex tube5-parser.tab.cc symbol_table.h
	$(LEX) -otube5-lexer.cc tube5.lex

tube5-parser.tab.cc: tube5.y symbol_table.h
	$(YACC) -o tube5-parser.tab.cc -d tube5.y

ast.o: ast.cc ast.h symbol_table.h
	$(GCC) $(CFLAGS) -c ast.cc

ic.o: ic.cc ic.h symbol_table.h
	$(GCC) $(CFLAGS) -c ic.cc

type_info.o: type_info.h type_info.cc
	$(GCC) $(CFLAGS) -c type_info.cc


# Cleanup all auto-generated files

clean:
	rm -f tube5 *.o tube5-lexer.cc *.tab.cc *.tab.hh *~
