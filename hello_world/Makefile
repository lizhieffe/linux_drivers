# a goal definition and define the module to be built (hello.o). The syntax is
# surprisingly intricate, for example obj-m defines a loadable module goal,
# whereas obj-y indicates a built-in object goal. 

obj-m+=hello_world.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
