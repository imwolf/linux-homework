obj-m += mt_syscall.o

CC=gcc

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

rmmod:
	sudo rmmod mt_syscall

insmod:
	sudo insmod mt_syscall.ko
