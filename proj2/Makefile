CC=gcc
FLEX=flex
BISON=bison

.lex: lex.l
	$(FLEX) lex.l
.syntax: syntax.y
	$(BISON) -t -d syntax.y -Wconflicts-sr
splc: clean  .syntax .lex
	@mkdir bin
	g++ ast.hpp syntax.tab.c semantic.hpp -lfl -ly -o bin/splc
	@chmod +x bin/splc
debug: clean  .syntax .lex
	@mkdir bin
	g++ ast.hpp syntax.tab.c semantic.hpp -D DEBUG -lfl -ly -o bin/splc 
	@chmod +x bin/splc
gen:
	$(FLEX) tmp.l
	$(CC) lex.yy.c -lfl -o test.out
clean:
	@rm -rf bin/ lex.yy.c syntax.tab.*
.PHONY: splc