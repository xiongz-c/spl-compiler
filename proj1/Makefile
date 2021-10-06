CC=gcc
FLEX=flex
BISON=bison

.lex: lex.l
	$(FLEX) lex.l
.syntax: syntax.y
	$(BISON) -t -d syntax.y
splc: clean  .syntax .lex
	@mkdir bin
	$(CC) syntax.tab.c ast.c -lfl -ly -o bin/splc
	@chmod +x bin/splc
gen:
	$(FLEX) tmp.l
	$(CC) lex.yy.c -lfl -o test.out
clean:
	@rm -rf bin/ lex.yy.c syntax.tab.*
.PHONY: splc