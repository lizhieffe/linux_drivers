# a goal definition and define the module to be built (hello.o). The syntax is
# surprisingly intricate, for example obj-m defines a loadable module goal,
# whereas obj-y indicates a built-in object goal. 

# obj-m will make the source build as loadable module.
# obj-y will make the source build as built-in module.
# obj-n will make the source not compiled.
obj-m += hello_world.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build/

all default: modules

install: modules_install

modules modules_install help clean:
	make -C $(KERNELDIR) M=$(shell pwd) $@
