scull-objs := scull_pipe.o
obj-m := scull_pipe.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

install:
	sudo insmod scull_pipe.ko
uninstall:
	sudo rmmod scull_pipe

