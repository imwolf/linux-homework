obj-m := pm1.o

MOD = pm1

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

rmmod:
	sudo rmmod pm1

install:
	sudo insmod pm1.ko

test:
	cat /proc/$(MOD)dir/$(MOD)file
