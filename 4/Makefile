obj-m := proc_module.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

rmmod:
	sudo rmmod proc_module

install:
	sudo insmod proc_module.ko
