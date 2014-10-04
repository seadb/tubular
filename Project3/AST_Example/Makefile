.cc.o:
	g++ -c $< -o $*.o

default: all

all: tube3

tube3.tab.cc: tube3.y
	bison -o tube3.tab.cc -b tube3 -d tube3.y

tube3.yy.cc: tube3.lex
	flex -o tube3.yy.cc tube3.lex

tube3: tube3.tab.o tube3.yy.o
	g++ -o tube3 tube3.tab.o tube3.yy.o -ll -ly

clean:
	rm -f tube3.tab.cc tube3.tab.hh tube3.yy.cc *~ *.o
