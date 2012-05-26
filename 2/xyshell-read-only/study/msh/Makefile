CC=gcc
CFLAGS=-g -Wall
SRC=msh.c lex.yy.c

all:
	flex parsecmd.l
	$(CC) $(CFLAGS) -o msh $(SRC) -lfl -lreadline -ltermcap

clean:
	rm -f msh msh.o lex.yy.c
