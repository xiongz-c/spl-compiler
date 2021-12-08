CC=gcc
CPP=g++
FLEX=flex
BISON=bison

.lex: lex.l
	$(FLEX) lex.l
.syntax: syntax.y
	$(BISON) -t -d syntax.y -Wconflicts-sr
splc: clean  .syntax .lex
	@mkdir bin
	$(CPP) ast.hpp syntax.tab.c semantic.hpp tac.hpp -lfl -ly -o bin/splc
	@chmod +x bin/splc
debug: clean  .syntax .lex
	@mkdir bin
	$(CPP) ast.hpp syntax.tab.c semantic.hpp tac.hpp -D DEBUG -lfl -ly -o bin/splc
	@chmod +x bin/splc
gen:
	$(FLEX) tmp.l
	$(CC) lex.yy.c -lfl -o test.out
clean:
	@rm -rf bin/ lex.yy.c syntax.tab.*
.PHONY: splc