CC=gcc
FLEX=flex
BISON=bison


wc:
	$(FLEX) lex.l
	$(CC) lex.yy.c -lfl -o wc.out
splc:
	@mkdir bin
	touch bin/splc
	@chmod +x bin/splc
clean:
	@rm -rf bin/
.PHONY: splc