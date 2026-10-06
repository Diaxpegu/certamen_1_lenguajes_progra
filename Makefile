CC = gcc
CFLAGS ?= -Wall -Wextra -std=gnu11
FLEX ?= flex
BISON ?= bison

.PHONY: all build run clean zip

all: turing

turing: main.c turing.h lex.yy.c parser.tab.c
	$(CC) $(CFLAGS) -o $@ main.c lex.yy.c parser.tab.c

parser.tab.c parser.tab.h &: parser.y turing.h
	$(BISON) -d -o parser.tab.c parser.y

lex.yy.c: lexer.l parser.tab.h
	$(FLEX) -o $@ lexer.l

build: turing

run: turing
	./turing maquina.tm

clean:
	rm -f lex.yy.c parser.tab.c parser.tab.h turing

zip: clean
	zip -r Certamen1-Diego_Peña-Nataniel_Riquelme.zip . -x "*.git*" "*.DS_Store"
