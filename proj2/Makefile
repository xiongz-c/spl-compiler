CC=gcc
FLEX=flex
BISON=bison

.lex: lex.l
	$(FLEX) lex.l
.syntax: syntax.y
	$(BISON) -t -d syntax.y -Wconflicts-sr
splc: clean  .syntax .lex
	@mkdir bin
	g++ ast.c syntax.tab.c semantic.cpp -lfl -ly -o bin/splc
	@chmod +x bin/splc
gen:
	$(FLEX) tmp.l
	$(CC) lex.yy.c -lfl -o test.out
clean:
	@rm -rf bin/ lex.yy.c syntax.tab.*
.PHONY: splc