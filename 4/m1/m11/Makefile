obj-m := pm11.o

MOD = pm11

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

rmmod:
	sudo rmmod $(MOD)

install:
	sudo insmod $(MOD).ko

test:
	cat /proc/$(MOD)dir/$(MOD)file
