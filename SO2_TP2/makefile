#Makefile de Trabajo Practico N°2 de Sistemas Operativos

all: netcdf

netcdf : readNetcdf.c
	gcc -Wall -Werror -pedantic -lm -lnetcdf -o matriz readNetcdf.c

clean:
	rm matriz \
