#Makefile start...
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CC = gcc
CCFLAGS= -g -Wall
TARGET = mytimer inter_moniter mygettimeofday fibnocci clock

all:$(TARGET)


%.o:%.c
	$(CC) -c $< $(CCFLAGS)
clean:
	rm *.o
#End Makefile
