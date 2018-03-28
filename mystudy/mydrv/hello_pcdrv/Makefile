KERN_DIR = $(shell uname -r)

build: kernel_modules
kernel_modules:
	make -C /lib/modules/$(KERN_DIR)/build M=$(CURDIR) modules 

clean:
	make -C /lib/modules/$(KERN_DIR)/build M=$(CURDIR) clean
	rm -rf modules.order

obj-m	+= hello.o
