obj-m := my_dt_driver.o
KDIR := /home/book/100ask_imx6ull-qemu/linux-4.9.88
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules    

clean:
	make -C $(KDIR) M=$(PWD) clean      
