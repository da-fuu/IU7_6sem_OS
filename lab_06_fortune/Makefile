ifneq ($(KERNELRELEASE),)
	obj-m := md.o
else
	CURRENT = $(shell uname -r)
	KDIR = /lib/modules/$(CURRENT)/build
	PWD = $(shell pwd)
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	@rm -f *.o .*.cmd .*.flags *.mod.c *.order *.mod .*.o ./.tmp_*
	@rm -f .*.*.cmd TODO.*
	@rm -fR .tmp*
	@rm -rf .tmp_versions
disclean: clean
	rm -f *.ko *.symvers
test: default
	sudo dmesg -C
	sudo insmod md.ko
	sudo rmmod md.ko
	sudo dmesg
endif
