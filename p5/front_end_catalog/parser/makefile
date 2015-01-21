#
# Makefile for the parser
#

.SUFFIXES: .o .cpp .l

#
# Compiler and loader definitions
#

LD =		ld
LDFLAGS =	

CXX =		g++

CC =		g++

INC =		-I..
CXXFLAGS =	$(INC) -g -Wall $(DEBUG)

LEX =		flex
LFLAGS =        -I -t

YACC =		bison
YFLAGS =        -y -d

#
# list of all object and source files
#

OBJS =		scan.o parse.o nodes.o interp.o yywrap.o
SRCS =		scan.l parse.y nodes.cpp interp.cpp yywrap.c
LIBS =		

all:		../parser.o

../parser.o:	$(OBJS)
		ld -o $@ -r $(OBJS)

y.tab.h:	parse.o

parse.o:	parse.y
		-rm -f y.tab.c
		$(YACC) $(YFLAGS) $<
		$(CXX) $(INC) -c y.tab.c -o $@
		-rm -f y.tab.c

scan.o:		y.tab.h scan.l scanhelp.cpp
		-rm -f $*.cpp
		$(LEX) $(LFLAGS) scan.l > scan.cpp
		$(CXX) $(INC) -c $*.cpp
		-rm -f $*.cpp

.c.o:
		$(CC) $(CFLAGS) -c $<

.cpp.o:
		$(CXX) $(CXXFLAGS) -c $<

clean:
		rm -f core  *.bak *~ *.o

depend:
		makedepend $(INC) -I/s/gcc/include/g++ $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
