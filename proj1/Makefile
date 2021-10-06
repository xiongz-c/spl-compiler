CC=gcc
FLEX=flex
BISON=bison

.lex: lex.l
	$(FLEX) lex.l
.syntax: syntax.y
	$(BISON) -t -d syntax.y
splc: .lex .syntax
	@mkdir bin
	$(CC) ast.c syntax.tab.c -lfl -ly -o bin/splc
	@chmod +x bin/splc
gen:
	$(FLEX) tmp.l
	$(CC) lex.yy.c -lfl -o test.out
clean:
	@rm -rf bin/ lex.yy.c syntax.tab.*
.PHONY: splc