# Setup some aliases to these can be easily altered in the future.
GCC = g++
CFLAGS = -g 
YACC = bison
LEX = flex

# Link the object files together into the final executable.

tube6: tube6-lexer.o tube6-parser.tab.o ast.o ic.o type_info.o
	$(GCC) tube6-parser.tab.o tube6-lexer.o ast.o ic.o type_info.o -o tube6 -ll -ly


# Use the lex and yacc templates to build the C++ code files.

tube6-lexer.o: tube6-lexer.cc tube6.lex symbol_table.h
	$(GCC) $(CFLAGS) -c tube6-lexer.cc

tube6-parser.tab.o: tube6-parser.tab.cc tube6.y symbol_table.h
	$(GCC) $(CFLAGS) -c tube6-parser.tab.cc


# Compile the individual code files into object files.

tube6-lexer.cc: tube6.lex tube6-parser.tab.cc symbol_table.h
	$(LEX) -otube6-lexer.cc tube6.lex

tube6-parser.tab.cc: tube6.y symbol_table.h
	$(YACC) -o tube6-parser.tab.cc -d tube6.y -v -t

ast.o: ast.cc ast.h symbol_table.h
	$(GCC) $(CFLAGS) -c ast.cc

ic.o: ic.cc ic.h symbol_table.h
	$(GCC) $(CFLAGS) -c ic.cc

type_info.o: type_info.h type_info.cc
	$(GCC) $(CFLAGS) -c type_info.cc


# Cleanup all auto-generated files

clean:
	rm -f tube6 *.o tube6-lexer.cc *.tab.cc *.tab.hh *~
