MODSRC=/var/www/html/andres/modulo
obj-m+=helloWorld.o

all: module

module:
	make -C /lib/modules/$(shell uname -r)/build M=${MODSRC} modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=${MODSRC} clean
