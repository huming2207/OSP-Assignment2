obj-m += main.o


all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) test.c -g -o test

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm test