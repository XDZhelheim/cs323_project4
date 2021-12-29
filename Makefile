ifeq ($(shell uname), Darwin) # macOS
	CXXFLAGS=-ll -ly --std=c++11 -mmacosx-version-min=12.1
else
	CXXFLAGS=-lfl -ly --std=c++11
endif

CC=gcc
CXX=g++
FLEX=flex
BISON=bison

.lex: lex.l
	$(FLEX) lex.l
.syntax: syntax.y
	$(BISON) -v -t -d syntax.y
splc: .lex .syntax
	mkdir -p bin && $(CXX) syntax.tab.c tac.cpp mips32.cpp -o bin/splc $(CXXFLAGS)  
trash:
	@rm -rf lex.yy.c syntax.tab.* syntax.output
clean:
	@rm -rf bin sample/*.ir sample/*.s sample/*.out test/*.ir test/*.s test/*.out
	@rm -rf lex.yy.c syntax.tab.* syntax.output
all: splc