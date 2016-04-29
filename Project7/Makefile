# Setup some aliases to these can be easily altered in the future.
GCC = g++
CFLAGS = -g
YACC = bison
LEX = flex

# Link the object files together into the final executable.

tube7: tube7-lexer.o tube7-parser.tab.o ast.o ic.o type_info.o
	$(GCC) tube7-parser.tab.o tube7-lexer.o ast.o ic.o type_info.o -o tube7 -ll -ly


# Use the lex and yacc templates to build the C++ code files.

tube7-lexer.o: tube7-lexer.cc tube7.lex symbol_table.h
	$(GCC) $(CFLAGS) -c tube7-lexer.cc

tube7-parser.tab.o: tube7-parser.tab.cc tube7.y symbol_table.h
	$(GCC) $(CFLAGS) -c tube7-parser.tab.cc


# Compile the individual code files into object files.

tube7-lexer.cc: tube7.lex tube7-parser.tab.cc symbol_table.h
	$(LEX) -otube7-lexer.cc tube7.lex

tube7-parser.tab.cc: tube7.y symbol_table.h
	$(YACC) -o tube7-parser.tab.cc -d tube7.y -v -t

ast.o: ast.cc ast.h symbol_table.h
	$(GCC) $(CFLAGS) -c ast.cc

ic.o: ic.cc ic.h symbol_table.h
	$(GCC) $(CFLAGS) -c ic.cc

type_info.o: type_info.h type_info.cc
	$(GCC) $(CFLAGS) -c type_info.cc


# Cleanup all auto-generated files

clean:
	rm -f tube7 *.o tube7-lexer.cc *.tab.cc *.tab.hh *~
