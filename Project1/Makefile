GCC = g++
LFLAGS = -ll

tube1: tube1-lexer.o
	$(GCC) tube1-lexer.o -o tube1 $(LFLAGS)

tube1-lexer.o: tube1-lexer.cc
	$(GCC) $(CFLAGS) -c tube1-lexer.cc

tube1-lexer.cc: tube1.lex
	flex -o tube1-lexer.cc tube1.lex
