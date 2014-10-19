.cc.o:
	g++ -c $< -o $*.o

default: all

all: tube5

tube5.tab.cc: tube5.y
	bison -o tube5.tab.cc -b tube5 -d tube5.y -t -v

tube5.yy.cc: tube5.lex
	flex -o tube5.yy.cc tube5.lex

tube5: tube5.tab.o tube5.yy.o
	g++ -o tube5 tube5.tab.o tube5.yy.o -ll -ly

clean:
	rm -f tube5.tab.cc tube5.tab.hh tube5 tube5.yy.cc *~ *.o
