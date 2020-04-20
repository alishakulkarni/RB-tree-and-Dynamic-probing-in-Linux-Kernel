#THIS IS JUST TO INSERT THE BASIC MODULE
CC=gcc

obj-m := rbt530_drv.o 



all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
