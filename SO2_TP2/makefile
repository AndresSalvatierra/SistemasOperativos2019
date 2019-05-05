#Makefile de Trabajo Practico N°2 de Sistemas Operativos

all: matriz

matriz : readNetcdf.c
	gcc -O3 -o matriz readNetcdf.c `nc-config --cflags --libs`

clean:
	rm matriz \
