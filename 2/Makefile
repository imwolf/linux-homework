CC=gcc
CFLAGS=-g -Wall
SRC=mysh.c parseline.c
all: 
	flex  -o parseline.c parseline.l
	$(CC) $(CFLAGS) -o mysh $(SRC) -lreadline	
