obj-m := pm2.o

MOD = pm2
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

rmmod:
	sudo rmmod pm2

install:
	sudo insmod pm2.ko

test:
	cat /proc/$(MOD)dir/$(MOD)file
