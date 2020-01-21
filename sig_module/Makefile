obj-m += signal_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test:
	-sudo rmmod signal_module
	sudo dmesg -C
	sudo insmod signal_module.ko
	dmesg