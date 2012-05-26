TARGETS = e_open e_wait e_signal e_close e_open2
CCFLAGS = -g -Wall
CC = gcc

all:$(TARGETS)

%.o:%.c
	$(CC) -c $< $(CCFLAGS)
