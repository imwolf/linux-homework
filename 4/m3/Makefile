obj-m := pm3.o

MOD = pm3

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

rmmod:
	sudo rmmod $(MOD)

install:
	sudo insmod $(MOD).ko

install1:
	sudo insmod $(MOD).ko param=1

test:
	cat /proc/$(MOD)dir/$(MOD)file
