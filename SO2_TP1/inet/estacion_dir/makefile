all: satelite.bin

satelite.bin : satelite.c
	arm-linux-gnueabi-gcc -Wall -Werror -pedantic -o satelite.bin satelite.c

clean :
	rm satelite.bin \
