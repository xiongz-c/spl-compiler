CC=gcc
FLEX=flex
BISON=bison

.lex: lex.l
	$(FLEX) lex.l
.syntax: syntax.y
	$(BISON) -t -d syntax.y
splc: .lex .syntax
	@mkdir bin
	$(CC) syntax.tab.c -lfl -ly -o bin/splc
	@chmod +x bin/splc lex.yy.c syntax.tab.*
clean:
	@rm -rf bin/
.PHONY: splc